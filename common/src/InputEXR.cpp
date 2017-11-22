/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Apple Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the <ORGANIZATION> nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 *************************************************************************************
 * \file InputEXR.cpp
 *
 * \brief
 *    InputEXR class C++ file for allowing input from OpenEXR files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <string.h>
#include "InputEXR.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

InputEXR::InputEXR(IOVideo *videoFile, FrameFormat *format) {
  // We currently only support floating point data
  m_isFloat           = TRUE;
  format->m_isFloat   = m_isFloat;
  videoFile->m_format.m_isFloat = m_isFloat;
  m_frameRate         = format->m_frameRate;
  
  m_memoryAllocated = FALSE;
  m_name = new char[255];
  m_type = new char[255];
  m_attributeSize = 0;
  m_value.resize(255);
  m_noChannels = 0;
  m_channels = new channelInfo[6];
  
  m_buf = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  m_offsetTableSize = 0;
  // Color space is explicitly specified here. We could do a test inside the code though to at least check if
  // RGB is RGB and YUV is YUV, but not important currently
  m_colorSpace       = format->m_colorSpace;
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;

}

InputEXR::~InputEXR() {
  if (m_channels != NULL) {
    delete [] m_channels;
    m_channels = NULL;
  }
  if (m_name != NULL) {
    delete [] m_name;
    m_name = NULL;
  }
  if (m_type != NULL) {
    delete [] m_type;
    m_type = NULL;
  }

  m_offsetTableSize = 0;
  
  freeMemory();
  
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

unsigned int InputEXR::halfToFloat (uint16 value)
{
  int sign        = (value >> 15) & 0x00000001;
  int exponent    = (value >> 10) & 0x0000001f;
  int significand =  value        & 0x000003ff;

  // First handle all special cases (e.g. if exponent is 0 or 31)
  if (exponent == 0) {
    if (significand == 0) {
	    return (sign << 31);
    }
    else  {
	    while (!(significand & 0x00000400)) {
        significand <<= 1;
        exponent -=  1;
	    }
	    significand &= ~0x00000400;
	    exponent += 1;
    }
  }
  else if (exponent == 31) {
    if (significand == 0) {
	    return ((sign << 31) | 0x7f800000);
    }
    else {
	    return ((sign << 31) | 0x7f800000 | (significand << 13));
    }
  }
  
  // consider exponent bias (127 for single, 15 for half => 127 - 15 = 112)
  exponent += 112;
  // extend significand precision from 10 to 23 bits
  significand <<= 13;
  
  // Gather sign, exponent, and significand and reconstruct the number as single precision.
  return ((sign << 31) | (exponent << 23) | significand);
}

/*!
 ************************************************************************
 * \brief
 *    Open file containing a single frame
 ************************************************************************
 */
int InputEXR::openFrameFile( IOVideo *inputFile, int FrameNumberInFile)
{
  char infile [FILE_NAME_SIZE], in_number[16];
  int length = 0;
  in_number[length]='\0';
  length = (int) strlen(inputFile->m_fHead);
  strncpy(infile, inputFile->m_fHead, length);
  infile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(inputFile->m_fTail) != 0) {
    if (inputFile->m_zeroPad == TRUE)
      snprintf(in_number, 16, "%0*d", inputFile->m_numDigits, FrameNumberInFile);
    else
      snprintf(in_number, 16, "%*d", inputFile->m_numDigits, FrameNumberInFile);
    
    //strncat(infile, in_number, sizeof(in_number));
    strncat(infile, in_number, strlen(in_number));
    length += sizeof(in_number);
    infile[length]='\0';
    strncat(infile, inputFile->m_fTail, strlen(inputFile->m_fTail));
    length += (int) strlen(inputFile->m_fTail);
    infile[length]='\0';
  }
  
  if ((inputFile->m_fileNum = open(infile, OPENFLAGS_READ)) == -1)  {
    printf ("openFrameFile: cannot open file %s\n", infile);
    return -1;
    //exit(EXIT_FAILURE);
  }
  
  return inputFile->m_fileNum;
}

