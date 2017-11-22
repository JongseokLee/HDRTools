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
 * \file DistortionMetricMSSSIM.H
 *
 * \brief
 *    MS-SSIM distortion computation class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricMSSSIM_H__
#define __DistortionMetricMSSSIM_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"

#ifndef UNBIASED_VARIANCE
//#define UNBIASED_VARIANCE // unbiased estimation of the variance
#endif


class DistortionMetricMSSSIM : public DistortionMetric {
private:
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

  void computeComponents (float *lumaCost, float *structCost, uint8  *inp0Data, uint8  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  void computeComponents (float *lumaCost, float *structCost, uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  void computeComponents (float *lumaCost, float *structCost, float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue);

  
  float computeStructuralComponents (uint8  *inp0Data, uint8  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  float computeStructuralComponents (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  float computeStructuralComponents (float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue);

  float computeLuminanceComponent   (uint8  *inp0Data, uint8  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue);
  float computeLuminanceComponent   (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue);
  float computeLuminanceComponent   (float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue);
  
  
  float compute                     (uint8  *inp0Data, uint8  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  float compute                     (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  float compute                     (float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue);
  
  void horizontalSymmetricExtension(int *buffer, int width, int height);
  void horizontalSymmetricExtension(float *buffer, int width, int height );

  void verticalSymmetricExtension  (int *buffer, int width, int height);
  void verticalSymmetricExtension  (float *buffer, int width, int height);
  
  void downsample                    (const uint8*  src, uint8*  out, int iWidth, int iHeight, int oWidth, int oHeight);
  void downsample                    (const uint16* src, uint16* out, int iWidth, int iHeight, int oWidth, int oHeight);
  void downsample                    (const float*  src, float*  out, int iWidth, int iHeight, int oWidth, int oHeight);

  void padImage                      (const uint8*  src, int   *buffer, int width, int height);
  void padImage                      (const uint16* src, int   *buffer, int width, int height);
  void padImage                      (const float*  src, float *buffer, int width, int height);
public:
  // Construct/Deconstruct
  DistortionMetricMSSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue);
  virtual ~DistortionMetricMSSSIM();
  
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


