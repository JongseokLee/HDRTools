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
 * \file DistortionMetricTFPSNR.cpp
 *
 * \brief
 *    PSNR distortion computation Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricTFPSNR.H"
#include "ColorTransformGeneric.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricTFPSNR::DistortionMetricTFPSNR(const FrameFormat *format, PSNRParams *params, double maxSampleValue)
: DistortionMetric()
{
  m_transferFunction   = DistortionTransferFunction::create(params->m_tfDistortion, params->m_tfLUTEnable);
  m_totalComponents    = TOTAL_COMPONENTS; // 3 for YCbCr, 3 for RGB, 3 for XYZ and three aggregators = 12
  m_colorSpace         = format->m_colorSpace;
  m_enableShowMSE      = params->m_enableShowMSE;
  m_computePsnrInRgb   = params->m_computePsnrInRgb;
  m_computePsnrInXYZ   = params->m_computePsnrInXYZ;
  m_computePsnrInYCbCr = params->m_computePsnrInYCbCr;
  m_computePsnrInYUpVp = params->m_computePsnrInYUpVp;
  
  for (int c = 0; c < m_totalComponents; c++) {
    m_mse[c]   = 0.0;
    m_sse[c]   = 0.0;
    m_psnr[c]  = 0.0;
    m_mseStats[c].reset();
    m_sseStats[c].reset();
    m_psnrStats[c].reset();
    
    m_maxValue[c] = (double) maxSampleValue;
  }
  
  if (format->m_isFloat == FALSE) {
    fprintf(stderr, "TFPSNR computations only supported for floating point inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_chromaFormat != CF_444) {
    fprintf(stderr, "TFPSNR computations only supported for 4:4:4 inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_colorSpace != CM_RGB) {
    fprintf(stderr, "TFPSNR computations only supported for RGB inputs.\n");
    exit(EXIT_FAILURE);
  }
}

DistortionMetricTFPSNR::~DistortionMetricTFPSNR()
{
  if (m_transferFunction != NULL) {
    delete m_transferFunction;
    m_transferFunction = NULL;
  }
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------


void DistortionMetricTFPSNR::convertToYCbCrBT2020(double *rgb, double *yCbCr) {
  // This should be replaced by native YCbCr diff 
  yCbCr[Y_COMP] = dClip( 0.262700 * rgb[R_COMP] + 0.678000 * rgb[G_COMP] + 0.059300 * rgb[B_COMP]      , 0, 1);
  // 0.5 needs to be added to chroma to bring them within the [0, 1] range
  yCbCr[U_COMP] = dClip(-0.139630 * rgb[R_COMP] - 0.360370 * rgb[G_COMP] + 0.500000 * rgb[B_COMP] + 0.5, 0, 1);
  yCbCr[V_COMP] = dClip( 0.500000 * rgb[R_COMP] - 0.459786 * rgb[G_COMP] - 0.040214 * rgb[B_COMP] + 0.5, 0, 1);
}

void DistortionMetricTFPSNR::convertToYCbCr(double *rgb, double *yCbCr, const double *transform0, const double *transform1, const double *transform2) {
  
  yCbCr[Y_COMP] = dClip( transform0[0] * rgb[R_COMP] + transform0[1] * rgb[G_COMP] + transform0[2] * rgb[B_COMP], 0, 1);
  yCbCr[U_COMP] = dClip( transform1[0] * rgb[R_COMP] + transform1[1] * rgb[G_COMP] + transform1[2] * rgb[B_COMP] + 0.5, 0, 1);
  yCbCr[V_COMP] = dClip( transform2[0] * rgb[R_COMP] + transform2[1] * rgb[G_COMP] + transform2[2] * rgb[B_COMP] + 0.5, 0, 1);
}


void DistortionMetricTFPSNR::convertToXYZ(double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2) {

  xyz[0] = dClip( transform0[0] * rgb[R_COMP] + transform0[1] * rgb[G_COMP] + transform0[2] * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( transform1[0] * rgb[R_COMP] + transform1[1] * rgb[G_COMP] + transform1[2] * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( transform2[0] * rgb[R_COMP] + transform2[1] * rgb[G_COMP] + transform2[2] * rgb[B_COMP], 0, 1);
}

void DistortionMetricTFPSNR::setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2) {
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

void DistortionMetricTFPSNR::setColorConversionYCbCr(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2) {
  int mode = CTF_IDENTITY;
  if (colorPrimaries == CP_709) {
    mode = CTF_RGB709_2_YUV709;
  }
  else if (colorPrimaries == CP_2020) {
    mode = CTF_RGB2020_2_YUV2020;
  }
  else if (colorPrimaries == CP_P3D65) {
    mode = CTF_RGBP3D65_2_YUVP3D65;
  }
  else if (colorPrimaries == CP_601) {
    mode = CTF_RGB601_2_YUV601;
  }
  
  *transform0 = FWD_TRANSFORM[mode][Y_COMP];
  *transform1 = FWD_TRANSFORM[mode][U_COMP];
  *transform2 = FWD_TRANSFORM[mode][V_COMP];
}

void DistortionMetricTFPSNR::compute(Frame* inp0, Frame* inp1)
{
  double rgb0Normal[3], xyz0Normal[3];
  double rgb1Normal[3], xyz1Normal[3];
  double rgb0Double[3], yCbCr0Double[3], xyz0Double[3];
  double rgb1Double[3], yCbCr1Double[3], xyz1Double[3];
  double YUpVp0Double[2], YUpVp1Double[2];
  double diff, diff2, diff2sum;
  
  const double *transform00 = NULL;
  const double *transform01 = NULL;
  const double *transform02 = NULL;
  const double *transform10 = NULL;
  const double *transform11 = NULL;
  const double *transform12 = NULL;
  const double *transformNCL0 = NULL;
  const double *transformNCL1 = NULL;
  const double *transformNCL2 = NULL;

  if (m_computePsnrInXYZ == TRUE || m_computePsnrInYUpVp == TRUE) {
    setColorConversion(inp0->m_colorPrimaries, &transform00, &transform01, &transform02);
    setColorConversion(inp1->m_colorPrimaries, &transform10, &transform11, &transform12);
  }
  if (m_computePsnrInYCbCr == TRUE) {
  // It is assumed that both inputs use the same primaries. Otherwise this would not make much sense
    setColorConversionYCbCr(inp0->m_colorPrimaries, &transformNCL0, &transformNCL1, &transformNCL2);
  }

  for (int c = 0; c < m_totalComponents; c++) {
    m_sse[c] = 0.0;
  }
  //printf("\n");

  for (int i = 0; i < inp0->m_compSize[Y_COMP]; i++) {
#if 0
    // TF inp0
    rgb0Normal[R_COMP] = dClip(inp0->m_floatComp[R_COMP][i] / m_maxValue[R_COMP], 0.0, 1.0);
    rgb0Normal[G_COMP] = dClip(inp0->m_floatComp[G_COMP][i] / m_maxValue[G_COMP], 0.0, 1.0);
    rgb0Normal[B_COMP] = dClip(inp0->m_floatComp[B_COMP][i] / m_maxValue[B_COMP], 0.0, 1.0);
    // TF inp1
    rgb1Normal[R_COMP] = dClip(inp1->m_floatComp[R_COMP][i] / m_maxValue[R_COMP], 0.0, 1.0);
    rgb1Normal[G_COMP] = dClip(inp1->m_floatComp[G_COMP][i] / m_maxValue[G_COMP], 0.0, 1.0);
    rgb1Normal[B_COMP] = dClip(inp1->m_floatComp[B_COMP][i] / m_maxValue[B_COMP], 0.0, 1.0);
#else
    // TF inp0
    rgb0Normal[R_COMP] = inp0->m_floatComp[R_COMP][i] / m_maxValue[R_COMP];
    rgb0Normal[G_COMP] = inp0->m_floatComp[G_COMP][i] / m_maxValue[G_COMP];
    rgb0Normal[B_COMP] = inp0->m_floatComp[B_COMP][i] / m_maxValue[B_COMP];
    // TF inp1
    rgb1Normal[R_COMP] = inp1->m_floatComp[R_COMP][i] / m_maxValue[R_COMP];
    rgb1Normal[G_COMP] = inp1->m_floatComp[G_COMP][i] / m_maxValue[G_COMP];
    rgb1Normal[B_COMP] = inp1->m_floatComp[B_COMP][i] / m_maxValue[B_COMP];
#endif    

    if (m_computePsnrInRgb == TRUE || m_computePsnrInYCbCr == TRUE) {
      rgb0Double[R_COMP] = m_transferFunction->performCompute( rgb0Normal[R_COMP] );
      rgb0Double[G_COMP] = m_transferFunction->performCompute( rgb0Normal[G_COMP] );
      rgb0Double[B_COMP] = m_transferFunction->performCompute( rgb0Normal[B_COMP] );
      // TF inp1
      rgb1Double[R_COMP] = m_transferFunction->performCompute( rgb1Normal[R_COMP] );
      rgb1Double[G_COMP] = m_transferFunction->performCompute( rgb1Normal[G_COMP] );
      rgb1Double[B_COMP] = m_transferFunction->performCompute( rgb1Normal[B_COMP] );
    }
    
    if (m_computePsnrInRgb == TRUE) {
      // compute RGB TF error
      diff = (rgb0Double[R_COMP] - rgb1Double[R_COMP]);
      m_sse[R_COMP + 4] += diff * diff;
      diff = (rgb0Double[G_COMP] - rgb1Double[G_COMP]);
      m_sse[G_COMP + 4] += diff * diff;
      diff = (rgb0Double[B_COMP] - rgb1Double[B_COMP]);
      m_sse[B_COMP + 4] += diff * diff;
    }
    
    if (m_computePsnrInYCbCr == TRUE) {
      // Now convert to Y'CbCr (non-constant luminance)
      //convertToYCbCrBT2020(rgb0Double, yCbCr0Double);
      //convertToYCbCrBT2020(rgb1Double, yCbCr1Double);
      convertToXYZ(rgb0Double, yCbCr0Double, transformNCL0, transformNCL1, transformNCL2);
      convertToXYZ(rgb1Double, yCbCr1Double, transformNCL0, transformNCL1, transformNCL2);
      
      diff = (yCbCr0Double[Y_COMP] - yCbCr1Double[Y_COMP]);
      m_sse[Y_COMP] += diff * diff;
      diff = (yCbCr0Double[U_COMP] - yCbCr1Double[U_COMP]);
      m_sse[U_COMP] += diff * diff;
      diff = (yCbCr0Double[V_COMP] - yCbCr1Double[V_COMP]);
      m_sse[V_COMP] += diff * diff;
    }
    
    // Finally convert to XYZ
    if ( m_computePsnrInXYZ == TRUE || m_computePsnrInYUpVp == TRUE )
    {
      convertToXYZ(rgb0Normal, xyz0Normal, transform00, transform01, transform02);
      convertToXYZ(rgb1Normal, xyz1Normal, transform10, transform11, transform12);
      // Y Component
      // TF inp0
      xyz0Double[1] = m_transferFunction->performCompute( xyz0Normal[1] );
      // TF inp1
      xyz1Double[1] = m_transferFunction->performCompute( xyz1Normal[1] );
      
      // Y component
      diff = (xyz0Double[1] - xyz1Double[1]);
      diff2 = diff * diff;
      diff2sum = diff2;
      m_sse[9] += diff2;
      
      if ( m_computePsnrInXYZ == TRUE ) {
        // TF inp0
        xyz0Double[0] = m_transferFunction->performCompute( xyz0Normal[0] );
        xyz0Double[2] = m_transferFunction->performCompute( xyz0Normal[2] );
        // TF inp1
        xyz1Double[0] = m_transferFunction->performCompute( xyz1Normal[0] );
        xyz1Double[2] = m_transferFunction->performCompute( xyz1Normal[2] );
        // X component
        diff = (xyz0Double[0] - xyz1Double[0]);
        diff2 = diff * diff;
        diff2sum += diff2;
        m_sse[8] += diff2;
        
        // Z component
        diff = (xyz0Double[2] - xyz1Double[2]);
        diff2 = diff * diff;
        diff2sum += diff2;
        m_sse[10] += diff2;
        m_sse[16] += sqrt(diff2sum);
      }
      
      if ( m_computePsnrInYUpVp == TRUE ) {
        double denom0 = (xyz0Normal[0] + 15.0 * xyz0Normal[1] + 3.0 * xyz0Normal[2]);
        if (denom0 == 0.0) {
          YUpVp0Double[0] = 0.197830013378341; // u' 
          YUpVp0Double[1] = 0.468319974939678; // v' 
        }
        else {
          double scale0 = 1.0 / denom0;
          YUpVp0Double[0] = dClip((4.0 * xyz0Normal[0] * scale0), 0.0, 1.0); // u' 
          YUpVp0Double[1] = dClip((9.0 * xyz0Normal[1] * scale0), 0.0, 1.0); // v' 
          
        }
        double denom1 = (xyz1Normal[0] + 15.0 * xyz1Normal[1] + 3.0 * xyz1Normal[2]);
      
        if (denom1 == 0.0) {
          YUpVp1Double[0] = 0.197830013378341; // u'
          YUpVp1Double[1] = 0.468319974939678; // v' 
        }
        else {
          double scale1 = 1.0 / denom1;    
          
          YUpVp1Double[0] = dClip((4.0 * xyz1Normal[0] * scale1), 0.0, 1.0); // u'
          YUpVp1Double[1] = dClip((9.0 * xyz1Normal[1] * scale1), 0.0, 1.0); // v' 
        }

        diff = (YUpVp0Double[0] - YUpVp1Double[0]);
        //printf("denom %10.6f %10.6f %10.6f %10.6f %10.6f", denom0, denom1, diff, YUpVp0Double[0], YUpVp1Double[0]);
        m_sse[12] += diff * diff;
        diff = (YUpVp0Double[1] - YUpVp1Double[1]);
        //printf(" %10.6f %10.6f %10.6f\n", diff, YUpVp0Double[0], YUpVp1Double[0]);

        m_sse[13] += diff * diff;
      }
    }
  }
  
  if (m_computePsnrInRgb == TRUE)
    m_sse[7] = (m_sse[R_COMP + 4] + m_sse[G_COMP + 4] + m_sse[B_COMP + 4]) / 3;
  if (m_computePsnrInYCbCr == TRUE)
    m_sse[3] = (m_sse[Y_COMP] + m_sse[U_COMP] + m_sse[V_COMP]) / 3;
  if (m_computePsnrInXYZ == TRUE)
    m_sse[11] = (m_sse[8] + m_sse[9] + m_sse[10]) / 3;
  if (m_computePsnrInYUpVp == TRUE)
    m_sse[14]= (m_sse[9] + m_sse[12] + m_sse[13]) / 3;

  for (int c = 0; c < m_totalComponents; c++) {
    m_sseStats[c].updateStats(m_sse[c]);
    m_mse[c] = (m_maxValue[c] * m_maxValue[c] * m_sse[c]) / (double) inp0->m_compSize[0];
    m_mseStats[c].updateStats(m_mse[c]);
    m_psnr[c] = psnr(1.0, inp0->m_compSize[0], m_sse[c]);
    if (c == 16)
      m_psnr[c] *= 2.0;
    m_psnrStats[c].updateStats(m_psnr[c]);
    
    //printf("%7.3f %7.3f %7.3f\n", m_mse[c], m_sse[c],m_psnr[c]);
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricTFPSNR::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      compute(inp0, inp1);
    }
    else {
      printf("Computation not supported for non floating point inputs.\n");
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricTFPSNR::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // Currently single component method not supported
}

void DistortionMetricTFPSNR::reportMetric  ()
{
  if (m_computePsnrInYCbCr == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnr[Y_COMP], m_psnr[U_COMP], m_psnr[V_COMP], m_psnr[3]);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mse[Y_COMP], m_mse[U_COMP], m_mse[V_COMP], m_mse[3]);
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnr[R_COMP + 4], m_psnr[G_COMP + 4], m_psnr[B_COMP + 4], m_psnr[7]);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mse[R_COMP + 4], m_mse[G_COMP + 4], m_mse[B_COMP + 4], m_mse[7]);
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f %10.3f ", m_psnr[8], m_psnr[9], m_psnr[10], m_psnr[11], m_psnr[16]);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f %10.3f ", m_mse[8], m_mse[9], m_mse[10], m_mse[11], m_mse[16]);
  }
  
  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnr[9], m_psnr[12], m_psnr[13], m_psnr[14]);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mse[9], m_mse[12], m_mse[13], m_mse[14]);
  }
}