void InputEXR::allocateMemory(FrameFormat *format)
{
  m_memoryAllocated = TRUE;
  format->m_height[Y_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_dataWindow.yMax + 1;
  format->m_width [Y_COMP] = format->m_width [U_COMP] = format->m_width [V_COMP] = m_dataWindow.xMax + 1;
  
  m_height[Y_COMP] = format->m_height[Y_COMP];
  m_width [Y_COMP] = format->m_width [Y_COMP];
  
  // We are only dealing with 4:4:4 data here
  m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
  m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];
  
  format->m_compSize[Y_COMP] = m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  format->m_compSize[U_COMP] = m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  format->m_compSize[V_COMP] = m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];
  if (m_noChannels > 3) {
    m_height [A_COMP] = m_height [Y_COMP];
    m_width  [A_COMP] = m_width  [Y_COMP];
  }
  else {
    m_height [A_COMP] = 0;
    m_width  [A_COMP] = 0;
  }
  format->m_height[A_COMP] = m_height[A_COMP];
  format->m_width [A_COMP] = m_width [A_COMP];
  format->m_compSize[A_COMP] = m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
 
  
  // Note that we do not read alpha but discard it
  format->m_size = m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  //format->m_colorSpace   = m_colorSpace   = CM_RGB;
  format->m_chromaFormat = m_chromaFormat = CF_444;
  
  format->m_bitDepthComp[Y_COMP] = m_bitDepthComp[Y_COMP] = 16;
  format->m_bitDepthComp[U_COMP] = m_bitDepthComp[U_COMP] = 16;
  format->m_bitDepthComp[V_COMP] = m_bitDepthComp[V_COMP] = 16;
  format->m_bitDepthComp[A_COMP] = m_bitDepthComp[A_COMP] = 16;
  
  m_isInterleaved = FALSE;
  m_isInterlaced  = FALSE;
  
  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];    
  }

  
  if (m_channels[Y_COMP].pixelType == HALF)
    m_buffer.resize((unsigned int) m_size * 2);
  else
    m_buffer.resize((unsigned int) m_size * 4);

  m_buf = &m_buffer[0];
  
  format->m_pixelType[Y_COMP] = m_pixelType[Y_COMP] = m_channels[Y_COMP].pixelType;
  format->m_pixelType[U_COMP] = m_pixelType[U_COMP] = m_channels[U_COMP].pixelType;
  format->m_pixelType[V_COMP] = m_pixelType[V_COMP] = m_channels[V_COMP].pixelType;
  format->m_pixelType[A_COMP] = m_pixelType[A_COMP] = m_channels[A_COMP].pixelType;
  
  m_comp[Y_COMP]      = NULL;
  m_comp[U_COMP]      = NULL;
  m_comp[V_COMP]      = NULL;
  m_ui16Comp[Y_COMP]  = NULL;
  m_ui16Comp[U_COMP]  = NULL;
  m_ui16Comp[V_COMP]  = NULL;
  m_floatData.resize((unsigned int) m_size);
  m_floatComp[Y_COMP] = &m_floatData[0];
  m_floatComp[U_COMP] = m_floatComp[Y_COMP] + m_compSize[Y_COMP];
  m_floatComp[V_COMP] = m_floatComp[U_COMP] + m_compSize[V_COMP];
  m_floatComp[A_COMP] = m_floatComp[V_COMP] + m_compSize[A_COMP];
}

void InputEXR::freeMemory()
{ 
  m_buf = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
}

