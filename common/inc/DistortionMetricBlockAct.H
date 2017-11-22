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
 * \file DistortionMetricBlockAct.H
 *
 * \brief
 *    New Blockiness metric based on simple Activity analysis
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __DistortionMetricBlockAct_H__
#define __DistortionMetricBlockAct_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "DistortionTransferFunction.H"
#include <vector>


class DistortionMetricBlockAct : public DistortionMetric {
private:
  DistortionTransferFunction *m_transferFunction;

  vector<double> m_verAct[2];
  vector<double> m_horAct[2];
  vector<double> m_verActDiff;
  vector<double> m_horActDiff;

  vector<double> m_sumW;
  vector<double> m_sumH;
  
  vector<double> m_dSum;
  vector<double> m_dSumDev;
  vector<double> m_dDistance;
  vector<double> m_dOutput;
  
  int     m_width;
  int     m_height;

  
  int m_memWidth;
  int m_memHeight;
  
  int m_blkWidth;
  int m_blkHeight;
  int m_blkWOffset;
  int m_blkHOffset;

  ColorSpace        m_colorSpace;
  double            m_maxValue[T_COMP];
  
  // Wiener filtering functions
  double windowSum(const double *iData, int iPosX, int iWidth);
  void   computeImgWindowSum(const double *iData, int iWidth);
  double computeShortDC(int iWidth);
  void   computeMeanDist(const double *iData, int iWidth);
  void   wienerFilter(const double *iData, double noise,  int iWidth);
  void   filter(double *imgData, int width);
  void   updateImg(double *out, const double *inp, int size);
  
  double avgSubsample(double *data, int step, int offset, int length);
  void computeActivity(float *inpData, int height, int width, int index, double maxValue);
  void compareActivity(int size);
  void accumulateHArray(double *array, double *out, int width, int height, int arraywidth, int posY, int posX);
  void accumulateVArray(double *array, double *out, int width, int height, int arraywidth, int posY, int posX);
  double accumulateVector(double *inVector, int width);
  double shiftAccumulateVector(double *inVector, int width);
  
  float compute                     (uint8  *inp0Data, uint8  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  float compute                     (uint16 *inp0Data, uint16 *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, int   maxPixelValue);
  float compute                     (float  *inp0Data, float  *inp1Data, int height, int width, int windowHeight, int windowWidth, int comp, float maxPixelValue);
  
public:
  // Construct/Deconstruct
  DistortionMetricBlockAct(double maxSampleValue, DistortionFunction distortionMethod);
  virtual ~DistortionMetricBlockAct();
  
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


