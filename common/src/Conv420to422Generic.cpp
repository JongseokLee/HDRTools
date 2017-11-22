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
 * \file Conv420to422Generic.cpp
 *
 * \brief
 *    Convert 420 to 422 using a Generic separable filter approach
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
#include "Conv420to422Generic.H"
#include "ScaleFilter.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ConvertFilter420To422::ConvertFilter420To422(int filter, int paramsMode, int inOffset, int inShift, int *outOffset, int *outShift) {
  switch (filter){
    case UCF_F0:
      m_numberOfTaps   = 4;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = -2.0;
      m_floatFilter[1] = 16.0;
      m_floatFilter[2] = 54.0;
      m_floatFilter[3] = -4.0;
      *outOffset      = 32;
      *outShift       = 6;
      break;
    case UCF_F1:
      m_numberOfTaps   = 4;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = -4.0;
      m_floatFilter[1] = 54.0;
      m_floatFilter[2] = 16.0;
      m_floatFilter[3] = -2.0;
      *outOffset      = 32;
      *outShift       = 6;
      break;
    case UCF_F2:
      m_numberOfTaps   = 4;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = -4.0;
      m_floatFilter[1] = 36.0;
      m_floatFilter[2] = 36.0;
      m_floatFilter[3] = -4.0;
      *outOffset      = 32;
      *outShift       = 6;
      break;
    case UCF_F3:
    default:
      m_numberOfTaps   = 2;
      m_floatFilter    = new float[m_numberOfTaps];
      m_floatFilter[0] = 0.0;
      m_floatFilter[1] = 1.0;
      *outOffset      = 0;
      *outShift       = 0;
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
    m_clip      = FALSE;
  }
  else { // additive mode
    m_i16Shift  = ( *outShift  + inShift  );
    m_i16Offset = ( 1 << (m_i16Shift) ) >> 1;
    m_clip      = FALSE;
  }
  
  m_positionOffset = (m_numberOfTaps + 1) >> 1;
  m_floatOffset    = (float) m_i16Offset;
  m_floatScale     = 1.0f / ((float) (1 << m_i16Shift));
}

ConvertFilter420To422::~ConvertFilter420To422() {
  if ( m_floatFilter != NULL ) {
    delete [] m_floatFilter;
    m_floatFilter = NULL;
  }
  if ( m_i16Filter != NULL ) {
    delete [] m_i16Filter;
    m_i16Filter = NULL;
  }
}


Conv420to422Generic::Conv420to422Generic(int width, int height, int method) {
  int offset, scale;
  // here we allocate the entire image buffers. To save on memory we could just allocate
  // these based on filter length, but this is test code so we don't care for now.
  switch (method) {
    case UF_F0:
    default:
      m_verFilter[0] = new ConvertFilter420To422(UCF_F0, 1,      0,     0, &offset, &scale); //even
      m_verFilter[1] = new ConvertFilter420To422(UCF_F1, 1,      0,     0, &offset, &scale); //odd
      break;
  }
}

Conv420to422Generic::~Conv420to422Generic() {
  delete m_verFilter[0];
  delete m_verFilter[1];
}