int InputEXR::readAttributeInfo( int vfile, FrameFormat *source)
{
  int cCount = 0;
  int nCount = 0;
  while (1) {
    cCount = 0;
    nCount ++;
    // read attribute name
    while (1) {
      if (mm_read(vfile, &m_name[cCount], sizeof(char)) != sizeof(char)) {
        printf ("InputEXR::readAttributeInfo: Unexpected end of file reached. Cannot read further!\n");
        return 0;
      }
      else {
        cCount++;
        if (m_name[cCount - 1] == 0) {
          // end of reading
          break;
        }
      }
    }

    if (cCount == 1) {
      break;
    }
    // read attribute type
    cCount = 0;
    while (1) {
      if (mm_read(vfile, &m_type[cCount], sizeof(char)) != sizeof(char)) {
        printf ("InputEXR::readAttributeInfo: Unexpected end of file reached. Cannot read further!\n");
        return 0;
      }
      else {
        cCount++;
        if (m_type[cCount - 1] == 0) {
          break;
        }
      }
    }

    // read attribute size
    int attributeSize = 0;
    if (mm_read(vfile, (char *) &attributeSize, sizeof(int)) != sizeof(int)) {
      printf ("InputEXR::readAttributeInfo: Cannot read attribute size from input file, unexpected EOF!\n");
      return 0;
    }
    m_value.resize(attributeSize);
    m_attributeSize = attributeSize;
  
    if (mm_read(vfile, &m_value[0], m_attributeSize) != m_attributeSize) {
      printf ("InputEXR::readAttributeInfo: Cannot read size sufficient data (%d) for current attribute in header. Unexpected EOF!\n", m_attributeSize);
      return 0;
    }
    
    //Parsing process for attributes
    if (strncmp(m_name, "channels", 8) == 0) {
      if (strncmp(m_type, "chlist", 6) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for channels attribute\n");
        return 0;
      }
      int channel = 0;
      int countData = 0;
      char *p_channelData = &m_value[0];
      
      // parse channel information
      while (countData < (m_attributeSize - 1)) {
        if (channel > 4) {
          printf("InputEXR::readAttributeInfo: Too many channels found in file. File not supported\n");
          return 0;
        }
        strncpy(m_channels[channel].name, p_channelData, strlen(p_channelData));
        
        //printf("name %s\n", m_channels[channel].name);
        p_channelData += strlen(p_channelData) + 1;
        countData     += (int) strlen(p_channelData) + 1;
        m_channels[channel].pixelType = *(int32 *)p_channelData;
        if (m_channels[channel].pixelType != HALF && m_channels[channel].pixelType != FLOAT) {
          printf("InputEXR::readAttributeInfo: Only half and single precision float data are supported (type %d). Please check your input file.\n", m_channels[channel].pixelType);
          return 0;
        }
        p_channelData += sizeof(int32);
        countData     += sizeof(int32);
        m_channels[channel].pLinear = *(uint32 *)p_channelData;
        p_channelData += sizeof(uint32);
        countData     += sizeof(uint32);
        m_channels[channel].xSampling = *(int32 *)p_channelData;
        p_channelData += sizeof(int32);
        countData     += sizeof(int32);
        m_channels[channel].ySampling = *(int32 *)p_channelData;
        p_channelData += sizeof(int32);
        countData     += sizeof(int32);
        
        channel++;
      }
      m_noChannels = channel;
    }

    if (strncmp(m_name, "compression", 11) == 0) {
      if (strncmp(m_type, "compression", 11) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for compression attribute\n");
        return 0;
      }
      
      if (m_value[0] != NO_COMPRESSION) {
        printf("InputEXR::readAttributeInfo: Invalid compression method (%d). Please use uncompressed data files.\n", m_value[0]);
        return 0;
      }
    }
    
    if (strncmp(m_name, "dataWindow", 10) == 0) {
      if (strncmp(m_type, "box2i", 5) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for dataWindow attribute\n");
        return 0;
      }
      m_dataWindow.xMin = *((int32 *) &m_value[0]);
      m_dataWindow.yMin = *((int32 *) &m_value[4]);
      m_dataWindow.xMax = *((int32 *) &m_value[8]);
      m_dataWindow.yMax = *((int32 *) &m_value[12]);
      
      if (m_offsetTableSize < (m_dataWindow.yMax + 1)) {
        m_offsetTable.resize(m_dataWindow.yMax + 1);
      }
      m_offsetTableSize = (m_dataWindow.yMax + 1);
    }
    
    if (strncmp(m_name, "displayWindow", 13) == 0) {
      if (strncmp(m_type, "box2i", 5) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for displayWindow attribute\n");
        return 0;
      }
      m_displayWindow.xMin = *((int32 *) &m_value[0]);
      m_displayWindow.yMin = *((int32 *) &m_value[4]);
      m_displayWindow.xMax = *((int32 *) &m_value[8]);
      m_displayWindow.yMax = *((int32 *) &m_value[12]);
    }
    
    if (strncmp(m_name, "lineOrder", 9) == 0) {
      if (strncmp(m_type, "lineOrder", 9) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for lineOrder attribute\n");
        return 0;
      }
      if (m_value[0] != '\0') {
        printf("InputEXR::readAttributeInfo: Invalid lineOrder value. Only 0 value supported.\n");
        return 0;
      }
    }
    if (strncmp(m_name, "pixelAspectRatio", 16) == 0) {
      if (strncmp(m_type, "float", 5) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for float attribute\n");
        return 0;
      }
      // Value is currently ignored
    }
    if (strncmp(m_name, "screenWindowCenter", 18) == 0) {
      if (strncmp(m_type, "v2f", 3) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for screenWindowCenter attribute\n");
        return 0;
      }
      // Value is currently ignored.
    }
    if (strncmp(m_name, "screenWindowWidth", 17) == 0) {
      if (strncmp(m_type, "float", 5) != 0) {
        printf("InputEXR::readAttributeInfo: Incorrect type for screenWindowWidth attribute\n");
        return 0;
      }
      // Value is currently ignored
    }
    //printf("Attribute Name: %s, Type: %s, size: %d, value: %s\n", m_name, m_type, m_attributeSize, m_value);
  }
  
  return 1;
}

