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
 * \file ColorTransformGeneric.cpp
 *
 * \brief
 *    ColorTransformGeneric Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Chad Fogg
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "ColorTransformGeneric.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformGeneric::ColorTransformGeneric(ColorTransformParams *params) {
  
  m_mode = CTF_IDENTITY; 
  m_isForward = TRUE;
  m_crDivider = 1.0;
  m_cbDivider = 1.0;
  setupParams(params);
  m_transformPrecision = FALSE;

  m_sClip = 0;
  
  if (m_iColorSpace == CM_XYZ && m_oColorSpace == CM_YDZDX ) {
    m_mode = CTF_XYZ_2_DZDX;  // SMPTE 2085
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_XYZ) {
    if (m_iColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_2020) {
      m_mode = CTF_RGB2020_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBEXT_2_XYZ;
    }
    else if (m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_XYZ;
    }
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr_CL && m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
    m_mode = CTF_RGB2020_2_YUV2020CL;
    m_isForward = TRUE;
  }
  else if (m_iColorSpace == CM_YCbCr_CL && m_oColorSpace == CM_RGB && m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
    m_mode = CTF_RGB2020_2_YUV2020CL;
    m_isForward = FALSE;
    m_sClip = 1;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr) {
    if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_YUV709;
      m_crDivider = 1.5748;
      m_cbDivider = 1.8556;
      m_transformPrecision = params->m_transformPrecision;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_YUV601;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
      if (m_useHighPrecision == 1)
        m_mode = CTF_RGB2020_2_YUV2020_HP;
      else
        m_mode = CTF_RGB2020_2_YUV2020;
      m_crDivider = 1.4746;
      m_cbDivider = 1.8814;
      m_transformPrecision = params->m_transformPrecision;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_YUVP3D65;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGBP3D60_2_YUVP3D60;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBEXT_2_YUVEXT;
    }
    else if (m_oColorPrimaries == CP_AMT) {
      m_mode = CTF_RGB_2_AMT;
    }
    else if ( m_oColorPrimaries == CP_YCOCG) {
      m_mode = CTF_RGB_2_YCOCG;
    }
    else if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_YCbCrLMS;
    } 
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_ICtCp) {
    if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_ICtCp;
    } 
  }
  else if (m_iColorSpace == CM_ICtCp && m_oColorSpace == CM_RGB) {
    if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_ICtCp;
    }
    m_isForward = FALSE;
    m_sClip = 1;
  }
  else if (m_iColorSpace == CM_YCbCr && m_oColorSpace == CM_RGB) {
    if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_YUV709;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_YUV601;
    }    
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
      if (m_useHighPrecision == 2)
        m_mode = CTF_RGB2020_2_YUV2020_HP;
      else    
        m_mode = CTF_RGB2020_2_YUV2020;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_YUVP3D65;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGBP3D60_2_YUVP3D60;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBEXT_2_YUVEXT;
    }
    else if (m_iColorPrimaries == CP_AMT) {
      m_mode = CTF_RGB_2_AMT;
    }
    else if (m_iColorPrimaries == CP_YCOCG) {
      m_mode = CTF_RGB_2_YCOCG;
    } 
    else if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_YCbCrLMS;
    } 
    m_isForward = FALSE;
    m_sClip = 1;
  }
  else if (m_iColorSpace == CM_XYZ && m_oColorSpace == CM_RGB) {
    if (m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_XYZ;
    }
    else if (m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_XYZ;
    }
    else if (m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB2020_2_XYZ;
    }
    else if (m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_XYZ;
    }
    else if (m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBEXT_2_XYZ;
    }
    else if (m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_XYZ;
    }
    m_isForward = FALSE;
    m_sClip = 1;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_RGB) { // RGB to RGB conversion
    m_mode = CTF_IDENTITY; // just for safety

    if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB709_2_RGB2020;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGB2020;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB601_2_RGB709;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_RGB709;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB601_2_RGB2020;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_RGB2020;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGBP3D65_2_RGB2020;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_RGB2020;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGB709_2_RGBP3D65;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGBP3D65;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGBP3D60_2_RGB2020;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGBP3D60_2_RGB2020;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGB709_2_RGBP3D60;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGBP3D60;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGB2020_2_RGBEXT;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB2020_2_RGBEXT;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGB709_2_RGBEXT;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGBEXT;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBP3D65_2_RGBEXT;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_RGBEXT;
      m_isForward = FALSE;
      m_sClip = 2;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_RGB2020_2_LMSD;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_RGBP3D65_2_LMSD;
    }
    else if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB2020_2_LMSD;
      m_isForward = FALSE;
      m_sClip = 1;
    }
    else if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_LMSD;
      m_isForward = FALSE;
      m_sClip = 1;
    }
    //m_sClip = 2;
	  // Force clipping into the desirable color space. This means that both negative and values > 1.0 will be thrown out
  }
  else {
    m_mode = CTF_IDENTITY;
  }
  
  if (m_isForward == TRUE) {
    m_transform0 = FWD_TRANSFORM[m_mode][Y_COMP];
    m_transform1 = FWD_TRANSFORM[m_mode][U_COMP];
    m_transform2 = FWD_TRANSFORM[m_mode][V_COMP];
  }
  else {
    m_transform0 = INV_TRANSFORM[m_mode][Y_COMP];
    m_transform1 = INV_TRANSFORM[m_mode][U_COMP];
    m_transform2 = INV_TRANSFORM[m_mode][V_COMP];
  }
  
  m_min = params->m_min;
  m_max = params->m_max;
}

