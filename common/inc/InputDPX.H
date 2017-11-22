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
 * \file InputDPX.H
 *
 * \brief
 *    InputDPX class header file for DPX files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __InputDPX_H__
#define __InputDPX_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include <vector>
#include "Input.H"
#include "IOTIFF.H"
#include "IODPX.H"

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------


class InputDPX : public Input {
  
private:
  
  DPXFileData     m_dpxFile;

  bool            m_memoryAllocated;
  
  int64           m_prevSize;
    
  void            allocateMemory         ( DPXFileData * t, FrameFormat *format );
  void            freeMemory             ( );
  
  int             readFileIntoMemory     ( DPXFileData * t, int *fd );
  int             readImageData          ( DPXFileData * t );

  int             readImageElement                (DPXFileData * t, DPXImageElement *imageElement);
  int             readGenericFileHeader           (DPXFileData * t);
  int             readGenericImageHeader          (DPXFileData * t);
  int             readGenericOrientationHeader    (DPXFileData * t);
  int             readIndustryFilmInfoHeader      (DPXFileData * t);
  int             readIndustryTelevisionInfoHeader(DPXFileData * t);
  int             readUserDefinedData             (DPXFileData * t);

  int             reformatData           ( );



  int           readDPX                ( FrameFormat *format, int *fd );
  int           openFrameFile          ( IOVideo *inputFile, int FrameNumberInFile );
  void          printHeader(DPXFileData * dpx);
public:
  InputDPX                ( IOVideo *inputFile, FrameFormat *format );
  virtual ~InputDPX       ( );
  virtual int readOneFrame ( IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip );
};

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
