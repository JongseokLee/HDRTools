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
 * \file FrameFilterNLMeans.cpp
 *
 * \brief
 *    FrameFilterNLMeans Class (NL means filtering)
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

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "FrameFilterNLMeans.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameFilterNLMeans::FrameFilterNLMeans(int width, int height, int wSizeX, int wSizeY) {  

  m_width  = width;
  m_height = height;
  m_dImgOutput.resize  ( m_width * m_height );
  m_integralMem.resize ( m_width * m_height );
  m_weightSum.resize   ( m_width * m_height );
  m_pixelSum.resize    ( m_width * m_height );

  
  m_hParam = 3.0 * 2.0;     // averaging weight decay parameter (larger values give smoother videos).
  m_patchSize = 7;            // pixel context region width (default=7, little need to change), should be odd number.
  m_range = 7;                // spatial search range (default=3), should be odd number.

  const int n = (m_patchSize | 1);

  const double weightFactor = 1.0 / (n * n) / (m_hParam * m_hParam);

  const int tableSize=EXP_TABSIZE;
  //  const float min_weight_in_table = 0.0005;
  const double minWeightInTable = 0.0005/256.0;
  
  
  const double stretch = tableSize / (-log(minWeightInTable));
  m_weightFactTable = weightFactor * stretch;
  
  m_diffMax = tableSize / m_weightFactTable;
    
  for (int i = 0; i < tableSize; i++) {
    m_expTable[i] = exp(-i / stretch);
  }
  m_expTable[tableSize - 1] = 0;

}

FrameFilterNLMeans::~FrameFilterNLMeans() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

template<typename ValueType0, typename ValueType1> void FrameFilterNLMeans::buildIntegralImage(
                                           ValueType0 *integralImage, int integralStride,
                                     const ValueType1 *currentImage, int currentImageStride,
                                     const ValueType1 *compareImage, int compareImageStride,
                                     int   iWidth,int  iHeight,
                                     int   dx,int dy)
{
  //memset(integralImage - 1 -integralStride, 0, (iWidth + 1) * sizeof(ValueType0));
  
  for (int y = 0; y < iHeight; y++) {
    const ValueType1 *p1 = &currentImage[ y * currentImageStride];
    const ValueType1 *p2 = &compareImage[ iClip((y + dy), 0, iHeight) * compareImageStride + iClip(dx, 0, iWidth)];
    ValueType0 *out = &integralImage[y * integralStride];
    
    ValueType0 diff = (ValueType0) *p1++ - (ValueType0) *p2++;
    *out = diff * diff;
    out++;

    for (int x = 0; x < iWidth;x++)
    {
      diff = *p1++ - *p2++;
      *out = *(out-1) + diff * diff;
      out++;
    }
    
    if (y > 0) {
      out = &(integralImage[y * integralStride]);
      
      for (int x = 0; x < iWidth; x++) {
        *out += *(out - integralStride);
        out++;
      }
    }
  }
}



template<typename ValueType>  void FrameFilterNLMeans::computeNLMeans(ValueType *imgData, int width, int height)
{
  const int w = width;
  const int h = height;
  
  const int n = (m_patchSize | 1);
  const int r = (m_range     | 1);
  
  const int nHalf = (n-1)/2;
  const int rHalf = (r-1)/2;
  
  const int integralStride = w;    
  
  // reset m_pixelSum/m_weightSum
  //std::fill(m_tempData.begin(), m_tempData.end(), 0);  
  //memset(&m_tempData[0], 0, w * h * sizeof(m_tempData));
  std::fill(m_pixelSum.begin(), m_pixelSum.end(), 0);
  std::fill(m_weightSum.begin(), m_weightSum.end(), 0);

  
  // copy input image
  const ValueType *currentImage = imgData;
  int currentImageStride        = width;
  
  const ValueType *compareImage = imgData;
  int compareImageStride        = width;
    
  // --- iterate through all displacements ---
  for (int dy = -rHalf ; dy <= rHalf ; dy++) {
    for (int dx = -rHalf ; dx <= rHalf ; dx++) {
      
      // special, simple implementation for no shift (no difference -> weight 1)
      if (dx==0 && dy==0) {
        for (int y = 0; y < h - n; y++) {
          for (int x = 0; x < w - n; x++) {
            //m_tempData[y * w + x].m_weightSum += 1.0;
            //m_tempData[y * w + x].m_pixelSum  += currentImage[y * currentImageStride + x];
            m_weightSum[y * w + x] += 1.0;
            m_pixelSum[y * w + x]  += currentImage[y * currentImageStride + x];
          }
        }
        continue;
      }

      std::fill(m_integralMem.begin(), m_integralMem.end(), 0);
      
      // --- regular case ---
      buildIntegralImage(&m_integralMem[0], integralStride, currentImage, currentImageStride,
                         compareImage, compareImageStride,
                         w, h, dx,dy);
      
      for (int y = 0; y <= h - n; y++) {
        int yPlus  = iClip(y + nHalf, 0, h);
        int yMinus = iClip(y - nHalf, 0, h);
        for (int x = 0; x <= w - n; x++) {     
          INTEGRAL_TYPE integralValue1L = m_integralMem[yMinus * integralStride + iClip(x - nHalf, 0, w)];    
          INTEGRAL_TYPE integralValue2L = m_integralMem[yPlus  * integralStride + iClip(x - nHalf, 0, w)];    
          INTEGRAL_TYPE integralValue1R = m_integralMem[yMinus * integralStride + iClip(x + nHalf, 0, w)];    
          INTEGRAL_TYPE integralValue2R = m_integralMem[yPlus  * integralStride + iClip(x + nHalf, 0, w)];    
          
          // patch difference
          INTEGRAL_TYPE diff = (INTEGRAL_TYPE)(integralValue2R - integralValue2L - integralValue1R + integralValue1L);
          //printf("value (%10.7f %10.7f %10.7f %10.7f) %10.7f\n", integralValue1L, integralValue1R, integralValue2L, integralValue2R, diff);
          
          // sum pixel with weight
          if (diff<m_diffMax) {
            int diffidx = (int) (diff * m_weightFactTable);
            
            double weight = m_expTable[diffidx];
            
            //m_tempData[y * w + x].m_weightSum += weight;
            //m_tempData[y * w + x].m_pixelSum  += weight * compareImage[iClip((y + dy), 0, h) * compareImageStride + iClip(x + dx, 0, w)];
            m_weightSum[y * w + x] += weight;
            m_pixelSum [y * w + x] += weight * compareImage[iClip((y + dy), 0, h) * compareImageStride + iClip(x + dx, 0, w)];
          }
        }
      }
    }
  }
  
  // output main image    
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      //m_dImgOutput[y * w + x] = m_tempData[y * w + x].m_pixelSum / m_tempData[y * w + x].m_weightSum;
      m_dImgOutput[y * w + x] = m_pixelSum[y * w + x] / m_weightSum[y * w + x];

    }
  }
}


