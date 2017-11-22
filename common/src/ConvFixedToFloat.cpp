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
 * \file ConvFixedToFloat.cpp
 *
 * \brief
 *    ConvFixedToFloat Class
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
#include "ConvFixedToFloat.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ConvFixedToFloat::ConvFixedToFloat() {
}

ConvFixedToFloat::~ConvFixedToFloat() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
float ConvFixedToFloat::convertValue (const imgpel iComp, double weight, double offset, float minValue, float maxValue) {
    return fClip((float) ((weight * (double) (iComp)) - offset), minValue, maxValue);
}

float ConvFixedToFloat::convertValue (const uint16 iComp, double weight, double offset, float minValue, float maxValue) {

    return fClip((float) ((weight * (double) (iComp)) - offset), minValue, maxValue);
}

float ConvFixedToFloat::convertValue (const uint16 iComp,  int bitScale, double weight, double offset, float minValue, float maxValue) {
    return fClip((float) (weight * (((double) (iComp >> bitScale)) - offset)), minValue, maxValue);
}


float ConvFixedToFloat::convertValue (const imgpel iComp, double weight, const uint16 offset, float minValue, float maxValue) {
    return fClip((float) ((weight * (double) (iComp - offset))), minValue, maxValue);
}

float ConvFixedToFloat::convertValue (const uint16 iComp, double weight, const uint16 offset, float minValue, float maxValue) {
    return fClip((float) ((weight * (double) (iComp - offset))), minValue, maxValue);
}

void ConvFixedToFloat::convertComponent (const imgpel *iComp, float *oComp, int compSize, double weight, double offset, float minValue, float maxValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = fClip((float) ((weight * (double) (*iComp++)) - offset), minValue, maxValue);
  }
}

void ConvFixedToFloat::convertComponent (const uint16 *iComp, float *oComp, int compSize, double weight, double offset, float minValue, float maxValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = fClip((float) ((weight * (double) (*iComp++)) - offset), minValue, maxValue);
  }
}

void ConvFixedToFloat::convertComponent (const imgpel *iComp, float *oComp, int compSize, double weight, const uint16 offset, float minValue, float maxValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = fClip((float) ((weight * (double) (*iComp++ - offset))), minValue, maxValue);
  }
}

void ConvFixedToFloat::convertComponent (const uint16 *iComp, float *oComp, int compSize, double weight, const uint16 offset, float minValue, float maxValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = fClip((float) ((weight * (double) (*iComp++ - offset))), minValue, maxValue);
  }
}

void ConvFixedToFloat::convertComponent (const uint16 *iComp, float *oComp, int bitScale, int compSize, double weight, double offset, float minValue, float maxValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = fClip((float) (weight * (((double) (*iComp++ >> bitScale)) - offset)), minValue, maxValue);
  }
}

