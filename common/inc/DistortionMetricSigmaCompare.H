/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Chad Fogg
 * <ORGANIZATION> = Chad Fogg
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Chad Fogg
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
 * \file DistortionMetricSigmaCompare.H
 *
 * \brief
 *  DistortionMetricSigmaCompare class header files
 *  "A Quality Metric for High Dynamic Range" by Gary Demos   (garyd@alumni.caltech.edu)
 *   algorithm description in: JCTVC-R0214:
 *        http://phenix.int-evry.fr/jct/doc_end_user/current_document.php?id=9285
 *
 * \author
 *     - Chad Fogg (9/22/2014)
 *************************************************************************************
 */

#ifndef __HDRTools__DistortionMetricSigmaCompare__
#define __HDRTools__DistortionMetricSigmaCompare__

//#define COMPUTE_LOCAL_SIGMA         1

//#include <iostream>
#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"

static const int SIGMA_MULTIPLES=7;      /*2,3,4,6,8,12,16*/
static const int NUMBER_OF_F16_STOPS=32; // note: the 5 bit half-float exponent is limited to a range of 31 stops (value 31 = +-inf nan out-of-range code).  Note: value 0-exp denorm extends lower),  Use 32-bit floating point I/O (e.g. dpx32) for ranges greater than 31 (+denorm) stops
static const int NUMBER_OF_U16_STOPS=16;


typedef struct {
  // measures
  double squareSumError;
  double sumOrigVal;
  double fStopCount;
  double furthestOutlierSum;
  double countMultipleSigma[SIGMA_MULTIPLES];
  
  // derivations
  double average;
  double sigma;
  double selfRelative;
  double MSE;
  double furthestOutlierAvg;
} f32_sigma_t;


typedef struct {
  f32_sigma_t neg;
  
  f32_sigma_t pos[NUMBER_OF_F16_STOPS];
#ifdef COMPUTE_LOCAL_SIGMA
  f32_sigma_t lpos[NUMBER_OF_F16_STOPS][NUMBER_OF_F16_STOPS];
#endif
  
} f32_stats_t;


typedef struct {
  // measures
  int64 squareSumError;
  int64 sumOrigVal;
  int64 fStopCount;
  int64 furthestOutlierSum;
  int64 countMultipleSigma[SIGMA_MULTIPLES];
  
  // derivations
  double average;
  double sigma;
  double selfRelative;
  double MSE;
  double furthestOutlierAvg;
} u16_sigma_t;


typedef struct {
  u16_sigma_t neg;
  u16_sigma_t pos[NUMBER_OF_U16_STOPS];
#ifdef COMPUTE_LOCAL_SIGMA
  u16_sigma_t lpos[NUMBER_OF_U16_STOPS][NUMBER_OF_U16_STOPS];
#endif
} u16_stats_t;


typedef struct {
  float *buf[3];
  int width;
  int height;
  int init;
} f32_pic_t;


typedef struct {
  unsigned short *buf;
  int width;
  int height;
  int init;
  
  int minLevel;
  int maxLevel;
  
  int bitDepth;
  int videoFullRangeFlag;
  int transferCharacteristics;
} u16_plane_t;

typedef struct {
  int chroma_format_idc;
  u16_plane_t plane[3];
  int init;
} u16_pic_t;


class DistortionMetricSigmaCompare : public DistortionMetric {
private:
  f32_stats_t       m_f32SigmaStats[T_COMP];
  u16_stats_t       m_u16SigmaStats[T_COMP];
  bool              m_isFloat;
  int               m_noPass;
    
  double            m_amplitudeFactor;
    
  ColorSpace        m_colorSpace;
  double            m_maxValue[T_COMP];

  int  kMultiple( int k );
  void initSigmaStats( f32_stats_t *stat );
  void initSigmaStats( u16_stats_t *stat );
  
  float PQ10000_f( float V);
  float PQ10000_r( float L);
  float bt1886_f ( float V, float gamma, float Lw, float Lb);
  float bt1886_r ( float L, float gamma, float Lw, float Lb);
  
#if 0
  
  void convert_u16_plane_to_f32( float *dst_plane, u16_plane_t *p,  float amplitudeFactor );
  void convertShortFrameToFloat( f32_pic_t *dst_pic, u16_pic_t *src_pic, float amplitudeFactor );
  
#endif
  
  void calcSigma( f32_sigma_t * s );
  void calcSigma( u16_sigma_t * s );
  int binValue( float sample );
  int binValue( unsigned short sample );
  int binValue( imgpel sample );
  
#ifdef COMPUTE_LOCAL_SIGMA
  float calcLocalSigma( int width, int height, int x, int y, float *plane );
#endif
  
  void measureFrameSigmaPass1( f32_stats_t *sig,  float  *src_pic, float  *test_pic, int width, int height );
  void measureFrameSigmaPass1( u16_stats_t *sig,  uint16 *src_pic, uint16 *test_pic, int width, int height );
  void measureFrameSigmaPass1( u16_stats_t *sig,  imgpel *src_pic, imgpel *test_pic, int width, int height );
  void measureFrameSigmaPass2( f32_stats_t *sig,  float  *src_pic, float  *test_pic, int width, int height );
  void measureFrameSigmaPass2( u16_stats_t *sig,  uint16 *src_pic, uint16 *test_pic, int width, int height );
  void measureFrameSigmaPass2( u16_stats_t *sig,  imgpel *src_pic, imgpel *test_pic, int width, int height );

  void calcPass1Stats();
  void printPass1Stats();
  void printPass2Stats();

public:
  // Construct/Deconstruct
  DistortionMetricSigmaCompare(const FrameFormat *format, double maxSampleValue, double amplitudeFactor);
  virtual ~DistortionMetricSigmaCompare();
  


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