void DistortionMetricTFPSNR::reportSummary  ()
{
  if (m_computePsnrInYCbCr == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[Y_COMP].getAverage(), m_psnrStats[U_COMP].getAverage(), m_psnrStats[V_COMP].getAverage(), m_psnrStats[3].getAverage());
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].getAverage(), m_mseStats[U_COMP].getAverage(), m_mseStats[V_COMP].getAverage(), m_mseStats[3].getAverage());
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[R_COMP + 4].getAverage(), m_psnrStats[G_COMP + 4].getAverage(), m_psnrStats[B_COMP + 4].getAverage(), m_psnrStats[7].getAverage());
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[R_COMP + 4].getAverage(), m_mseStats[G_COMP + 4].getAverage(), m_mseStats[B_COMP + 4].getAverage(), m_mseStats[7].getAverage());
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f %10.3f ", m_psnrStats[8].getAverage(), m_psnrStats[9].getAverage(), m_psnrStats[10].getAverage(), m_psnrStats[11].getAverage(), m_psnrStats[16].getAverage());
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f %10.3f ", m_mseStats[8].getAverage(), m_mseStats[9].getAverage(), m_mseStats[10].getAverage(), m_mseStats[11].getAverage(), m_mseStats[16].getAverage());
  }

  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[9].getAverage(), m_psnrStats[12].getAverage(), m_psnrStats[13].getAverage(), m_psnrStats[14].getAverage());
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[9].getAverage(), m_mseStats[12].getAverage(), m_mseStats[13].getAverage(), m_mseStats[14].getAverage());
  }
}

