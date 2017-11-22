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
 * \file TransferFunctionHGold.H
 *
 * \brief
 *    TransferFunctionHGold class Header for the BBC Hybrid Gamma (HG) Transfer Function
 *
 * \author
 *     - Matteo Naccari         <matteo.naccari@bbc.co.uk>
 *
 *************************************************************************************
 */


#ifndef __TransferFunctionHGold_H__
#define __TransferFunctionHGold_H__

#include "Global.H"
#include "Frame.H"
#include "TransferFunction.H"

class TransferFunctionHGold : public TransferFunction {
private:
  double m_mu;
  double m_eta;
  double m_rho;
  double m_xi;
  double m_gamma;
  
public:
  // Constructor and destructor
  TransferFunctionHGold(double gamma);
  virtual ~TransferFunctionHGold();
  
  virtual double forward(double value);
  virtual double inverse(double value);

  
  virtual void forward(Frame *out,  const Frame *inp);
  virtual void forward(Frame *out,  const Frame *inp, int component);
  virtual void inverse(Frame *out,  const Frame *inp);
  virtual void inverse(Frame *out,  const Frame *inp, int component);
};

#endif