ColorTransformGeneric::~ColorTransformGeneric() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ColorTransformGeneric::RGB2YCbCrConstantLuminance(Frame *out,  const Frame *inp)
{
  
  // AMT note that this conversion is only correct for 2.2 gamma
  for (int i = 0; i < inp->m_compSize[R_COMP]; i++) {
    float R, G, B;
    
    if (inp->m_isFloat == TRUE ) {
      R = inp->m_floatComp[0][i];
      G = inp->m_floatComp[1][i];
      B = inp->m_floatComp[2][i];
    }
    else if (inp->m_bitDepth > 8) {
      R = (float) inp->m_ui16Comp[0][i];
      G = (float) inp->m_ui16Comp[1][i];
      B = (float) inp->m_ui16Comp[2][i];
    }
    else {
      R = inp->m_comp[0][i];
      G = inp->m_comp[1][i];
      B = inp->m_comp[2][i];
    }
    
    
    float Y = 0.2627f * R + 0.6780f * G + 0.0593f * B;
    
    if (inp->m_isFloat == TRUE ) {
      if (m_sClip == 1)
        out->m_floatComp[0][i] = fClip( Y, 0.0f, 1.0f);
      else
        out->m_floatComp[0][i] = Y;
    }
    else if( inp->m_bitDepth > 8 )
      out->m_ui16Comp[0][i] = (uint16) fClip( Y + 0.5f, (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0] );
    else
      out->m_comp[0][i] = (imgpel) fClip( Y + 0.5f, (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0] );
    
    
    // Cb Component conversion
    float Cb = 0.0f;
    const float PB = 0.7910f;
    const float NB = 0.9702f;
    
    float tempB = B - Y;
    
    if( tempB >= -NB && tempB <= 0.0f )
      Cb = tempB / (2.0f * NB);
    else if( tempB > 0.0f  && tempB <= PB )
      Cb = tempB / (2.0f * PB);
    
    if (inp->m_isFloat == TRUE ) {
      if (m_sClip == 1)
        out->m_floatComp[1][i] = fClip( Cb, 0.0f, 1.0f);
      else
        out->m_floatComp[1][i] = Cb;
    }
    else if( inp->m_bitDepth > 8 )
      out->m_ui16Comp[1][i] = (uint16) fClip( Cb + 0.5f, (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1] );
    else
      out->m_comp[1][i] = (imgpel) fClip( Cb + 0.5f, (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1] );
    
    
    // Cr Component conversion
    float Cr = 0.0f;
    const float PR = 0.4969f;
    const float NR = 0.8591f;
    
    float tempR = R - Y;
    
    if( tempR >= -NR && tempR <= 0.0f )
      Cr = tempR / (2.0f * NR);
    else if( tempR > 0.0f  && tempR <= PR )
      Cr = tempR / (2.0f * PR);
    
    if (inp->m_isFloat == TRUE ) {
      if (m_sClip == 1)
        out->m_floatComp[2][i] = fClip( Cr, 0.0f, 1.0f);
      else
        out->m_floatComp[2][i] = Cr;
    }
    else if( inp->m_bitDepth > 8 )
      out->m_ui16Comp[2][i] = (uint16) fClip( Cr + 0.5f, (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2] );
    else
      out->m_comp[2][i] = (imgpel) fClip( Cr + 0.5f, (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2] );
  }
  
}


