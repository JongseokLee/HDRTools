/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Apple Inc.
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
 * \file Conv422to444Generic.cpp
 *
 * \brief
 *    Convert 422 to 444 using a Generic separable filter approach
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
#include "Conv422to444Generic.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------


Conv422to444Generic::Conv422to444Generic(int width, int height, int method, ChromaLocation chromaLocationType[2]) {
  int offset, scale;
  int hPhase[2];
  // Currently we only support progressive formats, and thus ignore the bottom chroma location type
  switch (chromaLocationType[FP_FRAME]) {
    case CL_FIVE:
      hPhase[0] = 1;
      hPhase[1] = 3;
      break;
    case CL_FOUR:
      hPhase[0] = 0;
      hPhase[1] = 2;
      break;
    case CL_THREE:
      hPhase[0] = 1;
      hPhase[1] = 3;
      break;
    case CL_TWO:
      hPhase[0] = 0;
      hPhase[1] = 2;
      break;
    case CL_ONE:
      hPhase[0] = 1;
      hPhase[1] = 3;
      break;
    case CL_ZERO:
    default:
      hPhase[0] = 0;
      hPhase[1] = 2;
      break;
  }
   
  m_horFilter[0] = new ScaleFilter(method, 1, 2, 0, 0, &offset, &scale, hPhase[0]); //even
  m_horFilter[1] = new ScaleFilter(method, 1, 2, 0, 0, &offset, &scale, hPhase[1]); //odd
}

Conv422to444Generic::~Conv422to444Generic() {
  delete m_horFilter[0];
  delete m_horFilter[1];
}

float Conv422to444Generic::filterHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  int i;
  float value = 0.0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_floatFilter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((value + filter->m_floatOffset) * filter->m_floatScale, minValue, maxValue);
  else
    return (value + filter->m_floatOffset) * filter->m_floatScale;
}


int Conv422to444Generic::filterHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv422to444Generic::filterHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}


int Conv422to444Generic::filterHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}


//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void Conv422to444Generic::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  int inp_width  = width >> 1;

  for (j = 0; j < height; j++) {
    for (i = 0; i < inp_width; i++) {
      out[ j * width + 2 * i     ] = filterHorizontal(&inp[ j * inp_width], m_horFilter[0], i    , inp_width - 1, minValue, maxValue);
      out[ j * width + 2 * i + 1 ] = filterHorizontal(&inp[ j * inp_width], m_horFilter[1], i + 1, inp_width - 1, minValue, maxValue);
    }
  }
}

void Conv422to444Generic::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inp_width  = width >> 1;
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < inp_width; i++) {
      out[ j * width + 2 * i     ] = filterHorizontal(&inp[ j * inp_width], m_horFilter[0], i    , inp_width - 1, minValue, maxValue);
      out[ j * width + 2 * i + 1 ] = filterHorizontal(&inp[ j * inp_width], m_horFilter[1], i + 1, inp_width - 1, minValue, maxValue);
    }
  }
}

void Conv422to444Generic::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inp_width  = width >> 1;
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < inp_width; i++) {
      out[ j * width + 2 * i     ] = filterHorizontal(&inp[ j * inp_width], m_horFilter[0], i    , inp_width - 1, minValue, maxValue);
      out[ j * width + 2 * i + 1 ] = filterHorizontal(&inp[ j * inp_width], m_horFilter[1], i + 1, inp_width - 1, minValue, maxValue);
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv422to444Generic::process ( Frame* out, const Frame *inp)
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


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
