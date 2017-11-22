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
 * \file ColorTransform.cpp
 *
 * \brief
 *    Base Class for color transform application
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "ColorTransform.H"
#include "ColorTransformNull.H"
#include "ColorTransformXYZ2YUpVp.H"
#include "ColorTransformGeneric.H"
#include "ColorTransformClosedLoop.H"
#include "ColorTransformClosedLoopY.H"
#include "ColorTransformClosedLoopCr.H"
#include "ColorTransformClosedLoopRGB.H"
#include "ColorTransformClosedLoopFRGB.H"
#include "ColorTransformYAdjust.H"
#include "ColorTransformYAdjustAlt.H"
#include "ColorTransformYAdjustFast.H"
#include "ColorTransformYAdjustFull.H"
#include "ColorTransformYAdjustHLG.H"
#include "ColorTransformYAdjustLFast.H"
#include "ColorTransformYAdjustTele.H"
#include "ColorTransformYAdjustXYZ.H"
#include "ColorTransformYInter.H"
#include "ColorTransformYMultiply.H"
#include "ColorTransformYPlus.H"
#include "ColorTransformYLuma.H"
#include "ColorTransformYLin.H"
#include "ColorTransformYSumLin.H"
#include "ColorTransformYAdjust2ndOrder.H"
#include "ColorTransformFVDO.H"
#include "ColorTransformCL.H"

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Protected methods
//-----------------------------------------------------------------------------

