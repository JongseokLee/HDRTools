/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Apple Inc.
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
 * \file Conv422to420Generic.cpp
 *
 * \brief
 *    Convert 444 to 420 using a Generic separable filter approach
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
#include "Conv422to420Generic.H"
#include "ScaleFilter.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ConvertFilter422To420::ConvertFilter422To420(int filter, int paramsMode, int inOffset, int inShift, int *outOffset, int *outShift) {
  switch (filter){          
    case DCF_MPEG2_TM5_H:  // no horizontal spatial shift (co-sited as per default chroma sample loc type)
      // MPEG-2 Test Model 5   4:4:4 to 4:2:2 (horizontal) filter: [21, 0, -52, 0, 159, 256, 159, 0, -52, 0, 21 ]
      m_numberOfTaps   = 11;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = m_floatFilter[10] = +21.0;
      m_floatFilter[2] = m_floatFilter[8] = -52.0;
      m_floatFilter[4] = m_floatFilter[6] = +159.0;
      m_floatFilter[5] = +256.0;  // center tap
      m_floatFilter[1] = m_floatFilter[3] = m_floatFilter[7] = m_floatFilter[9] = 0.0;  // zero crossings
      *outOffset      = 5;
      *outShift       = 9;
      break;
    case DCF_MPEG2_TM5_V:  // vertical shift of half sample as per default chroma sample loc type
     // MPEG-2 Test Model 5   4:2:2 to 4:2:0 (vertical) filter: [ 5, 11, -21, -37, 70, 228, 228, 70, -37, -21, 11, 5 ]
        m_numberOfTaps   = 12;
        m_floatFilter    = new float[m_numberOfTaps];
        m_floatFilter[0] = m_floatFilter[11] = +5.0;
        m_floatFilter[1] = m_floatFilter[10] = +11.0;
        m_floatFilter[2] = m_floatFilter[9] = -21.0;
        m_floatFilter[3] = m_floatFilter[8] = -37.0;
        m_floatFilter[4] = m_floatFilter[7] = +70.0;
        m_floatFilter[5] = m_floatFilter[6] = +228.0;  // center
        *outOffset      = 5;
        *outShift       = 9;
        break;
    case DCF_F1:
      m_numberOfTaps   = 3;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = m_floatFilter[2] = 1.0;
      m_floatFilter[1] = 2.0;
      *outOffset      = 2;
      *outShift       = 2;
      break;
    case DCF_F0:
      m_numberOfTaps   = 3;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = m_floatFilter[2] = 1.0;
      m_floatFilter[1] = 6.0;
      *outOffset      = 4;
      *outShift       = 3;
      break;
    case DCF_BILINEAR:
    default:
      m_numberOfTaps   = 2;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = m_floatFilter[1] = 1.0;
      *outOffset      = 1;
      *outShift       = 1;
      break;
  }
  
  m_i16Filter = new int16[m_numberOfTaps];
  for (int i = 0; i < m_numberOfTaps; i++) {
    m_i16Filter[i] = (int16) m_floatFilter[i];
  }

  // currently we only support powers of 2 for the divisor
  if (paramsMode == 0) { // zero mode
    m_i16Offset = 0;
    m_i16Shift  = 0;
    m_clip      = FALSE;
  }
  else if (paramsMode == 1) { // normal mode
    m_i16Offset = *outOffset;
    m_i16Shift  = *outShift;
    m_clip      = TRUE;
  }
  else { // additive mode
    m_i16Shift  = ( *outShift  + inShift  );
    m_i16Offset = ( 1 << (m_i16Shift) ) >> 1;
    m_clip      = TRUE;
  }

  m_positionOffset = (m_numberOfTaps + 1) >> 1;
  m_floatOffset    = (float) m_i16Offset;
  m_floatScale     = 1.0f / ((float) (1 << m_i16Shift));
}

ConvertFilter422To420::~ConvertFilter422To420() {
  if ( m_floatFilter != NULL ) {
    delete [] m_floatFilter;
    m_floatFilter = NULL;
  }
  if ( m_i16Filter != NULL ) {
    delete [] m_i16Filter;
    m_i16Filter = NULL;
  }
}


Conv422to420Generic::Conv422to420Generic(int width, int height, int method) {
  int offset, scale;
  
  switch (method) {
   case DF_TM:  // MPEG-2 test model 5
      m_verFilter = new ConvertFilter422To420(DCF_MPEG2_TM5_V,  0, 0, 0, &offset, &scale);
      break;
    case DF_F1:
      m_verFilter = new ConvertFilter422To420(DCF_BILINEAR   ,  2, 0, 0, &offset, &scale);
      break;
    case DF_F0:
    default:
      m_verFilter = new ConvertFilter422To420(DCF_BILINEAR   ,  2, 0, 0, &offset, &scale);
      break;
  }
}

Conv422to420Generic::~Conv422to420Generic() {
  delete m_verFilter;
}

float Conv422to420Generic::filterVertical(const float *inp, const ConvertFilter422To420 *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  int i;
  float value = 0.0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_floatFilter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((value + filter->m_floatOffset) * filter->m_floatScale, minValue, maxValue);
  else
    return (value + filter->m_floatOffset) * filter->m_floatScale;
}


int Conv422to420Generic::filterVertical(const int16 *inp, const ConvertFilter422To420 *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i16Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i16Offset) >> filter->m_i16Shift, minValue, maxValue);
  else
    return (value + filter->m_i16Offset) >> filter->m_i16Shift;
}

int Conv422to420Generic::filterVertical(const uint16 *inp, const ConvertFilter422To420 *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i16Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i16Offset) >> filter->m_i16Shift, minValue, maxValue);
  else
    return (value + filter->m_i16Offset) >> filter->m_i16Shift;
}

