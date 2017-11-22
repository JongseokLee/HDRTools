/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = British Broadcasting Corporation (BBC).
 * <ORGANIZATION> = BBC.
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, BBC.
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
 * \file TransferFunctionOHG.cpp
 *
 * \brief
 *    TransferFunctionOHG class implementation for an older version of 
 *    the BBC Hybrid Gamma (HG) Transfer Function. This code is retained as
 *    a reference and for historical purposes, and may not be functional at this point.
 *
 * \author
 *     - Matteo Naccari         <matteo.naccari@bbc.co.uk>
 *
 *************************************************************************************
 */

#include "Global.H"
#include "TransferFunctionOHG.H"

//-----------------------------------------------------------------------------
// Constructor / destructor implementation
//-----------------------------------------------------------------------------

TransferFunctionOHG::TransferFunctionOHG(double gamma)
{
  m_mu    = 0.139401137752;
  m_eta   = sqrt(m_mu)/2.0;
  m_rho   = sqrt(m_mu)*(1.0-log(sqrt(m_mu)));
  m_xi    = sqrt(m_mu);
  m_gamma = gamma;
  
  m_normalFactor = 1.0;
}

TransferFunctionOHG::TransferFunctionOHG(double gamma, double normalFactor)
{
  m_mu    = 0.139401137752;
  m_eta   = sqrt(m_mu)/2.0;
  m_rho   = sqrt(m_mu)*(1.0-log(sqrt(m_mu)));
  m_xi    = sqrt(m_mu);
  m_gamma = gamma;
  
  m_normalFactor = (double) normalFactor;
}

TransferFunctionOHG::~TransferFunctionOHG()
{
}

//-----------------------------------------------------------------------------
// PUBLIC
//-----------------------------------------------------------------------------

double TransferFunctionOHG::forward(double value) {
  return (value <= m_xi ? pow(value, 2.0 * m_gamma) : exp(m_gamma * (value - m_rho) / m_eta));
}

double TransferFunctionOHG::inverse(double value) {
  return (value <= m_mu ? pow(value, 0.5) : m_eta * log(value) + m_rho);
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

