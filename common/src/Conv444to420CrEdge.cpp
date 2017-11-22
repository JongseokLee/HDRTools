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
 * \file Conv444to420CrEdge.cpp
 *
 * \brief
 *    Convert 444 to 420 using an adaptive, Chroma edge detection method
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
#include "Conv444to420CrEdge.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

Conv444to420CrEdge::Conv444to420CrEdge(int width, int height, int method, ChromaLocation chromaLocationType[2], int useMinMax) {
  int offset,     scale;
  int downOffset, downScale;
  int hPhase, vPhase;

  // here we allocate the entire image buffers. To save on memory we could just allocate
  // these based on filter length, but this is test code so we don't care for now.
  m_i32Data.resize  ( (width >> 1) * height );
  m_floatData.resize( (width >> 1) * height );

  
  // Currently we only support progressive formats, and thus ignore the bottom chroma location type
  
  switch (chromaLocationType[FP_FRAME]) {
    case CL_FIVE:
      hPhase = 1;
      vPhase = 0;
      break;
    case CL_FOUR:
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

// Downsampling filters
  for (int index = 0; index < 5; index++) {
    m_horFilterDown[index] = new ScaleFilter(DF_GS - index, 0,  0,      0,     0, &offset, &scale, hPhase);
    m_verFilterDown[index] = new ScaleFilter(DF_GS - index, 0,  2, offset, scale, &downOffset, &downScale, vPhase);
  }
  
  m_edgeClassifier = 0.10;
}

Conv444to420CrEdge::~Conv444to420CrEdge() {
  for (int index = 0; index < 5; index++) {
    if (m_horFilterDown[index] != NULL) {
      delete m_horFilterDown[index];
      m_horFilterDown[index] = NULL;
    }
    if (m_verFilterDown[index] != NULL) {
      delete m_verFilterDown[index];
      m_verFilterDown[index] = NULL;
    }
  }
}

float Conv444to420CrEdge::filterHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  double value = 0.0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) filter->m_floatFilter[i] * (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), minValue, maxValue);
  else
    return (float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
}

int Conv444to420CrEdge::filterHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv444to420CrEdge::filterHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv444to420CrEdge::filterHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

bool Conv444to420CrEdge::analyzeHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  double value = 0.0;
  // compute mean
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  value /= (double) (filter->m_numberOfTaps + 1);

  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (dAbs(value - (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)]) > m_edgeClassifier)
      return 0;
  }
  
  return 1;
}

bool Conv444to420CrEdge::analyzeHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  value /= (filter->m_numberOfTaps + 1);
  
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (iAbs(value - inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)]) > (int) dRound(m_edgeClassifier * (double) (maxValue - minValue)))
      return 0;
  }
  
  return 1;
}

bool Conv444to420CrEdge::analyzeHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  value /= (filter->m_numberOfTaps + 1);
  
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (iAbs(value - inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)]) > (int) dRound(m_edgeClassifier * (double)(maxValue - minValue)))
      return 0;
  }
  
  return 1;
}

bool Conv444to420CrEdge::analyzeHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  value /= (filter->m_numberOfTaps + 1);
  
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (iAbs(value - inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)]) > (int) dRound(m_edgeClassifier * (double)(maxValue - minValue)))
      return 0;
  }
  return 1;
}

float Conv444to420CrEdge::filterVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  double value = 0.0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) filter->m_floatFilter[i] * (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), -0.5, 0.5);
  else
    return (float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
}

int Conv444to420CrEdge::filterVertical(const int32 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv444to420CrEdge::filterVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv444to420CrEdge::filterVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

bool Conv444to420CrEdge::analyzeVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  double value = 0.0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  value /= (double) (filter->m_numberOfTaps + 1);
  
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (dAbs(value - (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width]) > m_edgeClassifier)
      return 0;
  }
  
  return 1;
}

bool Conv444to420CrEdge::analyzeVertical(const int32 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  value /= (filter->m_numberOfTaps + 1);
  
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (iAbs(value - inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width]) > (int) dRound(m_edgeClassifier * (double) (maxValue - minValue)))
      return 0;
  }

  return 1;
}

bool Conv444to420CrEdge::analyzeVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  value /= (filter->m_numberOfTaps + 1);
  
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (iAbs(value - inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width]) > (int) dRound(m_edgeClassifier * (double) (maxValue - minValue)))
      return 0;
  }
  
  return 1;
}

bool Conv444to420CrEdge::analyzeVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  value /= (filter->m_numberOfTaps + 1);
  
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    if (iAbs(value - inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width]) > (int) dRound(m_edgeClassifier * (double) (maxValue - minValue)))
      return 0;
  }
  
  return 1;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void Conv444to420CrEdge::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j, index;
  int inp_width  = 2 * width;
  int inputHeight = 2 * height;
  bool  testSamples;
    
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, 0.0, 0.0);
        if (testSamples == TRUE || index == 4) {
          m_floatData[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, 0.0, 0.0);
          break;
        }
      }
    }
  }    
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&m_floatData[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
        out[ j * width + i ] = filterVertical(&m_floatData[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
}

void Conv444to420CrEdge::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j, index;
  int inp_width  = 2 * width;
  int inputHeight = 2 * height;
  bool  testSamples;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          m_i32Data[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, minValue, maxValue);
          break;
        }
      }
    }
  }    
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&m_i32Data[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          out[ j * width + i ] = filterVertical(&m_i32Data[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
}

void Conv444to420CrEdge::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j, index;
  int inp_width  = 2 * width;
  int inputHeight = 2 * height;
  bool  testSamples;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          m_i32Data[ j * width + i ] = filterHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, minValue, maxValue);
          break;
        }
      }
    }
  }    
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&m_i32Data[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          out[ j * width + i ] = filterVertical(&m_i32Data[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv444to420CrEdge::process ( Frame* out, const Frame *inp)
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