float ConvFixedToFloat::convertUi16CompValue(uint16 inpValue, SampleRange sampleRange, ColorSpace colorSpace, int bitDepth, int component) {
  
  if (sampleRange == SR_FULL) {
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default:
          return convertValue (inpValue, 1.0 / ((1 << bitDepth) - 1.0), (const uint16) 0, 0.0f, 1.0f);
        case U_COMP:
        case V_COMP:
          return convertValue (inpValue, 1.0 / ((1 << bitDepth) - 1.0), (const uint16) ((1 << (bitDepth - 1))), -0.5f, 0.5f);
      }
    }
    else {
      convertValue (inpValue, 1.0 / ((1 << bitDepth) - 1.0), 0.0,  0.0f, 1.0f);
    }
  }
  else if (sampleRange == SR_STANDARD) {
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default:
          return convertValue (inpValue, 1.0 / ((1 << (bitDepth-8)) * 219.0), 16.0 / 219.0,  0.0f, 1.0f);
        case U_COMP:
        case V_COMP:
          return convertValue (inpValue, 1.0 / ((1 << (bitDepth-8)) * 224.0), 128.0 / 224.0,  -0.5f, 0.5f);
      }
    }
    else {
      return convertValue (inpValue, 1.0 / ((1 << (bitDepth-8)) * 219.0), 16.0 / 219.0,  0.0f, 1.0f);
    }
  }
  else if (sampleRange == SR_RESTRICTED) {
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default: {
          double weight = 1.0 / (double) ((1 << bitDepth) - (1 << (bitDepth - 7)));
          double offset = (double) (1 << (bitDepth - 8)) * weight;
          return convertValue (inpValue, weight, offset,  0.0f, 1.0f);
        }
        case U_COMP:
        case V_COMP: {
          double weight = 1.0 / (double) ((1 << bitDepth) - (1 << (bitDepth-7)));
          double offset = (double) (1 << (bitDepth-1)) * weight;
          return convertValue (inpValue, weight, offset,  -0.5f, 0.5f);
        }
      }
    }
    else {
      double weight = 1.0 / (double) ((1 << bitDepth) - (1 << (bitDepth - 7)));
      double offset = (double) (1 << (bitDepth - 8)) * weight;
      return convertValue (inpValue, weight, offset,  0.0f, 1.0f);
    }
  }
  else if (sampleRange == SR_SDI_SCALED) {
    // SDI fixed mode for PQ encoded data, scaled by 16. Data were originally graded in "12" bits given the display, but stored
    // in the end in 16 bits. 
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default: {
          double weight = 1.0 / 4060.0;
          double offset = 16.0;
          return convertValue (inpValue, 4, weight, offset,  0.0f, 1.0f);
        }
        case U_COMP:
        case V_COMP: {
          double weight = 1.0 / 4048.0;
          double offset = 2048.0;
          return convertValue (inpValue, 4, weight, offset,  -0.5f, 0.5f);
        }
      }
    }
    else {
      double weight = 1.0 / 4060.0;
      double offset = 16.0;
      return convertValue (inpValue, 4, weight, offset,  0.0f, 1.0f);
    }
  }
  else if (sampleRange == SR_SDI) {
    // SDI specifies values in the range of Floor(1015 * D * N + 4 * D + 0.5), where N is the nonlinear color value from zero to unity, and D = 2^(B-10).
    // Note that we are cheating a bit here since SDI only allows values from 10-16 but we modified the function a bit to also handle bitdepths of 8 and 9 (although likely we will not use those).
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default: {
          double weight = 1.0 / (253.75 * (double) (1 << (bitDepth - 8)));
          double offset = (double) (1 << (bitDepth-8)) * weight;            
          return convertValue (inpValue, weight, offset,  0.0f, 1.0f);
        }
        case U_COMP:
        case V_COMP: {
          double weight = 1.0 / (253.00 * (double) (1 << (bitDepth - 8)));
          double offset = (double) (1 << (bitDepth-1)) * weight;
          return convertValue (inpValue, weight, offset,  -0.5f, 0.5f);
        }
      }
    }
    else {
      double weight = 1.0 / (253.75 * (double) (1 << (bitDepth - 8)));
      double offset = (double) (1 << (bitDepth-8)) * weight;            
      return convertValue (inpValue, weight, offset,  0.0f, 1.0f);
    }
  }
  return 0.0;
}

void ConvFixedToFloat::convertCompData(Frame* out, const Frame *inp) {
  switch (inp->m_sampleRange) {
    case SR_FULL:
      if (inp->m_colorSpace == CM_YCbCr || inp->m_colorSpace == CM_ICtCp) {
        convertComponent (inp->m_comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP])) - 1.0), (const uint16) 0,  0.0f, 1.0f);
        convertComponent (inp->m_comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP])) - 1.0), (const uint16) ((1 << (inp->m_bitDepthComp[U_COMP]-1))), -0.5f, 0.5f);
        convertComponent (inp->m_comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP])) - 1.0), (const uint16) ((1 << (inp->m_bitDepthComp[V_COMP]-1))), -0.5f, 0.5f);
      }
      else {
        convertComponent (inp->m_comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP])) - 1.0), 0.0,  0.0f, 1.0f);
        convertComponent (inp->m_comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP])) - 1.0), 0.0,  0.0f, 1.0f);
        convertComponent (inp->m_comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP])) - 1.0), 0.0,  0.0f, 1.0f);
      }
      break;
      // SR_RESTRICTED, SR_SDI_SCALED, and SR_SDI not supported for 8 bit data, so lets keep those the same as SR_STANDARD range
    case SR_STANDARD:
    default:
      if (inp->m_colorSpace == CM_YCbCr || inp->m_colorSpace == CM_ICtCp) {
        convertComponent (inp->m_comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP]-8)) * 219.0), (const uint16) (1 << (inp->m_bitDepthComp[Y_COMP]-4)),  0.0f, 1.0f);
        convertComponent (inp->m_comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP]-8)) * 224.0), (const uint16) (1 << (inp->m_bitDepthComp[U_COMP]-1)), -0.5f, 0.5f);
        convertComponent (inp->m_comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP]-8)) * 224.0), (const uint16) (1 << (inp->m_bitDepthComp[V_COMP]-1)), -0.5f, 0.5f);

      }
      else {
        convertComponent (inp->m_comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP]-8)) * 219.0),  16.0 / 219.0,  0.0f, 1.0f);
        convertComponent (inp->m_comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP]-8)) * 219.0),  16.0 / 219.0,  0.0f, 1.0f);
        convertComponent (inp->m_comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP]-8)) * 219.0),  16.0 / 219.0,  0.0f, 1.0f);
      }
      break;
  }
}

