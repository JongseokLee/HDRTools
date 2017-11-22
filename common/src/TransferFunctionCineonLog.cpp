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
 * \file TransferFunctionCineonLog.cpp
 *
 * \brief
 *    TransferFunctionCineonLog Class
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
#include "TransferFunctionCineonLog.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------
     //const double maxAimDensity[3] = {1.890, 2.046, 2.046};  // ARRI "carlos" aims
     //const double negGamma[3] = {0.49, 0.57, 0.60}; 					// gamma of negative (Kodak 5201)
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

TransferFunctionCineonLog::TransferFunctionCineonLog(TransferFunctionParams *params) {

if (params == NULL) {
  m_softClip = 0.0;
  m_refWhite = 1023;
  m_refBlack = 0.0; //95.0;
  m_displayGamma = 1.7;
  
  m_linRef = 0.18;
  m_logRef = 445.0;
  m_negGamma = 0.6;
  m_densityValue = 0.002;
}
else {
  m_softClip = params->m_dpxSoftClip;
  m_refWhite = params->m_dpxRefWhite;
  m_refBlack = params->m_dpxRefBlack;
  m_displayGamma = params->m_dpxDisplayGamma;
  
  m_linRef = params->m_dpxLinRef;
  m_logRef = params->m_dpxLogRef;
  m_negGamma = params->m_dpxNegGamma;
  m_densityValue = params->m_dpxDensityValue;
}

  m_breakpoint = m_refWhite - m_softClip;
  m_scale = m_densityValue / m_negGamma;
  m_invScale = 1.0 / m_scale;
  
  m_gain = 225.0 / pow((1.0 - pow(10, (m_refBlack - m_refWhite)) * m_densityValue / m_negGamma), (m_displayGamma / 1.7));
  //m_gain = 225.0 / (1.0 - pow(10, (m_refBlack - m_refWhite)) * m_densityValue / m_negGamma);

  m_offset = m_gain - 225.0;

  m_kneeSoft = pow(pow(10, ((m_breakpoint-m_refWhite) * m_densityValue/ m_negGamma)),  (m_displayGamma/1.7)) * m_gain - m_offset;
  //m_kneeSoft = pow(10, ((m_breakpoint-m_refWhite) * m_densityValue/ m_negGamma)) * m_gain - m_offset;

  m_kneeGain = (255 - m_kneeSoft)/( pow((5 * m_softClip), (m_softClip / 100)));

}

TransferFunctionCineonLog::~TransferFunctionCineonLog() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
double TransferFunctionCineonLog::forward(double value) {
  //value = dClip(value, 0, 1.0);

  return (float) (pow(10, (value * 1023.0 - m_refWhite) * m_scale) * m_gain - m_offset) / 255.0;
}

double TransferFunctionCineonLog::inverse(double value) {
  //value = dClip(value, 0, 1.0);
  double tempValue = dMax( value, 1e-10 ) / m_linRef;
  return (float) ((m_logRef + log10(tempValue) * m_invScale) / 1023.0);
}





//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
