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
 * \file InputY4M.cpp
 *
 * \brief
 *    InputY4M class C++ file for allowing input of YUV4MPEG2 files
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
#include "InputY4M.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

InputY4M::InputY4M(IOVideo *inputFile, FrameFormat *format) {

  m_isFloat                   = FALSE;
  format->m_isFloat           = m_isFloat;

  m_streamHeader.resize(256);
  m_streamHeaderSize = 256;
  readStreamHeader (inputFile,  format);

  m_colorSpace                = format->m_colorSpace;
  m_colorPrimaries            = format->m_colorPrimaries;
  m_transferFunction          = format->m_transferFunction;
  m_systemGamma               = format->m_systemGamma;
  m_sampleRange               = format->m_sampleRange;

  m_chromaFormat              = format->m_chromaFormat;
  m_isInterleaved             = inputFile->m_isInterleaved;
  m_isInterlaced              = format->m_isInterlaced;
  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];    
  }

  m_pixelFormat               = format->m_pixelFormat;
  m_frameRate                 = format->m_frameRate;
  m_bitDepthComp[Y_COMP]      = format->m_bitDepthComp[Y_COMP];
  m_bitDepthComp[U_COMP]      = format->m_bitDepthComp[U_COMP];
  m_bitDepthComp[V_COMP]      = format->m_bitDepthComp[V_COMP];
  m_height[Y_COMP]            = format->m_height[Y_COMP];
  m_width [Y_COMP]            = format->m_width [Y_COMP];

  if (format->m_chromaFormat == CF_420) {
    m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP] >> 1;
    m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP] >> 1;
  }
  else if (format->m_chromaFormat == CF_422) {
    m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
    m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP] >> 1;
  }
  else {
    m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
    m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];
  }

  m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];

  format->m_height[U_COMP] = format->m_height[V_COMP] = m_height [V_COMP];
  format->m_width [U_COMP] = format->m_width [V_COMP] = m_width  [V_COMP];
  format->m_compSize[Y_COMP] = m_compSize[Y_COMP];
  format->m_compSize[U_COMP] = m_compSize[U_COMP];
  format->m_compSize[V_COMP] = m_compSize[V_COMP];

  m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP];
  m_bufSize = m_size;
  if (m_pixelFormat == PF_B64A)
    m_bufSize += m_compSize[A_COMP];

  format->m_size = m_size;

  format->m_picUnitSizeOnDisk = (iMax(format->m_bitDepthComp[Y_COMP], format->m_bitDepthComp[U_COMP]) > 8) ? 16 : 8;
  format->m_picUnitSizeShift3 = format->m_picUnitSizeOnDisk >> 3;

  if (m_isInterleaved) {
    m_iBuffer.resize((unsigned int) m_bufSize * format->m_picUnitSizeShift3);
    m_iBuf = &m_iBuffer[0];
  }

  m_buffer.resize((unsigned int) m_bufSize * format->m_picUnitSizeShift3);
  m_buf = &m_buffer[0];

  if (format->m_picUnitSizeShift3 > 1) {
    m_ui16Data.resize((unsigned int) m_size);
    m_ui16Comp[Y_COMP] = &m_ui16Data[0];
    m_ui16Comp[U_COMP] = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
    m_ui16Comp[V_COMP] = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
    m_comp[Y_COMP] = NULL;
    m_comp[U_COMP] = NULL;
    m_comp[V_COMP] = NULL;
    m_floatComp[Y_COMP] = NULL;
    m_floatComp[U_COMP] = NULL;
    m_floatComp[V_COMP] = NULL;
  }
  else {
    m_ui16Comp[Y_COMP] = NULL;
    m_ui16Comp[U_COMP] = NULL;
    m_ui16Comp[V_COMP] = NULL;
    m_data.resize((unsigned int) m_size);
    m_comp[Y_COMP] = &m_data[0];
    m_comp[U_COMP] = m_comp[Y_COMP] + m_compSize[Y_COMP];
    m_comp[V_COMP] = m_comp[U_COMP] + m_compSize[U_COMP];
    m_floatComp[Y_COMP] = NULL;
    m_floatComp[U_COMP] = NULL;
    m_floatComp[V_COMP] = NULL;
  }  
}

