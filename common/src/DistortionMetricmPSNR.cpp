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
 * \file DistortionMetricmPSNR.cpp
 *
 * \brief
 *    mPSNR distortion computation Class
 *
 * \author
 *     - Jacob Strom         <jacob.strom@ericsson.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricmPSNR.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricmPSNR::DistortionMetricmPSNR(const FrameFormat *format, bool enableComponentmPSNR, double maxSampleValue)
: DistortionMetric()
{
  m_enableComponentmPSNR = enableComponentmPSNR;
  for (int c = 0; c < T_COMP; c++) {
    m_mse[c] = 0.0;
    m_sse[c] = 0.0;
    m_mseStats[c].reset();
    m_sseStats[c].reset();
    m_maxValue[c] = (double) maxSampleValue;
  }
  m_mPSNR = 0.0;
  m_mPSNRStats.reset();
  
  // memory buffers for the virtual images
  m_compSize = format->m_compSize[R_COMP];
  
  m_ldrSrc.resize(3 * m_compSize);
  m_ldrRef.resize(3 * m_compSize);
}

DistortionMetricmPSNR::~DistortionMetricmPSNR()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void DistortionMetricmPSNR::calculateSSEnoSaturationRGB(imgpel *lossyimg, imgpel *origimg, int size, int &numPixels, double &sse, double &sseR, double &sseG, double & sseB)
{
  // calculate Summed Square Error (SSE) but only over pixels that are neither
  // black (i.e., equal to (0,0,0)) nor saturated (such as e.g., (255, 13, 25).
  //
  // Calculates average RGB value as well as individual for R, G and B.
  double err;
  sse = 0;
  sseR = 0;
  sseG = 0;
  sseB = 0;
  numPixels = 0;
  
  for(int x=0; x < size; x++) {
    if (!(origimg[x*3 + 0]==0 && origimg[x*3 + 1]==0 && origimg[x*3 + 2]==0) && !((origimg[x*3 + 0]==255 || origimg[x*3 + 1]==255 || origimg[x*3 + 2]==255))) {
      // Neither black nor saturated. Calculate error and add.
      err = (1.0*lossyimg[x*3+0]-1.0*origimg[x*3+0]);
      sseR += err*err;
      
      err = (1.0*lossyimg[x*3+1]-1.0*origimg[x*3+1]);
      sseG += err*err;
      
      err = (1.0*lossyimg[x*3+2]-1.0*origimg[x*3+2]);
      sseB += err*err;
      
      numPixels++;
    }
  }
  
  sse = sseR + sseG + sseB;
}

void DistortionMetricmPSNR::calculateSSEnoSaturationYUV(imgpel *lossyimg, imgpel *origimg, int size, int &numPixels, double &sse, double &sseY, double &sseCb, double & sseCr)
{
  // calculate Summed Square Error (SSE) but only over pixels that are neither
  // black (i.e., equal to (0,0,0)) nor saturated (such as e.g., (255, 13, 25).
  //
  // After deciding what pixels to use, we both calculate the average RGB mean
  // square error, as well as transform the data to YUV and do another set
  // of MSE calculations there.
  sse = 0.0;
  sseY = 0.0;
  sseCb = 0.0;
  sseCr = 0.0;
  numPixels = 0;
  int rDiff, gDiff, bDiff;
  int64 sseI64 = 0;
  float error;
  //float ySample0, cbSample0, crSample0;
  //float ySample1, cbSample1, crSample1;

  
  for(int x=0; x<size; x++) {
    uint8 rOrig = *origimg++;
    uint8 gOrig = *origimg++;
    uint8 bOrig = *origimg++;

    if (!(rOrig==0 && gOrig==0 && bOrig==0) && !((rOrig == 255 || gOrig == 255 || bOrig == 255))) {
      // Neither black nor saturated. Calculate error and add.
      
      
      //uint8 rCopy = *lossyimg++;
      //uint8 gCopy = *lossyimg++;
      //uint8 bCopy = *lossyimg++;
      
      rDiff = rOrig - *lossyimg++;
      sseI64 += rDiff * rDiff;
      gDiff = gOrig - *lossyimg++;
      sseI64 += gDiff * gDiff;
      bDiff = bOrig - *lossyimg++;
      sseI64 += bDiff * bDiff;
      
      // Now do the same for Y Cb Cr:
      /*
      ySample0  =  0.2126f * rOrig + 0.7152f * gOrig + 0.0722f * bOrig;
      cbSample0 = -0.1146f * rOrig - 0.3854f * gOrig + 0.5000f * bOrig;
      crSample0 =	 0.5000f * rOrig - 0.4542f * gOrig - 0.0458f * bOrig;
      
      ySample1  =  0.2126f * rCopy + 0.7152f * gCopy + 0.0722f * bCopy;
      cbSample1 = -0.1146f * rCopy - 0.3854f * gCopy + 0.5000f * bCopy;
      crSample1 =	 0.5000f * rCopy - 0.4542f * gCopy - 0.0458f * bCopy;
      */
      // Y error
      error  =  0.2126f * rDiff + 0.7152f * gDiff + 0.0722f * bDiff;
      //error  = ySample0 - ySample1;
      sseY  += error * error;

      // CbError
      error = -0.1146f * rDiff - 0.3854f * gDiff + 0.5000f * bDiff;
      
      //error  = cbSample0 - cbSample1;
      sseCb += error * error;

      // CrError
      error =	0.5000f * rDiff - 0.4542f * gDiff - 0.0458f * bDiff;
      //error  = crSample0 - crSample1;
      sseCr += error * error;
      
      numPixels++;
    }
    else {
      lossyimg += 3;
      //      origimg += 3;
    }
  }
  sse = (double) sseI64;
  
}

