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
 * \file FrameFilter2DSep.cpp
 *
 * \brief
 *    FrameFilter2DSep Class (2D Wiener Filtering, as currently done in Matlab)
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
#include "FrameFilter2DSep.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameFilter2DSep::FrameFilter2DSep(int width, int height, int filter, bool mode) {  
  int offset, scale;

  m_mode   = mode;
  m_thres0 = 2.0;
  m_thres1 = 5.0;
  m_range  = m_thres1 - m_thres0;
  m_width  = width;
  m_height = height;
  m_dImgOutput.resize  ( m_width * m_height );
  m_floatData.resize   ( m_width * m_height );
  
  m_horFilter = new Filter1D(filter, 0,      0,     0, &offset, &scale);
  m_verFilter = new Filter1D(filter, 2, offset, scale, &offset, &scale);

}

FrameFilter2DSep::~FrameFilter2DSep() {
  delete m_horFilter;
  delete m_verFilter;
  m_horFilter = NULL;
  m_verFilter = NULL;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

void FrameFilter2DSep::updateImg(float *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (float) (*inp++);
  }  
}

void FrameFilter2DSep::updateImg(uint16 *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (uint16) dRound(*inp++);
  }  
}

void FrameFilter2DSep::updateImg(imgpel *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = (imgpel) dRound(*inp++);
  }  
}

template<typename ValueType>  double FrameFilter2DSep::edgeAdaptation(ValueType inpData, double fValue)
{
  double delta = dAbs(fValue - (double) inpData);
  if  ( delta > m_thres0 ) {
    delta = dClip(delta - m_thres0, 0.0, m_range);
    return ((double) inpData  * delta + fValue * (m_range - delta)) / m_range;
  }
  else 
    return fValue;
}

template<typename ValueType>  void FrameFilter2DSep::filter(ValueType *inpData, int width, int height, float minValue, float maxValue)
{
  int i, j;
  double fValue = 0.0;
  // Update array size
  if ( width * height > m_width * m_height ) {
    m_width  = width;
    m_height = height;
    
    m_dImgOutput.resize ( m_width * m_height );
    m_floatData.resize  ( m_width * m_height );
  }

  for (j = 0; j < m_height; j++) {
    for (i = 0; i < m_width; i++) {
      fValue = m_horFilter->filterH(&inpData[ j * m_width], i, m_width - 1, 0.0, 0.0);
      if (m_mode)
        m_floatData[ j * m_width + i ] = edgeAdaptation(inpData[ j * m_width + i ], fValue);
      else 
        m_floatData[ j * m_width + i ] = fValue;
    }
  }

  for (j = 0; j < m_height; j++) {
    for (i = 0; i < m_width; i++) {
      fValue = m_verFilter->filterV(&m_floatData[i], j, m_width, m_height - 1, minValue, maxValue);
      if (m_mode)
        m_dImgOutput[ j * m_width + i ] = edgeAdaptation(m_floatData[ j * m_width + i ], fValue);
      else 
        m_dImgOutput[ j * m_width + i ] = fValue;
    }
  }

  updateImg(inpData, &m_dImgOutput[0], width * height);
}

