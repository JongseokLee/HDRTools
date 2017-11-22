/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Samsung Electronics
 * <ORGANIZATION> = Samsung Electronics
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, Samsung Electronics
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


/* 
*************************************************************************************
 * \file DistortionMetricVQM.H
 *
 * \brief
 *    Distortion VQM Metric class Header
 *
 *    Computes HDRVQM score for input video sequences
 *    The code is based on the following work:
 *    M. Narwaria, M. P. Da Silva, and P. Le. Callet, 
 *    “HDR-VQM: An Objective Quality Measure for High Dynamic Range Video", 
 *    Signal Processing: Image Communication, 2015. 
 *    https://sites.google.com/site/narwariam/HDR-VQM_SPIC.pdf?attredirects=0&d=1
 *    https://sites.google.com/site/narwariam/hdr-vqm/
 *
 * \author
 *     - Sarvesh Sahota         <sa.sahota@samsung.com>
 *     - Kulbhushan Pachauri	  <kb.pachauri@samsung.com>
 *************************************************************************************
 */
#ifndef __DistortionMetricVQM_H__
#define __DistortionMetricVQM_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "FFT.H"
#include "ResizeBiCubic.H"
#include <vector>


#define VQM_DUMP_STATS 0

static const double PI_VQM = 3.1415926535;


class DistortionMetricVQM : public DistortionMetric {
private:

	//Gabor variables
  vector<Complex> m_fftIn0;
  vector<Complex> m_fftIn1;

  vector<Complex> m_fftOut0;
  vector<Complex> m_fftOut1;

	vector<float>   m_rszIn0;
	vector<float>   m_rszIn1;

	vector< vector<float> > m_filters;
	vector< vector<float> > m_subBandError;
	
	vector<float> m_poolError;

	float m_maxVideo[2];

	float m_vqmScore;
	float m_minLuma;
	float m_maxLuma;
	float m_sortPerc;

	int   m_numberOfFramesFixate;
	int   m_bSize;
	int   m_numberOfFrames;
	int   m_numberOfOrient;
	int   m_numberOfScale;
	int   m_resizeWidth;
	int   m_resizeHeight;
  int   m_resizeSize;
	int   m_poolFrameCnt;
	int   m_stTubeCnt;
	bool  m_displayAdapt;

  int    vqmRound(double value);

	void   initLogGabor();
	void   fftShift(float * mat, int sizeY, int sizeX);

  void   normalizeIntensity(Frame *inp);
	void   adaptDisplay (Frame * in, float percMax, int index);
	int    binarySearch (const double * arrayIn, double valueIn, int start, int end);
	double interp1      (const double * arrayIn, const double * arrayOut, double valueIn);
	void   clipLuminance(Frame * in);
	void   puEncode     (Frame * in);
  void   setColorConversion(int colorPrimaries, const double **transform);
	
	void   biCubic(float * in, int in_width, int in_height, float * out, int out_width, int out_height);
	
	void   calcLogGabor();
	void   spatioTemporalPooling();
	void   longTermPooling();

public:
	// Construct/Deconstruct
	DistortionMetricVQM(const FrameFormat *format, VQMParams *vqmParams);
	virtual ~DistortionMetricVQM();

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