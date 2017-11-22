/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2014-2016
 *
 * Copyright (c) 2014-2016, Apple Inc.
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
 * \file TransferFunction.cpp
 *
 * \brief
 *    Base Class for transfer function application
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
#include "TransferFunction.H"
#include "TransferFunctionNull.H"
#include "TransferFunctionPQ.H"
#include "TransferFunctionAPQ.H"
#include "TransferFunctionAPH.H"
#include "TransferFunctionAPQScaled.H"
#include "TransferFunctionMPQ.H"
#include "TransferFunctionAMPQ.H"
#include "TransferFunctionPH.H"
#include "TransferFunctionOHG.H"
#include "TransferFunctionHLG.H"
#include "TransferFunctionNormalize.H"
#include "TransferFunctionPower.H"
#include "TransferFunctionSRGB.H"
#include "TransferFunctionBiasedMPQ.H"
#include "TransferFunctionHPQ.H"
#include "TransferFunctionHPQ2.H"
#include "TransferFunctionAsRGB.H"
#include "TransferFunctionBT709.H"
#include "TransferFunctionST240.H"
#include "TransferFunctionDPXLog.H"
#include "TransferFunctionCineonLog.H"
#ifdef __SIM2_SUPPORT_ENABLED__
#include "TransferFunctionSim2.H"
#endif
#include <time.h>

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

TransferFunction *TransferFunction::create(int method, bool singleStep, float scale, float systemGamma, float minValue, float maxValue, bool enableLUT, bool enableFwdDerivLUT, TransferFunctionParams *params) {
  TransferFunction *result = NULL;
  
  switch (method){
    case TF_NULL:
      result = new TransferFunctionNull();
      break;
    case TF_APH:
      result = new TransferFunctionAPH(minValue, maxValue);
      break;
    case TF_APQ:
      result = new TransferFunctionAPQ(minValue, maxValue);
      break;
    case TF_APQS:
      result = new TransferFunctionAPQScaled(maxValue);
      break;
    case TF_PQ:
      result = new TransferFunctionPQ();
      break;
    case TF_HPQ:
      result = new TransferFunctionHPQ();
      break;
    case TF_HPQ2:
      result = new TransferFunctionHPQ2();
      break;
    case TF_PH:
      result = new TransferFunctionPH();
      break;
    case TF_HG:
      result = new TransferFunctionOHG(systemGamma);
      break;
    case TF_HLG:
      result = new TransferFunctionHLG();
      break;
    case TF_NORMAL:
      result = new TransferFunctionNormalize(scale);
      break;
    case TF_POWER:
      result = new TransferFunctionPower(systemGamma, minValue, maxValue);
      break;
    case TF_MPQ:
      result = new TransferFunctionMPQ(0.1f, 1.5f);
      break;
    case TF_AMPQ:
      result = new TransferFunctionAMPQ(0.1f, 1.5f, minValue, maxValue);
      break;
    case TF_BiasedMPQ:
      result = new TransferFunctionBiasedMPQ(0.1f, 2.0f);
      break;
    case TF_SRGB:
      result = new TransferFunctionSRGB();
      break;
    case TF_ASRGB:
      result = new TransferFunctionAsRGB(minValue, maxValue);
      break;
    case TF_POWERSMP:
      result = new TransferFunctionPower(systemGamma);
      break;
    case TF_BT709:
      result = new TransferFunctionBT709();
      break;
    case TF_DPXLOG:
      result = new TransferFunctionDPXLog();
      break;
    case TF_CINLOG:
      result = new TransferFunctionCineonLog(params);
      break;
    case TF_ST240:
      result = new TransferFunctionST240();
      break;
#ifdef __SIM2_SUPPORT_ENABLED__
    case TF_SIM2:
      result = new TransferFunctionSim2();
      break;
#endif
    default:
      fprintf(stderr, "\nUnsupported Transfer Function %d\n", method);
      exit(EXIT_FAILURE);
  }
  
  if (singleStep == TRUE) {
    result->m_normalFactor = scale;
    result->m_invNormalFactor = 1.0 / scale;
  }
  else {
    result->m_normalFactor = 1.0;
    result->m_invNormalFactor = 1.0;
  }
  
  if (method == TF_NULL) {
    result->m_enableLUT = FALSE;
    result->m_enableFwdDerivLUT = FALSE;
  }
  else {
    result->m_enableLUT = enableLUT;
    result->m_enableFwdDerivLUT = enableFwdDerivLUT;
  }
 
  result->initLUT();
  result->initfwdTFDerivLUT();


  // Unfortunately since we are using virtual functions, we cannot simply use the function pointer
  // There might be a way, but currently the init process is done as a separate step unfortunately
  // Currently this code is disabled.
#if 0
  if (enableLUT == TRUE) {
    // Create memory
    result->m_invLUT = new LookUpTable(enableLUT, MAX_BIN_LUT, MAX_ELEMENTS_LUT);
    result->m_fwdLUT = new LookUpTable(enableLUT, MAX_BIN_LUT, MAX_ELEMENTS_LUT);
    // Now set to appropriate values
    result->initInvTFLUTs(result->m_invLUT);
    result->initFwdTFLUTs(result->m_fwdLUT);
    // We can basically use m_fwdLUT->compute(value)/m_invLUT->compute(value) instead of forwardLUT(value)/inverseLUT(value) to access the luts
  }
  else {
    result->m_invLUT = NULL;
    result->m_fwdLUT = NULL;
  }
   
  if (enableFwdDerivLUT == TRUE) {
    // Create memory
    result->m_fwdDerivLUT = new LookUpTable(enableFwdDerivLUT, MAX_BIN_DERIV_LUT, MAX_ELEMENTS_DERIV_LUT);
    // Now set to appropriate values
    result->initDevTFLUTs(result->m_fwdDerivLUT);
    // We can basically use m_fwdDerivLUT->compute(value) instead of forwardDerivLUT(value) to access the lut
  }
  else {
    result->m_fwdDerivLUT = NULL;
  }
#endif
  
  return result;
}


