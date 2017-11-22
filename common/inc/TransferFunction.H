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
 * \file TransferFunction.H
 *
 * \brief
 *    Base Class Header for the application of a  Transfer Function
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __TransferFunction_H__
#define __TransferFunction_H__

#include "Global.H"
#include "Parameters.H"
#include "Frame.H"
#include "LookUpTable.H"

static const double DERIV_STEP             = 0.0000001;
static const double DERIV_LOWER_BOUND      = 0.0000000;
static const double DERIV_HIGHER_BOUND     = 1.0000000;
static const int    MAX_BIN_LUT            = 10;
static const int    MAX_ELEMENTS_LUT       = 10000;
static const int    MAX_BIN_DERIV_LUT      = 10;
static const int    MAX_ELEMENTS_DERIV_LUT = 10000;

class TransferFunctionParams {
public:
  int   m_method;
  int   m_singleStep;
  float m_scale;
  float m_systemGamma;
  float m_minValue;
  float m_maxValue;
  bool  m_enableLUT;
  
  // DPX parameters
  double m_dpxSoftClip;
  double m_dpxRefWhite;
  double m_dpxRefBlack;
  double m_dpxDisplayGamma;
  
  double m_dpxLinRef;
  double m_dpxLogRef;
  double m_dpxNegGamma;
  double m_dpxDensityValue;

  
  TransferFunctionParams() {
    m_method = 0;
    m_singleStep = 1;
    m_scale = 1.0f;
    m_systemGamma = 2.2f;
    m_minValue = 0.0f;
    m_maxValue = 1.0f;
    m_enableLUT = FALSE;    
  
    // Init dpx params
    m_dpxSoftClip = 0.0;
    m_dpxRefWhite = 685.0;
    m_dpxRefBlack = 95.0;
    m_dpxDisplayGamma = 1.7;
    
    m_dpxLinRef = 0.18;
    m_dpxLogRef = 445.0;
    m_dpxNegGamma = 0.6;
    m_dpxDensityValue = 0.002;
  }
};

class TransferFunction {
protected:
  double                  m_normalFactor;
  double                  m_invNormalFactor;

  // LUT related
  bool                    m_enableLUT;
  uint32                  m_binsLUT;
  uint32                  m_binsLUTExt;
  vector<uint32>          m_elementsLUT;
  vector<double>          m_boundLUT;
  vector<double>          m_multiplierLUT;
  vector<vector<double> > m_invTransformLUT;
  vector<vector<double> > m_fwdTransformLUT;
  double                  m_maxFwdLUT;
  double                  m_maxInvLUT;

  bool                    m_enableFwdDerivLUT;
  uint32                  m_derivBinsLUT;
  uint32                  m_derivBinsLUTExt;
  vector<uint32>          m_derivElementsLUT;
  vector<double>          m_derivBoundLUT;
  vector<double>          m_derivMultiplierLUT;
  vector<vector<double> > m_fwdTFDerivativeLUT;
  double                  m_maxFwdTFDerLUT;
  //double                  m_maxInvTFDerLUT;

  void   initLUT();
  void   initfwdTFDerivLUT();
  
  double inverseLUT         (double value);
  double forwardLUT         (double value);
  double forwardDerivLUT    (double value);
  
  LookUpTable *m_invLUT;
  LookUpTable *m_fwdLUT;
  LookUpTable *m_fwdDerivLUT;

  void initInvTFLUTs(LookUpTable *lut);
  void initFwdTFLUTs(LookUpTable *lut);
  void initDevTFLUTs(LookUpTable *lut);
  void initTFLUTs(LookUpTable *lut, double (*method) (double));
  
public:

  TransferFunction() {
    m_normalFactor = 1.0;
    m_invNormalFactor = 1.0;
    m_enableLUT = FALSE;
    m_enableFwdDerivLUT = FALSE;
    m_binsLUT = 1;
    m_derivBinsLUT = 1;
    m_invLUT = NULL;
    m_fwdLUT = NULL;
    m_fwdDerivLUT = NULL;
  }
  
  // Construct/Deconstruct
  virtual ~TransferFunction() {
    if (m_invLUT != NULL) {
      delete m_invLUT;
      m_invLUT = NULL;
    }
    if (m_fwdLUT != NULL) {
      delete m_fwdLUT;
      m_fwdLUT = NULL;
    }
    if (m_fwdDerivLUT != NULL) {
      delete m_fwdDerivLUT;
      m_fwdDerivLUT = NULL;
    }
  };
  
  static TransferFunction *create(int method, bool singleStep, float scale, float systemGamma, float minValue, float maxValue, bool enableLUT = FALSE, bool fwdTFDerivativeLUT = FALSE, TransferFunctionParams *params = NULL);
  
  double  getNormalFactor()       const { return m_normalFactor; };
  void    setNormalFactor(double value) { m_normalFactor = value; m_invNormalFactor = 1.0 / value; };
  
  void    forward           (Frame *frame);
  void    forward           (Frame *frame, int component);
  void    inverse           (Frame *frame);
  void    inverse           (Frame *frame, int component);

  double  getForward        (double value);
  double  getInverse        (double value);
  
  double  forwardDerivative (double value);
  double  inverseDerivative (double value);
  double  getForwardDerivative(double value);
   
  virtual double forward    (double value) = 0;
  virtual double inverse    (double value) = 0;
  virtual void   forward    (Frame *out,  const Frame *inp);
  virtual void   forward    (Frame *out,  const Frame *inp, int component);
  virtual void   inverse    (Frame *out,  const Frame *inp);
  virtual void   inverse    (Frame *out,  const Frame *inp, int component);
};

#endif
