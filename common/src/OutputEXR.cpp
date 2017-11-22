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
 * \file OutputEXR.cpp
 *
 * \brief
 *    OutputEXR class C++ file for allowing input from OpenEXR files
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
#include "OutputEXR.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------


static uint16 floatToHalfTrunc (uint32 value)
{
  int sign        = (value >> 31) & 0x00000001;
  int exponent    = (value >> 23) & 0x000000ff;
  int significand =  value        & 0x007fffff;
  
  // First handle all special cases (e.g. if exponent is 0 or 255)
  if (exponent == 0) {
    if (significand == 0) {
      return (sign << 15);
    }
    else  { // subnormal numbers
      while (!(significand & 0x00800000)) {
        significand <<= 1;
        exponent -=  1;
      }
      significand &= ~0x00800000;
      exponent += 1;
    }
  }
  else if (exponent == 255 ) {
    if (significand == 0) {
      return ((sign << 15) | 0x7C00);
    }
    else {
      return ((sign << 15) | 0x7C00 | (significand >> 13));
    }
  }
  
  // subtract floating point (single) exponent bias 127
  exponent -= 127;
  if (exponent > 15) {
    return ((sign << 15) | 0x7C00);
  }
  else if (exponent < -14 ) {
    return (sign << 15);
  }
  // Now add half float bias to the exponent (+15)
  exponent += 15;
  
  // reduce significand precision from 23 to 10 bits
  significand >>= 13;
  
  // Gather sign, exponent, and significand and reconstruct the number as single precision.
  return ((sign << 15) | (exponent << 10) | significand);
}

static uint16 floatToHalfRound(uint32 value)
{
  FP32 *f = (FP32 *) &value;
  FP16 o = { 0 };
  
  // Based on ISPC reference code (with minor modifications)
  if (f->Exponent == 0) // Signed zero/denormal (which will underflow)
    o.Exponent = 0;
  else if (f->Exponent == 255) // Inf or NaN (all exponent bits set)
  {
    o.Exponent = 31;
    o.Significand = f->Significand ? 0x200 : 0; // NaN->qNaN and Inf->Inf
  }
  else // Normalized number
  {
    // Exponent unbias the single, then bias the halfp
    int newexp = f->Exponent - 127 + 15;
    if (newexp >= 31) // Overflow, return signed infinity
      o.Exponent = 31;
    else if (newexp <= 0) // Underflow
    {
      if ((14 - newexp) <= 24) // Mantissa might be non-zero
      {
        uint32 mant = f->Significand | 0x800000; // Hidden 1 bit
        o.Significand = mant >> (14 - newexp);
        if ((mant >> (13 - newexp)) & 1) // Check for rounding
          o.u++; // Round, might overflow into exp bit, but this is OK
      }
    }
    else
    {
      o.Exponent = newexp;
      o.Significand = f->Significand >> 13;
      if (f->Significand & 0x1000) // Check for rounding
        o.u++; // Round, might overflow to inf, this is OK
    }
  }
  
  o.Sign = f->Sign;
  
  // return value
  return *((uint16*) &o);
}

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

OutputEXR::OutputEXR(IOVideo *outputFile, FrameFormat *format) {
  // We currently only support floating point data
  m_isFloat         = TRUE;
  format->m_isFloat = m_isFloat;
  m_frameRate       = format->m_frameRate;

  m_memoryAllocated = FALSE;
  m_name = new char[255]();
  m_type = new char[255]();
  m_attributeSize = 0;

  // size if m_value set to 256. If at any point a new attribute is introduced of larger size,
  // a user should ensure m_value is properly "resized"
  m_valueVectorSize = 256;
  m_value.resize(m_valueVectorSize);

  m_noChannels = 3;
  m_channels = new channelInfo[6]();
  
  m_buf = NULL;
    
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  
  m_offsetTableSize = 0;

  m_colorSpace       = format->m_colorSpace;
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;

  // set default values
  m_magicNumber  = 20000630;
  m_version      = 2;
  m_isTile       = FALSE;
  m_hasLongNames = FALSE;
  m_hasDeepData  = FALSE;
  m_isMultipart  = FALSE;
  m_versionField = m_reserved = 0;

  m_versionField = (m_versionField << 1) | (int) m_isMultipart;
  m_versionField = (m_versionField << 1) | (int) m_hasDeepData;
  m_versionField = (m_versionField << 1) | (int) m_hasLongNames;
  m_versionField = (m_versionField << 1) | (int) m_isTile;
  m_versionField = (m_versionField << 9) | (int) m_version;
  
  m_pixelType[Y_COMP] = format->m_pixelType[Y_COMP];
  m_pixelType[U_COMP] = format->m_pixelType[U_COMP];
  m_pixelType[V_COMP] = format->m_pixelType[V_COMP];
  m_pixelType[A_COMP] = format->m_pixelType[A_COMP];
  
  if (format->m_useFloatRound)
    floatToHalf = &floatToHalfRound;
  else
    floatToHalf = &floatToHalfTrunc;

  allocateMemory(format);
}

