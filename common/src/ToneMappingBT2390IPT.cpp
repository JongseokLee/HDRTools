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
 * \file ToneMappingBT2390IPT.cpp
 *
 * \brief
 *    ToneMappingBT2390IPT Class
 *    This process is based on the tone mapping method described in BT.2390
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Teun Baar
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "ColorTransformGeneric.H"
#include "ToneMappingBT2390IPT.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ToneMappingBT2390IPT::ToneMappingBT2390IPT(double minValue, double maxValue, double targetValue, bool scaleGammut) {
  m_scaleGammut = scaleGammut;
  m_maxValue    = maxValue;
  
  m_transferFunction = TransferFunction::create(TF_PQ, TRUE, 1.0, 1.0, 0.0, 1.0, TRUE);
  m_maxIntensity = m_transferFunction->inverse(targetValue / 10000.0);
  m_KS = 1.5 * m_maxIntensity - 0.5;
  m_KSIntensity = m_transferFunction->forward(m_KS);
  
}

ToneMappingBT2390IPT::~ToneMappingBT2390IPT() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ToneMappingBT2390IPT::convertToXYZ(double *rgb, double *xyz, const double transform[3][3]) {
  // Should we be clipping here? TBD
  xyz[0] = dClip( transform[0][0] * rgb[R_COMP] + transform[0][1] * rgb[G_COMP] + transform[0][2] * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( transform[1][0] * rgb[R_COMP] + transform[1][1] * rgb[G_COMP] + transform[1][2] * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( transform[2][0] * rgb[R_COMP] + transform[2][1] * rgb[G_COMP] + transform[2][2] * rgb[B_COMP], 0, 1);
}

void ToneMappingBT2390IPT::convertXYZToLMS(double *xyz, double *lms) {
    lms[0] =  0.4002 * xyz[0] + 0.7075 * xyz[1] - 0.0807 * xyz[2];
    lms[1] = -0.2280 * xyz[0] + 1.1500 * xyz[1] + 0.0612 * xyz[2];
    lms[2] =  0.9184 * xyz[2];
}

void ToneMappingBT2390IPT::convertLMSToIPT(double *lms, double *ipt) {
    ipt[0] = 0.4000 * lms[0] + 0.4000 * lms[1] + 0.2000 * lms[2];
    ipt[1] = 4.4550 * lms[0] - 4.8510 * lms[1] + 0.3960 * lms[2];
    ipt[2] = 0.8056 * lms[0] + 0.3572 * lms[1] - 1.1628 * lms[2];
}

void ToneMappingBT2390IPT::convertIPTToLMS(double *ipt, double *lms) {
    lms[0] =  1.0000 * ipt[0] + 0.0976 * ipt[1] + 0.2052 * ipt[2];
    lms[1] =  1.0000 * ipt[0] - 0.1139 * ipt[1] + 0.1332 * ipt[2];
    lms[2] =  1.0000 * ipt[0] + 0.0326 * ipt[1] - 0.6769 * ipt[2];
}


void ToneMappingBT2390IPT::convertLMSToXYZ(double *lms, double *xyz) {
    xyz[0] = 1.8502 * lms[0] - 1.1383 * lms[1] + 0.2384 * lms[2];
    xyz[1] = 0.3668 * lms[0] + 0.6439 * lms[1] - 0.0107 * lms[2];
    xyz[2] = 1.0889 * lms[2];
}

void ToneMappingBT2390IPT::convertToRGB(double *xyz, double *rgb, const double transform[3][3]) {
  rgb[0] = dClip( transform[0][0] * xyz[0] + transform[0][1] * xyz[1] + transform[0][2] * xyz[2], 0, 1);
  rgb[1] = dClip( transform[1][0] * xyz[0] + transform[1][1] * xyz[1] + transform[1][2] * xyz[2], 0, 1);
  rgb[2] = dClip( transform[2][0] * xyz[0] + transform[2][1] * xyz[1] + transform[2][2] * xyz[2], 0, 1);
}


void ToneMappingBT2390IPT::setColorConversion(int colorPrimaries, double transformFW[3][3], double transformBW[3][3]) {
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
  
  for (int i = Y_COMP; i <= V_COMP; i++){
    for (int j = 0; j < 3; j++) {
      transformFW[i][j] = FWD_TRANSFORM[mode][i][j];
      transformBW[i][j] = INV_TRANSFORM[mode][i][j];
    }
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ToneMappingBT2390IPT::process ( Frame* frame) {

 // It is assumed here that the content are linear RGB data
 // We thus need to convert the data back to XYZ, the xyY, process the luminance,
 // and then convert back the data to their original space.
  if (frame->m_isFloat == TRUE ) {
    double transformFW[3][3];
    double transformBW[3][3];
    double rgbNormal[3], xyzNormal[3];
    double rgbOutNormal[3], xyzOutNormal[3];
    double LMSin[3], LMSout[3];
    double IPTin[3], IPTout[3];
    double LpMpSpin[3], LpMpSpout[3];
    double E, t, p, tPower2, tPower3;
    double colourScale;
    setColorConversion(frame->m_colorPrimaries, transformFW, transformBW);
    
    for (int i = 0; i < frame->m_compSize[Y_COMP]; i++) {
      rgbNormal[R_COMP] = frame->m_floatComp[R_COMP][i] / m_maxValue;
      rgbNormal[G_COMP] = frame->m_floatComp[G_COMP][i] / m_maxValue;
      rgbNormal[B_COMP] = frame->m_floatComp[B_COMP][i] / m_maxValue;
      
      convertToXYZ(rgbNormal, xyzNormal, transformFW);
      convertXYZToLMS(xyzNormal, LMSin);
      LpMpSpin[0] = m_transferFunction->inverse(LMSin[0]);
      LpMpSpin[1] = m_transferFunction->inverse(LMSin[1]);
      LpMpSpin[2] = m_transferFunction->inverse(LMSin[2]);
      convertLMSToIPT(LpMpSpin, IPTin);
      
      //printf("values %10.7f %10.7f %d\n", IPTin[0], m_KS, m_scaleGammut);
      if (IPTin[0] >= m_KS) {
        E = IPTin[0];
        
        t = (E - m_KS) / ( 1 - m_KS);
        tPower2 = t * t;
        tPower3 = t * tPower2;
        p = (tPower3 - tPower2 - t + 1) * m_KS + (tPower3 - 2 * tPower2 + t) + (-2 * tPower3 + 3 * tPower2) * m_maxIntensity;

        colourScale = m_scaleGammut == FALSE ? 1.0 : dMin( p / E, E / p);
        //printf("values %10.7f %10.7f %d %10.7f\n", IPTin[0], m_KS, m_scaleGammut, colourScale);

        IPTout[0] = p;
        IPTout[1] = IPTin[1] * colourScale;
        IPTout[2] = IPTin[2] * colourScale;
        
        convertIPTToLMS(IPTout, LpMpSpout);
        
        LMSout[0] = m_transferFunction->forward(LpMpSpout[0]);
        LMSout[1] = m_transferFunction->forward(LpMpSpout[1]);
        LMSout[2] = m_transferFunction->forward(LpMpSpout[2]);
        
        convertLMSToXYZ(LMSout, xyzOutNormal);
        convertToRGB(xyzOutNormal, rgbOutNormal, transformBW);
        
        frame->m_floatComp[R_COMP][i] = (float) ( rgbOutNormal[R_COMP] * m_maxValue );
        frame->m_floatComp[G_COMP][i] = (float) ( rgbOutNormal[G_COMP] * m_maxValue );
        frame->m_floatComp[B_COMP][i] = (float) ( rgbOutNormal[B_COMP] * m_maxValue );
      }
      // else nothing needs to be done
    }
  }
}

void ToneMappingBT2390IPT::process ( Frame* out, const Frame *inp) {
  if (out == NULL) {
    // should only be done for pointers. This should help speeding up the code and reducing memory.
    out = (Frame *)inp;
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
    double transformFW[3][3];
    double transformBW[3][3];
    double rgbNormal[3], xyzNormal[3];
    double rgbOutNormal[3], xyzOutNormal[3];
    double LMSin[3], LMSout[3];
    double IPTin[3], IPTout[3];
    double LpMpSpin[3], LpMpSpout[3];
    double E, t, p, tPower2, tPower3;
    double colourScale;
    setColorConversion(inp->m_colorPrimaries, transformFW, transformBW);
    
      for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
        rgbNormal[R_COMP] = inp->m_floatComp[R_COMP][i] / m_maxValue;
        rgbNormal[G_COMP] = inp->m_floatComp[G_COMP][i] / m_maxValue;
        rgbNormal[B_COMP] = inp->m_floatComp[B_COMP][i] / m_maxValue;
        
        convertToXYZ(rgbNormal, xyzNormal, transformFW);
        convertXYZToLMS(xyzNormal, LMSin);
        LpMpSpin[0] = m_transferFunction->inverse(LMSin[0]);
        LpMpSpin[1] = m_transferFunction->inverse(LMSin[1]);
        LpMpSpin[2] = m_transferFunction->inverse(LMSin[2]);
        convertLMSToIPT(LpMpSpin, IPTin);
        
        if (IPTin[0] >= m_KS) {
          E = IPTin[0];
          
          t = (E - m_KS) / ( 1 - m_KS);
          tPower2 = t * t;
          tPower3 = t * tPower2;
          p = (tPower3 - tPower2 - t + 1) * m_KS + (tPower3 - 2 * tPower2 + t) + (-2 * tPower3 + 3 * tPower2) * m_maxIntensity;
          
          colourScale = m_scaleGammut == FALSE ? 1.0 : dMin( p / E, E / p);
          //printf("values %10.7f %10.7f %d %10.7f\n", IPTin[0], m_KS, m_scaleGammut, colourScale);
          
          IPTout[0] = p;
          IPTout[1] = IPTin[1] * colourScale;
          IPTout[2] = IPTin[2] * colourScale;
          
          convertIPTToLMS(IPTout, LpMpSpout);
          
          LMSout[0] = m_transferFunction->forward(LpMpSpout[0]);
          LMSout[1] = m_transferFunction->forward(LpMpSpout[1]);
          LMSout[2] = m_transferFunction->forward(LpMpSpout[2]);
          
          convertLMSToXYZ(LMSout, xyzOutNormal);
          convertToRGB(xyzOutNormal, rgbOutNormal, transformBW);
          
          out->m_floatComp[R_COMP][i] =  (float) ( rgbOutNormal[R_COMP] * m_maxValue );
          out->m_floatComp[G_COMP][i] =  (float) ( rgbOutNormal[G_COMP] * m_maxValue );
          out->m_floatComp[B_COMP][i] =  (float) ( rgbOutNormal[B_COMP] * m_maxValue );
        }
        else {
          out->m_floatComp[R_COMP][i] = inp->m_floatComp[R_COMP][i];
          out->m_floatComp[G_COMP][i] = inp->m_floatComp[G_COMP][i];
          out->m_floatComp[B_COMP][i] = inp->m_floatComp[B_COMP][i];
        }
      }
      // else nothing needs to be done
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
