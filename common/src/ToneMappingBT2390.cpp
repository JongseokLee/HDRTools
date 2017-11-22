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
 * \file ToneMappingBT2390.cpp
 *
 * \brief
 *    ToneMappingBT2390 Class
 *    This process is based on the tone mapping method described in BT.2390
 *    This module uses the ICtCp color space for performing the conversion.
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
#include "ToneMappingBT2390.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ToneMappingBT2390::ToneMappingBT2390(double minValue, double maxValue, double targetValue, bool scaleGammut) {
  m_scaleGammut = scaleGammut;
  m_minValue    = minValue;
  m_maxValue    = maxValue;
  
  m_transferFunction = TransferFunction::create(TF_PQ, TRUE, 1.0, 1.0, 0.0, 1.0, TRUE);
  m_minIntensity = m_transferFunction->inverse(m_minValue  / 10000.0);
  m_maxIntensity = m_transferFunction->inverse(targetValue / 10000.0);
  m_KS = 1.5 * m_maxIntensity - 0.5;
  m_KSIntensity = m_transferFunction->forward(m_KS);
  
}

ToneMappingBT2390::~ToneMappingBT2390() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ToneMappingBT2390::convertToXYZ(double *rgb, double *xyz, const double transform[3][3]) {
  // Should we be clipping here? TBD
  xyz[0] = dClip( transform[0][0] * rgb[R_COMP] + transform[0][1] * rgb[G_COMP] + transform[0][2] * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( transform[1][0] * rgb[R_COMP] + transform[1][1] * rgb[G_COMP] + transform[1][2] * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( transform[2][0] * rgb[R_COMP] + transform[2][1] * rgb[G_COMP] + transform[2][2] * rgb[B_COMP], 0, 1);
}

void ToneMappingBT2390::convertXYZToLMS(double *xyz, double *lms) {
    lms[0] =  0.359283259012122 * xyz[0] + 0.697605114777950 * xyz[1] - 0.035891593232029 * xyz[2];
    lms[1] = -0.192080846370499 * xyz[0] + 1.100476797037432 * xyz[1] + 0.075374865851912 * xyz[2];
    lms[2] =  0.007079784460748 * xyz[0] + 0.074839666218637 * xyz[1] + 0.843326545389877 * xyz[2];
}

void ToneMappingBT2390::convertLMSToICtCp(double *lms, double *ictcp) {
    ictcp[0] = 0.5000 * lms[0] + 0.5000 * lms[1];
    ictcp[1] = 1.613769531 * lms[0] - 3.323486328 * lms[1] + 1.709716797 * lms[2];
    ictcp[2] = 4.378173828 * lms[0] - 4.245605469 * lms[1] - 0.132568359 * lms[2];
}

void ToneMappingBT2390::convertICtCpToLMS(double *ictcp, double *lms) {
    lms[0] =  1.0000 * ictcp[0] + 0.008609037037933 * ictcp[1] + 0.111029625003026 * ictcp[2];
    lms[1] =  1.0000 * ictcp[0] - 0.008609037037933 * ictcp[1] - 0.111029625003026 * ictcp[2];
    lms[2] =  1.0000 * ictcp[0] + 0.560031335710679 * ictcp[1] - 0.320627174987319 * ictcp[2];
}


void ToneMappingBT2390::convertLMSToXYZ(double *lms, double *xyz) {
    xyz[0] =  2.070152218389423 * lms[0] - 1.326347338967156 * lms[1] + 0.206651047629405 * lms[2];
    xyz[1] =  0.364738520974807 * lms[0] + 0.680566024947228 * lms[1] - 0.045304545922035 * lms[2];
    xyz[2] = -0.049747207535812 * lms[0] - 0.049260966696614 * lms[1] + 1.188065924992303 * lms[2];
}

void ToneMappingBT2390::convertToRGB(double *xyz, double *rgb, const double transform[3][3]) {
  rgb[0] = dClip( transform[0][0] * xyz[0] + transform[0][1] * xyz[1] + transform[0][2] * xyz[2], 0, 1);
  rgb[1] = dClip( transform[1][0] * xyz[0] + transform[1][1] * xyz[1] + transform[1][2] * xyz[2], 0, 1);
  rgb[2] = dClip( transform[2][0] * xyz[0] + transform[2][1] * xyz[1] + transform[2][2] * xyz[2], 0, 1);
}


void ToneMappingBT2390::setColorConversion(int colorPrimaries, double transformFW[3][3], double transformBW[3][3]) {
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

void ToneMappingBT2390::process ( Frame* frame) {

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
      convertLMSToICtCp(LpMpSpin, IPTin);
      
      if (IPTin[0] >= m_KS) {
        E = IPTin[0];
        
        t = (E - m_KS) / ( 1 - m_KS);
        tPower2 = t * t;
        tPower3 = t * tPower2;
        p = (tPower3 - tPower2 - t + 1) * m_KS + (tPower3 - 2 * tPower2 + t) + (-2 * tPower3 + 3 * tPower2) * m_maxIntensity;
        
        if ( m_minIntensity > 0.0 )
         p = p + m_minIntensity * pow( 1.0 - p, 4.0);
        
        colourScale = m_scaleGammut == FALSE ? 1.0 : dMin( p / E, E / p);

        IPTout[0] = p;
        IPTout[1] = IPTin[1] * colourScale;
        IPTout[2] = IPTin[2] * colourScale;
        
        convertICtCpToLMS(IPTout, LpMpSpout);
        
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

void ToneMappingBT2390::process ( Frame* out, const Frame *inp) {
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
        convertLMSToICtCp(LpMpSpin, IPTin);
        
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
          
          convertICtCpToLMS(IPTout, LpMpSpout);
          
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
