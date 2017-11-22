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
 * \file DistortionMetricMSSSIM.cpp
 *
 * \brief
 *    MS-SSIM (Multi-Scale Structural Similarity) distortion computation Class
 *    Code is based on the JM implementation written by Dr. Peshala Pahalawatta
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricMSSSIM.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MS_SSIM_PAD  6
#define MS_SSIM_PAD2 3

static const double kMsSSIMBeta0 = 0.0448;
static const double kMsSSIMBeta1 = 0.2856;
static const double kMsSSIMBeta2 = 0.3001;
static const double kMsSSIMBeta3 = 0.2363;
static const double kMsSSIMBeta4 = 0.1333;

#define MAX_SSIM_LEVELS 5

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricMSSSIM::DistortionMetricMSSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue)
: DistortionMetric()
{
  // Metric parameters
  m_K1 = params->m_K1;
  m_K2 = params->m_K2;
  
  m_blockDistance = params->m_blockDistance;
  m_blockSizeX    = params->m_blockSizeX;
  m_blockSizeY    = params->m_blockSizeY;
  m_useLogSSIM    = params->m_useLog;
  
#ifdef UNBIASED_VARIANCE
  m_bias = 1;
#else
  m_bias = 0;
#endif

  m_colorSpace    = format->m_colorSpace;
  m_exponent[0] = (float) kMsSSIMBeta0;
  m_exponent[1] = (float) kMsSSIMBeta1;
  m_exponent[2] = (float) kMsSSIMBeta2;
  m_exponent[3] = (float) kMsSSIMBeta3;
  m_exponent[4] = (float) kMsSSIMBeta4;
    
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
  }
}

DistortionMetricMSSSIM::~DistortionMetricMSSSIM()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
//Computes the product of the contrast and structure components of the structural similarity metric.
void DistortionMetricMSSSIM::computeComponents (float *lumaCost, float *structCost, float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  //double maxPixelValueSquared = (double) (100.0 * 100.0);
  double C1 = m_K1 * m_K1 * maxPixelValueSquared;
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
  double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
  double structDistortion = 0.0;
  double lumaDistortion = 0.0;
  int i, j, n, m, windowCounter = 0;
  float *inp0Line = NULL;
  float *inp1Line = NULL;
  
  float maxValue = 0.0;
  double inp0, inp1;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      double sumInp0 = 0.0f;
      double sumInp1 = 0.0f;
      double sumSquareInp0  = 0.0f;
      double sumSquareInp1  = 0.0f;
      double sumMultiInp0Inp1 = 0.0f;
      
      for ( n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          if (maxValue < *inp0Line)
            maxValue = *inp0Line;
          inp0              = (double) *inp0Line++;
          inp1              = (double) *inp1Line++;
          sumInp0          += inp0;
          sumInp1          += inp1;
          sumSquareInp0    += inp0 * inp0;
          sumSquareInp1    += inp1 * inp1;
          sumMultiInp0Inp1 += inp0 * inp1;
        }
      }
      
      meanInp0 = sumInp0 / windowPixels;
      meanInp1 = sumInp1 / windowPixels;

      blockSSIM  = (2.0 * meanInp0 * meanInp1 + C1) / (meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1);
      
      lumaDistortion += blockSSIM;

      varianceInp0 = (sumSquareInp0    - sumInp0 * meanInp0) / windowPixelsBias;
      varianceInp1 = (sumSquareInp1    - sumInp1 * meanInp1) / windowPixelsBias;
      covariance   = (sumMultiInp0Inp1 - sumInp0 * meanInp1) / windowPixelsBias;
      
      blockSSIM  = (2.0 * covariance + C2) /(varianceInp0 + varianceInp1 + C2);
      
      structDistortion += blockSSIM;
      windowCounter++;
    }
  }

  lumaDistortion /= dMax(1.0, (double) windowCounter);
  
  if (lumaDistortion >= 1.0 && lumaDistortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    lumaDistortion = 1.0;
  
  *lumaCost = (float) lumaDistortion;
  
  structDistortion /= dMax(1.0, (double) windowCounter);
  
  if (structDistortion >= 1.0 && structDistortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    structDistortion = 1.0;
  
  *structCost = (float) structDistortion;

}

float DistortionMetricMSSSIM::computeStructuralComponents (uint8 *inp0Data, uint8 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  float maxPixelValueSquared = (float) (maxPixelValue * maxPixelValue);
  float C2 = (float) (m_K2 * m_K2 * maxPixelValueSquared);
  float windowPixels = (float) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;

  float blockSSIM = 0.0f, meanInp0 = 0.0f, meanInp1 = 0.0f;
  float varianceInp0 = 0.0f, varianceInp1 = 0.0f, covariance = 0.0f;
  int sumInp0 = 0, sumInp1 = 0, sumSquareInp0 = 0, sumSquareInp1 = 0, sumMultiplyInp0Inp1 = 0;
  float distortion = 0.0f;
  int i, j, n, m, windowCounter = 0;
  uint8 *inp0Line = NULL;
  uint8 *inp1Line = NULL;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      sumInp0 = 0;
      sumInp1 = 0;
      sumSquareInp0  = 0;
      sumSquareInp1  = 0;
      sumMultiplyInp0Inp1 = 0;
      
      for ( n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          sumInp0   += *inp0Line;
          sumInp1   += *inp1Line;
          sumSquareInp0    += *inp0Line * *inp0Line;
          sumSquareInp1    += *inp1Line * *inp1Line;
          sumMultiplyInp0Inp1 += *inp0Line++ * *inp1Line++;
        }
      }
      
      meanInp0 = (float) sumInp0 / windowPixels;
      meanInp1 = (float) sumInp1 / windowPixels;
      
      varianceInp0 = (float) (((float) sumSquareInp0 - ((float) sumInp0) * meanInp0) / windowPixelsBias);
      varianceInp1 = (float) (((float) sumSquareInp1 - ((float) sumInp1) * meanInp1) / windowPixelsBias);
      covariance   = (float) (((float) sumMultiplyInp0Inp1 - ((float) sumInp0) * meanInp1) / windowPixelsBias);
      
      blockSSIM  = (float) (2.0f * covariance + C2);
      blockSSIM /= (float) (varianceInp0 + varianceInp1 + C2);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
  
  distortion /= (float) dMax(1, windowCounter);
  
  if (distortion >= 1.0f && distortion < 1.01f) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0f;
  
  return distortion;
}