OutputEXR::~OutputEXR() {
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

/*!
 ************************************************************************
 * \brief
 *    Open file containing a single frame
 ************************************************************************
 */
int OutputEXR::openFrameFile( IOVideo *outputFile, int FrameNumberInFile)
{
  char outfile [FILE_NAME_SIZE], outNumber[16];
  int length = 0;
  outNumber[length]='\0';
  length = (int) strlen(outputFile->m_fHead);
  strncpy(outfile, outputFile->m_fHead, length);
  outfile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(outputFile->m_fTail) != 0) {
    if (outputFile->m_zeroPad)
      snprintf(outNumber, 16, "%0*d", outputFile->m_numDigits, FrameNumberInFile);
    else
      snprintf(outNumber, 16, "%*d", outputFile->m_numDigits, FrameNumberInFile);
    
    //strncat(infile, in_number, sizeof(in_number));
    strncat(outfile, outNumber, strlen(outNumber));
    length += sizeof(outNumber);
    outfile[length]='\0';
    strncat(outfile, outputFile->m_fTail, strlen(outputFile->m_fTail));
    length += (int) strlen(outputFile->m_fTail);
    outfile[length]='\0';
  }
  
  if ((outputFile->m_fileNum = open(outfile, OPENFLAGS_WRITE, OPEN_PERMISSIONS)) == -1)  {
    printf ("openFrameFile: cannot open file %s\n", outfile);
    exit(EXIT_FAILURE);
  }
  
  return outputFile->m_fileNum;
}

void OutputEXR::allocateMemory(FrameFormat *format)
{
  m_memoryAllocated = TRUE;
  
  m_chromaFormat = format->m_chromaFormat;

  m_height[Y_COMP] = format->m_height[Y_COMP];
  m_width [Y_COMP] = format->m_width [Y_COMP];
  
  // We are only dealing with 4:4:4 data here
  format->m_height[U_COMP] = format->m_height[V_COMP] = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
  format->m_width[U_COMP]  = format->m_width[V_COMP]  = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];