//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void TransferFunction::initInvTFLUTs(LookUpTable *lut) {
  uint32 i, j;
  for (i = 0; i < m_invLUT->getBins(); i++) {
    double stepSize = (lut->m_bound[i + 1] -  lut->m_bound[i]) / (lut->m_elements[i] - 1);
    for (j = 0; j < lut->m_elements[i] ; j++) {
      double curValue = lut->m_bound[i] + (double) j * stepSize;
      lut->m_data[i][j] = inverse(curValue);
    }
  }
}


void TransferFunction::initFwdTFLUTs(LookUpTable *lut) {
  uint32 i, j;
  for (i = 0; i < lut->getBins(); i++) {
    double stepSize = (lut->m_bound[i + 1] -  lut->m_bound[i]) / (lut->m_elements[i] - 1);
    for (j = 0; j < lut->m_elements[i] ; j++) {
      double curValue = lut->m_bound[i] + (double) j * stepSize;
      lut->m_data[i][j] = forward(curValue);
    }
  }
}

void TransferFunction::initTFLUTs(LookUpTable *lut, double (*method) (double)) {
  uint32 i, j;
  for (i = 0; i < lut->getBins(); i++) {
    double stepSize = (lut->m_bound[i + 1] -  lut->m_bound[i]) / (lut->m_elements[i] - 1);
    for (j = 0; j < lut->m_elements[i] ; j++) {
      double curValue = lut->m_bound[i] + (double) j * stepSize;
      lut->m_data[i][j] = method(curValue);
    }
  }
}

void TransferFunction::initDevTFLUTs(LookUpTable *lut) {
  uint32 i, j;
  for (i = 0; i < lut->getBins(); i++) {
    double stepSize = (lut->m_bound[i + 1] -  lut->m_bound[i]) / (lut->m_elements[i] - 1);
    for (j = 0; j < lut->m_elements[i] ; j++) {
      double curValue = lut->m_bound[i] + (double) j * stepSize;
      lut->m_data[i][j] = forwardDerivative(curValue);
    }
  }
}

