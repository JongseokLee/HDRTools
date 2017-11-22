/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Technicolor
 * <ORGANIZATION> = Technicolor
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Technicolor
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
 * \file DistortionMetricDeltaE.H
 *
 * \brief
 *    DeltaE distortion computation Class
 *
 * \author
 *     - Christophe Chevance         <christophe.chevance@technicolor.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricDELTAE_H__
#define __DistortionMetricDELTAE_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"

#define FILE_REF_IN_NITS_DOUBLE     1.0

#define DEG275                      4.7996554429844
#define DEG30                       0.523598775598299
#define DEG6                        0.1047197551196598
#define DEG63                       1.099557428756428
#define PI                          3.14159265
#define DEG25                       0.436332

class DistortionMetricDeltaE : public DistortionMetric {
private:
  bool              m_enableShowMSE;
  double            m_maxValue;
  int               m_deltaEPointsEnable;
  
  double            m_deltaE          [NB_REF_WHITE];
  MetricStatistics  m_deltaEStats     [NB_REF_WHITE];
  double            m_whitePointDeltaE[NB_REF_WHITE];
  double            m_PsnrDE          [NB_REF_WHITE];
  MetricStatistics  m_PsnrDEStats     [NB_REF_WHITE];

  double            m_maxDeltaE       [NB_REF_WHITE];
  MetricStatistics  m_maxDeltaEStats  [NB_REF_WHITE];
  double            m_PsnrMaxDE       [NB_REF_WHITE];
  MetricStatistics  m_PsnrMaxDEStats  [NB_REF_WHITE];

  double            m_PsnrL           [NB_REF_WHITE];
  MetricStatistics  m_PsnrLStats      [NB_REF_WHITE];
  
  double trueLab      (double r);
  void   xyz2TrueLab  (double srcX, double srcY, double srcZ, double *dstL, double *dstA, double *dstB, double whitePointDeltaE);
  void   xyz2TrueLab  (double srcX, double srcY, double srcZ, double *dstL, double *dstA, double *dstB, double invYn, double invXn, double invZn);

  double deltaE2000   (double lRef, double aStarRef, double bStarRef, double lIn, double aStarIn, double bStarIn);
  double getDeltaE2000(double x, double y, double z, double xRec, double yRec, double zRec, double whitePointDeltaE);
  double getDeltaE2000(double x, double y, double z, double xRec, double yRec, double zRec, double invYn, double invXn, double invZn);

public:
  // Construct/Deconstruct
  DistortionMetricDeltaE(const FrameFormat *format, bool enableShowMSE, double maxSampleValue, double *whitePointDeltaE, int deltaEPointsEnable);
  virtual ~DistortionMetricDeltaE();
  
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


