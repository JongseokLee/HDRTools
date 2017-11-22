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
 * \file ConvFixedToFloat.H
 *
 * \brief
 *    Base Class Header for fixed to float precision conversion
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __ConvFixedToFloat_H__
#define __ConvFixedToFloat_H__

#include "Global.H"
#include "Frame.H"
#include "Convert.H"

class ConvFixedToFloat : public Convert {
private:
  static float convertValue        (const imgpel iComp,               double weight, double offset, float minValue, float maxValue);  
  static float convertValue        (const uint16 iComp,               double weight, double offset, float minValue, float maxValue);
  static float convertValue        (const uint16 iComp, int bitScale, double weight, double offset, float minValue, float maxValue);
  static float convertValue        (const imgpel iComp,               double weight, const uint16 offset, float minValue, float maxValue);
  static float convertValue        (const uint16 iComp,               double weight, const uint16 offset, float minValue, float maxValue);
  
  void  convertComponent    (const imgpel *iComp, float *oComp,               int compSize, double weight, double offset, float minValue, float maxValue);
  void  convertComponent    (const uint16 *iComp, float *oComp,               int compSize, double weight, double offset, float minValue, float maxValue);
  void  convertComponent    (const uint16 *iComp, float *oComp, int bitScale, int compSize, double weight, double offset, float minValue, float maxValue);
  void  convertComponent    (const uint16 *iComp, float *oComp,               int compSize, double weight, const uint16 offset, float minValue, float maxValue);
  void  convertComponent    (const imgpel *iComp, float *oComp,               int compSize, double weight, const uint16 offset, float minValue, float maxValue);
  void  convertUi16CompData (Frame* out, const Frame *inp);
  void  convertCompData     (Frame* out, const Frame *inp);
public:
  static float convertUi16CompValue(uint16 inpValue, SampleRange sampleRange, ColorSpace colorSpace, int bitDepth, int component);
  // Construct/Deconstruct
  ConvFixedToFloat();
  virtual ~ConvFixedToFloat();
  
  virtual void process(Frame *out,  const Frame *inp);
};

#endif


