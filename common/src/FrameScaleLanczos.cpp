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
 * \file FrameScaleLanczos.cpp
 *
 * \brief
 *    Scale using Lanczos interpolation 
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
#include "FrameScaleLanczos.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

FrameScaleLanczos::FrameScaleLanczos(int iWidth, int iHeight, int oWidth, int oHeight, int lanczosLobes, ChromaLocation chromaLocationType) {

  /*
  int hPhase, vPhase;
  // here we allocate the entire image buffers. To save on memory we could just allocate
  // these based on filter length, but this is test code so we don't care for now.
  //m_i32Data.resize  ((width >> 1) * height);
  //m_floatData.resize((width >> 1) * height);
  
  // Currently we only support progressive formats, and thus ignore the bottom chroma location type
  switch (chromaLocationType) {
    case CL_FIVE:   // This is not yet correct. I need to add an additional phase to support this. TBD
      hPhase = 1;
      vPhase = 0;
      break;
    case CL_FOUR:  // This is not yet correct. I need to add an additional phase to support this. TBD
      hPhase = 0;  
      vPhase = 0;
      break;
    case CL_THREE:
      hPhase = 1;
      vPhase = 0;
      break;
    case CL_TWO:
      hPhase = 0;
      vPhase = 0;
      break;
    case CL_ONE:
      hPhase = 1;
      vPhase = 1;
      break;
    case CL_ZERO:
    default:
      hPhase = 0;
      vPhase = 1;
      break;
  }
  */

  m_lanczosLobes = lanczosLobes;
  m_lobesX = lanczosLobes;
  m_lobesY = lanczosLobes;
  

  m_pi = (double) M_PI  / (double) m_lanczosLobes;
  m_piSq = (double) M_PI * m_pi;

  /* if resampling is actually downsampling we have to EXTEND the length of the
  original filter; the ratio is calculated below */
  m_factorY = (double)(iHeight) / (double) (oHeight);
  m_factorX = (double)(iWidth ) / (double) (oWidth );

  m_offsetX = m_factorX > 1.0 ? 1 / m_factorX : 0.0;
  m_offsetY = m_factorY > 1.0 ? 1 / m_factorY : 0.0;

  
  //m_filterTapsX = (m_factorX == 1.0 ) ? 1 : (m_factorX > 1.0 ) ? (int) ceil (m_factorX * 2 * (m_lanczosLobes + 1)) :  2 * m_lanczosLobes;
  //m_filterTapsY = (m_factorY == 1.0 ) ? 1 : (m_factorY > 1.0 ) ? (int) ceil (m_factorY * 2 * (m_lanczosLobes + 1)) :  2 * m_lanczosLobes;
  //m_filterTapsX = (m_factorX == 1.0 ) ? 1 : (m_factorX > 1.0 ) ? 2 * m_lanczosLobes :  2 * m_lanczosLobes;
  //m_filterTapsY = (m_factorY == 1.0 ) ? 1 : (m_factorY > 1.0 ) ? 2 * m_lanczosLobes :  2 * m_lanczosLobes;
  m_filterTapsX = (m_factorX == 1.0 ) ? 1 : (m_factorX > 1.0 ) ? (int) ceil (m_factorX * 2 * (m_lobesX)) :  m_lobesX;
  m_filterTapsY = (m_factorY == 1.0 ) ? 1 : (m_factorY > 1.0 ) ? (int) ceil (m_factorY * 2 * (m_lobesY)) :  m_lobesY;


  // Allocate filter memory
  m_filterOffsetsX.resize(m_filterTapsX);
  m_filterOffsetsY.resize(m_filterTapsY);
  
  // We basically allocate filter coefficients for all target positions.
  // This is done once and saves us time from deriving the proper filter for each position.
  m_filterCoeffsX.resize(oWidth * m_filterTapsX);
  m_filterCoeffsY.resize(oHeight * m_filterTapsY);

  // Initialize the filter boundaries
  SetFilterLimits((int *) &m_filterOffsetsX[0], m_filterTapsX, iWidth,  oWidth,  m_factorX, &m_iMinX, &m_iMaxX, &m_oMinX, &m_oMaxX);
  SetFilterLimits((int *) &m_filterOffsetsY[0], m_filterTapsY, iHeight, oHeight, m_factorY, &m_iMinY, &m_iMaxY, &m_oMinY, &m_oMaxY);

  // Finally prepare the filter coefficients for horizontal and vertical filtering
  // Horizontal
  PrepareFilterCoefficients((double *) &m_filterCoeffsX[0], (int *) &m_filterOffsetsX[0], m_factorX, m_filterTapsX, m_offsetX, oWidth);

  // Vertical
  PrepareFilterCoefficients((double *) &m_filterCoeffsY[0], (int *) &m_filterOffsetsY[0], m_factorY, m_filterTapsY, m_offsetY, oHeight);


  // Note that the below functions prepare the filters according to JVT-R006
  // However, I am seeing some issues with these at this point, so keeping this disabled for now.
  // To be revisited.
  // Horizontal
  //PrepareLanczosFilterCoefficients((double *) &m_filterCoeffsX[0], (int *) &m_filterOffsetsX[0], m_factorX, m_filterTapsX, m_offsetX, oWidth, m_filterTapsX);

  // Vertical
  //PrepareLanczosFilterCoefficients((double *) &m_filterCoeffsY[0], (int *) &m_filterOffsetsY[0], m_factorY, m_filterTapsY, m_offsetY, oHeight, m_filterTapsY);

}

