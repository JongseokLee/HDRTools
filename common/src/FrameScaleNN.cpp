/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2017
 *
 * Copyright (c) 2017, Apple Inc.
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
 * \file FrameScaleNN.cpp
 *
 * \brief
 *    Rescale using the nearest neighbor method
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
#include "FrameScaleNN.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameScaleNN::FrameScaleNN(int iWidth, int iHeight, int oWidth, int oHeight) {
  /* if resampling is actually downsampling we have to EXTEND the length of the
  original filter; the ratio is calculated below */
  m_factorX = (double)(iWidth ) / (double) (oWidth );
  m_factorY = (double)(iHeight) / (double) (oHeight);

  // nearest neighbor uses 2 taps so we can fit the implementation within the generic
  // linear separable filter implementation that we have. The tap values are determined
  // to be equal to either 1 or 0 depending on distance.
  m_filterTapsX = 2;
  m_filterTapsY = 2;

  m_offsetX = m_factorX > 1.0 ? 1 / m_factorX : 0.0;
  m_offsetY = m_factorY > 1.0 ? 1 / m_factorY : 0.0;

  // Allocate filter memory
  m_filterOffsetsX.resize(m_filterTapsX);
  m_filterOffsetsY.resize(m_filterTapsY);
  
  // We basically allocate filter coefficients for all target positions.
  // This is done once and saves us time from deriving the proper filter for each position.
  m_filterCoeffsX.resize(oWidth  * m_filterTapsX);
  m_filterCoeffsY.resize(oHeight * m_filterTapsY);

  // Initialize the filter boundaries
  SetFilterLimits((int *) &m_filterOffsetsX[0], m_filterTapsX, iWidth,  oWidth,  m_factorX, &m_iMinX, &m_iMaxX, &m_oMinX, &m_oMaxX);
  SetFilterLimits((int *) &m_filterOffsetsY[0], m_filterTapsY, iHeight, oHeight, m_factorY, &m_iMinY, &m_iMaxY, &m_oMinY, &m_oMaxY);

  // Finally prepare the filter coefficients for horizontal and vertical filtering
  // Horizontal
  GetFilterCoefficients((double *) &m_filterCoeffsX[0], (int *) &m_filterOffsetsX[0], m_factorX, m_filterTapsX, m_offsetX, oWidth);

  // Vertical
  GetFilterCoefficients((double *) &m_filterCoeffsY[0], (int *) &m_filterOffsetsY[0], m_factorY, m_filterTapsY, m_offsetY, oHeight);

}

//FrameScaleNN::~FrameScaleNN() {
//}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

void FrameScaleNN::GetFilterCoefficients(double *factorCoeffs, int *filterOffsets, double factor, int filterTaps, double offset,
  int oSize)
{
  int x;
  int tapIndex, index;
  double coeffSum, dist;
  double off, posOrig;

  for ( x = 0, index = 0; x < oSize; x++, index += filterTaps ) {
    posOrig = offset + (double) x * factor;
    off     = posOrig - floor(posOrig);

    coeffSum = 0.0;
    for ( tapIndex = index; tapIndex < index + filterTaps; tapIndex++ ) {
      if ( factor <= 1.0 ) {
        dist = (filterOffsets[tapIndex - index] - off);
      }
      else {
        dist = (-filterOffsets[tapIndex - index] + off);
      }

      factorCoeffs[tapIndex] = FilterTap( dist );
      coeffSum += factorCoeffs[tapIndex];
      //printf("coefficient %d %10.7f %10.7f\n", tapIndex, dist, factorCoeffs[tapIndex]);
    }

    for ( tapIndex = index; tapIndex < index + filterTaps; tapIndex++ ) {
      factorCoeffs[ tapIndex ] /= coeffSum;
    }
  }
}

double FrameScaleNN::FilterTap( double dist ) {
  double absDist = dAbs(dist);
  return (absDist > 1.0) ? 1.0 : (((absDist < 0.5) || dist == 0.5 )? 1.0 : 0.0);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void FrameScaleNN::process ( Frame* out, const Frame *inp)
{
  if (( out->m_isFloat != inp->m_isFloat ) || (( inp->m_isFloat == 0 ) && ( out->m_bitDepth != inp->m_bitDepth ))) {
    fprintf(stderr, "Error: trying to copy frames of different data types. \n");
    exit(EXIT_FAILURE);
  }
    
  int c;
  
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  for (c = Y_COMP; c <= V_COMP; c++) {
    out->m_minPelValue[c]  = inp->m_minPelValue[c];
    out->m_midPelValue[c]  = inp->m_midPelValue[c];
    out->m_maxPelValue[c]  = inp->m_maxPelValue[c];
  }
  
  if (out->m_isFloat == TRUE) {    // floating point data
    for (c = Y_COMP; c <= V_COMP; c++) {
      filter( inp->m_floatComp[c], out->m_floatComp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], (double) out->m_minPelValue[c], (double) out->m_maxPelValue[c] );
    }
  }
  else if (out->m_bitDepth == 8) {   // 8 bit data
    for (c = Y_COMP; c <= V_COMP; c++) {
      filter( inp->m_comp[c], out->m_comp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c] );
    }
  }
  else { // 16 bit data
    for (c = Y_COMP; c <= V_COMP; c++) {
      filter( inp->m_ui16Comp[c], out->m_ui16Comp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c] );
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