void TransferFunction::initLUT() {
  if (m_enableLUT == FALSE) {
    m_binsLUT = 0;
  }
  else {
    printf("Initializing LUTs for TF computations\n");
    uint32 i, j;
    m_binsLUT = MAX_BIN_LUT;
    m_binsLUTExt = m_binsLUT + 2;

    m_elementsLUT.resize    (m_binsLUTExt);
    m_multiplierLUT.resize  (m_binsLUTExt);
    m_boundLUT.resize       (m_binsLUTExt + 1);
    m_invTransformLUT.resize(m_binsLUTExt);
    m_fwdTransformLUT.resize(m_binsLUTExt);
    m_boundLUT[0] = 0.0;

    for (i = 0; i < m_binsLUTExt; i++) {
      m_elementsLUT[i] = MAX_ELEMENTS_LUT; // Size of each bin.
                                // Could be different for each bin, but for now lets set this to be the same.
      int index = m_binsLUT - i - 1;
      if (index > 0)
        m_boundLUT[i + 1] = 1 / pow( 10.0 , (double) (index)); // upper bin boundary
      else
        m_boundLUT[i + 1] = pow( 10.0 , (double) (-index)); // upper bin boundary
      
      double stepSize = (m_boundLUT[i + 1] -  m_boundLUT[i]) / (m_elementsLUT[i] - 1);
      m_multiplierLUT[i] = (double) (m_elementsLUT[i] - 1) / (m_boundLUT[i + 1] -  m_boundLUT[i]);

      // Now allocate memory given the specified size
      m_invTransformLUT[i].resize(m_elementsLUT[i]);
      m_fwdTransformLUT[i].resize(m_elementsLUT[i]);
      for (j = 0; j < m_elementsLUT[i] ; j++) {
        double curValue = m_boundLUT[i] + (double) j * stepSize;
        m_invTransformLUT[i][j] = inverse(curValue);
        m_fwdTransformLUT[i][j] = forward(curValue);
      }
    }
    m_maxFwdLUT = m_boundLUT[m_binsLUTExt];
    m_maxInvLUT = m_boundLUT[m_binsLUTExt];
  }
}

void TransferFunction::initfwdTFDerivLUT() {
  if (m_enableFwdDerivLUT == FALSE) {
    m_derivBinsLUT = 0;
  }
  else {
    printf("Initializing TF derivative LUTs\n");
    //bool enableLUT = m_enableLUT;
    //m_enableLUT = FALSE;  // To initialize forward derivative using real TF calculations
                            // the above does not really seem necessary. Lets use what we have.
    uint32 i, j;
    m_derivBinsLUT = MAX_BIN_DERIV_LUT;
    m_derivBinsLUTExt = m_derivBinsLUT + 2;
    m_derivElementsLUT.resize    (m_derivBinsLUTExt);
    m_derivMultiplierLUT.resize  (m_derivBinsLUTExt);
    m_derivBoundLUT.resize       (m_derivBinsLUTExt + 1);
    m_fwdTFDerivativeLUT.resize  (m_derivBinsLUTExt);
    
    m_derivBoundLUT[0] = 0.0;
    for (i = 0; i < m_derivBinsLUTExt; i++) {
      m_derivElementsLUT[i] = MAX_ELEMENTS_DERIV_LUT; // Size of each bin.
                                                      // Could be different for each bin, but for now lets set this to be the same.
      //m_derivBoundLUT[i + 1] = 1 / pow( 10.0 , (double) (m_derivBinsLUT - i - 1)); // upper bin boundary
      int index = m_derivBinsLUT - i - 1;
      if (index > 0)
        m_derivBoundLUT[i + 1] = 1 / pow( 10.0 , (double) (index)); // upper bin boundary
      else
        m_derivBoundLUT[i + 1] = pow( 10.0 , (double) (-index)); // upper bin boundary

      double stepSize = (m_derivBoundLUT[i + 1] -  m_derivBoundLUT[i]) / (m_derivElementsLUT[i] - 1);
      m_derivMultiplierLUT[i] = (double) (m_derivElementsLUT[i] - 1) / (m_derivBoundLUT[i + 1] -  m_derivBoundLUT[i]);
      
      // Allocate also memory for the derivative LUT
      m_fwdTFDerivativeLUT[i].resize(m_derivElementsLUT[i]);
      for (j = 0; j < m_derivElementsLUT[i] ; j++) {
        double curValue = m_derivBoundLUT[i] + (double) j * stepSize;
        m_fwdTFDerivativeLUT[i][j] = forwardDerivative(curValue);
      }
    }
    m_maxFwdTFDerLUT = m_derivBoundLUT[m_derivBinsLUTExt];

    //m_enableLUT = enableLUT;
  }
}


double TransferFunction::inverseLUT(double value) {
  if (value <= 0.0)
    return m_invTransformLUT[0][0];
  else if (value >= m_maxInvLUT) {
    // top value, most likely 1.0
    return m_invTransformLUT[m_binsLUTExt - 1][m_elementsLUT[m_binsLUTExt - 1] - 1];
  }
  else { // now search for value in the table
    for (uint32 i = 0; i < m_binsLUTExt; i++) {
      if (value < m_boundLUT[i + 1]) { // value located
        double satValue = (value - m_boundLUT[i]) * m_multiplierLUT[i];
        //return (m_invTransformMap[(int) dRound(satValue)]);
        int    valuePlus     = (int) dCeil(satValue) ;
        double distancePlus  = (double) valuePlus - satValue;
        return (m_invTransformLUT[i][valuePlus - 1] * distancePlus + m_invTransformLUT[i][valuePlus] * (1.0 - distancePlus));
      }
    }
  }
  return 0.0;
}

