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
 * \file TransferFunctionDPXLog.cpp
 *
 * \brief
 *    TransferFunctionDPXLog Class
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
#include "TransferFunctionDPXLog.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

TransferFunctionDPXLog::TransferFunctionDPXLog() {
  m_linRef = 0.18;
  m_logRef = 445.0;
  m_negGamma = 0.6;
  m_densityValue = 0.002;
  m_scale = m_densityValue / m_negGamma;
  m_invScale = 1.0 / m_scale;
}

TransferFunctionDPXLog::~TransferFunctionDPXLog() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
double TransferFunctionDPXLog::forward(double value) {
  //value = dClip(value, 0, 1.0);

  return (float) (m_linRef *  pow(10.0, (value * 1023.0 - m_logRef) * m_scale ));
}

double TransferFunctionDPXLog::inverse(double value) {
  //value = dClip(value, 0, 1.0);
  double tempValue = dMax( value, 1e-10 ) / m_linRef;
  return (float) ((m_logRef + log10(tempValue) * m_invScale) / 1023.0);
}





//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
