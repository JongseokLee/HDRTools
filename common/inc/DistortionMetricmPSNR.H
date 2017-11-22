/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/ISO
 * <ORGANIZATION> = Ericsson
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Ericsson
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
 * \file DistortionMetricmPSNR.H
 *
 * \brief
 *    mPSNR distortion computation class
 *
 * \author
 *     - Jacob Strom         <jacob.strom@ericsson.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricmPSNR_H__
#define __DistortionMetricmPSNR_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"

class DistortionMetricmPSNR : public DistortionMetric {
private:
  bool              m_enableComponentmPSNR;
  double            m_maxValue[T_COMP];
  double            m_sse[T_COMP];
  double            m_mse[T_COMP];
  MetricStatistics  m_sseStats[T_COMP];
  MetricStatistics  m_mseStats[T_COMP];
  double            m_mPSNR;
  MetricStatistics  m_mPSNRStats;
  vector<imgpel>    m_ldrSrc;
  vector<imgpel>    m_ldrRef;
  int               m_compSize;
  
  double calculateError(Frame* inp0, Frame* inp1);
  double calculateErrorFast(Frame* inp0, Frame* inp1);
  double calculateErrorYUV(Frame* inp0, Frame* inp1);
  double calculateErrorYUVfast(Frame* inp0, Frame* inp1);
  void   calculateSSEnoSaturationRGB(imgpel *lossyimg, imgpel *origimg, int size, int &numPixels, double &sse, double &sseR, double &sseG, double & sseB);
  void   calculateSSEnoSaturationYUV(imgpel *lossyimg, imgpel *origimg, int size, int &numPixels, double &sse, double &sseY, double &sseCb, double & sseCr);
  void   calculateSSEnoSaturationYUVfast(imgpel *lossyimg, imgpel *origimg, int &numPixels, double &sse, double &sseY, double &sseCb, double & sseCr);


public:
  // Construct/Deconstruct
  DistortionMetricmPSNR(const FrameFormat *format, bool enableShowMSE, double maxSampleValue);
  virtual ~DistortionMetricmPSNR();
  
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


