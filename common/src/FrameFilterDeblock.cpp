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
 * \file FrameFilterDeblock.cpp
 *
 * \brief
 *    FrameFilterDeblock Class
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
#include "FrameFilterDeblock.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameFilterDeblock::FrameFilterDeblock(int width, int height, int blockSizeX, int blockSizeY) {  
  m_blockSizeX = blockSizeX;
  m_blockSizeY = blockSizeY;
}

FrameFilterDeblock::~FrameFilterDeblock() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

void FrameFilterDeblock::filter(float *imgData, int width, int height, float minValue, float maxValue)
{
  int i, j;
  float *pImg;
  double q0, q1, q2, p0, p1, p2;
  double ap, aq;
  double tc, delta;
  double const tc0 = 0.2;
  double const b = 0.002;
  
  for (j = m_blockSizeY; j < height; j += m_blockSizeY) {
    for (i = 0; i < width; i ++) {
      pImg = &imgData[ j * width + i];
      q0 = (double) *pImg;
      q1 = (double) *(pImg + width);
      q2 = (double) *(pImg + 2 * width);
      p0 = (double) *(pImg - width);
      p1 = (double) *(pImg - 2 * width);
      p2 = (double) *(pImg - 3 * width);
      ap = dAbs( p2 - p0 );
      aq = dAbs( q2 - q0 );
      tc = (tc0 + (ap < b) + (aq < b)) / 1000.0;
      //tc = (tc0 + (ap < b && ap != 0.0) + (aq < b && aq != 0.0)) / 1000.0;
      delta= dMax(-tc, dMin(tc,((4.0 * (q0 - p0) + (p1 - q1)) / 8.0)));
      
      *pImg           = (float) (q0 - delta);
      *(pImg - width) = (float) (p0 + delta);
      
      if (ap < b) {
        delta= dMax(-tc, dMin(tc,(p2 + ((p0 + q0) / 2.0) - 2.0 * p1) / 2.0));
        *(pImg - 2 * width) = (float) (p1 + delta);
      }
      if (aq < b) {
        delta= dMax(-tc, dMin(tc,(q2 + ((q0 + p0) / 2.0) - 2.0 * q1) / 2.0));
        *(pImg + width) = (float) (q1 + delta);
      }
    }
  }
  
  for (i = m_blockSizeX; i < width; i += m_blockSizeX) {
    for (j = 0; j < height; j ++) {
      pImg = &imgData[ j * width + i];
      q0 = (double) *pImg;
      q1 = (double) *(pImg + 1);
      q2 = (double) *(pImg + 2);
      p0 = (double) *(pImg - 1);
      p1 = (double) *(pImg - 2);
      p2 = (double) *(pImg - 3);
      ap = dAbs( p2 - p0 );
      aq = dAbs( q2 - q0 );
      tc = (tc0 + (ap < b) + (aq < b)) / 1000.0;
      //tc = (tc0 + (ap < b && ap != 0.0) + (aq < b && aq != 0.0)) / 1000.0;
      delta= dMax(-tc, dMin(tc,((4.0 * (q0 - p0) + (p1 - q1)) / 8.0)));
      
      *pImg       = (float) (q0 - delta);
      *(pImg - 1) = (float) (p0 + delta);
      
      if (ap < b) {
        delta= dMax(-tc, dMin(tc,(p2 + ((p0 + q0) / 2.0) - 2.0 * p1) / 2.0));
        *(pImg - 2) = (float) (p1 + delta);
      }
      if (aq < b) {
        delta= dMax(-tc, dMin(tc,(q2 + ((q0 + p0) / 2.0) - 2.0 * q1) / 2.0));
        *(pImg + 1) = (float) (q1 + delta);
      }
    }
  }
}


void FrameFilterDeblock::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int blockWidth  = width  / m_blockSizeX;
  int blockHeight = height / m_blockSizeY;
  
  for (j = 0; j < blockHeight; j++) {
    for (i = 0; i < blockWidth; i++) {
    

      //m_i32Data[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, minValue, maxValue);
    }
  }
  
  for (j = 0; j < blockHeight; j++) {
    for (i = 0; i < blockWidth; i++) {
      //out[ j * width + i ] = filterVertical(&m_i32Data[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void FrameFilterDeblock::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int blockWidth  = width  / m_blockSizeX;
  int blockHeight = height / m_blockSizeY;
  
  for (j = 0; j < blockHeight; j++) {
    for (i = 0; i < blockWidth; i++) {
      //m_i32Data[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, 0, 0);
    }
  }
  
  for (j = 0; j < blockHeight; j++) {
    for (i = 0; i < blockWidth; i++) {
      //out[ j * width + i ] = filterVertical(&m_i32Data[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void FrameFilterDeblock::process ( Frame* out, const Frame *inp, bool compY, bool compCb, bool compCr) {

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

void FrameFilterDeblock::process ( Frame* pFrame, bool compY, bool compCb, bool compCr) {
  
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
