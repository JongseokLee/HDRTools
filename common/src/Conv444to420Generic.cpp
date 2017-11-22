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
 * \file Conv444to420Generic.cpp
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
#include "Conv444to420Generic.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

Conv444to420Generic::Conv444to420Generic(int width, int height, int method, ChromaLocation chromaLocationType[2], int useMinMax) {
  int offset, scale;
  int hPhase, vPhase;

  // here we allocate the entire image buffers. To save on memory we could just allocate
  // these based on filter length, but this is test code so we don't care for now.
  m_i32Data.resize  ( (width >> 1) * height );
  m_floatData.resize( (width >> 1) * height );
  
  // Currently we only support progressive formats, and thus ignore the bottom chroma location type
  switch (chromaLocationType[FP_FRAME]) {
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

  m_horFilter = new ScaleFilter(method, 0,  0,      0,     0, &offset, &scale, hPhase);
  m_verFilter = new ScaleFilter(method, 0,  2, offset, scale, &offset, &scale, vPhase);
  
  m_useMinMax = useMinMax;
  
  if (m_useMinMax != 0) {
    m_horFilterM = new ScaleFilter(DF_F0,  0,  0,      0,     0, &offset, &scale, hPhase);
    m_verFilterM = new ScaleFilter(DF_F0,  0,  2, offset, scale, &offset, &scale, vPhase);
  }
  else {
    m_horFilterM = NULL;
    m_verFilterM = NULL;
  }
}

Conv444to420Generic::~Conv444to420Generic() {
  delete m_horFilter;
  delete m_verFilter;
  m_horFilter = NULL;
  m_verFilter = NULL;
  
  if (m_horFilterM != NULL) {
    delete m_horFilterM;
    m_horFilterM = NULL;
  }
  if (m_verFilterM != NULL) {
    delete m_verFilterM;  
    m_verFilterM = NULL;
  }
}

float Conv444to420Generic::filterHorizontalMinMax(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  double dMin = 1e20;
  double dMax = -1e20;
  double dCurPixel;
  
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    value += (double) filter->m_floatFilter[i] * dCurPixel;
    if (dMin > dCurPixel)
      dMin = dCurPixel;
    if (dMax < dCurPixel)
      dMax = dCurPixel;
  }
  double dScaledValue = ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
  
  if (dScaledValue > dMax || dScaledValue < dMin ) {
    value = 0.0;    
    for (i = 0; i < m_horFilterM->m_numberOfTaps; i++) {
      dCurPixel = (double) inp[iClip(pos_x + i - m_horFilterM->m_positionOffset, 0, width)];
      value += (double) m_horFilterM->m_floatFilter[i] * dCurPixel;
    }
    dScaledValue = ((value + (double) m_horFilterM->m_floatOffset) * (double) m_horFilterM->m_floatScale);    
  }
    
  if (filter->m_clip == TRUE)
    return fClip((float) dScaledValue, minValue, maxValue);
  else
    return (float) dScaledValue;
}


float Conv444to420Generic::filterVerticalMinMax(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  double dMin = 1e20;
  double dMax = -1e20;
  double dCurPixel;
  
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
    value += (double) filter->m_floatFilter[i] * dCurPixel;
    if (dMin > dCurPixel)
      dMin = dCurPixel;
    if (dMax < dCurPixel)
      dMax = dCurPixel;
  }
  
  double dScaledValue = ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
  
  if (dScaledValue > dMax || dScaledValue < dMin ) {
    value = 0.0;
    for (i = 0; i < m_verFilterM->m_numberOfTaps; i++) {
      dCurPixel = (double) inp[iClip(pos_y + i - m_verFilterM->m_positionOffset, 0, height) * width];
      value += (double) m_verFilterM->m_floatFilter[i] * dCurPixel;
    }
    dScaledValue = ((value + (double) m_verFilterM->m_floatOffset) * (double) m_verFilterM->m_floatScale);
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) dScaledValue, -0.5, 0.5);
  else
    return (float) dScaledValue;
}

float Conv444to420Generic::filterHorizontalMinMax2(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  double dMin = 1e20;
  double dMax = -1e20;
  double dCurPixel;
  
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    value += (double) filter->m_floatFilter[i] * dCurPixel;
  }
  
  
  double dScaledValue = ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
  
  //printf("value %10.8f %10.8f  %10.8f", dScaledValue, dMax, dMin);
  value = 0.0;    
  for (i = 0; i < m_horFilterM->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_x + i - m_horFilterM->m_positionOffset, 0, width)];
    value += (double) m_horFilterM->m_floatFilter[i] * dCurPixel;
    if (dMin > dCurPixel)
      dMin = dCurPixel;
    if (dMax < dCurPixel)
      dMax = dCurPixel;
  }
  if (dScaledValue > dMax || dScaledValue < dMin ) {
    dScaledValue = ((value + (double) m_horFilterM->m_floatOffset) * (double) m_horFilterM->m_floatScale);    
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) dScaledValue, minValue, maxValue);
  else
    return (float) dScaledValue;
}


