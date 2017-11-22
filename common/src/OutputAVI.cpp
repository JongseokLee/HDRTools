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
 * \file OutputAVI.cpp
 *
 * \brief
 *    OutputAVI class C++ file for allowing output of concatenated YUV files
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
#include <iostream>
#include <cstring>
#include <sstream>      // std::ostringstream

#include "OutputAVI.H"
#include "Global.H"
#include "IOFunctions.H"

//#define INFO_LIST
//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------
#define PAD_EVEN(x) ( ((x)+1) & ~1 )

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

OutputAVI::OutputAVI(IOVideo *outputFile, FrameFormat *format) {
  
  m_headerBytes          = (int) HEADERBYTES;
  m_outputFile           = outputFile;
  m_isFloat              = FALSE;
  format->m_isFloat      = m_isFloat;
  m_colorSpace           = format->m_colorSpace;
  m_colorPrimaries       = format->m_colorPrimaries;
  m_transferFunction     = format->m_transferFunction;
  m_systemGamma          = format->m_systemGamma;
  m_sampleRange          = format->m_sampleRange;
  m_pixelFormat          = format->m_pixelFormat;

  m_chromaFormat         = format->m_chromaFormat;
  m_isInterleaved        = outputFile->m_isInterleaved;
  m_isInterlaced         = format->m_isInterlaced;
  
  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];    
  }

  m_frameRate            = format->m_frameRate;
  m_picUnitSizeOnDisk    = format->m_picUnitSizeOnDisk;
  m_picUnitSizeShift3    = format->m_picUnitSizeShift3;


  m_bitDepthComp[Y_COMP] = format->m_bitDepthComp[Y_COMP];
  m_bitDepthComp[U_COMP] = format->m_bitDepthComp[U_COMP];
  m_bitDepthComp[V_COMP] = format->m_bitDepthComp[V_COMP];

  m_height[Y_COMP]       = format->m_height[Y_COMP];
  m_width [Y_COMP]       = format->m_width [Y_COMP];
  
  // Sim2 params
  m_cositedSampling       = format->m_cositedSampling;
  m_improvedFilter        = format->m_improvedFilter;

  if (format->m_chromaFormat == CF_420) {
    format->m_height[V_COMP] = format->m_height[U_COMP] = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP] >> 1;
    format->m_width [V_COMP] = format->m_width [U_COMP] = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP] >> 1;
  }
  else if (format->m_chromaFormat == CF_422) {
    format->m_height[V_COMP] = format->m_height[U_COMP] = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
    format->m_width [V_COMP] = format->m_width [U_COMP] = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP] >> 1;
  }
  else {
    format->m_height[V_COMP] = format->m_height[U_COMP] = m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
    format->m_width [V_COMP] = format->m_width [U_COMP] = m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];
  }

  format->m_compSize[Y_COMP] = m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  format->m_compSize[U_COMP] = m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  format->m_compSize[V_COMP] = m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];

  m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP];

  m_iBuffer.resize(3 * (unsigned int) m_size * m_picUnitSizeShift3);
  m_buffer.resize (3 * (unsigned int) m_size * m_picUnitSizeShift3);
  
  m_iBuf = &m_iBuffer[0];
  m_buf  = &m_buffer[0];

  if (m_picUnitSizeShift3 > 1) {
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
  m_format = *format;
  outputFile->m_format = *format;

  m_errorNumber = 0;
  m_avi = new AVIFile;
  
  if (m_avi == NULL) {
    m_errorNumber = AVI_ERR_NO_MEM;
    return;
  }
  
  if(format->m_pixelFormat == PF_R210 || format->m_pixelFormat == PF_R10K || format->m_pixelFormat == PF_V410)
    m_headerBytes = (1 + format->m_width[Y_COMP]) * 4;
  else if (format->m_pixelFormat == PF_V210)
    m_headerBytes = 4 + ((format->m_width[Y_COMP]) / 3) * 8;
  else 
    m_headerBytes = 2048;

  
  m_header = new byte[m_headerBytes];
  memset(m_header,0, m_headerBytes * sizeof(byte));
  
  m_avi->m_fileNum = outputFile->m_fileNum;
  
  if (m_avi->m_fileNum < 0) {
    m_errorNumber = AVI_ERR_OPEN;
    if (m_avi != NULL) {
      delete m_avi;
      m_avi = NULL;
    }
    if (m_header != NULL) {
      delete m_header;
      m_header = NULL;
    }
    return;
  }

  if (aviWrite(m_avi->m_fileNum, (char *) m_header, m_headerBytes) != m_headerBytes) {

    IOFunctions::closeFile(&m_avi->m_fileNum);
    m_errorNumber = AVI_ERR_WRITE;
    if (m_avi != NULL) {
      delete m_avi;
      m_avi = NULL;
    }
    if (m_header != NULL) {
      delete m_header;
      m_header = NULL;
    }
    return;
  }
  
  
  if (format->m_colorSpace == CM_RGB) {
    if (format->m_bitDepthComp[Y_COMP] == 10) {
      if(format->m_pixelFormat == PF_R210) // assumes m_pixelFormat PF_R210
        strcpy( m_avi->m_compressor, "r210");
      else // r10k
        strcpy( m_avi->m_compressor, "r10k");
    }
    else
    strcpy( m_avi->m_compressor, "\0\0\0\0");
  }
#ifdef __SIM2_SUPPORT_ENABLED__  
  else if (format->m_pixelFormat == PF_SIM2) {
    strcpy( m_avi->m_compressor, "\0\0\0\0");
  }
#endif
  else {
    if (format->m_colorSpace == CM_YCbCr) {
      switch (format->m_chromaFormat) {
        case CF_420:
          strcpy( m_avi->m_compressor, "I420");
          break;
        case CF_422:
          if (format->m_bitDepthComp[Y_COMP] == 10)
            strcpy( m_avi->m_compressor, "v210");
          else if (format->m_pixelFormat == PF_UYVY)
            strcpy( m_avi->m_compressor, "UYVY");
          else if (format->m_pixelFormat == PF_YUY2)
            strcpy( m_avi->m_compressor, "YUY2");
          else if (format->m_pixelFormat == PF_YVYU)
            strcpy( m_avi->m_compressor, "YVYU");
          else if (format->m_pixelFormat == PF_UYVY)
            strcpy( m_avi->m_compressor, "UYVY");
          else
            strcpy( m_avi->m_compressor, "I422");
          break;
        default:
        case CF_444:
          if (format->m_bitDepthComp[Y_COMP] == 10 && format->m_pixelFormat == PF_V410) // assumes m_pixelFormat PF_V410
            strcpy( m_avi->m_compressor, "v410");
          else
            strcpy( m_avi->m_compressor, "\0\0\0\0");
          break;
      }
    }
  }
  

  m_avi->m_pos  = m_headerBytes;
  m_avi->m_mode = AVI_MODE_WRITE;
  
  m_avi->m_numberAudioTracks   = 0;
  m_avi->m_currentAudioTrack   = 0;
  
  m_avi->m_width     = format->m_width[Y_COMP];
  m_avi->m_height    = format->m_height[Y_COMP];
  m_avi->m_frameRate = format->m_frameRate;
  
  // 0 byte
  m_avi->m_compressor[4] = 0;
  
  updateHeader(format);
}