float Conv420to422Generic::filterVertical(const float *inp, const ConvertFilter420To422 *filter, int pos_y, int width, int height, float minValue, float maxValue) {
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


int Conv420to422Generic::filterVertical(const int16 *inp, const ConvertFilter420To422 *filter, int pos_y, int width, int height, int minValue, int maxValue) {
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

int Conv420to422Generic::filterVertical(const uint16 *inp, const ConvertFilter420To422 *filter, int pos_y, int width, int height, int minValue, int maxValue) {
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

int Conv420to422Generic::filterVertical(const imgpel *inp, const ConvertFilter420To422 *filter, int pos_y, int width, int height, int minValue, int maxValue) {
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
void Conv420to422Generic::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  int inputHeight = height >> 1;

  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      out[ (2 * j    ) * width + i ] = filterVertical(&inp[i], m_verFilter[0], j    , width, inputHeight - 1, minValue, maxValue);
      out[ (2 * j + 1) * width + i ] = filterVertical(&inp[i], m_verFilter[1], j + 1, width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv420to422Generic::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inputHeight = height >> 1;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      out[ (2 * j    ) * width + i ] = filterVertical(&inp[i], m_verFilter[0], j    , width, inputHeight - 1, minValue, maxValue);
      out[ (2 * j + 1) * width + i ] = filterVertical(&inp[i], m_verFilter[1], j + 1, width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv420to422Generic::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inputHeight = height >> 1;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      out[ (2 * j    ) * width + i ] = filterVertical(&inp[i], m_verFilter[0], j    , width, inputHeight - 1, minValue, maxValue);
      out[ (2 * j + 1) * width + i ] = filterVertical(&inp[i], m_verFilter[1], j + 1, width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv420to422Generic::filterInterlaced(float *out, float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  
  float *oChromaLineCurrent = out;
  float *iChromaLineCurrent = inp, *iChromaLineNext = NULL;
  
  float *oLineCurrentEven = NULL;
  float *oLineCurrentOdd = NULL;
  
  
  // first top line
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  // second (bottom) line
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  
  // process center area
  for (j = ZERO; j < (height >> 2) - 1; j ++) {
    oLineCurrentEven   = &out[((j << 2)  + 2) * width];
    oLineCurrentOdd    = oLineCurrentEven + 2 * width;
    iChromaLineCurrent = &inp[(j << 1) * width];
    iChromaLineNext    = iChromaLineCurrent + 2 * width;
    // top
    for (i = ZERO; i < width; i++) {
      *(oLineCurrentEven++) = (5.0f * *(iChromaLineCurrent  ) + 3.0f * *(iChromaLineNext  ) + 4.0f) / 8.0f;
      *(oLineCurrentOdd++)  = (       *(iChromaLineCurrent++) + 7.0f * *(iChromaLineNext++) + 4.0f) / 8.0f;
    }
    // bottom
    for (i = ZERO; i < width; i++) {
      *(oLineCurrentEven++) = (7.0f * *(iChromaLineCurrent  ) +        *(iChromaLineNext  ) + 4.0f) / 8.0f;
      *(oLineCurrentOdd++)  = (3.0f * *(iChromaLineCurrent++) + 5.0f * *(iChromaLineNext++) + 4.0f) / 8.0f;
    }
  }
  
  // Process remaining two lines on the bottom
  oChromaLineCurrent = oLineCurrentOdd;
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
}

void Conv420to422Generic::filterInterlaced(uint16 *out, uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  
  uint16 *oChromaLineCurrent = out;
  uint16 *iChromaLineCurrent = inp, *iChromaLineNext = NULL;
  
  uint16 *oLineCurrentEven = NULL;
  uint16 *oLineCurrentOdd = NULL;

  
  // first top line
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  // second (bottom) line
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }

  // process center area
  for (j = ZERO; j < (height >> 2) - 1; j ++) {
    oLineCurrentEven = &out[((j << 2)  + 2) * width];
    oLineCurrentOdd = oLineCurrentEven + 2 * width;
    iChromaLineCurrent  = &inp[(j << 1) * width];
    iChromaLineNext     = iChromaLineCurrent + 2 * width;
    // top
    for (i = ZERO; i < width; i++) {
      *(oLineCurrentEven++) = iShiftRightRound((5 * *(iChromaLineCurrent  ) + 3 * *(iChromaLineNext  )), 3);
      *(oLineCurrentOdd++)  = iShiftRightRound((    *(iChromaLineCurrent++) + 7 * *(iChromaLineNext++)), 3);
    }
    // bottom
    for (i = ZERO; i < width; i++) {
      *(oLineCurrentEven++) = iShiftRightRound((7 * *(iChromaLineCurrent  ) +     *(iChromaLineNext  )), 3);
      *(oLineCurrentOdd++)  = iShiftRightRound((3 * *(iChromaLineCurrent++) + 5 * *(iChromaLineNext++)), 3);
    }
  }

  // Process remaining two lines on the bottom
  oChromaLineCurrent = oLineCurrentOdd;
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
}

void Conv420to422Generic::filterInterlaced(imgpel *out, imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  
  imgpel *oChromaLineCurrent = out;
  imgpel *iChromaLineCurrent = inp, *iChromaLineNext = NULL;
  
  imgpel *oLineCurrentEven = NULL;
  imgpel *oLineCurrentOdd = NULL;
  
  
  // first top line
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  // second (bottom) line
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  
  // process center area
  for (j = ZERO; j < (height >> 2) - 1; j ++) {
    oLineCurrentEven = &out[((j << 2)  + 2) * width];
    oLineCurrentOdd = oLineCurrentEven + 2 * width;
    iChromaLineCurrent  = &inp[(j << 1) * width];
    iChromaLineNext     = iChromaLineCurrent + 2 * width;
    // top
    for (i = ZERO; i < width; i++) {
      *(oLineCurrentEven++) = iShiftRightRound((5 * *(iChromaLineCurrent  ) + 3 * *(iChromaLineNext  )), 3);
      *(oLineCurrentOdd++)  = iShiftRightRound((    *(iChromaLineCurrent++) + 7 * *(iChromaLineNext++)), 3);
    }
    // bottom
    for (i = ZERO; i < width; i++) {
      *(oLineCurrentEven++) = iShiftRightRound((7 * *(iChromaLineCurrent  ) +     *(iChromaLineNext  )), 3);
      *(oLineCurrentOdd++)  = iShiftRightRound((3 * *(iChromaLineCurrent++) + 5 * *(iChromaLineNext++)), 3);
    }
  }
  
  // Process remaining two lines on the bottom
  oChromaLineCurrent = oLineCurrentOdd;
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
  for (i = ZERO; i < width; i++) {
    *(oChromaLineCurrent++) = *(iChromaLineCurrent++);
  }
}
//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv420to422Generic::process ( Frame* out, const Frame *inp)
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