void DistortionMetricTFPSNR::reportMinimum  ()
{
  if (m_computePsnrInYCbCr == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[Y_COMP].minimum, m_psnrStats[U_COMP].minimum, m_psnrStats[V_COMP].minimum, m_psnrStats[3].minimum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].minimum, m_mseStats[U_COMP].minimum, m_mseStats[V_COMP].minimum, m_mseStats[3].minimum);
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[R_COMP + 4].minimum, m_psnrStats[G_COMP + 4].minimum, m_psnrStats[B_COMP + 4].minimum, m_psnrStats[7].minimum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[R_COMP + 4].minimum, m_mseStats[G_COMP + 4].minimum, m_mseStats[B_COMP + 4].minimum, m_mseStats[7].minimum);
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f %10.3f ", m_psnrStats[8].minimum, m_psnrStats[9].minimum, m_psnrStats[10].minimum, m_psnrStats[11].minimum, m_psnrStats[16].minimum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f %10.3f ", m_mseStats[8].minimum, m_mseStats[9].minimum, m_mseStats[10].minimum, m_mseStats[11].minimum, m_mseStats[16].minimum);
  }

  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[9].minimum, m_psnrStats[12].minimum, m_psnrStats[13].minimum, m_psnrStats[14].minimum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[9].minimum, m_mseStats[12].minimum, m_mseStats[13].minimum, m_mseStats[14].minimum);
  }
}