OutputAVI::~OutputAVI() {
  
  close(&(m_outputFile->m_format));
  
  m_outputFile->m_fileNum = -1;

  if (m_avi != NULL) {
    delete m_avi;
    m_avi = NULL;
  }

  if (m_header) {
    delete [] m_header;
    m_header = NULL;
  }
  
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
void OutputAVI::out4CC(char *dest, const char *source, int64 *length) {
  if(*length <= m_headerBytes - 4)
    memcpy(dest + *length, source, 4);
  *length += 4;
}

void OutputAVI::outLong(char *dest, int32 value, int64 *length) {
  int32ToChar(dest + *length, value);
  *length += 4;
  
  /*std::stringstream strs;
  strs << value;
  std::string tempString = strs.str();
  
  out4CC(dest, tempString.c_str(), length);
   */
}


void OutputAVI::outShort(char *dest, int32 value, int64 *length) {
  if(*length <= m_headerBytes - 2) {
    dest[ *length  ] = (value   ) & 0xff;
    dest[ *length+1] = (value>>8) & 0xff;
  }
  *length += 2;
}

void OutputAVI::outChar(char *dest, int32 value, int64 *length) {
  if(*length <= m_headerBytes - 1) {
    dest[*length  ] = (value)&0xff;
  }
  *length += 1;
}

void OutputAVI::outMemory(char *dest, void *source, int64 *length, uint32 size) {
  if(*length <= m_headerBytes - size)
    memcpy(dest + *length, source, size);
  *length += size;
}

void OutputAVI::int32ToChar(char *dest, int32 value) {
  dest[0] = (value      ) & 0xff;
  dest[1] = (value >>  8) & 0xff;
  dest[2] = (value >> 16) & 0xff;
  dest[3] = (value >> 24) & 0xff;
}

/* Calculate audio sample size from number of bits and number of channels.
 This may have to be adjusted for eg. 12 bits and stereo */

int OutputAVI::sampleSize(int j)
{
  int s;
  s = ((m_avi->m_track[j].a_bits + 7) / 8) * m_avi->m_track[j].a_chans;
  if(s<4)
    s=4; /* avoid possible zero divisions */
  return s;
}


int OutputAVI::updateHeader(FrameFormat *format)
{
  int njunk, sampsize, ms_per_frame, frate, flag;
  int hdrl_start, strl_start, j;
  unsigned char *header;
  int64 nhb = 0;
  unsigned long xd_size, xd_size_align2;
  
  header = new unsigned char[m_headerBytes];
  //assume max size
  int movi_len = AVI_MAX_LEN - m_headerBytes + 4;
  
  //assume index will be written
  bool hasIndex = TRUE;
  
  if(m_avi->m_frameRate < 0.001) {
    frate=0;
    ms_per_frame=0;
  }
  else {
    frate = (int) (FRAME_RATE_SCALE * m_avi->m_frameRate + 0.5);
    ms_per_frame=(int) (1000000/m_avi->m_frameRate + 0.5);
  }
  
  /* Prepare the file header */
  
  /* The RIFF header */
  
  out4CC ((char *) header, "RIFF", &nhb);
  outLong((char *) header, movi_len, &nhb);    // assume max size
  out4CC ((char *) header, "AVI ", &nhb);
  
  /* Start the header list */
  
  out4CC ((char *) header, "LIST", &nhb);
  outLong((char *) header, 0, &nhb);        /* Length of list in bytes, don't know yet */
  hdrl_start = (int) nhb;  /* Store start position */
  out4CC ((char *) header, "hdrl", &nhb);
  
  /* The main AVI header */
  
  out4CC ((char *) header, "avih", &nhb);
  outLong((char *) header, 56, &nhb);                 /* # of bytes to follow */
  outLong((char *) header, ms_per_frame, &nhb);       /* Microseconds per frame */
  //ThOe ->0
  //   outLong((char *) header, 10000000, &nhb);           /* MaxBytesPerSec, I hope this will never be used */
  outLong((char *) header, 0, &nhb);
  outLong((char *) header, 0, &nhb);                  /* PaddingGranularity (whatever that might be) */

  /* Other sources call it 'reserved' */
  flag = AVIF_ISINTERLEAVED;
  if(hasIndex == TRUE)
    flag |= AVIF_HASINDEX;
  if((hasIndex == TRUE) && m_avi->m_mustUseIndex)
    flag |= AVIF_MUSTUSEINDEX;
  outLong((char *) header, flag, &nhb);               /* Flags */
  outLong((char *) header, 0, &nhb);                  // no frames yet
  outLong((char *) header, 0, &nhb);                  /* InitialFrames */
  
  outLong((char *) header, m_avi->m_numberAudioTracks+1, &nhb);
  
  outLong((char *) header, 0, &nhb);                  /* SuggestedBufferSize */
  outLong((char *) header, m_avi->m_width, &nhb);         /* Width */
  outLong((char *) header, m_avi->m_height, &nhb);        /* Height */
  /* MS calls the following 'reserved': */
  outLong((char *) header, 0, &nhb);                  /* TimeScale:  Unit used to measure time */
  outLong((char *) header, 0, &nhb);                  /* DataRate:   Data rate of playback     */
  outLong((char *) header, 0, &nhb);                  /* StartTime:  Starting time of AVI data */
  outLong((char *) header, 0, &nhb);                  /* DataLength: Size of AVI data chunk    */
  
  
  /* Start the video stream list ---------------------------------- */
  
  out4CC ((char *) header, "LIST", &nhb);
  outLong((char *) header, 0, &nhb);        /* Length of list in bytes, don't know yet */
  strl_start = (int) nhb;  /* Store start position */
  out4CC ((char *) header, "strl", &nhb);
  
  /* The video stream header */
  
  out4CC ((char *) header, "strh", &nhb);
  outLong((char *) header, 56, &nhb);                 /* # of bytes to follow */
  out4CC ((char *) header, "vids", &nhb);             /* Type */
  out4CC ((char *) header, m_avi->m_compressor, &nhb);    /* Handler */
  outLong((char *) header, 0, &nhb);                  /* Flags */
  outLong((char *) header, 0, &nhb);                  /* Reserved, MS says: wPriority, wLanguage */
  outLong((char *) header, 0, &nhb);                  /* InitialFrames */
  outLong((char *) header, FRAME_RATE_SCALE, &nhb);   /* Scale */
  outLong((char *) header, frate, &nhb);              /* Rate: Rate/Scale == samples/second */
  outLong((char *) header, 0, &nhb);                  /* Start */
  outLong((char *) header, 0, &nhb);                  // no frames yet
  outLong((char *) header, 0, &nhb);                  /* SuggestedBufferSize */
  outLong((char *) header, -1, &nhb);                 /* Quality */
  outLong((char *) header, 0, &nhb);                  /* SampleSize */
  outLong((char *) header, 0, &nhb);                  /* Frame */
  outLong((char *) header, 0, &nhb);                  /* Frame */
  
  /* The video stream format */
  
  xd_size        = m_avi->m_extraDataSize;
  xd_size_align2 = (m_avi->m_extraDataSize+1) & ~1;
  
  out4CC  ((char *) header, "strf", &nhb);
  outLong ((char *) header, 40 + xd_size_align2, &nhb);/* # of bytes to follow */
  outLong ((char *) header, 40 + xd_size, &nhb);	/* Size */
  outLong ((char *) header, m_avi->m_width, &nhb);         /* Width */
  outLong ((char *) header, m_avi->m_height, &nhb);        /* Height */
  outShort((char *) header,  1, &nhb);
  if (format->m_pixelFormat == PF_V410 || format->m_pixelFormat == PF_R210 || format->m_pixelFormat == PF_R10K) {
    outShort((char *) header, 30, &nhb);     /* Planes, Count */
  }
  else
    outShort((char *) header, 24, &nhb);     /* Planes, Count */
  out4CC  ((char *) header, m_avi->m_compressor, &nhb);    /* Compression */

  // ThOe (*3)
  int sizeImage = m_avi->m_width * m_avi->m_height * 3;
  if (format->m_pixelFormat == PF_V410 || format->m_pixelFormat == PF_R210 || format->m_pixelFormat == PF_R10K)
    sizeImage = m_avi->m_width * m_avi->m_height * 4;
  else if (format->m_pixelFormat == PF_V210)
    sizeImage = (m_avi->m_width * m_avi->m_height / 3) << 3;
  outLong ((char *) header, sizeImage * 10, &nhb);  /* SizeImage (in bytes?) */
  outLong ((char *) header, 0, &nhb);                  /* XPelsPerMeter */
  outLong((char *) header, 0, &nhb);                  /* YPelsPerMeter */
  outLong((char *) header, 0, &nhb);                  /* ClrUsed: Number of colors used */
  outLong((char *) header, 0, &nhb);                  /* ClrImportant: Number of colors important */
  
  // write extradata
  if (xd_size > 0 && m_avi->m_extraData) {
    outMemory((char *) header, m_avi->m_extraData, &nhb, xd_size);
    if (xd_size != xd_size_align2) {
      outChar((char *) header, 0, &nhb);
    }
  }
  
  /* Finish stream list, i.e. put number of bytes in the list to proper pos */
  
  int32ToChar((char *) header+strl_start-4,(int) (nhb-strl_start));
  
  
  /* Start the audio stream list ---------------------------------- */
  
  for(j=0; j<m_avi->m_numberAudioTracks; ++j) {
    
    sampsize = sampleSize(j);
    
    out4CC ((char *) header, "LIST", &nhb);
    outLong((char *) header, 0, &nhb);        /* Length of list in bytes, don't know yet */
    strl_start = (int) nhb;  /* Store start position */
    out4CC ((char *) header, "strl", &nhb);
    
    /* The audio stream header */
    
    out4CC ((char *) header, "strh", &nhb);
    outLong((char *) header, 56, &nhb);            /* # of bytes to follow */
    out4CC ((char *) header, "auds", &nhb);
    
    // -----------
    // ThOe
    outLong((char *) header, 0, &nhb);             /* Format (Optionally) */
    // -----------
    
    outLong((char *) header, 0, &nhb);             /* Flags */
    outLong((char *) header, 0, &nhb);             /* Reserved, MS says: wPriority, wLanguage */
    outLong((char *) header, 0, &nhb);             /* InitialFrames */
    
    // ThOe /4
    outLong((char *) header, sampsize/4, &nhb);      /* Scale */
    outLong((char *) header, 1000*m_avi->m_track[j].mp3rate/8, &nhb);
    outLong((char *) header, 0, &nhb);             /* Start */
    outLong((char *) header, (int) (4*m_avi->m_track[j].audioBytes/sampsize), &nhb);   /* Length */
    outLong((char *) header, 0, &nhb);             /* SuggestedBufferSize */
    outLong((char *) header, -1, &nhb);            /* Quality */
    
    // ThOe /4
    outLong((char *) header, sampsize/4, &nhb);    /* SampleSize */
    
    outLong((char *) header, 0, &nhb);             /* Frame */
    outLong((char *) header, 0, &nhb);             /* Frame */
    
    /* The audio stream format */
    
    out4CC ((char *) header, "strf", &nhb);
    outLong((char *) header, 16, &nhb);                   /* # of bytes to follow */
    outShort((char *) header, m_avi->m_track[j].a_fmt, &nhb);           /* Format */
    outShort((char *) header, m_avi->m_track[j].a_chans, &nhb);         /* Number of channels */
    outLong((char *) header, m_avi->m_track[j].a_rate, &nhb);          /* SamplesPerSec */
    // ThOe
    outLong((char *) header, 1000*m_avi->m_track[j].mp3rate/8, &nhb);
    //ThOe (/4)
    
    outShort((char *) header, sampsize/4, &nhb);           /* BlockAlign */
    
    
    outShort((char *) header, m_avi->m_track[j].a_bits, &nhb);          /* BitsPerSample */
    
    /* Finish stream list, i.e. put number of bytes in the list to proper pos */
    
    int32ToChar((char *) header+strl_start-4,(int32) nhb-strl_start);
  }
  
  /* Finish header list */
  
  int32ToChar((char *) header+hdrl_start-4,(int) (nhb-hdrl_start));
  
  
  /* Calculate the needed amount of junk bytes, output junk */
  
  njunk = m_headerBytes - (int) nhb - 8 - 12;
  /* Safety first: if njunk <= 0, somebody has played with
   m_headerBytes without knowing what (s)he did.
   This is a fatal error */
  
  if(njunk<=0)
  {
    fprintf(stderr,"AVI_close_output_file: # of header bytes too small\n");
    exit(1);
  }
  
  out4CC ((char *) header, "JUNK", &nhb);
  outLong((char *) header, njunk, &nhb);
  memset(header+nhb, 0, njunk);
  
  nhb += njunk;
  
  /* Start the movi list */
  
  out4CC ((char *) header, "LIST", &nhb);
  outLong((char *) header, movi_len, &nhb); /* Length of list in bytes */
  out4CC ((char *) header, "movi", &nhb);
  
  /* Output the header, truncate the file to the number of bytes
   actually written, report an error if someting goes wrong */
  
  if ( lseek(m_avi->m_fileNum, 0, SEEK_SET)<0 ||
      mm_write(m_avi->m_fileNum,(char *)header, m_headerBytes)!=m_headerBytes ||
      lseek(m_avi->m_fileNum,m_avi->m_pos, SEEK_SET)<0)
  {
    m_errorNumber = AVI_ERR_CLOSE;
	delete [] header;
    return -1;
  }
  
  delete [] header;
  return 0;
}

int64 OutputAVI::aviWrite (int fd, char *buffer, int64 len)
{
  if (mm_write (fd, buffer, (off_t) len) != len) {
    printf ("aviWrite: cannot write %lld bytes to output file, unexpected error!\n", len);
    return 0;
  };
  
  return len;
}



int64 OutputAVI::getFrameSizeInBytes(FrameFormat *source, bool isInterleaved)
{
  uint32 symbolSizeInBytes = m_picUnitSizeShift3;
  int64 framesizeInBytes;
  
  const int bytesY  = m_compSize[Y_COMP];
  const int bytesUV = m_compSize[U_COMP];
  
  if (0 && isInterleaved == FALSE) {
    framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
  }
  else {
#ifdef __SIM2_SUPPORT_ENABLED__
    if (source->m_pixelFormat == PF_SIM2) {
      framesizeInBytes = (bytesY * 3);
    }
    else 
#endif
    {
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
            default:
              fprintf(stderr, "Unsupported pixel format.\n");
              exit(EXIT_FAILURE);
              break;
          }
          break;
        case CF_444:
          if (source->m_pixelFormat == PF_V410 || source->m_pixelFormat == PF_R210 || source->m_pixelFormat == PF_R10K) {
            // Pack 3 10-bit samples into a 32 bit little-endian word
            framesizeInBytes = bytesY * 4;
          }
          else {
            framesizeInBytes = (bytesY + 2 * bytesUV) * symbolSizeInBytes;
          }
          break;
        default:
          fprintf(stderr, "Unknown Chroma Format type %d\n", source->m_chromaFormat);
          exit(EXIT_FAILURE);
          break;
      }
    }
  }
  
  return framesizeInBytes;
}

