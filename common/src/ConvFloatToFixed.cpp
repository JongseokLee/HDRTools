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
 * \file ConvFloatToFixed.cpp
 *
 * \brief
 *    ConvFloatToFixed Class
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
#include "ConvFloatToFixed.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ConvFloatToFixed::ConvFloatToFixed() {
  m_lowbias = FALSE;
}

ConvFloatToFixed::~ConvFloatToFixed() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

uint16 ConvFloatToFixed::convertValue (const float iComp, double weight, double offset, int maxPelValue) {
    return (uint16) fClip(fRound((float) (weight * (double) iComp + offset)), 0.0f, (float) maxPelValue);
}

uint16 ConvFloatToFixed::convertValue (const float iComp, int bitScale, double weight, double offset, int maxPelValue) {
    return ((uint16) fClip(fRound((float) (weight * (double) iComp + offset)), 0.0f, (float) maxPelValue)) << bitScale;
}


void ConvFloatToFixed::convertComponent (const float *iComp, imgpel *oComp, int compSize, double weight, double offset, int maxPelValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = (imgpel) fClip(fRound((float) (weight * (double) *iComp++ + offset)), 0.0f, (float) maxPelValue);
  }
}

void ConvFloatToFixed::convertComponent (const float *iComp, uint16 *oComp, int compSize, double weight, double offset, int maxPelValue) {

//KW-KYH Float to 10bits 

  for (int i = 0; i < compSize; i++) {
    *oComp++ = (uint16) fClip(fRound((float) (weight * (double) *iComp++ + offset)), 0.0f, (float) maxPelValue);
  }
}

void ConvFloatToFixed::convertComponentLowBias (const float *iComp, uint16 *oComp, int compSize, double weight, double offset, int maxPelValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = (uint16) fClip(fRoundLowBias ((float) (weight * (double) *iComp++ + offset)), 0.0f, (float) maxPelValue);
  }
}

// Function used for the SDI_SCALED case
void ConvFloatToFixed::convertComponent (const float *iComp, uint16 *oComp, int bitScale, int compSize, double weight, double offset, int maxPelValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = ((uint16) fClip(fRound((float) (weight * (double) *iComp++ + offset)), 0.0f, (float) maxPelValue)) << bitScale;
  }
}


// Function used for the Sim2 Display
void ConvFloatToFixed::convertComponent (const float *iComp, uint16 *oComp, int compSize, int minPelValue, int maxPelValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp++ = (uint16) fClip(fRound((float) *iComp++ ), (float) minPelValue, (float) maxPelValue);
  }
}