float Conv444to420Generic::filterVerticalMinMax2(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  double dMin = 1e20;
  double dMax = -1e20;
  double dCurPixel;
  
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
    value += (double) filter->m_floatFilter[i] * dCurPixel;
  }
  
  double dScaledValue = ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
  
  for (i = 0; i < m_verFilterM->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_y + i - m_verFilterM->m_positionOffset, 0, height) * width];
    if (dMin > dCurPixel)
      dMin = dCurPixel;
    if (dMax < dCurPixel)
      dMax = dCurPixel;
  }
  
  if (dScaledValue > dMax) {
    dScaledValue = dMax;    
  }
  if (dScaledValue < dMin) {
    dScaledValue = dMin;    
  }
  
  
  if (filter->m_clip == TRUE)
    return fClip((float) dScaledValue, -0.5, 0.5);
  else
    return (float) dScaledValue;
}

float Conv444to420Generic::filterHorizontalMinMax3(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  double dMin = 1e20;
  double dMax = -1e20;
  double dCurPixel;
  
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    value += (double) filter->m_floatFilter[i] * dCurPixel;
  }
  
  
  double dScaledValue = ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
  
  for (i = 0; i < m_horFilterM->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_x + i - m_horFilterM->m_positionOffset, 0, width)];
    if (dMin > dCurPixel)
      dMin = dCurPixel;
    if (dMax < dCurPixel)
      dMax = dCurPixel;
  }
  if (dScaledValue > dMax) {
    dScaledValue = dMax;    
  }
  if (dScaledValue < dMin) {
    dScaledValue = dMin;    
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) dScaledValue, minValue, maxValue);
  else
    return (float) dScaledValue;
}


float Conv444to420Generic::filterVerticalMinMax3(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  double dMin = 1e20;
  double dMax = -1e20;
  double dCurPixel;
  
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
    value += (double) filter->m_floatFilter[i] * dCurPixel;
  }
  
  double dScaledValue = ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
  
  value = 0.0;
  for (i = 0; i < m_verFilterM->m_numberOfTaps; i++) {
    dCurPixel = (double) inp[iClip(pos_y + i - m_verFilterM->m_positionOffset, 0, height) * width];
    value += (double) m_verFilterM->m_floatFilter[i] * dCurPixel;
    if (dMin > dCurPixel)
      dMin = dCurPixel;
    if (dMax < dCurPixel)
      dMax = dCurPixel;
  }
  
  if (dScaledValue > dMax || dScaledValue < dMin ) {
    dScaledValue = ((value + (double) m_verFilterM->m_floatOffset) * (double) m_verFilterM->m_floatScale);
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) dScaledValue, -0.5, 0.5);
  else
    return (float) dScaledValue;
}

float Conv444to420Generic::filterHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) filter->m_floatFilter[i] * (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    //printf("value %7.3f %7.3f\n", value, inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)]);
  }
  
  //printf("value %7.3f %7.3f %7.3f %7.3f %7.3f %d\n", value, filter->m_floatOffset, filter->m_floatScale, minValue, maxValue, filter->m_clip);
  
  if (filter->m_clip == TRUE)
    return fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), minValue, maxValue);
  else
    return (float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
}

int Conv444to420Generic::filterHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
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

int Conv444to420Generic::filterHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
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


int Conv444to420Generic::filterHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
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

float Conv444to420Generic::filterVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) filter->m_floatFilter[i] * (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), -0.5, 0.5);
  else
    return (float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
}

int Conv444to420Generic::filterVertical(const int32 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv444to420Generic::filterVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv444to420Generic::filterVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void Conv444to420Generic::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j;
  int inp_width  = 2 * width;
  int inputHeight = 2 * height;

  if (m_useMinMax == 1) {
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < width; i++) {
        m_floatData[ j * width + i ] = filterHorizontalMinMax(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, 0.0, 0.0);
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
        out[ j * width + i ] = filterVerticalMinMax(&m_floatData[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
      }
    }
  }
  else if (m_useMinMax == 2) {
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < width; i++) {
        m_floatData[ j * width + i ] = filterHorizontalMinMax2(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, 0.0, 0.0);
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
        out[ j * width + i ] = filterVerticalMinMax2(&m_floatData[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
      }
    }
  }
  else if (m_useMinMax == 3) {
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < width; i++) {
        m_floatData[ j * width + i ] = filterHorizontalMinMax3(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, 0.0, 0.0);
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
        out[ j * width + i ] = filterVerticalMinMax3(&m_floatData[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
      }
    }
  }
  else {
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < width; i++) {
        m_floatData[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, 0.0, 0.0);
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
        out[ j * width + i ] = filterVertical(&m_floatData[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
      }
    }
  }
}

void Conv444to420Generic::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inp_width = 2 * width;
  int inputHeight = 2 * height;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      m_i32Data[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, minValue, maxValue);
    }
  }

  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&m_i32Data[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

void Conv444to420Generic::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inp_width = 2 * width;
  int inputHeight = 2 * height;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      m_i32Data[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilter, 2 * i, inp_width - 1, 0, 0);
    }
  }
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      out[ j * width + i ] = filterVertical(&m_i32Data[i], m_verFilter, (2 * j), width, inputHeight - 1, minValue, maxValue);
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv444to420Generic::process ( Frame* out, const Frame *inp)
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
