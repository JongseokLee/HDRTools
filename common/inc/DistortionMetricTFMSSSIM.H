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
 * \file DistortionMetricTFMSSSIM.H
 *
 * \brief
 *    TF MS-SSIM distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricTFMSSSIM_H__
#define __DistortionMetricTFMSSSIM_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "DistortionTransferFunction.H"
#include <vector>


#ifndef UNBIASED_VARIANCE
//#define UNBIASED_VARIANCE // unbiased estimation of the variance
#endif


class DistortionMetricTFMSSSIM : public DistortionMetric {
private:
  vector<float> m_dataY0;
  vector<float> m_dataY1;
  int m_memWidth;
  int m_memHeight;

  vector<double> m_dsRef;
  vector<double> m_dsEnc;
  int m_dsWidth;
  int m_dsHeight;

  vector<double> m_temp;
  vector<double> m_dest;
  int m_tempWidth;
  int m_tempHeight;
  int m_destWidth;
  int m_destHeight;
  
  int m_maxSSIMLevelsMinusOne;

  DistortionTransferFunction *m_transferFunction;
  ColorSpace        m_colorSpace;
  double            m_maxValue[T_COMP];
  float             m_exponent[5];
  double            m_K1;
  double            m_K2;
  double            m_bias;
  
  int m_blockDistance;
  int m_blockSizeX;
  int m_blockSizeY;
  bool m_useLogSSIM;
  
  void setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2);
  void convertToXYZ(double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2);

 void computeComponents (float *lumaCost, float *structCost, float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, float maxPixelValue);

  
  float computeStructuralComponents (float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, float maxPixelValue);

  float computeLuminanceComponent   (float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, float maxPixelValue);
  
  
  void  compute                    (Frame* inp0, Frame* inp1);
  float computePlane               (float  *inp0Data, float  *inp1Data, int height, int width, float maxPixelValue);
  
  void horizontalSymmetricExtension(float *buffer, int width, int height );

  void verticalSymmetricExtension  (float *buffer, int width, int height);
  
  void downsample                    (const float*  src, float*  out, int iWidth, int iHeight, int oWidth, int oHeight);

  void padImage                      (const float*  src, float *buffer, int width, int height);
public:
  // Construct/Deconstruct
  DistortionMetricTFMSSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue);
  virtual ~DistortionMetricTFMSSSIM();
  
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