/*!
 ************************************************************************
 * \brief
 *    Read Header data
 ************************************************************************
 */
int InputEXR::readHeaderData( int vfile, FrameFormat *source)
{
  if (lseek (vfile, 0, SEEK_SET) == -1) {
    printf("InputEXR::readHeaderData: cannot lseek to start of input file\n");
    return 0;
  }
  else {
    int count = 0;
    // Read magic number
    if (mm_read(vfile, (char *) &m_magicNumber, sizeof(int)) != sizeof(int)) {
      printf ("InputEXR::readHeaderData: cannot read magic number from input file, unexpected EOF!\n");
      return 0;
    }
    else {
      if (m_magicNumber != 20000630) {
        printf ("InputEXR::readHeaderData: Invalid magic number (%d != 20000630). Likely not an OpenEXR image.\n", m_magicNumber);
        return 0;
      }
      count += sizeof(int);
    }
    
    if (mm_read(vfile, (char *) &m_versionField, sizeof(int)) != sizeof(int)) {
      printf ("InputEXR::readHeaderData: Cannot read version field from input file, unexpected EOF!\n");
      return 0;
    }
    else {
      // Parse data
      m_version      = (char) (m_versionField & 0x00FF);
      m_isTile       =        (m_versionField & 0x0200) ? TRUE : FALSE;
      m_hasLongNames =        (m_versionField & 0x0400) ? TRUE : FALSE;
      m_hasDeepData  =        (m_versionField & 0x0800) ? TRUE : FALSE;
      m_isMultipart  =        (m_versionField & 0x1000) ? TRUE : FALSE;
      m_reserved     =        (m_versionField & 0xE000);
      if (m_version > 2) {
        printf ("InputEXR::readHeaderData: Version numbers (%d) larger than 2 are not supported. Invalid file.\n", m_version);
        return 0;
      }
      if (m_isTile == TRUE) {
        printf ("InputEXR::readHeaderData: Tiled format files are not supported. Invalid file.\n");
        return 0;
      }
      if (m_hasLongNames == TRUE) {
        printf ("InputEXR::readHeaderData: Attributes with long names (>31 bytes) are not supported. Invalid file.\n");
        return 0;
      }
      if (m_hasDeepData == TRUE) {
        printf ("InputEXR::readHeaderData: Deep data are not supported. Invalid file.\n");
        return 0;
      }
      if (m_isMultipart == TRUE) {
        printf ("InputEXR::readHeaderData: Multipart files are not supported. Invalid file.\n");
        return 0;
      }
      if (m_reserved != 0) {
        printf ("InputEXR::readHeaderData: Invalid version number field. Invalid file.\n");
        return 0;
      }
      count += sizeof(int);
      // Now lets read the header data
      if (readAttributeInfo( vfile, source) == 0) {
        printf ("InputEXR::readHeaderData: Error reading attributes!\n");
        return 0;
      }
    }
    
    for (int i = 0; i < m_offsetTableSize; i++) {
      if (mm_read(vfile, (char *) &m_offsetTable[i], sizeof(uint64)) != sizeof(uint64)) {
        printf ("InputEXR::readHeaderData: cannot read offset number from input file, unexpected EOF!\n");
        return 0;
      }
      // Currently the following code should be seen as an "error concealment trick"
      // There is nothing in the specification that mentions encountering zero offsets
      //if (m_offsetTable[i] == 0 && i > 2)
      //m_offsetTable[i] = 2 * m_offsetTable[i - 1] - m_offsetTable[i - 2];
    }
    return count;
  }
}