/*
    switch (m_chromaFormat){
    case CF_400:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = 0;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = 0;
      break;
    case CF_420:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP] >> ONE;
      break;
    case CF_422:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      break;
    case CF_444:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP];
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      break;
    default:
      fprintf(stderr, "\n Unsupported Chroma Subsampling Format %d\n", m_chromaFormat);
      exit(EXIT_FAILURE);
  }
  */
  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  
  m_compSize[Y_COMP] = format->m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  m_compSize[U_COMP] = format->m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  m_compSize[V_COMP] = format->m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];
  m_compSize[A_COMP] = format->m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
  
  
  m_size = format->m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  m_dataWindow.xMin    = 0;
  m_dataWindow.yMin    = 0;
  m_dataWindow.xMax    = m_width[Y_COMP] - 1;
  m_dataWindow.yMax    = m_height[Y_COMP] - 1;
  m_displayWindow.xMin = 0;
  m_displayWindow.yMin = 0;
  m_displayWindow.xMax = m_width[Y_COMP] - 1;
  m_displayWindow.yMax = m_height[Y_COMP] - 1;
  
  m_bitDepthComp[Y_COMP] = format->m_bitDepthComp[Y_COMP];
  m_bitDepthComp[U_COMP] = format->m_bitDepthComp[U_COMP];
  m_bitDepthComp[V_COMP] = format->m_bitDepthComp[V_COMP];
  
  m_isInterleaved = FALSE;

  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];    
  }

  for (int channel = 0; channel < m_noChannels; channel++) {
    if (channel == 0)
      strcpy(m_channels[channel].name, "B");
    else if (channel == 1)
      strcpy(m_channels[channel].name, "G");
    else
      strcpy(m_channels[channel].name, "R");
    
    m_channels[channel].pixelType = m_pixelType[channel];
    m_channels[channel].pLinear   = 0;
    m_channels[channel].xSampling = 1;
    m_channels[channel].ySampling = 1;
  }
  
  if (m_channels[Y_COMP].pixelType == HALF)
    m_buffer.resize((unsigned int) m_size * 2);
  else
    m_buffer.resize((unsigned int) m_size * 4);
  
  m_buf               = &m_buffer[0];
  m_comp[Y_COMP]      = NULL;
  m_comp[U_COMP]      = NULL;
  m_comp[V_COMP]      = NULL;
  m_ui16Comp[Y_COMP]  = NULL;
  m_ui16Comp[U_COMP]  = NULL;
  m_ui16Comp[V_COMP]  = NULL;
  
  m_floatData.resize((unsigned int) m_size);
  m_floatComp[Y_COMP] = &m_floatData[0];
  m_floatComp[U_COMP] = m_floatComp[Y_COMP] + m_compSize[Y_COMP];
  m_floatComp[V_COMP] = m_floatComp[U_COMP] + m_compSize[U_COMP];
}

void OutputEXR::freeMemory()
{
  m_buf = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
}

int OutputEXR::writeAttributeAndType( int vfile, char *attributeName, char *attributeType, int attributeSize, char *attributeValue) {
  int lenName = (int) strlen(attributeName);
  int lenType = (int) strlen(attributeType);
  int cCount = 0;
  char endByte = 0;
  
  // Write attribute name
  if (mm_write(vfile, attributeName, lenName) != lenName) {
    printf ("Unexpected end of file reached. Cannot write further!\n");
    return 0;
  }
  else {
    cCount+=lenName;
  }

  if (mm_write(vfile, &endByte, 1) != 1) {
    printf ("Unexpected end of file reached. Cannot write further!\n");
    return 0;
  }
  else {
    cCount++;
  }

  if (*attributeName == 0)
    return cCount;
  
  // Write attribute type
  if (mm_write(vfile, attributeType, lenType) != lenType) {
    printf ("Unexpected end of file reached. Cannot write further!\n");
    return 0;
  }
  else {
    cCount+=lenType;
  }

  if (mm_write(vfile, &endByte, 1) != 1) {
    printf ("Unexpected end of file reached. Cannot write further!\n");
    return 0;
  }
  else {
    cCount++;
  }

  // Write attribute size
  if (mm_write(vfile, (char *) &attributeSize, sizeof(int)) != sizeof(int)) {
    printf ("Cannot write attribute size to output file, unexpected EOF!\n");
    return 0;
  }
  else
    cCount += sizeof(int);

  if (mm_write(vfile, attributeValue, attributeSize) != attributeSize) {
    printf ("Cannot write attribute data in header. Unexpected EOF!\n");
    return 0;
  }
  else
    cCount += attributeSize;

  
  return cCount;
}

void resetAttributeSize(int attributeSize, int *prevAttributeSize, char *value) {
  if (attributeSize != *prevAttributeSize) {
    delete [] value;
    value = new char[attributeSize]();
  }
  *prevAttributeSize = attributeSize;
}

