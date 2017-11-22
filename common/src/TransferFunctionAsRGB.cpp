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
 * \file TransferFunctionAsRGB.cpp
 *
 * \brief
 *    TransferFunctionAsRGB Class (Adaptive PQ)
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
#include "TransferFunctionAsRGB.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

TransferFunctionAsRGB::TransferFunctionAsRGB(double minValue, double maxValue) {
  
  m_gamma = 2.4;
  m_inverseGamma = 1.0 / m_gamma;
  m_alpha = 0.055;
  m_beta =  0.0031308;
  m_invBeta = inverseSRGB(m_beta);

  m_normalFactor = 1.0;

  m_minValue = minValue / 100.0;
  m_maxValue = maxValue / 100.0;
  m_minValueInv = inverseSRGB(m_minValue);
  m_maxValueInv = inverseSRGB(m_maxValue);
  
  m_denomInv  = m_maxValueInv - m_minValueInv;
}

TransferFunctionAsRGB::~TransferFunctionAsRGB() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
double TransferFunctionAsRGB::forwardSRGB(double value) {
  if (value <= m_invBeta)
    return (value / 12.92);
  else
  return (pow(dMax((value + m_alpha)/ (1.0 + m_alpha), 0.0), m_gamma));
}

double TransferFunctionAsRGB::inverseSRGB(double value) {
  if (value <= m_beta)
    return (12.92 * value);
  else
    return ((1.0 + m_alpha) * pow(value, m_inverseGamma) - m_alpha);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

double TransferFunctionAsRGB::forward(double value) {
  return (float) forwardSRGB(value * m_denomInv + m_minValueInv);
}

double TransferFunctionAsRGB::inverse(double value) {
  //printf("value %10.7f %10.7f %10.7f\n", value, (inverseSRGB(value) - m_minValueInv) / m_denomInv, (float) dMax(0.0, dMin(1.0, (inverseSRGB(value) - m_minValueInv) / m_denomInv)));
  return (float) dMax(0.0, dMin(1.0, (inverseSRGB(value) - m_minValueInv) / m_denomInv));
}





//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
