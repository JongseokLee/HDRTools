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
 * \file ProjectParameters.H
 *
 * \brief
 *    ProjectParameters class (main) definition for HDRConvScaler
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __ProjectParameters_H__
#define __ProjectParameters_H__

#include "Global.H"
#include "Parameters.H"
#include "AddNoise.H"
#include "ColorTransform.H"
#include "FrameScale.H"


#define DEFAULTCONFIGFILENAME "HDRConvScaler.cfg"

class ProjectParameters : public Parameters {
private:
public:
  /*input*/
  IOVideo         m_inputFile;
  IOVideo         m_outputFile;
  FrameFormat     m_source;
  FrameFormat     m_output;
  int             m_frameSkip;     //! Frame skipping for input
  
  float           m_srcNormalScale;
  float           m_outNormalScale;
  float           m_srcMinValue;               //!< Brightness range value (min)
  float           m_srcMaxValue;               //!< Brightness range value (max)
  float           m_outMinValue;               //!< Brightness range value (min)
  float           m_outMaxValue;               //!< Brightness range value (max)

  bool            m_transformPrecision;
  bool            m_useSingleTransferStep;   //! Use single or multiple steps (for normalization) when applying PQ
  bool            m_filterInFloat;           //! Use floating precision for filtering
  int             m_chromaDownsampleFilter;  //! Chroma downsampling filter
  int             m_chromaUpsampleFilter;    //! Chroma upsampling filter
  bool            m_outputSinglePrecision;   //! Set output, for OpenEXR files, to single instead of half precision
    // Scaling Parameters
  FrameScaleParams  m_fsParams;

  int             m_useMinMax;
  int             m_useHighPrecisionTransform; //!< High precision transform for BT.2020 (proper inverse)

  int             m_addNoise;                //! Add noise to the input signal
  double          m_noiseVariance;           //! Noise variance
  double          m_noiseMean;               //! Noise Mean
  ClosedLoopTrans m_closedLoopConversion;
  int             m_closedLoopIterations;
  bool            m_linearDownConversion;
  int             m_useAdaptiveUpsampling;
  int             m_useAdaptiveDownsampling;
  bool            m_rgbDownConversion;
  bool            m_bUseChromaDeblocking;
  bool            m_bUseWienerFiltering;
  bool            m_bUse2DSepFiltering;
  bool            m_b2DSepMode;
  bool            m_enableTFunctionLUT;        //!< Use Transfer function LUT for some operations
  bool            m_enableTFLUTs;              //!< Use Transfer function LUTs



  // Cropping parameters
  int             m_cropOffsetLeft;
  int             m_cropOffsetTop;
  int             m_cropOffsetRight;
  int             m_cropOffsetBottom;
  
  // Color Transform function parameters
  ColorTransformParams   m_ctParams;

  virtual void update();
  virtual void refresh();
};

extern ProjectParameters ccParams;

#endif