void DistortionMetricmPSNR::calculateSSEnoSaturationYUVfast(imgpel *lossyimg, imgpel *origimg, int &numPixels, double &sse, double &sseY, double &sseCb, double & sseCr)
{
  // calculate Summed Square Error (SSE) but only over pixels that are neither
  // black (i.e., equal to (0,0,0)) nor saturated (such as e.g., (255, 13, 25).
  //
  // After deciding what pixels to use, we both calculate the average RGB mean
  // square error, as well as transform the data to YUV and do another set
  // of MSE calculations there.
  //sseY = 0.0;
  //sseCb = 0.0;
  //sseCr = 0.0;
  //numPixels = 0;
  //float ySample0, cbSample0, crSample0;
  //float ySample1, cbSample1, crSample1;
  
  
  uint8 rOrig = *origimg++;
  uint8 gOrig = *origimg++;
  uint8 bOrig = *origimg++;
  
  if (!(rOrig==0 && gOrig==0 && bOrig==0) && !((rOrig == 255 || gOrig == 255 || bOrig == 255))) {
    // Neither black nor saturated. Calculate error and add.
    
    int rDiff   = rOrig - *lossyimg++;
    int sseI64  = rDiff * rDiff;
    int gDiff   = gOrig - *lossyimg++;
    sseI64 += gDiff * gDiff;
    int bDiff   = bOrig - *lossyimg++;
    sseI64 += bDiff * bDiff;
    sse    += (double) sseI64;
    
    // Now do the same for Y Cb Cr:
    // Y error
    float error  =  0.2126f * rDiff + 0.7152f * gDiff + 0.0722f * bDiff;
    //error  = ySample0 - ySample1;
    sseY  += error * error;
    
    // CbError
    error = -0.1146f * rDiff - 0.3854f * gDiff + 0.5000f * bDiff;
    
    //error  = cbSample0 - cbSample1;
    sseCb += error * error;
    
    // CrError
    error =	0.5000f * rDiff - 0.4542f * gDiff - 0.0458f * bDiff;
    //error  = crSample0 - crSample1;
    sseCr += error * error;
    
    numPixels++;
  }
}

