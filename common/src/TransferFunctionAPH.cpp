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
 * \file TransferFunctionAPH.cpp
 *
 * \brief
 *    TransferFunctionAPH Class for Philips Transfer Function (Adaptive)
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
#include "TransferFunctionAPH.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
TransferFunctionAPH::TransferFunctionAPH(double minValue, double maxValue) {
  m_rho   = 25.0;
  m_gamma = 2.4;
  
  m_normalFactor = 1.0;
  
  m_minValue = minValue / 4000.0;
  m_maxValue = maxValue / 4000.0;
  m_minValueInv = inversePH(m_minValue);
  m_maxValueInv = inversePH(m_maxValue);
  
  m_denomInv  = m_maxValueInv - m_minValueInv;
}

TransferFunctionAPH::~TransferFunctionAPH() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
double TransferFunctionAPH::forwardPH(double value) {
  return (pow(((pow( m_rho, value) - 1.0) / (m_rho-1.0)), m_gamma));
}

double TransferFunctionAPH::inversePH(double value) {
  value = dClip(value, 0, 1.0);
  return ((log(1.0 + (m_rho - 1.0) * pow( value, (1.0 / m_gamma))) / log(m_rho)));
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
double TransferFunctionAPH::forward(double value) {
  return (float) forwardPH(value * m_denomInv + m_minValueInv);
}

double TransferFunctionAPH::inverse(double value) {
  return (float) dMax(0.0, dMin(1.0, (inversePH(value) - m_minValueInv) / m_denomInv));
}



//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
