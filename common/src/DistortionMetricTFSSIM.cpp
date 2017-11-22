/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Apple Inc.
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
 * \file DistortionMetricTFSSIM.cpp
 *
 * \brief
 *    TF based MS-SSIM (Multi-Scale Structural Similarity) distortion computation Class
 *    Code is based on the JM implementation written by Dr. Peshala Pahalawatta
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricTFSSIM.H"
#include "ColorTransformGeneric.H"

#include <string.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricTFSSIM::DistortionMetricTFSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue)
: DistortionMetric()
{
  m_transferFunction   = DistortionTransferFunction::create(params->m_tfDistortion, params->m_tfLUTEnable);

  // Metric parameters
  m_K1 = params->m_K1;
  m_K2 = params->m_K2;
  
  m_blockDistance = params->m_blockDistance;
  m_blockSizeX    = params->m_blockSizeX;
  m_blockSizeY    = params->m_blockSizeY;
  m_useLogSSIM    = params->m_useLog;
  
#ifdef UNBIASED_VARIANCE
  m_bias = 1;
#else
  m_bias = 0;
#endif

  m_colorSpace    = format->m_colorSpace;
  
  m_memWidth = 0;
  m_memHeight = 0;
  m_dsWidth = 0;
  m_dsHeight = 0;
  
  
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
  }
  
  if (format->m_isFloat == FALSE) {
    fprintf(stderr, "TFSSIM computations only supported for floating point inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_chromaFormat != CF_444) {
    fprintf(stderr, "TFSSIM computations only supported for 4:4:4 inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_colorSpace != CM_RGB) {
    fprintf(stderr, "TFSSIM computations only supported for RGB inputs.\n");
    exit(EXIT_FAILURE);
  }

}

DistortionMetricTFSSIM::~DistortionMetricTFSSIM()
{
  if (m_transferFunction != NULL) {
    delete m_transferFunction;
    m_transferFunction = NULL;
  }
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
//Computes the product of the contrast and structure components of the structural similarity metric.
float DistortionMetricTFSSIM::computePlane (float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, float maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  double C1 = m_K1 * m_K1 * maxPixelValueSquared;
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
  double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
  double distortion = 0.0;
  int i, j, n, m, windowCounter = 0;
  float *inp0Line = NULL;
  float *inp1Line = NULL;
  
  float maxValue = 0.0;
  double inp0, inp1;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      double sumInp0 = 0.0f;
      double sumInp1 = 0.0f;
      double sumSquareInp0  = 0.0f;
      double sumSquareInp1  = 0.0f;
      double sumMultiInp0Inp1 = 0.0f;
      
      for ( n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          if (maxValue < *inp0Line)
            maxValue = *inp0Line;
          inp0              = (double) *inp0Line++;
          inp1              = (double) *inp1Line++;
          sumInp0          += inp0;
          sumInp1          += inp1;
          sumSquareInp0    += inp0 * inp0;
          sumSquareInp1    += inp1 * inp1;
          sumMultiInp0Inp1 += inp0 * inp1;
        }
      }
      
      meanInp0 = sumInp0 / windowPixels;
      meanInp1 = sumInp1 / windowPixels;

      varianceInp0 = (sumSquareInp0    - sumInp0 * meanInp0) / windowPixelsBias;
      varianceInp1 = (sumSquareInp1    - sumInp1 * meanInp1) / windowPixelsBias;
      covariance   = (sumMultiInp0Inp1 - sumInp0 * meanInp1) / windowPixelsBias;

      blockSSIM  = (2.0 * meanInp0 * meanInp1 + C1) * (2.0 * covariance + C2);
      blockSSIM /=  ((meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1) * (varianceInp0 + varianceInp1 + C2));
      
      distortion += blockSSIM;
      windowCounter++;

    }
  }

  distortion /= dMax(1.0, (double) windowCounter);
  
  if (distortion >= 1.0 && distortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0;
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return (float) distortion;
}




