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
 * \file ToneMappingCIE1931.H
 *
 * \brief
 *    ToneMappingCIE1931 class Header
 *    This function is based on the tone mapping method described in BT.2390
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Teun Baar
 *
 *************************************************************************************
 */


#ifndef __ToneMappingCIE1931_H__
#define __ToneMappingCIE1931_H__

#include "Global.H"
#include "Frame.H"
#include "ToneMapping.H"
#include "TransferFunction.H"

class ToneMappingCIE1931 : public ToneMapping {
private:
  bool   m_scaleGammut;
  double m_maxValue;
  
  double m_maxIntensity;
  double m_KS;
  double m_KSIntensity;
  
  TransferFunction   *m_transferFunction;

  void convertXYZToxyY(double *xyz, double *xyY);
  void convertxyYToXYZ(double *xyY, double *xyz);
  void convertToXYZ(double *rgb, double *xyz, const double transform[3][3]);
  void convertToRGB(double *xyz, double *rgb, const double transform[3][3]);
  void setColorConversion(int colorPrimaries,  double transformFW[3][3],  double transformBW[3][3]);
  
public:
  // Construct/Deconstruct
  ToneMappingCIE1931(  
                  double minValue,
                  double maxValue,
                  double targetValue,
                  bool   scaleGammut
                    );
  virtual ~ToneMappingCIE1931();

  virtual void process(Frame *out,  const Frame *inp);
  virtual void process(Frame *frame);
 };

#endif