int Conv422to420Generic::filterVertical(const imgpel *inp, const ConvertFilter422To420 *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i16Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i16Offset) * filter->m_i16Shift, minValue, maxValue);
  else
    return (value + filter->m_i16Offset) >> filter->m_i16Shift;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void Conv422to420Generic::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  int inputHeight = 2 * height;

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&inp[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv422to420Generic::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inputHeight = 2 * height;
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&inp[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv422to420Generic::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inputHeight = 2 * height;
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&inp[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv422to420Generic::filterInterlaced(float *out, float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  
  float *iChromaLineCurrent, *iChromaLineNext, *oChromaLineCurrent;
  
  // process top field
  for (j = ZERO; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[(j << 1) * width];
    iChromaLineNext = &inp[((j + 1) << 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (3.0f * *(iChromaLineCurrent++) + *(iChromaLineNext++) + 2.0f) / 4.0f;
    }
  }
  
  // process bottom field
  for (j = ONE; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[((j << 1) - 1) * width];
    iChromaLineNext = &inp[(((j + 1) << 1) - 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (*(iChromaLineCurrent++) + 3.0f * *(iChromaLineNext++) + 2.0f) / 4.0f;
    }
  }
}

void Conv422to420Generic::filterInterlaced(uint16 *out, uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  
  uint16 *iChromaLineCurrent, *iChromaLineNext, *oChromaLineCurrent;
  
  // process top field
  for (j = ZERO; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[(j << 1) * width];
    iChromaLineNext = &inp[((j + 1) << 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (3 * *(iChromaLineCurrent++) + *(iChromaLineNext++) + 2) >> 2;
    }
  }
  
  // process bottom field
  for (j = ONE; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[((j << 1) - 1) * width];
    iChromaLineNext = &inp[(((j + 1) << 1) - 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (*(iChromaLineCurrent++) + 3 * *(iChromaLineNext++) + 2) >> 2;
    }
  }
}

void Conv422to420Generic::filterInterlaced(imgpel *out, imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  
  imgpel *iChromaLineCurrent, *iChromaLineNext, *oChromaLineCurrent;
  
  // process top field
  for (j = ZERO; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[(j << 1) * width];
    iChromaLineNext = &inp[((j + 1) << 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (3 * *(iChromaLineCurrent++) + *(iChromaLineNext++) + 2) >> 2;
    }
  }
  
  // process bottom field
  for (j = ONE; j < height; j += 2) {
    oChromaLineCurrent = &out[j * width];
    iChromaLineCurrent = &inp[((j << 1) - 1) * width];
    iChromaLineNext = &inp[(((j + 1) << 1) - 1) * width];
    
    for (i = ZERO; i < width; i++) {
      *(oChromaLineCurrent++) = (*(iChromaLineCurrent++) + 3 * *(iChromaLineNext++) + 2) >> 2;
    }
  }
}
//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv422to420Generic::process ( Frame* out, const Frame *inp)
{
  if (( out->m_isFloat != inp->m_isFloat ) || (( inp->m_isFloat == 0 ) && ( out->m_bitDepth != inp->m_bitDepth ))) {
    fprintf(stderr, "Error: trying to copy frames of different data types. \n");
    exit(EXIT_FAILURE);
  }
  
  if (out->m_compSize[Y_COMP] != inp->m_compSize[Y_COMP]) {
    fprintf(stderr, "Error: trying to copy frames of different sizes (%d  vs %d). \n",out->m_compSize[Y_COMP], inp->m_compSize[Y_COMP]);
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
  
  if (inp->m_format.m_isInterlaced == TRUE) {
    if (out->m_isFloat == TRUE) {    // floating point data
      memcpy(out->m_floatComp[Y_COMP], inp->m_floatComp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(float));
      for (c = U_COMP; c <= V_COMP; c++) {
        filterInterlaced(out->m_floatComp[c], inp->m_floatComp[c], out->m_width[c], out->m_height[c], (float) out->m_minPelValue[c], (float) out->m_maxPelValue[c] );
      }
    }
    else if (out->m_bitDepth == 8) {   // 8 bit data
      memcpy(out->m_comp[Y_COMP], inp->m_comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(imgpel));
      for (c = U_COMP; c <= V_COMP; c++) {
        filterInterlaced(out->m_comp[c], inp->m_comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
    else { // 16 bit data
      memcpy(out->m_ui16Comp[Y_COMP], inp->m_ui16Comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(uint16));
      for (c = U_COMP; c <= V_COMP; c++) {
        filterInterlaced(out->m_ui16Comp[c], inp->m_ui16Comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
    
  }
  else {
    if (out->m_isFloat == TRUE) {    // floating point data
      memcpy(out->m_floatComp[Y_COMP], inp->m_floatComp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(float));
      for (c = U_COMP; c <= V_COMP; c++) {
        filter(out->m_floatComp[c], inp->m_floatComp[c], out->m_width[c], out->m_height[c], (float) out->m_minPelValue[c], (float) out->m_maxPelValue[c] );
      }
    }
    else if (out->m_bitDepth == 8) {   // 8 bit data
      memcpy(out->m_comp[Y_COMP], inp->m_comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(imgpel));
      for (c = U_COMP; c <= V_COMP; c++) {
        filter(out->m_comp[c], inp->m_comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
    else { // 16 bit data
      memcpy(out->m_ui16Comp[Y_COMP], inp->m_ui16Comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(uint16));
      for (c = U_COMP; c <= V_COMP; c++) {
        filter(out->m_ui16Comp[c], inp->m_ui16Comp[c], out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
      }
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