float DistortionMetricMSSSIM::computeStructuralComponents (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  float maxPixelValueSquared = (float) (maxPixelValue * maxPixelValue);
  float C2 = (float) (m_K2 * m_K2 * maxPixelValueSquared);
  float windowPixels = (float) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;

  float blockSSIM = 0.0f, meanInp0 = 0.0f, meanInp1 = 0.0f;
  float varianceInp0 = 0.0f, varianceInp1 = 0.0f, covariance = 0.0f;
  int sumInp0 = 0, sumInp1 = 0, sumSquareInp0 = 0, sumSquareInp1 = 0, sumMultiplyInp0Inp1 = 0;
  float distortion = 0.0f;
  int i, j, n, m, windowCounter = 0;
  uint16 *inp0Line = NULL;
  uint16 *inp1Line = NULL;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      sumInp0 = 0;
      sumInp1 = 0;
      sumSquareInp0  = 0;
      sumSquareInp1  = 0;
      sumMultiplyInp0Inp1 = 0;
      
      for ( n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          sumInp0   += *inp0Line;
          sumInp1   += *inp1Line;
          sumSquareInp0    += *inp0Line * *inp0Line;
          sumSquareInp1    += *inp1Line * *inp1Line;
          sumMultiplyInp0Inp1 += *inp0Line++ * *inp1Line++;
        }
      }
      
      meanInp0 = (float) sumInp0 / windowPixels;
      meanInp1 = (float) sumInp1 / windowPixels;
      
      varianceInp0 = (float) (((float) sumSquareInp0 - ((float) sumInp0) * meanInp0) / windowPixelsBias);
      varianceInp1 = (float) (((float) sumSquareInp1 - ((float) sumInp1) * meanInp1) / windowPixelsBias);
      covariance   = (float) (((float) sumMultiplyInp0Inp1 - ((float) sumInp0) * meanInp1) / windowPixelsBias);
      
      blockSSIM  = (float) (2.0f * covariance + C2);
      blockSSIM /= (float) (varianceInp0 + varianceInp1 + C2);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
  
  distortion /= (float) dMax(1, windowCounter);
  
  if (distortion >= 1.0f && distortion < 1.01f) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0f;
  
  return distortion;
}