/*
 Write the header of an AVI file and close it.
 returns 0 on success, -1 on write error.
 */

int OutputAVI::closeOutputFile(FrameFormat *format)
{
  
  int ret, njunk, sampsize, hasIndex, ms_per_frame, frate, idxerror, flag;
  unsigned long movi_len;
  int hdrl_start, strl_start, j;
  unsigned char *AVI_header = new unsigned char[m_headerBytes];
  int64 nhb = 0;
  unsigned long xd_size, xd_size_align2;
 
#ifdef INFO_LIST
  long info_len;
  long id_len, real_id_len;
  long info_start_pos;
  //   time_t calptr;
#endif
  
  /* Calculate length of movi list */
  // dump the rest of the index
  if (m_avi->m_isOpenDML) {
    int cur_std_idx = m_avi->m_videoSuperIndex->nEntriesInUse-1;
    int audtr;
    
#ifdef DEBUG_ODML
    printf("ODML dump the rest indices\n");
#endif
    ixnnEntry (m_avi->m_videoSuperIndex->stdindex[cur_std_idx],
                    &m_avi->m_videoSuperIndex->aIndex[cur_std_idx]);
    
    m_avi->m_videoSuperIndex->aIndex[cur_std_idx].dwDuration =
	   m_avi->m_videoSuperIndex->stdindex[cur_std_idx]->nEntriesInUse - 1;
    
    for (audtr = 0; audtr < m_avi->m_numberAudioTracks; audtr++) {
      if (!m_avi->m_track[audtr].m_audioSuperIndex) {
        // not initialized -> no index
        continue;
      }
      ixnnEntry (m_avi->m_track[audtr].m_audioSuperIndex->stdindex[cur_std_idx],
                      &m_avi->m_track[audtr].m_audioSuperIndex->aIndex[cur_std_idx]);
      m_avi->m_track[audtr].m_audioSuperIndex->aIndex[cur_std_idx].dwDuration =
      m_avi->m_track[audtr].m_audioSuperIndex->stdindex[cur_std_idx]->nEntriesInUse - 1;
      if (m_avi->m_track[audtr].a_fmt == 0x1) {
	       m_avi->m_track[audtr].m_audioSuperIndex->aIndex[cur_std_idx].dwDuration *=
        m_avi->m_track[audtr].a_bits*m_avi->m_track[audtr].a_rate*m_avi->m_track[audtr].a_chans/800;
      }
    }
    // The m_avi->m_videoSuperIndex->nEntriesInUse contains the offset
    m_avi->m_videoSuperIndex->stdindex[ cur_std_idx+1 ]->qwBaseOffset = m_avi->m_pos;
  }
  
  if (m_avi->m_isOpenDML) {
    // Correct!
    movi_len = (unsigned long) (m_avi->m_videoSuperIndex->stdindex[ 1 ]->qwBaseOffset - m_headerBytes + 4 - m_avi->m_noIndex * 16 - 8);
  } 
  else {
    movi_len = (unsigned long) (m_avi->m_pos - m_headerBytes + 4);
  }
  
  /* Try to ouput the index entries. This may fail e.g. if no space
   is left on device. We will report this as an error, but we still
   try to write the header correctly (so that the file still may be
   readable in the most cases */
  
  idxerror = 0;
  hasIndex = 1;
  if (!m_avi->m_isOpenDML) {
    //   fprintf(stderr, "pos=%lu, index_len=%ld             \n", m_avi->m_pos, m_avi->m_noIndex*16);
    ret = addChunk((unsigned char *)"idx1", (unsigned char *) m_avi->m_index[0].m_indexEntry, m_avi->m_noIndex * 16);
    hasIndex = (ret==0);
    //fprintf(stderr, "pos=%lu, index_len=%d\n", m_avi->m_pos, hasIndex);
    
    if(ret) {
      idxerror = 1;
      m_errorNumber = AVI_ERR_WRITE_INDEX;
    }
  }
  
  /* Calculate Microseconds per frame */
  
  if(m_avi->m_frameRate < 0.001) {
    frate=0;
    ms_per_frame=0;
  }
  else {
    frate = (int) (FRAME_RATE_SCALE*m_avi->m_frameRate + 0.5);
    ms_per_frame=(int) (1000000/m_avi->m_frameRate + 0.5);
  }
  
  
  /* The RIFF header */
  out4CC ((char *) AVI_header, "RIFF", &nhb);
  
  if (m_avi->m_isOpenDML) {
    outLong((char *) AVI_header, (int32) (m_avi->m_videoSuperIndex->stdindex[ 1 ]->qwBaseOffset - 8), &nhb);    /* # of bytes to follow */
  } else {
    outLong ((char *) AVI_header, (int32) (m_avi->m_pos - 8), &nhb);    /* # of bytes to follow */
  }
  
  out4CC ((char *) AVI_header,"AVI ", &nhb);
  
  /* Start the header list */
  
  out4CC ((char *) AVI_header,"LIST", &nhb);
  outLong ((char *) AVI_header,0, &nhb);        /* Length of list in bytes, don't know yet */
  hdrl_start = (int) nhb;  /* Store start position */
  out4CC ((char *) AVI_header,"hdrl", &nhb);
  
  /* The main AVI header */
  
  /* The Flags in AVI File header */
  
#define AVIF_HASINDEX           0x00000010      /* Index at end of file */
#define AVIF_MUSTUSEINDEX       0x00000020
#define AVIF_ISINTERLEAVED      0x00000100
#define AVIF_TRUSTCKTYPE        0x00000800      /* Use CKType to find key frames */
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVIF_COPYRIGHTED        0x00020000
  
  out4CC ((char *) AVI_header,"avih", &nhb);
  outLong ((char *) AVI_header,56, &nhb);                 /* # of bytes to follow */
  outLong ((char *) AVI_header,ms_per_frame, &nhb);       /* Microseconds per frame */
  //ThOe ->0
  //   outLong ((char *) AVI_header,10000000, &nhb);           /* MaxBytesPerSec, I hope this will never be used */
  outLong ((char *) AVI_header,0, &nhb);
  outLong ((char *) AVI_header,0, &nhb);                  /* PaddingGranularity (whatever that might be) */
  /* Other sources call it 'reserved' */
  flag = AVIF_ISINTERLEAVED;
  if(hasIndex) flag |= AVIF_HASINDEX;
  if(hasIndex && m_avi->m_mustUseIndex) flag |= AVIF_MUSTUSEINDEX;
  outLong ((char *) AVI_header,flag, &nhb);               /* Flags */
  outLong ((char *) AVI_header,m_avi->m_videoFrames, &nhb);  /* TotalFrames */
  outLong ((char *) AVI_header,0, &nhb);                  /* InitialFrames */
  
  outLong ((char *) AVI_header,m_avi->m_numberAudioTracks+1, &nhb);
  //   if (m_avi->m_track[0].audio_bytes)
  //      { outLong ((char *) AVI_header,2, &nhb); }           /* Streams */
  //   else
  //      { outLong ((char *) AVI_header,1, &nhb); }           /* Streams */
  
  outLong ((char *) AVI_header,0, &nhb);                  /* SuggestedBufferSize */
  outLong ((char *) AVI_header,m_avi->m_width, &nhb);         /* Width */
  outLong ((char *) AVI_header,m_avi->m_height, &nhb);        /* Height */
  /* MS calls the following 'reserved': */
  outLong ((char *) AVI_header,0, &nhb);                  /* TimeScale:  Unit used to measure time */
  outLong ((char *) AVI_header,0, &nhb);                  /* DataRate:   Data rate of playback     */
  outLong ((char *) AVI_header,0, &nhb);                  /* StartTime:  Starting time of AVI data */
  outLong ((char *) AVI_header,0, &nhb);                  /* DataLength: Size of AVI data chunk    */
  
  
  /* Start the video stream list ---------------------------------- */
  
  out4CC ((char *) AVI_header,"LIST", &nhb);
  outLong ((char *) AVI_header,0, &nhb);        /* Length of list in bytes, don't know yet */
  strl_start = (int) nhb;  /* Store start position */
  out4CC ((char *) AVI_header,"strl", &nhb);
  
  /* The video stream header */
  
  out4CC ((char *) AVI_header,"strh", &nhb);
  outLong ((char *) AVI_header,56, &nhb);                 /* # of bytes to follow */
  out4CC ((char *) AVI_header,"vids", &nhb);             /* Type */
  out4CC ((char *) AVI_header,m_avi->m_compressor, &nhb);    /* Handler */
  outLong ((char *) AVI_header,0, &nhb);                  /* Flags */
  outLong ((char *) AVI_header,0, &nhb);                  /* Reserved, MS says: wPriority, wLanguage */
  outLong ((char *) AVI_header,0, &nhb);                  /* InitialFrames */
  outLong ((char *) AVI_header,FRAME_RATE_SCALE, &nhb);   /* Scale */
  outLong ((char *) AVI_header,frate, &nhb);              /* Rate: Rate/Scale == samples/second */
  outLong ((char *) AVI_header,0, &nhb);                  /* Start */
  outLong ((char *) AVI_header,m_avi->m_videoFrames, &nhb);  /* Length */
  outLong ((char *) AVI_header,m_avi->m_maxLength, &nhb);       /* SuggestedBufferSize */
  outLong ((char *) AVI_header,0, &nhb);                  /* Quality */
  outLong ((char *) AVI_header,0, &nhb);                  /* SampleSize */
  outLong ((char *) AVI_header,0, &nhb);                  /* Frame */
  outLong ((char *) AVI_header,0, &nhb);                  /* Frame */
  //outLong ((char *) AVI_header,0, &nhb);                  /* Frame */
  //outLong ((char *) AVI_header,0, &nhb);                  /* Frame */
  
  /* The video stream format */
  xd_size        = m_avi->m_extraDataSize;
  xd_size_align2 = (m_avi->m_extraDataSize+1) & ~1;
  
  out4CC ((char *) AVI_header,"strf", &nhb);
  outLong ((char *) AVI_header,40 + xd_size_align2, &nhb);/* # of bytes to follow */
  outLong ((char *) AVI_header,40 + xd_size, &nhb);	/* Size */
  outLong ((char *) AVI_header,m_avi->m_width, &nhb);         /* Width */
  outLong ((char *) AVI_header,m_avi->m_height, &nhb);        /* Height */

  outShort ((char *) AVI_header,1, &nhb);
  if (format->m_pixelFormat == PF_V410 || format->m_pixelFormat == PF_R210 || format->m_pixelFormat == PF_R10K)
    outShort ((char *) AVI_header, 30, &nhb);     /* Planes, Count */
  else
    outShort ((char *) AVI_header,24, &nhb);     /* Planes, Count */
    
  out4CC ((char *) AVI_header,m_avi->m_compressor, &nhb);    /* Compression */
  // ThOe (*3)
  int sizeImage = m_avi->m_width * m_avi->m_height * 3;

  if (format->m_pixelFormat == PF_V410 || format->m_pixelFormat == PF_R210 || format->m_pixelFormat == PF_R10K)
    sizeImage = m_avi->m_width * m_avi->m_height * 4 ;
  else if (format->m_pixelFormat == PF_V210)
    sizeImage = (m_avi->m_width * m_avi->m_height / 3) << 3;
  
  outLong ((char *) AVI_header,sizeImage, &nhb);  /* SizeImage (in bytes?) */
  outLong ((char *) AVI_header,0, &nhb);                  /* XPelsPerMeter */
  outLong ((char *) AVI_header,0, &nhb);                  /* YPelsPerMeter */
  outLong ((char *) AVI_header,0, &nhb);                  /* ClrUsed: Number of colors used */
  outLong ((char *) AVI_header,0, &nhb);                  /* ClrImportant: Number of colors important */
  
  // write extradata if present
  if (xd_size > 0 && m_avi->m_extraData) {
    outMemory((char *) AVI_header, m_avi->m_extraData, &nhb, xd_size);
    if (xd_size != xd_size_align2) {
      outChar ((char *) AVI_header,0, &nhb);
    }
  }
  
  // dump index of indices for audio
  if (m_avi->m_isOpenDML) {
    
    int k;
    
    out4CC  ((char *) AVI_header,m_avi->m_videoSuperIndex->fcc, &nhb);
    outLong ((char *) AVI_header,2+1+1+4+4+3*4 + m_avi->m_videoSuperIndex->nEntriesInUse * (8+4+4), &nhb);
    outShort((char *) AVI_header,m_avi->m_videoSuperIndex->wLongsPerEntry, &nhb);
    outChar ((char *) AVI_header,m_avi->m_videoSuperIndex->bIndexSubType, &nhb);
    outChar ((char *) AVI_header,m_avi->m_videoSuperIndex->bIndexType, &nhb);
    outLong ((char *) AVI_header,m_avi->m_videoSuperIndex->nEntriesInUse, &nhb);
    out4CC  ((char *) AVI_header,m_avi->m_videoSuperIndex->dwChunkId, &nhb);
    outLong ((char *) AVI_header,0, &nhb);
    outLong ((char *) AVI_header,0, &nhb);
    outLong ((char *) AVI_header,0, &nhb);
    
    
    for (k = 0; k < (int) m_avi->m_videoSuperIndex->nEntriesInUse; k++) {
      uint32_t r = (m_avi->m_videoSuperIndex->aIndex[k].qwOffset >> 32) & 0xffffffff;
      uint32_t s = (m_avi->m_videoSuperIndex->aIndex[k].qwOffset) & 0xffffffff;
      
#ifdef DEBUG_ODML
      printf("VID NrEntries %d/%ld (%c%c%c%c) |0x%llX|%ld|%ld|\n",  k,
             (unsigned long)m_avi->m_videoSuperIndex->nEntriesInUse,
             m_avi->m_videoSuperIndex->dwChunkId[0],
             m_avi->m_videoSuperIndex->dwChunkId[1],
             m_avi->m_videoSuperIndex->dwChunkId[2],
             m_avi->m_videoSuperIndex->dwChunkId[3],
             (unsigned long long)m_avi->m_videoSuperIndex->aIndex[k].qwOffset,
             (unsigned long)m_avi->m_videoSuperIndex->aIndex[k].dwSize,
             (unsigned long)m_avi->m_videoSuperIndex->aIndex[k].dwDuration
             );
#endif
      
      outLong ((char *) AVI_header,s, &nhb);
      outLong ((char *) AVI_header,r, &nhb);
      outLong ((char *) AVI_header,m_avi->m_videoSuperIndex->aIndex[k].dwSize, &nhb);
      outLong ((char *) AVI_header,m_avi->m_videoSuperIndex->aIndex[k].dwDuration, &nhb);
    }
    
  }
  
  /* Finish stream list, i.e. put number of bytes in the list to proper pos */
  
  int32ToChar((char *) AVI_header + strl_start - 4, (int32) (nhb - strl_start));
  
  /* Start the audio stream list ---------------------------------- */
  
  for(j=0; j<m_avi->m_numberAudioTracks; ++j) {
    
    //if (m_avi->m_track[j].a_chans && m_avi->m_track[j].audio_bytes)
    {
      unsigned long nBlockAlign = 0;
      unsigned long avgbsec = 0;
      unsigned long scalerate = 0;
      
      sampsize = sampleSize(j);
      sampsize = m_avi->m_track[j].a_fmt==0x1?sampsize*4:sampsize;
      
      nBlockAlign = (m_avi->m_track[j].a_rate<32000)?576:1152;
      /*
       printf("XXX sampsize (%d) block (%ld) rate (%ld) audio_bytes (%ld) mp3rate(%ld,%ld)\n",
       sampsize, nBlockAlign, m_avi->m_track[j].a_rate,
       (long int)m_avi->m_track[j].audio_bytes,
       1000*m_avi->m_track[j].mp3rate/8, m_avi->m_track[j].mp3rate);
       */
      
      if (m_avi->m_track[j].a_fmt==0x1) {
        sampsize = (m_avi->m_track[j].a_chans<2)?sampsize/2:sampsize;
        avgbsec = m_avi->m_track[j].a_rate*sampsize/4;
        scalerate = m_avi->m_track[j].a_rate*sampsize/4;
      } else  {
        avgbsec = 1000*m_avi->m_track[j].mp3rate/8;
        scalerate = 1000*m_avi->m_track[j].mp3rate/8;
      }
      
      out4CC ((char *) AVI_header,"LIST", &nhb);
      outLong ((char *) AVI_header,0, &nhb);        /* Length of list in bytes, don't know yet */
      strl_start = (int) nhb;  /* Store start position */
      out4CC ((char *) AVI_header,"strl", &nhb);
      
      /* The audio stream header */
      
      out4CC ((char *) AVI_header,"strh", &nhb);
      outLong ((char *) AVI_header,56, &nhb);            /* # of bytes to follow */
      out4CC ((char *) AVI_header,"auds", &nhb);
      
      // -----------
      // ThOe
      outLong ((char *) AVI_header,0, &nhb);             /* Format (Optionally) */
      // -----------
      
      outLong ((char *) AVI_header,0, &nhb);             /* Flags */
      outLong ((char *) AVI_header,0, &nhb);             /* Reserved, MS says: wPriority, wLanguage */
      outLong ((char *) AVI_header,0, &nhb);             /* InitialFrames */
      
      // VBR
      if (m_avi->m_track[j].a_fmt == 0x55 && m_avi->m_track[j].a_vbr) {
        outLong ((char *) AVI_header,nBlockAlign, &nhb);                   /* Scale */
        outLong ((char *) AVI_header,m_avi->m_track[j].a_rate, &nhb);          /* Rate */
        outLong ((char *) AVI_header,0, &nhb);                             /* Start */
        outLong ((char *) AVI_header,m_avi->m_track[j].audioChunks, &nhb);    /* Length */
        outLong ((char *) AVI_header,0, &nhb);                      /* SuggestedBufferSize */
        outLong ((char *) AVI_header,0, &nhb);                             /* Quality */
        outLong ((char *) AVI_header,0, &nhb);                             /* SampleSize */
        outLong ((char *) AVI_header,0, &nhb);                             /* Frame */
        outLong ((char *) AVI_header,0, &nhb);                             /* Frame */
      }
      else {
        outLong ((char *) AVI_header,sampsize/4, &nhb);                    /* Scale */
        outLong ((char *) AVI_header,scalerate, &nhb);  /* Rate */
        outLong ((char *) AVI_header,0, &nhb);                             /* Start */
        outLong ((char *) AVI_header,(int32) (4*m_avi->m_track[j].audioBytes/sampsize), &nhb);   /* Length */
        outLong ((char *) AVI_header,0, &nhb);                             /* SuggestedBufferSize */
        outLong ((char *) AVI_header,0xffffffff, &nhb);                             /* Quality */
        outLong ((char *) AVI_header,sampsize/4, &nhb);                    /* SampleSize */
        outLong ((char *) AVI_header,0, &nhb);                             /* Frame */
        outLong ((char *) AVI_header,0, &nhb);                             /* Frame */
      }
      
      /* The audio stream format */
      
      out4CC ((char *) AVI_header,"strf", &nhb);
      
      if (m_avi->m_track[j].a_fmt == 0x55 && m_avi->m_track[j].a_vbr) {
        
        outLong ((char *) AVI_header,30, &nhb);                            /* # of bytes to follow */ // mplayer writes 28
        outShort ((char *) AVI_header,m_avi->m_track[j].a_fmt, &nhb);           /* Format */                  // 2
        outShort ((char *) AVI_header,m_avi->m_track[j].a_chans, &nhb);         /* Number of channels */      // 2
        outLong ((char *) AVI_header,m_avi->m_track[j].a_rate, &nhb);          /* SamplesPerSec */           // 4
                                                                              //ThOe/tibit
        outLong ((char *) AVI_header,1000*m_avi->m_track[j].mp3rate/8, &nhb);  /* maybe we should write an avg. */ // 4
        outShort ((char *) AVI_header,nBlockAlign, &nhb);                   /* BlockAlign */              // 2
        outShort ((char *) AVI_header,m_avi->m_track[j].a_bits, &nhb);          /* BitsPerSample */           // 2
        
        outShort ((char *) AVI_header,12, &nhb);                           /* cbSize */                   // 2
        outShort ((char *) AVI_header,1, &nhb);                            /* wID */                      // 2
        outLong ((char *) AVI_header,2, &nhb);                            /* fdwFlags */                 // 4
        outShort ((char *) AVI_header,nBlockAlign, &nhb);                  /* nBlockSize */               // 2
        outShort ((char *) AVI_header,1, &nhb);                            /* nFramesPerBlock */          // 2
        outShort ((char *) AVI_header,0, &nhb);                            /* nCodecDelay */              // 2
        
      }
      else if (m_avi->m_track[j].a_fmt == 0x55 && !m_avi->m_track[j].a_vbr) {
        
        outLong ((char *) AVI_header,30, &nhb);                            /* # of bytes to follow */
        outShort ((char *) AVI_header,m_avi->m_track[j].a_fmt, &nhb);           /* Format */
        outShort ((char *) AVI_header,m_avi->m_track[j].a_chans, &nhb);         /* Number of channels */
        outLong ((char *) AVI_header,m_avi->m_track[j].a_rate, &nhb);          /* SamplesPerSec */
        //ThOe/tibit
        outLong ((char *) AVI_header,1000*m_avi->m_track[j].mp3rate/8, &nhb);
        outShort ((char *) AVI_header,sampsize/4, &nhb);                    /* BlockAlign */
        outShort ((char *) AVI_header,m_avi->m_track[j].a_bits, &nhb);          /* BitsPerSample */
        
        outShort ((char *) AVI_header,12, &nhb);                           /* cbSize */
        outShort ((char *) AVI_header,1, &nhb);                            /* wID */
        outLong ((char *) AVI_header,2, &nhb);                            /* fdwFlags */
        outShort ((char *) AVI_header,nBlockAlign, &nhb);                  /* nBlockSize */
        outShort ((char *) AVI_header,1, &nhb);                            /* nFramesPerBlock */
        outShort ((char *) AVI_header,0, &nhb);                            /* nCodecDelay */
      }
      else {
        outLong ((char *) AVI_header,18, &nhb);                   /* # of bytes to follow */
        outShort ((char *) AVI_header,m_avi->m_track[j].a_fmt, &nhb);           /* Format */
        outShort ((char *) AVI_header,m_avi->m_track[j].a_chans, &nhb);         /* Number of channels */
        outLong ((char *) AVI_header,m_avi->m_track[j].a_rate, &nhb);          /* SamplesPerSec */
        //ThOe/tibit
        outLong ((char *) AVI_header,avgbsec, &nhb);  /* Avg bytes/sec */
        outShort ((char *) AVI_header,sampsize/4, &nhb);                    /* BlockAlign */
        outShort ((char *) AVI_header,m_avi->m_track[j].a_bits, &nhb);          /* BitsPerSample */
        outShort ((char *) AVI_header,0, &nhb);                           /* cbSize */
      }
    }
    if (m_avi->m_isOpenDML) {
      
      int k ;
      
      if (!m_avi->m_track[j].m_audioSuperIndex) {
        // not initialized -> no index
        continue;
      }
      
      out4CC ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->fcc, &nhb);    /* "indx" */
      outLong ((char *) AVI_header,2+1+1+4+4+3*4 + m_avi->m_track[j].m_audioSuperIndex->nEntriesInUse * (8+4+4), &nhb);
      outShort ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->wLongsPerEntry, &nhb);
      outChar ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->bIndexSubType, &nhb);
      outChar ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->bIndexType, &nhb);
      outLong ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->nEntriesInUse, &nhb);
      out4CC ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->dwChunkId, &nhb);
      outLong ((char *) AVI_header,0, &nhb);
      outLong ((char *) AVI_header,0, &nhb);
      outLong ((char *) AVI_header,0, &nhb);
      
      for (k = 0; k < (int) m_avi->m_track[j].m_audioSuperIndex->nEntriesInUse; k++) {
	       uint32_t r = (m_avi->m_track[j].m_audioSuperIndex->aIndex[k].qwOffset >> 32) & 0xffffffff;
	       uint32_t s = (m_avi->m_track[j].m_audioSuperIndex->aIndex[k].qwOffset) & 0xffffffff;
        
	       /*
          printf("AUD[%d] NrEntries %d/%ld (%c%c%c%c) |0x%llX|%ld|%ld| \n",  j, k,
          m_avi->m_track[j].m_audioSuperIndex->nEntriesInUse,
          m_avi->m_track[j].m_audioSuperIndex->dwChunkId[0], m_avi->m_track[j].m_audioSuperIndex->dwChunkId[1],
          m_avi->m_track[j].m_audioSuperIndex->dwChunkId[2], m_avi->m_track[j].m_audioSuperIndex->dwChunkId[3],
          m_avi->m_track[j].m_audioSuperIndex->aIndex[k].qwOffset,
          m_avi->m_track[j].m_audioSuperIndex->aIndex[k].dwSize,
          m_avi->m_track[j].m_audioSuperIndex->aIndex[k].dwDuration
          );
          */
        
	       outLong ((char *) AVI_header,s, &nhb);
	       outLong ((char *) AVI_header,r, &nhb);
	       outLong ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->aIndex[k].dwSize, &nhb);
	       outLong ((char *) AVI_header,m_avi->m_track[j].m_audioSuperIndex->aIndex[k].dwDuration, &nhb);
      }
    }
    /* Finish stream list, i.e. put number of bytes in the list to proper pos */
    int32ToChar((char *) AVI_header+strl_start-4, (int32) (nhb-strl_start));
  }
  
  if (m_avi->m_isOpenDML) {
    out4CC ((char *) AVI_header,"LIST", &nhb);
    outLong ((char *) AVI_header,16, &nhb);
    out4CC ((char *) AVI_header,"odml", &nhb);
    out4CC ((char *) AVI_header,"dmlh", &nhb);
    outLong ((char *) AVI_header,4, &nhb);
    outLong ((char *) AVI_header, m_avi->m_totalFrames, &nhb);
  }
  
  /* Finish header list */
  
  int32ToChar( (char *) AVI_header+hdrl_start-4, (int32) (nhb-hdrl_start));
  
  // add INFO list --- (0.6.0pre4)
  