void ConvFixedToFloat::convertUi16CompData(Frame* out, const Frame *inp) {
  double weight = 1.0;
  double offset = 0.0;
  
  switch (inp->m_sampleRange) {
    case SR_FULL:
      if (inp->m_colorSpace == CM_YCbCr || inp->m_colorSpace == CM_ICtCp) {
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP])) - 1.0), (const uint16) 0,  0.0f, 1.0f);
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP])) - 1.0), (const uint16) ((1 << (inp->m_bitDepthComp[U_COMP]-1))), -0.5f, 0.5f);
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP])) - 1.0), (const uint16) ((1 << (inp->m_bitDepthComp[V_COMP]-1))), -0.5f, 0.5f);
      }
      else {
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP])) - 1.0), 0.0,  0.0f, 1.0f);
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP])) - 1.0), 0.0,  0.0f, 1.0f);
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP])) - 1.0), 0.0,  0.0f, 1.0f);
      }
      break;
    case SR_STANDARD:
    default:
      if (inp->m_colorSpace == CM_YCbCr || inp->m_colorSpace == CM_ICtCp) {
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP]-8)) * 219.0), (const uint16) (1 << (inp->m_bitDepthComp[Y_COMP]-4)),  0.0f, 1.0f);
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP]-8)) * 224.0), (const uint16) (1 << (inp->m_bitDepthComp[U_COMP]-1)), -0.5f, 0.5f);
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP]-8)) * 224.0), (const uint16) (1 << (inp->m_bitDepthComp[V_COMP]-1)), -0.5f, 0.5f);
      }
      else {
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[Y_COMP]-8)) * 219.0),  16.0 / 219.0,  0.0f, 1.0f);
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[U_COMP]-8)) * 219.0),  16.0 / 219.0,  0.0f, 1.0f);
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], 1.0 / ((1 << (inp->m_bitDepthComp[V_COMP]-8)) * 219.0),  16.0 / 219.0,  0.0f, 1.0f);
      }
      break;
    case SR_RESTRICTED:
      // implemented by considering min = 1 << (bitdepth - 8) and max = (1 << bitdepth) - min - 1 and range = max - min + 1 =  (1 << bitdepth) - (1 << (bitdepth - 7))
      if (inp->m_colorSpace == CM_YCbCr || inp->m_colorSpace == CM_ICtCp) {
        weight = 1.0 / (double) ((1 << inp->m_bitDepthComp[Y_COMP]) - (1 << (inp->m_bitDepthComp[Y_COMP]-7)));
        offset = (double) (1 << (inp->m_bitDepthComp[Y_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], weight,  offset,  0.0f, 1.0f);
        weight = 1.0 / (double) ((1 << inp->m_bitDepthComp[U_COMP]) - (1 << (inp->m_bitDepthComp[U_COMP]-7)));
        offset = (double) (1 << (inp->m_bitDepthComp[U_COMP]-1)) * weight;
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], weight,  offset, -0.5f, 0.5f);
        weight = 1.0 / (double) ((1 << inp->m_bitDepthComp[V_COMP]) - (1 << (inp->m_bitDepthComp[V_COMP]-7)));
        offset = (double) (1 << (inp->m_bitDepthComp[V_COMP]-1)) * weight;
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], weight,  offset, -0.5f, 0.5f);
      }
      else {
        weight = 1.0 / (double) ((1 << inp->m_bitDepthComp[Y_COMP]) - (1 << (inp->m_bitDepthComp[Y_COMP]-7)));
        offset = (double) (1 << (inp->m_bitDepthComp[Y_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], weight,  offset, 0.0f, 1.0f);
        weight = 1.0 / (double) ((1 << inp->m_bitDepthComp[U_COMP]) - (1 << (inp->m_bitDepthComp[U_COMP]-7)));
        offset = (double) (1 << (inp->m_bitDepthComp[U_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], weight,  offset, 0.0f, 1.0f);
        weight = 1.0 / (double) ((1 << inp->m_bitDepthComp[V_COMP]) - (1 << (inp->m_bitDepthComp[V_COMP]-7)));
        offset = (double) (1 << (inp->m_bitDepthComp[V_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], weight,  offset, 0.0f, 1.0f);
      }
      break;
    case SR_SDI_SCALED:
      // SDI fixed mode for PQ encoded data, scaled by 16. Data were originally graded in "12" bits given the display, but stored
      // in the end in 16 bits. 
      if (inp->m_colorSpace == CM_YCbCr || inp->m_colorSpace == CM_ICtCp) {
        weight = 1.0 / 4060.0;
        offset = 16.0;
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], 4, inp->m_compSize[Y_COMP], weight,  offset,  0.0f, 1.0f);
        weight = 1.0 / 4048.0;
        offset = 2048.0;
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], 4, inp->m_compSize[U_COMP], weight,  offset, -0.5f, 0.5f);
        weight = 1.0 / 4048.0;
        offset = 2048.0;
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], 4, inp->m_compSize[V_COMP], weight,  offset, -0.5f, 0.5f);
      }
      else {
        weight = 1.0 / 4060.0;
        offset = 16.0;
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], 4, inp->m_compSize[Y_COMP], weight,  offset, 0.0f, 1.0f);
        weight = 1.0 / 4060.0;
        offset = 16.0;
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], 4, inp->m_compSize[U_COMP], weight,  offset, 0.0f, 1.0f);
        weight = 1.0 / 4060.0;
        offset = 16.0;
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], 4, inp->m_compSize[V_COMP], weight,  offset, 0.0f, 1.0f);
      }
      break;
    case SR_SDI:
      // SDI specifies values in the range of Floor(1015 * D * N + 4 * D + 0.5), where N is the nonlinear color value from zero to unity, and D = 2^(B-10).
      // Note that we are cheating a bit here since SDI only allows values from 10-16 but we modified the function a bit to also handle bitdepths of 8 and 9 (although likely we will not use those).
      if (inp->m_colorSpace == CM_YCbCr || inp->m_colorSpace == CM_ICtCp) {
        weight = 1.0 / (253.75 * (double) (1 << (inp->m_bitDepthComp[Y_COMP] - 8)));
        offset = (double) (1 << (inp->m_bitDepthComp[Y_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], weight,  offset,  0.0f, 1.0f);
        weight = 1.0 / (253.00 * (double) (1 << (inp->m_bitDepthComp[U_COMP] - 8)));
        offset = (float) (1 << (inp->m_bitDepthComp[U_COMP]-1)) * weight;
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], weight,  offset, -0.5f, 0.5f);
        weight = 1.0 / (253.00 * (double) (1 << (inp->m_bitDepthComp[V_COMP] - 8)));
        offset = (float) (1 << (inp->m_bitDepthComp[V_COMP]-1)) * weight;
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], weight,  offset, -0.5f, 0.5f);
      }
      else {
        weight = 1.0 / (253.75 * (double) (1 << (inp->m_bitDepthComp[Y_COMP] - 8)));
        offset = (double) (1 << (inp->m_bitDepthComp[Y_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[Y_COMP], out->m_floatComp[Y_COMP], inp->m_compSize[Y_COMP], weight,  offset, 0.0f, 1.0f);
        weight = 1.0 / (253.75 * (double) (1 << (inp->m_bitDepthComp[U_COMP] - 8)));
        offset = (double) (1 << (inp->m_bitDepthComp[U_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[U_COMP], out->m_floatComp[U_COMP], inp->m_compSize[U_COMP], weight,  offset, 0.0f, 1.0f);
        weight = 1.0 / (253.75 * (double) (1 << (inp->m_bitDepthComp[V_COMP] - 8)));
        offset = (double) (1 << (inp->m_bitDepthComp[V_COMP]-8)) * weight;
        convertComponent (inp->m_ui16Comp[V_COMP], out->m_floatComp[V_COMP], inp->m_compSize[V_COMP], weight,  offset, 0.0f, 1.0f);
      }
      break;
  }
}
//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void ConvFixedToFloat::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
    
  if (inp->m_bitDepth == 8)
    convertCompData(out, inp);
  else
    convertUi16CompData(out, inp);
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