void ColorTransform::setupParams( ColorTransformParams *params ){
  
  m_range                      = params->m_range;
  m_bitDepth                   = params->m_bitDepth;
  m_transferFunctions          = params->m_transferFunction;
  m_useMinMax                  = params->m_useMinMax;
  
  m_useHighPrecision           = params->m_useHighPrecision;
  //m_size = 0;
  m_maxIterations              = params->m_maxIterations;
  //m_tfDistance = TRUE;
  m_useFloatPrecision          = params->m_useFloatPrecision;
  
  m_iChromaFormat              = params->m_iChromaFormat;
  m_iColorSpace                = params->m_iColorSpace;
  m_iColorPrimaries            = params->m_iColorPrimaries;
  
  m_oChromaFormat              = params->m_oChromaFormat;
  m_oColorSpace                = params->m_oColorSpace;
  m_oColorPrimaries            = params->m_oColorPrimaries;
  m_oChromaLocation[FP_TOP   ] = params->m_oChromaLocationType[0];
  m_oChromaLocation[FP_BOTTOM] = params->m_oChromaLocationType[1];
  m_useAdaptiveDownsampler     = params->m_useAdaptiveDownsampler;
  m_useAdaptiveUpsampler       = params->m_useAdaptiveUpsampler;
  
  m_downMethod                 = params->m_downMethod;
  m_upMethod                   = params->m_upMethod;
  
  m_iSystemGamma               = params->m_iSystemGamma;
  m_oSystemGamma               = params->m_oSystemGamma;
  m_transformPrecision         = params->m_transformPrecision;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
ColorTransform *ColorTransform::create( 
                                       ColorSpace            iColorSpace, 
                                       ColorPrimaries        iColorPrimaries, 
                                       ColorSpace            oColorSpace, 
                                       ColorPrimaries        oColorPrimaries, 
                                       bool                  transformPrecision, 
                                       int                   useHighPrecision,
                                       ClosedLoopTrans       closedLoopTransform, 
                                       int                   iConstantLuminance, 
                                       int                   oConstantLuminance, 
                                       TransferFunctions     transferFunction,
                                       int                   bitDepth,
                                       SampleRange           range,
                                       int                   downMethod,
                                       int                   upMethod,
                                       int                   useAdaptiveDownsampler,
                                       int                   useAdaptiveUpsampler,
                                       int                   useMinMax,
                                       int                   maxIterations,
                                       ChromaFormat          oChromaFormat,
                                       ChromaLocation        *oChromaLocationType,
                                       bool                  useFloatPrecision, 
                                       bool                  enableTFLuts,
                                       ColorTransformParams *iParams
                                       ) {
  ColorTransformParams params;
  ColorTransform *result = NULL;
  if (iParams != NULL)
    params = *iParams;
  else
    params.m_enableTFDerivLUTs = enableTFLuts;

  params.m_iColorSpace = iColorSpace;
  params.m_oColorSpace = oColorSpace;
  params.m_iColorPrimaries = iColorPrimaries; 
  params.m_oColorPrimaries = oColorPrimaries; 
  params.m_transformPrecision = transformPrecision; 
  params.m_useHighPrecision = useHighPrecision;
  params.m_closedLoopTransform = closedLoopTransform; 
  params.m_iConstantLuminance = iConstantLuminance;
  params.m_oConstantLuminance = oConstantLuminance; 
  params.m_transferFunction = transferFunction;
  params.m_bitDepth = bitDepth;
  params.m_range = range;
  params.m_downMethod = downMethod;
  params.m_upMethod = upMethod;
  params.m_useAdaptiveDownsampler = useAdaptiveDownsampler;
  params.m_useAdaptiveUpsampler = useAdaptiveUpsampler;
  params.m_useMinMax = useMinMax;
  params.m_maxIterations = maxIterations;
  params.m_oChromaFormat = oChromaFormat;
  params.m_useFloatPrecision = useFloatPrecision;
  params.m_enableLUTs = enableTFLuts;
    
  if (oChromaLocationType != NULL) {
    params.m_oChromaLocationType[FP_TOP   ] = oChromaLocationType[FP_TOP   ];
    params.m_oChromaLocationType[FP_BOTTOM] = oChromaLocationType[FP_BOTTOM];
  }
  
  if ((iColorSpace == oColorSpace) && (iColorPrimaries == oColorPrimaries) && ((iConstantLuminance == oConstantLuminance) || (iColorSpace != CM_YCbCr && iColorSpace != CM_ICtCp)))
    result = new ColorTransformNull();
  else if ((iColorSpace == CM_XYZ && oColorSpace == CM_YUpVp) || (iColorSpace == CM_YUpVp && oColorSpace == CM_XYZ)) 
    result = new ColorTransformXYZ2YUpVp();
  else if ((iColorSpace >= CM_YFBFRV1 && iColorSpace <= CM_YFBFRV2) || (oColorSpace >= CM_YFBFRV1 && oColorSpace <= CM_YFBFRV2))
    result = new ColorTransformFVDO(iColorSpace, oColorSpace);
  else if ((iColorPrimaries == oColorPrimaries) && 
           (((oColorSpace == CM_YCbCr) && (oConstantLuminance == 1) && (iColorSpace == CM_RGB)) ||
            ((iColorSpace == CM_YCbCr) && (iConstantLuminance == 1) && (oColorSpace == CM_RGB)))){
             result = new ColorTransformCL( &params, 0);
           }
  else if ((iColorPrimaries == oColorPrimaries) && 
           (((oColorSpace == CM_YCbCr) && (oConstantLuminance == 2) && (iColorSpace == CM_RGB)) ||
            ((iColorSpace == CM_YCbCr) && (iConstantLuminance == 2) && (oColorSpace == CM_RGB)))){
             result = new ColorTransformCL(&params, 1);
           }
  else if ((iColorPrimaries == oColorPrimaries) && 
           (((oColorSpace == CM_YCbCr) && (oConstantLuminance == 3) && (iColorSpace == CM_RGB)) ||
            ((iColorSpace == CM_YCbCr) && (iConstantLuminance == 3) && (oColorSpace == CM_RGB)))){
             result = new ColorTransformCL(&params, 2);
           }
  else if (closedLoopTransform == CLT_NULL) {
    result = new ColorTransformGeneric( &params );
  }
  else if (closedLoopTransform == CLT_YADJ)
    result = new ColorTransformYAdjust( &params );
  else if (closedLoopTransform == CLT_YADJFULL)
    result = new ColorTransformYAdjustFull( &params );
  else if (closedLoopTransform == CLT_YADJALT)
    result = new ColorTransformYAdjustAlt( &params );
  else if (closedLoopTransform == CLT_CHROMA)
    result = new ColorTransformClosedLoopCr( &params );
  else if (closedLoopTransform == CLT_RGB)
    result = new ColorTransformClosedLoopRGB( &params );
  else if (closedLoopTransform == CLT_FRGB)
    result = new ColorTransformClosedLoopFRGB( &params );
  else if (closedLoopTransform == CLT_Y)
    result = new ColorTransformClosedLoopY( &params );
  else if (closedLoopTransform == CLT_XYZ)
    result = new ColorTransformYAdjustXYZ( &params );
  else if (closedLoopTransform == CLT_YADJFST)
    result = new ColorTransformYAdjustFast(&params);
  else if (closedLoopTransform == CLT_YADJLFST)
    result = new ColorTransformYAdjustLFast( &params );
  else if (closedLoopTransform == CLT_YADJTELE)
    result = new ColorTransformYAdjustTele( &params );
  else if (closedLoopTransform == CLT_YLIN)
    result = new ColorTransformYLin( &params );
  else if (closedLoopTransform == CLT_YSUMLIN)
    result = new ColorTransformYSumLin( &params );
  else if (closedLoopTransform == CLT_YADJ2ORD)
    result = new ColorTransformYAdjust2ndOrder( &params );
  else if (closedLoopTransform == CLT_YADJHLG)
    result = new ColorTransformYAdjustHLG( &params );
  else if (closedLoopTransform == CLT_YAPPLEI)
    result = new ColorTransformYInter( &params );
  else if (closedLoopTransform == CLT_YAPPLEM)
    result = new ColorTransformYMultiply( &params );
  else if (closedLoopTransform == CLT_YAPPLEP)
    result = new ColorTransformYPlus( &params );
  else if (closedLoopTransform == CLT_YAPPLEL)
    result = new ColorTransformYLuma( &params );
  else
    result = new ColorTransformClosedLoop( &params );
  
  return result;
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