#ifdef INFO_LIST
  out4CC ((char *) AVI_header,"LIST", &nhb);
  
  info_start_pos = nhb;
  info_len = MAX_INFO_STRLEN + 12;
  outLong ((char *) AVI_header,info_len, &nhb); // rewritten later
  out4CC ((char *) AVI_header,"INFO", &nhb);
  
  out4CC ((char *) AVI_header,"ISFT", &nhb);
  //outLong ((char *) AVI_header,MAX_INFO_STRLEN);
  memset(id_str, 0, MAX_INFO_STRLEN);
  
  sprintf(id_str, "%s-%s", PACKAGE, VERSION);
  real_id_len = id_len = strlen(id_str)+1;
  if (id_len&1) id_len++;
  
  outLong ((char *) AVI_header,real_id_len, &nhb);
  
  memset(AVI_header+nhb, 0, id_len);
  memcpy(AVI_header+nhb, id_str, id_len);
  nhb += id_len;
  
  info_len = avi_parse_comments (AVI->m_commentFilePos, AVI_header+nhb, m_headerBytes - nhb - 8 - 12);
  if (info_len <= 0)
    info_len=0;
  
  // write correct len
  long2str(AVI_header+info_start_pos, info_len + id_len + 4+4+4);
  
  nhb += info_len;
  
  //   out4CC ((char *) AVI_header,"ICMT", &nhb);
  //   outLong ((char *) AVI_header,MAX_INFO_STRLEN, &nhb);
  
  //   calptr=time(NULL);
  //   sprintf(id_str, "\t%s %s", ctime(&calptr), "");
  //   memset(AVI_header+nhb, 0, MAX_INFO_STRLEN);
  //   memcpy(AVI_header+nhb, id_str, 25);
  //   nhb += MAX_INFO_STRLEN;