void ColorTransformGeneric::YCbCrConstantLuminance2RGB(Frame *out,  const Frame *inp)
{
  // TODO
}

void ColorTransformGeneric::setYConversion(int colorPrimaries, const double **transformY) {
  int mode = CTF_IDENTITY;
  
  if (colorPrimaries == CP_709) {
    mode = CTF_RGB709_2_XYZ;
  }
  else if (colorPrimaries == CP_601) {
    mode = CTF_RGB601_2_XYZ;
  }
  else if (colorPrimaries == CP_2020) {
    mode = CTF_RGB2020_2_XYZ;
  }
  else if (colorPrimaries == CP_P3D65) {
    mode = CTF_RGBP3D65_2_XYZ;
  }
  else {
    // use of identity matrix for this mode
  }
  
  *transformY = FWD_TRANSFORM[mode][1];
}

void ColorTransformGeneric::setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2) {
  int mode = CTF_IDENTITY;
  if (colorPrimaries == CP_709) {
    mode = CTF_RGB709_2_XYZ;
  }
  else if (colorPrimaries == CP_601) {
    mode = CTF_RGB601_2_XYZ;
  }
  else if (colorPrimaries == CP_2020) {
    mode = CTF_RGB2020_2_XYZ;
  }
  else if (colorPrimaries == CP_P3D65) {
    mode = CTF_RGBP3D65_2_XYZ;
  }
  
  *transform0 = FWD_TRANSFORM[mode][Y_COMP];
  *transform1 = FWD_TRANSFORM[mode][U_COMP];
  *transform2 = FWD_TRANSFORM[mode][V_COMP];
}


