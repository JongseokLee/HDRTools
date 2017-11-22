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
 * \file Conv420to444Generic.cpp
 *
 * \brief
 *    Convert 420 to 444 using a Generic separable filter approach
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
#include "ScaleFilter.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define TEST_FILTER 0

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
ScaleFilter::ScaleFilter(int filter, int mode, int paramsMode, int inOffset, int inShift, int *outOffset, int *outShift, int phase) {
  int index = 0;

  if (mode == 0) { // Downsample
    // subtract two here to account for the NN and bilinear filters
    filter = filter - 2;
    m_numberOfTaps = (int) DCF_Filters[filter][phase][index];

    m_floatFilter  = new float[m_numberOfTaps];
    for (index = 1; index <= m_numberOfTaps; index++) {
      m_floatFilter[index - 1] = (float) DCF_Filters[filter][phase][index];
    }
    index = m_numberOfTaps + 1;
    *outOffset = (int) DCF_Filters[filter][phase][index++];
    *outShift  = (int) DCF_Filters[filter][phase][index];
    
  }
  else { // Upsample
    // subtract one here to account for the NN filter
    filter = filter - 1;
    m_numberOfTaps = (int) UCF_Filters[filter][phase][index];
    
    m_floatFilter  = new float[m_numberOfTaps];
    for (index = 1; index <= m_numberOfTaps; index++) {
      m_floatFilter[index - 1] = (float) UCF_Filters[filter][phase][index];
    }
    index = m_numberOfTaps + 1;
    *outOffset = (int) UCF_Filters[filter][phase][index++];
    *outShift  = (int) UCF_Filters[filter][phase][index];
  }
  
  m_i32Filter = new int32[m_numberOfTaps];
  for (int i = 0; i < m_numberOfTaps; i++) {
    m_i32Filter[i] = (int32) fRound(m_floatFilter[i]);
    //m_i32Filter[i] = (int32) m_floatFilter[i];
  }

  // currently we only support powers of 2 for the divisor
  if (paramsMode == 0) { // zero mode
    m_i32Offset = 0;
    m_i32Shift  = 0;   
    m_clip      = FALSE;
  }
  else if (paramsMode == 1) { // normal mode
    m_i32Offset = *outOffset;
    m_i32Shift  = *outShift;
    if (mode == 0) 
      m_clip    = TRUE;
    else
      m_clip    = FALSE;
  }
  else { // additive mode
    m_i32Shift  = ( *outShift  + inShift  );
    m_i32Offset = ( 1 << (m_i32Shift) ) >> 1;
    if (mode == 0) 
      m_clip    = TRUE;
    else
      m_clip    = FALSE;
  }

  if (mode == 0) {
    m_positionOffset = (m_numberOfTaps - 1) >> 1;
  }
  else {
    m_positionOffset = (m_numberOfTaps + 1) >> 1;
  }
  m_floatOffset    = 0.0f;
  //m_floatScale     = 1.0f / ((float) (1 << m_i32Shift));
  m_floatScale     = 1.0f / ((float) (1 << *outShift));
  
  //printf("scale %10.8f %10.8f\n", m_floatScale, 1.0f / ((float) (1 << *outShift)));
  //printf("taps (%d %d) {", m_numberOfTaps, m_positionOffset);
  //for (int i = 0; i < m_numberOfTaps; i++) {
  //printf("%7.3f ", m_floatFilter[i]);
  //printf("%d ", m_i32Filter[i]);
  //}
  //printf("} %d %d %d %d\n", *outOffset, *outShift, m_i32Offset, m_i32Shift);
}

ScaleFilter::~ScaleFilter() {
  if ( m_floatFilter != NULL ) {
    delete [] m_floatFilter;
    m_floatFilter = NULL;
  }
  if ( m_i32Filter != NULL ) {
    delete [] m_i32Filter;
    m_i32Filter = NULL;
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