float DistortionMetricMSSSIM::computeStructuralComponents (float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  //double maxPixelValueSquared = (double) (100.0 * 100.0);
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double distortion = 0.0;
  int    windowCounter = 0;
  float *inp0Line = NULL;
  float *inp1Line = NULL;
  
  float maxValue = 0.0;
  double inp0, inp1;
  
  for (int j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (int i = 0; i <= width - windowWidth; i += m_blockDistance) {
      double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
      double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
      double sumInp0 = 0.0f;
      double sumInp1 = 0.0f;
      double sumSquareInp0  = 0.0f;
      double sumSquareInp1  = 0.0f;
      double sumMultiInp0Inp1 = 0.0f;
      
      for ( int n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (int m = ZERO; m < windowWidth; m++) {
          if (maxValue < *inp0Line)
            maxValue = *inp0Line;
          inp0              = (double) *inp0Line++;
          inp1              = (double) *inp1Line++;
          sumInp0          += inp0;
          sumInp1          += inp1;
          sumSquareInp0    += inp0 * inp0;
          sumSquareInp1    += inp1 * inp1;
          sumMultiInp0Inp1 += inp0 * inp1;
        }
      }
      
      meanInp0 = sumInp0 / windowPixels;
      meanInp1 = sumInp1 / windowPixels;
      
      varianceInp0 = (sumSquareInp0    - sumInp0 * meanInp0) / windowPixelsBias;
      varianceInp1 = (sumSquareInp1    - sumInp1 * meanInp1) / windowPixelsBias;
      covariance   = (sumMultiInp0Inp1 - sumInp0 * meanInp1) / windowPixelsBias;
      
      blockSSIM  = (2.0 * covariance + C2) /(varianceInp0 + varianceInp1 + C2);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
    
  distortion /= dMax(1.0, (double) windowCounter);
    
  if (distortion >= 1.0 && distortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0;
  
  return (float) distortion;
}

float DistortionMetricMSSSIM::computeLuminanceComponent (uint8 *inp0Data, uint8 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  float maxPixelValueSquared = (float) (maxPixelValue * maxPixelValue);
  float C1 = (float) (m_K1 * m_K1 * maxPixelValueSquared);
  float windowPixels = (float) (windowWidth * windowHeight);
  float blockSSIM, meanInp0, meanInp1;
  int sumInp0, sumInp1;
  float distortion = 0.0f;
  int i, j, n, m, windowCounter = 0;
  
  uint8 *inp0Line = NULL;
  uint8 *inp1Line = NULL;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      sumInp0 = 0;
      sumInp1 = 0;
      
      for ( n = j; n < j + windowHeight; n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          sumInp0   += *inp0Line++;
          sumInp1   += *inp1Line++;
        }
      }
      
      meanInp0 = (float) sumInp0 / windowPixels;
      meanInp1 = (float) sumInp1 / windowPixels;
      
      blockSSIM  = (float) (2.0f * meanInp0 * meanInp1 + C1);
      blockSSIM /= (float) (meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
  
  distortion /= (float) windowCounter;
  
  if (distortion >= 1.0f && distortion < 1.01f) // avoid floating point accuracy problem
    distortion = 1.0f;
  
  return distortion;
}


float DistortionMetricMSSSIM::computeLuminanceComponent (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  float maxPixelValueSquared = (float) (maxPixelValue * maxPixelValue);
  float C1 = (float) (m_K1 * m_K1 * maxPixelValueSquared);
  float windowPixels = (float) (windowWidth * windowHeight);
  float blockSSIM, meanInp0, meanInp1;
  int   sumInp0, sumInp1;
  float distortion = 0.0f;
  int i, j, n, m, windowCounter = 0;
  
  uint16 *inp0Line = NULL;
  uint16 *inp1Line = NULL;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      sumInp0 = 0;
      sumInp1 = 0;
      
      for ( n = j; n < j + windowHeight; n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          sumInp0   += *inp0Line++;
          sumInp1   += *inp1Line++;
        }
      }
      
      meanInp0 = (float) sumInp0 / windowPixels;
      meanInp1 = (float) sumInp1 / windowPixels;
      
      blockSSIM  = (float) (2.0f * meanInp0 * meanInp1 + C1);
      blockSSIM /= (float) (meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
  
  distortion /= (float) windowCounter;
  
  if (distortion >= 1.0f && distortion < 1.01f) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0f;
  
  return distortion;
}

float DistortionMetricMSSSIM::computeLuminanceComponent (float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue)
{
  float maxPixelValueSquared = (float) (maxPixelValue * maxPixelValue);
  float C1 = (float) (m_K1 * m_K1 * maxPixelValueSquared);
  float windowPixels = (float) (windowWidth * windowHeight);
  float blockSSIM, meanInp0, meanInp1;
  float sumInp0, sumInp1;
  float distortion = 0.0f;
  int i, j, n, m, windowCounter = 0;
  
  float *inp0Line = NULL;
  float *inp1Line = NULL;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      sumInp0 = 0.0f;
      sumInp1 = 0.0f;
      
      for ( n = j; n < j + windowHeight; n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          sumInp0   += *inp0Line++;
          sumInp1   += *inp1Line++;
        }
      }
      
      meanInp0 = sumInp0 / (float) windowPixels;
      meanInp1 = sumInp1 / (float) windowPixels;
      
      blockSSIM  = (2.0f * meanInp0 * meanInp1 + C1);
      blockSSIM /= (meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
  
  distortion /= (float) windowCounter;
  
  if (distortion >= 1.0f && distortion < 1.01f) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0f;
  
  return distortion;
}

void DistortionMetricMSSSIM::horizontalSymmetricExtension(int *buffer, int width, int height )
{
  int j;
  int* buf;
  
  int heightPlusPad2 = height + MS_SSIM_PAD2;
  int widthMinusOne  = width - 1;
  
  // horizontal PSE
  for (j = MS_SSIM_PAD2 * (width + 1); j < heightPlusPad2 * width ; j+= width ) {
    // left column
    buf     = buffer + j;
    buf[-1] = buf[1];
    buf[-2] = buf[2];
    
    // right column
    buf   += widthMinusOne;
    buf[1] = buf[-1];
    buf[2] = buf[-2];
    buf[3] = buf[-3];
  }
}

void DistortionMetricMSSSIM::horizontalSymmetricExtension(float *buffer, int width, int height )
{
  int j;
  float* buf;
  
  int heightPlusPad2 = height + MS_SSIM_PAD2;
  int widthMinusOne  = width - 1;
  
  // horizontal PSE
  for (j = MS_SSIM_PAD2 * (width + 1); j < heightPlusPad2 * width ; j+= width ) {
    // left column
    buf     = buffer + j;
    buf[-1] = buf[1];
    buf[-2] = buf[2];
    
    // right column
    buf   += widthMinusOne;
    buf[1] = buf[-1];
    buf[2] = buf[-2];
    buf[3] = buf[-3];
  }
}

void DistortionMetricMSSSIM::verticalSymmetricExtension(int *buffer, int width, int height)
{
  memcpy(&buffer[(MS_SSIM_PAD2 - 3) * width], &buffer[(MS_SSIM_PAD2 + 3) * width], (width + MS_SSIM_PAD) * sizeof(int));
  memcpy(&buffer[(MS_SSIM_PAD2 - 2) * width], &buffer[(MS_SSIM_PAD2 + 2) * width], (width + MS_SSIM_PAD) * sizeof(int));
  memcpy(&buffer[(MS_SSIM_PAD2 - 1) * width], &buffer[(MS_SSIM_PAD2 + 1) * width], (width + MS_SSIM_PAD) * sizeof(int));
  memcpy(&buffer[(height + MS_SSIM_PAD2  ) * width], &buffer[(height + MS_SSIM_PAD2-2) * width], (width + MS_SSIM_PAD) * sizeof(int));
  memcpy(&buffer[(height + MS_SSIM_PAD2+1) * width], &buffer[(height + MS_SSIM_PAD2-3) * width], (width + MS_SSIM_PAD) * sizeof(int));
  memcpy(&buffer[(height + MS_SSIM_PAD2+2) * width], &buffer[(height + MS_SSIM_PAD2-4) * width], (width + MS_SSIM_PAD) * sizeof(int));
}

void DistortionMetricMSSSIM::verticalSymmetricExtension(float *buffer, int width, int height)
{
  memcpy(&buffer[(MS_SSIM_PAD2 - 3) * width], &buffer[(MS_SSIM_PAD2 + 3) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(MS_SSIM_PAD2 - 2) * width], &buffer[(MS_SSIM_PAD2 + 2) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(MS_SSIM_PAD2 - 1) * width], &buffer[(MS_SSIM_PAD2 + 1) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(height + MS_SSIM_PAD2  ) * width], &buffer[(height + MS_SSIM_PAD2-2) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(height + MS_SSIM_PAD2+1) * width], &buffer[(height + MS_SSIM_PAD2-3) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(height + MS_SSIM_PAD2+2) * width], &buffer[(height + MS_SSIM_PAD2-4) * width], (width + MS_SSIM_PAD) * sizeof(float));
}

void DistortionMetricMSSSIM::padImage(const uint8* src, int *buffer, int width, int height)
{
  int i, j;
  int* tmpDst = buffer + (MS_SSIM_PAD2 * width);
  const uint8* tmpSrc;
  
  for (j = 0; j < height; j++) {
    tmpSrc = src + j * width;
    tmpDst = &buffer[(j + MS_SSIM_PAD2) * width + MS_SSIM_PAD2];
    for (i = 0; i < width; i++) {
      tmpDst[i] = (int)tmpSrc[i];
    }
  }
}

void DistortionMetricMSSSIM::padImage(const uint16* src, int *buffer, int width, int height)
{
  int i, j;
  int* tmpDst = buffer + (MS_SSIM_PAD2 * width);
  const uint16* tmpSrc;
  
  for (j = 0; j < height; j++) {
    tmpSrc = src + j * width;
    tmpDst = &buffer[(j + MS_SSIM_PAD2) * width + MS_SSIM_PAD2];
    for (i = 0; i < width; i++) {
      tmpDst[i] = (int)tmpSrc[i];
    }
  }
}

void DistortionMetricMSSSIM::padImage(const float* src, float *buffer, int width, int height)
{
  int i, j;
  float* tmpDst = buffer + (MS_SSIM_PAD2 * width);
  const float* tmpSrc;
  
  for (j = 0; j < height; j++) {
    tmpSrc = src + j * width;
    tmpDst = &buffer[(j + MS_SSIM_PAD2) * width + MS_SSIM_PAD2];
    for (i = 0; i < width; i++) {
      tmpDst[i] = tmpSrc[i];
    }
  }
}

void DistortionMetricMSSSIM::downsample(const uint8* src, uint8* out, int iWidth, int iHeight, int oWidth, int oHeight)
{
  int i, j;
  int ii, jj;
  int tmp;
  int *tmpDst, *tmpSrc;
  
  vector<int> itemp((iHeight + MS_SSIM_PAD) *( iWidth + MS_SSIM_PAD));
  vector<int> dest ((iHeight + MS_SSIM_PAD) *( oWidth + MS_SSIM_PAD));
  int *p_destM1, *p_destM2, *p_destM0, *p_destP1, *p_destP2, *p_destP3;
  uint8 *pOut;
  static const int downSampleFilter[3] = { 1, 3, 28};
  static const int normalize = 6;
  
  padImage(src, &itemp[0], iWidth, iHeight);
  horizontalSymmetricExtension(&itemp[0], iWidth, iHeight);
  
  for (j = MS_SSIM_PAD2; j < iHeight + MS_SSIM_PAD2; j++) {
    tmpDst = &dest [j * oWidth + MS_SSIM_PAD2];
    tmpSrc = &itemp[j * iWidth + MS_SSIM_PAD2];
    for (i = 0; i < oWidth; i++) {
      ii = (i << 1);
      tmp  =
      downSampleFilter[0] * (tmpSrc[ii - 2] + tmpSrc[ii + 3]) +
      downSampleFilter[1] * (tmpSrc[ii - 1] + tmpSrc[ii + 2]) +
      downSampleFilter[2] * (tmpSrc[ii    ] + tmpSrc[ii + 1]);
      
      tmpDst[i] = iShiftRightRound(tmp, normalize);
    }
  }
  
  //Periodic extension
  verticalSymmetricExtension(&dest[0], oWidth, iHeight);
  
  for (j = 0; j < oHeight; j++)  {
    jj = (j << 1) + 1;
    p_destM2 = &dest[jj * oWidth + MS_SSIM_PAD2];
    p_destM1 = p_destM2 + oWidth;
    p_destM0 = p_destM1 + oWidth;
    p_destP1 = p_destM0 + oWidth;
    p_destP2 = p_destP1 + oWidth;
    p_destP3 = p_destP2 + oWidth;
    pOut    = out + j * oWidth;
    for (i = 0; i < oWidth; i++)    {
      tmp  =
      downSampleFilter[0] * (p_destM2[i] + p_destP3[i]) +
      downSampleFilter[1] * (p_destM1[i] + p_destP2[i]) +
      downSampleFilter[2] * (p_destM0[i] + p_destP1[i]);
      pOut[i] = (unsigned char) iShiftRightRound(tmp, normalize);   //Note: Should change for different bit depth
    }
  }
}

void DistortionMetricMSSSIM::downsample(const uint16* src, uint16* out, int iWidth, int iHeight, int oWidth, int oHeight)
{
  int i, j;
  int ii, jj;
  int tmp;
  int *tmpDst, *tmpSrc;
  
  vector<int> itemp((iHeight + MS_SSIM_PAD) *( iWidth + MS_SSIM_PAD));
  vector<int> dest ((iHeight + MS_SSIM_PAD) *( oWidth + MS_SSIM_PAD));
  int *p_destM1, *p_destM2, *p_destM0, *p_destP1, *p_destP2, *p_destP3;
  uint16 *pOut;
  static const int downSampleFilter[3] = { 1, 3, 28};
  static const int normalize = 6;
  
  padImage(src, &itemp[0], iWidth, iHeight);
  horizontalSymmetricExtension(&itemp[0], iWidth, iHeight);
  
  for (j = MS_SSIM_PAD2; j < iHeight + MS_SSIM_PAD2; j++) {
    tmpDst = &dest [j * oWidth + MS_SSIM_PAD2];
    tmpSrc = &itemp[j * iWidth + MS_SSIM_PAD2];
    for (i = 0; i < oWidth; i++) {
      ii = (i << 1);
      tmp  =
      downSampleFilter[0] * (tmpSrc[ii - 2] + tmpSrc[ii + 3]) +
      downSampleFilter[1] * (tmpSrc[ii - 1] + tmpSrc[ii + 2]) +
      downSampleFilter[2] * (tmpSrc[ii    ] + tmpSrc[ii + 1]);
      
      tmpDst[i] = iShiftRightRound(tmp, normalize);
    }
  }
  
  //Periodic extension
  verticalSymmetricExtension(&dest[0], oWidth, iHeight);
  
  for (j = 0; j < oHeight; j++)  {
    jj = (j << 1) + 1;
    p_destM2 = &dest[jj * oWidth + MS_SSIM_PAD2];
    p_destM1 = p_destM2 + oWidth;
    p_destM0 = p_destM1 + oWidth;
    p_destP1 = p_destM0 + oWidth;
    p_destP2 = p_destP1 + oWidth;
    p_destP3 = p_destP2 + oWidth;
    pOut    = out + j * oWidth;
    for (i = 0; i < oWidth; i++) {
      tmp  =
      downSampleFilter[0] * (p_destM2[i] + p_destP3[i]) +
      downSampleFilter[1] * (p_destM1[i] + p_destP2[i]) +
      downSampleFilter[2] * (p_destM0[i] + p_destP1[i]);
      
      pOut[i] = (unsigned char) iShiftRightRound(tmp, normalize);   //Note: Should change for different bit depth
    }
  }
}

void DistortionMetricMSSSIM::downsample(const float* src, float* out, int iWidth, int iHeight, int oWidth, int oHeight)
{
  int i, j;
  int ii, jj;
  float *tmpDst, *tmpSrc;
  
  vector<float> itemp((iHeight + MS_SSIM_PAD) *( iWidth + MS_SSIM_PAD));
  vector<float> dest ((iHeight + MS_SSIM_PAD) *( oWidth + MS_SSIM_PAD));
  float *p_destM1, *p_destM2, *p_destM0, *p_destP1, *p_destP2, *p_destP3;
  float *pOut;
  static const float downSampleFilter[3] = { 1.0f/64.0f, 3.0f/64.0f, 28.0f/64.0f};
  
  padImage(src, &itemp[0], iWidth, iHeight);
  horizontalSymmetricExtension(&itemp[0], iWidth, iHeight);
  
  for (j = MS_SSIM_PAD2; j < iHeight + MS_SSIM_PAD2; j++) {
    tmpDst = &dest [j * oWidth + MS_SSIM_PAD2];
    tmpSrc = &itemp[j * iWidth + MS_SSIM_PAD2];
    for (i = 0; i < oWidth; i++) {
      ii = (i << 1);
      tmpDst[i]  =
      downSampleFilter[0] * (tmpSrc[ii - 2] + tmpSrc[ii + 3]) +
      downSampleFilter[1] * (tmpSrc[ii - 1] + tmpSrc[ii + 2]) +
      downSampleFilter[2] * (tmpSrc[ii    ] + tmpSrc[ii + 1]);
    }
  }
  
  //Periodic extension
  verticalSymmetricExtension(&dest[0], oWidth, iHeight);
  
  for (j = 0; j < oHeight; j++)  {
    jj = (j << 1) + 1;
    p_destM2 = &dest[jj * oWidth + MS_SSIM_PAD2];
    p_destM1 = p_destM2 + oWidth;
    p_destM0 = p_destM1 + oWidth;
    p_destP1 = p_destM0 + oWidth;
    p_destP2 = p_destP1 + oWidth;
    p_destP3 = p_destP2 + oWidth;
    pOut    = out + j * oWidth;
    for (i = 0; i < oWidth; i++)   {
      pOut[i]  =
      downSampleFilter[0] * (p_destM2[i] + p_destP3[i]) +
      downSampleFilter[1] * (p_destM1[i] + p_destP2[i]) +
      downSampleFilter[2] * (p_destM0[i] + p_destP1[i]);
    }
  }
}

float DistortionMetricMSSSIM::compute(uint8 *inp0Data, uint8 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  float structural[MAX_SSIM_LEVELS];
  float distortion;
  float luminance;
  int   l_width  = width >> 1;
  int   l_height = height >> 1;
  vector<uint8> dsRef(l_height * l_width);
  vector<uint8> dsEnc(l_height * l_width);
  int m;
  static const int max_ssim_levels_minus_one = MAX_SSIM_LEVELS - 1;
  
  structural[0] = computeStructuralComponents(inp0Data, inp1Data, height, width, windowHeight, windowWidth, comp, maxPixelValue);
  distortion = (float) pow(structural[0], m_exponent[0]);
  
  downsample(inp0Data, &dsRef[0], width, height, l_width, l_height);
  downsample(inp1Data, &dsEnc[0], width, height, l_width, l_height);
  
  for (m = 1; m < MAX_SSIM_LEVELS; m++) {
    width  = l_width;
    height = l_height;
    l_width  = width >> 1;
    l_height = height >> 1;
    
    structural[m] = computeStructuralComponents(&dsRef[0], &dsEnc[0], height, width, iMin(windowHeight,height), iMin(windowWidth,width), comp, maxPixelValue);
    distortion *= (float)pow(structural[m], m_exponent[m]);
    
    if (m < max_ssim_levels_minus_one) {
      downsample(&dsRef[0], &dsRef[0], width, height, l_width, l_height);
      downsample(&dsEnc[0], &dsEnc[0], width, height, l_width, l_height);
    }
    else {
      luminance = computeLuminanceComponent(&dsRef[0], &dsEnc[0], height, width, iMin(windowHeight,height), iMin(windowWidth,width), comp, maxPixelValue);
      distortion *= (float)pow(luminance, m_exponent[m]);
    }
  }  
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return distortion;
}


float DistortionMetricMSSSIM::compute(uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int maxPixelValue)
{
  float structural[MAX_SSIM_LEVELS];
  float distortion;
  float luminance;
  int   l_width  = width >> 1;
  int   l_height = height >> 1;
  vector<uint16> dsRef(l_height * l_width);
  vector<uint16> dsEnc(l_height * l_width);
  int m;
  static const int max_ssim_levels_minus_one = MAX_SSIM_LEVELS - 1;
  
  structural[0] = computeStructuralComponents(inp0Data, inp1Data, height, width, windowHeight, windowWidth, comp, maxPixelValue);
  distortion = (float)pow(structural[0], m_exponent[0]);
  
  downsample(inp0Data, &dsRef[0], width, height, l_width, l_height);
  downsample(inp1Data, &dsEnc[0], width, height, l_width, l_height);
  
  for (m = 1; m < MAX_SSIM_LEVELS; m++) {
    width  = l_width;
    height = l_height;
    l_width  = width >> 1;
    l_height = height >> 1;
    
    structural[m] = computeStructuralComponents(&dsRef[0], &dsEnc[0], height, width, iMin(windowHeight,height), iMin(windowWidth,width), comp, maxPixelValue);
    distortion *= (float)pow(structural[m], m_exponent[m]);
    
    if (m < max_ssim_levels_minus_one) {
      downsample(&dsRef[0], &dsRef[0], width, height, l_width, l_height);
      downsample(&dsEnc[0], &dsEnc[0], width, height, l_width, l_height);
    }
    else {
      luminance = computeLuminanceComponent(&dsRef[0], &dsEnc[0], height, width, iMin(windowHeight,height), iMin(windowWidth,width), comp, maxPixelValue);
      distortion *= (float)pow(luminance, m_exponent[m]);
    }
  }
  
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return distortion;
}

float DistortionMetricMSSSIM::compute(float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue)
{
  float structural[MAX_SSIM_LEVELS];
  float luminance;
  int   l_width  = width >> 1;
  int   l_height = height >> 1;
  vector<float> dsRef(l_height * l_width);
  vector<float> dsEnc(l_height * l_width);

  int m;
  static const int max_ssim_levels_minus_one = MAX_SSIM_LEVELS - 1;
  
  structural[0] = computeStructuralComponents(inp0Data, inp1Data, height, width, windowHeight, windowWidth, comp, maxPixelValue);
  float distortion = (float)pow(structural[0], m_exponent[0]);
  
  downsample(inp0Data, &dsRef[0], width, height, l_width, l_height);
  downsample(inp1Data, &dsEnc[0], width, height, l_width, l_height);
  
  for (m = 1; m < MAX_SSIM_LEVELS; m++)  {
    width  = l_width;
    height = l_height;
    l_width  = width >> 1;
    l_height = height >> 1;
    
    structural[m] = computeStructuralComponents(&dsRef[0], &dsEnc[0], height, width, iMin(windowHeight,height), iMin(windowWidth,width), comp, maxPixelValue);
    distortion *= (float) pow(structural[m], m_exponent[m]);
    
    if (m < max_ssim_levels_minus_one)  {
      downsample(&dsRef[0], &dsRef[0], width, height, l_width, l_height);
      downsample(&dsEnc[0], &dsEnc[0], width, height, l_width, l_height);
    }
    else  {
      luminance = computeLuminanceComponent(&dsRef[0], &dsEnc[0], height, width, iMin(windowHeight,height), iMin(windowWidth,width), comp, maxPixelValue);
      distortion *= (float)pow(luminance, m_exponent[m]);
    }
  }
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return distortion;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricMSSSIM::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_height[c], inp0->m_width[c], m_blockSizeY, m_blockSizeX, c, (float) m_maxValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_comp[c], inp1->m_comp[c], inp0->m_height[c], inp0->m_width[c], m_blockSizeY, m_blockSizeX, c, (int) m_maxValue[c]);

        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else { // 16 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_metric[c] = (double) compute(inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_height[c], inp0->m_width[c], m_blockSizeY, m_blockSizeX, c, (int) m_maxValue[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}


void DistortionMetricMSSSIM::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  // Currently no TF is applied on the data. However, we may wish to apply some TF, e.g. PQ or combination of PQ and PH.
  
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      m_metric[component] = (double) compute(inp0->m_floatComp[component], inp1->m_floatComp[component], inp0->m_height[component], inp0->m_width[component], m_blockSizeY, m_blockSizeX, component, (float) m_maxValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      m_metric[component] = (double) compute(inp0->m_comp[component], inp1->m_comp[component], inp0->m_height[component], inp0->m_width[component], m_blockSizeY, m_blockSizeX, component, (int) m_maxValue[component]);
      
      m_metricStats[component].updateStats(m_metric[component]);
    }
    else { // 16 bit data
      m_metric[component] = (double) compute(inp0->m_ui16Comp[component], inp1->m_ui16Comp[component], inp0->m_height[component], inp0->m_width[component], m_blockSizeY, m_blockSizeX, component, (int) m_maxValue[component]);
      m_metricStats[component].updateStats(m_metric[component]);
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricMSSSIM::reportMetric  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
}

void DistortionMetricMSSSIM::reportSummary  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
}

void DistortionMetricMSSSIM::reportMinimum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
}

void DistortionMetricMSSSIM::reportMaximum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
}

void DistortionMetricMSSSIM::printHeader()
{
  if (m_isWindow == FALSE ) {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf(" MSSSIM-Y  "); // 11
        printf(" MSSSIM-U  "); // 11
        printf(" MSSSIM-V  "); // 11
        break;
      case CM_RGB:
        printf(" MSSSIM-R  "); // 11
        printf(" MSSSIM-G  "); // 11
        printf(" MSSSIM-B  "); // 11
        break;
      case CM_XYZ:
        printf(" MSSSIM-X  "); // 11
        printf(" MSSSIM-Y  "); // 11
        printf(" MSSSIM-Z  "); // 11
        break;
      default:
        printf(" MSSSIM-C0 "); // 11
        printf(" MSSSIM-C1 "); // 11
        printf(" MSSSIM-C2 "); // 11
        break;
    }
  }
  else {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("wMSSSIM-Y  "); // 11
        printf("wMSSSIM-U  "); // 11
        printf("wMSSSIM-V  "); // 11
        break;
      case CM_RGB:
        printf("wMSSSIM-R  "); // 11
        printf("wMSSSIM-G  "); // 11
        printf("wMSSSIM-B  "); // 11
        break;
      case CM_XYZ:
        printf("wMSSSIM-X  "); // 11
        printf("wMSSSIM-Y  "); // 11
        printf("wMSSSIM-Z  "); // 11
        break;
      default:
        printf("wMSSSIM-C0 "); // 11
        printf("wMSSSIM-C1 "); // 11
        printf("wMSSSIM-C2 "); // 11
        break;
    }
  }
}

void DistortionMetricMSSSIM::printSeparator(){
  printf("-----------");
  printf("-----------");
  printf("-----------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
