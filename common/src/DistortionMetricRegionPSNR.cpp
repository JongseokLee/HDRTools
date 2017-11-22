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
 * \file DistortionMetricRegionPSNR.cpp
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

#include "DistortionMetricRegionPSNR.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricRegionPSNR::DistortionMetricRegionPSNR(const FrameFormat *format, PSNRParams *params, double maxSampleValue)
 : DistortionMetric()
{
  m_blockWidth    = params->m_rPSNRBlockSizeX;
  m_blockHeight   = params->m_rPSNRBlockSizeY;
  m_overlapWidth  = params->m_rPSNROverlapX;
  m_overlapHeight = params->m_rPSNROverlapY;
  m_width         = format->m_width[Y_COMP];
  m_height        = format->m_height[Y_COMP];
  
  m_diffData.resize  ( m_width * m_height );

  m_colorSpace    = format->m_colorSpace;
  m_enableShowMSE = params->m_enableShowMSE;
  for (int c = 0; c < T_COMP; c++) {
    m_mse[c] = 0.0;
    m_sse[c] = 0.0;
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
    m_maxValue[c] = (double) maxSampleValue;
    m_maxMSEStats[c].reset();
    m_minMSEStats[c].reset();
    m_maxPSNRStats[c].reset();
    m_minPSNRStats[c].reset();
  }
}

DistortionMetricRegionPSNR::~DistortionMetricRegionPSNR()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
double DistortionMetricRegionPSNR::compute(const float *iComp0, const float *iComp1, int width, int height, int component, double maxValue)
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

  if (m_clipInputValues) { // Add clipping of distortion values as per Jacob's request
    for (int i = 0; i < width * height; i++) {
      m_diffData[i] = (dMin(iComp0[i], maxValue) - dMin(iComp1[i], maxValue));
    }
  }
  else {
    for (int i = 0; i < width * height; i++) {
      m_diffData[i] = (iComp0[i] - iComp1[i]);
    }
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

  return sum / (double) (countBlocks * m_blockHeight * m_blockWidth);
}

uint64 DistortionMetricRegionPSNR::compute(const uint16 *iComp0, const uint16 *iComp1, int width, int height, int component, int maxValue)
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
    m_diffData[i] = (double) (iComp0[i] - iComp1[i]);
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

  return (uint64) (sum / (double) (countBlocks * m_blockHeight * m_blockWidth));
}

