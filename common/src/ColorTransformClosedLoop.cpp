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
 * \file ColorTransformClosedLoop.cpp
 *
 * \brief
 *    ColorTransformClosedLoop Class
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
#include "ColorTransformClosedLoop.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformClosedLoop::ColorTransformClosedLoop( ColorTransformParams *params ) {
  
  m_mode = CTF_IDENTITY; 
  m_isForward = TRUE;
  m_crDivider = 1.0;
  m_cbDivider = 1.0;
  setupParams(params);
  m_transformPrecision = FALSE;
  m_closedLoopTransform = params->m_closedLoopTransform;
  m_clip = FALSE;
  
  m_size = 0;
  for (int index = 0; index < 4; index++) {
    m_floatComp[index] = NULL;
    
    m_compSize[index] = 0;       // number of samples in each color component
    m_height[index] = 0;         // height of each color component
    m_width[index] = 0;          // width of each color component
  }
  m_memoryAllocated = FALSE;
  
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
    m_isForward = FALSE;//
    m_clip = TRUE;
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
    else if (m_oColorPrimaries == CP_YCOCG) {
      m_mode = CTF_RGB_2_YCOCG;
    }
    else if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_YCbCrLMS;
    } 
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_ICtCp) {
    if (m_oColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_ICtCp;
    } 
  }
  else if (m_iColorSpace == CM_ICtCp && m_oColorSpace == CM_RGB) {
    if (m_iColorPrimaries == CP_LMSD) {
      m_mode = CTF_LMSD_2_ICtCp;
    } 
    m_isForward = FALSE;
    m_clip = 1;
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
    m_clip = TRUE;
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
    m_clip = TRUE;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_RGB) { // RGB to RGB conversion
    m_mode = CTF_IDENTITY; // just for safety
    
    if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB709_2_RGB2020;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGB2020;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB601_2_RGB709;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_RGB709;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB601_2_RGB2020;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_RGB2020;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGBP3D65_2_RGB2020;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_RGB2020;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGB709_2_RGBP3D65;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGBP3D65;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGBP3D60_2_RGB2020;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGBP3D60_2_RGB2020;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGB709_2_RGBP3D60;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGBP3D60;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGB2020_2_RGBEXT;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB2020_2_RGBEXT;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGB709_2_RGBEXT;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_RGBEXT;
      m_isForward = FALSE;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBP3D65_2_RGBEXT;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_RGBEXT;
      m_isForward = FALSE;
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
    }
    else if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_LMSD;
      m_isForward = FALSE;
    }
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
}

