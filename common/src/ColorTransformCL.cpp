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
 * \file ColorTransformCL.cpp
 *
 * \brief
 *    ColorTransformCL (Constant Luminance) Class
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
#include "ColorTransformCL.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------
#define SAME_TF 1

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformCL::ColorTransformCL(ColorTransformParams *iParams, int forceRange) {
  
  m_mode = CTF_IDENTITY;
  m_closedLoop = FALSE;
  
  setupParams(iParams);

  if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr && m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_709) {
    m_mode = CTF_RGB709_2_YUV709;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr && m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
    if (m_useHighPrecision == 1)
      m_mode = CTF_RGB2020_2_YUV2020_HP;
    else
    m_mode = CTF_RGB2020_2_YUV2020;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr && m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_P3D65) {
    m_mode = CTF_RGBP3D65_2_YUVP3D65;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr && m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_P3D60) {
    m_mode = CTF_RGBP3D60_2_YUVP3D60;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr && m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_601) {
    m_mode = CTF_RGB601_2_YUV601;
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr && m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_EXT) {
    m_mode = CTF_RGBEXT_2_YUVEXT;
  }
  else if (m_iColorSpace == CM_YCbCr && m_oColorSpace == CM_RGB && m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_601) {
    m_mode = CTF_RGB601_2_YUV601;
  }
  else if (m_iColorSpace == CM_YCbCr && m_oColorSpace == CM_RGB && m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_709) {
    m_mode = CTF_RGB709_2_YUV709;
  }
  else if (m_iColorSpace == CM_YCbCr && m_oColorSpace == CM_RGB && m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
    if (m_useHighPrecision == 2)
      m_mode = CTF_RGB2020_2_YUV2020_HP;
    else
    m_mode = CTF_RGB2020_2_YUV2020;
  }
  else if (m_iColorSpace == CM_YCbCr && m_oColorSpace == CM_RGB && m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_P3D65) {
    m_mode = CTF_RGBP3D65_2_YUVP3D65;
  }
  else if (m_iColorSpace == CM_YCbCr && m_oColorSpace == CM_RGB && m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_P3D60) {
    m_mode = CTF_RGBP3D60_2_YUVP3D60;
  }
  else if (m_iColorSpace == CM_YCbCr && m_oColorSpace == CM_RGB && m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_EXT) {
    m_mode = CTF_RGBEXT_2_YUVEXT;
  }
  else {
    m_mode = CTF_IDENTITY;
  }
  
  m_transformY = FWD_TRANSFORM[m_mode][Y_COMP];
    
  m_lumaTF   = TransferFunction::create(m_transferFunctions, TRUE, 1.0f, m_oSystemGamma, 0.0f, 1.0f);
  m_chromaTF = TransferFunction::create(m_transferFunctions, TRUE, 1.0f, m_oSystemGamma, 0.0f, 1.0f);

  if (forceRange == 1) {
    m_nB = m_pB = 0.9407f;
    m_nR = m_pR = 0.7373f;
  }
  else if (forceRange == 0) {
    //NB = ( 1 − KB )′	(52) 
    //PB = 1 − ( KB )′ 	(53)
    //NR = ( 1 − KR )′	(54)
    //PR = 1 − ( KR )′	(55)
    
    m_nB = m_chromaTF->inverse( 1.0 - m_transformY[B_COMP] );
    m_pB = 1.0 - m_chromaTF->inverse( m_transformY[B_COMP] );
    m_nR = m_chromaTF->inverse( 1.0 - m_transformY[R_COMP] );
    m_pR = 1.0 - m_chromaTF->inverse( m_transformY[R_COMP] );
  }
  else { // forceRange == 2
  // This is a special mode where basically everything is scaled (max of the two weights)
  // by the same value. This avoids the extra non linearity introduced
  // in the existing CL mode.
  
    m_nB = m_chromaTF->inverse( 1.0 - m_transformY[B_COMP] );
    m_pB = 1.0 - m_chromaTF->inverse( m_transformY[B_COMP] );
        
    m_nR = m_chromaTF->inverse( 1.0 - m_transformY[R_COMP] );
    m_pR = 1.0 - m_chromaTF->inverse( m_transformY[R_COMP] );
    
    m_nB = m_pB = dMax(m_nB, m_pB);
    m_nR = m_pR = dMax(m_nR, m_pR);
  }
}