double TransferFunction::forwardLUT(double value) {
  if (value <= 0.0)
    return m_fwdTransformLUT[0][0];
  else if (value >= m_maxFwdLUT ) {
    // top value, most likely 1.0
    return m_fwdTransformLUT[m_binsLUTExt - 1][m_elementsLUT[m_binsLUTExt - 1] - 1];
  }
  else {
    // now search for value in the table
    for (uint32 i = 0; i < m_binsLUTExt; i++) {
      if (value < m_boundLUT[i + 1]) { // value located
        double satValue     = (value - m_boundLUT[i]) * m_multiplierLUT[i];
        int    valuePlus    = (int) dCeil(satValue) ;
        double distancePlus = (double) valuePlus - satValue;
        return (m_fwdTransformLUT[i][valuePlus - 1] * distancePlus + m_fwdTransformLUT[i][valuePlus] * (1.0 - distancePlus));
      }
    }
  }
  return 0.0;
}

double TransferFunction::forwardDerivLUT(double value) {
  if (value <= 0.0)
    return m_fwdTFDerivativeLUT[0][0];
  else if (value >= m_maxFwdTFDerLUT ) {
    // top value, most likely 1.0
    return m_fwdTFDerivativeLUT[m_derivBinsLUT - 1][m_derivElementsLUT[m_derivBinsLUT - 1] - 1];
  }
  else {
    // now search for value in the table
    for (uint32 i = 0; i < m_derivBinsLUT; i++) {
      if (value < m_derivBoundLUT[i + 1]) { // value located
        double satValue     = (value - m_derivBoundLUT[i]) * m_derivMultiplierLUT[i];
        int    valuePlus    = (int) dCeil(satValue) ;
        double distancePlus = (double) valuePlus - satValue;
        return (m_fwdTFDerivativeLUT[i][valuePlus - 1] * distancePlus + m_fwdTFDerivativeLUT[i][valuePlus] * (1.0 - distancePlus));
      }
    }
  }
  return 0.0;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


double TransferFunction::forwardDerivative(double value) {
  double low  = value - DERIV_STEP;
  double high = value + DERIV_STEP;
  low  = ( low  > DERIV_LOWER_BOUND  ) ? low  : DERIV_LOWER_BOUND;
  high = ( high < DERIV_HIGHER_BOUND ) ? high : DERIV_HIGHER_BOUND;
  
  return (getForward(high) - getForward(low)) / (high - low) ;
}

double TransferFunction::inverseDerivative(double value) {
  double low  = value - DERIV_STEP;
  double high = value + DERIV_STEP;
  low  = ( low  > DERIV_LOWER_BOUND  ) ? low  : DERIV_LOWER_BOUND;
  high = ( high < DERIV_HIGHER_BOUND ) ? high : DERIV_HIGHER_BOUND;
  
  return (getInverse(high) - getInverse(low)) / (high - low) ;
}

double TransferFunction::getForwardDerivative(double value) {
  if (m_enableFwdDerivLUT == TRUE)
    return forwardDerivLUT(value);
  else
    return forwardDerivative(value);
}


double TransferFunction::getForward(double value) {
  if (m_enableLUT == FALSE)
    return forward(value);
  else 
    return forwardLUT(value);
}

double TransferFunction::getInverse(double value) {
  if (m_enableLUT == FALSE)
    return inverse(value);
  else 
    return inverseLUT(value);
}


void TransferFunction::forward( Frame* frame, int component ) {
  if (frame->m_isFloat == TRUE) {
    if (m_normalFactor == 1.0) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) forward((double) frame->m_floatComp[component][i]);
        }
      }
      else {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) forwardLUT((double) frame->m_floatComp[component][i]);
        }
      }
    }
    else {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) (m_normalFactor * forward((double) frame->m_floatComp[component][i]));
        }
      }
      else {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) (m_normalFactor * forwardLUT((double) frame->m_floatComp[component][i]));
        }
      }
    }
  }
}

