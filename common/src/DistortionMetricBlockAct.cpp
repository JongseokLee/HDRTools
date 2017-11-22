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
 * \file DistortionMetricBlockAct.cpp
 *
 * \brief
 *    New Blockiness metric based on simple activity analysis
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricBlockAct.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricBlockAct::DistortionMetricBlockAct(double maxSampleValue, DistortionFunction distortionMethod)
: DistortionMetric()
{
  m_transferFunction   = DistortionTransferFunction::create(distortionMethod, FALSE);

  m_memWidth = 0;
  m_memHeight = 0;
  m_blkWidth = 128;
  m_blkHeight = 128;
  m_blkWOffset = 64;
  m_blkHOffset = 64;
  m_width = 0;
  m_height = 0;
  
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
  }
}

DistortionMetricBlockAct::~DistortionMetricBlockAct()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------

double DistortionMetricBlockAct::windowSum(const double *iData, int iPosX, int iWidth) {
  double dValue = 0.0;
  const double *currentLine = &iData[0];
  int m_wSizeX = 1;
  
  for (int i = -m_wSizeX; i <= m_wSizeX; i++) {
    dValue += currentLine[iClip(iPosX + i, 0, iWidth)];
  }
  return dValue;
}


/*! \brief Compute NxN neighborhood DC Value.
 *
 *  Functions computes the Sum value within an NxN window around the
 *  the current sample.
 */
void DistortionMetricBlockAct::computeImgWindowSum(const double *iData, int iWidth) {   
  double  *currentSum = &m_dSum[0];
  
    for (int i = ZERO; i < iWidth; i++) {
      *currentSum++ = windowSum(iData, i, iWidth);
    }
}


//-----------------------------------------------------------------------------
// Compute the sum of all values in a 3x3 arrangement
//-----------------------------------------------------------------------------
double DistortionMetricBlockAct::computeShortDC(int iWidth) {
  double value;
  double noise = 0.0;
  double *data = &m_dDistance[0];
  double *currentSum = &m_dSumDev[0];
  
  for (int i = ZERO; i < iWidth; i++) {
    value = windowSum(data, i, iWidth);
    *currentSum++ = value;
    noise += value;
  }
  
  return (noise / (double) (iWidth));
}

//-----------------------------------------------------------------------------
// Absolute distance from 3x3 mean
//-----------------------------------------------------------------------------
void DistortionMetricBlockAct::computeMeanDist(const double *iData, int iWidth) {
  
  double *distImg = &m_dDistance[0];
  double *sumImg = &m_dSum[0];
  
  // Compute abs distance from window sum for each position
  for (int i = ZERO; i < iWidth; i++) {
    *distImg++ = dAbs(3 * (double) (*iData++) - (*sumImg++));
  }
}

//-----------------------------------------------------------------------------
// Wiener filtering given total noise, local mean and local absolute deviation
//-----------------------------------------------------------------------------
void DistortionMetricBlockAct::wienerFilter(const double *iData, double noise,  int iWidth) {
  double normValue;
  
  double *pSumImg = &m_dSum[0];
  double *pMdImg  = &m_dSumDev[0];
  double *pOutImg = &m_dOutput[0];
  
  
  // Compute abs distance from 3x3 mean for each position
  for (int i = ZERO; i < iWidth; i++) {
    if (*pMdImg > noise) {
      normValue = (3 * (double) *iData++) - *pSumImg;
      *pOutImg++ = (normValue * (1.0 - noise / (*pMdImg++)) + (*pSumImg++)) / 3;
    }
    else {
      // Set output to mean
      *pOutImg++ = *pSumImg++ / 3;
      iData++;
      pMdImg++;        
    }
  }
}

void DistortionMetricBlockAct::updateImg(double *out, const double *inp, int size) {
  for (int i = 0; i < size; i++) {
    *out++ = *inp++;
  }  
}


void DistortionMetricBlockAct::filter(double *imgData, int width)
{
  // Update array size
  if ( width > m_width) {
    m_width  = width;
    
    m_dSum.resize      ( m_width );
    m_dSumDev.resize   ( m_width );
    m_dDistance.resize ( m_width );
    m_dOutput.resize   ( m_width );
  }
  
  computeImgWindowSum(imgData, width);
  computeMeanDist    (imgData, width);
  
  double noise = computeShortDC(width);
  wienerFilter(imgData, noise, width);
  
  updateImg(imgData, &m_dOutput[0], width);
}


double DistortionMetricBlockAct::avgSubsample(double *data, int step, int offset, int length)
{
  double sum = 0.0;
  for (int j = offset; j < length - 1; j += step) {
    sum += data[j];
  }
  return (sum / ((length - 1 - offset) / step));
}


void DistortionMetricBlockAct::computeActivity(float *inpData, int height, int width, int index, double maxValue)
{
  for (int j = 0; j < height - 1; j++) {
    for (int i = 0; i < width - 1; i++) { 
      //m_verGrad[index][ j * width + i ] = (inpData[(j + 1) * width + i] - inpData[j * width + i]) / maxValue;
      //m_horGrad[index][ j * width + i ] = (inpData[ j * width + i + 1 ] - inpData[j * width + i]) / maxValue;
      m_verAct[index][ j * width + i ] = m_transferFunction->compute(inpData[(j + 1) * width + i] / maxValue) - m_transferFunction->compute(inpData[j * width + i] / maxValue);
      m_horAct[index][ j * width + i ] = m_transferFunction->compute(inpData[ j * width + i + 1 ] / maxValue) - m_transferFunction->compute(inpData[j * width + i] / maxValue);
    }
  }
}