double DistortionMetricmPSNR::calculateError(Frame* inp0, Frame* inp1)
{
  static const double gamma = 2.20; // Could this be parameterized?
  static const double inverseGamma = 1.0 / gamma;
  
  int totpixels = 0;
  double totsse = 0.0;
  double MSE = 0.0;
  int x;
  
  int size = inp0->m_compSize[R_COMP];
  
  // re-allocate memory for low dynamic range images (virtual photographs)
  // if size has changed for some odd reason
  m_compSize = size;
    
  m_ldrSrc.resize(3 * m_compSize);
  m_ldrRef.resize(3 * m_compSize);
  
  
  float *inp0Comp0 = inp0->m_floatComp[0];
  float *inp0Comp1 = inp0->m_floatComp[1];
  float *inp0Comp2 = inp0->m_floatComp[2];
  
  float *inp1Comp0 = inp1->m_floatComp[0];
  float *inp1Comp1 = inp1->m_floatComp[1];
  float *inp1Comp2 = inp1->m_floatComp[2];
  
  m_sse[R_COMP] = 0.0;
  m_sse[G_COMP] = 0.0;
  m_sse[B_COMP] = 0.0;
  
  for(int stops = 0; stops<50; stops++) {
    // Now create the two LDR images
    int stopVal = stops - 34;
    double twoToStopval = exp((double) stopVal * log(2.0));
    for(x = 0; x < size; x++) {
      // Clamp values to [0, 65504] to avoid negativel light and to
      // make sure infinities are not messing too much with the values,
      // since they are clamped to the largest possible half value.
      
      float rOrig = fClip(inp0Comp0[x], 0.0f, 65504.0f);
      float gOrig = fClip(inp0Comp1[x], 0.0f, 65504.0f);
      float bOrig = fClip(inp0Comp2[x], 0.0f, 65504.0f);
      
      float rCopy = fClip(inp1Comp0[x], 0.0f, 65504.0f);
      float gCopy = fClip(inp1Comp1[x], 0.0f, 65504.0f);
      float bCopy = fClip(inp1Comp2[x], 0.0f, 65504.0f);
      
      // Create virtual photographs
      m_ldrSrc[x*3 + 0] = (int) dRound(dClip(255.0 * exp( inverseGamma * log( twoToStopval * rOrig ) ), 0.0, 255.0));
      m_ldrSrc[x*3 + 1] = (int) dRound(dClip(255.0 * exp( inverseGamma * log( twoToStopval * gOrig ) ), 0.0, 255.0));
      m_ldrSrc[x*3 + 2] = (int) dRound(dClip(255.0 * exp( inverseGamma * log( twoToStopval * bOrig ) ), 0.0, 255.0));
      
      m_ldrRef[x*3 + 0] = (int) dRound(dClip(255.0 * exp( inverseGamma * log( twoToStopval * rCopy ) ), 0.0, 255.0));
      m_ldrRef[x*3 + 1] = (int) dRound(dClip(255.0 * exp( inverseGamma * log( twoToStopval * gCopy ) ), 0.0, 255.0));
      m_ldrRef[x*3 + 2] = (int) dRound(dClip(255.0 * exp( inverseGamma * log( twoToStopval * bCopy ) ), 0.0, 255.0));
    }
    
    int numpixels;
    double sse, sseR, sseG, sseB;
    calculateSSEnoSaturationRGB(&m_ldrRef[0], &m_ldrSrc[0], size, numpixels, sse, sseR, sseG, sseB);
    totpixels += numpixels;
    totsse += sse;
    m_sse[R_COMP] += sseR;
    m_sse[G_COMP] += sseG;
    m_sse[B_COMP] += sseB;
  }
  
  if(totpixels == 0)
    MSE = 65504.0*65504.0;  // No nonsaturated values. Will report maximum average error.
  else {
    MSE = totsse/(3.0*totpixels); // 3.0 is to average over R, G and B.
    for (int c = R_COMP; c <= B_COMP; c++)
      m_mse[c] = m_sse[c]/(totpixels);
  }
  
  return MSE;
}

