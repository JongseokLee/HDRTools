/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/ISO
 * <ORGANIZATION> = Ericsson
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Ericsson
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
 * \file DistortionMetricmPSNRfast.cpp
 *
 * \brief
 *    mPSNRfast distortion computation Class
 *
 * \author
 *     - Jacob Strom         <jacob.strom@ericsson.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricmPSNRfast.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricmPSNRfast::DistortionMetricmPSNRfast(const FrameFormat *format, bool enableComponentmPSNR, bool enableSymmetry)
 : DistortionMetric()
{
  m_enableComponentmPSNR = enableComponentmPSNR;
  m_enableSymmetry = enableSymmetry;
  m_count = m_enableSymmetry ? 3 : 1;
  for (int i = 0; i < 3; i++) {
    for (int c = 0; c < T_COMP; c++) {
      m_mse[i][c] = 0.0;
      m_sse[i][c] = 0.0;
      m_mseStats[i][c].reset();
      m_sseStats[i][c].reset();
      m_mPSNR[i][c] = 0.0;
      m_mPSNRStats[i][c].reset();
    }
    m_mseTotal[i] = 0.0;
    m_mPSNRTotal[i] = 0.0;
    m_mPSNRTotalStats[i].reset();
  }
}

DistortionMetricmPSNRfast::~DistortionMetricmPSNRfast()
{
}
  
//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
double DistortionMetricmPSNRfast::calculateErrorRGB(Frame* inp0, Frame* inp1, double *sse, double *mse)
{
  static const double gamma = 2.20; // Could this be parameterized?
  static const double inverseGamma = 1.0 / gamma;
  static const double l2 = log(2.0);
  static const double minConst = gamma * log(0.5/255.0)/l2;
  static const double maxConst = gamma * log(254.5/255.0)/l2;
  
  double curSSE = 0.0;
  int error;
  int numPixels = 0;

  int size = inp0->m_compSize[R_COMP];
  
  float *inp0Comp0 = inp0->m_floatComp[0];
  float *inp0Comp1 = inp0->m_floatComp[1];
  float *inp0Comp2 = inp0->m_floatComp[2];
  
  float *inp1Comp0 = inp1->m_floatComp[0];
  float *inp1Comp1 = inp1->m_floatComp[1];
  float *inp1Comp2 = inp1->m_floatComp[2];

  sse[R_COMP] = 0.0;
  sse[G_COMP] = 0.0;
  sse[B_COMP] = 0.0;

  for(int x = 0; x < size; x++) {
    // First calculate the lowest possible c.
    
    // Make sure neither original nor compressed pixel is below 0 or higher than 65504.
    // (Max value for half is 65504. Infs will be clipped to 65504.)
    float rOrig = fClip(*inp0Comp0++, 0.0f, 65504.0f);
    float gOrig = fClip(*inp0Comp1++, 0.0f, 65504.0f);
    float bOrig = fClip(*inp0Comp2++, 0.0f, 65504.0f);
      
    float rCopy = fClip(*inp1Comp0++, 0.0f, 65504.0f);
    float gCopy = fClip(*inp1Comp1++, 0.0f, 65504.0f);
    float bCopy = fClip(*inp1Comp2++, 0.0f, 65504.0f);

    // Get the biggest color component
    double colMax = dMax(bOrig, dMax(rOrig, gOrig));
    
    // Calculate the first value that will give a contribution
    // This happens 255*(2^c * colMax)^(1/gamma) is exactly 0.5
    // which is equivalent to cMin = gamma*log(0.5/255)/log(2) - log(colMax)/log(2);
    double cMin = minConst - log(colMax)/l2;

    // If cMin is, say, -10.6, the first valid integer c will be -10
    int cMinInt = double2IntCeil(cMin);

    // Calculate last value.
    // This happens 255*(2^c * colMax)^(1/gamma) is exactly 254.5
    // which is equivalent to cMin = gamma*log(254.5/255)/log(2) - log(colMax)/log(2);
    double cMax = maxConst - log(colMax)/l2;
    
    // If cMax is, say, 4.6, the last valid integer c will be 4
    int cMaxInt = double2IntFloor(cMax);
    
    // use the fact that nextValue = oldValue*(2^(1/gamma));
    double twoToC = 255.0 * exp( inverseGamma * ((cMinInt-1)*l2));
    // This seems to help with caching
    double factor = exp(inverseGamma * l2);
    // starting values
    // pow seems slower than exp(y * log (x)) :(.
    //double rOrigProcessed = twoToC *  pow( rOrig, inverseGamma );
    double rOrigProcessed = twoToC *  exp( inverseGamma * log( rOrig ) );
    double gOrigProcessed = twoToC *  exp( inverseGamma * log( gOrig ) );
    double bOrigProcessed = twoToC *  exp( inverseGamma * log( bOrig ) );
    double rCopyProcessed = twoToC *  exp( inverseGamma * log( rCopy ) );
    double gCopyProcessed = twoToC *  exp( inverseGamma * log( gCopy ) );
    double bCopyProcessed = twoToC *  exp( inverseGamma * log( bCopy ) );


    for(int c = cMinInt; c<=cMaxInt; c++) {
      rOrigProcessed *= factor;
      gOrigProcessed *= factor;
      bOrigProcessed *= factor;
      rCopyProcessed *= factor;
      gCopyProcessed *= factor;
      bCopyProcessed *= factor;
      
      // Now clamp these values to [0, 255] and round to integer. 
      //
      // After clamping, all values are positive and we can use (int)(x+.5) instead of dRound(x)
      // Even before clamping, all values are postivie since 255*(2^c * x)^(1/gamma) > 0 if x > 0, 
      // hence we can use dMin(x, 255) instead of dClip(x, 0, 255)
      // Original values are guarranteed to be below 254.5 so can avoid clamping altogether. 
      //

      int LDRorigR = (int) (rOrigProcessed + 0.5); // guarranteed to be lower than 254.5 otherwise we will saturate
      int LDRorigG = (int) (gOrigProcessed + 0.5); // guarranteed to be lower than 254.5 otherwise we will saturate
      int LDRorigB = (int) (bOrigProcessed + 0.5); // guarranteed to be lower than 254.5 otherwise we will saturate

      int LDRcopyR = (int) dMin( rCopyProcessed + 0.5, 255.5);
      int LDRcopyG = (int) dMin( gCopyProcessed + 0.5, 255.5);
      int LDRcopyB = (int) dMin( bCopyProcessed + 0.5, 255.5);

      error = (LDRorigR - LDRcopyR);
      sse[R_COMP] += (double) (error * error);
      error = (LDRorigG - LDRcopyG);
      sse[G_COMP] += (double) (error * error);
      error = (LDRorigB - LDRcopyB);
      sse[B_COMP] += (double) (error * error);
      
    }
    numPixels += (cMaxInt - cMinInt + 1);
  }
  curSSE = sse[R_COMP] + sse[G_COMP] + sse[B_COMP];

  for (int c = R_COMP; c <= B_COMP; c++)
    mse[c] = sse[c]/(numPixels);
  
  return curSSE/(3.0*numPixels);
}

