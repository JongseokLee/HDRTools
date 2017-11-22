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
 * \file InputY4M.H
 *
 * \brief
 *    InputY4M class header file for YUV4MPEG2 files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __InputY4M_H__
#define __InputY4M_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Input.H"
#include <vector>

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

class InputY4M : public Input {
private:

  vector<char> m_streamHeader;
  int m_streamHeaderSize;
  int readData (int vfile, FrameFormat *source,    uint8 *buf);
  int readData (int vfile, int framesizeInBytes, uint8 *buf);
  int64 getFrameSizeInBytes(FrameFormat *source, bool isInterleaved);
  int readStreamHeader (IOVideo *inputFile,  FrameFormat *source);
  int parseChromaSubsampling (FrameFormat *source, const char *taggedField);
  int parseInterlaceSpec (FrameFormat *source, const char *taggedField);
  void parseRatio (int *value0, int *value1,  char *taggedField);
  int readFrameHeader (int vfile,  FrameFormat *format);
public:
  InputY4M(IOVideo *inputFile, FrameFormat *format);
  virtual ~InputY4M();
  virtual int readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip);
};

#endif 
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
