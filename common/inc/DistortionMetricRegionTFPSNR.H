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
 * \file DistortionMetricRegionTFPSNR.H
 *
 * \brief
 *    PSNR distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricRegionTFPSNR_H__
#define __DistortionMetricRegionTFPSNR_H__

#include <vector>
#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "DistortionMetricTFPSNR.H"


class DistortionMetricRegionTFPSNR : public DistortionMetric {
private:
  DistortionTransferFunction *m_transferFunction;
  ColorSpace                  m_colorSpace;
  int                         m_totalComponents;
  bool                        m_enableShowMSE;
  bool                        m_computePsnrInYCbCr;
  bool                        m_computePsnrInRgb;
  bool                        m_computePsnrInXYZ;
  bool                        m_computePsnrInYUpVp;

  int               m_blockWidth;
  int               m_blockHeight;
  int               m_overlapWidth;
  int               m_overlapHeight;
  int               m_width;
  int               m_height;
  double            m_maxValue[TOTAL_COMPONENTS];
  double            m_sse [TOTAL_COMPONENTS];
  double            m_mse [TOTAL_COMPONENTS];
  double            m_psnr[TOTAL_COMPONENTS];

  
  double            m_maxSSE [TOTAL_COMPONENTS];
  double            m_maxMSE [TOTAL_COMPONENTS];
  double            m_minSSE [TOTAL_COMPONENTS];
  double            m_minMSE [TOTAL_COMPONENTS];
  double            m_maxPSNR[TOTAL_COMPONENTS];
  double            m_minPSNR[TOTAL_COMPONENTS];
  int               m_maxPos [TOTAL_COMPONENTS][2];
  int               m_minPos [TOTAL_COMPONENTS][2];
  
  MetricStatistics  m_sseStats    [TOTAL_COMPONENTS];
  MetricStatistics  m_mseStats    [TOTAL_COMPONENTS];
  MetricStatistics  m_psnrStats   [TOTAL_COMPONENTS];

  MetricStatistics  m_maxMSEStats [TOTAL_COMPONENTS];
  MetricStatistics  m_minMSEStats [TOTAL_COMPONENTS];
  MetricStatistics  m_maxPSNRStats[TOTAL_COMPONENTS];
  MetricStatistics  m_minPSNRStats[TOTAL_COMPONENTS];
  
  vector<double> m_diffData;
  
  vector<double> m_rgb0NormalData;
  vector<double> m_rgb0TFData;
  vector<double> m_xyz0TFData;
  vector<double> m_ycbcr0TFData;
  vector<double> m_yupvp0Data;

  vector<double> m_rgb1NormalData;
  vector<double> m_rgb1TFData;
  vector<double> m_xyz1TFData;
  vector<double> m_ycbcr1TFData;
  vector<double> m_yupvp1Data;

  void   convert (Frame* inp, double *rgbNormalData,
                  double *rgbTFData,
                  double *xyzTFData,
                  double *ycbcrTFData,
                  double *yupvpData);
                                
  void   compute               (Frame* inp0, Frame* inp1);

  double applyTransferFunction (double value);
  void   convertToYCbCrBT2020  (double r, double g, double b, double *y, double *cb, double *cr);
  void   convertToXYZ          (double r, double g, double b, double *x, double *y, double *z);
  void   convertToXYZ          (double r, double g, double b, double *x, double *y, double *z, const double *transform0, const double *transform1, const double *transform2);
  void   convertToYCbCrBT2020  (double *rgb, double *yCbCr);
  void   convertToXYZ          (double *rgb, double *xyz);
  void   convertToXYZ          (double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2);

  void   setColorConversion    (int colorPrimaries, const double **transform0, const double **transform1, const double **transform2);


  double compute(double  *iComp0, double  *iComp1, int width, int height, int component, double maxValue);  
  //double compute(const float  *iComp0, const float  *iComp1, int width, int height, int component, double maxValue);
  uint64 compute(const uint16 *iComp0, const uint16 *iComp1, int size);
  uint64 compute(const uint8  *iComp0, const uint8  *iComp1, int size);
public:
  // Construct/Deconstruct
  DistortionMetricRegionTFPSNR(const FrameFormat *format, PSNRParams *params, double maxSampleValue);
  virtual ~DistortionMetricRegionTFPSNR();
  
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