void ColorTransformGeneric::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;

  // Current condition to perform this is that Frames are of same size and in 4:4:4
  // Can add more code to do the interpolation on the fly (and save memory/improve speed),
  // but this keeps our code more flexible for now.
  if (inp->m_compSize[Y_COMP] == out->m_compSize[Y_COMP] && inp->m_compSize[Y_COMP] == inp->m_compSize[U_COMP])
  {
    // This mode is actually obsolete and should not be used.
    // To be removed in an upcoming version
    if( m_mode == CTF_RGB2020_2_YUV2020CL  && (inp->m_bitDepth == out->m_bitDepth))
      // ITU-R BT.2020 constant luminance ?
    {
      if( m_isForward == TRUE )
        RGB2YCbCrConstantLuminance( out, inp );
      else
        YCbCrConstantLuminance2RGB( out, inp );
    }
    else {
    //printf("Matrix0 %14.10f %14.10f %14.10f\n", m_transform0[0], m_transform0[1], m_transform0[2]);
    //printf("Matrix1 %14.10f %14.10f %14.10f\n", m_transform1[0], m_transform1[1], m_transform1[2]);
    //printf("Matrix2 %14.10f %14.10f %14.10f\n", m_transform2[0], m_transform2[1], m_transform2[2]);
      if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE)  {
        if (m_transformPrecision == FALSE) {
          if (m_sClip == 1) {
            for (int i = 0; i < inp->m_compSize[0]; i++) {
              double comp0 = inp->m_floatComp[0][i];
              double comp1 = inp->m_floatComp[1][i];
              double comp2 = inp->m_floatComp[2][i];
              // Note that since the input and output may be either RGB or YUV, it might be "better" not to use Y_COMP/R_COMP here to avoid confusion.
              out->m_floatComp[0][i] = fClip((float) (m_transform0[0] * comp0 + m_transform0[1] * comp1 + m_transform0[2] * comp2), m_min, m_max);
              out->m_floatComp[1][i] = fClip((float) (m_transform1[0] * comp0 + m_transform1[1] * comp1 + m_transform1[2] * comp2), m_min, m_max);
              out->m_floatComp[2][i] = fClip((float) (m_transform2[0] * comp0 + m_transform2[1] * comp1 + m_transform2[2] * comp2), m_min, m_max);
            }
          }
          else if (m_sClip == 2) {
            for (int i = 0; i < inp->m_compSize[0]; i++) {
              double comp0 = inp->m_floatComp[0][i];
              double comp1 = inp->m_floatComp[1][i];
              double comp2 = inp->m_floatComp[2][i];
              // Note that since the input and output may be either RGB or YUV, it might be "better" not to use Y_COMP/R_COMP here to avoid confusion.
              out->m_floatComp[0][i] = fMax((float) (m_transform0[0] * comp0 + m_transform0[1] * comp1 + m_transform0[2] * comp2), 0.0f);
              out->m_floatComp[1][i] = fMax((float) (m_transform1[0] * comp0 + m_transform1[1] * comp1 + m_transform1[2] * comp2), 0.0f);
              out->m_floatComp[2][i] = fMax((float) (m_transform2[0] * comp0 + m_transform2[1] * comp1 + m_transform2[2] * comp2), 0.0f);
            }
          }
          else {
//KW-KYH Color conersion: R¡¯G¡¯B¡¯ to *NCL Y¡¯CbCr

            for (int i = 0; i < inp->m_compSize[0]; i++) {
              double comp0 = inp->m_floatComp[0][i];
              double comp1 = inp->m_floatComp[1][i];
              double comp2 = inp->m_floatComp[2][i];

              out->m_floatComp[0][i] = (float) (m_transform0[0] * comp0 + m_transform0[1] * comp1 + m_transform0[2] * comp2);
              out->m_floatComp[1][i] = (float) (m_transform1[0] * comp0 + m_transform1[1] * comp1 + m_transform1[2] * comp2);
              out->m_floatComp[2][i] = (float) (m_transform2[0] * comp0 + m_transform2[1] * comp1 + m_transform2[2] * comp2);

              //printf("(%d) Values (Generic) (%7.3f %7.3f %7.3f) => (%7.3f %7.3f %7.3f)\n", i, inp->m_floatComp[0][i], inp->m_floatComp[1][i], inp->m_floatComp[2][i], out->m_floatComp[0][i], out->m_floatComp[1][i], out->m_floatComp[2][i]);
              //printf("(%d) Values (Generic) (%7.3f %7.3f %7.3f) => (%7.3f %7.3f %7.3f) & (%7.3f %7.3f %7.3f)\n", i, inp->m_floatComp[0][i], inp->m_floatComp[1][i], inp->m_floatComp[2][i], out->m_floatComp[0][i], out->m_floatComp[1][i], out->m_floatComp[2][i], fClip(out->m_floatComp[0][i], 0, 1.0), out->m_floatComp[1][i], out->m_floatComp[2][i]);

            }
          }
        }
        else {
          for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
            out->m_floatComp[Y_COMP][i] = (float) (m_transform0[R_COMP] * inp->m_floatComp[R_COMP][i] + m_transform0[G_COMP] * inp->m_floatComp[G_COMP][i] + m_transform0[B_COMP] * inp->m_floatComp[B_COMP][i]);
            out->m_floatComp[U_COMP][i] = (float) ((inp->m_floatComp[B_COMP][i] - out->m_floatComp[Y_COMP][i]) / m_cbDivider);
            out->m_floatComp[V_COMP][i] = (float) ((inp->m_floatComp[R_COMP][i] - out->m_floatComp[Y_COMP][i]) / m_crDivider);
          }
        }
      }
      else { // fixed precision, integer image data
             // We should always ideally have inp->m_bitDepth == out->m_bitDepth. Still, the code currently does not
             // perform any enforcing of that condition, but maybe we should.
        if (inp->m_bitDepth == out->m_bitDepth) {
          if (inp->m_bitDepth > 8) {
            int i;
            if (m_transformPrecision == FALSE) {
              // In this scenario, we always clip
              for (i = 0; i < inp->m_compSize[0]; i++) {
                // Note that since the input and output may be either RGB or YUV, it might be "better" not to use Y_COMP/R_COMP here to avoid confusion.
                out->m_ui16Comp[0][i] = (uint16) fClip((float) (m_transform0[0] * (float) inp->m_ui16Comp[0][i] + m_transform0[1] * (float) inp->m_ui16Comp[1][i] + m_transform0[2] * (float) inp->m_ui16Comp[2][i] + 0.5), (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0]);
                out->m_ui16Comp[1][i] = (uint16) fClip((float) (m_transform1[0] * (float) inp->m_ui16Comp[0][i] + m_transform1[1] * (float) inp->m_ui16Comp[1][i] + m_transform1[2] * (float) inp->m_ui16Comp[2][i] + 0.5), (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1]);
                out->m_ui16Comp[2][i] = (uint16) fClip((float) (m_transform2[0] * (float) inp->m_ui16Comp[0][i] + m_transform2[1] * (float) inp->m_ui16Comp[1][i] + m_transform2[2] * (float) inp->m_ui16Comp[2][i] + 0.5), (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2]);
              }
            }
            else {
              for (i = 0; i < inp->m_compSize[Y_COMP]; i++) {
                out->m_ui16Comp[Y_COMP][i] = (uint16) fClip((float) (m_transform0[R_COMP] * (float) inp->m_ui16Comp[R_COMP][i] + m_transform0[G_COMP] * (float) inp->m_ui16Comp[G_COMP][i] + m_transform0[B_COMP] * (float) inp->m_ui16Comp[B_COMP][i] + 0.5), (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0]);
                out->m_ui16Comp[U_COMP][i] = (uint16) fClip((float) (((float) inp->m_ui16Comp[B_COMP][i] - (float) out->m_ui16Comp[Y_COMP][i]) / m_cbDivider + 0.5), (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1]);
                out->m_ui16Comp[V_COMP][i] = (uint16) fClip((float) (((float) inp->m_ui16Comp[R_COMP][i] - (float) out->m_ui16Comp[Y_COMP][i]) / m_crDivider + 0.5), (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2]);
              }
            }
          }
          else {
            int i;
            if (m_transformPrecision == FALSE) {
              // In this scenario, we always clip
              for (i = 0; i < inp->m_compSize[0]; i++) {
                // Note that since the input and output may be either RGB or YUV, it might be "better" not to use Y_COMP/R_COMP here to avoid confusion.
                out->m_comp[0][i] = (imgpel) fClip((float) (m_transform0[0] * (float) inp->m_comp[0][i] + m_transform0[1] * (float) inp->m_comp[1][i] + m_transform0[2] * (float) inp->m_comp[2][i] + 0.5), (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0]);
                out->m_comp[1][i] = (imgpel) fClip((float) (m_transform1[0] * (float) inp->m_comp[0][i] + m_transform1[1] * (float) inp->m_comp[1][i] + m_transform1[2] * (float) inp->m_comp[2][i] + 0.5), (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1]);
                out->m_comp[2][i] = (imgpel) fClip((float) (m_transform2[0] * (float) inp->m_comp[0][i] + m_transform2[1] * (float) inp->m_comp[1][i] + m_transform2[2] * (float) inp->m_comp[2][i] + 0.5), (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2]);
              }
            }
            else {
              for (i = 0; i < inp->m_compSize[Y_COMP]; i++) {
                out->m_comp[Y_COMP][i] = (imgpel) fClip((float) (m_transform0[R_COMP] * (float) inp->m_comp[R_COMP][i] + m_transform0[G_COMP] * (float) inp->m_comp[G_COMP][i] + m_transform0[B_COMP] * (float) inp->m_comp[B_COMP][i] + 0.5f), (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0]);
                out->m_comp[U_COMP][i] = (imgpel) fClip((float) (((float) inp->m_comp[B_COMP][i] - (float) out->m_comp[Y_COMP][i]) / m_cbDivider + 0.5f), (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1]);
                out->m_comp[V_COMP][i] = (imgpel) fClip((float) (((float) inp->m_comp[R_COMP][i] - (float) out->m_comp[Y_COMP][i]) / m_crDivider + 0.5f), (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2]);
              }
            }
          }
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
