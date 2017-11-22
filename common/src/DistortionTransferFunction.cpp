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
 * \file DistortionTransferFunction.cpp
 *
 * \brief
 *    Distortion Transfer Function Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionTransferFunction.H"
//#include "ColorTransformGeneric.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionTransferFunction *DistortionTransferFunction::create(DistortionFunction method, bool enableLUT) {
  DistortionTransferFunction *result = NULL;
  
  switch (method){
    default:
    case DIF_PQPH10K:
      result = new DistortionTransferFunctionCombo();
      break;
    case DIF_PQ:
      result = new DistortionTransferFunctionPQ();
      break;
    case DIF_DE2K:
      result = new DistortionTransferFunctionDE();
      break;
    case DIF_PQNS:
      result = new DistortionTransferFunctionPQNoise();
      break;
    case DIF_NULL:
      result = new DistortionTransferFunctionNull();
      break;
    case DIF_HPQ:
      result = new DistortionTransferFunctionHPQ();
      break;
    case DIF_HPQ2:
      result = new DistortionTransferFunctionHPQ2();
      break;
  }
  result->m_enableLUT = enableLUT;
    
  result->initLUT();

  return result;
}


void DistortionTransferFunction::initLUT() {  
  if (m_enableLUT == FALSE) {
    m_binsLUT = 0;
  }
  else {
    printf("Initializing LUTs for TF metric computations\n");
    uint32 i, j;
    m_binsLUT = 10;
    m_elementsLUT.resize   (m_binsLUT);
    m_multiplierLUT.resize (m_binsLUT);
    m_boundLUT.resize      (m_binsLUT + 1);
    m_computeLUT.resize    (m_binsLUT);
    m_boundLUT[0] = 0.0;
    for (i = 0; i < m_binsLUT; i++) {
      m_elementsLUT[i] = 10000; // Size of each bin. 
                                // Could be different for each bin, but for now lets set this to be the same.
      m_boundLUT[i + 1] = 1 / pow( 10.0 , (double) (m_binsLUT - i - 1)); // upper bin boundary
      double stepSize = (m_boundLUT[i + 1] -  m_boundLUT[i]) / (m_elementsLUT[i] - 1);
      m_multiplierLUT[i] = (double) (m_elementsLUT[i] - 1) / (m_boundLUT[i + 1] -  m_boundLUT[i]);
      
      // Now allocate memory given the specified size
      m_computeLUT[i].resize(m_elementsLUT[i]);
      for (j = 0; j < m_elementsLUT[i] ; j++) {
        double curValue = m_boundLUT[i] + (double) j * stepSize;
        m_computeLUT[i][j] = compute(curValue);
      }
    }
  }
}

double DistortionTransferFunction::computeNull(double value) {
  return (value);
}

double DistortionTransferFunction::computeDE(double value) {
  value *= 100.0;
  return ((value >= 0.008856) ? pow(value, 1.0 / 3.0) : (7.78704 * value + 0.137931)) / pow(100.0, 1.0/3.0);
}

double DistortionTransferFunction::computePQ(double value) {
  static const double m1 = 2610.0 / 16384.0;
  static const double m2 = 2523.0 /    32.0;
  static const double c1 = 3424.0 /  4096.0;
  static const double c2 = 2413.0 /   128.0;
  static const double c3 = 2392.0 /   128.0;
  
  // value = dClip(value, 0.0, 1.0);
  double tempValue = pow(value, m1);
  return (pow(((c2 * (tempValue) + c1)/(1.0 + c3 * (tempValue))), m2));
}

double DistortionTransferFunction::computeHPQ(double value) {
  static const double m1 = 2610.0 / 16384.0;
  static const double m2 = 2523.0 /    32.0;
  static const double c1 = 3424.0 /  4096.0;
  static const double c2 = 2413.0 /   128.0;
  static const double c3 = 2392.0 /   128.0;
  
  static const double linear = 4.5;
  static const double weight = 1.099;
  static const double offset = 0.099;
  static const double iGamma = 0.45;
  //static const double gamma = 1.0 / 0.45;
  static const double value0 = 0.00018;
  static const double value1 = 0.01;
  static const double scale = computePQ(value1);
  
  //printf(" Value %10.8f %10.8f\n", scale, value);
  //value = dClip(value, 0.0, 1.0);
  if (value < value0) {
    return ((linear * value * 100.0) * scale);
  }
  else if (value < value1) {
    //return ((pow(value * 100.0, iGamma)) * scale);
    return ((weight * pow(value * 100.0, iGamma) - offset) * scale);
  }
  else {
    double tempValue = pow(value, m1);
    return (pow(((c2 * (tempValue) + c1)/(1.0 + c3 * (tempValue))), m2));
  }
}