InputY4M::~InputY4M() {
  m_iBuf = NULL;
  m_buf = NULL;

  m_comp[Y_COMP] = NULL;
  m_comp[U_COMP] = NULL;
  m_comp[V_COMP] = NULL;
  
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
int InputY4M::parseChromaSubsampling (FrameFormat *format, const char *taggedField) {
  if (strncmp(taggedField, "420jpeg", 7) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ONE;
    format->m_chromaLocation[FP_BOTTOM] = CL_ONE;
    format->m_chromaFormat              = CF_420;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  if (strncmp(taggedField, "420mpeg2", 7) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_420;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  if (strncmp(taggedField, "420p10", 6) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_420;
    format->m_bitDepthComp  [Y_COMP]    = 10;
    format->m_bitDepthComp  [U_COMP]    = 10;
    format->m_bitDepthComp  [V_COMP]    = 10;
    return 1;
  }
  if (strncmp(taggedField, "420p12", 6) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_420;
    format->m_bitDepthComp  [Y_COMP]    = 12;
    format->m_bitDepthComp  [U_COMP]    = 12;
    format->m_bitDepthComp  [V_COMP]    = 12;
    return 1;
  }
  if (strncmp(taggedField, "420p14", 6) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_420;
    format->m_bitDepthComp  [Y_COMP]    = 14;
    format->m_bitDepthComp  [U_COMP]    = 14;
    format->m_bitDepthComp  [V_COMP]    = 14;
    return 1;
  }
  if (strncmp(taggedField, "422p10", 6) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_422;
    format->m_bitDepthComp  [Y_COMP]    = 10;
    format->m_bitDepthComp  [U_COMP]    = 10;
    format->m_bitDepthComp  [V_COMP]    = 10;
    return 1;
  }
  if (strncmp(taggedField, "422p12", 6) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_422;
    format->m_bitDepthComp  [Y_COMP]    = 12;
    format->m_bitDepthComp  [U_COMP]    = 12;
    format->m_bitDepthComp  [V_COMP]    = 12;
    return 1;
  }
  if (strncmp(taggedField, "422p14", 6) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_422;
    format->m_bitDepthComp  [Y_COMP]    = 14;
    format->m_bitDepthComp  [U_COMP]    = 14;
    format->m_bitDepthComp  [V_COMP]    = 14;
    return 1;
  }
  if (strncmp(taggedField, "444p10", 6) == 0) {
    format->m_chromaFormat              = CF_444;
    format->m_bitDepthComp  [Y_COMP]    = 10;
    format->m_bitDepthComp  [U_COMP]    = 10;
    format->m_bitDepthComp  [V_COMP]    = 10;
    return 1;
  }
  if (strncmp(taggedField, "444p12", 6) == 0) {
    format->m_chromaFormat              = CF_444;
    format->m_bitDepthComp  [Y_COMP]    = 12;
    format->m_bitDepthComp  [U_COMP]    = 12;
    format->m_bitDepthComp  [V_COMP]    = 12;
    return 1;
  }
  if (strncmp(taggedField, "444p14", 6) == 0) {
    format->m_chromaFormat              = CF_444;
    format->m_bitDepthComp  [Y_COMP]    = 14;
    format->m_bitDepthComp  [U_COMP]    = 14;
    format->m_bitDepthComp  [V_COMP]    = 14;
    return 1;
  }
  if (strncmp(taggedField, "444p16", 6) == 0) {
    format->m_chromaFormat              = CF_444;
    format->m_bitDepthComp  [Y_COMP]    = 16;
    format->m_bitDepthComp  [U_COMP]    = 16;
    format->m_bitDepthComp  [V_COMP]    = 16;
    return 1;
  }
  if (strncmp(taggedField, "420paldv", 8) == 0) {
    printf("format 444 with alpha (not supported)\n");
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_420;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  if (strncmp(taggedField, "444alpha", 8) == 0) {
    printf("format 444 with alpha (not supported)\n");
    format->m_chromaFormat              = CF_444;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  if (strncmp(taggedField, "411", 3) == 0) {
    printf("format 411 cosited (not supported)\n");
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_420;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  if (strncmp(taggedField, "422", 3) == 0) {
    format->m_chromaLocation[FP_TOP]    = CL_ZERO;
    format->m_chromaLocation[FP_BOTTOM] = CL_ZERO;
    format->m_chromaFormat              = CF_422;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  if (strncmp(taggedField, "444", 3) == 0) {
    format->m_chromaFormat              = CF_444;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  if (strncmp(taggedField, "mono", 4) == 0) {
    format->m_chromaFormat              = CF_400;
    format->m_bitDepthComp  [Y_COMP]    = 8;
    format->m_bitDepthComp  [U_COMP]    = 8;
    format->m_bitDepthComp  [V_COMP]    = 8;
    return 1;
  }
  return 0;
}

int InputY4M::parseInterlaceSpec (FrameFormat *format, const char *taggedField) {
  if (strncmp(taggedField, "?", 1) == 0) {
    format->m_isInterlaced = FALSE;
    return 1;
  }
  if (strncmp(taggedField, "p", 1) == 0) {
    format->m_isInterlaced = FALSE;
    return 1;
  }
  if (strncmp(taggedField, "t", 1) == 0) {
    format->m_isInterlaced = TRUE;
    return 1;
  }
  if (strncmp(taggedField, "b", 1) == 0) {
    format->m_isInterlaced = TRUE;
    return 1;
  }
  if (strncmp(taggedField, "m", 1) == 0) {
    format->m_isInterlaced = TRUE;
    return 1;
  }
  return 0;
}

void InputY4M::parseRatio (int *value0, int *value1,  char *taggedField) {
  char *p1 = taggedField, *p2 = taggedField;  
  
  *value0 = 0;
  *value1 = 0;
  
  if (p1 != NULL && p2 != NULL)  {
    p1 = strstr( p1, ":");
    
    if (p1 == NULL) {
      // invalid
    }
    int position = (int) (p1 - p2);
    taggedField[position] = '\0';
    *value0 = atoi(taggedField);
    *value1 = atoi(&p1[1]);
    
  }
}

int InputY4M::readStreamHeader (IOVideo *inputFile,  FrameFormat *format) {
  int  vfile = inputFile->m_fileNum;
  int  cCount = 0;
  int  nCount = 0;
  bool foundEOL = FALSE;
  while (1) {
    if (foundEOL == TRUE) {
      // end of reading
      break;
    }

    cCount = 0;
    nCount ++;
    // read attribute name
    while (1) {
      if (mm_read(vfile, &m_streamHeader[cCount], sizeof(char)) != sizeof(char)) {
        printf ("InputY4M::readStreamHeader: Unexpected end of file reached. Cannot read further!\n");
        return 0;
      }
      else {
        cCount++;
        if (cCount > m_streamHeaderSize) {
          m_streamHeaderSize += 256;
          m_streamHeader.resize(m_streamHeaderSize);
        }

        if (m_streamHeader[cCount - 1] == '\n') {
        // end of reading
          inputFile->m_fileHeader = (int) tell((int) vfile);
          foundEOL = TRUE;
          break;
        }

        if (m_streamHeader[cCount - 1] == ' ') {
          break;
        }        
      }
    }
    
    if (cCount == 1) {
      break;
    }
    if (nCount == 1 && strncmp(&m_streamHeader[0], "YUV4MPEG2", 9) != 0) {
      printf("Magic String not found in stream header. Invalid file.\n");
      return 0;
    }
        
    // Tagged field
    // Width
    if (strncmp(&m_streamHeader[0], "W", 1) == 0) {
      format->m_width[Y_COMP] = atoi(&m_streamHeader[1]);
      continue;
    }
    // Height
    if (strncmp(&m_streamHeader[0], "H", 1) == 0) {
      format->m_height[Y_COMP] = atoi(&m_streamHeader[1]);
      continue;
    }
    // Chroma Subsampling
    if (strncmp(&m_streamHeader[0], "C", 1) == 0) {
      parseChromaSubsampling(format, &m_streamHeader[1]);
      continue;
    }
    // Frame Rate
    if (strncmp(&m_streamHeader[0], "F", 1) == 0) {
      int fps0, fps1;
      parseRatio(&fps0, &fps1, &m_streamHeader[1]);
      format->m_frameRate = (float) (double(fps0) / double(fps1));
      continue;
    }
    // Aspect Ratio
    if (strncmp(&m_streamHeader[0], "A", 1) == 0) {
      int asp0, asp1;
      parseRatio(&asp0, &asp1, &m_streamHeader[1]);
      //printf("ratio %d:%d\n", asp0, asp1);
      continue;
    }
    // Interlace specification
    if (strncmp(&m_streamHeader[0], "I", 1) == 0) {
      parseInterlaceSpec(format, &m_streamHeader[1]);
      continue;
    }

    // Metadata
    if (strncmp(&m_streamHeader[0], "X", 1) == 0) {
      continue;
    }
  }
  
  return 1;
}

int InputY4M::readFrameHeader (int vfile,  FrameFormat *format) {
  int  cCount = 0;
  int  nCount = 0;
  bool foundEOL = FALSE;
  while (1) {
    if (foundEOL == TRUE) {
      // end of reading
      break;
    }
    
    cCount = 0;
    nCount ++;
    // read attribute name
    while (1) {
      if (mm_read(vfile, &m_streamHeader[cCount], sizeof(char)) != sizeof(char)) {
        printf ("InputY4M::readFrameHeader: Unexpected end of file reached. Cannot read further!\n");
        return 0;
      }
      else {
        cCount++;
        if (cCount > m_streamHeaderSize) {
          m_streamHeaderSize += 256;
          m_streamHeader.resize(m_streamHeaderSize);
        }
        
        if (m_streamHeader[cCount - 1] == '\n') {
          // end of reading
          
          foundEOL = TRUE;
          break;
        }
        
        if (m_streamHeader[cCount - 1] == ' ') {
          break;
        }        
      }
    }
    
    if (cCount == 1) {
      break;
    }
    if (nCount == 1 && strncmp(&m_streamHeader[0], "FRAME", 5) != 0) {
      printf("Magic String not found in frame header. Invalid file.\n");
      return 0;
    }
    
    // Tagged field
    // Interlace specification
    if (strncmp(&m_streamHeader[0], "I", 1) == 0) {
      // currently ignored
      continue;
    }
    
    // Metadata
    if (strncmp(&m_streamHeader[0], "X", 1) == 0) {
      continue;
    }
  }
  
  return 1;
}

int InputY4M::readData (int vfile,  FrameFormat *source, uint8 *buf) {
  uint8 *curBuf = buf;
  int readSize = source->m_picUnitSizeShift3 * source->m_width[Y_COMP];
  int i, j;
  for (i = 0; i < source->m_height[Y_COMP]; i++) {
    if (mm_read(vfile, curBuf, readSize) != readSize) {
      //printf ("readData: cannot read %d bytes from input file, unexpected EOF!\n", source->m_width[Y_COMP]);
      return 0;
    }
    curBuf += readSize;
  }
  
  if (source->m_chromaFormat != CF_400) {
    for (j = U_COMP; j <= V_COMP; j++) {
      readSize = source->m_picUnitSizeShift3 * source->m_width[ (int) j];
      for (i = 0; i < source->m_height[(int) j]; i++) {
        if (mm_read(vfile, curBuf, readSize) != readSize) {
          printf ("readData: cannot read %d bytes from input file, unexpected EOF!\n", source->m_width[1]);
          return 0;
        }
        curBuf += readSize;
      }
    }
  }
  return 1;
}

int InputY4M::readData (int vfile, int framesizeInBytes, uint8 *buf)
{
  if (mm_read(vfile, buf, (int) framesizeInBytes) != (int) framesizeInBytes)  {
    printf ("read_one_frame: cannot read %d bytes from input file, unexpected EOF!\n", (int) framesizeInBytes);
    return 0;
  }
  else  {
    return 1;
  }
}

int64 InputY4M::getFrameSizeInBytes(FrameFormat *source, bool isInterleaved)
{
  uint32 symbolSizeInBytes = source->m_picUnitSizeShift3;
  int64 framesizeInBytes;
  
  const int bytesY  = m_compSize[Y_COMP];
  const int bytesUV = m_compSize[U_COMP];
  
  if (isInterleaved == FALSE) {
    framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
  }
  else {
    switch (source->m_chromaFormat) {
      case CF_420:
        framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
        break;
      case CF_422:
        switch (source->m_pixelFormat) {
          case PF_YUYV:
          case PF_YUY2:
          case PF_YVYU:
          case PF_UYVY:
            framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
            break;
          case PF_V210:
          case PF_UYVY10:
            // Pack 12 10-bit samples into each 16 bytes
            //framesizeInBytes = ((bytesY + 2 * bytesUV) / 3) << 2;
            framesizeInBytes = (bytesY / 3) << 3;
            break;
#ifdef __SIM2_SUPPORT_ENABLED__
          case PF_SIM2:
            framesizeInBytes = (bytesY * 3);
            break;
#endif
          default:
            fprintf(stderr, "Unsupported pixel format.\n");
            exit(EXIT_FAILURE);
            break;
        }
        break;
      case CF_444:
        if(source->m_pixelFormat== PF_V410 || source->m_pixelFormat== PF_R210 || source->m_pixelFormat== PF_R10K) {
          // Pack 3 10-bit samples into a 32 bit little-endian word
          framesizeInBytes = bytesY * 4;
        }
        else if(source->m_pixelFormat== PF_B64A) {
          framesizeInBytes = bytesY * 4;
        }
        else
          framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
        break;
      default:
        fprintf(stderr, "Unknown Chroma Format type %d\n", source->m_chromaFormat);
        exit(EXIT_FAILURE);
        break;
    }
  }
  
  return framesizeInBytes;
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
 * \param f_header
 *    Number of bytes in the source file to be skipped
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int InputY4M::readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileRead = 0;
  int vfile = inputFile->m_fileNum;
  FrameFormat *format = &inputFile->m_format;
  uint32 symbolSizeInBytes = format->m_picUnitSizeShift3;

  const int64 framesizeInBytes = getFrameSizeInBytes(format, inputFile->m_isInterleaved);
  bool isBytePacked = (bool) (inputFile->m_isInterleaved && (format->m_pixelFormat == PF_V210 || format->m_pixelFormat == PF_UYVY10)) ? TRUE : FALSE;

  int frameHeader = 6;

  // Let us seek directly to the current frame
  if (lseek (vfile, fileHeader + (framesizeInBytes + frameHeader) * (frameNumber + frameSkip), SEEK_SET) == -1)  {
    fprintf(stderr, "readOneFrame: cannot lseek to (Header size) in input file\n");
    exit(EXIT_FAILURE);
  }
  
  readFrameHeader (vfile,  format);
  // Here we are at the correct position for the source frame in the file.  
  // Now read it.

  if ((format->m_picUnitSizeOnDisk & 0x07) == 0)  {
    if (isBytePacked == TRUE)
      fileRead = readData (vfile, (int) framesizeInBytes, m_buf);
    else
      fileRead = readData (vfile, format, m_buf);
  
    // If format is interleaved, then perform deinterleaving
    if (m_isInterleaved)
      deInterleave ( &m_buf, &m_iBuf, format, symbolSizeInBytes);

    if (m_bitDepthComp[Y_COMP] == 8)
      imageReformat ( m_buf, &m_data[0], format, symbolSizeInBytes );
    else {
      imageReformatUInt16 ( m_buf, format, symbolSizeInBytes );
    }
  }
  else {
    fprintf (stderr, "readOneFrame (NOT IMPLEMENTED): pic unit size on disk must be divided by 8");
    exit(EXIT_FAILURE);
  }

  return fileRead;
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

