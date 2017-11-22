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
 * \file ColorTransform.H
 *
 * \brief
 *    Base Class Header for the application of a Color Transform
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __ColorTransform_H__
#define __ColorTransform_H__

#include "Global.H"
#include "Parameters.H"
#include "Frame.H"

class ColorTransformParams {
public:
  ColorSpace              m_iColorSpace;
  ColorPrimaries          m_iColorPrimaries; 
  ChromaFormat            m_iChromaFormat;

  ColorSpace              m_oColorSpace;
  ColorPrimaries          m_oColorPrimaries; 
  ChromaFormat            m_oChromaFormat;

  bool                    m_transformPrecision; 
  int                     m_useHighPrecision;
  ClosedLoopTrans         m_closedLoopTransform; 
  int                     m_iConstantLuminance;
  int                     m_oConstantLuminance; 
  TransferFunctions       m_transferFunction;
  int                     m_bitDepth;
  SampleRange             m_range;
  int                     m_downMethod;
  int                     m_upMethod;
  int                     m_useAdaptiveDownsampler;
  int                     m_useAdaptiveUpsampler;
  int                     m_useMinMax;
  int                     m_maxIterations;
  vector <ChromaLocation> m_oChromaLocationType;
  bool                    m_useFloatPrecision;
  bool                    m_enableLUTs;
  bool                    m_enableTFDerivLUTs;
  char                    m_yAdjustModelFile[MAX_LINE_LEN];
  float                   m_min;
  float                   m_max;
  float                   m_iSystemGamma;
  float                   m_oSystemGamma;
  
  float                   m_minValue;
  float                   m_maxValue;
  
  DisplayAdjustment       m_displayAdjustment;

  
  ColorTransformParams() {
    m_iColorSpace = CM_RGB;
    m_iColorPrimaries = CP_709; 
    m_iChromaFormat = CF_444;

    m_oColorSpace = CM_RGB;
    m_oColorPrimaries = CP_709; 
    m_oChromaFormat = CF_444;
    m_transformPrecision = FALSE; 
    m_useHighPrecision = 0;
    m_closedLoopTransform = CLT_NULL; 
    m_iConstantLuminance = 0;
    m_oConstantLuminance = 0; 
    m_transferFunction = TF_NULL;
    m_bitDepth = 10;
    m_range = SR_FULL;
    m_downMethod = 0;
    m_upMethod = 0;
    m_useAdaptiveDownsampler = 0;
    m_useAdaptiveUpsampler = 0;
    m_useMinMax = 0;
    m_maxIterations = 0;
    m_oChromaFormat = CF_444;
    m_oChromaLocationType.resize(FP_TOTAL);
    m_oChromaLocationType[FP_TOP   ] = CL_TWO;
    m_oChromaLocationType[FP_BOTTOM] = CL_TWO;
    m_useFloatPrecision = TRUE;
    m_enableLUTs = TRUE;
    m_enableTFDerivLUTs = TRUE;
    m_min = 0.0f;
    m_max = 1.0f;
    m_iSystemGamma = 1.0f;
    m_oSystemGamma = 1.0f;
    m_minValue     = 0.0f;
    m_maxValue     = 10000.0f;
    
    m_displayAdjustment = DA_NULL;
   }
};

class ColorTransform {
// shared parameters across all functions
// maybe using "protected" variables might not be 
// the best way to go, but somehow this saves us some code.
protected: 

  SampleRange         m_range;
  int                 m_bitDepth;
  bool                m_transformPrecision;

  ChromaFormat        m_iChromaFormat;
  ColorSpace          m_iColorSpace;
  ColorPrimaries      m_iColorPrimaries;
  
  ChromaFormat        m_oChromaFormat;  
  ColorSpace          m_oColorSpace;
  ColorPrimaries      m_oColorPrimaries;
  ChromaLocation      m_oChromaLocation[FP_TOTAL];

  int                 m_useAdaptiveDownsampler;
  int                 m_useAdaptiveUpsampler;

  int                 m_downMethod;
  int                 m_upMethod;
  bool                m_useFloatPrecision;
  TransferFunctions   m_transferFunctions;
  int                 m_maxIterations;
  int                 m_useMinMax;
  
  float               m_iSystemGamma;
  float               m_oSystemGamma;
  int                 m_useHighPrecision;



  void setupParams( ColorTransformParams *params );

private:
  ColorTransformParams m_params;
  
public:
  // Construct/Deconstruct
  virtual ~ColorTransform() {};
  static ColorTransform *create( 
                                ColorSpace        iColorTransform, 
                                ColorPrimaries    iColorPrimaries, 
                                ColorSpace        oColorTransform, 
                                ColorPrimaries    oColorPrimaries, 
                                bool              transformPrecision,
                                int               useHighPrecision,
                                ClosedLoopTrans   closedLoopTransform = CLT_NULL, 
                                int               iConstantLuminance = 0, 
                                int               oConstantLuminance = 0,
                                TransferFunctions transferFunction = TF_PQ,
                                int               bitDepth = 10,
                                SampleRange       range = SR_STANDARD,
                                int               downMethod = 2,
                                int               upMethod = 1,
                                int               useAdaptiveDownsampler = 0,
                                int               useAdaptiveUpsampler = 0,
                                int               useMinMax = 0,
                                int               maxIterations = 20,
                                ChromaFormat      oChromaFormat = CF_420,
                                ChromaLocation    *oChromaLocationType = NULL,
                                bool              useFloatPrecision = TRUE,
                                bool              enableTFLuts = FALSE,
                                ColorTransformParams *iParams = NULL
                                );
  
  virtual void process(Frame *out,  const Frame *inp) = 0;
};

#endif