double DistortionMetricmPSNR::calculateErrorFast(Frame* inp0, Frame* inp1)
{
  static const float gamma = 2.20f; // Could this be parameterized?
  static const float inverseGamma = 1.0f / gamma;
  static const float logStop = log(2.0f) * inverseGamma;
  static const float stopScale = 255.0f * exp(-34.0f * logStop);
  int totpixels = 0;
  double totsse = 0.0;
  double MSE = 0.0;
  imgpel virtPicR0, virtPicG0, virtPicB0;
  imgpel virtPicR1, virtPicG1, virtPicB1;
  float twoToStopval;
  int rDiff, gDiff, bDiff;
  
  float *inp0Comp0 = inp0->m_floatComp[0];
  float *inp0Comp1 = inp0->m_floatComp[1];
  float *inp0Comp2 = inp0->m_floatComp[2];
  
  float *inp1Comp0 = inp1->m_floatComp[0];
  float *inp1Comp1 = inp1->m_floatComp[1];
  float *inp1Comp2 = inp1->m_floatComp[2];
  
  m_sse[R_COMP] = 0.0;
  m_sse[G_COMP] = 0.0;
  m_sse[B_COMP] = 0.0;
  
  for(int x = 0; x < inp0->m_compSize[R_COMP]; x++) {
    // Clamp values to [0, 65504] to avoid negative light and to
    // make sure infinities are not messing too much with the values,
    // since they are clamped to the largest possible half value.
    float rOrig = fClip(*inp0Comp0++, 0.0f, 65504.0f);
    float gOrig = fClip(*inp0Comp1++, 0.0f, 65504.0f);
    float bOrig = fClip(*inp0Comp2++, 0.0f, 65504.0f);
    
    float rCopy = fClip(*inp1Comp0++, 0.0f, 65504.0f);
    float gCopy = fClip(*inp1Comp1++, 0.0f, 65504.0f);
    float bCopy = fClip(*inp1Comp2++, 0.0f, 65504.0f);
    
    for(int stops = 0; stops<50; stops++) {
      twoToStopval =  stopScale * exp((float) (stops * logStop));
      
      // Create virtual photographs
      // interesting that exp( y * log (x)) seems faster that pow (x, y)
      // virtPicR0 = (imgpel) float2int(fClip( twoToStopval * pow( rOrig, inverseGamma ), 0.0f, 255.0f));
      // Question: Why is rounding needed? Couldn't we just use full precision?
      virtPicR0 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( rOrig )), 0.0f, 255.0f));
      virtPicG0 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( gOrig )), 0.0f, 255.0f));
      virtPicB0 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( bOrig )), 0.0f, 255.0f));
      
      if (!(virtPicR0==0 && virtPicG0==0 && virtPicB0==0) && !((virtPicR0 == 255 || virtPicG0 == 255 || virtPicB0 == 255))) {
        // Neither black nor saturated. Calculate error and add.
        virtPicR1 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( rCopy )), 0.0f, 255.0f));
        virtPicG1 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( gCopy )), 0.0f, 255.0f));
        virtPicB1 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( bCopy )), 0.0f, 255.0f));
        
        rDiff = virtPicR0 - virtPicR1;
        gDiff = virtPicG0 - virtPicG1;
        bDiff = virtPicB0 - virtPicB1;
        
        //if (rDiff != 0 || gDiff != 0 || bDiff != 0) {
          // Accumulate error in RGB
        m_sse[R_COMP] += (double) rDiff * rDiff;
        m_sse[G_COMP] += (double) gDiff * gDiff;
        m_sse[B_COMP] += (double) bDiff * bDiff;
        //}
        
        totpixels++;
      }
    }
  }
  totsse  = m_sse[R_COMP] + m_sse[G_COMP] + m_sse[B_COMP];
  
  if(totpixels == 0)
    MSE = 65504.0 * 65504.0;  // No nonsaturated values. Will report maximum average error.
  else {
    MSE = totsse/(3.0 * totpixels); // 3.0 is to average over R, G and B.
    for (int c = R_COMP; c <= B_COMP; c++)
      m_mse[c] = m_sse[c]/(totpixels);
  }
  
  return MSE;
}