#endif
  
  // ----------------------------
  
  /* Calculate the needed amount of junk bytes, output junk */
  
  njunk = (int) (m_headerBytes - nhb - 8 - 12);
  
  /* Safety first: if njunk <= 0, somebody has played with
   m_headerBytes without knowing what (s)he did.
   This is a fatal error */
  
  if(njunk<=0)
  {
    fprintf(stderr,"AVI_close_output_file: # of header bytes too small\n");
    exit(1);
  }
  
  out4CC ((char *) AVI_header,"JUNK", &nhb);
  outLong ((char *) AVI_header,njunk, &nhb);
  memset(AVI_header + nhb,0,njunk);
  
  nhb += njunk;
  
  /* Start the movi list */
  out4CC ((char *) AVI_header,"LIST", &nhb);
  outLong ((char *) AVI_header,movi_len, &nhb); /* Length of list in bytes */
  out4CC ((char *) AVI_header,"movi", &nhb);
  
  /* Output the header, truncate the file to the number of bytes
   actually written, report an error if someting goes wrong */
  
  if ( lseek(m_avi->m_fileNum, 0, SEEK_SET)<0 ||
      mm_write(m_avi->m_fileNum,(char *)AVI_header, m_headerBytes)!=m_headerBytes ||
      ftruncate(m_avi->m_fileNum, m_avi->m_pos) <0 )
  {
    m_errorNumber = AVI_ERR_CLOSE;
    return -1;
  }
  
  
  // Fix up the empty additional RIFF and LIST chunks
  if (m_avi->m_isOpenDML) {
    int k = 0;
    char f[4];
    unsigned int len;
    
    for (k = 1; k < (int) m_avi->m_videoSuperIndex->nEntriesInUse; k++) {
      // the len of the RIFF Chunk
      lseek(m_avi->m_fileNum, m_avi->m_videoSuperIndex->stdindex[k]->qwBaseOffset+4, SEEK_SET);
      len = (unsigned int) (m_avi->m_videoSuperIndex->stdindex[k+1]->qwBaseOffset - m_avi->m_videoSuperIndex->stdindex[k]->qwBaseOffset - 8);
      int32ToChar(f, len);
      if (mm_write(m_avi->m_fileNum, f, 4) != 4) {
        m_errorNumber = AVI_ERR_CLOSE;
        return -1;
      }
      
      // len of the LIST/movi chunk
      lseek(m_avi->m_fileNum, 8, SEEK_CUR);
      len -= 12;
      int32ToChar(f, len);
      if (mm_write(m_avi->m_fileNum, f, 4) != 4) {
        m_errorNumber = AVI_ERR_CLOSE;
        return -1;
      }
    }
  }
   
  delete [] AVI_header;

  if(idxerror)
    return -1;
  
  return 0;
}

