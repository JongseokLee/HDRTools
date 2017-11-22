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
 * \file ColorTransformGeneric.H
 *
 * \brief
 *    ColorTransformGeneric class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __ColorTransformGeneric_H__
#define __ColorTransformGeneric_H__

#include "Global.H"
#include "Frame.H"
#include "ColorTransform.H"
#include "ColorSpaceMatrix.H"


class ColorTransformGeneric : public ColorTransform {
private:
  
  const double   *m_transform0;
  const double   *m_transform1;
  const double   *m_transform2;
  unsigned short  m_sClip;
  double          m_crDivider;
  double          m_cbDivider;
  
  int             m_mode;
  bool            m_isForward;
  float           m_min;
  float           m_max;
   
public:
  // Construct/Deconstruct
  ColorTransformGeneric(ColorTransformParams *params);
  
  static void setYConversion    (int colorPrimaries, const double **transformY);
  static void setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2);

  virtual ~ColorTransformGeneric();
  
  virtual void process(Frame *out,  const Frame *inp);
  
  virtual void RGB2YCbCrConstantLuminance(Frame *out,  const Frame *inp);
  virtual void YCbCrConstantLuminance2RGB(Frame *out,  const Frame *inp);
};

#endif
