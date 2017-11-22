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
 * \file DistortionMetricTFMSSSIM.cpp
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

#include "DistortionMetricTFMSSSIM.H"
#include "ColorTransformGeneric.H"

#include <string.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define MS_SSIM_PAD  6
#define MS_SSIM_PAD2 3

static const double kMsSSIMBeta0 = 0.0448;
static const double kMsSSIMBeta1 = 0.2856;
static const double kMsSSIMBeta2 = 0.3001;
static const double kMsSSIMBeta3 = 0.2363;
static const double kMsSSIMBeta4 = 0.1333;

#define MAX_SSIM_LEVELS 5

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricTFMSSSIM::DistortionMetricTFMSSSIM(const FrameFormat *format, SSIMParams *params, double maxSampleValue)
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
  m_exponent[0] = (float) kMsSSIMBeta0;
  m_exponent[1] = (float) kMsSSIMBeta1;
  m_exponent[2] = (float) kMsSSIMBeta2;
  m_exponent[3] = (float) kMsSSIMBeta3;
  m_exponent[4] = (float) kMsSSIMBeta4;
  
  m_memWidth = 0;
  m_memHeight = 0;
  m_dsWidth = 0;
  m_dsHeight = 0;
  
  m_tempWidth = 0;
  m_tempHeight = 0;
  m_destWidth = 0;
  m_destHeight = 0;
  
  m_maxSSIMLevelsMinusOne = MAX_SSIM_LEVELS - 1;
    
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
  }
  
  if (format->m_isFloat == FALSE) {
    fprintf(stderr, "TFMSSSIM computations only supported for floating point inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_chromaFormat != CF_444) {
    fprintf(stderr, "TFMSSSIM computations only supported for 4:4:4 inputs.\n");
    exit(EXIT_FAILURE);
  }
  if (format->m_colorSpace != CM_RGB) {
    fprintf(stderr, "TFMSSSIM computations only supported for RGB inputs.\n");
    exit(EXIT_FAILURE);
  }

}