int OutputAVI::close(FrameFormat *format)
{
  int ret;
  
  /* If the file was open for writing, the header and index still have
   to be written */

  if(m_avi->m_mode == AVI_MODE_WRITE)
    ret = closeOutputFile(format);
  else
    ret = 0;
  
  return ret;
}



// inits a super index structure including its enclosed stdindex
int OutputAVI::initSuperIndex(unsigned char *idxtag, AVISuperIndexChunk **si)
{
  int k;
  
  AVISuperIndexChunk *sil = NULL;
  
  sil = new AVISuperIndexChunk (idxtag);

  memcpy (sil->fcc, "indx", 4);
  
  sil->dwSize = 0; // size of this chunk
  sil->wLongsPerEntry = 4;
  sil->bIndexSubType = 0;
  sil->bIndexType = AVI_INDEX_OF_INDEXES;
  sil->nEntriesInUse = 0; // none are in use
  memcpy (sil->dwChunkId, idxtag, 4);
  memset (sil->dwReserved, 0, sizeof (sil->dwReserved));
  
  // NR_IXNN_CHUNKS == allow 32 indices which means 32 GB files -- arbitrary
  if (sil->aIndex == NULL)
    sil->aIndex = new AVISuperIndexEntry[sil->wLongsPerEntry * NR_IXNN_CHUNKS];
  
  if (sil->aIndex == NULL) {
    m_errorNumber = AVI_ERR_NO_MEM;
    return -1;
  }
  memset (sil->aIndex, 0, sil->wLongsPerEntry * NR_IXNN_CHUNKS * sizeof (uint32_t));
  
  sil->stdindex = new AVIStandardIndexChunk*[NR_IXNN_CHUNKS];
  if (sil->stdindex == NULL) {
    m_errorNumber = AVI_ERR_NO_MEM;
    return -1;
  }
  for (k = 0; k < NR_IXNN_CHUNKS; k++) {
    sil->stdindex[k] = new AVIStandardIndexChunk;
    // gets rewritten later
    sil->stdindex[k]->qwBaseOffset = (uint64_t)k * NEW_RIFF_THRES;
  }
  
  *si = sil;
  
  return 0;
}

int OutputAVI::addChunk(unsigned char *tag, unsigned char *data, int length)
{
  unsigned char c[32];
  char p=0;
  
  /* Copy tag and length into c, so that we need only 1 write system call
   for these two values */
  memcpy(c,tag,4);
  int32ToChar((char *) c + 4,length);
  
  /* Output tag, length and data, restore previous position
   if the write fails */

  if( aviWrite(m_avi->m_fileNum,(char *)c, 8) != 8 ||
      aviWrite(m_avi->m_fileNum,(char *)data, length) != length ||
     aviWrite(m_avi->m_fileNum,&p,length&1) != (length&1)) // if len is uneven, write a pad byte
  {
    lseek(m_avi->m_fileNum, m_avi->m_pos,SEEK_SET);
    m_errorNumber = AVI_ERR_WRITE;
    return -1;
  }
  /* Update file position */
  
  m_avi->m_pos += 8 + PAD_EVEN(length);
  
  //fprintf(stderr, "pos=%lu %s\n", m_avi->m_pos, tag);
  
  return 0;
}

