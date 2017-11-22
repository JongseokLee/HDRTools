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
 * \file DistortionTransferFunction.H
 *
 * \brief
 *    Distortion Transfer Function class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionTransferFunction_H__
#define __DistortionTransferFunction_H__

#include "Global.H"
#include <vector>
//#include "Frame.H"
//#include "DistortionMetric.H"

class DistortionTransferFunction {
protected:
  // LUT related
  bool                    m_enableLUT;
  uint32                  m_binsLUT;
  vector<uint32>          m_elementsLUT;
  vector<double>          m_boundLUT;
  vector<double>          m_multiplierLUT;
  vector<vector<double> > m_computeLUT;

  double computeDE      (double value);
  double computePQ      (double value);
  double computeHPQ     (double value);
  double computeHPQ2    (double value);
  double computePQNoise (double value);
  double computePH10K   (double value);
  double computeNull    (double value);
  
  void   initLUT();
  double computeLUT(double value);

public:
  static DistortionTransferFunction *create(DistortionFunction method, bool enableLUT);
  
  virtual double compute(double value) = 0;
  virtual ~DistortionTransferFunction() {};
  double performCompute(double value);

};

class DistortionTransferFunctionNull : public DistortionTransferFunction {
public:
  virtual double compute(double value);
};

class DistortionTransferFunctionPQNoise : public DistortionTransferFunction {
public:
  virtual double compute(double value);
};

class DistortionTransferFunctionPQ : public DistortionTransferFunction {
public:
  virtual double compute(double value);
};

class DistortionTransferFunctionHPQ : public DistortionTransferFunction {
public:
  virtual double compute(double value);
};

class DistortionTransferFunctionHPQ2 : public DistortionTransferFunction {
public:
  virtual double compute(double value);
};

class DistortionTransferFunctionCombo : public DistortionTransferFunction {
public:
  virtual double compute(double value);
};

class DistortionTransferFunctionDE : public DistortionTransferFunction {
public:
  virtual double compute(double value);
};

#endif


