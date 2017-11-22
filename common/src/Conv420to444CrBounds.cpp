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
 * \file Conv420to444CrBounds.cpp
 *
 * \brief
 *    Convert 420 to 444 using an adaptive filter selection approach
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
#include "Conv420to444CrBounds.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------


Conv420to444CrBounds::Conv420to444CrBounds(int width, int height, int method, ChromaLocation chromaLocationType[2]) {
  int hPhase[2];
  int vPhase[2];
  // here we allocate the entire image buffers. To save on memory we could just allocate
  // these based on filter length, but this is test code so we don't care for now.
  m_i32Data.resize  (width * height * 2);
  m_floatData.resize(width * height * 2);
  // Currently we only support progressive formats, and thus ignore the bottom chroma location type
  switch (chromaLocationType[FP_FRAME]) {
    case CL_FIVE:
      hPhase[0] = 1;
      hPhase[1] = 3;
      vPhase[0] = 2;
      vPhase[1] = 0;
      break;
    case CL_FOUR:
      hPhase[0] = 0;
      hPhase[1] = 2;
      vPhase[0] = 2;
      vPhase[1] = 0;
      break;
    case CL_THREE:
      hPhase[0] = 1;
      hPhase[1] = 3;
      vPhase[0] = 0;
      vPhase[1] = 2;
      break;
    case CL_TWO:
      hPhase[0] = 0;
      hPhase[1] = 2;
      vPhase[0] = 0;
      vPhase[1] = 2;
      break;
    case CL_ONE:
      hPhase[0] = 1;
      hPhase[1] = 3;
      vPhase[0] = 1;
      vPhase[1] = 3;
      break;
    case CL_ZERO:
    default:
      hPhase[0] = 0;
      hPhase[1] = 2;
      vPhase[0] = 1;
      vPhase[1] = 3;
      break;
  }
   
  setupFilter(0, UF_LS6,  hPhase, vPhase); // L6
  setupFilter(1, UF_LS5,  hPhase, vPhase); // L5
  setupFilter(2, UF_LS4,  hPhase, vPhase); // L4
  setupFilter(3, UF_LS3,  hPhase, vPhase); // L3
  setupFilter(4, UF_F0,   hPhase, vPhase); // L2
  
  m_edgeClassifier = 0.15;
}

Conv420to444CrBounds::~Conv420to444CrBounds() {

for (int phase = 0; phase < 2; phase++) {
  for (int index = 0; index < 5; index++) {
    if (m_horFilter[phase][index] != NULL) {
      delete m_horFilter[phase][index];
      m_horFilter[phase][index] = NULL;
    }
    if (m_verFilter[phase][index] != NULL) {
      delete m_verFilter[phase][index];
      m_verFilter[phase][index] = NULL;
    }
  }
  }
}

void Conv420to444CrBounds::setupFilter(int index, UpSamplingFilters filter, int *hPhase, int *vPhase) {
  int offset,     scale;  
  
  m_verFilter[0][index] = new ScaleFilter(filter, 1, 0,      0,     0, &offset, &scale, vPhase[0]); //even
  m_horFilter[0][index] = new ScaleFilter(filter, 1, 2, offset, scale, &offset, &scale, hPhase[0]); //even
  m_verFilter[1][index] = new ScaleFilter(filter, 1, 0,      0,     0, &offset, &scale, vPhase[1]); //odd
  m_horFilter[1][index] = new ScaleFilter(filter, 1, 2, offset, scale, &offset, &scale, hPhase[1]); //odd
}

float Conv420to444CrBounds::filterHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  double value = 0.0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) filter->m_floatFilter[i] * (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), minValue, maxValue);
  else
    return (float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
}


int Conv420to444CrBounds::filterHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv420to444CrBounds::filterHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}


int Conv420to444CrBounds::filterHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

bool Conv420to444CrBounds::analyzeHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
  double minRange =  1e37;
  double maxRange = -1e37;
  
  double value = 0.0;
  // compute bounds
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = (double) inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }
  
    return 1;
}

bool Conv420to444CrBounds::analyzeHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int minRange = INT_MAX;
  int maxRange = INT_MIN;
  
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }
  
  return 1;
}

bool Conv420to444CrBounds::analyzeHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int minRange = INT_MAX;
  int maxRange = INT_MIN;
  
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }
  return 1;
}

bool Conv420to444CrBounds::analyzeHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int minRange = INT_MAX;
  int maxRange = INT_MIN;
  
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }
  return 1;
}
float Conv420to444CrBounds::filterVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  double value = 0.0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) filter->m_floatFilter[i] * (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), minValue, maxValue);
  else
    return (float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
}


int Conv420to444CrBounds::filterVertical(const int32 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }

  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv420to444CrBounds::filterVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv420to444CrBounds::filterVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}