ColorTransformClosedLoop::~ColorTransformClosedLoop() {
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ColorTransformClosedLoop::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  // Current condition to perform this is that Frames are of same size and in 4:4:4
  // Can add more code to do the interpolation on the fly (and save memory/improve speed),
  // but this keeps our code more flexible for now.
  
  if (inp->m_compSize[Y_COMP] == out->m_compSize[Y_COMP] && inp->m_compSize[Y_COMP] == inp->m_compSize[U_COMP])
  {
    if( m_mode == CTF_RGB2020_2_YUV2020CL  && (inp->m_bitDepth == out->m_bitDepth))
      // ITU-R BT.2020 constant luminance ?
    {
    }
    else {
      if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE)  {
        if (m_transformPrecision == FALSE) {
          if (m_clip == TRUE) {
            for (int i = 0; i < inp->m_compSize[0]; i++) {
              // Note that since the input and output may be either RGB or YUV, it might be "better" not to use Y_COMP/R_COMP here to avoid confusion.
              out->m_floatComp[0][i] = fClip((float) (m_transform0[0] * inp->m_floatComp[0][i] + m_transform0[1] * inp->m_floatComp[1][i] + m_transform0[2] * inp->m_floatComp[2][i]), 0.0f, 1.0f);
              out->m_floatComp[1][i] = fClip((float) (m_transform1[0] * inp->m_floatComp[0][i] + m_transform1[1] * inp->m_floatComp[1][i] + m_transform1[2] * inp->m_floatComp[2][i]), 0.0f, 1.0f);
              out->m_floatComp[2][i] = fClip((float) (m_transform2[0] * inp->m_floatComp[0][i] + m_transform2[1] * inp->m_floatComp[1][i] + m_transform2[2] * inp->m_floatComp[2][i]), 0.0f, 1.0f);
            }
          }
          else {
          
            if (m_closedLoopTransform) {
              // Allocate memory
              if (m_memoryAllocated == FALSE) { 
                m_width [Y_COMP] = inp->m_width[ Y_COMP];
                m_width [Y_COMP] = inp->m_width[ U_COMP];
                m_width [Y_COMP] = inp->m_width[ V_COMP];
                
                for (int index = ZERO; index < 3; index++) {
                  m_width[index]  = inp->m_width[index];
                  m_height[index] = inp->m_height[index];
                  
                  m_compSize[index] = inp->m_compSize[index];
                }
                
                m_size =  m_compSize[ZERO] + m_compSize[ONE] + m_compSize[TWO];
                m_floatData.resize((unsigned int) m_size);
                if (m_floatData.size() != (unsigned int) m_size) {
                  fprintf(stderr, "ColorTransformClosedLoop: Not enough memory to create array m_floatData, of size %d", (int) m_size);
                  exit(-1);
                }
                
                m_floatComp[Y_COMP] = &m_floatData[0];
                m_floatComp[U_COMP] = m_floatComp[Y_COMP] + m_compSize[Y_COMP];
                m_floatComp[V_COMP] = m_floatComp[U_COMP] + m_compSize[U_COMP];
                m_memoryAllocated = TRUE;
              }              
            }
            
            if (m_closedLoopTransform == CLT_NULL) {
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                out->m_floatComp[0][i] = (float) (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i]);
                out->m_floatComp[1][i] = (float) (m_transform1[0] * (double) inp->m_floatComp[0][i] + m_transform1[1] * (double) inp->m_floatComp[1][i] + m_transform1[2] * inp->m_floatComp[2][i]);
                out->m_floatComp[2][i] = (float) (m_transform2[0] * (double) inp->m_floatComp[0][i] + m_transform2[1] * (double) inp->m_floatComp[1][i] + m_transform2[2] * (double) inp->m_floatComp[2][i]);
                
                //printf("(%d) Values (Generic) (%7.3f %7.3f %7.3f) => (%7.3f %7.3f %7.3f)\n", i, inp->m_floatComp[0][i], inp->m_floatComp[1][i], inp->m_floatComp[2][i], out->m_floatComp[0][i], out->m_floatComp[1][i], out->m_floatComp[2][i]);
              }
            }
            else if (m_mode == CTF_RGB_2_AMT || m_mode == CTF_RGB_2_YCOCG){ //YCgCo
              double lumaWeight = (1 << (m_bitDepth - 8)) * 219.0;
              double lumaOffset = (1 << (m_bitDepth - 8)) * 16.0;
              double chromaWeight = (1 << (m_bitDepth - 8)) * 224.0;
              double chromaOffset = (1 << (m_bitDepth - 8)) * 128.0;
              
              // Y component
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                //out->m_floatComp[0][i] = (float) (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i]);
                out->m_floatComp[0][i] = (float) (0.25 * (double) inp->m_floatComp[0][i] + 0.5 * (double) inp->m_floatComp[1][i] + 0.25 * (double) inp->m_floatComp[2][i]);
                
                m_floatComp[0][i] = (float) (((double) fClip(fRound((float) (lumaWeight * (double) out->m_floatComp[0][i] + lumaOffset)), 0.0f, (float) inp->m_maxPelValue[Y_COMP]) - lumaOffset) / lumaWeight);
              }
              
              // Cg
              for (int i = 0; i < inp->m_compSize[1]; i++) { // G - Y
                out->m_floatComp[1][i] = (float) ( (double) inp->m_floatComp[1][i] - (double) m_floatComp[0][i]);
                
                m_floatComp[1][i] = (float) (((double) fClip(fRound((float) (chromaWeight * (double) out->m_floatComp[1][i] + chromaOffset)), 0.0f, (float) inp->m_maxPelValue[U_COMP]) - chromaOffset) / chromaWeight);
              }
              
              // Co
              for (int i = 0; i < inp->m_compSize[2]; i++) { // R + Cg - Y or (Y - Cg) - B
               // out->m_floatComp[2][i] = (float) ( (double) inp->m_floatComp[0][i] +  (double) m_floatComp[1][i] - (double) m_floatComp[0][i]);
                //out->m_floatComp[2][i] = (float) ( (double) m_floatComp[0][i] - (double) m_floatComp[1][i] - (double) inp->m_floatComp[2][i]);
                out->m_floatComp[2][i] = (float) (0.5 * ( (double) inp->m_floatComp[0][i] - (double) inp->m_floatComp[2][i]));
              }
            }
            else if (m_closedLoopTransform == CLT_BASE) {
              double lumaWeight = (1 << (m_bitDepth - 8)) * 219.0;
              double lumaOffset = (1 << (m_bitDepth - 8)) * 16.0;
              double comb_transform1[4];
              //double comb_denom1 = m_transform0[0]; // Should we do the division here or after the sum? 
              // Complexity versus precision question. TBD
              double comb_transform2[4];
              //double comb_denom2 = m_transform0[2]; // Should we do the division here or after the sum? 
              // Complexity versus precision question. TBD
              
              // Note that I am keeping 0/1 positions just for reference and I am already assuming that these are 0.0.
              // This makes the code not generic enough and caution should be applied in using this module. 
              // This now only works for RGB to YCbCr conversion
              comb_transform1[0] = 0.0; // Red
              comb_transform1[1] = 0.0; // Green
              comb_transform1[2] = ((m_transform1[2] *  m_transform0[0])- (m_transform0[2] * m_transform1[0])) / m_transform0[0]; // Blue
              comb_transform1[3] = (m_transform1[0] / m_transform0[0]); // Y
              
              comb_transform2[0] = ((m_transform2[0] *  m_transform0[2])- (m_transform0[0] * m_transform2[2])) / m_transform0[2]; // Red
              comb_transform2[1] = 0.0; // Green
              comb_transform2[2] = 0.0; // Blue
              comb_transform2[3] = (m_transform2[2] / m_transform0[2]); // Y
              
              if (m_range == SR_STANDARD) {
                lumaWeight = (double)(1 << (m_bitDepth - 8)) * 219.0;
                lumaOffset = (double)(1 << (m_bitDepth - 8)) * 16.0;
              }
              else if (m_range == SR_FULL) {
                lumaWeight = (double)(1 << (m_bitDepth)) - 1.0;
                lumaOffset = 0.0;
              }
              else if (m_range == SR_SDI) {
                lumaWeight = (double)(1 << (m_bitDepth - 8)) * 253.75;
                lumaOffset = (double) (1 << (m_bitDepth - 8));
              }
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                out->m_floatComp[0][i] = (float) (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i]);
                
                m_floatComp[0][i] = (float) (((double) fClip(fRound((float) (lumaWeight * (double) out->m_floatComp[0][i] + lumaOffset)), 0.0f, (float) inp->m_maxPelValue[Y_COMP]) - lumaOffset) / lumaWeight);
              }
              
              for (int i = 0; i < inp->m_compSize[1]; i++) {
                // Red and Green are 0. Only Blue and Y are considered here.
                // Maybe an important condition to avoid error in the gray areas.
                // This might be unecessary of course and given the limited precision we have
                // This may result always in 0. Still, lets keep this here for now.
                if (inp->m_floatComp[2][i] == out->m_floatComp[0][i])
                  out->m_floatComp[1][i] = 0.0f;
                else
                  out->m_floatComp[1][i] = (float) (comb_transform1[2] * (double) inp->m_floatComp[2][i] + comb_transform1[3] * (double) m_floatComp[0][i]);
              }
              
              for (int i = 0; i < inp->m_compSize[2]; i++) {
                // Blue and Green are 0. Only Red and Y are considered here.
                if (inp->m_floatComp[0][i] == out->m_floatComp[0][i])
                  out->m_floatComp[2][i] = 0.0f;
                else
                  out->m_floatComp[2][i] = (float) (comb_transform2[0] * (double) inp->m_floatComp[0][i] + comb_transform2[3] * (double) m_floatComp[0][i]);
              }
            }
            else if (m_closedLoopTransform == CLT_BASE2) {
              double comb_transform1 = (m_transform0[0] / m_transform1[0]); // Y
              double comb_transform2 = (m_transform0[2] / m_transform2[2]); // Y
              
              double chromaWeight = (1 << (m_bitDepth - 8)) * 224.0;
              double chromaOffset = (1 << (m_bitDepth - 8)) * 128.0;
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                // We now convert Cb and Cr first
                out->m_floatComp[1][i] = (float) (m_transform1[0] * (double) inp->m_floatComp[0][i] + m_transform1[1] * (double) inp->m_floatComp[1][i] + m_transform1[2] * inp->m_floatComp[2][i]);
                out->m_floatComp[2][i] = (float) (m_transform2[0] * (double) inp->m_floatComp[0][i] + m_transform2[1] * (double) inp->m_floatComp[1][i] + m_transform2[2] * (double) inp->m_floatComp[2][i]);
                                
                m_floatComp[1][i] = (float) (((double) fClip(fRound((float) (chromaWeight * (double) out->m_floatComp[1][i] + chromaOffset)), 0.0f, (float) inp->m_maxPelValue[U_COMP]) - chromaOffset) / chromaWeight);
                m_floatComp[2][i] = (float) (((double) fClip(fRound((float) (chromaWeight * (double) out->m_floatComp[2][i] + chromaOffset)), 0.0f, (float) inp->m_maxPelValue[V_COMP]) - chromaOffset) / chromaWeight);
                //out->m_floatComp[1][i]  = m_floatComp[1][i];
                //out->m_floatComp[2][i]  = m_floatComp[2][i];
              }
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                
                // (Red - Blue - alpha * Cb - beta * Cr)/2.0
                if (m_floatComp[1][i] == 0.0) {
                  if (m_floatComp[2][i] == 0.0) {
                    out->m_floatComp[0][i] = (float) (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i]);
                  }
                  else { // Cr not 0
                         // Y = (R - Cr * 1.4746)
                    out->m_floatComp[0][i] = (float) ((double) inp->m_floatComp[0][i] + (double) m_floatComp[2][i] * comb_transform2);
                  }
                } // Cb not 0
                else if (m_floatComp[2][i] == 0.0) {
                // Y = (B - Cb * 1.8814)
                  out->m_floatComp[0][i] = (float)((double) inp->m_floatComp[2][i] + (double) m_floatComp[1][i] * comb_transform1);                  
                }
                else // Cb and Cr not 0
                  out->m_floatComp[0][i] = (float) (((double) inp->m_floatComp[0][i] + (double) inp->m_floatComp[2][i] + (double) m_floatComp[1][i] * comb_transform1 + (double) m_floatComp[2][i] * comb_transform2) * 0.5);

                //out->m_floatComp[0][i] = (float) (m_transform0[0] * inp->m_floatComp[0][i] + m_transform0[1] * inp->m_floatComp[1][i] + m_transform0[2] * inp->m_floatComp[2][i]);
                out->m_floatComp[0][i] = (float) ((((double) inp->m_floatComp[0][i] + (double) inp->m_floatComp[2][i] + (double) m_floatComp[1][i] * comb_transform1 + (double) m_floatComp[2][i] * comb_transform2) + 2.0 * (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i])) * 0.25);
              }              
            } 
            else if (m_closedLoopTransform == CLT_BASE3) {
              double comb_transform1 = (m_transform0[0] / m_transform1[0]); // Y
              double comb_transform2 = (m_transform0[2] / m_transform2[2]); // Y
              
              double chromaWeight = (1 << (m_bitDepth - 8)) * 224.0;
              double chromaOffset = (1 << (m_bitDepth - 8)) * 128.0;
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                // We now convert Cb and Cr first
                out->m_floatComp[1][i] = (float) (m_transform1[0] * (double) inp->m_floatComp[0][i] + m_transform1[1] * (double) inp->m_floatComp[1][i] + m_transform1[2] * inp->m_floatComp[2][i]);
                out->m_floatComp[2][i] = (float) (m_transform2[0] * (double) inp->m_floatComp[0][i] + m_transform2[1] * (double) inp->m_floatComp[1][i] + m_transform2[2] * (double) inp->m_floatComp[2][i]);
                
                m_floatComp[1][i] = (float) (((double) fClip(fRound((float) (chromaWeight * (double) out->m_floatComp[1][i] + chromaOffset)), 0.0f, (float) inp->m_maxPelValue[U_COMP]) - chromaOffset) / chromaWeight);
                m_floatComp[2][i] = (float) (((double) fClip(fRound((float) (chromaWeight * (double) out->m_floatComp[2][i] + chromaOffset)), 0.0f, (float) inp->m_maxPelValue[V_COMP]) - chromaOffset) / chromaWeight);
                //out->m_floatComp[1][i]  = m_floatComp[1][i];
                //out->m_floatComp[2][i]  = m_floatComp[2][i];
              }
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                
                out->m_floatComp[0][i] = (float) ((((double) inp->m_floatComp[0][i] + (double) inp->m_floatComp[2][i] + (double) m_floatComp[1][i] * comb_transform1 + (double) m_floatComp[2][i] * comb_transform2) + 2.0 * (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i])) * 0.25);
              }              
            }
            else if (m_closedLoopTransform == CLT_BASE4) {
            // Special luma mode where we only use the closed loop method if Cb or Cr result in smaller absolute values.
            // That works like a filter and may result in easier to encode content.
              double comb_transform1[4];
              //double comb_denom1 = m_transform0[0]; // Should we do the division here or after the sum? 
              // Complexity versus precision question. TBD
              double comb_transform2[4];
              //double comb_denom2 = m_transform0[2]; // Should we do the division here or after the sum? 
              // Complexity versus precision question. TBD
              
              // Note that I am keeping 0/1 positions just for reference and I am already assuming that these are 0.0.
              // This makes the code not generic enough and caution should be applied in using this module. 
              // This now only works for RGB to YCbCr conversion
              comb_transform1[0] = 0.0; // Red
              comb_transform1[1] = 0.0; // Green
              comb_transform1[2] = ((m_transform1[2] *  m_transform0[0])- (m_transform0[2] * m_transform1[0])) / m_transform0[0]; // Blue
              comb_transform1[3] = (m_transform1[0] / m_transform0[0]); // Y
              
              comb_transform2[0] = ((m_transform2[0] *  m_transform0[2])- (m_transform0[0] * m_transform2[2])) / m_transform0[2]; // Red
              comb_transform2[1] = 0.0; // Green
              comb_transform2[2] = 0.0; // Blue
              comb_transform2[3] = (m_transform2[2] / m_transform0[2]); // Y
              
              double lumaWeight = (1 << (m_bitDepth - 8)) * 219.0;
              double lumaOffset = (1 << (m_bitDepth - 8)) * 16.0;
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                out->m_floatComp[0][i] = (float) (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i]);
                
                m_floatComp[0][i] = (float) (((double) fClip(fRound((float) (lumaWeight * (double) out->m_floatComp[0][i] + lumaOffset)), 0.0f, (float) inp->m_maxPelValue[Y_COMP]) - lumaOffset) / lumaWeight);
              }
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                // Red and Green are 0. Only Blue and Y are considered here.
                // Maybe an important condition to avoid error in the gray areas.
                // This might be unecessary of course and given the limited precision we have
                // This may result always in 0. Still, lets keep this here for now.
                if (inp->m_floatComp[2][i] == out->m_floatComp[0][i])
                  out->m_floatComp[1][i] = 0.0f;
                else {
                  double closedTransform = (comb_transform1[2] * (double) inp->m_floatComp[2][i] + comb_transform1[3] * (double) m_floatComp[0][i]);
                  double openTransform = (m_transform1[0] * (double) inp->m_floatComp[0][i] + m_transform1[1] * (double) inp->m_floatComp[1][i] + m_transform1[2] * inp->m_floatComp[2][i]);
                  
                  if (dAbs(closedTransform) > dAbs(openTransform))
                    out->m_floatComp[1][i] = (float) openTransform;
                  else
                    out->m_floatComp[1][i] = (float) closedTransform;
                }
              }
              
              for (int i = 0; i < inp->m_compSize[0]; i++) {
                // Blue and Green are 0. Only Red and Y are considered here.
                if (inp->m_floatComp[0][i] == out->m_floatComp[0][i])
                  out->m_floatComp[2][i] = 0.0f;
                else {
                  double closedTransform = (comb_transform2[0] * (double) inp->m_floatComp[0][i] + comb_transform2[3] * (double) m_floatComp[0][i]);
                  double openTransform = (m_transform2[0] * (double) inp->m_floatComp[0][i] + m_transform2[1] * (double) inp->m_floatComp[1][i] + m_transform2[2] * (double) inp->m_floatComp[2][i]);

                  if (dAbs(closedTransform) > dAbs(openTransform))
                    out->m_floatComp[2][i] = (float) openTransform;
                  else
                    out->m_floatComp[2][i] = (float) closedTransform;

                }
              }
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
