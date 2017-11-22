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
 * \file DistortionMetricRegionTFPSNR.cpp
 *
 * \brief
 *    Region PSNR distortion computation Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricRegionTFPSNR.H"
#include "ColorTransformGeneric.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricRegionTFPSNR::DistortionMetricRegionTFPSNR(const FrameFormat *format, PSNRParams *params, double maxSampleValue)
 : DistortionMetric()
{
  m_transferFunction   = DistortionTransferFunction::create(params->m_tfDistortion, params->m_tfLUTEnable);
  m_totalComponents    = TOTAL_COMPONENTS; // 3 for YCbCr, 3 for RGB, 3 for XYZ and three aggregators = 12
  
  m_blockWidth    = params->m_rPSNRBlockSizeX;
  m_blockHeight   = params->m_rPSNRBlockSizeY;
  m_overlapWidth  = params->m_rPSNROverlapX;
  m_overlapHeight = params->m_rPSNROverlapY;
  m_width         = format->m_width[Y_COMP];
  m_height        = format->m_height[Y_COMP];
  
  m_computePsnrInRgb   = params->m_computePsnrInRgb;
  m_computePsnrInXYZ   = params->m_computePsnrInXYZ;
  m_computePsnrInYCbCr = params->m_computePsnrInYCbCr;
  m_computePsnrInYUpVp = params->m_computePsnrInYUpVp;
  
  m_diffData.resize  ( m_width * m_height );
   
  m_rgb0NormalData.resize  ( m_width * m_height * 3);
  m_rgb1NormalData.resize  ( m_width * m_height * 3);
  m_rgb0TFData.resize      ( m_width * m_height * 3);
  m_rgb1TFData.resize      ( m_width * m_height * 3);
  
  if (m_computePsnrInYCbCr == TRUE) {
    m_ycbcr0TFData.resize  ( m_width * m_height * 3);
    m_ycbcr1TFData.resize  ( m_width * m_height * 3);
  }
  if (m_computePsnrInXYZ == TRUE || m_computePsnrInYUpVp == TRUE) {
    m_xyz0TFData.resize      ( m_width * m_height * 3);
    m_xyz1TFData.resize      ( m_width * m_height * 3);
  }
  if (m_computePsnrInYUpVp == TRUE) {
    m_yupvp0Data.resize      ( m_width * m_height * 3);
    m_yupvp1Data.resize      ( m_width * m_height * 3);
  }

  m_colorSpace    = format->m_colorSpace;
  m_enableShowMSE = params->m_enableShowMSE;
  
  for (int c = 0; c < m_totalComponents; c++) {
    m_mse[c]  = 0.0;
    m_sse[c]  = 0.0;
    m_psnr[c] = 0.0;
    m_maxSSE [c] = 0.0;
    m_maxMSE [c] = 0.0;
    m_minSSE [c] = 0.0;
    m_minMSE [c] = 0.0;
    m_maxPSNR[c] = 0.0;
    m_minPSNR[c] = 0.0;
    m_maxPos [c][0] = 0;
    m_maxPos [c][1] = 0;
    m_minPos [c][0] = 0;
    m_minPos [c][1] = 0;
    m_mseStats[c].reset();
    m_sseStats[c].reset();
    m_psnrStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
    m_maxMSEStats[c].reset();
    m_minMSEStats[c].reset();
    m_maxPSNRStats[c].reset();
    m_minPSNRStats[c].reset();
  }
  
  if (format->m_isFloat == FALSE) {
    fprintf(stderr, "RTFPSNR computations only supported for floating point inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_chromaFormat != CF_444) {
    fprintf(stderr, "RTFPSNR computations only supported for 4:4:4 inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_colorSpace != CM_RGB) {
    fprintf(stderr, "RTFPSNR computations only supported for RGB inputs.\n");
    exit(EXIT_FAILURE);
  }
}

DistortionMetricRegionTFPSNR::~DistortionMetricRegionTFPSNR()
{
  if (m_transferFunction != NULL) {
    delete m_transferFunction;
    m_transferFunction = NULL;
  }
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------

double DistortionMetricRegionTFPSNR::applyTransferFunction(double value) {
  static const double m1 = (2610.0        ) / (4096 * 4);
  static const double m2 = (2523.0 * 128.0) / 4096;
  static const double c1 = (3424.0        ) / 4096;
  static const double c2 = (2413.0 *  32.0) / 4096;
  static const double c3 = (2392.0 *  32.0) / 4096;
  
  double tempValue = pow(dMax(0.0, dMin(value, 1.0)), m1);
  return (pow(((c2 *(tempValue) + c1)/(1 + c3 *(tempValue))), m2));
}

void DistortionMetricRegionTFPSNR::convertToYCbCrBT2020(double r, double g, double b, double *y, double *cb, double *cr) {
  /*  RGB to YUV BT.2020 (3)
   {  0.262700,   0.678000,   0.059300 },
   { -0.139630,  -0.360370,   0.500000 },
   {  0.500000,  -0.459786,  -0.040214 }
   */
  
  *y  = dClip( 0.262700 * r + 0.678000 * g + 0.059300 * b      , 0, 1);
  // 0.5 needs to be added to chroma to bring them within the [0, 1] range
  *cb = dClip(-0.139630 * r - 0.360370 * g + 0.500000 * b + 0.5, 0, 1);
  *cr = dClip( 0.500000 * r - 0.459786 * g - 0.040214 * b + 0.5, 0, 1);
}

void DistortionMetricRegionTFPSNR::convertToXYZ(double r, double g, double b, double *x, double *y, double *z) {
  /*   RGB to XYZ BT.2020 (12)
   { 0.636958, 0.144617, 0.168881 },
   { 0.262700, 0.677998, 0.059302 },
   { 0.000000, 0.028073, 1.060985 }
   */
  
  *x = dClip( 0.636958 * r + 0.144617 * g + 0.168881 * b, 0, 1);
  *y = dClip( 0.262700 * r + 0.677998 * g + 0.059302 * b, 0, 1);
  *x = dClip( 0.000000 * r + 0.028073 * g + 1.060985 * b, 0, 1);
}

void DistortionMetricRegionTFPSNR::convertToXYZ(double r, double g, double b, double *x, double *y, double *z, const double *transform0, const double *transform1, const double *transform2) {
  
  *x = dClip( transform0[0] * r + transform0[1] * g + transform0[2] * b, 0, 1);
  *y = dClip( transform1[0] * r + transform1[1] * g + transform1[2] * b, 0, 1);
  *z = dClip( transform2[0] * r + transform2[1] * g + transform2[2] * b, 0, 1);
}

void DistortionMetricRegionTFPSNR::convertToYCbCrBT2020(double *rgb, double *yCbCr) {
  /*  RGB to YUV BT.2020 (3)
   {  0.262700,   0.678000,   0.059300 },
   { -0.139630,  -0.360370,   0.500000 },
   {  0.500000,  -0.459786,  -0.040214 }
   */
  
  yCbCr[Y_COMP] = dClip( 0.262700 * rgb[R_COMP] + 0.678000 * rgb[G_COMP] + 0.059300 * rgb[B_COMP]      , 0, 1);
  // 0.5 needs to be added to chroma to bring them within the [0, 1] range
  yCbCr[U_COMP] = dClip(-0.139630 * rgb[R_COMP] - 0.360370 * rgb[G_COMP] + 0.500000 * rgb[B_COMP] + 0.5, 0, 1);
  yCbCr[V_COMP] = dClip( 0.500000 * rgb[R_COMP] - 0.459786 * rgb[G_COMP] - 0.040214 * rgb[B_COMP] + 0.5, 0, 1);
}

void DistortionMetricRegionTFPSNR::convertToXYZ(double *rgb, double *xyz) {
  /*   RGB to XYZ BT.2020 (12)
   { 0.636958, 0.144617, 0.168881 },
   { 0.262700, 0.677998, 0.059302 },
   { 0.000000, 0.028073, 1.060985 }
   */
  
  xyz[0] = dClip( 0.636958 * rgb[R_COMP] + 0.144617 * rgb[G_COMP] + 0.168881 * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( 0.262700 * rgb[R_COMP] + 0.677998 * rgb[G_COMP] + 0.059302 * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( 0.000000 * rgb[R_COMP] + 0.028073 * rgb[G_COMP] + 1.060985 * rgb[B_COMP], 0, 1);
}

void DistortionMetricRegionTFPSNR::convertToXYZ(double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2) {
  
  xyz[0] = dClip( transform0[0] * rgb[R_COMP] + transform0[1] * rgb[G_COMP] + transform0[2] * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( transform1[0] * rgb[R_COMP] + transform1[1] * rgb[G_COMP] + transform1[2] * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( transform2[0] * rgb[R_COMP] + transform2[1] * rgb[G_COMP] + transform2[2] * rgb[B_COMP], 0, 1);
}


void DistortionMetricRegionTFPSNR::setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2) {
  int mode = CTF_IDENTITY;
  if (colorPrimaries == CP_709) {
    mode = CTF_RGB709_2_XYZ;
  }
  else if (colorPrimaries == CP_2020) {
    mode = CTF_RGB2020_2_XYZ;
  }
  else if (colorPrimaries == CP_601) {
    mode = CTF_RGB601_2_XYZ;
  }
  else if (colorPrimaries == CP_P3D65) {
    mode = CTF_RGBP3D65_2_XYZ;
  }
  
  *transform0 = FWD_TRANSFORM[mode][Y_COMP];
  *transform1 = FWD_TRANSFORM[mode][U_COMP];
  *transform2 = FWD_TRANSFORM[mode][V_COMP];
}


double DistortionMetricRegionTFPSNR::compute(double *iComp0, double *iComp1, int width, int height, int component, double maxValue)
{
  double minSSE =  1e30;
  double maxSSE = -1e30;
  double sum = 0.0;
  double sumBlock;
  int    maxBest[2] = {0};
  int    minBest[2] = {0};
  int64  countBlocks = ZERO;  
  
  if ( width * height > m_width * m_height ) {
    m_width  = width;
    m_height = height;
    
    m_diffData.resize  ( m_width * m_height );
  }

  for (int i = 0; i < width * height; i++) {
    m_diffData[i] = iComp0[i] - iComp1[i];
  }


  for (int i = ZERO; i < height - m_blockHeight + 1; i += m_overlapHeight) {
    for (int j = ZERO; j < width - m_blockWidth + 1; j += m_overlapWidth) {
      sumBlock = ZERO;
      for (int k = i; k < i + m_blockHeight; k++) {
        for (int l = j; l < j + m_blockWidth; l++) {
          sumBlock += m_diffData[k * width + l] * m_diffData[k * width + l];
        }
      }
      
      if (minSSE > sumBlock) {
        minSSE = sumBlock;
        minBest[0] = i;
        minBest[1] = j;
      }
      if (maxSSE < sumBlock) {
        maxSSE = sumBlock;
        maxBest[0] = i;
        maxBest[1] = j;
      }              
      sum += sumBlock;
      countBlocks ++;
    }
  }
  
  m_maxSSE [component] = maxSSE;
  m_maxMSE [component] = maxSSE / (double) (m_blockHeight * m_blockWidth);
  m_minPSNR[component] = psnr(maxValue, m_blockHeight * m_blockWidth, maxSSE);


  m_minSSE [component] = minSSE;
  m_minMSE [component] = minSSE / (double) (m_blockHeight * m_blockWidth);;
  m_maxPSNR[component] = psnr(maxValue, m_blockHeight * m_blockWidth, minSSE);
  
  m_maxPos [component][0] = maxBest[0];
  m_maxPos [component][1] = maxBest[1];
  m_minPos [component][0] = minBest[0];
  m_minPos [component][1] = minBest[1];


  m_maxMSEStats [component].updateStats(m_maxMSE [component]);
  m_minMSEStats [component].updateStats(m_minMSE [component]);
  m_maxPSNRStats[component].updateStats(m_maxPSNR[component]);
  m_minPSNRStats[component].updateStats(m_minPSNR[component]);

  //printf("\nValue %9.3f %9.3f (%9.3f %9.3f) (%9.3f %9.3f) %9.3f %9.3f %9.3f %lld (%9.3f)\n",  maxSSE, minSSE, m_maxMSE [component], m_minMSE [component], m_minPSNR [component], m_maxPSNR [component], sum / (double) (countBlocks * m_blockHeight * m_blockWidth), sum,   (double) (countBlocks * m_blockHeight * m_blockWidth), countBlocks, psnr(maxValue, 1, sum / (double) (countBlocks * m_blockHeight * m_blockWidth)));
  return sum / (double) (countBlocks * m_blockHeight * m_blockWidth);
}

uint64 DistortionMetricRegionTFPSNR::compute(const uint16 *iComp0, const uint16 *iComp1, int size)
{
  int32  diff = 0;
  uint64 sum = 0;
  for (int i = 0; i < size; i++) {
    diff = (iComp0[i] - iComp1[i]);
    sum += diff * diff;
  }
  return sum;
}

uint64 DistortionMetricRegionTFPSNR::compute(const uint8 *iComp0, const uint8 *iComp1, int size)
{
  int32  diff = 0;
  uint64  sum = 0;
  for (int i = 0; i < size; i++) {
    diff = (iComp0[i] - iComp1[i]);
    sum += diff * diff;
  }
  return sum;
}

void DistortionMetricRegionTFPSNR::convert (Frame* inp, double *rgbNormalData,
                                            double *rgbTFData,
                                            double *xyzTFData,
                                            double *ycbcrTFData,
                                            double *yupvpData) {
  int planeSize = inp->m_compSize[Y_COMP];
  
  double rgbNormal[3], xyzNormal[3];
  double rgbDouble[3], yCbCrDouble[3], xyzDouble[3];
  double YUpVpDouble[2];
  const double *transform0 = NULL;
  const double *transform1 = NULL;
  const double *transform2 = NULL;

  if (m_computePsnrInXYZ == TRUE || m_computePsnrInYUpVp == TRUE) {
    setColorConversion(inp->m_colorPrimaries, &transform0, &transform1, &transform2);
  }

  
  for (int i = 0; i < planeSize; i++) {
    rgbNormalData[planeSize * 0 + i] = rgbNormal[R_COMP] = inp->m_floatComp[R_COMP][i] / m_maxValue[R_COMP];
    rgbNormalData[planeSize * 1 + i] = rgbNormal[G_COMP] = inp->m_floatComp[G_COMP][i] / m_maxValue[G_COMP];
    rgbNormalData[planeSize * 2 + i] = rgbNormal[B_COMP] = inp->m_floatComp[B_COMP][i] / m_maxValue[B_COMP];
    
    if (m_computePsnrInRgb == TRUE || m_computePsnrInYCbCr == TRUE) {
      rgbTFData[planeSize * 0 + i] = rgbDouble[R_COMP] = m_transferFunction->performCompute( rgbNormal[R_COMP] );
      rgbTFData[planeSize * 1 + i] = rgbDouble[G_COMP] = m_transferFunction->performCompute( rgbNormal[G_COMP] );
      rgbTFData[planeSize * 2 + i] = rgbDouble[B_COMP] = m_transferFunction->performCompute( rgbNormal[B_COMP] );
    }
    
    if (m_computePsnrInYCbCr == TRUE) {
      // Now convert to Y'CbCr (non-constant luminance)
      convertToYCbCrBT2020(rgbDouble, yCbCrDouble);
      ycbcrTFData[planeSize * 0 + i] = yCbCrDouble[0];
      ycbcrTFData[planeSize * 1 + i] = yCbCrDouble[1];
      ycbcrTFData[planeSize * 2 + i] = yCbCrDouble[2];
    }
    
    // Finally convert to XYZ
    if ( m_computePsnrInXYZ == TRUE || m_computePsnrInYUpVp == TRUE )
    {
      convertToXYZ(rgbNormal, xyzNormal, transform0, transform1, transform2);
      // Y Component
      xyzDouble[1] = m_transferFunction->performCompute( xyzNormal[1] );
      
      if ( m_computePsnrInXYZ == TRUE ) {
        xyzDouble[0] = m_transferFunction->performCompute( xyzNormal[0] );
        xyzDouble[2] = m_transferFunction->performCompute( xyzNormal[2] );
        xyzTFData[planeSize * 0 + i] = xyzDouble[0];
        xyzTFData[planeSize * 1 + i] = xyzDouble[1];
        xyzTFData[planeSize * 2 + i] = xyzDouble[2];
      }
      
      if ( m_computePsnrInYUpVp == TRUE ) {
        double denom = (xyzNormal[0] + 15.0 * xyzNormal[1] + 3.0 * xyzNormal[2]);
        if (denom == 0.0) {
          YUpVpDouble[0] = 0.197830013378341; // u' 
          YUpVpDouble[1] = 0.468319974939678; // v' 
        }
        else {
          double scale = 1.0 / denom;
          YUpVpDouble[0] = dClip((4.0 * xyzNormal[0] * scale), 0.0, 1.0); // u' 
          YUpVpDouble[1] = dClip((9.0 * xyzNormal[1] * scale), 0.0, 1.0); // v'           
        }
        yupvpData[planeSize * 0 + i] = xyzDouble[1];
        yupvpData[planeSize * 1 + i] = YUpVpDouble[0];
        yupvpData[planeSize * 2 + i] = YUpVpDouble[1];
      }
    }
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricRegionTFPSNR::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
    
      int width = inp0->m_width[Y_COMP];
      int height = inp0->m_height[Y_COMP];
      int curComp;
      
      // Prepare memory
      if ( width * height > m_width * m_height ) {
        m_width  = width;
        m_height = height;
        
        m_rgb0NormalData.resize  ( m_width * m_height * 3);
        m_rgb1NormalData.resize  ( m_width * m_height * 3);
        m_rgb0TFData.resize      ( m_width * m_height * 3);
        m_rgb1TFData.resize      ( m_width * m_height * 3);
        
        if (m_computePsnrInYCbCr == TRUE) {
          m_ycbcr0TFData.resize  ( m_width * m_height * 3);
          m_ycbcr1TFData.resize  ( m_width * m_height * 3);
        }
        if (m_computePsnrInXYZ == TRUE || m_computePsnrInYUpVp == TRUE) {
          m_xyz0TFData.resize      ( m_width * m_height * 3);
          m_xyz1TFData.resize      ( m_width * m_height * 3);
        }
        if (m_computePsnrInYUpVp == TRUE) {
          m_yupvp0Data.resize      ( m_width * m_height * 3);
          m_yupvp1Data.resize      ( m_width * m_height * 3);
        }
        
        m_diffData.resize  ( m_width * m_height );
      }
      
            
      convert (inp0, &m_rgb0NormalData[0], &m_rgb0TFData[0], &m_xyz0TFData[0], &m_ycbcr0TFData[0], &m_yupvp0Data[0]);
      convert (inp1, &m_rgb1NormalData[0], &m_rgb1TFData[0], &m_xyz1TFData[0], &m_ycbcr1TFData[0], &m_yupvp1Data[0]);
      
      if (m_computePsnrInYCbCr == TRUE) {
        for (int c = Y_COMP; c <= V_COMP; c++) {
          m_mse        [c] = compute(&m_ycbcr0TFData[c * width * height], &m_ycbcr1TFData[c * width * height], width, height, c, 1.0);
          m_mseStats   [c].updateStats(m_mse[c]);
          m_psnr       [c] = psnr(1.0, 1, m_mse[c]);
          m_psnrStats  [c].updateStats(m_psnr[c]);
        }
      }
      if (m_computePsnrInRgb == TRUE) {
        for (int c = R_COMP; c <= G_COMP; c++) {
          curComp = c + 4;
          m_mse        [curComp] = compute(&m_rgb0TFData[c * width * height], &m_rgb1TFData[c * width * height], width, height, curComp, 1.0);
          m_mseStats   [curComp].updateStats(m_mse[curComp]);
          m_psnr       [curComp] = psnr(1.0, 1, m_mse[curComp]);
          m_psnrStats  [curComp].updateStats(m_psnr[curComp]);
        }
      }
      if (m_computePsnrInXYZ == TRUE) {
        for (int c = 0; c <= 2; c++) {
          curComp = c + 8;
          m_mse        [curComp] = compute(&m_xyz0TFData[c * width * height], &m_xyz1TFData[c * width * height], width, height, curComp, 1.0);
          m_mseStats   [curComp].updateStats(m_mse[curComp]);
          m_psnr       [curComp] = psnr(1.0, 1, m_mse[curComp]);
          m_psnrStats  [curComp].updateStats(m_psnr[curComp]);
        }
      }
      if (m_computePsnrInYUpVp == TRUE) {
        for (int c = 0; c <= 2; c++) {
          curComp = c + 12;
          m_mse        [curComp] = compute(&m_yupvp0Data[c * width * height], &m_yupvp1Data[c * width * height], width, height, curComp, 1.0);
          m_mseStats   [curComp].updateStats(m_mse[curComp]);
          m_psnr       [curComp] = psnr(1.0, 1, m_mse[curComp]);
          m_psnrStats  [curComp].updateStats(m_psnr[curComp]);
        }
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
    }
    else { // 16 bit data
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}
void DistortionMetricRegionTFPSNR::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
    }
    else { // 16 bit data
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricRegionTFPSNR::reportMetric  ()
{
  int pos;
  if (m_computePsnrInYCbCr == TRUE) {
    pos = 0; // Y_COMP;
    printf("%8.3f %8.3f %8.3f ", m_psnr[pos], m_psnr[pos + 1], m_psnr[pos + 2]);
    printf("%8.3f %8.3f %8.3f ", m_minPSNR[pos], m_minPSNR[pos + 1], m_minPSNR[pos + 2]);
    printf("%5d %5d ", m_maxPos[pos][0], m_maxPos[pos][1]);
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    pos = 4;
    printf("%8.3f %8.3f %8.3f ", m_psnr[pos], m_psnr[pos + 1], m_psnr[pos + 2]);
    printf("%8.3f %8.3f %8.3f ", m_minPSNR[pos], m_minPSNR[pos + 1], m_minPSNR[pos + 2]);
    printf("%5d %5d ", m_maxPos[pos + 1][0], m_maxPos[pos + 1][1]);
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    pos = 8;
    printf("%8.3f %8.3f %8.3f ", m_psnr[pos], m_psnr[pos + 1], m_psnr[pos + 2]);
    printf("%8.3f %8.3f %8.3f ", m_minPSNR[pos], m_minPSNR[pos + 1], m_minPSNR[pos + 2]);
    printf("%5d %5d ", m_maxPos[pos + 1][0], m_maxPos[pos + 1][1]);
  }
  
  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    pos = 12;
    printf("%8.3f %8.3f %8.3f ", m_psnr[pos], m_psnr[pos + 1], m_psnr[pos + 2]);
    printf("%8.3f %8.3f %8.3f ", m_minPSNR[pos], m_minPSNR[pos + 1], m_minPSNR[pos + 2]);
    printf("%5d %5d ", m_maxPos[pos][0], m_maxPos[pos][1]);
  }
  
  //printf("%5d %5d ", m_maxPos[Y_COMP][0], m_maxPos[Y_COMP][1]);
}

void DistortionMetricRegionTFPSNR::reportSummary  ()
{
  
  if (m_computePsnrInYCbCr == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[Y_COMP].getAverage(), m_psnrStats[U_COMP].getAverage(), m_psnrStats[V_COMP].getAverage());
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[Y_COMP].getAverage(), m_minPSNRStats[U_COMP].getAverage(), m_minPSNRStats[V_COMP].getAverage());
    printf("            ");
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[R_COMP + 4].getAverage(), m_psnrStats[G_COMP + 4].getAverage(), m_psnrStats[B_COMP + 4].getAverage());
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[R_COMP + 4].getAverage(), m_minPSNRStats[G_COMP + 4].getAverage(), m_minPSNRStats[B_COMP + 4].getAverage());
    printf("            ");
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[8].getAverage(), m_psnrStats[9].getAverage(), m_psnrStats[10].getAverage());
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[8].getAverage(), m_minPSNRStats[9].getAverage(), m_minPSNRStats[10].getAverage());
    printf("            ");
  }
  
  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[12].getAverage(), m_psnrStats[13].getAverage(), m_psnrStats[14].getAverage());
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[12].getAverage(), m_minPSNRStats[13].getAverage(), m_minPSNRStats[14].getAverage());
    printf("            ");
  }
}