void FrameFilterNLMeans::updateImg(float *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (float) (*inp++);
  }  
}

void FrameFilterNLMeans::updateImg(uint16 *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (uint16) dRound(*inp++);
  }  
}

void FrameFilterNLMeans::updateImg(imgpel *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (imgpel) dRound(*inp++);
  }  
}

template<typename ValueType>  void FrameFilterNLMeans::filter(ValueType *imgData, int width, int height, float minValue, float maxValue)
{
  // Update array size
  if ( width * height > m_width * m_height ) {
    m_width  = width;
    m_height = height;
    
    m_dImgOutput.resize   ( m_width * m_height );
    m_weightSum.resize    ( m_width * m_height );
    m_pixelSum.resize     ( m_width * m_height );
    //m_tempData.resize     ( m_width * m_height );
    m_integralMem.resize  ( m_width * m_height );
  }
  
  computeNLMeans(imgData, width, height);

  updateImg(imgData, &m_dImgOutput[0], width * height);
}

template<typename ValueType>  void FrameFilterNLMeans::filter(ValueType *out, const ValueType *inp, int width, int height, int minValue, int maxValue)
{
  // Update array size
  if ( width * height > m_width * m_height ) {
    m_width  = width;
    m_height = height;
    
    m_dImgOutput.resize   ( m_width * m_height );
    m_weightSum.resize    ( m_width * m_height );
    m_pixelSum.resize     ( m_width * m_height );
    //m_tempData.resize     ( m_width * m_height );
    m_integralMem.resize  ( m_width * m_height );
  }  
  
  computeNLMeans(inp, width, height);  

  updateImg(out, &m_dImgOutput[0], width * height);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void FrameFilterNLMeans::process ( Frame* out, const Frame *inp, bool compY, bool compCb, bool compCr) {

  if (out->m_isFloat == TRUE) {    // floating point data
    if (compY == TRUE) {
      out->copy((Frame *) inp, Y_COMP);
      filter(out->m_floatComp[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], (float) out->m_minPelValue[Y_COMP], (float) out->m_maxPelValue[Y_COMP] );
    }
    if (compCb == TRUE) {
      out->copy((Frame *) inp, U_COMP);
      filter(out->m_floatComp[U_COMP], out->m_width[U_COMP], out->m_height[U_COMP], (float) out->m_minPelValue[U_COMP], (float) out->m_maxPelValue[U_COMP] );
    }
    if (compCr == TRUE) {
      out->copy((Frame *) inp, V_COMP);
      filter(out->m_floatComp[V_COMP], out->m_width[V_COMP], out->m_height[V_COMP], (float) out->m_minPelValue[V_COMP], (float) out->m_maxPelValue[V_COMP] );
    }
  }
  else if (out->m_bitDepth == 8) {   // 8 bit data
    if (compY == TRUE) {
      out->copy((Frame *) inp, Y_COMP);
    }
    if (compCb == TRUE) {
      out->copy((Frame *) inp, U_COMP);
    }
    if (compCr == TRUE) {
      out->copy((Frame *) inp, V_COMP);
    }
  }
  else { // 16 bit data
    if (compY == TRUE) {
      out->copy((Frame *) inp, Y_COMP);
    }
    if (compCb == TRUE) {
      out->copy((Frame *) inp, U_COMP);
    }
    if (compCr == TRUE) {
      out->copy((Frame *) inp, V_COMP);
    }
  }
}

void FrameFilterNLMeans::process ( Frame* pFrame, bool compY, bool compCb, bool compCr) {

  
  
  if (pFrame->m_isFloat == TRUE) {    // floating point data
    if (compY == TRUE)
      filter(pFrame->m_floatComp[Y_COMP], pFrame->m_width[Y_COMP], pFrame->m_height[Y_COMP], (float) pFrame->m_minPelValue[Y_COMP], (float) pFrame->m_maxPelValue[Y_COMP] );
    if (compCb == TRUE)
      filter(pFrame->m_floatComp[U_COMP], pFrame->m_width[U_COMP], pFrame->m_height[U_COMP], (float) pFrame->m_minPelValue[U_COMP], (float) pFrame->m_maxPelValue[U_COMP] );
    if (compCr == TRUE)
      filter(pFrame->m_floatComp[V_COMP], pFrame->m_width[V_COMP], pFrame->m_height[V_COMP], (float) pFrame->m_minPelValue[V_COMP], (float) pFrame->m_maxPelValue[V_COMP] );
  }
  else if (pFrame->m_bitDepth == 8) {   // 8 bit data
  }
  else { // 16 bit data
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
