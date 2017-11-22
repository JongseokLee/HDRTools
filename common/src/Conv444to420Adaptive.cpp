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
 * \file Conv444to420Adaptive.cpp
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
#include "Conv444to420Adaptive.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

Conv444to420Adaptive::Conv444to420Adaptive(int width, int height, int method, ChromaLocation chromaLocationType[2], int useMinMax) {
  int offset,     scale;
  int upOffset,   upScale;
  int downOffset, downScale;
  int hPhase, vPhase;
  int hPhaseUp[2];
  int vPhaseUp[2];

  // here we allocate the entire image buffers. To save on memory we could just allocate
  // these based on filter length, but this is test code so we don't care for now.
  m_i32Data.resize  ( (width >> 1) * height );
  m_floatData.resize( (width >> 1) * height );
  
  // Currently we only support progressive formats, and thus ignore the bottom chroma location type
  
  switch (chromaLocationType[FP_FRAME]) {
    case CL_FIVE:
      hPhaseUp[0] = 1;
      hPhaseUp[1] = 3;
      vPhaseUp[0] = 2;
      vPhaseUp[1] = 0;
      hPhase = 1;
      vPhase = 0;
      break;
    case CL_FOUR:
      hPhaseUp[0] = 0;
      hPhaseUp[1] = 2;
      vPhaseUp[0] = 2;
      vPhaseUp[1] = 0;
      hPhase = 0;  
      vPhase = 0;
      break;
    case CL_THREE:
      hPhaseUp[0] = 1;
      hPhaseUp[1] = 3;
      vPhaseUp[0] = 0;
      vPhaseUp[1] = 2;
      hPhase = 1;
      vPhase = 0;
      break;
    case CL_TWO:
      hPhaseUp[0] = 0;
      hPhaseUp[1] = 2;
      vPhaseUp[0] = 0;
      vPhaseUp[1] = 2;
      hPhase = 0;
      vPhase = 0;
      break;
    case CL_ONE:
      hPhaseUp[0] = 1;
      hPhaseUp[1] = 3;
      vPhaseUp[0] = 1;
      vPhaseUp[1] = 3;
      hPhase = 1;
      vPhase = 1;
      break;
    case CL_ZERO:
    default:
      hPhaseUp[0] = 0;
      hPhaseUp[1] = 2;
      vPhaseUp[0] = 1;
      vPhaseUp[1] = 3;
      hPhase = 0;
      vPhase = 1;
      break;
  }

// Downsampling filters
  for (int index = 0; index < 5; index++) {
    m_horFilterDown[index] = new ScaleFilter(DF_GS - index, 0,  0,      0,     0, &offset, &scale, hPhase);
    m_verFilterDown[index] = new ScaleFilter(DF_GS - index, 0,  2, offset, scale, &downOffset, &downScale, vPhase);

    m_floatDataTemp[index].resize(iMax((width >> 1), (height >> 1)) );
    m_i32DataTemp  [index].resize(iMax((width >> 1), (height >> 1)) );
  }
  
  // Upsampling
  m_verFilterUp[0] = new ScaleFilter(UF_F0, 1, 0,      0,     0, &upOffset, &upScale, vPhaseUp[0]); //even
  m_horFilterUp[0] = new ScaleFilter(UF_F0, 1, 2, offset, scale, &upOffset, &upScale, hPhaseUp[0]); //even
  m_verFilterUp[1] = new ScaleFilter(UF_F0, 1, 0,      0,     0, &upOffset, &upScale, vPhaseUp[1]); //odd
  m_horFilterUp[1] = new ScaleFilter(UF_F0, 1, 2, offset, scale, &upOffset, &upScale, hPhaseUp[1]); //odd
}

Conv444to420Adaptive::~Conv444to420Adaptive() {
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
  for (int index = 0; index < 2; index++) {
    if (m_verFilterUp[index] != NULL) {
      delete m_verFilterUp[index];
      m_verFilterUp[index] = NULL;
    }
    if (m_horFilterUp[index] != NULL) {
      delete m_horFilterUp[index];
      m_horFilterUp[index] = NULL;
    }
  }
}

float Conv444to420Adaptive::filterHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
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

int Conv444to420Adaptive::filterHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
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

int Conv444to420Adaptive::filterHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
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


int Conv444to420Adaptive::filterHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
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