void DistortionMetricRegionTFPSNR::reportMinimum  ()
{
  if (m_computePsnrInYCbCr == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[Y_COMP].minimum, m_psnrStats[U_COMP].minimum, m_psnrStats[V_COMP].minimum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[Y_COMP].minimum, m_minPSNRStats[U_COMP].minimum, m_minPSNRStats[V_COMP].minimum);
    printf("            ");
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[R_COMP + 4].minimum, m_psnrStats[G_COMP + 4].minimum, m_psnrStats[B_COMP + 4].minimum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[R_COMP + 4].minimum, m_minPSNRStats[G_COMP + 4].minimum, m_minPSNRStats[B_COMP + 4].minimum);
    printf("            ");
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[8].minimum, m_psnrStats[9].minimum, m_psnrStats[10].minimum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[8].minimum, m_minPSNRStats[9].minimum, m_minPSNRStats[10].minimum);
    printf("            ");
  }
  
  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[12].minimum, m_psnrStats[13].minimum, m_psnrStats[14].minimum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[12].minimum, m_minPSNRStats[13].minimum, m_minPSNRStats[14].minimum);
    printf("            ");
  }
}

void DistortionMetricRegionTFPSNR::reportMaximum  ()
{
  if (m_computePsnrInYCbCr == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[Y_COMP].maximum, m_psnrStats[U_COMP].maximum, m_psnrStats[V_COMP].maximum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[Y_COMP].maximum, m_minPSNRStats[U_COMP].maximum, m_minPSNRStats[V_COMP].maximum);
    printf("            ");
  }
  
  // RGB values
  if (m_computePsnrInRgb == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[R_COMP + 4].maximum, m_psnrStats[G_COMP + 4].maximum, m_psnrStats[B_COMP + 4].maximum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[R_COMP + 4].maximum, m_minPSNRStats[G_COMP + 4].maximum, m_minPSNRStats[B_COMP + 4].maximum);
    printf("            ");
  }
  
  // XYZ values
  if (m_computePsnrInXYZ == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[8].maximum, m_psnrStats[9].maximum, m_psnrStats[10].maximum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[8].maximum, m_minPSNRStats[9].maximum, m_minPSNRStats[10].maximum);
    printf("            ");
  }
  
  // YUpVp values
  if (m_computePsnrInYUpVp == TRUE) {
    printf("%8.3f %8.3f %8.3f ", m_psnrStats[12].maximum, m_psnrStats[13].maximum, m_psnrStats[14].maximum);
    printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[12].maximum, m_minPSNRStats[13].maximum, m_minPSNRStats[14].maximum);
    printf("            ");
  }
}