DistortionMetricTFMSSSIM::~DistortionMetricTFMSSSIM()
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
void DistortionMetricTFMSSSIM::computeComponents (float *lumaCost, float *structCost, float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, float maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  double C1 = m_K1 * m_K1 * maxPixelValueSquared;
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
  double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
  double structDistortion = 0.0;
  double lumaDistortion = 0.0;
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

      blockSSIM  = (2.0 * meanInp0 * meanInp1 + C1) / (meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1);
      
      lumaDistortion += blockSSIM;

      varianceInp0 = (sumSquareInp0    - sumInp0 * meanInp0) / windowPixelsBias;
      varianceInp1 = (sumSquareInp1    - sumInp1 * meanInp1) / windowPixelsBias;
      covariance   = (sumMultiInp0Inp1 - sumInp0 * meanInp1) / windowPixelsBias;
      
      blockSSIM  = (2.0 * covariance + C2) /(varianceInp0 + varianceInp1 + C2);
      
      structDistortion += blockSSIM;
      windowCounter++;
    }
  }

  lumaDistortion /= dMax(1.0, (double) windowCounter);
  
  if (lumaDistortion >= 1.0 && lumaDistortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    lumaDistortion = 1.0;
  
  *lumaCost = (float) lumaDistortion;
  
  structDistortion /= dMax(1.0, (double) windowCounter);
  
  if (structDistortion >= 1.0 && structDistortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    structDistortion = 1.0;
  
  *structCost = (float) structDistortion;

}

float DistortionMetricTFMSSSIM::computeStructuralComponents (float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, float maxPixelValue)
{
  double maxPixelValueSquared = (double) (maxPixelValue * maxPixelValue);
  double C2 = m_K2 * m_K2 * maxPixelValueSquared;
  double windowPixels = (double) (windowWidth * windowHeight);
  double windowPixelsBias = windowPixels - m_bias;
  
  double distortion = 0.0;
  int    windowCounter = 0;
  float *inp0Line = NULL;
  float *inp1Line = NULL;
  
  float maxValue = 0.0;
  double inp0, inp1;
  
  for (int j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (int i = 0; i <= width - windowWidth; i += m_blockDistance) {
      double blockSSIM = 0.0, meanInp0 = 0.0, meanInp1 = 0.0;
      double varianceInp0 = 0.0, varianceInp1 = 0.0, covariance = 0.0;
      double sumInp0 = 0.0;
      double sumInp1 = 0.0;
      double sumSquareInp0  = 0.0;
      double sumSquareInp1  = 0.0;
      double sumMultiInp0Inp1 = 0.0;
      
      for ( int n = j; n < j + windowHeight;n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (int m = ZERO; m < windowWidth; m++) {
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
      
      blockSSIM  = (2.0 * covariance + C2) /(varianceInp0 + varianceInp1 + C2);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
    
  distortion /= dMax(1.0, (double) windowCounter);
    
  if (distortion >= 1.0 && distortion < 1.01) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0;
  
  return (float) distortion;
}

float DistortionMetricTFMSSSIM::computeLuminanceComponent (float *inp0Data, float *inp1Data, int height, int width, int windowHeight, int windowWidth, float maxPixelValue)
{
  float maxPixelValueSquared = (float) (maxPixelValue * maxPixelValue);
  float C1 = (float) (m_K1 * m_K1 * maxPixelValueSquared);
  float windowPixels = (float) (windowWidth * windowHeight);
  float blockSSIM, meanInp0, meanInp1;
  float sumInp0, sumInp1;
  float distortion = 0.0f;
  int i, j, n, m, windowCounter = 0;
  
  float *inp0Line = NULL;
  float *inp1Line = NULL;
  
  for (j = 0; j <= height - windowHeight; j += m_blockDistance) {
    for (i = 0; i <= width - windowWidth; i += m_blockDistance) {
      sumInp0 = 0.0f;
      sumInp1 = 0.0f;
      
      for ( n = j; n < j + windowHeight; n++) {
        inp0Line = &inp0Data[n * width + i];
        inp1Line = &inp1Data[n * width + i];
        for (m = ZERO; m < windowWidth; m++) {
          sumInp0   += *inp0Line++;
          sumInp1   += *inp1Line++;
        }
      }
      
      meanInp0 = sumInp0 / (float) windowPixels;
      meanInp1 = sumInp1 / (float) windowPixels;
      
      blockSSIM  = (2.0f * meanInp0 * meanInp1 + C1);
      blockSSIM /= (meanInp0 * meanInp0 + meanInp1 * meanInp1 + C1);
      
      distortion += blockSSIM;
      windowCounter++;
    }
  }
  
  distortion /= (float) windowCounter;
  
  if (distortion >= 1.0f && distortion < 1.01f) // avoid float accuracy problem at very low QP(e.g.2)
    distortion = 1.0f;
  
  return distortion;
}

void DistortionMetricTFMSSSIM::horizontalSymmetricExtension(float *buffer, int width, int height )
{
  int j;
  float* buf;
  
  int heightPlusPad2 = height + MS_SSIM_PAD2;
  int widthMinusOne  = width - 1;
  
  // horizontal PSE
  for (j = MS_SSIM_PAD2 * (width + 1); j < heightPlusPad2 * width ; j+= width ) {
    // left column
    buf     = buffer + j;
    buf[-1] = buf[1];
    buf[-2] = buf[2];
    
    // right column
    buf   += widthMinusOne;
    buf[1] = buf[-1];
    buf[2] = buf[-2];
    buf[3] = buf[-3];
  }
}

void DistortionMetricTFMSSSIM::verticalSymmetricExtension(float *buffer, int width, int height)
{
  memcpy(&buffer[(MS_SSIM_PAD2 - 3) * width], &buffer[(MS_SSIM_PAD2 + 3) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(MS_SSIM_PAD2 - 2) * width], &buffer[(MS_SSIM_PAD2 + 2) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(MS_SSIM_PAD2 - 1) * width], &buffer[(MS_SSIM_PAD2 + 1) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(height + MS_SSIM_PAD2  ) * width], &buffer[(height + MS_SSIM_PAD2-2) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(height + MS_SSIM_PAD2+1) * width], &buffer[(height + MS_SSIM_PAD2-3) * width], (width + MS_SSIM_PAD) * sizeof(float));
  memcpy(&buffer[(height + MS_SSIM_PAD2+2) * width], &buffer[(height + MS_SSIM_PAD2-4) * width], (width + MS_SSIM_PAD) * sizeof(float));
}

void DistortionMetricTFMSSSIM::padImage(const float* src, float *buffer, int width, int height)
{
  int i, j;
  float* tmpDst = buffer + (MS_SSIM_PAD2 * width);
  const float* tmpSrc;
  
  for (j = 0; j < height; j++) {
    tmpSrc = src + j * width;
    tmpDst = &buffer[(j + MS_SSIM_PAD2) * width + MS_SSIM_PAD2];
    for (i = 0; i < width; i++) {
      tmpDst[i] = tmpSrc[i];
    }
  }
}

void DistortionMetricTFMSSSIM::downsample(const float* src, float* out, int iWidth, int iHeight, int oWidth, int oHeight)
{
  int i, j;
  int ii, jj;
  float *tmpDst, *tmpSrc;
  
  vector<float> itemp((iHeight + MS_SSIM_PAD) *( iWidth + MS_SSIM_PAD));
  vector<float> dest ((iHeight + MS_SSIM_PAD) *( oWidth + MS_SSIM_PAD));

/*
  if (iWidth + MS_SSIM_PAD > m_tempWidth || iHeight + MS_SSIM_PAD > m_tempHeight ) {
    m_tempWidth  = iWidth + MS_SSIM_PAD;
    m_tempHeight = iHeight + MS_SSIM_PAD;
    
    m_temp.resize ( m_tempWidth * m_tempHeight );
  }
  if (oWidth + MS_SSIM_PAD > m_destWidth || iHeight + MS_SSIM_PAD > m_destHeight ) {
    m_destWidth  = oWidth + MS_SSIM_PAD;
    m_destHeight = iHeight + MS_SSIM_PAD;
    
    m_dest.resize ( m_destWidth * m_destHeight );
  }
*/
  float *p_destM1, *p_destM2, *p_destM0, *p_destP1, *p_destP2, *p_destP3;
  float *pOut;
  static const float downSampleFilter[3] = { 1.0f/64.0f, 3.0f/64.0f, 28.0f/64.0f};
  
  padImage(src, &itemp[0], iWidth, iHeight);
  horizontalSymmetricExtension(&itemp[0], iWidth, iHeight);
  
  for (j = MS_SSIM_PAD2; j < iHeight + MS_SSIM_PAD2; j++) {
    tmpDst = &dest [j * oWidth + MS_SSIM_PAD2];
    tmpSrc = &itemp[j * iWidth + MS_SSIM_PAD2];
    for (i = 0; i < oWidth; i++) {
      ii = (i << 1);
      tmpDst[i]  =
      downSampleFilter[0] * (tmpSrc[ii - 2] + tmpSrc[ii + 3]) +
      downSampleFilter[1] * (tmpSrc[ii - 1] + tmpSrc[ii + 2]) +
      downSampleFilter[2] * (tmpSrc[ii    ] + tmpSrc[ii + 1]);
    }
  }
  
  //Periodic extension
  verticalSymmetricExtension(&dest[0], oWidth, iHeight);
  
  for (j = 0; j < oHeight; j++)  {
    jj = (j << 1) + 1;
    p_destM2 = &dest[jj * oWidth + MS_SSIM_PAD2];
    p_destM1 = p_destM2 + oWidth;
    p_destM0 = p_destM1 + oWidth;
    p_destP1 = p_destM0 + oWidth;
    p_destP2 = p_destP1 + oWidth;
    p_destP3 = p_destP2 + oWidth;
    pOut    = out + j * oWidth;
    for (i = 0; i < oWidth; i++)   {
      pOut[i]  =
      downSampleFilter[0] * (p_destM2[i] + p_destP3[i]) +
      downSampleFilter[1] * (p_destM1[i] + p_destP2[i]) +
      downSampleFilter[2] * (p_destM0[i] + p_destP1[i]);
    }
  }
}


void DistortionMetricTFMSSSIM::setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2) {
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


void DistortionMetricTFMSSSIM::convertToXYZ(double *rgb, double *xyz, const double *transform0, const double *transform1, const double *transform2) {
  
  xyz[0] = dClip( transform0[0] * rgb[R_COMP] + transform0[1] * rgb[G_COMP] + transform0[2] * rgb[B_COMP], 0, 1);
  xyz[1] = dClip( transform1[0] * rgb[R_COMP] + transform1[1] * rgb[G_COMP] + transform1[2] * rgb[B_COMP], 0, 1);
  xyz[2] = dClip( transform2[0] * rgb[R_COMP] + transform2[1] * rgb[G_COMP] + transform2[2] * rgb[B_COMP], 0, 1);
}


float DistortionMetricTFMSSSIM::computePlane(float *inp0Data, float *inp1Data, int height, int width, float maxPixelValue)
{
  float structural[MAX_SSIM_LEVELS];
  float luminance;
  int   dsWidth  = width >> 1;
  int   dsHeight = height >> 1;
  vector<float>  dsRef(dsHeight * dsWidth);
  vector<float>  dsEnc(dsHeight * dsWidth);
  
  structural[0] = computeStructuralComponents(inp0Data, inp1Data, height, width, m_blockSizeY, m_blockSizeX, maxPixelValue);
  float distortion = (float)pow(structural[0], m_exponent[0]);
  
  downsample(inp0Data, &dsRef[0], width, height, dsWidth, dsHeight);
  downsample(inp1Data, &dsEnc[0], width, height, dsWidth, dsHeight);
  
  for (int m = 1; m < MAX_SSIM_LEVELS; m++)  {
    width  = dsWidth;
    height = dsHeight;
    dsWidth  = width >> 1;
    dsHeight = height >> 1;
    
    structural[m] = computeStructuralComponents(&dsRef[0], &dsEnc[0], height, width, iMin(m_blockSizeY,height), iMin(m_blockSizeX,width), maxPixelValue);
    distortion *= (float) pow(structural[m], m_exponent[m]);
    
    if (m < m_maxSSIMLevelsMinusOne)  {
      downsample(&dsRef[0], &dsRef[0], width, height, dsWidth, dsHeight);
      downsample(&dsEnc[0], &dsEnc[0], width, height, dsWidth, dsHeight);
    }
    else  {
      luminance = computeLuminanceComponent(&dsRef[0], &dsEnc[0], height, width, iMin(m_blockSizeY,height), iMin(m_blockSizeX,width), maxPixelValue);
      distortion *= (float)pow(luminance, m_exponent[m]);
    }
  }
  
  if (m_useLogSSIM)
    return (float) ssimSNR(distortion);
  else
    return distortion;
}

void DistortionMetricTFMSSSIM::compute(Frame* inp0, Frame* inp1)
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
  
  m_metric[0] = (double) computePlane(&m_dataY0[0], &m_dataY1[0], inp0->m_height[0], inp0->m_width[0], 1.0);
  m_metricStats[0].updateStats(m_metric[0]);

}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricTFMSSSIM::computeMetric (Frame* inp0, Frame* inp1)
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


void DistortionMetricTFMSSSIM::computeMetric (Frame* inp0, Frame* inp1, int component)
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

void DistortionMetricTFMSSSIM::reportMetric  ()
{
  printf("%10.5f ", m_metric[Y_COMP]);
}

void DistortionMetricTFMSSSIM::reportSummary  ()
{
  printf("%10.5f ", m_metricStats[Y_COMP].getAverage());
}

void DistortionMetricTFMSSSIM::reportMinimum  ()
{
  printf("%10.5f ", m_metricStats[Y_COMP].minimum);
}

void DistortionMetricTFMSSSIM::reportMaximum  ()
{
  printf("%10.5f ", m_metricStats[Y_COMP].maximum);
}

void DistortionMetricTFMSSSIM::printHeader()
{
  if (m_isWindow == FALSE ) {
    printf(" tMSSSIM-Y "); // 11
  }
  else {
    printf("wtMSSSIM-Y "); // 11
  }
}

void DistortionMetricTFMSSSIM::printSeparator(){
  printf("-----------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
