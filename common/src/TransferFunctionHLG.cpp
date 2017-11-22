/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = British Broadcasting Corporation (BBC).
 * <ORGANIZATION> = BBC.
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, BBC.
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
 * \file TransferFunctionHLG.cpp
 *
 * \brief
 *    TransferFunctionHLG class implementation for the BBC Hybrid Log-Gamma 
 *    Transfer Function (HLG). 
 *    A complete documentation of this transfer function is available in 
 *    the BBC's response to the MPEG call for evidence on HDR and WCG video coding 
 *   (document m36249, available at:
 *    http://wg11.sc29.org/doc_end_user/documents/112_Warsaw/wg11/m36249-v2-m36249.zip)
 *
 * \author
 *     - Matteo Naccari         <matteo.naccari@bbc.co.uk>
 *     - Manish Pindoria        <manish.pindoria@bbc.co.uk>
 *
 *************************************************************************************
 */

#include "Global.H"
#include "TransferFunctionHLG.H"


//-----------------------------------------------------------------------------
// Constructor / destructor implementation
//-----------------------------------------------------------------------------

TransferFunctionHLG::TransferFunctionHLG()
{
  m_tfScale      = 12.0; //19.6829249; // transfer function scaling - assuming super whites
  m_invTfScale   = 1.0 / m_tfScale;
  m_normalFactor = 1.0;
  m_a            = 0.17883277;
  m_b            = 0.28466892;
  m_c            = 0.55991073;
}


TransferFunctionHLG::~TransferFunctionHLG()
{
}

double TransferFunctionHLG::forward(double value) {
  return ((value < 0.5 ? (value * value * 4.0) : (exp((value - m_c) / m_a) + m_b)) * m_invTfScale);
}

double TransferFunctionHLG::inverse(double value) {
 //printf("the values are %10.7f\n", value);
  value *= m_tfScale;
  return (value < 1.0 ? 0.5 * sqrt(value) : m_a * log(value - m_b) + m_c);
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