double DistortionMetricmPSNR::calculateErrorYUV(Frame* inp0, Frame* inp1)
{
  static const float gamma = 2.20f; // Could this be parameterized?
  static const float inverseGamma = 1.0f / gamma;
  
  int totpixels = 0;
  double totsse = 0.0;
  double MSE = 0.0;
  int x;
  
  int size = inp0->m_compSize[R_COMP];
  
  // re-allocate memory for low dynamic range images (virtual photographs)
  // if size changed for some odd reason

  m_compSize = size;
  
  m_ldrSrc.resize(3 * m_compSize);
  m_ldrRef.resize(3 * m_compSize);
  
  float *inp0Comp0 = inp0->m_floatComp[0];
  float *inp0Comp1 = inp0->m_floatComp[1];
  float *inp0Comp2 = inp0->m_floatComp[2];
  
  float *inp1Comp0 = inp1->m_floatComp[0];
  float *inp1Comp1 = inp1->m_floatComp[1];
  float *inp1Comp2 = inp1->m_floatComp[2];
  
  m_sse[R_COMP] = 0.0;
  m_sse[G_COMP] = 0.0;
  m_sse[B_COMP] = 0.0;
  
  for(int stops = 0; stops<50; stops++) {
    // Now create the two LDR images
    int stopVal = stops - 34;
    float twoToStopval = exp((float) stopVal * log(2.0f));
    for(x = 0; x < size; x++) {
      // Clamp values to [0, 65504] to avoid negative light and to
      // make sure infinities are not messing too much with the values,
      // since they are clamped to the largest possible half value.
      float rOrig = fClip(inp0Comp0[x], 0.0f, 65504.0f);
      float gOrig = fClip(inp0Comp1[x], 0.0f, 65504.0f);
      float bOrig = fClip(inp0Comp2[x], 0.0f, 65504.0f);
      
      float rCopy = fClip(inp1Comp0[x], 0.0f, 65504.0f);
      float gCopy = fClip(inp1Comp1[x], 0.0f, 65504.0f);
      float bCopy = fClip(inp1Comp2[x], 0.0f, 65504.0f);
      
      
      // Create virtual photographs
      m_ldrSrc[x*3 + 0] = (int) dRound(fClip(255.0f * exp( inverseGamma * log( twoToStopval * rOrig ) ), 0.0f, 255.0f));
      m_ldrSrc[x*3 + 1] = (int) dRound(fClip(255.0f * exp( inverseGamma * log( twoToStopval * gOrig ) ), 0.0f, 255.0f));
      m_ldrSrc[x*3 + 2] = (int) dRound(fClip(255.0f * exp( inverseGamma * log( twoToStopval * bOrig ) ), 0.0f, 255.0f));
      
      m_ldrRef[x*3 + 0] = (int) dRound(fClip(255.0f * exp( inverseGamma * log( twoToStopval * rCopy ) ), 0.0f, 255.0f));
      m_ldrRef[x*3 + 1] = (int) dRound(fClip(255.0f * exp( inverseGamma * log( twoToStopval * gCopy ) ), 0.0f, 255.0f));
      m_ldrRef[x*3 + 2] = (int) dRound(fClip(255.0f * exp( inverseGamma * log( twoToStopval * bCopy ) ), 0.0f, 255.0f));
    }
    
    int numpixels;
    double sse, sseY, sseU, sseV;
    calculateSSEnoSaturationYUV(&m_ldrRef[0], &m_ldrSrc[0], size, numpixels, sse, sseY, sseU, sseV);
    totpixels += numpixels;
    totsse += sse;
    m_sse[Y_COMP] += sseY;
    m_sse[U_COMP] += sseU;
    m_sse[V_COMP] += sseV;
  }
  
  if(totpixels == 0)
    MSE = 65504.0 * 65504.0;  // No nonsaturated values. Will report maximum average error.
  else {
    MSE = totsse/(3.0*totpixels); // 3.0 is to average over R, G and B.
    for (int c = R_COMP; c <= B_COMP; c++)
      m_mse[c] = m_sse[c]/(totpixels);
  }
  
  return MSE;
}