int OutputEXR::writeAttributeInfo( int vfile, FrameFormat *source)
{
  int   cCount = 0;
  int   nCount = 0;
  int   countData = 0;
  char *p_channelData;
  
  // compute channel size
  for (int channel = 0; channel < m_noChannels; channel++) {
    countData += 2;              // Channel Name + null character
    countData += sizeof(int32);  // PixelType
    countData += sizeof(uint32); // Linear
    countData += sizeof(int32);  // xSampling
    countData += sizeof(int32);  // ySampling
  }
  m_attributeSize = countData + 1;

  // resize if needed because of multiple channels
  if (m_attributeSize > m_valueVectorSize) {
    m_valueVectorSize = m_attributeSize;
    m_value.resize(m_valueVectorSize);
  }

  // Now do the proper writing
  p_channelData = &m_value[0];
  
  strcpy(m_name, "channels");
  strcpy(m_type, "chlist");
  
  for (int channel = 0; channel < m_noChannels; channel++) {
    strncpy(p_channelData++, m_channels[channel].name, 1);
    *(p_channelData++) = 0;
    *(int32 *)p_channelData = m_channels[channel].pixelType;
    p_channelData += sizeof(int32);
    *(uint32 *)p_channelData = m_channels[channel].pLinear;
    p_channelData += sizeof(uint32);
    *(int32 *)p_channelData = m_channels[channel].xSampling;
    p_channelData += sizeof(int32);
    *(int32 *)p_channelData = m_channels[channel].ySampling;
    p_channelData += sizeof(int32);
  }
  
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;
  
  strcpy(m_name, "compression");
  strcpy(m_type, "compression");
  m_attributeSize = 1; // m_attributeSize < m_valueVectorSize

  m_value[0] = NO_COMPRESSION;
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;

  strcpy(m_name, "dataWindow");
  strcpy(m_type, "box2i");
  m_attributeSize = 16; // m_attributeSize < m_valueVectorSize
  
  *((int32 *) &m_value[0])  = m_dataWindow.xMin;
  *((int32 *) &m_value[4])  = m_dataWindow.yMin;
  *((int32 *) &m_value[8])  = m_dataWindow.xMax;
  *((int32 *) &m_value[12]) = m_dataWindow.yMax;
  m_offsetTableSize = (m_dataWindow.yMax + 1);
  
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;

  strcpy(m_name, "displayWindow");
  strcpy(m_type, "box2i");
  m_attributeSize = 16; // m_attributeSize < m_valueVectorSize

  *((int32 *) &m_value[0])  = m_displayWindow.xMin;
  *((int32 *) &m_value[4])  = m_displayWindow.yMin;
  *((int32 *) &m_value[8])  = m_displayWindow.xMax;
  *((int32 *) &m_value[12]) = m_displayWindow.yMax;
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;
  
  strcpy(m_name, "lineOrder");
  strcpy(m_type, "lineOrder");
  m_attributeSize = 1; // m_attributeSize < m_valueVectorSize

  m_value[0] = FALSE;
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;

  strcpy(m_name, "pixelAspectRatio");
  strcpy(m_type, "float");
  m_attributeSize = 4; // m_attributeSize < m_valueVectorSize

  *((float *) &m_value[0]) = 1.0;
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;
  
  strcpy(m_name, "screenWindowCenter");
  strcpy(m_type, "v2f");
  m_attributeSize = 8; // m_attributeSize < m_valueVectorSize

  *((double *) &m_value[0]) = 0.0;
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;

  strcpy(m_name, "screenWindowWidth");
  strcpy(m_type, "float");
  m_attributeSize = 4; // m_attributeSize < m_valueVectorSize

  *((float *) &m_value[0]) = 1.0; // (float) m_width[Y_COMP];
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;

  *m_name = 0;
  strcpy(m_type, "");
  m_attributeSize = 0;
  
  strcpy(&m_value[0], "");
  cCount += writeAttributeAndType( vfile, m_name, m_type, m_attributeSize, &m_value[0]);
  nCount++;
  
  return cCount;
}

/*!
 ************************************************************************
 * \brief
 *    Read Header data
 ************************************************************************
 */
