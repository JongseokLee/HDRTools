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
 * \file DistortionMetricVQM.cpp
 *
 * \brief
 *    Computes HDRVQM score for input video sequences
 *
 *    The code is based on the following work:
 *    M. Narwaria, M. P. Da Silva, and P. Le. Callet, 
 *    “HDR-VQM: An Objective Quality Measure for High Dynamic Range Video", 
 *    Signal Processing: Image Communication, 2015. 
 *    https://sites.google.com/site/narwariam/HDR-VQM_SPIC.pdf?attredirects=0&d=1
 *    https://sites.google.com/site/narwariam/hdr-vqm/
 *
 *    There are small differences in this code versus the original Matlab implementation
 *    a) A different FFT implementation is used that may result in different results.
 *    b) The current implementation only supports FFTs that have power of 2 dimensions.
 *       In the original implementation a size of 896x512 (width x height).
 *    c) This implementation computes the "true" luminance given the colour primaries 
 *       of the signal.
 *
 * \author
 *     - Sarvesh Sahota       <sa.sahota@samsung.com>
 *     - Kulbhushan Pachauri	<kb.pachauri@samsung.com>
 *************************************************************************************
 */

#include "DistortionMetricVQM.H"
#include "ColorTransformGeneric.H"
#include "PUEncode.H"
#include "Frame.H"

#include <algorithm>
#include <functional>
#include <iostream>
#include <cmath>
#include <cassert>

#include <stdio.h>
#include <string.h>

#if defined(ENABLE_SSE_OPT) || defined(ENABLE_AVX_OPT)
#if defined(WIN32) || defined(WIN64)
#include <emmintrin.h>
#else
#include <x86intrin.h>
#include <mmintrin.h>
#endif
#if defined(__SSE3__)
#include <pmmintrin.h>
#endif
#endif

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricVQM::DistortionMetricVQM(const FrameFormat *format, VQMParams *vqmParams)
: DistortionMetric()
{
	m_resizeWidth  = vqmParams->m_columnsFFT;
	m_resizeHeight = vqmParams->m_rowsFFT;
  
  // Currently only powers of two are supported. Resize width/height to the closest power of 2 or fhe original width/height
  m_resizeWidth  = setClosest(m_resizeWidth, clp2(m_resizeWidth), flp2(m_resizeWidth));
  m_resizeHeight = setClosest(m_resizeHeight, clp2(m_resizeHeight), flp2(m_resizeHeight));
  
  m_resizeSize   = m_resizeWidth * m_resizeHeight;
	
  m_vqmScore     = 0.0;
	m_allFrames    = false;
	m_maxVideo[0]  = 0;
	m_maxVideo[1]  = 0;
	m_poolFrameCnt = 0;
	m_stTubeCnt    = 0;

	m_minLuma  = vqmParams->m_minDisplay;
	m_maxLuma  = vqmParams->m_maxDisplay;
	m_sortPerc = vqmParams->m_poolingPerc;
	//TBD
	m_numberOfFrames = vqmParams->m_numberOfFrames;
	m_numberOfOrient = vqmParams->m_numberOfOrient;
	m_numberOfScale  = vqmParams->m_numberOfScale;

  // Always limit m_numberOfFramesFixate by m_numberOfFrames so we can check short clips
	m_numberOfFramesFixate = iMin( m_numberOfFrames, (int)ceil(vqmParams->m_frameRate * vqmParams->m_fixationTime));

	m_displayAdapt = vqmParams->m_displayAdapt;
	
	float bSizeTemp = (float) ((tan(PI_VQM / 90.0 ) * (double) vqmParams->m_viewingDistance * sqrt(((double) vqmParams->m_rowsDisplay * (double) vqmParams->m_colsDisplay)/ (double) vqmParams->m_displayArea)) / 2.0);
  
	int minDiff = INT_MAX, ind = 0, diff;
	for(int i = 2; i < 11; i++)	{
		diff = (int) (dAbs((double) bSizeTemp - pow(2.0,(double) i)));
		if(minDiff > diff)		{
			minDiff = diff;
			ind = i;
		}
	}
	m_bSize = 1 << ind; // pow(2.0, ind);

	//For pooling
	m_subBandError.resize(m_numberOfFramesFixate);
	for(int i = 0; i < m_numberOfFramesFixate; i++)	{
		m_subBandError[i].resize(m_resizeSize);
	}

	int tPoolSize = (int) (ceil((double) m_resizeHeight / m_bSize) * ceil((double)m_resizeWidth / m_bSize) * (m_numberOfFrames / m_numberOfFramesFixate));

	m_poolError.resize(tPoolSize);
	
	m_rszIn0.resize(m_resizeSize);
	m_rszIn1.resize(m_resizeSize);

	m_filters.resize(m_numberOfScale * m_numberOfOrient);
	for(int i = 0; i< m_numberOfOrient * m_numberOfScale; i++){
    m_filters[i].resize(m_resizeSize);
	}

	//For gabor 
  m_fftIn0.resize(m_resizeSize);
  m_fftIn1.resize(m_resizeSize);

  m_fftOut0.resize(m_resizeSize);
  m_fftOut1.resize(m_resizeSize);
	
  initLogGabor();
}

