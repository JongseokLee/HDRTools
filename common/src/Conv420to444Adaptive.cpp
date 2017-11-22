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
 * \file Conv420to444Adaptive.cpp
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
#include "Conv420to444Adaptive.H"
#include "ColorTransformGeneric.H"
#include "ConvFixedToFloat.H"
#include "ConvFloatToFixed.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define TEST_FILTER 0

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

Conv420to444Adaptive::Conv420to444Adaptive(int width, int height, int method, ChromaLocation chromaLocationType[2]) {
  int offset, scale;
  int hPhase[2];
  int vPhase[2];

  // here we allocate the entire image buffers. To save on memory we could just allocate
  // these based on filter length, but this is test code so we don't care for now.
  m_i32Data.resize  (width * height * 2);
  m_floatData.resize(width * height * 2);

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

  m_verFilter[0] = new ScaleFilter(method, 1, 0,      0,     0, &offset, &scale, vPhase[0]); //even
  m_horFilter[0] = new ScaleFilter(method, 1, 2, offset, scale, &offset, &scale, hPhase[0]); //even
  m_verFilter[1] = new ScaleFilter(method, 1, 0,      0,     0, &offset, &scale, vPhase[1]); //odd
  m_horFilter[1] = new ScaleFilter(method, 1, 2, offset, scale, &offset, &scale, hPhase[1]); //odd
}

Conv420to444Adaptive::~Conv420to444Adaptive() {
  delete m_horFilter[0];
  delete m_horFilter[1];
  delete m_verFilter[0];
  delete m_verFilter[1];
}



//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
float Conv420to444Adaptive::filterHorizontal(const float *inp, const ScaleFilter *filter, int pos_x, int width, float minValue, float maxValue) {
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


int Conv420to444Adaptive::filterHorizontal(const uint16 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) * filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv420to444Adaptive::filterHorizontal(const int32 *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_x + i - filter->m_positionOffset, 0, width)];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) * filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}


int Conv420to444Adaptive::filterHorizontal(const imgpel *inp, const ScaleFilter *filter, int pos_x, int width, int minValue, int maxValue) {
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

float Conv420to444Adaptive::filterVertical(const float *inp, const ScaleFilter *filter, int pos_y, int width, int height, float minValue, float maxValue) {
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


int Conv420to444Adaptive::filterVertical(const int32 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
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

int Conv420to444Adaptive::filterVertical(const uint16 *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    // printf("%d (%d) ", (iClip(pos_y + i - filter->m_positionOffset, 0, height) ), filter->m_i32Filter[i]);

    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
   // printf(" (%d)\n", pos_y);
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) >> filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

int Conv420to444Adaptive::filterVertical(const imgpel *inp, const ScaleFilter *filter, int pos_y, int width, int height, int minValue, int maxValue) {
  int i;
  int value = 0;
  for (i = 0; i < filter->m_numberOfTaps; i++) {
    value += filter->m_i32Filter[i] * inp[iClip(pos_y + i - filter->m_positionOffset, 0, height) * width];
  }
  
  if (filter->m_clip == TRUE)
    return iClip((value + filter->m_i32Offset) * filter->m_i32Shift, minValue, maxValue);
  else
    return (value + filter->m_i32Offset) >> filter->m_i32Shift;
}

void Conv420to444Adaptive::filter(float *out, const float *inp, const float *inpY, int conversionMatrix, int width, int height, float minValue, float maxValue)
{
  int i, j;
  int inputWidth  = width >> 1;
  int inputHeight = height >> 1;

  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < inputWidth; i++) {
      m_floatData[ (2 * j    ) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[0], j    , inputWidth, inputHeight - 1, 0.0, 0.0);
      m_floatData[ (2 * j + 1) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[1], j + 1, inputWidth, inputHeight - 1, 0.0, 0.0);
    }
  }

  for (j = 0; j < height; j++) {
    for (i = 0; i < inputWidth; i++) {
      out[ j * width + 2 * i     ] = filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[0], i    , inputWidth - 1, minValue, maxValue);
      out[ j * width + 2 * i + 1 ] = filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[1], i + 1, inputWidth - 1, minValue, maxValue);
    }
  }
}


