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
 * \file DistortionMetric.cpp
 *
 * \brief
 *    Parent class for distortion metric computation
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "DistortionMetric.H"
#include "DistortionMetricPSNR.H"
#include "DistortionMetricTFPSNR.H"
#include "DistortionMetricmPSNR.H"
#include "DistortionMetricmPSNRfast.H"
#include "DistortionMetricMSSSIM.H"
#include "DistortionMetricSSIM.H"
#include "DistortionMetricDeltaE.H"
#include "DistortionMetricSigmaCompare.H"
#include "DistortionMetricRegionPSNR.H"
#include "DistortionMetricRegionTFPSNR.H"
#include "DistortionMetricTFMSSSIM.H"
#include "DistortionMetricTFSSIM.H"
#include "DistortionMetricBlockinessJ341.H"
#include "DistortionMetricBlockAct.H"
#include "DistortionMetricVQM.H"
#include "DistortionMetricVIF.H"

//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
DistortionMetric::DistortionMetric()
{
  for (int c = ZERO; c < T_COMP; c++) {
    m_metricStats[c].reset();
    m_metric[c] = 0.0;
    m_totalFrames = 0;
  }
  m_isWindow = FALSE;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void MetricStatistics::reset()
{
  sum     = 0.0;
  maximum = -1e30;
  minimum = 1e30;
  counter = 0;
}

void MetricStatistics::updateStats(double value)
{
  sum += value;
  minimum = dMin(minimum, value);
  maximum = dMax(maximum, value);
  counter++;
}

double MetricStatistics::getAverage()
{
  return sum / iMax(1, counter);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
DistortionMetric *DistortionMetric::create(const FrameFormat *format, int distortionMetric, DistortionParameters *distortionParameters, bool isWindow)
{
  DistortionMetric *result = NULL;

  switch (distortionMetric) {
    default:
    case DIST_PSNR:
      result = new DistortionMetricPSNR(format, 
                                        distortionParameters->m_PSNR.m_enableShowMSE, 
                                        distortionParameters->m_maxSampleValue,
                                        distortionParameters->m_PSNR.m_enablexPSNR,
                                        distortionParameters->m_PSNR.m_xPSNRweights
                                        );
      break;
    case DIST_MPSNR:
      result = new DistortionMetricmPSNR(format, 
                                         distortionParameters->m_enableCompmPSNR, 
                                         distortionParameters->m_maxSampleValue);
      break;
    case DIST_MPSNRFAST:
      result = new DistortionMetricmPSNRfast(format, 
                                             distortionParameters->m_enableCompmPSNRfast, 
                                             distortionParameters->m_enableSymmetry);
      break;
    case DIST_TFPSNR:
      result = new DistortionMetricTFPSNR(format, 
                                          &distortionParameters->m_PSNR,
                                          distortionParameters->m_maxSampleValue);
      break;
    case DIST_DELTAE:
      result = new DistortionMetricDeltaE(format, 
                                          distortionParameters->m_PSNR.m_enableShowMSE, 
                                          distortionParameters->m_maxSampleValue, 
                                          distortionParameters->m_whitePointDeltaE, 
                                          distortionParameters->m_deltaEPointsEnable);
      break;
    case DIST_SIGMACOMPARE:
      result = new DistortionMetricSigmaCompare(format, distortionParameters->m_maxSampleValue, distortionParameters->m_amplitudeFactor );
      break;
    case DIST_MSSSIM:
      result = new DistortionMetricMSSSIM(format, 
                                          &distortionParameters->m_SSIM,
                                          distortionParameters->m_maxSampleValue);
      break;
    case DIST_TFMSSSIM:
      result = new DistortionMetricTFMSSSIM(format, 
                                            &distortionParameters->m_SSIM,
                                            distortionParameters->m_maxSampleValue);
      break;
    case DIST_RPSNR:
      result = new DistortionMetricRegionPSNR(format, 
                                              &distortionParameters->m_PSNR,
                                              distortionParameters->m_maxSampleValue);
      break;
    case DIST_RTFPSNR:
    result = new DistortionMetricRegionTFPSNR(format,
                                              &distortionParameters->m_PSNR,
                                              distortionParameters->m_maxSampleValue);
      break;
    case DIST_BLKJ341:
      result = new DistortionMetricBlockinessJ341(distortionParameters->m_maxSampleValue,
                                                  distortionParameters->m_PSNR.m_tfDistortion);
      break;
    case DIST_BLK:
      result = new DistortionMetricBlockAct(distortionParameters->m_maxSampleValue,
                                            distortionParameters->m_PSNR.m_tfDistortion);
      break;
    case DIST_VQM:
      result = new DistortionMetricVQM(format, &distortionParameters->m_VQM);
      break;
    case DIST_VIF:
      result = new DistortionMetricVIF(format, &distortionParameters->m_VIF);
      break;
    case DIST_SSIM:
      result = new DistortionMetricSSIM(format, 
                                          &distortionParameters->m_SSIM,
                                          distortionParameters->m_maxSampleValue);
      break;
    case DIST_TFSSIM:
      result = new DistortionMetricTFSSIM(format,
                                            &distortionParameters->m_SSIM,
                                            distortionParameters->m_maxSampleValue);
      break;
  }

  result->m_clipInputValues = distortionParameters->m_clipInputValues;
  result->m_isWindow        = isWindow;
  return result;
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------