#define OUTD(n) int32ToChar(ix00+bl,n); bl+=4
#define OUTW(n) ix00[bl] = (n)&0xff; ix00[bl+1] = (n>>8)&0xff; bl+=2
#define OUTC(n) ix00[bl] = (n)&0xff; bl+=1
#define OUTS(s) memcpy(ix00+bl,s,4); bl+=4

// this does the physical writeout of the ix## structure
int OutputAVI::ixnnEntry(AVIStandardIndexChunk *ch, AVISuperIndexEntry *en)
{
  int bl, k;
  unsigned int max = ch->nEntriesInUse * sizeof (uint32_t) * ch->wLongsPerEntry + 24; // header
  char *ix00 = new char[max];
  char dfcc[5];
  memcpy (dfcc, ch->fcc, 4);
  dfcc[4] = 0;
  
  bl = 0;
  
  if (en) {
    en->qwOffset = m_avi->m_pos;
    en->dwSize = max;
    //en->dwDuration = ch->nEntriesInUse -1; // NUMBER OF stream ticks == frames for video/samples for audio
  }
  
#ifdef DEBUG_ODML
  //printf ("ODML Write %s: Entries %ld size %d \n", dfcc, ch->nEntriesInUse, max);
#endif
  
  //OUTS(ch->fcc);
  //OUTD(max);
  OUTW(ch->wLongsPerEntry);
  OUTC(ch->bIndexSubType);
  OUTC(ch->bIndexType);
  OUTD(ch->nEntriesInUse);
  OUTS(ch->dwChunkId);
  OUTD(ch->qwBaseOffset&0xffffffff);
  OUTD((ch->qwBaseOffset>>32)&0xffffffff);
  OUTD(ch->dwReserved3);
  
  for (k = 0; k < (int) ch->nEntriesInUse; k++) {
    OUTD(ch->m_aIndex[k].dwOffset);
    OUTD(ch->m_aIndex[k].dwSize);
    
  }
  addChunk ((unsigned char *) ch->fcc, (unsigned char *) ix00, max);
  
  delete [] ix00;
  
  return 0;
}
#undef OUTS
#undef OUTW
#undef OUTD
#undef OUTC

int OutputAVI::addStandardIndex(unsigned char *idxtag, unsigned char *strtag, AVIStandardIndexChunk *stdil)
{
  memcpy (stdil->fcc, idxtag, 4);
  stdil->dwSize = 4096;
  stdil->wLongsPerEntry = 2;
  stdil->bIndexSubType = 0;
  stdil->bIndexType = AVI_INDEX_OF_CHUNKS;
  stdil->nEntriesInUse = 0;
  
  // cp 00db ChunkId
  memcpy(stdil->dwChunkId, strtag, 4);
  
  stdil->m_aIndex = new AVIStandardIndexEntry[stdil->dwSize * stdil->wLongsPerEntry] ;
  
  return 0;
}


int OutputAVI::addOdmlIndexEntryCore(long flags, int64_t pos, unsigned long len, AVIStandardIndexChunk *si)
{
  int cur_chunk_idx;
  // put new chunk into index
  si->nEntriesInUse++;
  cur_chunk_idx = si->nEntriesInUse-1;
  
  // need to fetch more memory
  if (cur_chunk_idx >= (int) si->dwSize) {
    si->dwSize += 4096;
    
    if (si->m_aIndex != NULL)
      delete si->m_aIndex;
    si->m_aIndex = new  AVIStandardIndexEntry[ si->dwSize * si->wLongsPerEntry];
  }
  
  if(len > m_avi->m_maxLength)
    m_avi->m_maxLength = len;
  
  // if bit 31 is set, it is NOT a keyframe
  if (flags != 0x10) {
    len |= 0x80000000;
  }
  
  si->m_aIndex [ cur_chunk_idx ].dwSize = len;
  si->m_aIndex [ cur_chunk_idx ].dwOffset = (uint32) (pos - si->qwBaseOffset + 8);
  
  //printf("ODML: POS: 0x%lX\n", si->aIndex [ cur_chunk_idx ].dwOffset);
  
  return 0;
}