void Conv420to444Adaptive::filter(float *out, const float *inp, const float *inpY, int conversionMatrix, int width, int height, float minValue, float maxValue, int component)
{
  int i, j;
  int inputWidth  = width >> 1;
  int inputHeight = height >> 1;
  
  // it seems better if I convert the data from fixed precision to float precision first. 
  // For now, lets keep the current structure, but unfortunately, this would be very suboptimal code :(.
  
  const double *transformY  = FWD_TRANSFORM[conversionMatrix][Y_COMP];
  const double *transformCb = FWD_TRANSFORM[conversionMatrix][Cb_COMP];
  const double *transformCr = FWD_TRANSFORM[conversionMatrix][Cr_COMP];
  double yDouble = 0.0;
  
  double minCompFloat = 0.0;
  double maxCompFloat = 0.0;
  
  if (component == U_COMP) {
    double denom = -transformY[0]/transformCb[0];
    
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < inputWidth; i++) {
        
        yDouble = inpY[ (2 * j    ) * width + 2 * i ]; 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[2], 0.0) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        maxCompFloat = (dMin(yDouble, transformY[2]) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        
                
        m_floatData[ (2 * j    ) * inputWidth + i ] = (float) dClip(filterVertical(&inp[i], m_verFilter[0], j    , inputWidth, inputHeight - 1, minValue, maxValue), minCompFloat, maxCompFloat);
        
        yDouble = inpY[ (2 * j + 1 ) * width + 2 * i ]; 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[2], 0.0) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        maxCompFloat = (dMin(yDouble, transformY[2]) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        m_floatData[ (2 * j + 1) * inputWidth + i ] = (float) dClip(filterVertical(&inp[i], m_verFilter[1], j + 1, inputWidth, inputHeight - 1, minValue, maxValue), minCompFloat, maxCompFloat);
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < inputWidth; i++) {
      
        yDouble = inpY[j * width + 2 * i]; 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[2], 0.0) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        maxCompFloat = (dMin(yDouble, transformY[2]) - (transformY[2] * yDouble)) / (transformY[2] * denom);

        out[ j * width + 2 * i     ] = (float) dClip(filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[0], i    , inputWidth - 1, minValue, maxValue), minCompFloat, maxCompFloat);
        yDouble = inpY[j * width + 2 * i + 1]; 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[2], 0.0) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        maxCompFloat = (dMin(yDouble, transformY[2]) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        out[ j * width + 2 * i + 1 ] = (float) dClip(filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[1], i + 1, inputWidth - 1, minValue, maxValue), minCompFloat, maxCompFloat);
      }
    }
  }
  else {
    double denom = -transformY[2]/transformCr[2];
    
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < inputWidth; i++) {
        
        yDouble = inpY[ (2 * j    ) * width + 2 * i ]; 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[0], 0.0) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        maxCompFloat = (dMin(yDouble, transformY[0]) - (transformY[0] * yDouble)) / (transformY[0] * denom);
                
        
        m_floatData[ (2 * j    ) * inputWidth + i ] = (float) dClip(filterVertical(&inp[i], m_verFilter[0], j    , inputWidth, inputHeight - 1, minValue, maxValue), minCompFloat, maxCompFloat);
        
        yDouble = inpY[ (2 * j + 1 ) * width + 2 * i ]; 
        minCompFloat = (dMax(yDouble - 1.0 + transformY[0], 0.0) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        maxCompFloat = (dMin(yDouble, transformY[0]) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        
        m_floatData[ (2 * j + 1) * inputWidth + i ] = (float) dClip(filterVertical(&inp[i], m_verFilter[1], j + 1, inputWidth, inputHeight - 1, minValue, maxValue), minCompFloat, maxCompFloat);
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < inputWidth; i++) {
      
        yDouble = inpY[ j * width + 2 * i     ]; 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[0], 0.0) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        maxCompFloat = (dMin(yDouble, transformY[0]) - (transformY[0] * yDouble)) / (transformY[0] * denom);

        out[ j * width + 2 * i     ] = (float) dClip(filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[0], i    , inputWidth - 1, minValue, maxValue), minCompFloat, maxCompFloat);
        yDouble = inpY[j * width + 2 * i + 1]; 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[0], 0.0) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        maxCompFloat = (dMin(yDouble, transformY[0]) - (transformY[0] * yDouble)) / (transformY[0] * denom);

        out[ j * width + 2 * i + 1 ] = (float) dClip(filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[1], i + 1, inputWidth - 1, minValue, maxValue), minCompFloat, maxCompFloat);
      }
    }
  }
}

