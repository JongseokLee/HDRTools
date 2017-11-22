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
 * \file TransferFunctionMPQ.cpp
 *
 * \brief
 *    TransferFunctionMPQ Class
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
#include "TransferFunctionMPQ.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

TransferFunctionMPQ::TransferFunctionMPQ(float inflectionPoint, float scale) {
  m1 = (2610.0        ) / (4096.0 * 4.0);
  m2 = (2523.0 * 128.0) / 4096.0;
  c1 = (3424.0        ) / 4096.0;
  c2 = (2413.0 *  32.0) / 4096.0;
  c3 = (2392.0 *  32.0) / 4096.0;
  
  m_normalFactor        = 1.0;
  m_invNormalFactor     = 1.0;
  m_inflectionPoint     = (double) inflectionPoint;
  m_invInflectionPoint  = inversePQ(m_inflectionPoint);
  m_scale               = scale;
  m_invInflectionPointDiv2 = m_invInflectionPoint / m_scale;
  
}

TransferFunctionMPQ::~TransferFunctionMPQ() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
double TransferFunctionMPQ::forwardPQ(double value) {
  double tempValue = pow(value, (1.0 / m2));
  return (pow(dMax(0.0, (tempValue - c1))/(c2 - c3 * tempValue),(1.0 / m1)));
}

double TransferFunctionMPQ::inversePQ(double value) {
  double tempValue = pow((double) value, m1);
  return (float) (pow(((c2 *(tempValue) + c1)/(1.0 + c3 *(tempValue))), m2));
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


double TransferFunctionMPQ::forward(double value) {
  if (value <= m_invInflectionPointDiv2) {
    return forwardPQ(value * m_scale);

    //double tempValue = pow(value * m_scale, (1.0 / m2));
    //return(float) (pow(dMax(0.0, (tempValue - c1))/(c2 - c3 * tempValue),(1.0 / m1)));
  }
  else {
    return forwardPQ((((value -  m_invInflectionPointDiv2) * (1.0 - m_invInflectionPoint))/(1.0 - m_invInflectionPointDiv2) +  m_invInflectionPoint ));
    
    double tempValue = pow((((value -  m_invInflectionPointDiv2) * (1.0 - m_invInflectionPoint))/(1.0 - m_invInflectionPointDiv2) +  m_invInflectionPoint ), (1.0 / m2));
    return (float) (pow(dMax(0.0, (tempValue - c1))/(c2 - c3 * tempValue),(1.0 / m1)));
  }
}

double TransferFunctionMPQ::inverse(double value) {
  value = dClip(value, 0, 1.0);
  //double tempValue0 = pow((double) value, m1);
  //double tempValue1 = (pow(((c2 *(tempValue0) + c1)/(1.0 + c3 *(tempValue0))), m2));
  double tempValue1 = inversePQ(value);
  
  if (value <= m_inflectionPoint) {
    return (float) tempValue1 / m_scale;
  }
  else {
    return (float) ((((tempValue1 - m_invInflectionPoint) * (1.0 - m_invInflectionPointDiv2)) / (1.0 - m_invInflectionPoint)) + m_invInflectionPointDiv2);
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