void ConvFloatToFixed::convertCompData(Frame* out, const Frame *inp) {
  switch (out->m_sampleRange) {
    case SR_FULL:
      if (out->m_colorSpace == CM_YCbCr || out->m_colorSpace == CM_ICtCp) {
        convertComponent (inp->m_floatComp[Y_COMP], out->m_comp[Y_COMP], inp->m_compSize[Y_COMP], (double) (1 << (out->m_bitDepthComp[Y_COMP])) - 1.0, 0.0,  out->m_maxPelValue[Y_COMP]);
        convertComponent (inp->m_floatComp[U_COMP], out->m_comp[U_COMP], inp->m_compSize[U_COMP], (double) (1 << (out->m_bitDepthComp[U_COMP])) - 1.0, (double) (1 << (out->m_bitDepthComp[U_COMP]-1)), out->m_maxPelValue[U_COMP]);
        convertComponent (inp->m_floatComp[V_COMP], out->m_comp[V_COMP], inp->m_compSize[V_COMP], (double) (1 << (out->m_bitDepthComp[V_COMP])) - 1.0, (double) (1 << (out->m_bitDepthComp[V_COMP]-1)), out->m_maxPelValue[V_COMP]);
      }
      else {
        convertComponent (inp->m_floatComp[Y_COMP], out->m_comp[Y_COMP], inp->m_compSize[Y_COMP], (double) (1 << (out->m_bitDepthComp[Y_COMP])) - 1.0, 0.0, out->m_maxPelValue[Y_COMP]);
        convertComponent (inp->m_floatComp[U_COMP], out->m_comp[U_COMP], inp->m_compSize[U_COMP], (double) (1 << (out->m_bitDepthComp[U_COMP])) - 1.0, 0.0, out->m_maxPelValue[U_COMP]);
        convertComponent (inp->m_floatComp[V_COMP], out->m_comp[V_COMP], inp->m_compSize[V_COMP], (double) (1 << (out->m_bitDepthComp[V_COMP])) - 1.0, 0.0, out->m_maxPelValue[V_COMP]);
      }
      break;
      // SR_RESTRICTED, SR_SDI_SCALED, and SR_SDI not supported for 8 bit data, so lets keep those the same as SR_STANDARD range
    case SR_STANDARD:
    default:
      if (out->m_colorSpace == CM_YCbCr || out->m_colorSpace == CM_ICtCp) {
        convertComponent (inp->m_floatComp[Y_COMP], out->m_comp[Y_COMP], inp->m_compSize[Y_COMP], (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 16.0,  out->m_maxPelValue[Y_COMP]);
        convertComponent (inp->m_floatComp[U_COMP], out->m_comp[U_COMP], inp->m_compSize[U_COMP], (1 << (out->m_bitDepthComp[U_COMP]-8)) * 224.0, (1 << (out->m_bitDepthComp[U_COMP]-8)) * 128.0, out->m_maxPelValue[U_COMP]);
        convertComponent (inp->m_floatComp[V_COMP], out->m_comp[V_COMP], inp->m_compSize[V_COMP], (1 << (out->m_bitDepthComp[V_COMP]-8)) * 224.0, (1 << (out->m_bitDepthComp[V_COMP]-8)) * 128.0, out->m_maxPelValue[V_COMP]);
      }
      else {
        convertComponent (inp->m_floatComp[Y_COMP], out->m_comp[Y_COMP], inp->m_compSize[Y_COMP], (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 16.0, out->m_maxPelValue[Y_COMP]);
        convertComponent (inp->m_floatComp[U_COMP], out->m_comp[U_COMP], inp->m_compSize[U_COMP], (1 << (out->m_bitDepthComp[U_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[U_COMP]-8)) * 16.0, out->m_maxPelValue[U_COMP]);
        convertComponent (inp->m_floatComp[V_COMP], out->m_comp[V_COMP], inp->m_compSize[V_COMP], (1 << (out->m_bitDepthComp[V_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[V_COMP]-8)) * 16.0, out->m_maxPelValue[V_COMP]);
      }
      break;
  }
}

uint16 ConvFloatToFixed::convertUi16Value(float inpValue, SampleRange sampleRange, ColorSpace colorSpace, int bitDepth, int component)
{
  switch (sampleRange) {
    case SR_FULL:
      if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
        if (component == Y_COMP)
          return convertValue (inpValue, (float) (1 << (bitDepth)) - 1.0, 0.0,  (1 << bitDepth) - 1);
        else
          return convertValue (inpValue, (float) (1 << (bitDepth)) - 1.0, (double) (1 << (bitDepth-1)), (1 << bitDepth) - 1);
      }
      else {
          return convertValue (inpValue, (float) (1 << (bitDepth)) - 1.0, 0.0, (1 << bitDepth) - 1);
      }
      break;
    case SR_STANDARD:
    default:
      if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
        if (component == Y_COMP)
          return convertValue (inpValue, (1 << (bitDepth-8)) * 219.0, (1 << (bitDepth-8)) * 16.0,  (1 << bitDepth) - 1);
        else
          return convertValue (inpValue, (1 << (bitDepth-8)) * 224.0, (1 << (bitDepth-8)) * 128.0, (1 << bitDepth) - 1);
      }
      else {
          return convertValue (inpValue, (1 << (bitDepth-8)) * 219.0, (1 << (bitDepth-8)) * 16.0, (1 << bitDepth) - 1);
      }
      break;
    case SR_RESTRICTED:
      // implemented by considering min = 1 << (bitdepth - 8) and max = (1 << bitdepth) - min - 1 and range = max - min + 1 =  (1 << bitdepth) - (1 << (bitdepth - 7))
      if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) { // For YCbCr color values are centered around 0. So appropriate offset needs to be added
        if (component == Y_COMP)
          return convertValue (inpValue, (double) ((1 << bitDepth) - (1 << (bitDepth-7))), (double) (1 << (bitDepth-8)), (1 << bitDepth) - 1);
        else
          return convertValue (inpValue, (double) ((1 << bitDepth) - (1 << (bitDepth-7))), (double) (1 << (bitDepth-1)), (1 << bitDepth) - 1);
      }
      else {
          return convertValue (inpValue, (double) ((1 << bitDepth) - (1 << (bitDepth-7))), (double) (1 << (bitDepth-8)), (1 << bitDepth) - 1);
      }
      break;
    case SR_SDI_SCALED:
      // SDI specifies values in the range of Floor(1015 * D * N + 4 * D + 0.5), where N is the nonlinear color value from zero to unity, and D = 2^(B-10).
      // Note that we are cheating a bit here since SDI only allows values from 10-16 but we modified the function a bit to also handle bitdepths of 8 and 9 (although likely we will not use those). In this mode, values are scaled to use 16 bits.
      if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
        if (component == Y_COMP)
          return convertValue (inpValue, (16 - bitDepth), 253.75 * (1 << (bitDepth - 8)), (double) (1 << (bitDepth - 8)), (1 << bitDepth) - 1);
        else
          return convertValue (inpValue, (16 - bitDepth), 253.00 * (1 << (bitDepth - 8)), (double) (1 << (bitDepth - 1)), (1 << bitDepth) - 1);
      }
      else {
        return convertValue (inpValue, (16 - bitDepth), 253.75 * (1 << (bitDepth - 8)), (double) (1 << (bitDepth - 8)), (1 << bitDepth) - 1);
      }
      break;
    case SR_SDI:
      // SDI specifies values in the range of Floor(1015 * D * N + 4 * D + 0.5), where N is the nonlinear color value from zero to unity, and D = 2^(B-10).
      // Note that we are cheating a bit here since SDI only allows values from 10-16 but we modified the function a bit to also handle bitdepths of 8 and 9 (although likely we will not use those).
      if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
        if (component == Y_COMP)
          return convertValue (inpValue, 253.75 * (1 << (bitDepth - 8)), (double) (1 << (bitDepth - 8)), (1 << bitDepth) - 1);
        else
          return convertValue (inpValue, 253.00 * (1 << (bitDepth - 8)), (double) (1 << (bitDepth - 1)), (1 << bitDepth) - 1);
      }
      else {
          return convertValue (inpValue, 253.75 * (1 << (bitDepth - 8)), (double) (1 << (bitDepth - 8)), (1 << bitDepth) - 1);
      }
      break;
  }
}