double DistortionTransferFunction::computeHPQ2(double value) {
  static const double m1 = 2610.0 / 16384.0;
  static const double m2 = 2523.0 /    32.0;
  static const double c1 = 3424.0 /  4096.0;
  static const double c2 = 2413.0 /   128.0;
  static const double c3 = 2392.0 /   128.0;
  
  static const double iGamma = 1.0 / 2.4;
  static const double scale = computePQ(0.01);
  
  //printf(" Value %10.8f %10.8f\n", scale, value);
  //value = dClip(value, 0.0, 1.0);
  if (value < 0.01) {
    return ((pow(value * 100.0, iGamma)) * scale);
  }
  else {
    double tempValue = pow(value, m1);
    return (pow(((c2 * (tempValue) + c1)/(1.0 + c3 * (tempValue))), m2));
  }
}

double DistortionTransferFunction::computePQNoise(double value) {  
  static const double minLimit = 0.0001;
  if (value < minLimit) // use a 
    return (4.5 * value);
  else {
    static const double pqLimit = computePQ(minLimit);
    static const double linLimit = 4.5 * minLimit;
    double tempValue = (((computePQ(value) - pqLimit) / ( 1.0 - pqLimit)) * (1.0 - linLimit)) + linLimit;
    return (tempValue);
  }
}

double DistortionTransferFunction::computePH10K(double value) {
  static const double rho   = 25.0;
  static const double invGamma = 1 / 2.4;
  static const double ratio = 2.0; // Philips EOTF was created with a max of 5K cd/m^2. With a ratio of 2 we now extend this to 10K.
  static const double maxValue = (log(1.0 + (rho - 1.0) * pow(ratio, invGamma)) / log(rho));
  
  return (log(1.0 + (rho - 1.0) * pow(ratio * value, invGamma)) / (log(rho) * maxValue));
}

double DistortionTransferFunction::computeLUT(double value) {
  if (value <= 0.0)
    return m_computeLUT[0][0];
  else if (value >= 1.0) {
    // top value, most likely 1.0
    return m_computeLUT[m_binsLUT - 1][m_elementsLUT[m_binsLUT - 1] - 1];
  }
  else { // now search for value in the table
    for (uint32 i = 0; i < m_binsLUT; i++) {
      if (value < m_boundLUT[i + 1]) { // value located
        double satValue = (value - m_boundLUT[i]) * m_multiplierLUT[i];
        int    valuePlus     = (int) dCeil(satValue) ;
        double distancePlus  = (double) valuePlus - satValue;
        return (m_computeLUT[i][valuePlus - 1] * distancePlus + m_computeLUT[i][valuePlus] * (1.0 - distancePlus));
      }
    }
  }
  return 0.0;
}

double DistortionTransferFunctionNull::compute(double value) {
  
  double clippedValue = dMax(0.0, dMin(value, 1.0));
  
  return computeNull(clippedValue);
}


double DistortionTransferFunctionDE::compute(double value) {
  
  double clippedValue = dMax(0.0, dMin(value, 1.0));
  
  return computeDE(clippedValue);
}


double DistortionTransferFunctionPQ::compute(double value) {
  
  double clippedValue = dMax(0.0, dMin(value, 1.0));
  
  return computePQ(clippedValue);
}

double DistortionTransferFunctionHPQ::compute(double value) {
  
  double clippedValue = dMax(0.0, dMin(value, 1.0));
  
  return computeHPQ(clippedValue);
}

double DistortionTransferFunctionHPQ2::compute(double value) {
  
  double clippedValue = dMax(0.0, dMin(value, 1.0));
  
  return computeHPQ2(clippedValue);
}


double DistortionTransferFunctionPQNoise::compute(double value) {
  
  double clippedValue = dMax(0.0, dMin(value, 1.0));
  
  return computePQNoise(clippedValue);
}

double DistortionTransferFunctionCombo::compute(double value) {
  double clippedValue = dMax(0.0, dMin(value, 1.0));
  
  return (computePQ(clippedValue) + computePH10K(clippedValue)) * 0.5;
}

//-----------------------------------------------------------------------------
// Public
//-----------------------------------------------------------------------------

double DistortionTransferFunction::performCompute(double value) {
  if (m_enableLUT == TRUE)
    return computeLUT(value);
  else
    return compute(value);
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