void DistortionMetricTFSSIM::setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2) {
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


void DistortionMetricTFSSIM::convertToXYZ(double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2) {
  
  xyz[0] = dClip( transform0[0] * rgb[R_COMP] + transform0[1] * rgb[G_COMP] + transform0[2] * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( transform1[0] * rgb[R_COMP] + transform1[1] * rgb[G_COMP] + transform1[2] * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( transform2[0] * rgb[R_COMP] + transform2[1] * rgb[G_COMP] + transform2[2] * rgb[B_COMP], 0, 1);
}

void DistortionMetricTFSSIM::compute(Frame* inp0, Frame* inp1)
{
  double rgb0Normal[3], xyz0Normal[3];
  double rgb1Normal[3], xyz1Normal[3];
  
  const double *transform00 = NULL;
  const double *transform01 = NULL;
  const double *transform02 = NULL;
  const double *transform10 = NULL;
  const double *transform11 = NULL;
  const double *transform12 = NULL;

  setColorConversion(inp0->m_colorPrimaries, &transform00, &transform01, &transform02);
  setColorConversion(inp1->m_colorPrimaries, &transform10, &transform11, &transform12);
  
  // allocate memory for Y data if not already allocated
  if (inp0->m_width[0] > m_memWidth || inp0->m_height[0] > m_memHeight ) {
    m_memWidth  = inp0->m_width[0];
    m_memHeight = inp0->m_height[0];
    
    m_dataY0.resize ( m_memWidth * m_memHeight );
    m_dataY1.resize ( m_memWidth * m_memHeight );
  }
    
  for (int i = 0; i < inp0->m_compSize[Y_COMP]; i++) {
    // TF inp0
    rgb0Normal[R_COMP] = inp0->m_floatComp[R_COMP][i] / m_maxValue[R_COMP];
    rgb0Normal[G_COMP] = inp0->m_floatComp[G_COMP][i] / m_maxValue[G_COMP];
    rgb0Normal[B_COMP] = inp0->m_floatComp[B_COMP][i] / m_maxValue[B_COMP];
    // TF inp1
    rgb1Normal[R_COMP] = inp1->m_floatComp[R_COMP][i] / m_maxValue[R_COMP];
    rgb1Normal[G_COMP] = inp1->m_floatComp[G_COMP][i] / m_maxValue[G_COMP];
    rgb1Normal[B_COMP] = inp1->m_floatComp[B_COMP][i] / m_maxValue[B_COMP];
        
    
    // Finally convert to XYZ
    convertToXYZ(rgb0Normal, xyz0Normal, transform00, transform01, transform02);
    convertToXYZ(rgb1Normal, xyz1Normal, transform10, transform11, transform12);
    // Y Component
    // TF inp0
    m_dataY0[i] = (float) m_transferFunction->performCompute( xyz0Normal[1] );
    // TF inp1
    m_dataY1[i] = (float) m_transferFunction->performCompute( xyz1Normal[1] );
  }  
  
  m_metric[0] = (double) computePlane(&m_dataY0[0], &m_dataY1[0], inp0->m_height[0], inp0->m_width[0], m_blockSizeY, m_blockSizeX, 1.0);
  m_metricStats[0].updateStats(m_metric[0]);

}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricTFSSIM::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      compute(inp0, inp1);
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


void DistortionMetricTFSSIM::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  // Currently no TF is applied on the data. However, we may wish to apply some TF, e.g. PQ or combination of PQ and PH.
  
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      compute(inp0, inp1);
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

void DistortionMetricTFSSIM::reportMetric  ()
{
  printf("%10.5f ", m_metric[Y_COMP]);
}

void DistortionMetricTFSSIM::reportSummary  ()
{
  printf("%10.5f ", m_metricStats[Y_COMP].getAverage());
}

void DistortionMetricTFSSIM::reportMinimum  ()
{
  printf("%10.5f ", m_metricStats[Y_COMP].minimum);
}

void DistortionMetricTFSSIM::reportMaximum  ()
{
  printf("%10.5f ", m_metricStats[Y_COMP].maximum);
}

void DistortionMetricTFSSIM::printHeader()
{
  if (m_isWindow == FALSE ) {
    printf(" tMSSSIM-Y "); // 11
  }
  else {
    printf("wtMSSSIM-Y "); // 11
  }
}

void DistortionMetricTFSSIM::printSeparator(){
  printf("-----------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
