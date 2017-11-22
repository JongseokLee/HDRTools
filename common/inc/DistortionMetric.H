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
 * \file DistortionMetric.H
 *
 * \brief
 *    Distortion Metric parent class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetric_H__
#define __DistortionMetric_H__

#include "Global.H"
#include "Parameters.H"
#include "Frame.H"

enum DistortionMetrics{
  DIST_NULL         =  0,
  DIST_SSE          =  1, // SSE Metric
  DIST_MSE          =  2, // MSE Metric
  DIST_SNR          =  3, // SNR Metric
  DIST_PSNR         =  4, // PSNR Metric
  DIST_MPSNR        =  5, // mPSNR Metric
  DIST_MPSNRFAST    =  6, // mPSNRfast Metric
  DIST_TFPSNR       =  7, // TF domain PSNR Metric
  DIST_DELTAE       =  8, // DeltaE Metric
  DIST_SIGMACOMPARE =  9, // Sigma Compare Metric
  DIST_MSSSIM       = 10, // Multi-Scale Structural Similarity Index (MS-SSIM)
  DIST_RPSNR        = 11, // Region PSNR
  DIST_RTFPSNR      = 12, // TF Domain Region PSNR
  DIST_TFMSSSIM     = 13, // TF Multi-Scale Structural Similarity Index (tMS-SSIM)
  DIST_BLKJ341      = 14,
  DIST_BLK          = 15,
  DIST_VQM			    = 16, // HDR Video Quality Metric
  DIST_VIF          = 17, // VIF Distortion Metric
  DIST_SSIM         = 18, // SSIM Metric
  DIST_TFSSIM       = 19,
  DIST_METRICS
};

static const int NB_REF_WHITE = 3;


class MetricStatistics	{
public:
  double sum;
  double maximum;
  double minimum;
  int    counter;

  void   reset();
  void   updateStats(double value);
  double getAverage();
  MetricStatistics() {
    reset();
  }
};

class SSIMParams {
public:
  int     m_blockDistance;
  int     m_blockSizeX;
  int     m_blockSizeY;
  bool    m_useLog;
  double  m_K1;
  double  m_K2;
  DistortionFunction m_tfDistortion;
  bool    m_tfLUTEnable;

  SSIMParams () {
    m_blockDistance = 1;
    m_blockSizeX    = 4;
    m_blockSizeY    = 4;
    m_useLog        = FALSE;
    m_K1            = 0.01f;
    m_K2            = 0.03f;    
    m_tfDistortion  = DIF_PQPH10K;
    m_tfLUTEnable   = FALSE;
  }
};

class VIFParams {
public:
  int m_vifBitDepth;
  VIFParams() {
    m_vifBitDepth = 10;
  }
};

class VQMParams {
public:
  float	  m_poolingPerc;
  int     m_rowsDisplay;
  int     m_colsDisplay;
  int     m_displayArea;
  float   m_maxDisplay;
  float   m_minDisplay;
  int     m_viewingDistance;
  float   m_fixationTime;
  int     m_numberOfScale;
  int     m_numberOfOrient;
  float   m_frameRate;
  int     m_numberOfFrames;
  bool    m_displayAdapt;
  
  int     m_columnsFFT;
  int     m_rowsFFT;
  
  VQMParams() {
    m_poolingPerc      = 0.0f;
    m_rowsDisplay      = 16;
    m_colsDisplay      = 16;
    m_displayArea      = 256;
    m_maxDisplay       = 1.0f;
    m_minDisplay       = 0.0f;
    m_viewingDistance  = 1;
    m_fixationTime     = 0.2f;
    m_numberOfScale    = 1;
    m_numberOfOrient   = 1;
    m_frameRate        = 60.0f;
    m_numberOfFrames   = 1;
    m_displayAdapt     = TRUE;
    
    m_columnsFFT       = 1024;
    m_rowsFFT          = 512;
  }
};

class PSNRParams {
public:
  bool    m_enableShowMSE;
  bool    m_computePsnrInYCbCr;
  bool    m_computePsnrInRgb;
  bool    m_computePsnrInXYZ;
  bool    m_computePsnrInYUpVp;
  DistortionFunction m_tfDistortion;
  bool    m_tfLUTEnable;
  bool    m_enablexPSNR;
  int     m_rPSNRBlockSizeX;
  int     m_rPSNRBlockSizeY;
  int     m_rPSNROverlapX;
  int     m_rPSNROverlapY;
  double  m_xPSNRweights[T_COMP];
  
  PSNRParams() {
    m_enableShowMSE       = FALSE;
    m_computePsnrInYCbCr  = TRUE;
    m_computePsnrInRgb    = FALSE;
    m_computePsnrInXYZ    = FALSE;
    m_computePsnrInYUpVp  = FALSE;
    m_enablexPSNR         = FALSE;
    m_tfDistortion        = DIF_PQPH10K;
    m_tfLUTEnable         = FALSE;
    m_rPSNRBlockSizeX     = 64;
    m_rPSNRBlockSizeY     = 64;
    m_rPSNROverlapX       = 4;
    m_rPSNROverlapY       = 4;
    for (int c = Y_COMP; c < T_COMP; c++) {
      m_xPSNRweights[c] = 1.0;
    }
  }
};


class DistortionParameters {
public:
  bool    m_enableCompmPSNR;
  bool    m_enableCompmPSNRfast;
  bool    m_clipInputValues;
  double  m_maxSampleValue;
  double  m_whitePointDeltaE[NB_REF_WHITE];
  double  m_amplitudeFactor;
  bool    m_enableSymmetry;
  
  int                m_deltaEPointsEnable;
  VQMParams          m_VQM; 
  VIFParams          m_VIF;
  SSIMParams         m_SSIM;
  PSNRParams         m_PSNR;
    
  DistortionParameters() {
    for (int index = 0; index < NB_REF_WHITE; index ++)
      m_whitePointDeltaE[index] = 1000.0;
    m_enableCompmPSNR           = FALSE;
    m_enableCompmPSNRfast       = FALSE;
    m_clipInputValues           = FALSE;
    m_maxSampleValue            = 10000.0;
    m_amplitudeFactor           = 1.0;
    m_enableSymmetry            = FALSE;
    m_deltaEPointsEnable        = 1;
  }
};

class DistortionMetric {
private:
public:
  MetricStatistics     m_metricStats[T_COMP];
  double               m_metric     [T_COMP];
  int                  m_totalFrames;
  bool                 m_clipInputValues;
  bool                 m_isWindow;
  //edit by Sarvesh
  bool                 m_allFrames;		// This is required by the VQM implementation. VQM requires two passes.
                                        // This flag informs the VQM module that the first pass is over.

  // Construct/Deconstruct
  DistortionMetric();
  virtual ~DistortionMetric() {};
  static  DistortionMetric *create(const FrameFormat *format, int distortionMetric, DistortionParameters *distortionParameters, bool isWindow = FALSE);
  
  virtual void computeMetric (Frame* inp0, Frame* inp1) = 0;                // Compute metric for all components
  virtual void computeMetric (Frame* inp0, Frame* inp1, int component) = 0; // Compute metric for a single component
  virtual void reportMetric  () = 0;                                        // report results
  virtual void reportSummary () = 0;                                        // report summary results
  virtual void reportMinimum () = 0;                                        // report minimum
  virtual void reportMaximum () = 0;                                        // report maximum
  virtual void printHeader   () = 0;                                        // Print metric Header
  virtual void printSeparator() = 0;                                        // Print line separator

};

#endif