int OutputAVI::addOdmlIndexEntry(unsigned char *tag, long flags, int64 pos, unsigned long len)
{
  char fcc[5];
    //int audio = (strchr (tag, 'w') ? 1 : 0);
  int audio = 0;
  int video = !audio;
  
  unsigned int cur_std_idx;
  int audtr;
  int64_t towrite = 0LL;
  
  if (video) {
    if (!m_avi->m_videoSuperIndex) {
      if (initSuperIndex( (unsigned char *) "ix00", &m_avi->m_videoSuperIndex) < 0)
        return -1;
      m_avi->m_videoSuperIndex->nEntriesInUse++;
      cur_std_idx = m_avi->m_videoSuperIndex->nEntriesInUse - 1;
      
      if (addStandardIndex ((unsigned char *) "ix00", (unsigned char *) "00db", m_avi->m_videoSuperIndex->stdindex[ cur_std_idx ]) < 0)
      return -1;
    } // init
    
  } // video
  
  
  towrite = 0;
  if (m_avi->m_videoSuperIndex) {
    
    cur_std_idx = m_avi->m_videoSuperIndex->nEntriesInUse-1;
    //towrite += m_avi->m_videoSuperIndex->stdindex[cur_std_idx]->nEntriesInUse * 8 + 4 + 4 + 2 + 1 + 1 + 4 + 4 + 8 + 4;
    towrite += m_avi->m_videoSuperIndex->stdindex[cur_std_idx]->nEntriesInUse * 8 + 32;
    if (cur_std_idx == 0) {
      towrite += m_avi->m_noIndex * 16 + 8;
      towrite += m_headerBytes;
    }
  }
  
  for (audtr=0; audtr<m_avi->m_numberAudioTracks; audtr++) {
    if (m_avi->m_track[audtr].m_audioSuperIndex) {
      cur_std_idx = m_avi->m_track[audtr].m_audioSuperIndex->nEntriesInUse-1;
      towrite += m_avi->m_track[audtr].m_audioSuperIndex->stdindex[cur_std_idx]->nEntriesInUse*8
      + 4+4+2+1+1+4+4+8+4;
    }
  }
  towrite += len + (len&1) + 8;
  
  //printf("ODML: towrite = 0x%llX = %lld\n", towrite, towrite);
  
  if (m_avi->m_videoSuperIndex &&
      (int64_t)(m_avi->m_pos + towrite) > (int64_t)((int64_t) NEW_RIFF_THRES * m_avi->m_videoSuperIndex->nEntriesInUse)) {
    
#ifdef DEBUG_ODML
    fprintf(stderr, "Adding a new RIFF chunk: %d\n", m_avi->m_videoSuperIndex->nEntriesInUse);
#endif
    
    // rotate ALL indices
    m_avi->m_videoSuperIndex->nEntriesInUse++;
    cur_std_idx = m_avi->m_videoSuperIndex->nEntriesInUse-1;
    
    if (m_avi->m_videoSuperIndex->nEntriesInUse > (uint32) NR_IXNN_CHUNKS) {
      fprintf (stderr, "Internal error in avilib - redefine NR_IXNN_CHUNKS\n");
      fprintf (stderr, "[avilib dump] cur_std_idx=%d NR_IXNN_CHUNKS=%d"
               "POS=%lld towrite=%lld\n",
               cur_std_idx,NR_IXNN_CHUNKS, m_avi->m_pos, (long long int) towrite);
      return -1;
    }
    if (addStandardIndex ((unsigned char *) "ix00", (unsigned char *) "00db", m_avi->m_videoSuperIndex->stdindex[ cur_std_idx ]) < 0)
      return -1;

    for (audtr = 0; audtr < m_avi->m_numberAudioTracks; audtr++) {
      char aud[5];
      if (!m_avi->m_track[audtr].m_audioSuperIndex) {
        // not initialized -> no index
        continue;
      }
      m_avi->m_track[audtr].m_audioSuperIndex->nEntriesInUse++;
      
      sprintf(fcc, "ix%02d", audtr+1);
      sprintf(aud, "0%01dwb", audtr+1);
      if (addStandardIndex ((unsigned char *) fcc, (unsigned char *) aud, m_avi->m_track[audtr].m_audioSuperIndex->stdindex[m_avi->m_track[audtr].m_audioSuperIndex->nEntriesInUse - 1 ]) < 0)
        return -1;
    }
    
    // write the new riff;
    if (cur_std_idx > 0) {
      
      // dump the _previous_ == already finished index
      ixnnEntry (m_avi->m_videoSuperIndex->stdindex[cur_std_idx - 1], &m_avi->m_videoSuperIndex->aIndex[cur_std_idx - 1]);
      m_avi->m_videoSuperIndex->aIndex[cur_std_idx - 1].dwDuration =
      m_avi->m_videoSuperIndex->stdindex[cur_std_idx - 1]->nEntriesInUse - 1;
      
      for (audtr = 0; audtr < m_avi->m_numberAudioTracks; audtr++) {
        
        if (!m_avi->m_track[audtr].m_audioSuperIndex) {
          // not initialized -> no index
          continue;
        }
        ixnnEntry (m_avi->m_track[audtr].m_audioSuperIndex->stdindex[cur_std_idx - 1], &m_avi->m_track[audtr].m_audioSuperIndex->aIndex[cur_std_idx - 1]);
        m_avi->m_track[audtr].m_audioSuperIndex->aIndex[cur_std_idx - 1].dwDuration =
        m_avi->m_track[audtr].m_audioSuperIndex->stdindex[cur_std_idx - 1]->nEntriesInUse - 1;
        if (m_avi->m_track[audtr].a_fmt == 0x1) {
          m_avi->m_track[audtr].m_audioSuperIndex->aIndex[cur_std_idx - 1].dwDuration *=
          m_avi->m_track[audtr].a_bits*m_avi->m_track[audtr].a_rate*m_avi->m_track[audtr].a_chans/800;
        }
      }
      
      // XXX: dump idx1 structure
      if (cur_std_idx == 1) {
        addChunk((unsigned char *)"idx1", (unsigned char *)m_avi->m_index[0].m_indexEntry, m_avi->m_noIndex * 16);
        // qwBaseOffset will contain the start of the second riff chunk
      }
      // Fix the Offsets later at closing time
      addChunk((unsigned char *)"RIFF", (unsigned char *) "AVIXLIST\0\0\0\0movi", 16);
      
      m_avi->m_videoSuperIndex->stdindex[ cur_std_idx ]->qwBaseOffset = m_avi->m_pos -16 -8;
#ifdef DEBUG_ODML
      printf("ODML: RIFF No.%02d at Offset 0x%llX\n", cur_std_idx, m_avi->m_pos -16 -8);
#endif
      
      for (audtr = 0; audtr < m_avi->m_numberAudioTracks; audtr++) {
        if (m_avi->m_track[audtr].m_audioSuperIndex)
          m_avi->m_track[audtr].m_audioSuperIndex->stdindex[ cur_std_idx ]->qwBaseOffset =
          m_avi->m_pos -16 -8;
        
      }
      
      // now we can be sure
      m_avi->m_isOpenDML++;
    }
  }
  
  
  if (video) {
    addOdmlIndexEntryCore(flags, m_avi->m_pos, len, m_avi->m_videoSuperIndex->stdindex[ m_avi->m_videoSuperIndex->nEntriesInUse-1 ]);
    m_avi->m_totalFrames++;
  } // video
  
  if (audio) {
    addOdmlIndexEntryCore(flags, m_avi->m_pos, len, m_avi->m_track[m_avi->m_currentAudioTrack].m_audioSuperIndex->stdindex[m_avi->m_track[m_avi->m_currentAudioTrack].m_audioSuperIndex->nEntriesInUse-1 ]);
  }
  
  return 0;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
int OutputAVI::addIndexEntry(unsigned char *tag, long flags, unsigned long pos, unsigned long len)
{
  void *ptr;
  
  if(m_avi->m_noIndex >= m_avi->m_maxIndex) {
    ptr = realloc((void *)m_avi->m_index, (m_avi->m_maxIndex + 4096) * sizeof(aviIndexEntries));
   
    if(ptr == 0) {
      m_errorNumber = AVI_ERR_NO_MEM;
      return -1;
    }
    m_avi->m_maxIndex += 4096;
    m_avi->m_index = (aviIndexEntries *) ptr;
  }
  
  /* Add index entry */
  
  //   fprintf(stderr, "INDEX %s %ld %lu %lu\n", tag, flags, pos, len);
  
  memcpy(m_avi->m_index[m_avi->m_noIndex].m_indexEntry, tag, 4);
  int32ToChar((char *) m_avi->m_index[m_avi->m_noIndex].m_indexEntry + 4,flags);
  int32ToChar((char *) m_avi->m_index[m_avi->m_noIndex].m_indexEntry + 8, pos);
  int32ToChar((char *) m_avi->m_index[m_avi->m_noIndex].m_indexEntry +12, len);

  /* Update counter */
  
  m_avi->m_noIndex++;
  
  if(len > m_avi->m_maxLength)
    m_avi->m_maxLength = len;

  return 0;
}


int OutputAVI::writeData(char *data, unsigned long length, int audio, int keyframe)
{
  int n = 0;
  
  unsigned char astr[6];
  
  /* Add index entry */
  
  //set tag for current audio track
  sprintf((char *)astr, "0%1dwb", (int)(m_avi->m_currentAudioTrack+1));

  if(audio) {
    if (!m_avi->m_isOpenDML)
      n = addIndexEntry(astr, 0x10, (unsigned long) m_avi->m_pos, length);
    n += addOdmlIndexEntry(astr,0x10,m_avi->m_pos,length);
  }
  else {
    if (!m_avi->m_isOpenDML)
      n = addIndexEntry((unsigned char *)"00db", ((keyframe)?0x10:0x0), (unsigned long) m_avi->m_pos, length);
    n += addOdmlIndexEntry((unsigned char *)"00db", ((keyframe)?0x10:0x0), m_avi->m_pos, length);
  }
  
  if(n)
    return -1;
  
  /* Output tag and data */
  
  if(audio)
    n = addChunk((unsigned char *)astr, (unsigned char *) data,length);
  else
    n = addChunk((unsigned char *)"00db",(unsigned char *) data,length);

  if (n)
    return -1;
  
  return 0;
}


void OutputAVI::formatRGB8(uint8** input,       //!< input buffer
                           uint8** output,      //!< output buffer
                           FrameFormat *source,         //!< format of source buffer
                           int symbolSizeInBytes     //!< number of bytes per symbol
)
{
  int i, j;
  // final buffer
  uint8 *ocmp  = NULL;
  // original buffer
  uint8 *icmp0 = *input;
  
  uint8 *icmp1 = icmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *icmp2 = icmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_height[Y_COMP]; i++) {
    ocmp  = *output + (3 * (source->m_height[Y_COMP] - i - 1) * source->m_width[Y_COMP]) *  symbolSizeInBytes;
    for (j = 0; j < source->m_width[Y_COMP]; j++) {
      memcpy(ocmp, icmp0, symbolSizeInBytes);
      ocmp  += symbolSizeInBytes;
      icmp0 += symbolSizeInBytes;
      memcpy(ocmp, icmp1, symbolSizeInBytes);
      ocmp  += symbolSizeInBytes;
      icmp1 += symbolSizeInBytes;
      memcpy(ocmp, icmp2, symbolSizeInBytes);
      ocmp  += symbolSizeInBytes;
      icmp2 += symbolSizeInBytes;
    }
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}


void OutputAVI::reFormat(uint8** input,       //!< input buffer
                         uint8** output,      //!< output buffer
                         FrameFormat *format,         //!< format of source buffer
                         int symbolSizeInBytes     //!< number of bytes per symbol
)
{
#ifdef __SIM2_SUPPORT_ENABLED__
  if (format->m_pixelFormat == PF_SIM2) {
    if (format->m_cositedSampling) {
      if (format->m_improvedFilter)
        formatSim2CoSitedF2(input, output, format, symbolSizeInBytes);
      else
        formatSim2CoSited(input, output, format, symbolSizeInBytes);
    }
    else {
      if (format->m_improvedFilter)
        formatSim2F2(input, output, format, symbolSizeInBytes);
      else
        formatSim2(input, output, format, symbolSizeInBytes);
    }
  }
  else
#endif
  if (format->m_pixelFormat == PF_BGR || format->m_pixelFormat == PF_RGB) {
    formatRGB8(input, output, format, symbolSizeInBytes);
  }
  else {
    reInterleave(input, output, format, symbolSizeInBytes);
  }
}

/*
 ************************************************************************
 * \brief
 *    Writes one new frame into a an AVI file
 *
 * \param outputFile
 *    Output file to write to
 * \param frameNumber
 *    Frame number in the output file
 * \param fileHeader
 *    Number of bytes in the output file to be skipped when writing
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int OutputAVI::writeOneFrame (IOVideo *outputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileWrite = 0;
  FrameFormat *format = &outputFile->m_format;
  int64 pos = 0;
  
  const int64 framesizeInBytes = getFrameSizeInBytes(format, outputFile->m_isInterleaved);
  
  // Here we are at the correct position for the source frame in the file.
  // Now write it.
  
  memset(m_buf, 0, (size_t) framesizeInBytes);
  
  if ((format->m_picUnitSizeOnDisk & 0x07) == 0)  {
    if (m_bitDepthComp[Y_COMP] == 8)
      imageReformat ( m_buf, &m_data[0], format, m_picUnitSizeShift3 );
    else
      imageReformatUInt16 ( m_buf, format, m_picUnitSizeShift3 );

    // If format is interleaved, then perform reinterleaving
    //if ((format->m_chromaFormat != CF_420) || m_isInterleaved) {
    if (format->m_chromaFormat != CF_420) {
      reFormat ( &m_buf, &m_iBuf, format, m_picUnitSizeShift3);
    }
    else {
    }
    
    if(m_avi->m_mode==AVI_MODE_READ) {
      m_errorNumber = AVI_ERR_NOT_PERM;
      return -1;
    }
    
    pos = m_avi->m_pos;
    int keyframe = 0;
    
    if (writeData((char *) m_buf, (int) framesizeInBytes, 0, keyframe))
      return -1;
    
    m_avi->m_lastPosition = pos;
    m_avi->m_lastLength = m_picUnitSizeShift3;
    m_avi->m_videoFrames++;

  }
  else {
    fprintf (stderr, "writeOneFrame (NOT IMPLEMENTED): pic unit size on disk must be divided by 8");
    exit(EXIT_FAILURE);
  }

  return fileWrite;
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