double DistortionMetricmPSNR::calculateErrorYUVfast(Frame* inp0, Frame* inp1)
{
  static const float gamma = 2.20f; // Could this be parameterized?
  static const float inverseGamma = 1.0f / gamma;
  static const float logStop = log(2.0f) * inverseGamma;
  static const float stopScale = 255.0f * exp(-34.0f * logStop);
  
  int totpixels = 0;
  double totsse = 0.0;
  imgpel virtPicR0, virtPicG0, virtPicB0;
  imgpel virtPicR1, virtPicG1, virtPicB1;
  float twoToStopval, error;
  int rDiff, gDiff, bDiff;
  int64 sseI64 = 0;
  
  float *inp0Comp0 = inp0->m_floatComp[0];
  float *inp0Comp1 = inp0->m_floatComp[1];
  float *inp0Comp2 = inp0->m_floatComp[2];
  
  float *inp1Comp0 = inp1->m_floatComp[0];
  float *inp1Comp1 = inp1->m_floatComp[1];
  float *inp1Comp2 = inp1->m_floatComp[2];
  
  m_sse[Y_COMP] = 0.0;
  m_sse[U_COMP] = 0.0;
  m_sse[V_COMP] = 0.0;
  
  for(int x = 0; x < inp0->m_compSize[R_COMP]; x++) {
    // Clamp values to [0, 65504] to avoid negative light and to
    // make sure infinities are not messing too much with the values,
    // since they are clamped to the largest possible half value.
    float rOrig = fClip(*inp0Comp0++, 0.0f, 65504.0f);
    float gOrig = fClip(*inp0Comp1++, 0.0f, 65504.0f);
    float bOrig = fClip(*inp0Comp2++, 0.0f, 65504.0f);
    
    float rCopy = fClip(*inp1Comp0++, 0.0f, 65504.0f);
    float gCopy = fClip(*inp1Comp1++, 0.0f, 65504.0f);
    float bCopy = fClip(*inp1Comp2++, 0.0f, 65504.0f);
    
    for(int stops = 0; stops<50; stops++) {
      twoToStopval =  stopScale * exp((float) (stops * logStop));
      //printf("value %7.3f %7.3f %7.3f\n", twoToStopval,logStop, exp( inverseGamma * log( rOrig )));
      
      // Create virtual photographs
      // interesting that exp( y * log (x)) seems faster that pow (x, y)
      //virtPicR0 = (imgpel) float2int(fClip( twoToStopval * pow( rOrig, inverseGamma ), 0.0f, 255.0f));
      // Question: Why is rounding needed? Couldn't we just use full precision?
      virtPicR0 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( rOrig )), 0.0f, 255.0f));
      virtPicG0 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( gOrig )), 0.0f, 255.0f));
      virtPicB0 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( bOrig )), 0.0f, 255.0f));
      
      if (!(virtPicR0==0 && virtPicG0==0 && virtPicB0==0) && !((virtPicR0 == 255 || virtPicG0 == 255 || virtPicB0 == 255))) {
        // Neither black nor saturated. Calculate error and add.
        virtPicR1 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( rCopy )), 0.0f, 255.0f));
        virtPicG1 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( gCopy )), 0.0f, 255.0f));
        virtPicB1 = (imgpel) float2int(fClip( twoToStopval * exp( inverseGamma * log( bCopy )), 0.0f, 255.0f));
        
        rDiff = virtPicR0 - virtPicR1;
        gDiff = virtPicG0 - virtPicG1;
        bDiff = virtPicB0 - virtPicB1;
        
        if (rDiff != 0 || gDiff != 0 || bDiff != 0) {
          // Accumulate error in RGB
          sseI64 += rDiff * rDiff;
          sseI64 += gDiff * gDiff;
          sseI64 += bDiff * bDiff;
          
          // Now do the same for Y Cb Cr. Note that since we are using linear transforms, we can just
          // transform the error instead and save on computations.
          // Y error
          error  =  0.2126f * rDiff + 0.7152f * gDiff + 0.0722f * bDiff;
          //error  = ySample0 - ySample1;
          m_sse[Y_COMP]  += error * error;
          
          // CbError
          error = -0.1146f * rDiff - 0.3854f * gDiff + 0.5000f * bDiff;
          //error  = cbSample0 - cbSample1;
          m_sse[U_COMP] += error * error;
          
          // CrError
          error =	0.5000f * rDiff - 0.4542f * gDiff - 0.0458f * bDiff;
          //error  = crSample0 - crSample1;
          m_sse[V_COMP] += error * error;
        }
        
        totpixels++;
      }
    }
  }
  totsse  = (double) sseI64;
  
  if(totpixels == 0) {
    // set also m_sse/m_mse to avoid errors;
    for (int c = Y_COMP; c <= V_COMP; c++) {
      m_sse[c] = 0.0;
      m_mse[c] = m_sse[c];
    }
    return( 65504.0 * 65504.0 );  // No nonsaturated values. Will report maximum average error.
  }
  else {
    for (int c = Y_COMP; c <= V_COMP; c++)
      m_mse[c] = m_sse[c]/(totpixels);
    return (totsse/(3.0 * totpixels)); // 3.0 is to average over R, G and B.
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricmPSNR::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_format.m_colorSpace != CM_RGB) {
      printf("mPSNR only supported for RGB data.\n");
    }
    else {
      if (inp0->m_isFloat == TRUE) {
        // floating point data
        if (m_enableComponentmPSNR == TRUE) {
          double mseTotal = calculateErrorYUVfast(inp0, inp1);
          m_mPSNR = psnr(255.0, 1, mseTotal);
          m_mPSNRStats.updateStats(m_mPSNR);
          for (int c = R_COMP; c < inp0->m_noComponents; c++) {
            m_sseStats[c].updateStats(m_sse[c]);
            m_mseStats[c].updateStats(m_mse[c]);
            m_metric[c] = psnr(255.0, 1, m_mse[c]);
            m_metricStats[c].updateStats(m_metric[c]);
          }
        }
        else {
          double mseTotal = calculateErrorFast(inp0, inp1);
          m_mPSNR = psnr(255.0, 1, mseTotal);
          m_mPSNRStats.updateStats(m_mPSNR);
        }
      }
      else if (inp0->m_bitDepth == 8) {   // 8 bit data
        printf("mPSNR for 8 bit data not implemented. Exiting.\n");
        exit(1);
      }
      else { // 16 bit data
        printf("mPSNR for 16 bit integer data not implemented. Exiting.\n");
        exit(1);
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}
void DistortionMetricmPSNR::computeMetric (Frame* inp0, Frame* inp1, int component)
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

void DistortionMetricmPSNR::reportMetric  ()
{
  if (m_enableComponentmPSNR)
    printf("%9.3f %9.3f %9.3f ", m_metric[Y_COMP], m_metric[U_COMP], m_metric[V_COMP]);
  printf("%8.3f ", m_mPSNR);
}

void DistortionMetricmPSNR::reportSummary  ()
{
  if (m_enableComponentmPSNR)
    printf("%9.3f %9.3f %9.3f ", m_metricStats[Y_COMP].getAverage(), m_metricStats[U_COMP].getAverage(), m_metricStats[V_COMP].getAverage());
  printf("%8.3f ", m_mPSNRStats.getAverage());
}

void DistortionMetricmPSNR::reportMinimum  ()
{
  if (m_enableComponentmPSNR)
    printf("%9.3f %9.3f %9.3f ", m_metricStats[Y_COMP].minimum, m_metricStats[U_COMP].minimum, m_metricStats[V_COMP].minimum);
  printf("%8.3f ", m_mPSNRStats.minimum);
}

void DistortionMetricmPSNR::reportMaximum  ()
{
  if (m_enableComponentmPSNR)
    printf("%9.3f %9.3f %9.3f ", m_metricStats[Y_COMP].maximum, m_metricStats[U_COMP].maximum, m_metricStats[V_COMP].maximum);
  printf("%8.3f ", m_mPSNRStats.maximum);
}

void DistortionMetricmPSNR::printHeader()
{
  if (m_isWindow == FALSE ) {
    if (m_enableComponentmPSNR) {
      printf(" mPSNRY   "); // 10
      printf(" mPSNRU   "); // 10
      printf(" mPSNRV   "); // 10
    }
    printf(" mPSNR   "); // 9
  }
  else {
    if (m_enableComponentmPSNR) {
      printf(" wmPSNRY  "); // 10
      printf(" wmPSNRU  "); // 10
      printf(" wmPSNRV  "); // 10
    }
    printf(" wmPSNR  "); // 9
  }
}

void DistortionMetricmPSNR::printSeparator(){
  if (m_enableComponentmPSNR) {
    printf("----------");
    printf("----------");
    printf("----------");
  }
  printf("---------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
