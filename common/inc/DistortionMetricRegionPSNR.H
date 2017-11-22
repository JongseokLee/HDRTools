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
 * \file DistortionMetricRegionPSNR.H
 *
 * \brief
 *    PSNR distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricRegionPSNR_H__
#define __DistortionMetricRegionPSNR_H__

#include <vector>
#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"

class DistortionMetricRegionPSNR : public DistortionMetric {
private:
  int               m_blockWidth;
  int               m_blockHeight;
  int               m_overlapWidth;
  int               m_overlapHeight;
  int               m_width;
  int               m_height;
  ColorSpace        m_colorSpace;
  bool              m_enableShowMSE;
  double            m_maxValue[T_COMP];
  double            m_sse[T_COMP];
  double            m_mse[T_COMP];
  
  double            m_maxSSE [T_COMP];
  double            m_maxMSE [T_COMP];
  double            m_minSSE [T_COMP];
  double            m_minMSE [T_COMP];
  double            m_maxPSNR[T_COMP];
  double            m_minPSNR[T_COMP];
  int               m_maxPos [T_COMP][2];
  int               m_minPos [T_COMP][2];
  
  MetricStatistics  m_sseStats    [T_COMP];
  MetricStatistics  m_mseStats    [T_COMP];
  MetricStatistics  m_maxMSEStats [T_COMP];
  MetricStatistics  m_minMSEStats [T_COMP];
  MetricStatistics  m_maxPSNRStats[T_COMP];
  MetricStatistics  m_minPSNRStats[T_COMP];
  
  vector<double> m_diffData;

  
  double compute(const float *iComp0, const float *iComp1, int width, int height, int component, double maxValue);
  uint64 compute(const uint16 *iComp0, const uint16 *iComp1, int width, int height, int component, int maxValue);
  uint64 compute(const uint8 *iComp0, const uint8 *iComp1, int width, int height, int component, int maxValue);
public:
  // Construct/Deconstruct
  DistortionMetricRegionPSNR(const FrameFormat *format, PSNRParams *params, double maxSampleValue);
  virtual ~DistortionMetricRegionPSNR();
  
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