void DistortionMetricRegionTFPSNR::printHeader()
{
  if (m_isWindow == FALSE ) {
    if (m_computePsnrInYCbCr == TRUE) {
      printf("rtPSNR-Y "); // 9
      printf("rtPSNR-U "); // 9
      printf("rtPSNR-V "); // 9
      printf("dtPSNR-Y "); // 9
      printf("dtPSNR-U "); // 9
      printf("dtPSNR-V "); // 9
      printf(" mY-x ");
      printf(" mY-y ");
    }
    if (m_computePsnrInRgb == TRUE) {
      printf("rtPSNR-R "); // 9
      printf("rtPSNR-G "); // 9
      printf("rtPSNR-B "); // 9
      printf("dtPSNR-R "); // 9
      printf("dtPSNR-G "); // 9
      printf("dtPSNR-B "); // 9
      printf(" mG-x ");
      printf(" mG-y ");
    }
    if (m_computePsnrInXYZ == TRUE) {
      printf("rtPSNR-X "); // 9
      printf("rtPSNR-Y "); // 9
      printf("rtPSNR-Z "); // 9
      printf("dtPSNR-X "); // 9
      printf("dtPSNR-Y "); // 9
      printf("dtPSNR-Z "); // 9
      printf(" mY-x ");
      printf(" mY-y ");
    }
    if (m_computePsnrInYUpVp == TRUE) {
      printf("rtPSNR-Y "); // 9
      printf("rPSNR-u' "); // 9
      printf("rPSNR-v' "); // 9
      printf("dtPSNR-Y "); // 9
      printf("dPSNR-u' "); // 9
      printf("dPSNR-v' "); // 9
      printf(" mY-x ");
      printf(" mY-y ");
    }
  }
  else {
    if (m_computePsnrInYCbCr == TRUE) {
      printf("wrtSNR-Y "); // 9
      printf("wrtSNR-U "); // 9
      printf("wrtSNR-V "); // 9
      printf("wdtSNR-Y "); // 9
      printf("wdtSNR-U "); // 9
      printf("wdtSNR-V "); // 9
      printf(" mY-x ");
      printf(" mY-y ");
    }
    if (m_computePsnrInRgb == TRUE) {
      printf("wrtSNR-R "); // 9
      printf("wrtSNR-G "); // 9
      printf("wrtSNR-B "); // 9
      printf("wdtSNR-R "); // 9
      printf("wdtSNR-G "); // 9
      printf("wdtSNR-B "); // 9
      printf(" mG-x ");
      printf(" mG-y ");
    }
    if (m_computePsnrInXYZ == TRUE) {
      printf("wrtSNR-X "); // 9
      printf("wrtSNR-Y "); // 9
      printf("wrtSNR-Z "); // 9
      printf("wdtSNR-X "); // 9
      printf("wdtSNR-Y "); // 9
      printf("wdtSNR-Z "); // 9
      printf(" mY-x ");
      printf(" mY-y ");
    }
    if (m_computePsnrInYUpVp == TRUE) {
      printf("wrtSNR-Y "); // 9
      printf("wrSNR-u' "); // 9
      printf("wrSNR-v' "); // 9
      printf("wdtSNR-Y "); // 9
      printf("wdSNR-u' "); // 9
      printf("wdSNR-v' "); // 9
      printf(" mY-x ");
      printf(" mY-y ");
    }
  }
}

void DistortionMetricRegionTFPSNR::printSeparator(){
  if (m_computePsnrInYCbCr == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("------------");
  }
  if (m_computePsnrInRgb == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("------------");
  }
  if (m_computePsnrInXYZ == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("------------");
  }
  if (m_computePsnrInYUpVp == TRUE) {
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("---------");
    printf("------------");
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
