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
 * \file FrameFilterWiener2DDark.cpp
 *
 * \brief
 *    FrameFilterWiener2DDark Class (2D Wiener Filtering, as currently done in Matlab)
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
#include "FrameFilterWiener2DDark.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameFilterWiener2DDark::FrameFilterWiener2DDark(int width, int height, int wSizeX, int wSizeY) {  
  m_wSizeX = wSizeX;
  m_wSizeY = wSizeY;
  m_wArea  = (double) ((2 * wSizeX + 1) * (2 * wSizeY + 1));

  m_width  = width;
  m_height = height;
  m_dImgSum.resize     ( m_width * m_height );
  m_dImgSumDev.resize  ( m_width * m_height );
  m_dImgDistance.resize( m_width * m_height );
  m_dImgOutput.resize  ( m_width * m_height );
}

FrameFilterWiener2DDark::~FrameFilterWiener2DDark() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
template<typename ValueType> double FrameFilterWiener2DDark::windowSum(const ValueType *iData, int iPosX, int iPosY, int iWidth, int iHeight) {
  double dValue = 0.0;
  const ValueType *currentLine = NULL;
  
  for (int j = -m_wSizeY; j <= m_wSizeY; j++) {
    currentLine = &iData[iClip(iPosY + j, 0, iHeight) * iWidth];
  
    for (int i = -m_wSizeX; i <= m_wSizeX; i++) {
      dValue += (double) currentLine[iClip(iPosX + i, 0, iWidth)];
      //printf("sum %10.8f %10.8f %d %d\n", dValue, currentLine[iClip(iPosX + i, 0, iWidth)], iClip(iPosY + j, 0, iHeight), iClip(iPosX + i, 0, iWidth));
    }
  }
  return dValue;
}


/*! \brief Compute NxN neighborhood DC Value.
 *
 *  Functions computes the Sum value within an NxN window around the
 *  the current sample.
 */
template<typename ValueType> void FrameFilterWiener2DDark::computeImgWindowSum(const ValueType *iData, int iWidth, int iHeight) {   
  double  *currentSum = &m_dImgSum[0];
  
  for (int j = ZERO;j < iHeight; j++) {    
    for (int i = ZERO; i < iWidth; i++) {
      *currentSum++ = windowSum(iData, i, j, iWidth, iHeight);
    }
  }
}


//-----------------------------------------------------------------------------
// Compute the sum of all values in a 3x3 arrangement
//-----------------------------------------------------------------------------
double FrameFilterWiener2DDark::computeShortDC(int iWidth, int iHeight) {
  double value;
  double noise = 0.0;
  double *data = &m_dImgDistance[0];
  double *currentSum = &m_dImgSumDev[0];
  
  for (int j = ZERO;j < iHeight; j++) {    
    for (int i = ZERO; i < iWidth; i++) {
      value = windowSum(data, i, j, iWidth, iHeight);
      *currentSum++ = value;
      noise += value;
    }
  }

  return (noise / (double) (iWidth * iHeight));
}

//-----------------------------------------------------------------------------
// Absolute distance from 3x3 mean
//-----------------------------------------------------------------------------
template<typename ValueType> void FrameFilterWiener2DDark::computeMeanDist(const ValueType *iData, int iWidth, int iHeight) {
  
  double *distImg = &m_dImgDistance[0];
  double *sumImg = &m_dImgSum[0];
  
  // Compute abs distance from window sum for each position
  for (int j = ZERO; j < iHeight; j++) {    
    for (int i = ZERO; i < iWidth; i++) {
      *distImg++ = dAbs(m_wArea * (double) (*iData++) - (*sumImg++));
    }
  }
}

//-----------------------------------------------------------------------------
// Wiener filtering given total noise, local mean and local absolute deviation
//-----------------------------------------------------------------------------
template<typename ValueType> void FrameFilterWiener2DDark::wienerFilter(const ValueType *iData, double noise,  int iWidth, int iHeight) {
  double normValue;
  
  double *pSumImg = &m_dImgSum[0];
  double *pMdImg  = &m_dImgSumDev[0];
  double *pOutImg = &m_dImgOutput[0];
  
  
  // Compute abs distance from 3x3 mean for each position
  for (int j = ZERO; j < iHeight; j++) {    
    for (int i = ZERO; i < iWidth; i++) {
      if (*iData < 1.0) {
        if (*pMdImg > noise) {
          normValue = (m_wArea * (double) *iData++) - *pSumImg;
          *pOutImg++ = (normValue * (1.0 - noise / (*pMdImg++)) + (*pSumImg++)) / m_wArea;
        }
        else {
          // Set output to mean
          *pOutImg++ = *pSumImg++ / m_wArea;
          iData++;
          pMdImg++;        
        }
      }
      else {
        // Set output to input
        *pOutImg++ = *iData++ ;
        pMdImg++;  
        pSumImg++;              
      }
    }
  }
}

void FrameFilterWiener2DDark::updateImg(float *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (float) (*inp++);
  }  
}

void FrameFilterWiener2DDark::updateImg(uint16 *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (uint16) dRound(*inp++);
  }  
}

void FrameFilterWiener2DDark::updateImg(imgpel *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (imgpel) dRound(*inp++);
  }  
}

template<typename ValueType>  void FrameFilterWiener2DDark::filter(ValueType *imgData, int width, int height, float minValue, float maxValue)
{
  // Update array size
  if ( width * height > m_width * m_height ) {
    m_width  = width;
    m_height = height;
    
    m_dImgSum.resize      ( m_width * m_height );
    m_dImgSumDev.resize   ( m_width * m_height );
    m_dImgDistance.resize ( m_width * m_height );
    m_dImgOutput.resize   ( m_width * m_height );
  }

  computeImgWindowSum(imgData, width, height);
  computeMeanDist    (imgData, width, height);
  
  double noise = computeShortDC(width, height);
  wienerFilter(imgData, noise, width, height);

  updateImg(imgData, &m_dImgOutput[0], width * height);
}

template<typename ValueType>  void FrameFilterWiener2DDark::filter(ValueType *out, const ValueType *inp, int width, int height, int minValue, int maxValue)
{
  // Update array size
  if ( width * height > m_width * m_height ) {
    m_width  = width;
    m_height = height;
    
    m_dImgSum.resize      ( m_width * m_height );
    m_dImgSumDev.resize   ( m_width * m_height );
    m_dImgDistance.resize ( m_width * m_height );
    m_dImgOutput.resize   ( m_width * m_height );
  }
  
  computeImgWindowSum(inp, width, height);
  computeMeanDist    (inp, width, height);
  
  double noise = computeShortDC(width, height);
  wienerFilter(inp, noise, width, height);

  updateImg(out, &m_dImgOutput[0], width * height);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void FrameFilterWiener2DDark::process ( Frame* out, const Frame *inp, bool compY, bool compCb, bool compCr) {

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

void FrameFilterWiener2DDark::process ( Frame* pFrame, bool compY, bool compCb, bool compCr) {  
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