void Conv420to444Adaptive::filter(float *out, const float *inp, const float *inpY, const float *inpCb, int conversionMatrix, int width, int height, float minValue, float maxValue)
{
  int i, j;
  int inputWidth  = width >> 1;
  int inputHeight = height >> 1;
  
  //minCr=(max(0,Y_C^'-(w_YG+w_YB*(2*(1-w_YB ) 〖*Cb〗_C+Y_C^' )))-w_YR*Y_C^')/(2*(1-w_YR )*w_YR )
  //maxCr==(min(w_YR,Y_C^'-w_YB*(2*(1-w_YB ) 〖*Cb〗_C+Y_C^' ))-w_YR*Y_C^')/(2*(1-w_YR )*w_YR )
  const double wYR = FWD_TRANSFORM[conversionMatrix][Y_COMP][R_COMP];
  const double wYG = FWD_TRANSFORM[conversionMatrix][Y_COMP][G_COMP];
  const double wYB = FWD_TRANSFORM[conversionMatrix][Y_COMP][B_COMP];
  double yDouble = 0.0;
  double cbDouble = 0.0;
  
  double minCompDouble = 0.0;
  double maxCompDouble = 0.0;
  
  double denom = 2 * (1 - wYR) * wYR;
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < inputWidth; i++) {
      
      yDouble  = (double) inpY[ (2 * j    ) * width + 2 * i ]; 
      cbDouble = (double) inpCb[ (2 * j    ) * inputWidth + i  ]; 
      
      minCompDouble = (dMax(0, yDouble - (wYG + wYB * (2 * (1 - wYB) * cbDouble + yDouble))) - wYR * yDouble) / denom;
      maxCompDouble = (dMin( wYR, yDouble - wYB * (2 * (1 - wYB) * cbDouble + yDouble)) -wYR * yDouble)/ denom;
            
      m_floatData[ (2 * j    ) * inputWidth + i ] = (float) dClip(filterVertical(&inp[i], m_verFilter[0], j    , inputWidth, inputHeight - 1, minValue, maxValue), minCompDouble, maxCompDouble);
      
      yDouble  = (double) inpY[ (2 * j + 1 ) * width + 2 * i ]; 
      cbDouble = (double) inpCb[ (2 * j + 1) * inputWidth + i  ]; 

      minCompDouble = (dMax(0, yDouble - (wYG + wYB * (2 * (1 - wYB) * cbDouble + yDouble))) - wYR * yDouble) / denom;
      maxCompDouble = (dMin( wYR, yDouble - wYB * (2 * (1 - wYB) * cbDouble + yDouble)) -wYR * yDouble)/ denom;
      
      m_floatData[ (2 * j + 1) * inputWidth + i ] = (float) dClip(filterVertical(&inp[i], m_verFilter[1], j + 1, inputWidth, inputHeight - 1, minValue, maxValue), minCompDouble, maxCompDouble);
    }
  }
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < inputWidth; i++) {
      yDouble  = (double) inpY [ j * width + 2 * i ]; 
      cbDouble = (double) inpCb[ j * width + 2 * i ]; 
      
      minCompDouble = (dMax(0, yDouble - (wYG + wYB * (2 * (1 - wYB) * cbDouble + yDouble))) - wYR * yDouble) / denom;
      maxCompDouble = (dMin( wYR, yDouble - wYB * (2 * (1 - wYB) * cbDouble + yDouble)) -wYR * yDouble)/ denom;
      out[ j * width + 2 * i     ] = (float) dClip(filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[0], i    , inputWidth - 1, minValue, maxValue), minCompDouble, maxCompDouble);
      
      yDouble  = (double) inpY [ j * width + 2 * i + 1]; 
      cbDouble = (double) inpCb[ j * width + 2 * i + 1]; 
      
      minCompDouble = (dMax(0, yDouble - (wYG + wYB * (2 * (1 - wYB) * cbDouble + yDouble))) - wYR * yDouble) / denom;
      maxCompDouble = (dMin( wYR, yDouble - wYB * (2 * (1 - wYB) * cbDouble + yDouble)) -wYR * yDouble)/ denom;
      out[ j * width + 2 * i + 1 ] = (float) dClip(filterHorizontal(&m_floatData[ j * inputWidth], m_horFilter[1], i + 1, inputWidth - 1, minValue, maxValue), minCompDouble, maxCompDouble);
    }
  }
}



