/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Apple Inc.
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
 * \file AnalyzeGamut.H
 *
 * \brief
 *    Analyze Gamut process
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __AnalyzeGamut_H__
#define __AnalyzeGamut_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "DistortionTransferFunction.H"

static const int TOTAL_COMPONENTS = 4;

class AnalyzeGamut {
private:
  int                         m_histBinsDiv3;
  int                         m_totalComponents;
  int                         m_counter;

  uint16                      m_minData [TOTAL_COMPONENTS];
  uint16                      m_maxData [TOTAL_COMPONENTS];

  double                      m_totalPixels;
  double                      m_cntMinRGB [TOTAL_COMPONENTS];
  double                      m_cntMinYUV [TOTAL_COMPONENTS];
  double                      m_cntMaxRGB [TOTAL_COMPONENTS];
  double                      m_cntMaxYUV [TOTAL_COMPONENTS];
  double                      m_cntAllRGB;

  MetricStatistics            m_allRGBStats;
  MetricStatistics            m_minStats    [TOTAL_COMPONENTS];
  MetricStatistics            m_maxStats    [TOTAL_COMPONENTS];
  MetricStatistics            m_minRGBStats [TOTAL_COMPONENTS];
  MetricStatistics            m_maxRGBStats [TOTAL_COMPONENTS];
  MetricStatistics            m_minYUVStats [TOTAL_COMPONENTS];
  MetricStatistics            m_maxYUVStats [TOTAL_COMPONENTS];
  
  vector<double>              m_histRGB     [TOTAL_COMPONENTS];
  double                      *m_pHistogram [TOTAL_COMPONENTS];
  
  float convertValue (const imgpel iComp, double weight, double offset);
  float convertValue (const uint16 iComp, double weight, double offset);
  float convertValue (const uint16 iComp,  int bitScale, double weight, double offset);
  float convertValue (const imgpel iComp, double weight, const uint16 offset);
  float convertValue (const uint16 iComp, double weight, const uint16 offset);
  float convertCompValue(uint16 inpValue, SampleRange sampleRange, ColorSpace colorSpace, int bitDepth, int component);
  float convertCompValue(imgpel inpValue, SampleRange sampleRange, ColorSpace colorSpace, int bitDepth, int component);
  
  void   reportHistogram         ();
  void   compute                 (Frame* inp0);
  void   convertToRGB            (double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2);
  void   setColorConversion      (int colorPrimaries, const double **transform0, const double **transform1, const double **transform2);
public:
  // Construct/Deconstruct
  AnalyzeGamut();
  ~AnalyzeGamut();
  
  void   process (Frame* inp);                // Compute metric for all components
  void   reportMetric  ();                                        // report frame level results
  void   reportSummary ();                                        // report summary results
  void   reportMinimum ();
  void   reportMaximum ();
  void   printHeader   ();
  void   printSeparator();
};

#endif


