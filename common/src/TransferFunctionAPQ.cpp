/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, Apple Inc.
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
 * \file TransferFunctionAPQ.cpp
 *
 * \brief
 *    TransferFunctionAPQ Class (Adaptive PQ)
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
#include "TransferFunctionAPQ.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

TransferFunctionAPQ::TransferFunctionAPQ(double minValue, double maxValue) {
  
  m1 = (2610.0        ) / (4096 * 4);
  m2 = (2523.0 * 128.0) / 4096;
  c1 = (3424.0        ) / 4096;
  c2 = (2413.0 *  32.0) / 4096;
  c3 = (2392.0 *  32.0) / 4096;
  
  m_normalFactor = 1.0;

  m_minValue = minValue / 10000.0;
  m_maxValue = maxValue / 10000.0;
  m_minValueInv = inversePQ(m_minValue);
  m_maxValueInv = inversePQ(m_maxValue);
  
  m_denomInv  = m_maxValueInv - m_minValueInv;
}

TransferFunctionAPQ::~TransferFunctionAPQ() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
double TransferFunctionAPQ::forwardPQ(double value) {
  double tempValue = pow((double) value, (1.0 / m2));
  return (float) (pow(dMax(0.0, (tempValue - c1))/(c2 - c3 * tempValue),(1.0 / m1)));
}

double TransferFunctionAPQ::inversePQ(double value) {
  double  tempValue = pow((double) value, m1);
  return (float) (pow(((c2 *(tempValue) + c1)/(1 + c3 *(tempValue))), m2));
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

double TransferFunctionAPQ::forward(double value) {
  return (float) forwardPQ(value * m_denomInv + m_minValueInv);
}

double TransferFunctionAPQ::inverse(double value) {
  return (float) dMax(0.0, dMin(1.0, (inversePQ(value) - m_minValueInv) / m_denomInv));
}





//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
