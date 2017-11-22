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
 ************************************************************************
 * \file IOVideo.H
 *
 * \brief
 *    Video I/O Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 ************************************************************************
 */

#ifndef _IOVideo_H_
#define _IOVideo_H_

#include "Defines.H"
#include "TypeDefs.H"
#include "FrameFormat.H"


static const char COLOR_SPACE       [12][20] = { "Unknown", "YCbCr", "RGB", "XYZ", "YDZDX", "YCbCr_CL", "YFBFRV1", "YFBFRV2", "YFBFRV3", "YFBFRV4", "Yu'v'", "ICtCp"};
static const char COLOR_PRIMARIES   [13][20] = { "Unknown", "BT.709", "BT.2020", "P3D60", "P3D65", "XYZ", "NONE", "AMT", "YCoCg", "EXT", "LMSD", "BT.601" };
static const char COLOR_FORMAT      [ 5][20] = { "Unknown", "4:0:0", "4:2:0", "4:2:2", "4:4:4"};
static const char PIXEL_TYPE        [ 3][20] = { "UINT", "HALF", "SINGLE"};
static const char INTERLACED_TYPE   [ 2][2 ] = { "p", "i"};
static const char SOURCE_RANGE_TYPE [ 6][20] = { "Unknown", "STANDARD", "FULL", "RESTRICTED", "SDI Scaled", "SDI"};
static const char TRANSFER_CHAR     [24][20] = { "Null", "PQ", "PH", "HG", "Adapt-PQ", "Adapt-PH", "POWER", "Mod-PQ", "Adapt-PQS", "Biased-MPQ", "Adapt-MPQ", "HLG", "Normal", "Hybrid-PQ", "Hybrid-PQ2", "sRGB", "asRGB", "PowerSimp", "BT.709", "DPXLog", "CineonLog", "ST 240", "SIM2"};


class IOVideo {
public:
  char          m_fName[FILE_NAME_SIZE];  //!< video file name
  char          m_fHead[FILE_NAME_SIZE];  //!< header of video file name
  char          m_fTail[FILE_NAME_SIZE];  //!< tail of video file name
  int           m_fileNum;                //!< video file number
  VideoFileType m_videoType;              //!< File format
  FrameFormat   m_format;                 //!< video format information
  bool          m_isConcatenated;         //!< Single or multifile input?
  bool          m_isInterleaved;          //!< Support for interleaved and non-interleaved input sources
  int           m_fileHeader;
  bool          m_zeroPad;                //!< Used when separate image files are used as input. Enables zero padding for file numbering
  int           m_numDigits;              //!< Number of digits for file numbering
  int           m_startFrame;             //!< start frame
  int           m_endFrame;               //!< end frame
  int           m_numFrames;              //!< number of frames

  // AVI related information to be added here
  int*          m_avi;
  
  IOVideo() {
    m_fName[0]   = '\0';
    m_fHead[0]   = '\0';
    m_fTail[0]   = '\0';
    m_fileNum = -1;
    m_isConcatenated = FALSE;
    m_isInterleaved  = FALSE;
    m_fileHeader     = 0;
    m_zeroPad        = FALSE;
    m_numDigits      = 0;
    m_startFrame     = 0;
    m_endFrame       = 1;
    m_numFrames      = 1;
    m_avi            = NULL;
  }
};

#endif