void DistortionMetricTFPSNR::reportMaximum  ()
{
  if (m_computePsnrInYCbCr == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[Y_COMP].maximum, m_psnrStats[U_COMP].maximum, m_psnrStats[V_COMP].maximum, m_psnrStats[3].maximum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].maximum, m_mseStats[U_COMP].maximum, m_mseStats[V_COMP].maximum, m_mseStats[3].maximum);
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[R_COMP + 4].maximum, m_psnrStats[G_COMP + 4].maximum, m_psnrStats[B_COMP + 4].maximum, m_psnrStats[7].maximum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[R_COMP + 4].maximum, m_mseStats[G_COMP + 4].maximum, m_mseStats[B_COMP + 4].maximum, m_mseStats[7].maximum);
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f %10.3f ", m_psnrStats[8].maximum, m_psnrStats[9].maximum, m_psnrStats[10].maximum, m_psnrStats[11].maximum, m_psnrStats[16].maximum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f %10.3f ", m_mseStats[8].maximum, m_mseStats[9].maximum, m_mseStats[10].maximum, m_mseStats[11].maximum, m_mseStats[16].maximum);
  }

  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    printf("%8.3f %8.3f %8.3f %10.3f ", m_psnrStats[9].maximum, m_psnrStats[12].maximum, m_psnrStats[13].maximum, m_psnrStats[14].maximum);
    if (m_enableShowMSE == TRUE)
      printf("%10.3f %10.3f %10.3f %10.3f ", m_mseStats[9].maximum, m_mseStats[12].maximum, m_mseStats[13].maximum, m_mseStats[14].maximum);
  }
}