uint64 DistortionMetricRegionPSNR::compute(const uint8 *iComp0, const uint8 *iComp1, int width, int height, int component, int maxValue)
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
    m_diffData[i] = (double) (iComp0[i] - iComp1[i]);
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

  //printf("Value %9.3f %9.3f %9.3f %9.3f %9.3f %lld\n",  maxSSE, minSSE, sum / (double) (countBlocks * m_blockHeight * m_blockWidth), sum,   (double) (countBlocks * m_blockHeight * m_blockWidth), countBlocks);
  return (uint64) (sum / (double) (countBlocks * m_blockHeight * m_blockWidth));
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricRegionPSNR::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_mse[c] = compute(inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_width[c], inp0->m_height[c], c, m_maxValue[c]);
        m_mseStats[c].updateStats(m_mse[c]);
        m_metric[c] = psnr(m_maxValue[c], 1, m_mse[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_mse[c] = (double) compute(inp0->m_comp[c], inp1->m_comp[c], inp0->m_width[c], inp0->m_height[c], c, (int) m_maxValue[c]);
        m_mseStats[c].updateStats(m_mse[c]);
        m_metric[c] = psnr(m_maxValue[c], 1, m_mse[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else { // 16 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        m_mse[c] = (double) compute(inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_width[c], inp0->m_height[c], c, (int) m_maxValue[c]);
        m_mseStats[c].updateStats(m_mse[c]);
        m_metric[c] = psnr(m_maxValue[c], 1, m_mse[c]);
        m_metricStats[c].updateStats(m_metric[c]);
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}
void DistortionMetricRegionPSNR::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
        m_mse[component] = compute(inp0->m_floatComp[component], inp1->m_floatComp[component], inp0->m_width[component], inp0->m_height[component], component, m_maxValue[component]);
        m_mseStats[component].updateStats(m_mse[component]);
        m_metric[component] = psnr(m_maxValue[component], 1, m_mse[component]);
        m_metricStats[component].updateStats(m_metric[component]);
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
        m_mse[component] = (double) compute(inp0->m_comp[component], inp1->m_comp[component], inp0->m_width[component], inp0->m_height[component], component, (int) m_maxValue[component]);
        m_mseStats[component].updateStats(m_mse[component]);
        m_metric[component] = psnr(m_maxValue[component], 1, m_mse[component]);
        m_metricStats[component].updateStats(m_metric[component]);
    }
    else { // 16 bit data
        m_mse[component] = (double) compute(inp0->m_ui16Comp[component], inp1->m_ui16Comp[component], inp0->m_width[component], inp0->m_height[component], component, (int) m_maxValue[component]);
        m_mseStats[component].updateStats(m_mse[component]);
        m_metric[component] = psnr(m_maxValue[component], 1, m_mse[component]);
        m_metricStats[component].updateStats(m_metric[component]);
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricRegionPSNR::reportMetric  ()
{
  printf("%8.3f %8.3f %8.3f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);  
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mse[Y_COMP], m_mse[U_COMP], m_mse[V_COMP]);
  printf("%8.3f %8.3f %8.3f ", m_minPSNR[Y_COMP], m_minPSNR[U_COMP], m_minPSNR[V_COMP]);
  printf("%5d %5d ", m_maxPos[Y_COMP][0], m_maxPos[Y_COMP][1]);
}

void DistortionMetricRegionPSNR::reportSummary  ()
{
  printf("%8.3f %8.3f %8.3f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].getAverage(), m_mseStats[U_COMP].getAverage(), m_mseStats[V_COMP].getAverage());
  printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[Y_COMP].getAverage(), m_minPSNRStats[U_COMP].getAverage(), m_minPSNRStats[V_COMP].getAverage());
  printf("            ");
}

void DistortionMetricRegionPSNR::reportMinimum  ()
{
  printf("%8.3f %8.3f %8.3f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].minimum, m_mseStats[U_COMP].minimum, m_mseStats[V_COMP].minimum);
  printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[Y_COMP].minimum, m_minPSNRStats[U_COMP].minimum, m_minPSNRStats[V_COMP].minimum);
  printf("            ");
}

void DistortionMetricRegionPSNR::reportMaximum  ()
{
  printf("%8.3f %8.3f %8.3f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
  if (m_enableShowMSE == TRUE)
    printf("%10.3f %10.3f %10.3f ", m_mseStats[Y_COMP].maximum, m_mseStats[U_COMP].maximum, m_mseStats[V_COMP].maximum);
  printf("%8.3f %8.3f %8.3f ", m_minPSNRStats[Y_COMP].maximum, m_minPSNRStats[U_COMP].maximum, m_minPSNRStats[V_COMP].maximum);
  printf("            ");
}

void DistortionMetricRegionPSNR::printHeader()
{
  if (m_isWindow == FALSE ) {
  switch (m_colorSpace) {
    case CM_YCbCr:
      printf("rPSNR-Y  "); // 9
      printf("rPSNR-U  "); // 9
      printf("rPSNR-V  "); // 9
      if (m_enableShowMSE == TRUE) {
        printf("  rMSE-Y   "); // 11
        printf("  rMSE-U   "); // 11
        printf("  rMSE-V   "); // 11
      }
      printf("bPSNR-Y  "); // 9
      printf("bPSNR-U  "); // 9
      printf("bPSNR-V  "); // 9
      break;
    case CM_RGB:
      printf("rPSNR-R  "); // 9
      printf("rPSNR-G  "); // 9
      printf("rPSNR-B  "); // 9
      if (m_enableShowMSE == TRUE) {
        printf("  rMSE-R   "); // 11
        printf("  rMSE-G   "); // 11
        printf("  rMSE-B   "); // 11
      }
      printf("bPSNR-R  "); // 9
      printf("bPSNR-G  "); // 9
      printf("bPSNR-B  "); // 9
      break;
    case CM_XYZ:
      printf("rPSNR-X  "); // 9
      printf("rPSNR-Y  "); // 9
      printf("rPSNR-Z  "); // 9
      if (m_enableShowMSE == TRUE) {
        printf("  rMSE-X   "); // 11
        printf("  rMSE-Y   "); // 11
        printf("  rMSE-Z   "); // 11
      }
      printf("bPSNR-X  "); // 9
      printf("bPSNR-Y  "); // 9
      printf("bPSNR-Z  "); // 9
      break;
    default:
      printf("rPSNRC0  "); // 9
      printf("rPSNRC1  "); // 9
      printf("rPSNRC2  "); // 9
      if (m_enableShowMSE == TRUE) {
        printf("  rMSE-C0  "); // 11
        printf("  rMSE-C1  "); // 11
        printf("  rMSE-C2  "); // 11
      }
      printf("bPSNRC0  "); // 9
      printf("bPSNRC1  "); // 9
      printf("bPSNRC2  "); // 9
      break;
  }
    printf("X-Pos "); // 6
    printf("Y-Pos "); // 6
}
  else {
    switch (m_colorSpace) {
      case CM_YCbCr:
        printf("wrPSNR-Y "); // 9
        printf("wrPSNR-U "); // 9
        printf("wrPSNR-V "); // 9
        if (m_enableShowMSE == TRUE) {
          printf("  wrMSE-Y  "); // 11
          printf("  wrMSE-U  "); // 11
          printf("  wrMSE-V  "); // 11
        }
        break;
      case CM_RGB:
        printf("wrPSNR-R "); // 9
        printf("wrPSNR-G "); // 9
        printf("wrPSNR-B "); // 9
        if (m_enableShowMSE == TRUE) {
          printf("  wrMSE-R  "); // 11
          printf("  wrMSE-G  "); // 11
          printf("  wrMSE-B  "); // 11
        }
        break;
      case CM_XYZ:
        printf("wrPSNR-X "); // 9
        printf("wrPSNR-Y "); // 9
        printf("wrPSNR-Z "); // 9
        if (m_enableShowMSE == TRUE) {
          printf("  wrMSE-X  "); // 11
          printf("  wrMSE-Y  "); // 11
          printf("  wrMSE-Z  "); // 11
        }
        break;
      default:
        printf("wrPSNRC0 "); // 9
        printf("wrPSNRC1 "); // 9
        printf("wrPSNRC2 "); // 9
        if (m_enableShowMSE == TRUE) {
          printf("  wrMSE-C0 "); // 11
          printf("  wrMSE-C1 "); // 11
          printf("  wrMSE-C2 "); // 11
        }
        break;
    }
    printf("wbPSNR   "); // 9
    printf("wbPSNR   "); // 9
    printf("wbPSNR   "); // 9
    printf("wXPos "); // 6
    printf("wYPos "); // 6
  }
}

void DistortionMetricRegionPSNR::printSeparator(){
  printf("---------");
  printf("---------");
  printf("---------");
  if (m_enableShowMSE == TRUE) {
    printf("-----------");
    printf("-----------");
    printf("-----------");
  }
  printf("---------");
  printf("---------");
  printf("---------");
  printf("------------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