void Conv420to444Adaptive::filter(uint16 *out, const uint16 *inp, const uint16 *inpY, int conversionMatrix, int width, int height, int minValue, int maxValue, int component)
{
  int i, j;
  int inputWidth  = width >> 1;
  int inputHeight = height >> 1;

  // it seems better if I convert the data from fixed precision to float precision first. 
  // For now, lets keep the current structure, but unfortunately, this would be very suboptimal code :(.
  
  const double *transformY  = FWD_TRANSFORM[conversionMatrix][Y_COMP];
  const double *transformCb = FWD_TRANSFORM[conversionMatrix][Cb_COMP];
  const double *transformCr = FWD_TRANSFORM[conversionMatrix][Cr_COMP];
  double yDouble = 0.0;
  double compFloat = 0.0;
  
  double minCompFloat = 0.0;
  double maxCompFloat = 0.0;

  if (component == U_COMP) {
    double denom = -transformY[0]/transformCb[0];
    
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < inputWidth; i++) {
        
        yDouble = ConvFixedToFloat::convertUi16CompValue(inpY[ (2 * j    ) * width + 2 * i ], m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[Y_COMP], Y_COMP); 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[2], 0.0) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        maxCompFloat = (dMin(yDouble, transformY[2]) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        
        
        
        m_i32Data[ (2 * j    ) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[0], j    , inputWidth, inputHeight - 1, minValue, maxValue);
        compFloat = ConvFixedToFloat::convertUi16CompValue((m_i32Data[ (2 * j    ) * inputWidth + i ] + 128) >> 8, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component); 
        if (compFloat < minCompFloat || compFloat > maxCompFloat) {
          //printf("value Cb %d %7.3f %7.3f %7.3f  %7.3f\n", inpY[ (2 * j    ) * width + 2 * i ], yFloat, minCompFloat, maxCompFloat, compFloat);
          if (compFloat < minCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j    ) * inputWidth + i ], ConvFloatToFixed::convertUi16Value(minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j    ) * inputWidth + i ] = ConvFloatToFixed::convertUi16Value((float) minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
          else if (compFloat > maxCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j    ) * inputWidth + i ], ConvFloatToFixed::convertUi16Value(maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j    ) * inputWidth + i ] = ConvFloatToFixed::convertUi16Value((float) maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
        }
        
        yDouble = ConvFixedToFloat::convertUi16CompValue(inpY[ (2 * j + 1 ) * width + 2 * i ], m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[Y_COMP], Y_COMP); 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[2], 0.0) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        maxCompFloat = (dMin(yDouble, transformY[2]) - (transformY[2] * yDouble)) / (transformY[2] * denom);
        m_i32Data[ (2 * j + 1) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[1], j + 1, inputWidth, inputHeight - 1, minValue, maxValue);
        compFloat = ConvFixedToFloat::convertUi16CompValue((m_i32Data[ (2 * j + 1) * inputWidth + i ] + 128) >> 8, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component); 
        if (compFloat < minCompFloat || compFloat > maxCompFloat) {
         // printf("value Cb %d %7.3f %7.3f %7.3f %7.3f\n", inpY[ (2 * j + 1 ) * width + 2 * i ], yFloat, minCompFloat, maxCompFloat, compFloat);
          if (compFloat < minCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j + 1) * inputWidth + i ], ConvFloatToFixed::convertUi16Value(minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j + 1) * inputWidth + i ]  = ConvFloatToFixed::convertUi16Value((float) minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
          else if (compFloat > maxCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j + 1) * inputWidth + i ] , ConvFloatToFixed::convertUi16Value(maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j + 1) * inputWidth + i ] = ConvFloatToFixed::convertUi16Value((float) maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
        }
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < inputWidth; i++) {
        out[ j * width + 2 * i     ] = filterHorizontal(&m_i32Data[ j * inputWidth], m_horFilter[0], i    , inputWidth - 1, minValue, maxValue);
        out[ j * width + 2 * i + 1 ] = filterHorizontal(&m_i32Data[ j * inputWidth], m_horFilter[1], i + 1, inputWidth - 1, minValue, maxValue);
      }
    }
  }
  else {
    double denom = -transformY[2]/transformCr[2];
    
    for (j = 0; j < inputHeight; j++) {
      for (i = 0; i < inputWidth; i++) {
        
        yDouble = ConvFixedToFloat::convertUi16CompValue(inpY[ (2 * j    ) * width + 2 * i ], m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[Y_COMP], Y_COMP); 
        
        minCompFloat = (dMax(yDouble - 1.0 + transformY[0], 0.0) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        maxCompFloat = (dMin(yDouble, transformY[0]) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        
        //printf("value %d %7.3f %7.3f %7.3f \n", inpY[ (2 * j    ) * width + 2 * i ], yFloat, minCompFloat, maxCompFloat);
        
        
        m_i32Data[ (2 * j    ) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[0], j    , inputWidth, inputHeight - 1, minValue, maxValue);
        compFloat = ConvFixedToFloat::convertUi16CompValue((m_i32Data[ (2 * j    ) * inputWidth + i ] + 128) >> 8, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component); 
        if (compFloat < minCompFloat || compFloat > maxCompFloat) {
          //printf("value Cr %d %7.3f %7.3f %7.3f  %7.3f\n", inpY[ (2 * j    ) * width + 2 * i ], yFloat, minCompFloat, maxCompFloat, compFloat);
          if (compFloat < minCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j    ) * inputWidth + i ], ConvFloatToFixed::convertUi16Value(minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j    ) * inputWidth + i ] = ConvFloatToFixed::convertUi16Value((float) minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
          else if (compFloat > maxCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j    ) * inputWidth + i ], ConvFloatToFixed::convertUi16Value(maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j    ) * inputWidth + i ] = ConvFloatToFixed::convertUi16Value((float) maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
        }
        
        yDouble = ConvFixedToFloat::convertUi16CompValue(inpY[ (2 * j + 1 ) * width + 2 * i ], m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[Y_COMP], Y_COMP); 
        minCompFloat = (dMax(yDouble - 1.0 + transformY[0], 0.0) - (transformY[0] * yDouble)) / (transformY[0] * denom);
        maxCompFloat = (dMin(yDouble, transformY[0]) - (transformY[0] * yDouble)) / (transformY[0] * denom);

        m_i32Data[ (2 * j + 1) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[1], j + 1, inputWidth, inputHeight - 1, minValue, maxValue);
        compFloat = ConvFixedToFloat::convertUi16CompValue((m_i32Data[ (2 * j + 1) * inputWidth + i ] + 128) >> 8, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component); 
        if (compFloat < minCompFloat || compFloat > maxCompFloat) {
          //printf("value Cr %d %7.3f %7.3f %7.3f %7.3f\n", inpY[ (2 * j + 1 ) * width + 2 * i ], yFloat, minCompFloat, maxCompFloat, compFloat);
          if (compFloat < minCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j + 1) * inputWidth + i ], ConvFloatToFixed::convertUi16Value(minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j + 1) * inputWidth + i ]  = ConvFloatToFixed::convertUi16Value((float) minCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
          else if (compFloat > maxCompFloat) {
            //printf("value %d %d\n", m_i32Data[ (2 * j + 1) * inputWidth + i ] , ConvFloatToFixed::convertUi16Value(maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256);
            m_i32Data[ (2 * j + 1) * inputWidth + i ] = ConvFloatToFixed::convertUi16Value((float) maxCompFloat, m_format->m_sampleRange, m_format->m_colorSpace, m_format->m_bitDepthComp[component], component) * 256;
          }
        }
      }
    }
    
    for (j = 0; j < height; j++) {
      for (i = 0; i < inputWidth; i++) {
        out[ j * width + 2 * i     ] = filterHorizontal(&m_i32Data[ j * inputWidth], m_horFilter[0], i    , inputWidth - 1, minValue, maxValue);
        out[ j * width + 2 * i + 1 ] = filterHorizontal(&m_i32Data[ j * inputWidth], m_horFilter[1], i + 1, inputWidth - 1, minValue, maxValue);
      }
    }
  }
}

