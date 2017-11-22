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
 * \file OutputEXR.H
 *
 * \brief
 *    OutputEXR class header file for OpenEXR files
 *    FloatToHalf alternative code taken from code provided by Fabian "ryg" Giesen
 *    and can be found at: https://gist.github.com/rygorous/2156668
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __OutputEXR_H__
#define __OutputEXR_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include <vector>
#include "Output.H"
#include "OpenEXR.H"


union FP32
{
  uint32 u;
  float f;
  struct
  {
    uint32 Significand : 23;
    uint32 Exponent    : 8;
    uint32 Sign        : 1;
  };
};

union FP16
{
  unsigned short u;
  struct
  {
    uint32 Significand : 10;
    uint32 Exponent    : 5;
    uint32 Sign        : 1;
  };
};

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

class OutputEXR : public Output {
  
private:
  bool                 m_memoryAllocated;
  // Header
  int                  m_magicNumber;  // 20000630
  int                  m_versionField; // [0..7] = version number, [8..31]: format related flags
  
  // breakdown of version field
  char                 m_version;
  bool                 m_isTile;
  bool                 m_hasLongNames;
  bool                 m_hasDeepData;
  bool                 m_isMultipart;
  int                  m_reserved;
  // attributes
  char                *m_name;
  char                *m_type;
  int                  m_attributeSize;
  int                  m_valueVectorSize;  // Sets m_value assigned size
  vector<char>    m_value;
  int                  m_noChannels;
  channelInfo         *m_channels;
  
  box2i                m_dataWindow;
  box2i                m_displayWindow;
  
  vector<uint64>  m_offsetTable;
  int                  m_offsetTableSize;
  
  // data chunk info.
  // note that the actual data chunk is definited in Input.H
  int           m_yCoordinate;
  int           m_pixelDataSize;
  
  void          allocateMemory   ( FrameFormat *format );
  void          freeMemory       ();

  //uint16        floatToHalfTrunc (uint32 value);
  //uint16        floatToHalfRound (uint32 value);
  uint16        (*floatToHalf)   (uint32);

  int           openFrameFile     ( IOVideo *outputFile, int FrameNumberInFile);
  int           writeAttributeInfo( int vfile, FrameFormat *source );
  int           writeHeaderData   ( int vfile, FrameFormat *source );
  int           writeData         ( int vfile, uint8 *buf );
  int           reformatData (uint8 *buf,  float  *floatComp[3]);
  int           writeAttributeAndType( int vfile, char *attributeName, char *attributeType, int attributeSize, char *attributeValue);
  
public:
  OutputEXR                 (IOVideo *inputFile, FrameFormat *format);
  virtual ~OutputEXR        ();
  virtual int writeOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip);
};

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