int OutputEXR::writeHeaderData( int vfile, FrameFormat *source)
{
  if (lseek (vfile, 0, SEEK_SET) == -1) {
    printf("writeHeaderData: cannot lseek to start of input file\n");
    return 0;
  }
  else {
    int count = 0;
    // Write magic number
    if (mm_write(vfile, (char *) &m_magicNumber, sizeof(int)) != sizeof(int)) {
      printf ("cannot write magic number to output file, unexpected EOF!\n");
      return 0;
    }
    else {
      count += sizeof(int);
    }
    
    if (mm_write(vfile, (char *) &m_versionField, sizeof(int)) != sizeof(int)) {
      printf ("Cannot write version field to output file, unexpected EOF!\n");
      return 0;
    }
    else {
      count += sizeof(int);
      // Now lets write the header data
      if (writeAttributeInfo( vfile, source) == 0) {
        printf ("Error reading attributes!\n");
        return 0;
      }
    }
    
    int i;
   
    int curPos = (int) tell((int) vfile);
    // create offset table and then write

    m_offsetTable.resize(m_offsetTableSize);
    
    if (m_channels[Y_COMP].pixelType == HALF)
      m_pixelDataSize = 2 * m_noChannels * m_width[Y_COMP];
    else
      m_pixelDataSize = 4 * m_noChannels * m_width[Y_COMP];

    for (i = 0; i < m_offsetTableSize; i++) {
      m_offsetTable[i] = m_offsetTableSize * sizeof(uint64) + i * (m_pixelDataSize + 2 * sizeof(int))+ curPos;
      if (mm_write(vfile, (char *) &m_offsetTable[i], sizeof(uint64)) != sizeof(uint64)) {
        printf ("cannot write m_offsetTable number to output file!\n");
        return 0;
      }
    }
    return count;
  }
}

int OutputEXR::writeData (int vfile,  uint8 *buf) {
  uint8 *curBuf = buf;

  for (int i = 0; i < m_offsetTableSize; i++) {
    lseek(vfile, m_offsetTable[i], SEEK_SET);
    
    m_yCoordinate = i;
    if (mm_write(vfile, (char *) &m_yCoordinate, sizeof(int)) != sizeof(int)) {
      printf ("cannot write m_yCoordinate to output file!\n");
      return 0;
    }

    if (mm_write(vfile, (char *) &m_pixelDataSize, sizeof(int)) != sizeof(int)) {
      printf ("cannot write m_pixelDataSize to output file!\n");
      return 0;
    }
    
    if (mm_write(vfile, curBuf, m_pixelDataSize) != m_pixelDataSize) {
      printf ("readData: cannot write %d bytes to output file!\n", m_pixelDataSize);
      return 0;
    }
    curBuf += m_pixelDataSize;
  }

  return 1;
}

int OutputEXR::reformatData (uint8 *buf,  float  *floatComp[3]) {
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
          // Convert data from single precision to half.
          *curBuf++ = floatToHalf(*comp++);
        }
      }
    }
  }
  else {
    uint32 *curBuf = (uint32 *) buf;

    for (k = 0; k < m_height[Y_COMP]; k++) {
      for (j = 0; j < m_noChannels; j++) {
        comp   = (uint32 *) &floatComp[component[j]][k * m_width[component[j]]];
        for (i = 0; i < m_width[component[j]]; i++) {
          *curBuf++ = *comp++;
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
 * \param outputFile
 *    Input file to write to
 * \param frameNumber
 *    Frame number in the source file
 * \param f_header
 *    Number of bytes in the source file to be skipped
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int OutputEXR::writeOneFrame (IOVideo *outputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileWrite = 0;
  int *vfile = &outputFile->m_fileNum;
  FrameFormat *format = &outputFile->m_format;
  
  openFrameFile( outputFile, frameNumber + frameSkip);
  
  if (writeHeaderData( *vfile, format) == 0) {
    if (*vfile != -1) {
      close(*vfile);
      *vfile = -1;
    }
  }
  
  if (m_memoryAllocated == FALSE) {
    allocateMemory(format);
  }

  reformatData (m_buf,  m_floatComp);
  
  fileWrite = writeData (*vfile, m_buf);
  
  
  if (*vfile != -1) {
    close(*vfile);
    *vfile = -1;
  }
    
  return fileWrite;
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