//FrameScaleLanczos::~FrameScaleLanczos() {
//}

//-----------------------------------------------------------------------------
// Private Methods
//-----------------------------------------------------------------------------

void FrameScaleLanczos::PrepareLanczosFilterCoefficients(double *factorCoeffs, int *filterOffsets, double factor, int filterTaps, double offset, int oSize, int lobes)
{
  int x;
  int tapIndex, index;
  double coeffSum, dist;
  double off, posOrig;
  
  double pi = (double) M_PI  / (double) lobes;

  for ( x = 0, index = 0; x < oSize; x++, index += filterTaps ) {
    posOrig = offset + (double) x * factor;
    off     = posOrig - floor(posOrig);

    coeffSum = 0.0;
    for ( tapIndex = index; tapIndex < index + filterTaps; tapIndex++ ) {
      if ( factor <= 1.0 ) {
        dist = dAbs(filterOffsets[tapIndex - index] - off);
      }
      else {
        dist = dAbs(-filterOffsets[tapIndex - index] + off);
      }
      
      factorCoeffs[tapIndex] = FilterTap( dist,  pi, factor, lobes);
      coeffSum += factorCoeffs[tapIndex];
      //printf("coefficient %d %10.7f %10.7f\n", tapIndex, dist, factorCoeffs[tapIndex]);
    }
    
    for ( tapIndex = index; tapIndex < index + filterTaps; tapIndex++ ) {
      factorCoeffs[ tapIndex ] /= coeffSum;
      //    printf("coefficient %d %10.7f %10.7f\n", tapIndex, dist, factorCoeffs[tapIndex]);
    }
  }
}

double FrameScaleLanczos::FilterTap( double dist, double piScaled, double factor, int lobes ) {
  double tap;
  dist = dAbs(dist);
  
  if ( dist > lobes )
    return 0.0;
  else {
    if (dist > 0.0) {
      double xL = (M_PI * dist) / (lobes / 2);
      double wTerm = sin ( xL ) / xL;
      double sTerm = sin( M_PI * dist / factor)/(M_PI * dist / factor);
      double gTerm = wTerm * sTerm;
      tap = gTerm;
      //            printf("tap values %10.7f %10.7f %10.7f %10.7f %10.7f %d %10.7f %10.7f\n", xL, wTerm, sTerm, dist, tap, lobes, factor, lobes / factor);
    }
    else
      tap = 1.0;
    return tap;
  }
}


double FrameScaleLanczos::FilterTap( double dist ) {
  double tap;
  dist = dAbs(dist);

  if ( dist > m_lanczosLobes )
    return 0.0;
  else {
    tap = (dist > 0.0) ? sin( M_PI * dist ) * sin ( m_pi * dist ) / (double) ( m_piSq * dist * dist ) : 1.0;
    
    //printf("tap values %10.7f %10.7f %d\n", dist, tap, m_lanczosLobes);
    return tap;
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void FrameScaleLanczos::process ( Frame* out, const Frame *inp)
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
  //      filter( inp->m_floatComp[c], out->m_floatComp[c], inp->m_width[c], inp->m_height[c], out->m_width[c], out->m_height[c], -10000000.0, 10000000.0);
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