double DistortionMetricmPSNRfast::calculateErrorYUV(Frame* inp0, Frame* inp1, double *sse, double *mse)
{
  static const double gamma = 2.20; // Could this be parameterized?
  static const double inverseGamma = 1.0 / gamma;
  static const double l2 = log(2.0);
  static const double minConst = gamma * log(0.5/255.0)/l2;
  static const double maxConst = gamma * log(254.5/255.0)/l2;


  double curSSE = 0.0;
  int numPixels = 0;

  int size = inp0->m_compSize[R_COMP];
  
  float *inp0Comp0 = inp0->m_floatComp[0];
  float *inp0Comp1 = inp0->m_floatComp[1];
  float *inp0Comp2 = inp0->m_floatComp[2];
  
  float *inp1Comp0 = inp1->m_floatComp[0];
  float *inp1Comp1 = inp1->m_floatComp[1];
  float *inp1Comp2 = inp1->m_floatComp[2];

  int errorR, errorG, errorB;
  double error;

  sse[R_COMP] = 0.0;
  sse[G_COMP] = 0.0;
  sse[B_COMP] = 0.0;

  for(int x = 0; x < size; x++) {
    // First calculate the lowest possible c.
    
    // Make sure neither original nor compressed pixel is below 0 or higher than 65504.
    // (Max value for half is 65504. Infs will be clipped to 65504.)
    float rOrig = fClip(*inp0Comp0++, 0.0f, 65504.0f);
    float gOrig = fClip(*inp0Comp1++, 0.0f, 65504.0f);
    float bOrig = fClip(*inp0Comp2++, 0.0f, 65504.0f);
    
    float rCopy = fClip(*inp1Comp0++, 0.0f, 65504.0f);
    float gCopy = fClip(*inp1Comp1++, 0.0f, 65504.0f);
    float bCopy = fClip(*inp1Comp2++, 0.0f, 65504.0f);

    // Get the biggest color component
    double colMax = dMax(bOrig, dMax(rOrig, gOrig));
    
    // Calculate the first value that will give a contribution
    // This happens 255*(2^c * colMax)^(1/gamma) is exactly 0.5
    // which is equivalent to cMin = gamma*log(0.5/255)/log(2) - log(colMax)/log(2);
    double cMin = minConst - log(colMax)/l2;
    
    // Calculate last value.
    // This happens 255*(2^c * colMax)^(1/gamma) is exactly 254.5
    // which is equivalent to cMin = gamma*log(254.5/255)/log(2) - log(colMax)/log(2);
    double cMax = maxConst - log(colMax)/l2;
    
    // If cMin is, say, -10.6, the first valid integer c will be -10
    int cMinInt = double2IntCeil(cMin);
    // If cMax is, say, 4.6, the last valid integer c will be 4
    int cMaxInt = double2IntFloor(cMax);

    // use the fact that nextValue = oldValue*(2^(1/gamma));
    double twoToC = 255.0 * exp( inverseGamma * ((cMinInt-1)*l2));
    double factor = exp(inverseGamma*l2);
    // starting values
    double rOrigProcessed = twoToC *  exp( inverseGamma * log( rOrig ) );
    double gOrigProcessed = twoToC *  exp( inverseGamma * log( gOrig ) );
    double bOrigProcessed = twoToC *  exp( inverseGamma * log( bOrig ) );
    double rCopyProcessed = twoToC *  exp( inverseGamma * log( rCopy ) );
    double gCopyProcessed = twoToC *  exp( inverseGamma * log( gCopy ) );
    double bCopyProcessed = twoToC *  exp( inverseGamma * log( bCopy ) );

    
    for(int c = cMinInt; c<=cMaxInt; c++) {
      // Update the values
      rOrigProcessed *= factor;
      gOrigProcessed *= factor;
      bOrigProcessed *= factor;
      rCopyProcessed *= factor;
      gCopyProcessed *= factor;
      bCopyProcessed *= factor;
      
      // Now clamp these values to [0, 255] and round to integer. 
      //
      // After clamping, all values are positive and we can use (int)(x+.5) instead of dRound(x)
      // Even before clamping, all values are postivie since 255*(2^c * x)^(1/gamma) > 0 if x > 0, 
      // hence we can use dMin(x, 255) instead of dClip(x, 0, 255)
      // Original values are guarranteed to be below 254.5 so can avoid clamping altogether. 
      //
      int LDRorigR = (int) (rOrigProcessed+0.5); // guarranteed to be lower than 254.5 otherwise we will saturate
      int LDRorigG = (int) (gOrigProcessed+0.5); // guarranteed to be lower than 254.5 otherwise we will saturate
      int LDRorigB = (int) (bOrigProcessed+0.5); // guarranteed to be lower than 254.5 otherwise we will saturate
      int LDRcopyR = (int) dMin( rCopyProcessed+0.5, 255.5); 
      int LDRcopyG = (int) dMin( gCopyProcessed+0.5, 255.5); 
      int LDRcopyB = (int) dMin( bCopyProcessed+0.5, 255.5); 

      errorR = (LDRorigR - LDRcopyR);
      errorG = (LDRorigG - LDRcopyG);
      errorB = (LDRorigB - LDRcopyB);
     
      if (errorR != 0 || errorG != 0 || errorB != 0) {
        // Accumulate error in RGB
        curSSE   += (double) (errorR * errorR);
        curSSE   += (double) (errorG * errorG);
        curSSE   += (double) (errorB * errorB);
        
        // Now do the same for Y Cb Cr. Note that since we are using linear transforms, we can just
        // transform the error instead and save on computations.
        // Y error
        error  =  0.2126f * errorR + 0.7152f * errorG + 0.0722f * errorB;
        //error  = ySample0 - ySample1;
        sse[Y_COMP]  += error * error;
        
        // CbError
        error = -0.1146f * errorR - 0.3854f * errorG + 0.5000f * errorB;
        //error  = cbSample0 - cbSample1;
        sse[U_COMP] += error * error;
        
        // CrError
        error =	0.5000f * errorR - 0.4542f * errorG - 0.0458f * errorB;
        //error  = crSample0 - crSample1;
        sse[V_COMP] += error * error;
      }
    }
    numPixels += (cMaxInt - cMinInt + 1);
  }

  for (int c = R_COMP; c <= B_COMP; c++)
    mse[c] = sse[c]/(numPixels);
  
  return curSSE/(3.0*numPixels);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricmPSNRfast::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_format.m_colorSpace != CM_RGB) {
      printf("mPSNRfast only supported for RGB data.\n");
    }
    else {
      if (inp0->m_isFloat == TRUE) {
        // floating point data
        if (m_enableComponentmPSNR == TRUE) {
          m_mseTotal[0] = calculateErrorYUV(inp0, inp1, m_sse[0], m_mse[0]);
          // enable symmetric computation
          if (m_enableSymmetry == TRUE) {
            m_mseTotal[1] = calculateErrorYUV(inp1, inp0, m_sse[1], m_mse[1]);
            m_mseTotal[2] = (m_mseTotal[0] + m_mseTotal[1]) / 2.0;
            for (int c = R_COMP; c <= B_COMP; c++) {
              m_sse[2][c] = (m_sse[0][c] + m_sse[1][c]) / 2.0;
              m_mse[2][c] = (m_mse[0][c] + m_mse[1][c]) / 2.0;
            }
          }
          for (int i = 0; i < m_count; i++) {
            m_mPSNRTotal[i] = psnr(255.0, 1, m_mseTotal[i]);
            m_mPSNRTotalStats[i].updateStats(m_mPSNRTotal[i]);
            for (int c = R_COMP; c < inp0->m_noComponents; c++) {
              m_sseStats[i][c].updateStats(m_sse[i][c]);
              m_mseStats[i][c].updateStats(m_mse[i][c]);
              m_mPSNR[i][c] = psnr(255.0, 1, m_mse[i][c]);
              m_mPSNRStats[i][c].updateStats(m_mPSNR[i][c]);
            }
          }
        }
        else {
          m_mseTotal[0] = calculateErrorRGB(inp0, inp1, m_sse[0], m_mse[0]);
          if (m_enableSymmetry == TRUE) {
            m_mseTotal[1] = calculateErrorRGB(inp1, inp0, m_sse[1], m_mse[1]);
            m_mseTotal[2] = (m_mseTotal[0] + m_mseTotal[1]) / 2.0;
          }
          for (int i = 0; i < m_count; i++) {
            m_mPSNRTotal[i] = psnr(255.0, 1, m_mseTotal[i]);
            m_mPSNRTotalStats[i].updateStats(m_mPSNRTotal[i]);
          }
        }
      }
      else if (inp0->m_bitDepth == 8) {   // 8 bit data
        printf("mPSNRfast for 8 bit data not implemented. Exiting.\n");
        exit(1);
      }
      else { // 16 bit data
        printf("mPSNRfast for 16 bit integer data not implemented. Exiting.\n");
        exit(1);
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}
void DistortionMetricmPSNRfast::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // Currently this function does nothing. TBD
  
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

void DistortionMetricmPSNRfast::reportMetric  ()
{
  for (int i = 0; i < m_count; i++) {
    if (m_enableComponentmPSNR)
      printf("%9.3f %9.3f %9.3f ", m_mPSNR[i][Y_COMP], m_mPSNR[i][U_COMP], m_mPSNR[i][V_COMP]);
    printf("%8.3f ", m_mPSNRTotal[i]);
  }
}

void DistortionMetricmPSNRfast::reportSummary  ()
{
  for (int i = 0; i < m_count; i++) {
    if (m_enableComponentmPSNR)
      printf("%9.3f %9.3f %9.3f ", m_mPSNRStats[i][Y_COMP].getAverage(), m_mPSNRStats[i][U_COMP].getAverage(), m_mPSNRStats[i][V_COMP].getAverage());
    printf("%8.3f ", m_mPSNRTotalStats[i].getAverage());
  }
}

void DistortionMetricmPSNRfast::reportMinimum  ()
{
  for (int i = 0; i < m_count; i++) {
    if (m_enableComponentmPSNR)
      printf("%9.3f %9.3f %9.3f ", m_mPSNRStats[i][Y_COMP].minimum, m_mPSNRStats[i][U_COMP].minimum, m_mPSNRStats[i][V_COMP].minimum);
    printf("%8.3f ", m_mPSNRTotalStats[i].minimum);
  }
}

void DistortionMetricmPSNRfast::reportMaximum  ()
{
  for (int i = 0; i < m_count; i++) {
    if (m_enableComponentmPSNR)
      printf("%9.3f %9.3f %9.3f ", m_mPSNRStats[i][Y_COMP].maximum, m_mPSNRStats[i][U_COMP].maximum, m_mPSNRStats[i][V_COMP].maximum);
    printf("%8.3f ", m_mPSNRTotalStats[i].maximum);
  }
}

void DistortionMetricmPSNRfast::printHeader()
{
  for (int i = 0; i < m_count; i++) {
    if (m_isWindow == FALSE ) {
      if (m_enableComponentmPSNR) {
        printf(" mfPSNRY%d ", i); // 10
        printf(" mfPSNRU%d ", i); // 10
        printf(" mfPSNRV%d ", i); // 10
      }
      printf(" mfPSNR%d ", i); // 9
    }
    else {
      if (m_enableComponentmPSNR) {
        printf("wmfPSNRY%d ", i); // 10
        printf("wmfPSNRU%d ", i); // 10
        printf("wmfPSNRV%d ", i); // 10
      }
      printf("wmfPSNR%d ", i); // 9
    }
  }
}

void DistortionMetricmPSNRfast::printSeparator(){
  for (int i = 0; i < m_count; i++) {
    if (m_enableComponentmPSNR) {
      printf("----------");
      printf("----------");
      printf("----------");
    }
    printf("---------");
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