void DistortionMetricTFPSNR::printHeader()
{
  if (m_isWindow == FALSE ) {
    if (m_computePsnrInYCbCr == TRUE) {
      printf("tPSNR-Y  "); // 9
      printf("tPSNR-U  "); // 9
      printf("tPSNR-V  "); // 9
      printf(" tPSNR-YUV "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  tMSE-Y   "); // 11
        printf("  tMSE-U   "); // 11
        printf("  tMSE-V   "); // 11
        printf(" tMSE-YUV  "); // 11
      }
    }
    if (m_computePsnrInRgb == TRUE) {
      printf("tPSNR-R  "); // 9
      printf("tPSNR-G  "); // 9
      printf("tPSNR-B  "); // 9
      printf(" tPSNR-RGB "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  tMSE-R   "); // 11
        printf("  tMSE-G   "); // 11
        printf("  tMSE-B   "); // 11
        printf(" tMSE-RGB  "); // 11
      }
    }
    if (m_computePsnrInXYZ == TRUE) {
      printf("tPSNR-X  "); // 9
      printf("tPSNR-Y  "); // 9
      printf("tPSNR-Z  "); // 9
      printf(" tPSNR-XYZ "); // 11
      printf(" tOSNR-XYZ "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  tMSE-X   "); // 11
        printf("  tMSE-Y   "); // 11
        printf("  tMSE-Z   "); // 11
        printf(" tMSE-XYZ  "); // 11
        printf(" tOSE-XYZ  "); // 11
      }
    }
    if (m_computePsnrInYUpVp == TRUE) {
      printf("tPSNR-Y  "); // 9
      printf(" PSNR-u' "); // 9
      printf(" PSNR-v' "); // 9
      printf(" tPSNR-Yuv "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  tMSE-Y   "); // 11
        printf("   MSE-u'  "); // 11
        printf("   MSE-v'  "); // 11
        printf(" tMSE-Yuv  "); // 11
      }
    }
  }
  else {
    if (m_computePsnrInYCbCr == TRUE) {
      printf("wtPSNR-Y "); // 9
      printf("wtPSNR-U "); // 9
      printf("wtPSNR-V "); // 9
      printf("wtPSNR-YUV "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  wtMSE-Y  "); // 11
        printf("  wtMSE-U  "); // 11
        printf("  wtMSE-V  "); // 11
        printf(" wtMSE-YUV "); // 11
      }
    }
    if (m_computePsnrInRgb == TRUE) {
      printf("wtPSNR-R "); // 9
      printf("wtPSNR-G "); // 9
      printf("wtPSNR-B "); // 9
      printf("wtPSNR-RGB "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  wtMSE-R  "); // 11
        printf("  wtMSE-G  "); // 11
        printf("  wtMSE-B  "); // 11
        printf(" wtMSE-RGB "); // 11
      }
    }
    if (m_computePsnrInXYZ == TRUE) {
      printf("wtPSNR-X "); // 9
      printf("wtPSNR-Y "); // 9
      printf("wtPSNR-Z "); // 9
      printf("wtPSNR-XYZ "); // 11
      printf("wtOSNR-XYZ "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  wtMSE-X  "); // 11
        printf("  wtMSE-Y  "); // 11
        printf("  wtMSE-Z  "); // 11
        printf(" wtMSE-XYZ "); // 11
        printf(" wtOSE-XYZ "); // 11
      }
    }
    if (m_computePsnrInYUpVp == TRUE) {
      printf("wtPSNR-Y "); // 9
      printf("wPSNR-u' "); // 9
      printf("wPSNR-v' "); // 9
      printf("wtPSNR-Yuv "); // 11
      if (m_enableShowMSE == TRUE) {
        printf("  wtMSE-Y  "); // 11
        printf("   wMSE-u' "); // 11
        printf("   wMSE-v' "); // 11
        printf(" wtMSE-Yuv "); // 11
      }
    }
  }
}

void DistortionMetricTFPSNR::printSeparator(){
  if (m_computePsnrInYCbCr == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("-----------");
    if (m_enableShowMSE == TRUE) {
      printf("-----------");
      printf("-----------");
      printf("-----------");
      printf("-----------");
    }
  }
  if (m_computePsnrInRgb == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("-----------");
    if (m_enableShowMSE == TRUE) {
      printf("-----------");
      printf("-----------");
      printf("-----------");
      printf("-----------");
    }
  }
  if (m_computePsnrInXYZ == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("-----------");
    printf("-----------");
    if (m_enableShowMSE == TRUE) {
      printf("-----------");
      printf("-----------");
      printf("-----------");
      printf("-----------");
      printf("-----------");
    }
  }
  if (m_computePsnrInYUpVp == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("-----------");
    if (m_enableShowMSE == TRUE) {
      printf("-----------");
      printf("-----------");
      printf("-----------");
      printf("-----------");
    }
  }
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