ColorTransformCL::~ColorTransformCL() {
  delete m_lumaTF;
  delete m_chromaTF;
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ColorTransformCL::forward ( Frame* out, const Frame *inp) {
  double compY, compC;
  
  for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
    compY = m_transformY[R_COMP] * inp->m_floatComp[R_COMP][i] + m_transformY[G_COMP] * inp->m_floatComp[G_COMP][i] + m_transformY[B_COMP] * inp->m_floatComp[B_COMP][i];
    
    out->m_floatComp[Y_COMP][i] = (float) m_lumaTF->inverse( compY );
    
    if (m_closedLoop)
      compY = m_lumaTF->forward(dRound((double) out->m_floatComp[Y_COMP][i] * (219.0 * 4.0)) / (219.0 * 4.0));
    
    // CB component
    compC = m_chromaTF->inverse ( (double) inp->m_floatComp[B_COMP][i] ) - m_chromaTF->inverse( compY );
    if (compC <= 0.0)
      compC = (compC / (2.0 * m_nB) );
    else 
      compC = (compC / (2.0 * m_pB) );
    
    out->m_floatComp[Cb_COMP][i] = (float) compC;
    
    // CR component
    compC = m_chromaTF->inverse ( (double) inp->m_floatComp[R_COMP][i] ) - m_chromaTF->inverse( compY );
    if (compC <= 0.0)
      compC = (compC / (2.0 * m_nR) );
    else 
      compC = (compC / (2.0 * m_pR) );
    
    out->m_floatComp[Cr_COMP][i] = (float) compC;
    
    //printf("values %10.7f %10.7f %10.7f %10.7f %10.7f %10.7f \n", inp->m_floatComp[R_COMP][i] , inp->m_floatComp[G_COMP][i] , inp->m_floatComp[B_COMP][i] , out->m_floatComp[Y_COMP][i], out->m_floatComp[Cb_COMP][i], out->m_floatComp[Cr_COMP][i]);

  }
}

void ColorTransformCL::inverse ( Frame* out, const Frame *inp) {
  double compY, compC, compB, compR, compG;
  
  for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
    compY = m_lumaTF->forward( (double) inp->m_floatComp[Y_COMP][i] );
    /* 
     if ((double) inp->m_floatComp[Y_COMP][i] != dRound(((double) inp->m_floatComp[Y_COMP][i] * (219 * 4))) / (219 * 4))
     printf("value %20.18f %20.18f %20.18f  %20.18f\n", inp->m_floatComp[Y_COMP][i], dRound(((double) inp->m_floatComp[Y_COMP][i] * (219 * 4))) / (219 * 4), 
     (double) inp->m_floatComp[Y_COMP][i]- dRound((double) inp->m_floatComp[Y_COMP][i] * (219 * 4)) / (219 * 4), 
     dRound((dRound(((double) inp->m_floatComp[Y_COMP][i] * (219 * 4))) / (219 * 4)) * (219.0 * 4.0)) / (219.0 * 4.0));
     */
    // Blue component
    compC = (double) inp->m_floatComp[Cb_COMP][i];
    
#if (SAME_TF == 1)
    if (compC <= 0.0) {
      compB = m_chromaTF->forward( dClip(2.0 * m_nB * compC + inp->m_floatComp[Y_COMP][i], 0.0, 1.0 ) );
    }
    else {
      compB = m_chromaTF->forward( dClip(2.0 * m_pB * compC + inp->m_floatComp[Y_COMP][i], 0.0, 1.0 ) );
    }
#else
    if (compC <= 0.0) {
      compB = m_chromaTF->forward( dClip(2.0 * m_nB * compC + m_chromaTF->inverse(compY), 0.0, 1.0 ) );
    }
    else {
      compB = m_chromaTF->forward( dClip(2.0 * m_pB * compC + m_chromaTF->inverse(compY), 0.0, 1.0 ) );
    }
#endif
    compB = dClip(compB, 0.0, 1.0);
    
    // Red component
    compC = (double) inp->m_floatComp[Cr_COMP][i];
    
#if (SAME_TF == 1)
    if (compC <= 0.0) {
      compR = m_chromaTF->forward( dClip(2.0 * m_nR * compC + inp->m_floatComp[Y_COMP][i], 0.0, 1.0 ) );
    }
    else {
      compR = m_chromaTF->forward( dClip(2.0 * m_pR * compC + inp->m_floatComp[Y_COMP][i], 0.0, 1.0 ) );
    }
#else
    if (compC <= 0.0) {
      compR = m_chromaTF->forward( dClip(2.0 * m_nR * compC + m_chromaTF->inverse(compY), 0.0, 1.0 ) );
    }
    else {
      compR = m_chromaTF->forward( dClip(2.0 * m_pR * compC + m_chromaTF->inverse(compY), 0.0, 1.0 ) );
    }
#endif
    compR = dClip(compR, 0.0, 1.0);
    
    compG = ((compY - m_transformY[B_COMP] * compB - m_transformY[R_COMP] * compR) / m_transformY[G_COMP] );
    
    compG = dClip(compG, 0.0, 1.0);
    
    out->m_floatComp[R_COMP][i] = (float) compR;
    out->m_floatComp[G_COMP][i] = (float) compG;
    out->m_floatComp[B_COMP][i] = (float) compB;
    
    //printf("values %10.7f %10.7f %10.7f %10.7f %10.7f %10.7f \n", compR, compG, compB, inp->m_floatComp[Y_COMP][i], inp->m_floatComp[Cb_COMP][i], inp->m_floatComp[Cr_COMP][i]);
    
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
void ColorTransformCL::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  // Current condition to perform this is that Frames are of same size and in 4:4:4
  // Only supports floating point data
  
  if (inp->m_compSize[Y_COMP] == out->m_compSize[Y_COMP] && inp->m_compSize[Y_COMP] == inp->m_compSize[U_COMP])
  {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE)  {
      if (inp->m_colorSpace == CM_RGB) {
        forward(out, inp);
      }
      else {   
        inverse(out, inp);     
      }
    }
    else { // fixed precision, integer image data... currently not supported
      if (inp->m_bitDepth == out->m_bitDepth) {
        if (inp->m_bitDepth > 8) {
          for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
          }
        }
        else {
          for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
          }
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
