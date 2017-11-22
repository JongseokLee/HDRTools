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
 * \file FrameFormat.H
 *
 * \brief
 *    FrameFormat class header. 
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __FrameFormat_H__
#define __FrameFormat_H__

#include "Global.H"

class FrameFormat {  
public:
  
  bool              m_isFloat;                      //!< Floating point data
  bool              m_isInterlaced;                 //!< Interlaced source
  ChromaFormat      m_chromaFormat;                 //!< YUV format (0=4:0:0, 1=4:2:0, 2=4:2:2, 3=4:4:4)
  ColorSpace        m_colorSpace;                   //!< ColorSpace (0: YUV, 1: RGB, 2: XYZ)
  ColorPrimaries    m_colorPrimaries;               //!< Color primaries (0: BT.709, 1: BT.2020, 2:P3D60)
  int               m_iConstantLuminance;           //!< Constant luminance. 0: Off, 1: normal, 2: Trimmed
  PixelFormat       m_pixelFormat;                  //!< Pixel Format which provides info mainly for interleaved formats
  ChromaLocation    m_chromaLocation[FP_TOTAL];     //!< Chroma location type for subsampled ChromaFormats. 
                                                    //!< Note that we keep two elements for top and bottom fields.
  SampleRange       m_sampleRange;                  //!< Sample range (0: Standard, 1: Full, 2: SDI)
  TransferFunctions m_transferFunction;             //!< Frame's transfer function
  float             m_systemGamma;                  //!< Overall system gamma used by the Hybrid Gamma transfer function

  DisplayAdjustment m_displayAdjustment;
  
  float             m_frameRate;                    //!< frame rate
  int               m_width[4];                     //!< component frame width
  int               m_height[4];                    //!< component frame height
  int               m_compSize[4];                  //!< component sizes (width * height)
  int64             m_size;                         //!< total image size (sum of m_compSize)
  int32             m_pixelType[4];                 //!< pixel type (for OpenEXR)
  int               m_bitDepthComp[4];              //!< component bit depth
  int               m_picUnitSizeOnDisk;            //!< picture sample unit size on storage medium
  int               m_picUnitSizeShift3;            //!< m_picUnitSizeOnDisk >> 3
  
  bool              m_useFloatRound;                //!< used for rounding float values
  
  // These are special parameters to control Sim2 file conversion
  // Given the current structure of the code, it was easier to add these here instead
  // of a different location
  bool              m_cositedSampling;              //!< Use co-sited chroma sampling for the Sim2 display
  bool              m_improvedFilter;               //!< update the chroma downsampling filter (for the Sim2)
  
  FrameFormat() {
    m_isFloat            = FALSE;
    m_isInterlaced       = FALSE;
    m_chromaFormat       = CF_UNKNOWN;
    m_colorSpace         = CM_UNKNOWN;
    m_pixelFormat        = PF_UNKNOWN;
    m_colorPrimaries     = CP_UNKNOWN;
    m_iConstantLuminance = 0;
    m_sampleRange        = SR_STANDARD;
    m_transferFunction   = TF_NULL;
    m_displayAdjustment  = DA_NULL;
    m_frameRate          = 1.0;
    m_width[0]           = m_width[1]        = m_width[2]        = 8;
    m_height[0]          = m_height[1]       = m_height[2]       = 8;
    m_compSize[0]        = m_compSize[1]     = m_compSize[2]     = 64;
    m_width[3]           = m_height[3]       = m_compSize[3]     = 0;
    m_size               = 192;
    m_bitDepthComp[0]    = m_bitDepthComp[1] = m_bitDepthComp[2] = 8;
    m_bitDepthComp[3]    = 0;
    m_pixelType[0] = m_pixelType[1] = m_pixelType[2] = m_pixelType[3] = HALF;
    m_picUnitSizeOnDisk  = 8;
    m_picUnitSizeShift3  = m_picUnitSizeOnDisk >> 3;
    m_useFloatRound      = FALSE;
    m_cositedSampling    = FALSE;
    m_improvedFilter     = FALSE;
    m_chromaLocation[FP_TOP] = m_chromaLocation[FP_BOTTOM] = CL_ZERO;
  }
} ;


#endif