DistortionMetricVQM::~DistortionMetricVQM()
{
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

int DistortionMetricVQM::vqmRound(double value)
{
#if defined _MSC_VER && defined _M_IX86
  int t;
  __asm
  {
    fld value;
    fistp t;
  }
  return t;
#else
  return (int)(value + (value >= 0 ? 0.5 : -0.5));
#endif
}

void DistortionMetricVQM::fftShift(float * in, int height, int width)
{
  int half_h = (height + 1) >> 1;
  int half_w = (width  + 1) >> 1;
  
  for (int i = 0; i < height; i++)	{
    for (int j = 0; j < half_w; j++)	{
      if((width & 0x01) == 0 && (height & 0x01) == 0)	{
        if(i< half_h) {
          swap(&in[i * width + j], &in[(i + half_h) * width + (j + half_w)]);
        }
        else {
          swap(&in[i * width + j], &in[(i - half_h) * width + (j + half_w)]);
        }
      }
    }
  }
}

void DistortionMetricVQM::adaptDisplay(Frame * in, float percMax, int idx)
{
  double tMean = 0; 
  
  for (int i = 0; i < 3; i++) {
    float *Comp = in->m_floatComp[i];
    for(int x = 0; x < in->m_compSize[i]; x++) {
      if(*Comp < 0.0f)
        *Comp = 0.0f;
      Comp++;
    }
  }
  
  std::sort(in->m_floatComp[0], in->m_floatComp[0] + 3 * in->m_compSize[R_COMP], std::less<float>());
  
  int tCount = (int) (3.0 * (double) in->m_compSize[R_COMP] - (double) percMax * 3.0 * (double) in->m_compSize[R_COMP] - 1.0);
  for( ; tCount < 3 * in->m_compSize[R_COMP]; tCount++) {
    tMean += (double) (*(in->m_floatComp))[tCount];
  }
  
  tCount = (int) ((double) percMax * 3.0 * (double) in->m_compSize[R_COMP] + 1.0);
  tMean /= (double) (tCount);
  
  if((double) m_maxVideo[idx] < tMean)
    m_maxVideo[idx] = (float) tMean;
  
  return;
}


void DistortionMetricVQM::setColorConversion(int colorPrimaries, const double **transform) {
  int mode = CTF_IDENTITY;
  
  if (colorPrimaries == CP_709) {
    mode = CTF_RGB709_2_YUV709;
  }
  else if (colorPrimaries == CP_2020) {
    mode = CTF_RGB2020_2_YUV2020;
  }
  else if (colorPrimaries == CP_P3D65) {
    mode = CTF_RGBP3D65_2_YUVP3D65;
  }
  else if (colorPrimaries == CP_601) {
    mode = CTF_RGB601_2_YUV601;
  }
  
  *transform = FWD_TRANSFORM[mode][Y_COMP];
}


void DistortionMetricVQM::clipLuminance(Frame * in)
{
  float *Comp0 = in->m_floatComp[0];
  float *Comp1 = in->m_floatComp[1];
  float *Comp2 = in->m_floatComp[2];
  
  const double *transform = NULL;

  setColorConversion(in->m_colorPrimaries, &transform);

  for(int x = 0; x < in->m_compSize[R_COMP]; x++) {
    
    double rOrig = dClip(*Comp0, 0.0, 65504.0);
    double gOrig = dClip(*Comp1, 0.0, 65504.0);
    double bOrig = dClip(*Comp2, 0.0, 65504.0);
    
    double temp = rOrig * transform[R_COMP] + gOrig * transform[G_COMP] + bOrig * transform[B_COMP];
    double tempClipped = dClip(temp, m_minLuma, m_maxLuma);
    
    if(temp == 0.0) {
      *Comp0 = 0.0f;
      *Comp1 = 0.0f;
      *Comp2 = 0.0f;
    }
    else {
      *Comp0 = (float) ((double) *Comp0 * tempClipped / temp);
      *Comp1 = (float) ((double) *Comp1 * tempClipped / temp);
      *Comp2 = (float) ((double) *Comp2 * tempClipped / temp);
    }
    
    *Comp0 = (float) ((double) (*Comp0) * transform[R_COMP] + (double) (*Comp1) * transform[G_COMP] + (double) (*Comp2) * transform[B_COMP]);
    Comp0++;
    Comp1++;
    Comp2++;
  }
}

int DistortionMetricVQM::binarySearch(const double * in, double val, int start, int end)
{
  if((end - start) <= 1)
    return end;
  
  int tIdx = (start + end)/2;
  int rVal;
  
  if(in[tIdx] < val) {
    rVal = binarySearch(in, val, tIdx, end);
  }
  else if(in[tIdx] > val) {
    rVal = binarySearch(in, val, start, tIdx);
  }
  else
    return tIdx;
  
  return rVal;
}

double DistortionMetricVQM::interp1 (const double * in, const double * out, double val)
{
  int i = binarySearch(in, val, 0, SIZE_PU_ENC);
  
  double t1 = out[i + 1] - out[i];
  double t2 = in [i + 1] - in [i];
  
  return (out[i] + (t1 * (val - in[i])) / t2);
}

void DistortionMetricVQM::puEncode (Frame * in)
{
  double t0, t1;
  
  float *Comp0 = in->m_floatComp[Y_COMP];
  const double ln10 = log(10.0);
  
  for(int x = 0; x < in->m_compSize[Y_COMP]; x++) {
    t0 = log10((double) *Comp0);
    t1 = interp1(PUIn, PUOut, t0);
    *Comp0++ = (float) exp(t1 * ln10);
  }
}

void DistortionMetricVQM::biCubic (float * in, int inWidth, int inHeight, float * out, int outWidth, int outHeight)
{
  if(inWidth == outWidth && inHeight == outHeight) {
    memcpy(out, in, inHeight * inWidth * sizeof(float));
  }
  else {
    ResizeBiCubic::compute(in, inWidth, inHeight, out, outWidth, outHeight, 1);
  }
}

void DistortionMetricVQM::calcLogGabor()
{
#if defined(ENABLE_SSE_OPT) && defined(__SSE3__)
  float arr[4], sim[2];
  __m128 r1, r2, r3, r4;
  float * ptr_i0, * ptr_i1, * ptr_o0, * ptr_o1, * ptr_mul;
  int i, o, s;
  
  int tRows = m_resizeHeight;
  int tCols = m_resizeWidth;
  int tSize = tRows * tCols;
  
  float * t_similarityError = &m_subBandError[m_poolFrameCnt % m_numberOfFramesFixate][0];
  
  for(i=0; i<tRows*tCols; i++) {
    m_fftIn0[i].real = m_rszIn0[i];
    m_fftIn0[i].imag = 0.0f;
    
    m_fftIn1[i].real = m_rszIn1[i];
    m_fftIn1[i].imag = 0.0f;
  }
  
  FFT::compute2D(&m_fftIn0[0], tRows, tCols, 1, tCols);
  FFT::compute2D(&m_fftIn1[0], tRows, tCols, 1, tCols);
  
  m_metric[0] = 0.0;
  
  /* For scale */
  for(s = 0; s < m_numberOfScale; s++) {
    /* For orientation */
    for(o = 0; o < m_numberOfOrient; o++) {
      float *filters = &m_filters[o * m_numberOfScale + s][0];
      ptr_i0 = &(m_fftIn0[0].real);
      ptr_i1 = &(m_fftIn1[0].real);
      ptr_o0 = &(m_fftOut0[0].real);
      ptr_o1 = &(m_fftOut1[0].real);
      ptr_mul = filters;
      
      for(i=0; i<tSize; i+=2)
      {
        r1 = _mm_loadu_ps(ptr_i0);
        r2 = _mm_loadu_ps(ptr_i1);
        r3 = _mm_set_ps(ptr_mul[i+1], ptr_mul[i+1], ptr_mul[i], ptr_mul[i]);
        
        r1 = _mm_mul_ps(r1, r3);
        r2 = _mm_mul_ps(r2, r3);
        
        _mm_storeu_ps(ptr_o0, r1);
        _mm_storeu_ps(ptr_o1, r2);
        
        ptr_i0 += 4;
        ptr_i1 += 4;
        ptr_o0 += 4;
        ptr_o1 += 4;
      }
      FFT::compute2D(&m_fftOut0[0], tRows, tCols, -1, tCols);
      FFT::compute2D(&m_fftOut1[0], tRows, tCols, -1, tCols);
      
      r3 = _mm_set_ps1((float) tSize);
      
      for(i = 0; i < tSize; i += 2) {
        r1 = _mm_loadu_ps((const float *) &m_fftOut0[i]);
        r2 = _mm_loadu_ps((const float *) &m_fftOut1[i]);
        
        r1 = _mm_div_ps(r1, r3);
        r2 = _mm_div_ps(r2, r3);
        
        r1 = _mm_mul_ps(r1, r1);
        r2 = _mm_mul_ps(r2, r2);
        
        r4 = _mm_hadd_ps(r1, r2);
        r4 = _mm_sqrt_ps(r4);
        
        _mm_storeu_ps(arr, r4);
        
        sim[0] = (float) ((2.0 * arr[0] * arr[2] + 0.2) / ( arr[0]*arr[0] + arr[2]*arr[2] + 0.2));
        sim[1] = (float) ((2.0 * arr[1] * arr[3] + 0.2) / ( arr[1]*arr[1] + arr[3]*arr[3] + 0.2));
        
        t_similarityError[i  ]	+= sim[0];
        t_similarityError[i+1]	+= sim[1];
        m_metric[0] += t_similarityError[i];
        m_metric[0] += t_similarityError[i + 1];
      }
      m_metric[0] /= (double) tSize;
    }
  }
//#elif defined(ENABLE_AVX_OPT)
  
#else
  float t0, t1, t2;
  double tSim;
  double t3 = 0.0, t4 = 0.0;
  int i, o, s;
  
  int tRows = m_resizeHeight;
  int tCols = m_resizeWidth;
  int tSize = tRows * tCols;
  
  float * t_similarityError = &m_subBandError[m_poolFrameCnt % m_numberOfFramesFixate][0];
  
  for(i=0; i<tRows*tCols; i++) {
    m_fftIn0[i].real = m_rszIn0[i];
    m_fftIn0[i].imag = 0.0f;
    
    m_fftIn1[i].real = m_rszIn1[i];
    m_fftIn1[i].imag = 0.0f;
  }
  
  FFT::compute2D(&m_fftIn0[0], tRows, tCols, 1, tCols);
  FFT::compute2D(&m_fftIn1[0], tRows, tCols, 1, tCols);
  
  m_metric[0] = 0.0;
  
  /* For scale */
  for(s = 0; s < m_numberOfScale; s++) {
    /* For orientation */
    for(o = 0; o < m_numberOfOrient; o++) {
      float *filters = &m_filters[o * m_numberOfScale + s][0];
      for(i=0; i<tSize; i++) {  
        double vFilter =  (double) filters[i];
        
        m_fftOut0[i].real = (float) ((double) m_fftIn0[i].real * vFilter);
        m_fftOut0[i].imag = (float) ((double) m_fftIn0[i].imag * vFilter);
        m_fftOut1[i].real = (float) ((double) m_fftIn1[i].real * vFilter);
        m_fftOut1[i].imag = (float) ((double) m_fftIn1[i].imag * vFilter);
      }
      FFT::compute2D(&m_fftOut0[0], tRows, tCols, -1, tCols);
      FFT::compute2D(&m_fftOut1[0], tRows, tCols, -1, tCols);
      
      for(i = 0; i < tSize; i++) {
        t0 = (float) ((double) m_fftOut0[i].real / (double) tSize);
        t2 = (float) ((double) m_fftOut0[i].imag / (double) tSize);
        t3 = ((double) t0 * (double) t0 + (double) t2 * (double) t2);
        t3 = sqrt(t3);
        
        t1 = (float) ((double) m_fftOut1[i].real / (double) (tSize));
        t2 = (float) ((double) m_fftOut1[i].imag / (double) (tSize));
        t4 = ((double) t1 * (double) t1 + (double) t2 * (double) t2);
        t4 = sqrt(t4);
        
        //calculating similarity
        tSim = (2.0 * t3 * t4 + 0.2)/(t3 * t3 + t4 * t4 + 0.2);
        
        //taking mod of complex number and saving in frame val
        //from now, we have the error image in the 0th frame. 
        //No need of second frame now!
        t_similarityError[i] = (float) ((double) t_similarityError[i] + tSim);
        m_metric[0] += (double) t_similarityError[i];
      }
      m_metric[0] /= (double) tSize;
    }
  }
  
#endif
  
  return;
}

void DistortionMetricVQM::spatioTemporalPooling()
{	
  int totalSamples = m_bSize * m_bSize * m_numberOfFramesFixate;
  vector<float> tStdArr(totalSamples);
  
  float * frameSubBandError = NULL;
  double tMean = 0.0, tStdSum = 0.0;
  int tidx, i = 0;
  int stTubeIdx = (int)(m_stTubeCnt * ceil((double) m_resizeHeight / m_bSize) * m_resizeWidth / m_bSize);

  
  for(int row = 0; row < m_resizeHeight; row += m_bSize) {
    for(int col = 0; col < m_resizeWidth; col += m_bSize) {
      for(int frame = 0; frame < m_numberOfFramesFixate; frame++) {
        frameSubBandError = &m_subBandError[frame][0];
        for(int stRow = row; (stRow < row + m_bSize) && (stRow < m_resizeHeight); stRow++) {
          for(int stCol = col; (stCol < col + m_bSize) && (stCol < m_resizeWidth); stCol++) {
            tidx = stRow * m_resizeWidth + stCol;
            tStdArr[i] = frameSubBandError[tidx];
            frameSubBandError[tidx] = 0.0f;
            tMean += (double) tStdArr[i++];
          }
        }
      }
      //here we have all elements of one st-tube
      
      //below is the code for short term spatio-temporal pooling
      tMean /= (double) totalSamples;
      for(int j = 0; j < totalSamples; j++) {
        tStdSum += ((double) tStdArr[j] - tMean) * ((double) tStdArr[j] - tMean);
        tStdArr[j] = 0.0f;
      }

      //all the values of st-tubes stored contiguously in Frame.
      //m_poolError[(int)(m_stTubeCnt * ceil((double) m_resizeHeight / m_bSize) * m_resizeWidth / m_bSize) + ((row * m_resizeWidth) / (m_bSize * m_bSize)) + col / m_bSize] = (float) sqrt(tStdSum / (double) totalSamples);
      m_poolError[stTubeIdx + ((row * m_resizeWidth) / (m_bSize * m_bSize)) + col / m_bSize] = (float) sqrt(tStdSum / (double) totalSamples);

      tMean = 0.0;
      tStdSum = 0.0;
      i = 0;
    }
  }
}

void DistortionMetricVQM::longTermPooling()
{
  int tRows = ceilDivide(m_resizeHeight, m_bSize); // (int)ceil((double)m_resizeHeight / (double) m_bSize);
  int tCols = ceilDivide(m_resizeWidth, m_bSize);  // (int)ceil((double)m_resizeWidth  / (double) m_bSize);
  
  int tMax = m_numberOfFrames/m_numberOfFramesFixate;
  
  int tSize = tRows * tCols;
  int tSize1 = vqmRound((tSize-1) * m_sortPerc) + 1;
  
  // Add one to avoid problem with the subsequent tMax computation
  vector<float> tArr(tMax + 1);

  memset(&tArr[0], 0, sizeof(float)*(tMax + 1));
  
  for(int i = 0; i < tMax; i++)	{
    std::sort(&m_poolError[i*tSize], &m_poolError[i*tSize+tSize], std::less<float>());
    for( int kkk = 0; kkk < tSize1; kkk++) {
      tArr[i] =  (float) ((double) tArr[i] + (double) m_poolError[i*tSize+kkk]);
    }
    tArr[i] = (float) ((double) tArr[i] / (double) tSize1);
  }
  
  std::sort(&tArr[0], &tArr[tMax], std::less<float>());
  tMax = vqmRound((double) (tMax - 1) * (double) m_sortPerc) + 1;
  for(int i = 0; i < tMax; i++)	{
    m_vqmScore = (float) ((double) m_vqmScore + (double) tArr[i]);
  }
  m_vqmScore =  (float) ((double) m_vqmScore / (double) tMax);
  
  //printf("longTermPooling %f\n", m_vqmScore);
  //if(m_vqmScore != 0)
  //	m_vqmScore = 1.0f/m_vqmScore;
  //else
  //	m_vqmScore = 999.0f;
  //printf("longTermPooling %f\n", m_vqmScore);
}

void DistortionMetricVQM::initLogGabor()
{	
	float x_x;
  double logRfo;
	double angl, cosAngl, sinAngl;
  double aTan2;

	const double dThetaOnSigma = 1.5;
	const double sigmaOnF = 0.55;
	const double logSigmaScale = (2.0 * log(sigmaOnF) * log(sigmaOnF));
  const double minWaveLength = 3.0;
  const double mult = 3.0;
  const double logMult = log(mult);
	const double thetaSigma = (PI_VQM /((double) m_numberOfOrient * dThetaOnSigma));
  const double thetaSigmaSqr = 2.0 * thetaSigma * thetaSigma;
  const double lnMinWaveLength = log(2.0) - log(minWaveLength);
  
  int i, j, o, s;

  vector<float> x        (m_resizeSize);
  vector<float> y        (m_resizeSize);
  vector<float> logRadius(m_resizeSize);
	vector<float> sinTheta (m_resizeSize);
	vector<float> cosTheta (m_resizeSize);
  vector<float> spread   (m_resizeSize);
  vector<float> logGabor (m_resizeSize);
  vector<float> y_y      (m_resizeWidth);
  vector<float> logWaveLength(m_numberOfScale);
  double ds;
  double dc;

  for(int i=0; i<m_numberOfScale; i++)	{
    logWaveLength[i] = (float) (lnMinWaveLength - (double) i * logMult);
  }

	for (i = 0; i < m_resizeHeight; i++)	{
    x_x = (float)(((double) (2 * i - m_resizeHeight ) / (double) m_resizeHeight ));
		for ( j = 0; j < m_resizeWidth; j++)		{
			y[i * m_resizeWidth + j] = x_x;
		}
	}

	for (i = 0; i < m_resizeWidth; i++)	{
    y_y[i] = (float)(((double) (2 * i - m_resizeWidth) / (double) m_resizeWidth ));
	}

	for (j = 0; j < m_resizeSize; j += m_resizeWidth)
		memcpy(&x[j],&y_y[0], sizeof(float) * m_resizeWidth);

	for (i = 0; i < m_resizeSize; i++)	{
    logRadius[i] = (float) log((sqrt(((double) x[i] * (double) x[i]) + ((double) y[i] * (double) y[i]))));
    aTan2        = atan2((double) y[i],  (double) x[i]);
		sinTheta [i] = (float) (sin(aTan2));
		cosTheta [i] = (float) (cos(aTan2));
	}

  logRadius[(m_resizeSize + m_resizeHeight) >> 1] = 0.0f;

	for (o = 0; o < m_numberOfOrient; o++)	{
		angl = ((double) o * PI_VQM / (double) m_numberOfOrient);
    cosAngl = cos(angl);
    sinAngl = sin(angl);
		for(i = 0; i < m_resizeSize; i++)		{
	    ds = ((double) sinTheta[i] * cosAngl - (double) cosTheta[i] * sinAngl);
			dc = ((double) cosTheta[i] * cosAngl + (double) sinTheta[i] * sinAngl);
			aTan2 = atan2(ds, dc);
      spread[i] = (float) (exp(-(aTan2 * aTan2) / thetaSigmaSqr));
		}

		/* For scale */
		for (s = 0; s < m_numberOfScale; s++)		{
      float *filters = &m_filters[o * m_numberOfScale + s][0];

      logRfo = (double) logWaveLength[s];
			for(i = 0; i < m_resizeSize; i++)	{
        double curLogRadius = (double) logRadius[i] - logRfo;
				logGabor[i] = (float) (exp(-(curLogRadius * curLogRadius) / logSigmaScale));
			}
			logGabor[(m_resizeSize + m_resizeHeight) >> 1 ] = 0.0f;

      for(i = 0; i < m_resizeSize; i++)	{
				filters[i] = (float) ((double) spread[i] * (double) logGabor[i]);
			}

			//fftshift
			fftShift(filters, m_resizeHeight, m_resizeWidth);
		}
	}
}

void DistortionMetricVQM::normalizeIntensity(Frame *inp) {
  for (int i = 0; i < 3; i++ ) {
    float *Comp = inp->m_floatComp[i];
    for(int xx = 0; xx < inp->m_compSize[i]; xx++) {
      if(*Comp < 0.0f)
        *Comp = 0.0f;
      else
        *Comp = (float) ((double) *Comp * (double) m_maxLuma / (double) m_maxVideo[0]);
      Comp++;
    }
  }
}


#if(VQM_DUMP_STATS == 1)

void dumpFileDouble(char * f_name, double * var, int rows, int cols)
{
  FILE *f = fopen(f_name, "w");
  for(int jh = 0; jh < rows * cols; jh++) {
    fprintf(f, "%lf\n", var[jh]);
  }
  fclose(f);
}

void dumpFileFloat(char * f_name, float * var, int rows, int cols)
{
  FILE *f = fopen(f_name, "w");
  for(int jh = 0; jh < rows * cols; jh++) {
    fprintf(f, "%f\n", var[jh]);
  }
  fclose(f);
}

void dumpFileFloatIntA(char * f_name, float var, int count)
{
  FILE *f = fopen(f_name, "a");
  fprintf(f, "%f\n", var);
  fprintf(f, "%d\n", count);
  fclose(f);
}

void dumpFileFloat2(char * f_name, Complex * var, int rows, int cols, int i)
{
  char temp[256];
  sprintf(temp,"%d",i);
  strcat(temp, f_name);
  std::cout<<"****** "<< temp<<std::endl;
  FILE *f = fopen(temp, "w");
  for(int jh = 0; jh < rows * cols; jh++) {
    fprintf(f, "%f\n", var[jh].real);
  }
  fclose(f);
}

void dumpFileComplex(char * f_name, char * f_name2, Complex * var, int rows, int cols)
{
  FILE *f = fopen(f_name, "a");
  for(int jh = 0; jh < rows * cols; jh++) {
    fprintf(f, "%f\n", var[jh].real);
  }
  fclose(f);
}

void dumpFileFloat3(char * f_name, float * var, int rows, int cols, int i)
{
  char temp[256];
  sprintf(temp,"%d",i);
  strcat(temp, f_name);
  std::cout<<"****** "<< temp<<std::endl;
  FILE *f = fopen(temp, "w");
  for(int jh = 0; jh < rows * cols; jh++) {
    fprintf(f, "%f\n", var[jh]);
  }
  fclose(f);
}

void dumpFileDouble3(char * f_name, double * var, int rows, int cols, int i)
{
  char temp[256];
  sprintf(temp,"%d",i);
  strcat(temp, f_name);
  std::cout<<"****** "<< temp<<std::endl;
  FILE *f = fopen(temp, "w");
  for(int jh = 0; jh < rows * cols; jh++) {
    fprintf(f, "%lf\n", var[jh]);
  }
  fclose(f);
}

#endif

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricVQM::computeMetric(Frame* inp0, Frame* inp1)
{
  if (inp0->m_colorSpace != CM_RGB || inp1->m_colorSpace != CM_RGB) {
    printf("Only RGB data currently supported.\n");
    return;
  }
  if (inp0->m_isFloat != TRUE || inp1->m_isFloat != TRUE) {
    printf("Only floating point data currently supported.\n");
    return;
  }
  
  
	if(!m_allFrames && m_displayAdapt) {
		adaptDisplay(inp0, 0.05f, 0);
		adaptDisplay(inp1, 0.05f, 1);
	}
	else {

    if(m_displayAdapt) {
      normalizeIntensity(inp0);
      normalizeIntensity(inp1);
		}
   
		clipLuminance(inp0);
		clipLuminance(inp1);

		puEncode(inp0);
		puEncode(inp1);

		biCubic(inp0->m_floatComp[0], inp0->m_width[0], inp0->m_height[0], &m_rszIn0[0], m_resizeWidth, m_resizeHeight);
		biCubic(inp1->m_floatComp[0], inp1->m_width[0], inp1->m_height[0], &m_rszIn1[0], m_resizeWidth, m_resizeHeight);

		calcLogGabor();

		if((m_poolFrameCnt % m_numberOfFramesFixate < m_numberOfFramesFixate - 1) && (m_poolFrameCnt < m_numberOfFrames - 1))	{
			m_poolFrameCnt++;
		}
		else {
			if(m_poolFrameCnt == m_numberOfFrames - 1) {
				if(m_numberOfFrames%m_numberOfFramesFixate == 0) {
					spatioTemporalPooling();
				}
				longTermPooling();
#if(VQM_DUMP_STATS==1)
				dumpFileFloat("VQM.txt", &m_vqmScore, 1,1);
#endif
			}
			else {
        spatioTemporalPooling();
			}
			m_poolFrameCnt++;
			m_stTubeCnt++;
		}
	}
}

// Compute metric for only one component
void DistortionMetricVQM::computeMetric (Frame* inp0, Frame* inp1, int component)
{
	return;
}

// report frame level results
void DistortionMetricVQM::reportMetric  ()
{
	printf(" %10.6f ", m_metric[0]); 
	return;
}                                        

// report summary results
void DistortionMetricVQM::reportSummary ()                                        
{
	printf("%f ", m_vqmScore);
	FILE * pFile = fopen("VQM_Scores.txt", "at");
	fprintf(pFile, "%f\n", m_vqmScore);
	fclose(pFile);
}

void DistortionMetricVQM::reportMinimum ()
{
	printf("%f ", m_vqmScore);
}

void DistortionMetricVQM::reportMaximum ()
{
	printf("%f ", m_vqmScore);
}

void DistortionMetricVQM::printHeader   ()
{
	printf("  HDR-VQM   "); 
}

void DistortionMetricVQM::printSeparator()
{
	printf("------------");
}