bool Conv420to444CrBounds::analyzeVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  double minRange =  1e37;
  double maxRange = -1e37;

  double value = 0.0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];

    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }
  
  return 1;
}

bool Conv420to444CrBounds::analyzeVertical(const int32 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int minRange = INT_MAX;
  int maxRange = INT_MIN;
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }

  return 1;
}

bool Conv420to444CrBounds::analyzeVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int minRange = INT_MAX;
  int maxRange = INT_MIN;
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }
  
  return 1;
}
                    
bool Conv420to444CrBounds::analyzeVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int minRange = INT_MAX;
  int maxRange = INT_MIN;
  int value = 0;
  for (int i = 0; i < filter->m_numberOfTaps; i++) {
    value = inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
    if (value < minRange)
      minRange = value;
    if (value > maxRange)
      maxRange = value;
    if ((maxRange - minRange) > m_edgeClassifier)
      return 0;
  }
  
  return 1;
}
                    
//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void Conv420to444CrBounds::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j, index;
  int inp_width  = width >> 1;
  int inputHeight = height >> 1;
  bool  testSamples;

  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < inp_width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&inp[i], m_verFilter[0][index], j    , inp_width, inputHeight - 1, 0.0, 0.0);
        if (testSamples == TRUE || index == 4) {
          m_floatData[ (2 * j    ) * inp_width + i ] = filterVertical(&inp[i], m_verFilter[0][index], j    , inp_width, inputHeight - 1, 0.0, 0.0);
          break;
        }
      }
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&inp[i], m_verFilter[1][index], j    , inp_width, inputHeight - 1, 0.0, 0.0);
        if (testSamples == TRUE || index == 4) {
          m_floatData[ (2 * j + 1) * inp_width + i ] = filterVertical(&inp[i], m_verFilter[1][index], j + 1, inp_width, inputHeight - 1, 0.0, 0.0);
          break;
        }
      }
    }
  }
  for (j = 0; j < height; j++) {
    for (i = 0; i < inp_width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&m_floatData[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          out[ j * width + 2 * i     ] = filterHorizontal(&m_floatData[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
          break;
        }
      }
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&m_floatData[ j * inp_width], m_horFilter[1][index], i    , inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          out[ j * width + 2 * i + 1 ] = filterHorizontal(&m_floatData[ j * inp_width], m_horFilter[1][index], i + 1, inp_width - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
}

void Conv420to444CrBounds::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j, index;
  int inp_width  = width >> 1;
  int inputHeight = height >> 1;
  
  bool testSamples;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < inp_width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&inp[i], m_verFilter[0][index], j    , inp_width, inputHeight - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          m_i32Data[ (2 * j    ) * inp_width + i ] = filterVertical(&inp[i], m_verFilter[0][index], j    , inp_width, inputHeight - 1, minValue, maxValue);
          break;
        }
      }
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&inp[i], m_verFilter[1][index], j    , inp_width, inputHeight - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          m_i32Data[ (2 * j + 1) * inp_width + i ] = filterVertical(&inp[i], m_verFilter[1][index], j + 1, inp_width, inputHeight - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < inp_width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&m_i32Data[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          out[ j * width + 2 * i     ] = filterHorizontal(&m_i32Data[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
          break;
        }
      }
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&m_i32Data[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          
          out[ j * width + 2 * i + 1 ] = filterHorizontal(&m_i32Data[ j * inp_width], m_horFilter[1][index], i + 1, inp_width - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
}

void Conv420to444CrBounds::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j, index;
  int inp_width  = width >> 1;
  int inputHeight = height >> 1;
  
  bool testSamples;

  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < inp_width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&inp[i], m_verFilter[0][index], j    , inp_width, inputHeight - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          m_i32Data[ (2 * j    ) * inp_width + i ] = filterVertical(&inp[i], m_verFilter[0][index], j    , inp_width, inputHeight - 1, minValue, maxValue);
          break;
        }
      }
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeVertical(&inp[i], m_verFilter[1][index], j    , inp_width, inputHeight - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          m_i32Data[ (2 * j + 1) * inp_width + i ] = filterVertical(&inp[i], m_verFilter[1][index], j + 1, inp_width, inputHeight - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < inp_width; i++) {
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&m_i32Data[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          out[ j * width + 2 * i     ] = filterHorizontal(&m_i32Data[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
          break;
        }
      }
      for (index = 0; index < 5; index ++) {
        testSamples = analyzeHorizontal(&m_i32Data[ j * inp_width], m_horFilter[0][index], i    , inp_width - 1, minValue, maxValue);
        if (testSamples == TRUE || index == 4) {
          
          out[ j * width + 2 * i + 1 ] = filterHorizontal(&m_i32Data[ j * inp_width], m_horFilter[1][index], i + 1, inp_width - 1, minValue, maxValue);
          break;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv420to444CrBounds::process ( Frame* out, const Frame *inp)
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
