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
 * \file DisplayGammaAdjustHLG.H
 *
 * \brief
 *    DisplayGammaAdjustHLG class Header.
 *    This is an implementation of the display gamma normalization process
 *    in BBC's Hybrid Log-Gamma system (HLG).
 *    A complete documentation of this process is available in 
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


#ifndef __DisplayGammaAdjustHLG_H__
#define __DisplayGammaAdjustHLG_H__

#include "Global.H"
#include "Frame.H"
#include "DisplayGammaAdjust.H"

class DisplayGammaAdjustHLG : public DisplayGammaAdjust {
private:
  double m_linScale;
  double m_tfScale;
  double m_gamma;
  double *m_transformY;

  
public:
  // Constructor and destructor
  DisplayGammaAdjustHLG(double gamma, double scale);  
  virtual ~DisplayGammaAdjustHLG();
  
  virtual void setup  (Frame *frame);
  virtual void forward(const double iComp0, const double iComp1, const double iComp2,  double *oComp0, double *oComp1, double *oComp2);
  virtual void forward(double &comp0, double &comp1, double &comp2);
  virtual void forward(Frame *out,  const Frame *inp);
  virtual void forward(Frame *frame);
  virtual void inverse(const double iComp0, const double iComp1, const double iComp2,  double *oComp0, double *oComp1, double *oComp2);
  virtual void inverse(double &comp0, double &comp1, double &comp2);
  virtual void inverse(Frame *out,  const Frame *inp);
  virtual void inverse(Frame *frame);  
};

#endif
