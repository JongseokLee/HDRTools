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
 * \file InputEXR.H
 *
 * \brief
 *    InputEXR class header file for OpenEXR files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __InputEXR_H__
#define __InputEXR_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <vector>
#include "Input.H"
#include "OpenEXR.H"

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

class InputEXR : public Input {
  
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
  
  unsigned int  halfToFloat (uint16 value);
  
  void          allocateMemory   ( FrameFormat *format );
  void          freeMemory       ();

  int           readAttributeInfo( int vfile, FrameFormat *source );
  int           readHeaderData   ( int vfile, FrameFormat *source );
  int           readData         ( int vfile,  FrameFormat *source, uint8 *buf );
  int           reformatData     ( uint8 *buf,  float  *floatComp[3] );
  int           openFrameFile    ( IOVideo *inputFile, int FrameNumberInFile);
  
public:
  InputEXR( IOVideo *inputFile, FrameFormat *format );
  virtual ~InputEXR();
  
  virtual int readOneFrame ( IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip );
};

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