template<typename ValueType>  void FrameFilter2DSep::filter(ValueType *outData, ValueType *inpData, int width, int height, float minValue, float maxValue)
{
  int i, j;
  double fValue = 0.0;

  // Update array size
  if ( width * height > m_width * m_height ) {
    m_width  = width;
    m_height = height;
    
    m_dImgOutput.resize ( m_width * m_height );
    m_floatData.resize  ( m_width * m_height );
  }
  
  for (j = 0; j < m_height; j++) {
    for (i = 0; i < m_width; i++) {
      fValue = m_horFilter->filterH(&inpData[ j * m_width], i, m_width - 1, 0.0, 0.0);
      if (m_mode)
        m_floatData[ j * m_width + i ] = edgeAdaptation(inpData[ j * m_width + i ], fValue);
      else 
        m_floatData[ j * m_width + i ] = fValue;
    }
  }
  
  for (j = 0; j < m_height; j++) {
    for (i = 0; i < m_width; i++) {
      fValue = m_verFilter->filterV(&m_floatData[i], j, m_width, m_height - 1, minValue, maxValue);
      if (m_mode)
        m_dImgOutput[ j * m_width + i ] = edgeAdaptation(m_floatData[ j * m_width + i ], fValue);
      else 
        m_dImgOutput[ j * m_width + i ] = fValue;
    }
  }

  updateImg(outData, &m_dImgOutput[0], m_width * m_height);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void FrameFilter2DSep::process ( Frame* out, const Frame *inp, bool compY, bool compCb, bool compCr) {

  if (out->m_isFloat == TRUE) {    // floating point data
    if (compY == TRUE) {
      filter(out->m_floatComp[Y_COMP], inp->m_floatComp[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], (float) out->m_minPelValue[Y_COMP], (float) out->m_maxPelValue[Y_COMP] );
    }
    if (compCb == TRUE) {
      filter(out->m_floatComp[U_COMP], inp->m_floatComp[U_COMP], out->m_width[U_COMP], out->m_height[U_COMP], (float) out->m_minPelValue[U_COMP], (float) out->m_maxPelValue[U_COMP] );
    }
    if (compCr == TRUE) {
      filter(out->m_floatComp[V_COMP], inp->m_floatComp[V_COMP], out->m_width[V_COMP], out->m_height[V_COMP], (float) out->m_minPelValue[V_COMP], (float) out->m_maxPelValue[V_COMP] );
    }
  }
  else if (out->m_bitDepth == 8) {   // 8 bit data
    if (compY == TRUE) {
      filter(out->m_comp[Y_COMP], inp->m_comp[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], (float) out->m_minPelValue[Y_COMP], (float) out->m_maxPelValue[Y_COMP] );
    }
    if (compCb == TRUE) {
      filter(out->m_comp[U_COMP], inp->m_comp[U_COMP], out->m_width[U_COMP], out->m_height[U_COMP], (float) out->m_minPelValue[U_COMP], (float) out->m_maxPelValue[U_COMP] );
    }
    if (compCr == TRUE) {
      filter(out->m_comp[V_COMP], inp->m_comp[V_COMP], out->m_width[V_COMP], out->m_height[V_COMP], (float) out->m_minPelValue[V_COMP], (float) out->m_maxPelValue[V_COMP] );
    }
  }
  else { // 16 bit data
    if (compY == TRUE) {
      filter(out->m_ui16Comp[Y_COMP], inp->m_ui16Comp[Y_COMP], out->m_width[Y_COMP], out->m_height[Y_COMP], (float) out->m_minPelValue[Y_COMP], (float) out->m_maxPelValue[Y_COMP] );
    }
    if (compCb == TRUE) {
      filter(out->m_ui16Comp[U_COMP], inp->m_ui16Comp[U_COMP], out->m_width[U_COMP], out->m_height[U_COMP], (float) out->m_minPelValue[U_COMP], (float) out->m_maxPelValue[U_COMP] );
    }
    if (compCr == TRUE) {
      filter(out->m_ui16Comp[V_COMP], inp->m_ui16Comp[V_COMP], out->m_width[V_COMP], out->m_height[V_COMP], (float) out->m_minPelValue[V_COMP], (float) out->m_maxPelValue[V_COMP] );
    }
  }
}

void FrameFilter2DSep::process ( Frame* pFrame, bool compY, bool compCb, bool compCr) {

  if (pFrame->m_isFloat == TRUE) {    // floating point data
    if (compY == TRUE)
      filter(pFrame->m_floatComp[Y_COMP], pFrame->m_width[Y_COMP], pFrame->m_height[Y_COMP], (float) pFrame->m_minPelValue[Y_COMP], (float) pFrame->m_maxPelValue[Y_COMP] );
    if (compCb == TRUE)
      filter(pFrame->m_floatComp[U_COMP], pFrame->m_width[U_COMP], pFrame->m_height[U_COMP], (float) pFrame->m_minPelValue[U_COMP], (float) pFrame->m_maxPelValue[U_COMP] );
    if (compCr == TRUE)
      filter(pFrame->m_floatComp[V_COMP], pFrame->m_width[V_COMP], pFrame->m_height[V_COMP], (float) pFrame->m_minPelValue[V_COMP], (float) pFrame->m_maxPelValue[V_COMP] );
  }
  else if (pFrame->m_bitDepth == 8) {   // 8 bit data
    if (compY == TRUE)
      filter(pFrame->m_comp[Y_COMP], pFrame->m_width[Y_COMP], pFrame->m_height[Y_COMP], (float) pFrame->m_minPelValue[Y_COMP], (float) pFrame->m_maxPelValue[Y_COMP] );
    if (compCb == TRUE)
      filter(pFrame->m_comp[U_COMP], pFrame->m_width[U_COMP], pFrame->m_height[U_COMP], (float) pFrame->m_minPelValue[U_COMP], (float) pFrame->m_maxPelValue[U_COMP] );
    if (compCr == TRUE)
      filter(pFrame->m_comp[V_COMP], pFrame->m_width[V_COMP], pFrame->m_height[V_COMP], (float) pFrame->m_minPelValue[V_COMP], (float) pFrame->m_maxPelValue[V_COMP] );
  }
  else { // 16 bit data
    if (compY == TRUE)
      filter(pFrame->m_ui16Comp[Y_COMP], pFrame->m_width[Y_COMP], pFrame->m_height[Y_COMP], (float) pFrame->m_minPelValue[Y_COMP], (float) pFrame->m_maxPelValue[Y_COMP] );
    if (compCb == TRUE)
      filter(pFrame->m_ui16Comp[U_COMP], pFrame->m_width[U_COMP], pFrame->m_height[U_COMP], (float) pFrame->m_minPelValue[U_COMP], (float) pFrame->m_maxPelValue[U_COMP] );
    if (compCr == TRUE)
      filter(pFrame->m_ui16Comp[V_COMP], pFrame->m_width[V_COMP], pFrame->m_height[V_COMP], (float) pFrame->m_minPelValue[V_COMP], (float) pFrame->m_maxPelValue[V_COMP] );
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