int InputEXR::readData (int vfile,  FrameFormat *source, uint8 *buf) {
  uint8 *curBuf = buf;
  int i, k;
  int isZero = FALSE;

  for (i = 0; i < m_offsetTableSize; i++) {
    if (m_offsetTable[i] != 0)
      lseek(vfile, m_offsetTable[i], SEEK_SET);
    else {
      if (isZero == FALSE) {
        printf("InputEXR::readData(): Zero value EXR Table offset encountered.\n");
        isZero = TRUE;
      }
    }
    
    if ((k = mm_read(vfile, (char *) &m_yCoordinate, sizeof(int))) != sizeof(int)) {
      return 0;
    }

    if (mm_read(vfile, (char *) &m_pixelDataSize, sizeof(int)) != sizeof(int)) {
      printf ("InputEXR::readData(): cannot read m_pixelDataSize from input file, unexpected EOF!\n");
      return 0;
    }
    
    if (mm_read(vfile, curBuf, m_pixelDataSize) != m_pixelDataSize) {
      printf ("InputEXR::readData(): cannot read %d bytes from input file, unexpected EOF!\n", m_pixelDataSize);
      return 0;
    }
    curBuf += m_pixelDataSize;
  }

  return 1;
}

int InputEXR::reformatData (uint8 *buf,  float  *floatComp[4]) {
  int i, j, k;
  int component[4] = {V_COMP, U_COMP, Y_COMP, A_COMP};
  uint32 *comp = NULL;
  
  if (m_noChannels == 4) {
    component[0] = A_COMP;
    component[1] = B_COMP;
    component[2] = G_COMP;
    component[3] = R_COMP;
  }
  
  if (m_channels[Y_COMP].pixelType == HALF) {
    uint16 *curBuf = (uint16 *) buf;
    // Unpack the data appropriately (interleaving is done at the row level).
    for (k = 0; k < m_height[Y_COMP]; k++) {
      for (j = 0; j < m_noChannels; j++) {
        comp   = (uint32 *) &floatComp[component[j]][k * m_width[component[j]]];
        for (i = 0; i < m_width[component[j]]; i++) {
          // Convert data from half precision to float.
          *comp++ = halfToFloat(*curBuf++);
        }
      }
    }
  }
  else {
    uint32 *curBuf = (uint32 *) buf;

    // Unpack the data appropriately (interleaving is done at the row level).
    for (k = 0; k < m_height[Y_COMP]; k++) {
      for (j = 0; j < m_noChannels; j++) {
        comp   = (uint32 *) &floatComp[component[j]][k * m_width[component[j]]];
        for (i = 0; i < m_width[component[j]]; i++) {
          *comp++ = *curBuf++;
        }
      }
    }
  }
  
  return 1;
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

/*!
 ************************************************************************
 * \brief
 *    Reads one new frame from concatenated raw file
 *
 * \param inputFile
 *    Input file to read from
 * \param frameNumber
 *    Frame number in the source file
 * \param fileHeader
 *    Number of bytes in the source file to be skipped
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int InputEXR::readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileRead = 0;
  int *vfile = &inputFile->m_fileNum;
  FrameFormat *source = &inputFile->m_format;
  
  if (openFrameFile( inputFile, frameNumber + frameSkip) != -1) {
    
    if (readHeaderData( *vfile, source) == 0) {
      if (*vfile != -1) {
        close(*vfile);
        *vfile = -1;
      }
      return fileRead;
    }
    
    if (m_memoryAllocated == FALSE) {
      allocateMemory(source);
    }
    
    fileRead = readData (*vfile, source, m_buf);
    
    reformatData (m_buf,  m_floatComp);
    
    if (*vfile != -1) {
      close(*vfile);
      *vfile = -1;
    }
    return fileRead;
  } 
  else 
    return 0; 
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

