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
 * \file DistortionMetricTFPSNR.H
 *
 * \brief
 *    PSNR distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricTFPSNR_H__
#define __DistortionMetricTFPSNR_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "DistortionTransferFunction.H"

static const int TOTAL_COMPONENTS = 17;

class DistortionMetricTFPSNR : public DistortionMetric {
private:
  DistortionTransferFunction *m_transferFunction;
  ColorSpace                  m_colorSpace;
  int                         m_totalComponents;
  bool                        m_enableShowMSE;
  bool                        m_computePsnrInYCbCr;
  bool                        m_computePsnrInRgb;
  bool                        m_computePsnrInXYZ;
  bool                        m_computePsnrInYUpVp;
  double                      m_maxValue  [TOTAL_COMPONENTS];
  double                      m_sse       [TOTAL_COMPONENTS];
  double                      m_mse       [TOTAL_COMPONENTS];
  double                      m_psnr      [TOTAL_COMPONENTS];
  MetricStatistics            m_sseStats  [TOTAL_COMPONENTS];
  MetricStatistics            m_mseStats  [TOTAL_COMPONENTS];
  MetricStatistics            m_psnrStats [TOTAL_COMPONENTS];
  
  void   compute                 (Frame* inp0, Frame* inp1);
  void   convertToYCbCrBT2020    (double *rgb, double *yCbCr);
  void   convertToYCbCr          (double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2);
  void   convertToXYZ            (double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2);
  void   setColorConversion      (int colorPrimaries, const double **transform0, const double **transform1, const double **transform2);
  void   setColorConversionYCbCr (int colorPrimaries, const double **transform0, const double **transform1, const double **transform2);
public:
  // Construct/Deconstruct
  DistortionMetricTFPSNR(const FrameFormat *format, PSNRParams *params, double maxSampleValue);
  virtual ~DistortionMetricTFPSNR();
  
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