void TransferFunction::forward( Frame* frame ) {

  if (frame->m_isFloat) {
    if (m_normalFactor == 1.0) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) forward((double) frame->m_floatData[i]);
        }
      }
      else {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) forwardLUT((double) frame->m_floatData[i]);
        }
      }
    }
    else {
    if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) (m_normalFactor * forward((double) frame->m_floatData[i]));
        }
      }
      else {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) (m_normalFactor * forwardLUT((double) frame->m_floatData[i]));
        }
      }
    }
  }
}

void TransferFunction::inverse( Frame* frame, int component ) {
  if (frame->m_isFloat == TRUE) {
    if (m_normalFactor == 1.0) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) inverse((double) frame->m_floatComp[component][i]);
        }
      }
      else {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) inverseLUT((double) frame->m_floatComp[component][i]);
        }
      }
    }
    else {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) inverse((double) frame->m_floatComp[component][i] / m_normalFactor);
        }
      }
      else {
        for (int i = 0; i < frame->m_compSize[component]; i++) {
          frame->m_floatComp[component][i] = (float) inverseLUT((double) frame->m_floatComp[component][i] / m_normalFactor);
        }
      }
    }
  }
}

void TransferFunction::inverse( Frame* frame ) {
  if (frame->m_isFloat) {
    if (m_normalFactor == 1.0) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) inverse((double) frame->m_floatData[i]);
        }
      }
      else {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) inverseLUT((double) frame->m_floatData[i]);
        }
      }
    }
    else {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) inverse((double) frame->m_floatData[i] / m_normalFactor);
        }
      }
      else {
        for (int i = 0; i < frame->m_size; i++) {
          frame->m_floatData[i] = (float) inverseLUT((double) frame->m_floatData[i] / m_normalFactor);
        }
      }
    }
  }
}


void TransferFunction::forward( Frame* out, const Frame *inp, int component ) {
  // In this scenario, we should likely copy the frame number externally
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      for (int i = 0; i < inp->m_compSize[component]; i++) {
        out->m_floatComp[component][i] = (float) forward((double) inp->m_floatComp[component][i]);
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp, component);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      for (int i = 0; i < inp->m_compSize[component]; i++) {
        out->m_floatComp[component][i] = (float) (m_normalFactor * forward((double) inp->m_floatComp[component][i]));
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      
      out->copy((Frame *) inp, component);
    }
  }
}

void TransferFunction::forward( Frame* out, const Frame *inp ) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < inp->m_size; i++) {
          out->m_floatData[i] = (float) forward((double) inp->m_floatData[i]);
        }
      }
      else {
        for (int i = 0; i < inp->m_size; i++) {
          out->m_floatData[i] = (float) forwardLUT((double) inp->m_floatData[i]);
        }
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < inp->m_size; i++) {
          // ideally, we should remove the double cast. However, we are currently keeping compatibility with the old code
          out->m_floatData[i] = (float) (m_normalFactor * (double) ((float) forward((double) inp->m_floatData[i])));
        }
      }
      else {
        for (int i = 0; i < inp->m_size; i++) {
          out->m_floatData[i] = (float) (m_normalFactor * (double) forwardLUT((double) inp->m_floatData[i]));
        }
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
}

void TransferFunction::inverse( Frame* out, const Frame *inp, int component ) {
  // In this scenario, we should likely copy the frame number externally
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      for (int i = 0; i < inp->m_compSize[component]; i++) {
        out->m_floatComp[component][i] = (float) inverse((double) inp->m_floatComp[component][i]);
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp, component);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      for (int i = 0; i < inp->m_compSize[component]; i++) {
        out->m_floatComp[component][i] = (float) inverse((double) inp->m_floatComp[component][i] / m_normalFactor);
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp, component);
    }
  }
}

void TransferFunction::inverse( Frame* out, const Frame *inp ) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < inp->m_size; i++) {
          out->m_floatData[i] = (float) inverse((double) inp->m_floatData[i]);
        }
      }
      else {
        for (int i = 0; i < inp->m_size; i++) {
          out->m_floatData[i] = (float) inverseLUT((double) inp->m_floatData[i]);
        }
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      if (m_enableLUT == FALSE) {
        for (int i = 0; i < inp->m_size; i++) {
          out->m_floatData[i] = (float) inverse((double) inp->m_floatData[i] / m_normalFactor);
        }
      }
      else {
//KW_KYH inverse EOTF LUT
        for (int i = 0; i < inp->m_size; i++) {
          out->m_floatData[i] = (float) inverseLUT((double) inp->m_floatData[i] / m_normalFactor);
        }
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
