/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Apple Inc.
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
 * \file InputDPX.cpp
 *
 * \brief
 *    InputDPX class C++ file for allowing input from DPX files
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
#include <assert.h>
#include "InputDPX.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

InputDPX::InputDPX(IOVideo *videoFile, FrameFormat *format) {
  // We currently only support fixed precision data
  m_isFloat         = FALSE;
  format->m_isFloat = m_isFloat;
  videoFile->m_format.m_isFloat = m_isFloat;
  m_frameRate       = format->m_frameRate;
  
  m_memoryAllocated = FALSE;
  m_size      = 0;
  m_prevSize  = -1;
  
  m_buf       = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
    
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  m_ui16Comp[A_COMP] = NULL;
  
  // Color space is explicitly specified here. We could do a test inside the code though to at least check if
  // RGB is RGB and YUV is YUV, but not important currently
  m_colorSpace       = format->m_colorSpace;
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;
  m_components       = 3;
  
}

InputDPX::~InputDPX() {
  
  freeMemory();
  
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
/*!
 ************************************************************************
 * \brief
 *   Get an unsigned short without swapping.
 *
 ************************************************************************
 */
static uint16 getU16 (DPXFileData * t)
{
  uint16 in;
  
  *(((char *) &in) + 0) = *t->m_mp++;
  *(((char *) &in) + 1) = *t->m_mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an unsigned int32 without swapping.
 *
 ************************************************************************
 */
static uint32 getU32 (DPXFileData * t)
{
  uint32 in;
  
  *(((char *) &in) + 0) = *t->m_mp++;
  *(((char *) &in) + 1) = *t->m_mp++;
  *(((char *) &in) + 2) = *t->m_mp++;
  *(((char *) &in) + 3) = *t->m_mp++;
  
  return in;
  
}

/*!
 ************************************************************************
 * \brief
 *   Get an single (float) without swapping.
 *
 ************************************************************************
 */
static float getFloat (DPXFileData * t)
{
  float in;
  
  *(((char *) &in) + 0) = *t->m_mp++;
  *(((char *) &in) + 1) = *t->m_mp++;
  *(((char *) &in) + 2) = *t->m_mp++;
  *(((char *) &in) + 3) = *t->m_mp++;
  
  return in;
  
}


// Swap versions

/*!
 ************************************************************************
 * \brief
 *   Get an unsigned short and swap.
 *
 ************************************************************************
 */
static uint16 getSwappedU16 (DPXFileData * t)
{
  uint16 in;
  
  *(((char *) &in) + 1) = *t->m_mp++;
  *(((char *) &in) + 0) = *t->m_mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an unsigned int32 and swap.
 *
 ************************************************************************
 */
static uint32 getSwappedU32 (DPXFileData * t)
{
  uint32 in;
  
  *(((char *) &in) + 3) = *t->m_mp++;
  *(((char *) &in) + 2) = *t->m_mp++;
  *(((char *) &in) + 1) = *t->m_mp++;
  *(((char *) &in) + 0) = *t->m_mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an single (float) and swap.
 *
 ************************************************************************
 */
static float getSwappedFloat (DPXFileData * t)
{
  float in;
  
  *(((char *) &in) + 3) = *t->m_mp++;
  *(((char *) &in) + 2) = *t->m_mp++;
  *(((char *) &in) + 1) = *t->m_mp++;
  *(((char *) &in) + 0) = *t->m_mp++;
  
  return in;
}

/*!
 ************************************************************************
 * \brief
 *   Read N characters
 *
 ************************************************************************
 */
static void getCharString (char *in, DPXFileData * t, int count)
{
  memcpy(in, t->m_mp, count * sizeof(char));    
  t->m_mp += count;
}

void InputDPX::printHeader(DPXFileData * dpx) {
  printf("magic %d\n",        dpx->m_fileFormat.m_fileHeader.m_magic);
  printf("offset %d\n",       dpx->m_fileFormat.m_fileHeader.m_imageOffset);
  printf("version %s\n",      dpx->m_fileFormat.m_fileHeader.m_version);
  printf("fileSize %d\n",     dpx->m_fileFormat.m_fileHeader.m_fileSize);
  printf("dittoKey %d\n",     dpx->m_fileFormat.m_fileHeader.m_dittoKey);
  printf("genericSize %d\n",  dpx->m_fileFormat.m_fileHeader.m_genericSize);
  printf("industrySize %d\n", dpx->m_fileFormat.m_fileHeader.m_industrySize);
  printf("userSize %d\n",     dpx->m_fileFormat.m_fileHeader.m_userSize);
  printf("fileName %s\n",     dpx->m_fileFormat.m_fileHeader.m_fileName);
  printf("TimeDate %s\n",     dpx->m_fileFormat.m_fileHeader.m_timeDate);
  printf("creator %s\n",      dpx->m_fileFormat.m_fileHeader.m_creator);
  printf("Project %s\n",      dpx->m_fileFormat.m_fileHeader.m_project);
  printf("Copyright %s\n",    dpx->m_fileFormat.m_fileHeader.m_copyright);
  printf("EncryptKey %d\n",   dpx->m_fileFormat.m_fileHeader.m_encryptKey);
  printf("== Image Header ==\n");
  printf("orientation %d\n",  dpx->m_fileFormat.m_imageHeader.m_orientation);
  printf("#Elements %d\n",    dpx->m_fileFormat.m_imageHeader.m_numberElements);
  printf("pelsPerLine %d\n",  dpx->m_fileFormat.m_imageHeader.m_pixelsPerLine);
  printf("lnsPerEl %d\n",     dpx->m_fileFormat.m_imageHeader.m_linesPerElement);
  for (int ii = 0; ii < dpx->m_fileFormat.m_imageHeader.m_numberElements; ii++) {
    printf("Component %d\n", ii);
    printf("dataSign %d\n",     dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_dataSign);
    printf("lowData %d\n",      dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_lowData);
    printf("highData %d\n",     dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_highData);
    printf("descriptor %d\n",   dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_descriptor);
    printf("transfer %d\n",     dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_transfer);
    printf("colorimetric %d\n", dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_colorimetric);
    printf("bitSize %d\n",      dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_bitSize);
    printf("packing %d\n",      dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_packing);
    printf("encoding %d\n",     dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_encoding);
    printf("dataOffset %d\n",   dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_dataOffset);
    printf("description %s\n",  dpx->m_fileFormat.m_imageHeader.m_imageElement[ii].m_description);
  }
  printf("== Orientation Header ==\n");
  printf("xOffset %d\n", dpx->m_fileFormat.m_orientHeader.m_xOffset);
  printf("yOffset %d\n", dpx->m_fileFormat.m_orientHeader.m_yOffset); 
  printf("xCenter %7.3f\n", dpx->m_fileFormat.m_orientHeader.m_xCenter); 
  printf("yCenter %7.3f\n", dpx->m_fileFormat.m_orientHeader.m_yCenter); 
  printf("xOriginalSize %d\n", dpx->m_fileFormat.m_orientHeader.m_xOriginalSize);
  printf("yOriginalSize %d\n", dpx->m_fileFormat.m_orientHeader.m_yOriginalSize); 
}

/*!
 ************************************************************************
 * \brief
 *   Read file named 'path' into memory buffer 'm_buffer'.
 *
 * \return
 *   0 if successful
 ************************************************************************
 */
int InputDPX::readFileIntoMemory (DPXFileData * dpx, int *fd)
{
  long cnt, result;
  int endian = 1;

  int machineLittleEndian = (*( (char *)(&endian) ) == 1) ? 1 : 0;
    
  cnt = (long) lseek( *fd, 0, SEEK_END); // DPX files by definition cannot exceed 2^32
  if (cnt == -1L)
    return 1;
  
  if (lseek( *fd, 0, SEEK_SET) == -1L)   // reposition file at beginning
    return 1;
  
  dpx->m_buffer.resize(cnt);
  
  result = (long) mm_read( *fd, (char *) &dpx->m_buffer[0], cnt);
  if (result != cnt) {
    if (*fd != - 1) {
      close( *fd);
      *fd = -1;
    }
    return 1;
  }
  
  //dpx->m_fileFormat = *((DPXFileFormat *) &dpx->m_buffer[0]);
  dpx->m_fileFormat.m_fileHeader.m_magic = (dpx->m_buffer[0] << 24) | (dpx->m_buffer[1] << 16) | (dpx->m_buffer[2] << 8) | (dpx->m_buffer[3]);

  switch (dpx->m_fileFormat.m_fileHeader.m_magic) {
    case 0x58504453:                        // little endian file
      dpx->m_le = 1;
      //printf( "Little endian file\n");
      break;
    case 0x53445058:                        // big endian file
      dpx->m_le = 0;
      //printf( "Big endian file\n");
      break;
    default:
      fprintf( stderr, "First four bytes indicate:  Not a DPX file\n");
      return 1;
  }
  
  if (dpx->m_le == machineLittleEndian)  { // endianness of machine matches file
    dpx->getU16   = getU16;
    dpx->getU32   = getU32;
    dpx->getFloat = getFloat;
  }
  else {                               // endianness of machine does not match file
    dpx->getU16   = getSwappedU16;
    dpx->getU32   = getSwappedU32;
    dpx->getFloat = getSwappedFloat;
  }
  dpx->m_mp = (uint8 *) &dpx->m_buffer[0];
  return 0;
}

int InputDPX::readGenericFileHeader(DPXFileData * t) {
  DPXGenericFileHeader *fileHeader = &t->m_fileFormat.m_fileHeader;

  fileHeader->m_magic        = t->getU32(t);
  fileHeader->m_imageOffset  = t->getU32(t);
  getCharString (fileHeader->m_fileName,  t, 8);      /* Version stamp of header format */
  fileHeader->m_fileSize     =  t->getU32(t);         /* Total DPX file size in bytes */
  fileHeader->m_dittoKey     =  t->getU32(t);         /* Image content specifier */
  fileHeader->m_genericSize  =  t->getU32(t);         /* Generic section header length in bytes */
  fileHeader->m_industrySize =  t->getU32(t);         /* Industry-specific header length in bytes */
  fileHeader->m_userSize     =  t->getU32(t);         /* User-defined data length in bytes */
  getCharString (fileHeader->m_fileName,  t, 100);    /* Name of DPX file */
  getCharString (fileHeader->m_timeDate,  t, 24);     /* Time and date of file creation */
  getCharString (fileHeader->m_creator,   t, 100);    /* Name of file creator */
  getCharString (fileHeader->m_project,   t, 200);    /* Name of project */
  getCharString (fileHeader->m_copyright, t, 200);    /* File contents copyright information */  
  fileHeader->m_encryptKey  =  t->getU32(t);          /* Encryption key */
  t->m_mp += 104;                                     /* Padding */
  
  return 0;
}

int InputDPX::readImageElement(DPXFileData * t, DPXImageElement *imageElement) {
  imageElement->m_dataSign     = t->getU32(t);          /* Data sign extension */
  imageElement->m_lowData      = t->getU32(t);          /* Reference low data code value */
  imageElement->m_lowQuantity  = t->getFloat(t);        /* Reference low quantity represented */
  imageElement->m_highData     = t->getU32(t);          /* Reference high data code value */
  imageElement->m_highQuantity = t->getFloat(t);        /* Reference high quantity represented */
  imageElement->m_descriptor   = *(t->m_mp++);          /* Descriptor for image element */
  imageElement->m_transfer     = *(t->m_mp++);          /* Transfer characteristics for element */
  imageElement->m_colorimetric = *(t->m_mp++);          /* Colormetric specification for element */
  imageElement->m_bitSize      = *(t->m_mp++);          /* Bit size for element */
  imageElement->m_packing      = t->getU16(t);          /* Packing for element */
  imageElement->m_encoding     = t->getU16(t);          /* Encoding for element */
  imageElement->m_dataOffset   = t->getU32(t);          /* Offset to data of element */
  imageElement->m_endOfLinePadding  = t->getU32(t);     /* End of line padding used in element */
  imageElement->m_endOfImagePadding = t->getU32(t);     /* End of image padding used in element */
  getCharString (imageElement->m_description, t, 32);   /* Description of element */ 

  return 0;
}

int InputDPX::readGenericImageHeader(DPXFileData * t) {
  DPXGenericImageHeader *imageHeader = &t->m_fileFormat.m_imageHeader;
  
  imageHeader->m_orientation     = t->getU16(t);        /* Image orientation */
  imageHeader->m_numberElements  = t->getU16(t);        /* Number of image elements */
  imageHeader->m_pixelsPerLine   = t->getU32(t);        /* Pixels per line */
  imageHeader->m_linesPerElement = t->getU32(t);        /* Lines per image element */
  for (int i = 0; i < imageHeader->m_numberElements; i++)
    readImageElement(t, &imageHeader->m_imageElement[i]);
  t->m_mp += 52;             // Reserved field used for padding

  return 0;
}

int InputDPX::readGenericOrientationHeader(DPXFileData * t) {
  DPXGenericOrientationHeader *orientHeader = &t->m_fileFormat.m_orientHeader;
  int i;
  
  orientHeader->m_xOffset     = t->getU32(t);           /* X offset */
  orientHeader->m_yOffset     = t->getU32(t);           /* Y offset */
  orientHeader->m_xCenter     = t->getFloat(t);         /* X center */
  orientHeader->m_yCenter     = t->getFloat(t);         /* Y center */
  orientHeader->m_xOriginalSize     = t->getU32(t);     /* X original size */
  orientHeader->m_yOriginalSize     = t->getU32(t);     /* Y original size */
  getCharString (orientHeader->m_fileName, t, 100);     /* Source image file name */
  getCharString (orientHeader->m_timeDate, t, 24);      /* Source image date and time */
  getCharString (orientHeader->m_inputName, t, 32);     /* Input device name */
  getCharString (orientHeader->m_inputSN, t, 32);       /* Input device serial number */
  for (i = 0; i < 4; i++)
    orientHeader->m_order[i]        = t->getU16(t);          /* Border validity (XL, XR, YT, YB) */
  for (i = 0; i < 2; i++)
    orientHeader->m_aspectRatio[i]  = t->getU32(t);    /* Pixel aspect ratio (H:V) */
  t->m_mp += 28;                                       /* Reserved field used for padding */

  return 0;
}

int InputDPX::readIndustryFilmInfoHeader(DPXFileData * t) {
  // Currently ignored
  return 0;
}

int InputDPX::readIndustryTelevisionInfoHeader(DPXFileData * t) {
  // Currently ignored
  return 0;  
}

int InputDPX::readUserDefinedData(DPXFileData * t) {
  // Currently ignored
  return 0;  
}

/*!
 ************************************************************************
 * \brief
 *    Read image data into 't->m_img'.
 *
 ************************************************************************
 */
int InputDPX::readImageData (DPXFileData * t)
{
// Following code was only tested for 10 bit "filled" big endian data
// It is likely incorrect for other formats and needs to be tested.
  int     j, n;
  uint8  *mp, *s;
  uint16 *p;
  uint32 imageOffset = t->m_fileFormat.m_fileHeader.m_imageOffset;
  byte   isPacked = t->m_fileFormat.m_imageHeader.m_imageElement[0].m_packing == 0 ? true : false;
  byte   bitSize = t->m_fileFormat.m_imageHeader.m_imageElement[0].m_bitSize;
  uint32 width  = t->m_fileFormat.m_imageHeader.m_pixelsPerLine;
  uint32 height = t->m_fileFormat.m_imageHeader.m_linesPerElement;
  if (isPacked) {
    int    bitCount  = 0;
    int    dataCount = 0;
    int    byteCount = 0;
    int    bitOffset = 0;
    uint16 byteData;
    uint32 size = (width * height * m_components * bitSize + 7) >> 3;
    
    t->m_img.resize(size);

    switch (bitSize) {
      case 8:
        p = (uint16 *) &t->m_img[0];
        s = (uint8 *) &t->m_buffer[imageOffset];
        for (j = 0; j < (int) size; ++j) {
          *p++ = *s++;
        }
        break;
      case 10:
        mp = t->m_mp;                       // save memory pointer
        p = (uint16 *) &t->m_img[0];
        n = size;
        t->m_mp = (uint8 *) &t->m_buffer[imageOffset];
        for (j=0; j < n; ++j) {
          bitCount = dataCount * bitSize;
          byteCount = bitCount >> 3;
          bitOffset = bitCount - (byteCount << 3);
          byteData = *((uint16 * ) &t->m_mp[byteCount]);
          *p++ = (byteData << bitOffset) >> bitOffset;
          dataCount++;
        }
        t->m_mp = mp;                       // restore memory pointer
        break;
      case 16:
        mp = t->m_mp;                       // save memory pointer
        p = (uint16 *) &t->m_img[0];
        t->m_mp = (uint8 *) &t->m_buffer[imageOffset];
        for (j = 0; j < (int) size; ++j) {
          *p++ = (uint16) t->getU16( t);
        }
        t->m_mp = mp;                       // restore memory pointer
        break;
      default:
        printf("bitdepth currently not supported\n");
    }
  }
  else {    
    uint32 samplesPerDWord = 32 / bitSize;
    uint32 size = ((width * height * 4) * m_components + samplesPerDWord - 1) / samplesPerDWord;
    uint32 sizeDWord = (size + 3) / 4;
    uint32 curDWord;

    switch (bitSize) {
      case 8:
        t->m_img.resize(size);
        p = (uint16 *) &t->m_img[0];
        s = (uint8 *) &t->m_buffer[imageOffset];
        for (j = 0; j < (int) size; ++j) {
          *p++ = *s++;
        }
        break;
      case 10:
        t->m_img.resize(size * 3);
        mp = t->m_mp;                       // save memory pointer
        p = (uint16 *) &t->m_img[0];
        t->m_mp = (uint8 *) &t->m_buffer[imageOffset];
        for (j = 0; j < (int) sizeDWord; ++j) {
          curDWord = t->getU32( t) >> 2;
          *p++ = (uint16) ((curDWord >> 20) & 0x3FF);
          *p++ = (uint16) ((curDWord >> 10) & 0x3FF);
          *p++ = (uint16) ((curDWord >>  0) & 0x3FF);
        }
        t->m_mp = mp;                       // restore memory pointer
        break;
      case 12:
        t->m_img.resize(size * 2);
        mp = t->m_mp;                       // save memory pointer
        p = (uint16 *) &t->m_img[0];
        t->m_mp = (uint8 *) &t->m_buffer[imageOffset];
        for (j = 0; j < (int) sizeDWord; ++j) {
          curDWord = t->getU32( t) >> 4;
          *p++ = (uint16) ((curDWord >> 16) & 0xFFF);
          *p++ = (uint16) ((curDWord >>  0) & 0xFFF);
        }
        t->m_mp = mp;                       // restore memory pointer
        break;
      case 16:
        t->m_img.resize(size);
        mp = t->m_mp;                       // save memory pointer
        p = (uint16 *) &t->m_img[0];
        t->m_mp = (uint8 *) &t->m_buffer[imageOffset];
        for ( j =0; j < (int) size; ++j) {
          *p++ = (uint16) t->getU16( t);
        }
        t->m_mp = mp;                       // restore memory pointer
        break;
      default:
        printf("bitdepth currently not supported\n");
    }

  }
  return 0;
}



/*!
 *****************************************************************************
 * \brief
 *    Read the DPX file named 'path' into 't'.
 *
 *****************************************************************************
 */
int InputDPX::readDPX (FrameFormat *format, int *fd) {
  assert( &m_dpxFile);
  
  if (readFileIntoMemory( &m_dpxFile, fd))
    goto Error;
    
  if (readGenericFileHeader( &m_dpxFile ))
      goto Error;
  if (readGenericImageHeader( &m_dpxFile ))
    goto Error;     
  if(readGenericOrientationHeader( &m_dpxFile ))
    goto Error;
    
  //printHeader(&m_dpxFile);
  
  allocateMemory(&m_dpxFile, format);
  
  if (readImageData( &m_dpxFile))
    goto Error;
  
  return 1;
  
Error:
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    Open file containing a single frame
 ************************************************************************
 */
int InputDPX::openFrameFile( IOVideo *inputFile, int FrameNumberInFile)
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
    //exit(EXIT_FAILURE);
    return -1;
  }
  
  return inputFile->m_fileNum;
}

void InputDPX::allocateMemory(DPXFileData * t, FrameFormat *format)
{
  format->m_height[Y_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = t->m_fileFormat.m_imageHeader.m_linesPerElement;
  format->m_width [Y_COMP] = format->m_width [U_COMP] = format->m_width [V_COMP] = t->m_fileFormat.m_imageHeader.m_pixelsPerLine;
  
  m_height[Y_COMP] = format->m_height[Y_COMP];
  m_width [Y_COMP] = format->m_width [Y_COMP];
  
  // We are only dealing with 4:4:4 data here
  m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
  m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];
  
  format->m_compSize[Y_COMP] = m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  format->m_compSize[U_COMP] = m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  format->m_compSize[V_COMP] = m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];
  
  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  
  format->m_height[A_COMP] = m_height[A_COMP];
  format->m_width [A_COMP] = m_width [A_COMP];
  format->m_compSize[A_COMP] = m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
  
  
  // Note that we do not read alpha but discard it
  format->m_size = m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  switch (t->m_fileFormat.m_imageHeader.m_imageElement[0].m_descriptor)
  {
    case DPX_DS_LUMINANCE:
    // Currently not supported
      format->m_colorSpace = m_colorSpace = CM_YCbCr;
      format->m_chromaFormat = m_chromaFormat = CF_400;
      m_components = 1;
      break;
    case DPX_DS_RGB:
      format->m_colorSpace = m_colorSpace = CM_RGB;
      format->m_chromaFormat = m_chromaFormat = CF_444;
      m_components = 3;
      break;
    default:
      printf("Unsupported DPX format\n");
      format->m_chromaFormat = m_chromaFormat = CF_444;
      break;    
  }
  
  format->m_bitDepthComp[Y_COMP] = m_bitDepthComp[Y_COMP] = t->m_fileFormat.m_imageHeader.m_imageElement[0].m_bitSize;
  format->m_bitDepthComp[U_COMP] = m_bitDepthComp[U_COMP] = t->m_fileFormat.m_imageHeader.m_imageElement[0].m_bitSize;
  format->m_bitDepthComp[V_COMP] = m_bitDepthComp[V_COMP] = t->m_fileFormat.m_imageHeader.m_imageElement[0].m_bitSize;
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

  
  m_comp[Y_COMP]      = NULL;
  m_comp[U_COMP]      = NULL;
  m_comp[V_COMP]      = NULL;
  
  if (m_memoryAllocated == FALSE || m_size != m_prevSize) {
    m_ui16Data.resize((unsigned int) m_size);
    m_ui16Comp[Y_COMP]  = &m_ui16Data[0];
    m_ui16Comp[U_COMP]  = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
    m_ui16Comp[V_COMP]  = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
  }
  
  m_prevSize          = m_size;
  m_ui16Comp [A_COMP] = NULL;
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  
  m_floatComp[A_COMP] = NULL;

  m_memoryAllocated = TRUE;

}

void InputDPX::freeMemory()
{
    m_ui16Comp[Y_COMP] = NULL;
    m_ui16Comp[U_COMP] = NULL;
    m_ui16Comp[V_COMP] = NULL;
    m_ui16Comp[A_COMP] = NULL;
    m_floatComp[Y_COMP] = NULL;
    m_floatComp[U_COMP] = NULL;
    m_floatComp[V_COMP] = NULL;
    m_floatComp[A_COMP] = NULL;
}


/*!
 ************************************************************************
 * \brief
 *    Reformat Image data
 ************************************************************************
 */
int InputDPX::reformatData () {
  int i, k;
  uint16 *comp0 = NULL;
  uint16 *comp1 = NULL;
  uint16 *comp2 = NULL;

  uint16 *curBuf = (uint16 *) &m_dpxFile.m_img[0];
  
  // Unpack the data appropriately (interleaving is done at the row level).
  for (k = 0; k < m_height[Y_COMP]; k++) {
    comp0   = &m_ui16Comp[0][k * m_width[0]];
    comp1   = &m_ui16Comp[1][k * m_width[1]];
    comp2   = &m_ui16Comp[2][k * m_width[2]];
    for (i = 0; i < m_width[0]; i++) {
      *comp0++ = *curBuf++;
      *comp1++ = *curBuf++;
      *comp2++ = *curBuf++;
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
 *    Reads one new frame from a single TIFF file
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
int InputDPX::readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileRead = 1;
  int *vfile = &inputFile->m_fileNum;
  FrameFormat *source = &inputFile->m_format;
  
  if (openFrameFile( inputFile, frameNumber + frameSkip) != -1) {
    
    fileRead = readDPX( source, vfile);
    reformatData ();
    
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