float Conv444to420Adaptive::filterVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
  int i;
  double value = 0.0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += (double) filter->m_floatFilter[i] * (double) inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  //printf("value %7.3f %7.3f %7.3f %7.3f %7.3f %7.3f %d\n", value, filter->m_floatOffset, filter->m_floatScale, minValue, maxValue, fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), minValue, maxValue), filter->m_clip);

  if (filter->m_clip == TRUE)
    return fClip((float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale), -0.5, 0.5);
  else
    return (float) ((value + (double) filter->m_floatOffset) * (double) filter->m_floatScale);
}

int Conv444to420Adaptive::filterVertical(const int32 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
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

int Conv444to420Adaptive::filterVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
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

int Conv444to420Adaptive::filterVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
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
void Conv444to420Adaptive::filter(float *out, const float *inp, int width, int height, float minValue, float maxValue)
{
  int i, j, index;
  int inp_width  = 2 * width;
  int inputHeight = 2 * height;
  double dist;
  double distortion = 0.0;
  double minDistortion = 0.0;
  int   best = 0;
    
    
    // Note that for chroma location type 2, code could be much faster since we would only need to compute one of the upsampling positions
  for (j = 0; j < inputHeight; j++) {
    minDistortion = 1E+37;
    
    for (index = 0; index < 5; index ++) {
      distortion = 0.0;
      
      for (i = 0; i < width; i++) {
        m_floatDataTemp[index][ i ] = filterHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, 0.0, 0.0);
      }
      
      for (i = 0; i < width; i++) {
        //printf("%7.3f %7.3f ", inp[ j * inp_width + 2 * i     ], filterHorizontal(&m_floatDataTemp[index][ 0 ], m_horFilterUp[0], i    , width - 1, -0.5, 0.5) );
        dist = (inp[ j * inp_width + 2 * i     ] - filterHorizontal(&m_floatDataTemp[index][ 0 ], m_horFilterUp[0], i    , width - 1, -0.5, 0.5));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
        //printf("%7.3f %7.3f\n", inp[ j * inp_width + 2 * i + 1], filterHorizontal(&m_floatDataTemp[index][ 0 ], m_horFilterUp[1], i +1   , width - 1, -0.5, 0.5) );
        dist = (inp[ j * inp_width + 2 * i + 1 ] - filterHorizontal(&m_floatDataTemp[index][ 0 ], m_horFilterUp[1], i + 1, width - 1, -0.5, 0.5));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
      }
      //printf("(%d) index %d %7.3f %7.3f\n", i, index, distortion, minDistortion);
      if (distortion < minDistortion) {
        minDistortion = distortion;
        best = index;
      }
    }
    //printf("best horizontal %d\n", best);

    for (i = 0; i < width; i++) {
      m_floatData[ j * width + i ] = m_floatDataTemp[best][ i ] ;
    }
  }
  
  for (i = 0; i < width; i++) {
    minDistortion = 1E+37;
    best = 0;

    for (index = 0; index < 5; index ++) {
      distortion = 0.0;
      for (j = 0; j < height; j++) {
        m_floatDataTemp[index][j] = filterVertical(&m_floatData[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
      }


      for (j = 0; j < height; j++) {
        // We use filterHorizontal since m_floatDataTemp is an 1D array
        dist = (m_floatData[ 2 * j * width + i ] - filterHorizontal(&m_floatDataTemp[index][ 0 ], m_verFilterUp[0], j    , height - 1, -0.5, 0.5));
        //printf("%d %7.3f %7.3f %7.3f ", index, m_floatData[ 2 * j * width + i    ], filterHorizontal(&m_floatDataTemp[index][ 0 ], m_verFilterUp[0], j    , height - 1, -0.5, 0.5), dist );
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
        dist = (m_floatData[ (2 * j + 1) * width + i ] - filterHorizontal(&m_floatDataTemp[index][ 0 ], m_verFilterUp[1], j + 1, height - 1, -0.5, 0.5));
        //printf("%7.3f %7.3f %7.3f\n", m_floatData[(2 * j + 1) * width + i ], filterHorizontal(&m_floatDataTemp[index][ 0 ], m_verFilterUp[1], j +1   , height - 1, -0.5, 0.5), dist );
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
      }
      //printf("(%d) index %d %7.3f %7.3f\n", j, index, distortion, minDistortion);
      if (distortion < minDistortion) {
        minDistortion = distortion;
        best = index;
      }

    }
    //printf("best %d\n", best);
    for (j = 0; j < height; j++) {
      out[ j * width + i ] = m_floatDataTemp[best][ j ] ;
    }
  }
}

void Conv444to420Adaptive::filter(uint16 *out, const uint16 *inp, int width, int height, int minValue, int maxValue)
{
  int i, j, index;
  int inp_width = 2 * width;
  int inputHeight = 2 * height;
  int64 dist;
  int64 distortion = 0;
  int64 minDistortion = 0;
  int   best = 0;
  
  
  for (j = 0; j < inputHeight; j++) {
    minDistortion = LLONG_MAX;
    
    for (index = 0; index < 5; index ++) {
      distortion = 0;
      
      for (i = 0; i < width; i++) {
        m_i32DataTemp[index][ i ] = filterHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, 0, 0);
      }
      
      for (i = 0; i < width; i++) {
        dist = (inp[ j * inp_width + 2 * i     ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_horFilterUp[0], i    , width - 1, minValue, maxValue));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
        dist = (inp[ j * inp_width + 2 * i + 1 ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_horFilterUp[1], i + 1, width - 1, minValue, maxValue));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
      }
      if (distortion < minDistortion) {
        minDistortion = distortion;
        best = index;
      }
    }
    for (i = 0; i < width; i++) {
      m_i32Data[ j * width + i ] = m_i32DataTemp[best][ i ] ;
    }
  }
  
  
  for (i = 0; i < width; i++) {
    minDistortion = LLONG_MAX;
    best = 0;
    
    for (index = 0; index < 5; index ++) {
      distortion = 0;
      for (j = 0; j < height; j++) {
        m_i32DataTemp[index][j] = filterVertical(&m_i32Data[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
      }
      
      for (j = 0; j < height; j++) {
        dist = (m_i32Data[ 2 * j * width + i ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_verFilterUp[0], j    , height - 1, minValue, maxValue));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
        dist = (m_i32Data[ (2 * j + 1) * width + i ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_verFilterUp[1], j + 1, height - 1, minValue, maxValue));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
      }
      if (distortion < minDistortion) {
        minDistortion = distortion;
        best = index;
      }
      
    }
    //printf("best %d\n", best);
    for (j = 0; j < height; j++) {
      out[ j * width + i ] = m_i32DataTemp[best][ j ] ;
    }
  }

}

void Conv444to420Adaptive::filter(imgpel *out, const imgpel *inp, int width, int height, int minValue, int maxValue)
{
  int i, j, index;
  int inp_width = 2 * width;
  int inputHeight = 2 * height;
  int64 dist;
  int64 distortion = 0;
  int64 minDistortion = 0;
  int   best = 0;
  
  for (j = 0; j < inputHeight; j++) {
    minDistortion = LLONG_MAX;
    
    for (index = 0; index < 5; index ++) {
      distortion = 0;
      
      for (i = 0; i < width; i++) {
        m_i32DataTemp[index][ i ] = filterHorizontal(&inp[ j * inp_width], m_horFilterDown[index], 2 * i, inp_width - 1, 0, 0);
      }
      
      for (i = 0; i < width; i++) {
        dist = (inp[ j * inp_width + 2 * i     ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_horFilterUp[0], i    , width - 1, minValue, maxValue));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
        dist = (inp[ j * inp_width + 2 * i + 1 ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_horFilterUp[1], i + 1, width - 1, minValue, maxValue));
        distortion += dist * dist;        
        if (distortion >= minDistortion) {
          break;
        }
      }
      if (distortion < minDistortion) {
        minDistortion = distortion;
        best = index;
      }
    }
    for (i = 0; i < width; i++) {
      m_i32Data[ j * width + i ] = m_i32DataTemp[best][ i ] ;
    }
  }

  for (i = 0; i < width; i++) {
    minDistortion = LLONG_MAX;
    best = 0;
    
    for (index = 0; index < 5; index ++) {
      distortion = 0;
      for (j = 0; j < height; j++) {
        m_i32DataTemp[index][j] = filterVertical(&m_i32Data[i], m_verFilterDown[index], (2 * j), width, inputHeight - 1, minValue, maxValue);
      }
      
      for (j = 0; j < height; j++) {
        dist = (m_i32Data[ 2 * j * width + i ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_verFilterUp[0], j    , height - 1, minValue, maxValue));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
        dist = (m_i32Data[ (2 * j + 1) * width + i ] - filterHorizontal(&m_i32DataTemp[index][ 0 ], m_verFilterUp[1], j + 1, height - 1, minValue, maxValue));
        distortion += dist * dist;
        if (distortion >= minDistortion) {
          break;
        }
      }
      if (distortion < minDistortion) {
        minDistortion = distortion;
        best = index;
      }
      
    }
    //printf("best %d\n", best);
    for (j = 0; j < height; j++) {
      out[ j * width + i ] = m_i32DataTemp[best][ j ] ;
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv444to420Adaptive::process ( Frame* out, const Frame *inp)
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
