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
 * \file OutputTIFF.H
 *
 * \brief
 *    OutputTIFF class header file for OpenEXR files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __OutputTIFF_H__
#define __OutputTIFF_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include <vector>
#include "Output.H"
#include "IOTIFF.H"

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------


class OutputTIFF : public Output {
  
private:
  bool          m_memoryAllocated;
  Tiff          m_tiff;
  int64         m_tiffSize;
  int64         maxFramePosition;
    
  void          allocateMemory          ( FrameFormat *format );
  void          freeMemory              ();
  
  uint32        setIntArray             ( Tiff * t, uint32 offset, TiffType type, uint32 a[], int n );
  uint32        writeDirectoryEntry     (Tiff * t, uint32 tag, TiffType type, uint32 count, uint32 offset);
  uint32        writeImageData          ( Tiff * t );
  uint32        writeImageFileDirectory ( Tiff * t );
  uint32        writeImageFileHeader    ( Tiff * t );
  
  int           reformatData            ( );
  int           openFrameFile           ( IOVideo *outputFile, int FrameNumberInFile);
  int           writeAttributeInfo      ( int vfile, FrameFormat *source );
  int           writeData               ( int vfile,  FrameFormat *source, uint8 *buf );
  int           writeFileFromMemory     ( Tiff * t, int *fd, uint32 counter);
  int           writeHeaderData         ( int vfile, FrameFormat *source );
  int           writeTiff               (FrameFormat *format, int *fd);
public:
  OutputTIFF( IOVideo *inputFile, FrameFormat *format );
  virtual ~OutputTIFF();
  
  virtual int writeOneFrame ( IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip );
};

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