void ConvFloatToFixed::convertUi16CompData(Frame* out, const Frame *inp) {
#ifdef __SIM2_SUPPORT_ENABLED__
  if (out->m_format.m_pixelFormat == PF_SIM2) {
    convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], 32, 8159);
    convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP],  4, 1019);
    convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP],  4, 1019);
  }
  else 
#endif
  {
    switch (out->m_sampleRange) {
      case SR_FULL:
        if (out->m_colorSpace == CM_YCbCr || out->m_colorSpace == CM_ICtCp) {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], (float) (1 << (out->m_bitDepthComp[Y_COMP])) - 1.0, 0.0,  out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], (float) (1 << (out->m_bitDepthComp[U_COMP])) - 1.0, (double) (1 << (out->m_bitDepthComp[U_COMP]-1)), out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], (float) (1 << (out->m_bitDepthComp[V_COMP])) - 1.0, (double) (1 << (out->m_bitDepthComp[V_COMP]-1)), out->m_maxPelValue[V_COMP]);
        }
        else {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], (float) (1 << (out->m_bitDepthComp[Y_COMP])) - 1.0, 0.0, out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], (float) (1 << (out->m_bitDepthComp[U_COMP])) - 1.0, 0.0, out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], (float) (1 << (out->m_bitDepthComp[V_COMP])) - 1.0, 0.0, out->m_maxPelValue[V_COMP]);
        }
        break;
      case SR_STANDARD:
      default:
        if (out->m_colorSpace == CM_YCbCr || out->m_colorSpace == CM_ICtCp) {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 16.0,  out->m_maxPelValue[Y_COMP]);
          if (m_lowbias == FALSE) {
            convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], (1 << (out->m_bitDepthComp[U_COMP]-8)) * 224.0, (1 << (out->m_bitDepthComp[U_COMP]-8)) * 128.0, out->m_maxPelValue[U_COMP]);
            convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], (1 << (out->m_bitDepthComp[V_COMP]-8)) * 224.0, (1 << (out->m_bitDepthComp[V_COMP]-8)) * 128.0, out->m_maxPelValue[V_COMP]);
          }
          else {
            convertComponentLowBias (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], (1 << (out->m_bitDepthComp[U_COMP]-8)) * 224.0, (1 << (out->m_bitDepthComp[U_COMP]-8)) * 128.0, out->m_maxPelValue[U_COMP]);
            convertComponentLowBias (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], (1 << (out->m_bitDepthComp[V_COMP]-8)) * 224.0, (1 << (out->m_bitDepthComp[V_COMP]-8)) * 128.0, out->m_maxPelValue[V_COMP]);
          }
        }
        else {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[Y_COMP]-8)) * 16.0, out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], (1 << (out->m_bitDepthComp[U_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[U_COMP]-8)) * 16.0, out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], (1 << (out->m_bitDepthComp[V_COMP]-8)) * 219.0, (1 << (out->m_bitDepthComp[V_COMP]-8)) * 16.0, out->m_maxPelValue[V_COMP]);
        }
        break;
      case SR_RESTRICTED:
        // implemented by considering min = 1 << (bitdepth - 8) and max = (1 << bitdepth) - min - 1 and range = max - min + 1 =  (1 << bitdepth) - (1 << (bitdepth - 7))
        if (out->m_colorSpace == CM_YCbCr || out->m_colorSpace == CM_ICtCp) { // For YCbCr color values are centered around 0. So appropriate offset needs to be added
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], (double) ((1 << out->m_bitDepthComp[Y_COMP]) - (1 << (out->m_bitDepthComp[Y_COMP]-7))), (double) (1 << (out->m_bitDepthComp[Y_COMP]-8)), out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], (double) ((1 << out->m_bitDepthComp[U_COMP]) - (1 << (out->m_bitDepthComp[U_COMP]-7))), (double) (1 << (out->m_bitDepthComp[U_COMP]-1)), out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], (double) ((1 << out->m_bitDepthComp[V_COMP]) - (1 << (out->m_bitDepthComp[V_COMP]-7))), (double) (1 << (out->m_bitDepthComp[V_COMP]-1)), out->m_maxPelValue[V_COMP]);
        }
        else {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], (double) ((1 << out->m_bitDepthComp[Y_COMP]) - (1 << (out->m_bitDepthComp[Y_COMP]-7))), (double) (1 << (out->m_bitDepthComp[Y_COMP]-8)), out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], (double) ((1 << out->m_bitDepthComp[U_COMP]) - (1 << (out->m_bitDepthComp[U_COMP]-7))), (double) (1 << (out->m_bitDepthComp[U_COMP]-8)), out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], (double) ((1 << out->m_bitDepthComp[V_COMP]) - (1 << (out->m_bitDepthComp[V_COMP]-7))), (double) (1 << (out->m_bitDepthComp[V_COMP]-8)), out->m_maxPelValue[V_COMP]);
        }
        break;
      case SR_SDI_SCALED:
        // SDI specifies values in the range of Floor(1015 * D * N + 4 * D + 0.5), where N is the nonlinear color value from zero to unity, and D = 2^(B-10).
        // Note that we are cheating a bit here since SDI only allows values from 10-16 but we modified the function a bit to also handle bitdepths of 8 and 9 (although likely we will not use those). In this mode, values are scaled to use 16 bits.
        if (out->m_colorSpace == CM_YCbCr || out->m_colorSpace == CM_ICtCp) {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], (16 - out->m_bitDepthComp[Y_COMP]), inp->m_compSize[Y_COMP], 253.75 * (1 << (out->m_bitDepthComp[Y_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[Y_COMP] - 8)), out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], (16 - out->m_bitDepthComp[U_COMP]), inp->m_compSize[U_COMP], 253.00 * (1 << (out->m_bitDepthComp[U_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[U_COMP] - 1)), out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], (16 - out->m_bitDepthComp[V_COMP]), inp->m_compSize[V_COMP], 253.00 * (1 << (out->m_bitDepthComp[V_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[V_COMP] - 1)), out->m_maxPelValue[V_COMP]);
        }
        else {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], (16 - out->m_bitDepthComp[Y_COMP]), inp->m_compSize[Y_COMP], 253.75 * (1 << (out->m_bitDepthComp[Y_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[Y_COMP] - 8)), out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], (16 - out->m_bitDepthComp[U_COMP]), inp->m_compSize[U_COMP], 253.75 * (1 << (out->m_bitDepthComp[U_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[U_COMP] - 8)), out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], (16 - out->m_bitDepthComp[V_COMP]), inp->m_compSize[V_COMP], 253.75 * (1 << (out->m_bitDepthComp[V_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[V_COMP] - 8)), out->m_maxPelValue[V_COMP]);
        }
        break;
      case SR_SDI:
        // SDI specifies values in the range of Floor(1015 * D * N + 4 * D + 0.5), where N is the nonlinear color value from zero to unity, and D = 2^(B-10).
        // Note that we are cheating a bit here since SDI only allows values from 10-16 but we modified the function a bit to also handle bitdepths of 8 and 9 (although likely we will not use those).
        if (out->m_colorSpace == CM_YCbCr || out->m_colorSpace == CM_ICtCp) {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], 253.75 * (1 << (out->m_bitDepthComp[Y_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[Y_COMP] - 8)), out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], 253.00 * (1 << (out->m_bitDepthComp[U_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[U_COMP] - 1)), out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], 253.00 * (1 << (out->m_bitDepthComp[V_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[V_COMP] - 1)), out->m_maxPelValue[V_COMP]);
        }
        else {
          convertComponent (inp->m_floatComp[Y_COMP], out->m_ui16Comp[Y_COMP], inp->m_compSize[Y_COMP], 253.75 * (1 << (out->m_bitDepthComp[Y_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[Y_COMP] - 8)), out->m_maxPelValue[Y_COMP]);
          convertComponent (inp->m_floatComp[U_COMP], out->m_ui16Comp[U_COMP], inp->m_compSize[U_COMP], 253.75 * (1 << (out->m_bitDepthComp[U_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[U_COMP] - 8)), out->m_maxPelValue[U_COMP]);
          convertComponent (inp->m_floatComp[V_COMP], out->m_ui16Comp[V_COMP], inp->m_compSize[V_COMP], 253.75 * (1 << (out->m_bitDepthComp[V_COMP] - 8)), (double) (1 << (out->m_bitDepthComp[V_COMP] - 8)), out->m_maxPelValue[V_COMP]);
        }
        break;
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void ConvFloatToFixed::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  if (out->m_bitDepth == 8)
    convertCompData(out, inp);
  else
    convertUi16CompData(out, inp);
}



//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
