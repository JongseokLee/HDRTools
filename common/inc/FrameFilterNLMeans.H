/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/MPEG
 * <ORGANIZATION> = ITU/MPEG
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, ITU/MPEG
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
 * \file FrameFilterNLMeans.H
 *
 * \brief
 *    FrameFilterNLMeans Class Header (NL means filtering)
 *    Code is a C++ port of the code provided by Chad on the xyz-ee1 reflector (12/6/2015)
 *    The algorithm is based on work from A. Buades, B. Coll, J.M. Morel and the paper:
 *    "A non-local algorithm for image denoising" 
 *    in IEEE Computer Vision and Pattern Recognition 2005, Vol 2, pp: 60-65, 2005
 *    
 *    Code is based on code written by Dr. Dirk Farin (dirk.farin@gmail.com) 
 *    and the original can be found here: 
 *          http://www.dirk-farin.net/projects/nlmeans/
 *
 * \author
 *     - Chad Fogg                       <chad.fogg@gmail.com>
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __FrameFilterNLMeans_H__
#define __FrameFilterNLMeans_H__

#include "Global.H"
#include "Frame.H"
#include "FrameFilter.H"
#include <vector>

#ifndef INTEGRAL_TYPE
//#define INTEGRAL_TYPE   uint64_t
#define INTEGRAL_TYPE   double
#endif

#define EXP_TABSIZE 128*256


class PixelSum {
  public:
  double m_weightSum;
  double m_pixelSum;
  PixelSum() {
    m_weightSum = 0.0;
    m_pixelSum = 0.0;
  }
};

//template <class ImgData>
class FrameFilterNLMeans : public FrameFilter {
private:
  vector<double> m_dImgOutput;
  vector<double> m_weightSum;
  vector<double> m_pixelSum;
  //vector<PixelSum> m_tempData;
  vector<INTEGRAL_TYPE> m_integralMem;
  
  int     m_width;
  int     m_height;
  // NLMeans Parameters
  double m_hParam;
  int    m_patchSize;
  int    m_range;      // search range (must be odd number)
  
  
  INTEGRAL_TYPE m_diffMax;
  double        m_weightFactTable;
  double        m_expTable[EXP_TABSIZE];


  double computeShortDC (int iWidth, int iHeight);
  void   updateImg      (float *out,  const double *inp, int size);
  void   updateImg      (uint16 *out, const double *inp, int size);
  void   updateImg      (imgpel *out, const double *inp, int size);
  
  template<typename ValueType>  void  filter   (ValueType  *imgData, int width, int height, float minValue, float maxValue);
  template<typename ValueType>  void  filter   (ValueType *out, const ValueType *inp, int width, int height, int   minValue, int   maxValue);

  template<typename ValueType0, typename ValueType1> void buildIntegralImage(ValueType0 *integralImage, int integralStride, const ValueType1 *currentImage, int currentImageStride, const ValueType1 *compareImage, int compareImageStride, int iWidth,int iHeight, int   dx,int dy);
  template<typename ValueType>  void computeNLMeans(ValueType *imgData, int width, int height);
  
public:
  // Construct/Deconstruct
  FrameFilterNLMeans(int width, int height, int wSizeX = 1, int wSizeY = 1);
  virtual ~FrameFilterNLMeans();
   
  virtual void process ( Frame *out,  const Frame *inp, bool compY, bool compCb, bool compCr );
  virtual void process ( Frame* pFrame, bool compY, bool compCb, bool compCr );
};

#endif