void DistortionMetricBlockAct::compareActivity(int size)
{
    for (int i = 0; i < size; i++) { 
      m_verActDiff[ i ] = m_verAct[0][ i ] - m_verAct[1][ i ];
      m_horActDiff[ i ] = m_horAct[0][ i ] - m_horAct[1][ i ];
    }
}

void DistortionMetricBlockAct::accumulateHArray(double *array, double *out, int width, int height, int arraywidth, int posY, int posX)
{
  for (int i = 0; i < width; i++) { 
    out[ i ] = array[ posY * arraywidth + posX +i];
  }
  
  for (int j = posY + 1; j < posY + height; j++) { 
    for (int i = 0; i < width; i++) { 
      out[ i ] += array[ j * arraywidth + posX + i];
    }
  }

  for (int i = 0; i < width; i++) { 
    out[ i ] /= arraywidth;
  }
}

void DistortionMetricBlockAct::accumulateVArray(double *array, double *out, int width, int height, int arraywidth, int posY, int posX)
{
  for (int j = 0; j < height; j++) { 
    out[j] = array[ (j + posY) * arraywidth + posX];
    for (int i = 0; i < width; i++) { 
      out[ j ] += array[ (j + posY) * arraywidth + posX + i];
    }
    out[ j ] /= arraywidth;
  }
}

double DistortionMetricBlockAct::shiftAccumulateVector(double *inVector, int width)
{
  double sum = 0.0;
  for (int i = 0; i < width - 1; i++) { 
    sum += dAbs(inVector[ i ] - inVector[ i + 1]);
  }
  return (sum / (double) (width - 1));
}

double DistortionMetricBlockAct::accumulateVector(double *inVector, int width)
{
  double sum = 0.0;
  for (int i = 0; i < width; i++) { 
    //sum += dAbs(inVector[ i ]);
    sum += dAbs2(inVector[ i ]);
  }
  return (sum / (double) width);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricBlockAct::computeMetric (Frame* inp0, Frame* inp1)
{
  
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
  
    if (inp0->m_width[0] > m_memWidth || inp0->m_height[0] > m_memHeight ) {
      m_memWidth  = inp0->m_width[0];
      m_memHeight = inp0->m_height[0];
      for (int c = 0; c < 2; c++) {      
        m_verAct[c].resize  ( m_memWidth * m_memHeight );
        m_horAct[c].resize  ( m_memWidth * m_memHeight );
        m_verActDiff.resize ( m_memWidth * m_memHeight );
        m_horActDiff.resize ( m_memWidth * m_memHeight );
        m_sumW.resize ( m_memHeight );
        m_sumH.resize ( m_memWidth  );
      }
    }
  
    if (inp0->m_isFloat == TRUE) {    // floating point data
      
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        double blockCostH = 0.0;
        double blockCostV = 0.0;
        // source image
        computeActivity(inp0->m_floatComp[c], inp0->m_height[c], inp0->m_width[c], 0, m_maxValue[c]);        
        
        // test image
        computeActivity(inp1->m_floatComp[c], inp1->m_height[c], inp1->m_width[c], 1, m_maxValue[c]);
        
        // Now compare activities from both images
        compareActivity(inp0->m_height[c] * inp0->m_width[c]);
        
        // accumulate horizontal edges
        for (int j = 0; j < inp0->m_height[c] - (m_blkHeight + 1); j += m_blkHOffset) {
          accumulateHArray(&m_horActDiff[0], &m_sumW[0], inp0->m_width[c] - 1, m_blkHeight, inp0->m_width[c], j, 0);
          filter(&m_sumW[0], inp0->m_width[c] - 1);
          blockCostH += accumulateVector(&m_sumW[0], inp0->m_width[c] - 1);
        }
        blockCostH /= ((inp0->m_height[c] - (m_blkHeight + 1)) / m_blkHOffset);

        // accumulate vertical edges
        for (int j = 0; j < inp0->m_width[c] - (m_blkWidth + 1); j += m_blkWOffset) {
          accumulateVArray(&m_verActDiff[0], &m_sumH[0], m_blkWidth, inp0->m_height[c] - 1, inp0->m_width[c], 0, j);
          filter(&m_sumH[0], inp0->m_height[c] - 1);
          blockCostV += accumulateVector(&m_sumH[0], inp0->m_height[c] - 1);
        }
        blockCostV /= ((inp0->m_width[c] - (m_blkWidth + 1)) / m_blkWOffset);
        
        m_metric[c] = 1.0 - 1.0 / ((blockCostH + blockCostV) * 1000000.0 + 1.0);
        m_metricStats[c].updateStats(m_metric[c]);
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


void DistortionMetricBlockAct::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  // Currently no TF is applied on the data. However, we may wish to apply some TF, e.g. PQ or combination of PQ and PH.
  
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

void DistortionMetricBlockAct::reportMetric  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
}

void DistortionMetricBlockAct::reportSummary  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
}

void DistortionMetricBlockAct::reportMinimum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
}

void DistortionMetricBlockAct::reportMaximum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
}

void DistortionMetricBlockAct::printHeader()
{
  if (m_isWindow == FALSE ) {
    printf("  Blk-CM0  "); // 11
    printf("  Blk-CM1  "); // 11
    printf("  Blk-CM2  "); // 11
  }
  else {
    printf(" wBlk-CM0  "); // 11
    printf(" wBlk-CM1  "); // 11
    printf(" wBlk-CM2  "); // 11
  }
}

void DistortionMetricBlockAct::printSeparator(){
  printf("-----------");
  printf("-----------");
  printf("-----------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
