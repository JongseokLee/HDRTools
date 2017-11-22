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
 * \file DistortionMetricBlockinessJ341.cpp
 *
 * \brief
 *    Blockiness metric based on J.341
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricBlockinessJ341.H"


//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricBlockinessJ341::DistortionMetricBlockinessJ341(double maxSampleValue, DistortionFunction distortionMethod)
: DistortionMetric()
{
  m_transferFunction   = DistortionTransferFunction::create(distortionMethod, FALSE);

  m_memWidth = 0;
  m_memHeight = 0;
  
  for (int c = 0; c < T_COMP; c++) {
    m_metric[c] = 0.0;
    m_metricStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
  }
}

DistortionMetricBlockinessJ341::~DistortionMetricBlockinessJ341()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
// sTransform taken from vquad code
double DistortionMetricBlockinessJ341::sTransform( const double x, double px, double py, double q ){
  double y = 0.0;
  if( x < px ){
    double b = (px * q) / py;
    double a =  py / pow(px, b );
    y = a * pow( x, b );
    // check for a numerical error
    if( !(y >= 0.0 && y <= py) ){
      // perform partial linear estimate
      double deltaX = py / q;
      y = 0.0;
      if( x > px - deltaX ){			
        y = (x - (px - deltaX)) * q;
      }
    }
  }
  else{
    double d = 1.0 - py;
    double c = 2.0 * q / d;
    y = 2.0 * d * (1.0 / (1.0 + exp(-c * (x - px))) - 0.5) + py;
  }
  return y;		
}


double DistortionMetricBlockinessJ341::avgSubsample(double *data, int step, int offset, int length)
{
  double sum = 0.0;
  for (int j = offset; j < length - 1; j += step) {
    sum += data[j];
  }
  return (sum / ((length - 1 - offset) / step));
}


void DistortionMetricBlockinessJ341::computeGradients(float *inpData, int height, int width, int index, double maxValue)
{
  double w, h;
  for (int j = 0; j < height - 1; j++) {
    for (int i = 0; i < width - 1; i++) { 
      //w = m_verGrad[index][ j * width + i ] = (inpData[(j + 1) * width + i] - inpData[j * width + i]) / maxValue;
      //h = m_horGrad[index][ j * width + i ] = (inpData[ j * width + i + 1 ] - inpData[j * width + i]) / maxValue;
      w = m_verGrad[index][ j * width + i ] = m_transferFunction->compute(inpData[(j + 1) * width + i] / maxValue) - m_transferFunction->compute(inpData[j * width + i] / maxValue);
      h = m_horGrad[index][ j * width + i ] = m_transferFunction->compute(inpData[ j * width + i + 1 ] / maxValue) - m_transferFunction->compute(inpData[j * width + i] / maxValue);
      m_sumW[index][j] += log(1.0 + dMax(0.0, maxValue * dAbs(w) - 2.0 * (maxValue / 255)));
      m_sumH[index][i] += log(1.0 + dMax(0.0, maxValue * dAbs(h) - 2.0 * (maxValue / 255)));
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricBlockinessJ341::computeMetric (Frame* inp0, Frame* inp1)
{
  
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
  
    if (inp0->m_width[0] > m_memWidth || inp0->m_height[0] > m_memHeight ) {
      m_memWidth  = inp0->m_width[0];
      m_memHeight = inp0->m_height[0];
      for (int c = 0; c < 2; c++) {      
        m_verGrad[c].resize ( m_memWidth * m_memHeight );
        m_horGrad[c].resize ( m_memWidth * m_memHeight );
        m_sumW   [c].resize ( m_memHeight );
        m_sumH   [c].resize ( m_memWidth  );
      }
    }
  
    if (inp0->m_isFloat == TRUE) {    // floating point data
      double dH0, dH1, dW0, dW1;
      double edgeMax0, edgeMax1, edgeMin0, edgeMin1;
      double deltaEdge0, deltaEdge1;
      double x;
      
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        // source image
        computeGradients(inp0->m_floatComp[c], inp0->m_height[c], inp0->m_width[c], 0, m_maxValue[c]);
        dH0 = avgSubsample(&m_sumH[0][0], 2, 0, inp0->m_height[c]);
        dH1 = avgSubsample(&m_sumH[0][0], 2, 1, inp0->m_height[c]);
        dW0 = avgSubsample(&m_sumW[0][0], 2, 0, inp0->m_width[c]);
        dW1 = avgSubsample(&m_sumW[0][0], 2, 1, inp0->m_width[c]);
        
        edgeMax0 = 0.5 * (dMax(dW0, dW1) + dMax(dH0, dH1));
        edgeMin0 = 0.5 * (dMin(dW0, dW1) + dMin(dH0, dH1));
        deltaEdge0 = edgeMax0 - edgeMin0;
        
        
        // test image
        computeGradients(inp1->m_floatComp[c], inp1->m_height[c], inp1->m_width[c], 1, m_maxValue[c]);
        dH0 = avgSubsample(&m_sumH[1][0], 2, 0, inp0->m_height[c]);
        dH1 = avgSubsample(&m_sumH[1][0], 2, 1, inp0->m_height[c]);
        dW0 = avgSubsample(&m_sumW[1][0], 2, 0, inp0->m_width[c]);
        dW1 = avgSubsample(&m_sumW[1][0], 2, 1, inp0->m_width[c]);
        
        edgeMax1 = 0.5 * (dMax(dW0, dW1) + dMax(dH0, dH1));
        edgeMin1 = 0.5 * (dMin(dW0, dW1) + dMin(dH0, dH1));
        deltaEdge1 = edgeMax1 - edgeMin1;
                
        // note that this metric is not symmetric
        // also note that although in the spec only distance of 2 is mentioned, in the provided
        // code a distance of 3 is also tested. If distance of 3 has a larger gap than 2 
        // (between min/max) then the distance 3 values are used instead
        x = dMax( 0.0, deltaEdge1 - deltaEdge0) / (1.0 + edgeMax1);
        // as in the vquad code, use a sigoid function to convert the data
        m_metric[c] = x; // sTransform(x, 0.2, 0.1, 2.0);
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


void DistortionMetricBlockinessJ341::computeMetric (Frame* inp0, Frame* inp1, int component)
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

void DistortionMetricBlockinessJ341::reportMetric  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
}

void DistortionMetricBlockinessJ341::reportSummary  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
}

void DistortionMetricBlockinessJ341::reportMinimum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
}

void DistortionMetricBlockinessJ341::reportMaximum  ()
{
  printf("%10.5f %10.5f %10.5f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
}

void DistortionMetricBlockinessJ341::printHeader()
{
  if (m_isWindow == FALSE ) {
    printf(" B341-CM0  "); // 11
    printf(" B341-CM1  "); // 11
    printf(" B341-CM2  "); // 11
  }
  else {
    printf("wB341-CM0  "); // 11
    printf("wB341-CM1  "); // 11
    printf("wB341-CM2  "); // 11
  }
}

void DistortionMetricBlockinessJ341::printSeparator(){
  printf("-----------");
  printf("-----------");
  printf("-----------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
