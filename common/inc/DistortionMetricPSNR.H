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
 * \file DistortionMetricPSNR.H
 *
 * \brief
 *    PSNR distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricPSNR_H__
#define __DistortionMetricPSNR_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"

class DistortionMetricPSNR : public DistortionMetric {
private:
  ColorSpace        m_colorSpace;
  bool              m_enableShowMSE;
  bool              m_enablexPSNR;
  double            m_xPSNRweights[T_COMP];
  double            m_maxValue[T_COMP];
  double            m_sse[T_COMP];
  double            m_mse[T_COMP];
  double            m_oError;
  double            m_xPSNR;
  
  
  
  MetricStatistics  m_sseStats[T_COMP];
  MetricStatistics  m_mseStats[T_COMP];
  MetricStatistics  m_xPSNRStats;
  
  int            m_width;
  int            m_height;
  vector<double> m_chromaError;

  double computeLumaError(const float  *iComp0, const float  *iComp1, int width, int height, int shiftwidth, int shiftheight, double maxValue, double weight);
  uint64 computeLumaError(const uint16 *iComp0, const uint16 *iComp1, int width, int height, int shiftwidth, int shiftheight, double weight);
  uint64 computeLumaError(const uint8  *iComp0, const uint8  *iComp1, int width, int height, int shiftwidth, int shiftheight, double weight);

  double computeChromaError(const float  *iComp0, const float  *iComp1, int size, double maxValue, bool addError, double weight);
  uint64 computeChromaError(const uint16 *iComp0, const uint16 *iComp1, int size, bool addError, double weight);
  uint64 computeChromaError(const uint8  *iComp0, const uint8  *iComp1, int size, bool addError, double weight);

  
  double compute(const float *iComp0, const float *iComp1, int size, double maxValue);
  uint64 compute(const uint16 *iComp0, const uint16 *iComp1, int size);
  uint64 compute(const uint8 *iComp0, const uint8 *iComp1, int size);
public:
  // Construct/Deconstruct
  DistortionMetricPSNR(const FrameFormat *format, bool enableShowMSE, double maxSampleValue, bool enablexPSNR, double *xPSNRweights);
  virtual ~DistortionMetricPSNR();
  
  virtual void   computeMetric (Frame* inp0, Frame* inp1);                // Compute metric for all components
  virtual void   computeMetric (Frame* inp0, Frame* inp1, int component); // Compute metric for only one component
  virtual void   reportMetric  ();                                        // report frame level results
  virtual void   reportSummary ();                                        // report summary results
  virtual void   reportMinimum ();
  virtual void   reportMaximum ();
  virtual void   printHeader   ();
  virtual void   printSeparator();
};

#endif