void Conv420to444Adaptive::filter(imgpel *out, const imgpel *inp, const imgpel *inpY, int conversionMatrix, int width, int height, int minValue, int maxValue)
{
  int i, j;
  int inputWidth  = width >> 1;
  int inputHeight = height >> 1;
  
  for (j = 0; j < inputHeight; j++) {
    for (i = 0; i < inputWidth; i++) {
      m_i32Data[ (2 * j    ) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[0], j    , inputWidth, inputHeight - 1, minValue, maxValue);
      m_i32Data[ (2 * j + 1) * inputWidth + i ] = filterVertical(&inp[i], m_verFilter[1], j + 1, inputWidth, inputHeight - 1, minValue, maxValue);
    }
  }
  
  for (j = 0; j < height; j++) {
    for (i = 0; i < inputWidth; i++) {
      out[ j * width + 2 * i     ] = filterHorizontal(&m_i32Data[ j * inputWidth], m_horFilter[0], i    , inputWidth - 1, minValue, maxValue);
      out[ j * width + 2 * i + 1 ] = filterHorizontal(&m_i32Data[ j * inputWidth], m_horFilter[1], i + 1, inputWidth - 1, minValue, maxValue);
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void Conv420to444Adaptive::process ( Frame* out, const Frame *inp)
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
  
  // Since I am lazy, lets copy the format parameters here and not pass them internally.
  m_format = (FrameFormat *) &inp->m_format;
  
  int    conversionMatrix = CTF_IDENTITY;
  if (inp->m_colorPrimaries == CP_709) {
    conversionMatrix = CTF_RGB709_2_YUV709;
  }
  else if (inp->m_colorPrimaries == CP_P3D65) {
    conversionMatrix = CTF_RGBP3D65_2_YUVP3D65;
  }
  else if (inp->m_colorPrimaries == CP_2020) {
    conversionMatrix = CTF_RGB2020_2_YUV2020;
  }
  else if (inp->m_colorPrimaries == CP_601) {
    conversionMatrix = CTF_RGB601_2_YUV601;
  }
  
  
  if (out->m_isFloat == TRUE) {    // floating point data
    memcpy(out->m_floatComp[Y_COMP], inp->m_floatComp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(float));
#if 0
    for (c = U_COMP; c <= V_COMP; c++) {
      filter(out->m_floatComp[c], inp->m_floatComp[c], inp->m_floatComp[Y_COMP], conversionMatrix, out->m_width[c], out->m_height[c], (float) out->m_minPelValue[c], (float) out->m_maxPelValue[c], c );
    }
#else
    filter(out->m_floatComp[U_COMP], inp->m_floatComp[U_COMP], inp->m_floatComp[Y_COMP],                           conversionMatrix, out->m_width[U_COMP], out->m_height[U_COMP], (float) out->m_minPelValue[U_COMP], (float) out->m_maxPelValue[U_COMP], U_COMP );
    filter(out->m_floatComp[V_COMP], inp->m_floatComp[V_COMP], out->m_floatComp[Y_COMP], out->m_floatComp[U_COMP], conversionMatrix, out->m_width[V_COMP], out->m_height[V_COMP], (float) out->m_minPelValue[V_COMP], (float) out->m_maxPelValue[V_COMP]);
#endif
  }
  else if (out->m_bitDepth == 8) {   // 8 bit data
    memcpy(out->m_comp[Y_COMP], inp->m_comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(imgpel));
    for (c = U_COMP; c <= V_COMP; c++) {
      filter(out->m_comp[c], inp->m_comp[c], inp->m_comp[Y_COMP], conversionMatrix, out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c]);
    }
  }
  else { // 16 bit data
    memcpy(out->m_ui16Comp[Y_COMP], inp->m_ui16Comp[Y_COMP], (int) out->m_compSize[Y_COMP] * sizeof(uint16));
    for (c = U_COMP; c <= V_COMP; c++) {
      filter(out->m_ui16Comp[c], inp->m_ui16Comp[c], inp->m_ui16Comp[Y_COMP], conversionMatrix, out->m_width[c], out->m_height[c], out->m_minPelValue[c], out->m_maxPelValue[c], c);
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
