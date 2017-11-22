/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = DML UBC
 * <ORGANIZATION> = DML UBC
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, DML UBC
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
 * \file DistortionMetricVIF.cpp
 *
 * \brief
 *    This is a C++ implementation of the Visual Information Fidelity (VIF) measure.
 *    This implementation was prepared by the Digital Multimedia Lab (DML), 
 *    at the University of British Columbia, BC, Canada
 *    The original code in Matlab is available at:
 *          http://live.ece.utexas.edu/research/quality/VIF.htm
 *
 * \authors
 *     - Hamid Reza Tohidypour        <htohidyp@ece.ubc.ca>
 *     - Maryam Azimi                 <maryama@ece.ubc.ca>
 *     - Mahsa T. Pourazad            <mahsa.pourazad@gmail.com>
 *     - Panos Nasiopoulos            <panos.p.n@ieee.org>
 *************************************************************************************
 */


#include "DistortionMetricVIF.H"
#include "ColorSpaceMatrix.H"
#include <math.h>
#include <vector>
#include "Eigenvalue.H"
#include <iostream>
#include <string.h>


DistortionMetricVIF::DistortionMetricVIF(const FrameFormat *format, VIFParams *vifParams)
:DistortionMetric()
{
  m_blockLength = 3; // M parameter for the block length
  m_blockSize = m_blockLength * m_blockLength; // M * M;
  m_vifFrame = 0; 
  m_vifFrameStats.reset();
  m_vifYBitDepth = vifParams->m_vifBitDepth;
  
  m_transferFunction = TransferFunction::create(TF_PQ, TRUE, 1.0, 1.0, 0.0, 1.0, TRUE);
}

DistortionMetricVIF::~DistortionMetricVIF()
{
}

// Private functions

/* 
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ;;;  
 ;;;  Convolution Functions below were ported from code by Eero Simoncelli
 ;;;  Description: General convolution code for 2D images
 ;;;  Original Creation Date: Spring, 1987.
 ;;;            in the upper-left corner of a filter with odd Y and X dimensions.
 ;;;  ----------------------------------------------------------------
 ;;;    Object-Based Vision and Image Understanding System (OBVIUS),
 ;;;      Copyright 1988, Vision Science Group,  Media Laboratory,  
 ;;;              Massachusetts Institute of Technology.
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 */


/*
 --------------------------------------------------------------------
 Correlate FILT with IMAGE, subsampling according to START, STEP, and
 STOP parameters, with values placed into RESULT array.  RESULT
 dimensions should be ceil((stop-start)/step).  TEMP should be a
 pointer to a temporary double array the size of the filter.
 EDGES is a string specifying how to handle boundaries -- see edges.c.
 The convolution is done in 9 sections, where the border sections use
 specially computed edge-handling filters (see edges.c). The origin 
 of the filter is assumed to be (floor(xFDim/2), floor(yFDim/2)).
 ------------------------------------------------------------------------ */

/* abstract out the inner product computation */
void DistortionMetricVIF::innerProduct(int XCNR,int YCNR,int xDim, int xFDim, int filtSize, int resPos, double* result,  double* temp, double* image)
{
  double sum       = 0.0; 
  int    imPos     = YCNR * xDim + XCNR;
  int    filtPos   = 0;
  int    xFiltStop = xFDim;
  for (;  xFiltStop <= filtSize; imPos += (xDim - xFDim), xFiltStop += xFDim) {
    for (; filtPos < xFiltStop; filtPos++, imPos++) {
      sum+= image[imPos] * temp[filtPos]; 
    }
  }
  result[resPos] = sum;
}

int DistortionMetricVIF::internalReduce(double* image, int xDim, int yDim, 
                             const double *filt, double *temp, int xFDim, int yFDim,
                             int xStart, int xStep, int xStop, 
                             int yStart, int yStep, int yStop,
                             double* result)
{ 
  int xPos     = xFDim * yFDim;
  int filtSize = xFDim * yFDim;
  int yPos, resPos;
  int yCtrStop = yDim - ((yFDim == 1) ? 0 : yFDim);
  int xCtrStop = xDim - ((xFDim == 1) ? 0 : xFDim);
  int xResDim = (xStop - xStart + xStep - 1) / xStep;
  int xCtrStart = ((xFDim == 1) ? 0 : 1);
  int yCtrStart = ((yFDim == 1) ? 0 : 1);
  int xFMid = xFDim >> 1;
  int yFMid = yFDim >> 1;
  int baseResPos;
  // fptr reflect = edge_function(edges);  /* look up edge-handling function */
  //  if (!reflect) return(-1);
  
  /* shift start/stop coords to filter upper left hand corner */
  xStart -= xFMid;   
  yStart -= yFMid;
  xStop  -= xFMid;   
  yStop  -= yFMid;
  
  xCtrStop = iMin(xCtrStop, (int) xStop);
  yCtrStop = iMin(yCtrStop, (int) yStop);
  
  /* TOP ROWS */
  for (resPos = 0, yPos = yStart; yPos < yCtrStart; yPos += yStep)  {
    /* TOP-LEFT CORNER */
    for (xPos = xStart; xPos < xCtrStart; xPos += xStep, resPos++)    {
      reflect1(filt, xFDim, yFDim, xPos - 1, yPos - 1, temp, 0);
      innerProduct(0, 0, xDim, xFDim, filtSize, resPos, result, temp, image);
    }
    
    reflect1(filt, xFDim,yFDim,0,yPos-1,temp, 0);
    
    /* TOP EDGE */
    for (; xPos < xCtrStop; xPos += xStep, resPos++) 
      innerProduct(xPos, 0, xDim, xFDim, filtSize, resPos, result, temp, image);
    
    /* TOP-RIGHT CORNER */
    for (;xPos < xStop; xPos += xStep, resPos++) {
      reflect1(filt, xFDim, yFDim, xPos - xCtrStop + 1, yPos - 1, temp, 0);
      innerProduct(xCtrStop, 0, xDim, xFDim, filtSize, resPos, result, temp, image);
    }
  } /* end TOP ROWS */   
  
  yCtrStart = yPos;			      /* hold location of top */
  
  /* LEFT EDGE */
  for (baseResPos = resPos, xPos = xStart; xPos < xCtrStart; xPos += xStep, baseResPos++) {
    reflect1(filt, xFDim, yFDim, xPos - 1, 0, temp, 0);
    for (yPos = yCtrStart, resPos = baseResPos; yPos < yCtrStop; yPos += yStep, resPos += xResDim)
      innerProduct(0, yPos, xDim, xFDim, filtSize, resPos, result, temp, image);
  }
  
  reflect1(filt,xFDim,yFDim,0,0,temp, 0);
  /* CENTER */
  for (; xPos < xCtrStop; xPos += xStep, baseResPos++) {
    for (yPos = yCtrStart, resPos = baseResPos; yPos < yCtrStop; yPos += yStep, resPos += xResDim)
      innerProduct(xPos, yPos, xDim, xFDim, filtSize, resPos, result, temp, image);
  }
  /* RIGHT EDGE */
  for (; xPos < xStop; xPos += xStep, baseResPos++)  {
    reflect1(filt, xFDim, yFDim, xPos - xCtrStop + 1, 0, temp, 0);    
    for (yPos = yCtrStart, resPos = baseResPos; yPos<yCtrStop; yPos += yStep, resPos += xResDim)
      innerProduct(xCtrStop, yPos, xDim, xFDim, filtSize, resPos, result, temp, image);
  }
  
  /* BOTTOM ROWS */
  for (resPos -= (xResDim - 1); yPos < yStop; yPos += yStep) {
    /* BOTTOM-LEFT CORNER */
    for (xPos = xStart; xPos < xCtrStart; xPos+=xStep, resPos++) {
      reflect1(filt, xFDim, yFDim, xPos - 1, yPos - yCtrStop + 1, temp, 0);
      innerProduct(0, yCtrStop, xDim, xFDim, filtSize, resPos, result,  temp,  image);
    }
    
    reflect1(filt, xFDim, yFDim, 0, yPos - yCtrStop + 1, temp, 0);
    /* BOTTOM EDGE */
    for (;xPos < xCtrStop; xPos += xStep, resPos++) 
      innerProduct(xPos, yCtrStop, xDim, xFDim, filtSize, resPos, result, temp, image);
    
    /* BOTTOM-RIGHT CORNER */
    for (; xPos < xStop; xPos += xStep, resPos++) {
      reflect1(filt, xFDim, yFDim, xPos - xCtrStop + 1, yPos - yCtrStop + 1,temp, 0);
      innerProduct(xCtrStop, yCtrStop, xDim, xFDim, filtSize, resPos, result, temp, image);
    }
  } /* end BOTTOM */
  return(0);
} /* end of internalReduce */


/*
 --------------------------------------------------------------------
 Upsample IMAGE according to START,STEP, and STOP parameters and then
 convolve with FILT, adding values into RESULT array.  IMAGE
 dimensions should be ceil((stop-start)/step).  See
 description of internalReduce (above).
 
 WARNING: this subroutine destructively modifies the RESULT array!
 ------------------------------------------------------------------------ */

/* abstract out the inner product computation */

int DistortionMetricVIF::reflect1( const double * filt, int xDim, int yDim, int xPos, int yPos, double *result, int rOrE)
{
  int filtSz = xDim * yDim;
  int yFilt,xFilt, yRes, xRes;
  int xBase  = (xPos > 0) ? (xDim - 1):0;
  int yBase  = xDim * ((yPos > 0) ? (yDim - 1) : 0); 
  int xOverhang = (xPos > 0) ? (xPos - 1) : ((xPos < 0) ? (xPos + 1) : 0);
  int yOverhang = xDim * ((yPos > 0) ? (yPos - 1) : ((yPos < 0) ? (yPos + 1) : 0));
  int i;
  int mXPos = (xPos < 0) ? (xDim >> 1) : ((xDim - 1) >> 1);
  int mYPos = xDim * ((yPos < 0) ? (yDim >> 1) : ((yDim - 1) >> 1));
  
  for (i = 0; i < filtSz; i++) 
    result[i] = 0.0;
  
  if (rOrE == 0) {
    for (yFilt = 0, yRes = yOverhang - yBase; yFilt < filtSz; yFilt += xDim, yRes += xDim) {
      for (xFilt = yFilt, xRes = xOverhang - xBase; xFilt < yFilt + xDim; xFilt++, xRes++) {
        result[iAbs(yBase - iAbs(yRes)) + iAbs(xBase - iAbs(xRes))] += filt[xFilt];
      }
    }
  }
  else {
    yOverhang = iAbs(yOverhang); 
    xOverhang = iAbs(xOverhang);
    for (yRes = yBase, yFilt = yBase - yOverhang; yFilt > yBase - filtSz; yFilt -= xDim, yRes -= xDim)    {
      for (xRes = xBase, xFilt = xBase - xOverhang; xFilt > xBase - xDim; xRes--, xFilt--)
        result[iAbs(yRes) + iAbs(xRes)] += filt[iAbs(yFilt) + iAbs(xFilt)];
      
      if ((xOverhang != mXPos) && (xPos != 0)) {
        for (xRes = xBase, xFilt = xBase - 2 * mXPos + xOverhang; xFilt > xBase - xDim; xRes--, xFilt--)
          result[iAbs(yRes) + iAbs(xRes)] += filt[iAbs(yFilt) + iAbs(xFilt)];
      }
    }
    if ((yOverhang != mYPos) && (yPos != 0)) {
      for (yRes = yBase, yFilt = yBase - 2 * mYPos + yOverhang; yFilt > yBase - filtSz; yFilt -= xDim, yRes -= xDim)  {
        for (xRes = xBase, xFilt = xBase - xOverhang; xFilt > xBase - xDim; xRes--, xFilt--)
          result[iAbs(yRes) + iAbs(xRes)] += filt[iAbs(yFilt) + iAbs(xFilt)];
        
        if  ((xOverhang != mXPos) && (xPos != 0)) {
          for (xRes=xBase, xFilt = xBase - 2 * mXPos + xOverhang; xFilt > xBase - xDim; xRes--, xFilt--)
            result[iAbs(yRes) + iAbs(xRes)] += filt[iAbs(yFilt) + iAbs(xFilt)];
        }
      }
    }
  }
  return(0);
  
}


// http://www.sanfoundry.com/cpp-program-finds-inverse-graph-matrix/
// The code is modified for the VIF metric
void DistortionMetricVIF::inverseMatrix(vector<vector<double> > &input, int n, vector<vector<double> >  &output)
{
  vector<vector<double> >  a;
  a.resize(2 * n);
  
  for (int i = 0; i < 2 * n;i++) {
    a[i].resize(2 * n);
    for(int j = 0; j < 2 * n;j++) {
      if(j > n - 1 || i > n - 1)
        a[i][j] = 0;
      else if(i < n && j < n)
        a[i][j] = input[i][j];
    }
  }
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <  2*n; j++) {
      if (j == (i + n)) {
        a[i][j] = 1;
      }
    }
  }
  
  for (int i = n-1; i > 0; i--)  {
    if (a[i-1][1] < a[i][1]) {
      for(int j = 0; j < n * 2; j++) {
        swap(&a[i][j], &a[i - 1][j]);
      }
    }
  }
  
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n * 2; j++) {
      if (j != i) {
        double d = a[j][i] / a[i][i];
        for (int k =0 ; k < n * 2; k++) {
          a[j][k] -= (a[i][k] * d);
        }
      }
    }
  }
  
  for (int i = 0; i < n; i++)  {
    double d = a[i][i];
    for (int j = 0; j < n * 2; j++) {
      a[i][j] /= d;
      if (j > n - 1) {
        output[i][j - n] = a[i][j];
      }
    }
  }
}

//compute mean of a matrix
void DistortionMetricVIF::mean (vector<vector<double> >  &input, int w, int h, double* mcu)
{
  for (int i = 0; i < w; i++)	{
    double sum1 = 0;
    for (int j = 0; j < h; j++)
      sum1 += input[i][j] / ((double) h);
    mcu[i] = (sum1);
  }
}


void DistortionMetricVIF::reshape (vector<vector<double> >  &input, int newW, int newH ,  vector<vector<double> >  &s)
{
  int k = 0;
  for (int i = 0; i < newW; i++)	{
    for (int j = 0; j < newH; j++) {
      s[i][j] = input[0][k++];
    }
  }
}


void DistortionMetricVIF::vectorMax (int* input, int size , int imax)
{
  imax = input[0];
  for (int j = 0; j < size; j++) {
    if (input[j] > imax) {
      imax = input[j];
    }
  }
}

void DistortionMetricVIF::differenceVector(vector<vector<double> >  &input1, vector<vector<double> >  &input2, int w, int h, vector<vector<double> >  &output)
{
  for (int i = 0; i < w; i++)	{
    output[i].resize(h);
    for (int j = 0; j < h; j++)	{
      output[i][j] = (input1[i][j] - input2[i][j]);
    }
  }
}

void DistortionMetricVIF::invertVector(vector<vector<double> >  &input1,int w, int h, vector<vector<double> >  &output)
{
  
  for (int j = 0; j < h; j++)	{
    output[j].resize(w);
  }
  
  for (int j = 0; j < h; j++)	{
    for (int i = 0; i < w; i++)	{	
      output[j][i]= input1[i][j];
    }
  }
}

// This code should use the already available color conversion code. Please fix

//convert RGB to R'G'B' then compute the non constant luminance 
//This function accepts r:red, g:green and b:blue of the RGB and fist it PQ codes the RGB frame 
//Then it converts the pq coded RGB to luminace based on the color primaires and bit depth of the frame.
double DistortionMetricVIF::rgbPQYuv (double r, double g, double b, int bitdepth,  ColorPrimaries colorPrimaries)
{
  r = m_transferFunction->getInverse(r / 10000.0);
  g = m_transferFunction->getInverse(g / 10000.0);
  b = m_transferFunction->getInverse(b / 10000.0);
  
  double y=0;
  
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
  
  const double *transform = FWD_TRANSFORM[mode][Y_COMP];

  y = transform[0] * r + transform[1] * g + transform[2] * b;
  
  // % Scaling and quantization
  //Taken from ITU. (2002). RECOMMENDATION ITU-R BT.1361.
  y = (219 * y + 16) * (1 << (bitdepth - 8));
  
  return(y);
}


//output=input1*input2
void DistortionMetricVIF::vectorMultiplication(vector<vector<double> >  &input1, int w1,int h1,vector<vector<double> >  &input2, int w2,int h2,  vector<vector<double> >  &output)
{
  for (int i = 0;i < w1; i++)	{
    output[i].resize(h2);
    for (int i1 = 0; i1 < h2; i1++)	{
      double sum = 0;
      for (int j = 0; j < h1; j++) {
        sum += (input1[i][j] * input2[j][i1]);
      }
      output [i][i1]= (sum);
    }
  }
}

void DistortionMetricVIF::vectorMultiplicationInverse (vector<vector<double> >  &input1, int w1,int h1,vector<vector<double> >  &input2,  vector<vector<double> >  &output)
{
  int h2 = w1;
  //	int w2=h1;
  for (int i = 0; i < w1; i++) {
    output[i].resize(h2);    
    for (int i1 = 0; i1 < h2; i1++) {
      double sum=0;
      for (int j = 0; j < h1; j++) {
        sum +=(input1[i][j] * input2[i1][j]);
      }
      output [i][i1]= (sum);
    }
  }
}

//output=input1/input2
void DistortionMetricVIF::vectorMultiplyFixNum(vector<vector<double> >  &input1, int w1, int h1,double input2,  vector<vector<double> >  &output)
{
  for (int i = 0; i < w1; i++) {
    output[i].resize(h1);
    for (int j = 0; j < h1; j++) {
      output[i][j]=(input1[i][j] * input2);
    }
  }
}

//output=input1+input2
void DistortionMetricVIF::vectorAddFixNum(vector<vector<double> >  &input1, int w1, int h1,double input2,  vector<vector<double> >  &output)
{
  for (int i = 0; i < w1; i++) {
    output[i].resize(h1);
    for (int j = 0; j < h1; j++) {
      output[i][j]=(input1[i][j]+input2);
    }
  }
}

//output=input1.*input2
void DistortionMetricVIF::vectorMultiplicationSameSize(vector<vector<double> >  &input1, int w1, int h1, vector<vector<double> >  &input2,  vector<vector<double> >  &output)
{
  for (int i = 0; i < w1; i++)	{
    output[i].resize(h1);
    for (int j = 0; j < h1; j++) {
      output[i][j]= (input1[i][j] * input2[i][j]);
    }
  }
}


double DistortionMetricVIF::sumSumLog2GGSSLambdaDivVV(vector<vector<double> >  &g, int w1, int h1, vector<vector<double> >  &ss, double lambda, double eps, vector<vector<double> >  &vv)
{
  double sum = 0.0;
  for (int i = 0; i < w1; i++) {
    for (int j = 0; j < h1; j++) {
      sum+= log(1.0 + g[i][j] * g[i][j] * ss[i][j] * lambda / (vv[i][j] + eps));
    }
  }
  sum /= log(2.0);
  return sum;
}


// temp2=temp2+sum(sum((log2(1+ss.*lambda(j)./(sigma_nsq))))); % reference image information in VIF matlab code
double DistortionMetricVIF::sumSumLog2SSLambdaDivSigma(vector<vector<double> > &ss, int w1, int h1,double lambda, double eps)
{
  double sum = 0.0;
  for (int i = 0; i < w1; i++) {
    for (int j = 0; j < h1; j++) {
      sum += log(1.0 + ss[i][j] * lambda/(eps));  //reference image information
    }
  }
  sum /= log(2.0);
  return sum;
}

//  The function is added for sumSSTempMM temp2=temp2+sum(sum((log2(1+ss.*lambda(j)./(sigma_nsq)))));
void DistortionMetricVIF::sumSSTempMM(vector<vector<double> >  &input1, int w1, int h1, vector<vector<double> >  &input2, double input3, vector<vector<double> >  &output)
{
  output[0].resize(h1);
  for (int j = 0; j < h1; j++)	{
    double sum = 0.0;
    for (int i = 0;i < w1; i++)	{
      sum += (input1[i][j] * input2[i][j]);
    }
    output[0][j]=sum / input3;
  }
}

//output=input1./input2
void DistortionMetricVIF::vectorDivisionSameSize(vector<vector<double> >  &input1, int w1, int h1, vector<vector<double> >  &input2, double input3, vector<vector<double> >  &output)
{
  //input3 is to avoid devision by zero
  for (int i = 0;i < w1; i++)	{
    output[i].resize(h1);
    for (int j = 0; j < h1; j++) {
      output[i][j]= (input1[i][j] / (input2[i][j] + input3));
    }
  }
}

//repmat (double* input, int w, int h, vector<vector<double> >  output)   creates a large matrix output 
//consisting of an w-by-h tiling of copies of input. 
void DistortionMetricVIF::repmat (double* input, int w, int h, vector<vector<double> >  &output)
{
  for (int i = 0; i < w; i++)	{
    output[i].resize(h);
    for (int j = 0; j < h; j++)	{
      output[i][j]= input[i];
    }
  }
}

//
//output (input < compVal) = setVal;
void DistortionMetricVIF::setCompare (double* input, int size, double compVal, double setVal, bool iseq, double *output )
{
  if ( iseq)	{
    for (int i = 0; i < size; i++)	{
      if (input[i] <= compVal)
        output[i] = setVal; 
    }
  }
  else	{
    for (int i = 0; i < size; i++)	{
      if (input[i] < compVal)
        output[i] = setVal;  
    }
  }
}

//output (input < compVal) = setInput (input < compVal);
void DistortionMetricVIF::setCompareWithOther (double *compInput, double *setInput, int size, double compVal, bool iseq, double *output )
{
  if (iseq == TRUE) {
    for (int i = 0; i < size; i++)	{
      if (compInput[i] <= compVal)
        output[i] = setInput[i]; 
    }
  }
  else {
    for (int i = 0; i < size; i++)	{
      if (compInput[i] < compVal)
        output[i] = setInput[i]; 
    }
  }
}

//output=input1-input2
void DistortionMetricVIF::difference (double* input1, double* input2, int size, double* output )
{
  for (int i = 0; i < size; i++)
    output[i] = input1[i] - input2[i];
}

//output=input+num
void DistortionMetricVIF::addFix (double* input, int size, double num, double* output )
{
  for (int i = 0; i < size; i++)
    output[i] = input[i] + num ;
}

//output=input/num
void DistortionMetricVIF::divFix (double* input, int size, double num, double* output )
{
  for (int i = 0; i < size; i++)
    output[i] = input[i] / num ;
}

//output=input/input2
void DistortionMetricVIF::div (double* input, double* input2, int size, double* output )
{
  for (int i = 0; i < size; i++)
    output[i] = input[i] / input2[i] ;
}

//output=input1*input2
void DistortionMetricVIF::multiply (double* input1, double* input2, int size, double num,  double* output )
{
  for (int i = 0; i < size; i++)
    output[i] = input1[i] * input2[i] * num ;
}

void  DistortionMetricVIF::convertOneRowVar ( const double** input, double * output)
{
  int xFDimInput = sizeof(input) / sizeof(*input);
  for (int i = 0; i < xFDimInput; i ++)	{
    for (int j = 0; j < xFDimInput; j ++)		{
      //reshape the filter into matrice
      output[j + i * xFDimInput] =	input[j][i] ;
    }
  }
}



//==========================================

//[ssarr, lArr, cuArr]=refParamsVecGSM(org,subands,M)
//This function computes the parameters of the reference image. This is called by vifvec.m in the matlab code.
void DistortionMetricVIF::refParamsVecGSM (vector<vector<double> > &org,  
                                           int* subbands, 
                                           vector<vector<int> > &lenWh, 
                                           int sizeSubBand, 	  
                                           vector<vector<vector<double> > >  &ssArr,  
                                           vector<vector<double> > &lArr, 
                                           vector<vector<vector<double> > >  &cuArr
                                           ) 
{
  Eigenvalue eg1;  
  for (int subi = 0; subi < sizeSubBand; subi++)	{
    int   sub = subbands[subi];
    
    //force subband size to be multiple of M
    int newSizeYWidth  = (lenWh[sub][0] / m_blockLength) * m_blockLength;
    int newSizeYHeight = (lenWh[sub][1] / m_blockLength) * m_blockLength;
    
    vector<vector<double> > yMM(newSizeYWidth);
    for (int w = 0; w < newSizeYWidth; w++) {
      yMM[w].resize(newSizeYHeight);
      for (int h = 0; h < newSizeYHeight; h++)			{
        yMM[w][h]=org[sub][w + h * lenWh[sub][0]];
      }
    }
    
    //================================
    //Collect MxM blocks. Rearrange each block into an
    //M^2 dimensional vector and collect all such vectors.
    //Collece ALL possible MXM blocks (even those overlapping) from the subband
    vector<vector<double> > temp(m_blockSize);
    int count=0;
    int sizeTempH=0;
    
    for (int k = 0; k < m_blockLength; k++)	{
      for (int j = 0; j < m_blockLength; j++)	{
        int totalSize= (newSizeYWidth - (m_blockLength - 1 - k) - k) * (newSizeYHeight - (m_blockLength - 1 - j) - j);
        temp[count].resize(totalSize);
        sizeTempH = 0;
        for (int w0 = k; w0 < newSizeYWidth - (m_blockLength - 1 - k); w0++) {
          for (int h0 = j; h0 < newSizeYHeight - (m_blockLength - 1 - j); h0++) {
            temp[count][sizeTempH++] = (yMM[w0][h0]);
          }
        }
        count++;
      }
    }
    
    int totalSize = (newSizeYWidth - (m_blockLength - 1)) * (newSizeYHeight - (m_blockLength - 1));
    
    //estimate mean 
    vector<double> mcu(m_blockSize);
    mean(temp, m_blockSize, totalSize, &mcu[0]);
    
    //estimate covariance
    vector<vector<double> > cu1 (m_blockSize);
    vector<vector<double> > cu2 (m_blockSize);
    vector<vector<double> > rep1(m_blockSize);
    
    repmat(&mcu[0], m_blockSize, sizeTempH, rep1);
    differenceVector(temp, rep1, m_blockSize, sizeTempH, cu1);
    vectorMultiplicationInverse (cu1, m_blockSize, sizeTempH, cu1, cu2);
    for (int ii = 0; ii < m_blockSize; ii++)	{
      for (int jj = 0;jj < m_blockSize; jj++) {
        cuArr[sub] [ii][jj]=(cu2[ii][jj] / sizeTempH); //% covariance matrix for U
      }
    }
    
    //================================
    //Collect MxM blocks as above. Use ONLY non-overlapping blocks to
    //calculate the S field
    vector<vector<double> > temp1(m_blockSize);
    
    count = 0;
    int temp1Size = 0;
    for (int k = 0; k < m_blockLength; k++)		{
      for (int j = 0;j < m_blockLength; j++)			{
        temp1Size = 0;
        temp1[count].resize(newSizeYWidth * newSizeYHeight);
        for (int w0 = k; w0 < newSizeYWidth; w0 += m_blockLength) {
          for (int h0 = j; h0 < newSizeYHeight;h0 += m_blockLength) {
            temp1[count][temp1Size]=(yMM[w0][h0]);
            temp1Size++;
          }
        }
        count++;
      }
    }
    
    //Calculate the S field
    vector<vector<double> > inCuCo(m_blockSize);    
    
    for (int i = 0; i < m_blockSize;i++)	{
      inCuCo[i].resize(m_blockSize); 
    }
    inverseMatrix(cuArr[sub], m_blockSize, inCuCo);
    
    vector<vector<double> > ss1(m_blockSize);
    vectorMultiplication(inCuCo, m_blockSize, m_blockSize, temp1, m_blockSize, temp1Size, ss1);
    vector<vector<double> >  ss4(1);
    
    sumSSTempMM (ss1, m_blockSize, temp1Size, temp1, m_blockSize, ss4);
    reshape(ss4,(int)(newSizeYWidth / (double) m_blockLength), (int)(newSizeYHeight / (double) m_blockLength), ssArr[sub]);
    
    //==========================
    //Eigen-decomposition
    vector<double> a(m_blockSize * m_blockSize);
    
    int k = 0;
    for (int i = 0; i < m_blockSize; i++)	{
      for (int j = 0;j < m_blockSize; j++)	{
        a[k++] = (cuArr[sub][i][j]);
      }
    }
    int n = m_blockSize;
    
    vector<double> v(m_blockSize * m_blockSize);
    
    int itMax = 100;
    int rotNum;
    eg1.jacobiEigenvalue ( n,  &a[0], itMax, &v[0],  &lArr[sub][0], itMax, rotNum );
  }
}
//=================================

void DistortionMetricVIF::vifSubEstM (vector<vector<double> >  &org, 
                                      vector<vector<double> >  &dist, 
                                      int* subbands, 
                                      vector<vector<int> > &lenWh, 
                                      int sizeSubBand, 
                                      vector<vector<vector<double> > >  &gAll, 
                                      vector<vector<vector<double> > >  &VvAll, 
                                      vector<vector<int> > &lenWhGAll
                                      )
{
  double tol = 1e-15; 
  for (int i = 0; i < sizeSubBand; i++) {
    int sub = subbands[i];
    int sizeY = lenWh[sub][0] * lenWh[sub][1];
    vector<double> y (sizeY);
    vector<double> yn(sizeY);
    
    memcpy (&y[0], &org[sub][0],  sizeY * sizeof (double));
    memcpy (&yn[0],&dist[sub][0], sizeY * sizeof (double));
    
    //compute the size of the window used in the distortion channel estimation
    int  lev = (int) ceil((sub - 1)/6.0);
    int winSize= (1 << lev) + 1; // pow(2.0, lev) + 1; 
                                 //    double offset=(winSize-1)/2.0;
    vector<double> win      (winSize * winSize);
    vector<double> winNormal(winSize * winSize);
    
	// Compute normal value once and then initialize window arrays
	double normalValue = 1.0 / (double)(winSize * winSize);
    for (int j = 0; j < winSize * winSize; j++) {
      win[j] = 1.0; 
      winNormal[j] = normalValue;
    }
    
    //force subband size to be multiple of M
    int newSizeYWidth  = (lenWh[sub][0] / m_blockLength) * m_blockLength;
    int newSizeYHeight = (lenWh[sub][1] / m_blockLength) * m_blockLength;
    
    vector<double> yMM (newSizeYWidth * newSizeYHeight);
    vector<double> ynMM(newSizeYWidth * newSizeYHeight);
    
    for (int w = 0; w < newSizeYWidth; w++) {
      for (int h = 0; h < newSizeYHeight; h++) {
        yMM [w + h * newSizeYWidth] = y [w + h * lenWh[sub][0]];
        ynMM[w + h * newSizeYWidth] = yn[w + h * lenWh[sub][0]];
      }
    }
    
    //Correlation with downsampling. This is faster than downsampling after
    //computing full correlation.
    int winStepX       = m_blockLength; 
    int winStepY       = m_blockLength;
    int winStartX      = winStepX >> 1; 
    int winStartY      = winStartX;
    int winStopX       = newSizeYWidth  - ((winStepX + 1) >> 1) + 1; 
    int winStopY       = newSizeYHeight - ((winStepY + 1) >> 1) + 1; 
    int newBlockWidth  = newSizeYWidth  / winStepX;
    int newBlockHeight = newSizeYHeight / winStepY;
    int newSizeBlocks  = (newSizeYWidth * newSizeYHeight) / (winStepX * winStepY);
    
    //mean
    int meanXSize = newSizeBlocks;
    vector<double> meanX(meanXSize);
    vector<double> meanY(newSizeBlocks);
    vector<double> temp  (newSizeYWidth * newSizeYHeight);
    
    internalReduce(&yMM[0],  newSizeYWidth, newSizeYHeight, &winNormal[0], &temp[0], winSize, winSize, winStartX, winStepX, winStopX, winStartY, winStepY, winStopY, &meanX[0]);    
    internalReduce(&ynMM[0], newSizeYWidth, newSizeYHeight, &winNormal[0], &temp[0], winSize, winSize, winStartX, winStepX, winStopX, winStartY, winStepY, winStopY, &meanY[0]); 
    
    //cov
    vector<double> yMMYnMM(newSizeYWidth * newSizeYHeight);
    
    multiply (&yMM[0], &ynMM[0], newSizeYWidth * newSizeYHeight, 1.0, &yMMYnMM[0]);
    
    vector<double> covXY (newSizeBlocks);
    vector<double> covXY1(newSizeBlocks);
    vector<double> covXY2(newSizeBlocks);
    
    multiply (&meanX[0], &meanY[0], newSizeBlocks, winSize * winSize, &covXY2[0]);    
    
    internalReduce(&yMMYnMM[0], newSizeYWidth,  newSizeYHeight, &win[0], &temp[0], winSize, winSize, winStartX, winStepX, winStopX, winStartY, winStepY, winStopY, &covXY1[0]); 
    
    difference (&covXY1[0],&covXY2[0], newSizeBlocks, &covXY[0]);    
    //varx
    vector<double> SsX1(newSizeBlocks);
    vector<double> SsX2(newSizeBlocks);
    vector<double> SsX (newSizeBlocks);
    vector<double> yMM_2(newSizeYWidth * newSizeYHeight);
    
    multiply (&yMM[0], &yMM[0], newSizeYWidth * newSizeYHeight, 1.0, &yMM_2[0]);
    internalReduce(&yMM_2[0], newSizeYWidth,  newSizeYHeight, &win[0], &temp[0], winSize, winSize,	winStartX, winStepX, winStopX, winStartY, winStepY, winStopY, &SsX1[0]); 
    
    multiply (&meanX[0], &meanX[0],  newSizeBlocks, winSize * winSize,  &SsX2[0]);
    difference (&SsX1[0], &SsX2[0],  newSizeBlocks, &SsX[0]);
    
    //vary 
    vector<double> SsY1 (newSizeBlocks);
    vector<double> SsY2 (newSizeBlocks);
    vector<double> SsY  (newSizeBlocks);
    vector<double> ynMM2(newSizeYWidth * newSizeYHeight);
    
    multiply (&ynMM[0], &ynMM[0], newSizeYWidth * newSizeYHeight, 1.0, &ynMM2[0]);
    
    internalReduce(&ynMM2[0], newSizeYWidth,  newSizeYHeight, &win[0], &temp[0],  winSize,  winSize, winStartX,  winStepX,  winStopX, winStartY, winStepY, winStopY, &SsY1[0]); 
    
    multiply   (&meanY[0], &meanY[0],  newSizeBlocks, winSize * winSize,  &SsY2[0] );
    difference (&SsY1[0],   &SsY2[0],  newSizeBlocks, &SsY[0]);
    
    // get rid of numerical problems, very small negative numbers, or very
    // small positive numbers, or other theoretical impossibilities.
    setCompare(&SsX[0],  newSizeBlocks, 0, 0, FALSE, &SsX[0]);
    setCompare(&SsY[0],  newSizeBlocks, 0, 0, FALSE, &SsY[0]);
    
    // Regression 
    vector<double> gNum(newSizeBlocks);
    vector<double> g   (newSizeBlocks);
    
    addFix(&SsX[0],  newSizeBlocks, tol,&gNum[0]);
    div (&covXY[0], &gNum[0], newSizeBlocks, &g[0]);
    
    //Variance of error in regression
    vector<double> vv    (newSizeBlocks);
    vector<double> vvNum1(newSizeBlocks);
    vector<double> vvNum (newSizeBlocks);
    
    multiply   (&g[0],    &covXY[0],  newSizeBlocks, 1.0, &vvNum1[0]);
    difference (&SsY[0], &vvNum1[0],  newSizeBlocks, &vvNum[0]);
    divFix     (&vvNum[0],            newSizeBlocks, winSize * winSize, &vv[0]);
    
    
    // get rid of numerical problems, very small negative numbers, or very
    // small positive numbers, or other theoretical impossibilities.
    setCompare          (&SsX[0],           newSizeBlocks, tol,   0, FALSE, &g[0]);
    setCompareWithOther (&SsX[0],  &SsY[0], newSizeBlocks, tol,      FALSE, &vv[0]);
    setCompare          (&SsX[0],           newSizeBlocks, tol,   0, FALSE, &SsX[0]);
    setCompare          (&SsY[0],           newSizeBlocks, tol,   0, FALSE, &g[0]);
    setCompare          (&SsY[0],           newSizeBlocks, tol,   0, FALSE, &vv[0]);
    
    //constrain g to be non-negative. 
    setCompareWithOther (&g[0],    &SsY[0], newSizeBlocks,        0, FALSE, &vv[0]);
    setCompare          (&g[0],             newSizeBlocks,   0,   0, FALSE, &g[0]);
    
    //take care of numerical errors, vv could be very small negative
    setCompare          (&vv[0],            newSizeBlocks, tol, tol,  TRUE, &vv[0]);
    
    for (int jj = 0; jj < newBlockWidth; jj++) {
      for (int h = 0; h < newBlockHeight; h++) {
        gAll [sub][jj][h] = g [jj + h * newBlockWidth];
        VvAll[sub][jj][h] = vv[jj + h * newBlockWidth];
      }
    }
  }
}


//Construct a steerable pyramid on matrix IM.  Convolutions are performed using spatial filters.
int DistortionMetricVIF::buildSteerablePyramidLevels(double* lo0, 
                                                     int ht, 
                                                     int width, 
                                                     int height, 
                                                     vector<vector<double> > &pyr, 
                                                     int max_ht, 
                                                     int pyrElement, 
                                                     vector<vector<double> >  &pyrArr, 
                                                     vector<bool> &subbandUsed)
{
  int bFiltsZ  = 0;
  int bFiltsZ2 = sizeof(bfilts)  / sizeof(*bfilts); 
  int bFiltsZ1 = sizeof(*bfilts) / sizeof(double);
  bFiltsZ      = (int) sqrt ((double) bFiltsZ1);

  int widthHalf  = (width  + 1) >> 1; 
  int heightHalf = (height + 1) >> 1;
  
  vector<double>  bands;
  vector<double>  band;
  vector<double>  bind;
  vector<double>  temp;
  vector<double>  lo;
  vector<double>  bFiltsOneRow;
  
  bands.resize( width * height * bFiltsZ2 );
  band.resize ( width * height );
  bind.resize ( 2 + bFiltsZ2 * 2);  
  temp.resize ( width * height );
  lo.resize   ( widthHalf * heightHalf );
  bFiltsOneRow.resize  (bFiltsZ * bFiltsZ);
    
  if (ht <=0)	{
    if (subbandUsed[pyrElement] == TRUE)		{
      for (int i = 0; i < width * height; i++) {
        pyrArr[pyrElement][i] = lo0 [i]; 
      }
    }
  }
  else {			
    for (int b = 0; b < bFiltsZ2; b ++ )		{     
      for (int k = 0, i = 0; i < bFiltsZ  ; i ++) {
        for (int j = i; j < i + bFiltsZ * bFiltsZ ; j += bFiltsZ) {
          //reshape the filter
          bFiltsOneRow[j] = bfilts[b][k];
          k++;
        }
      }
      
      internalReduce(lo0, width, height, &bFiltsOneRow[0], &temp[0],  bFiltsZ,  bFiltsZ, 0, 1, width, 0,  1, height,	&band[0]);
      
      if (subbandUsed [pyrElement]==TRUE) {
        for (int i = 0; i < width * height; i++) {
          pyrArr[pyrElement][i] = band [i]; 
        }
      }
      pyrElement--;
      
      bind[ b * 2    ] = width;
      bind[ b * 2 + 1] = height;
    }
    
    int lioFiltSize = sizeof(lofilt) / sizeof(*lofilt);
    vector<double>  loFiltOneRow( lioFiltSize * lioFiltSize);
    
    for (int i = 0; i < lioFiltSize ; i ++) {
      for (int j = 0; j < lioFiltSize ; j ++) {
        //reshape the filter
        loFiltOneRow[j + i * lioFiltSize]=	lofilt[j][i] ;
      }
    }
    
    internalReduce(lo0, width,  height, &loFiltOneRow[0], &temp[0],  lioFiltSize,  lioFiltSize, 0,  2, width, 0,  2,  height, &lo[0]);    
    buildSteerablePyramidLevels(&lo[0], ht - 1, widthHalf, heightHalf, pyr, max_ht, pyrElement, pyrArr, subbandUsed);
  }
  
  return(0);
}

int DistortionMetricVIF::maxPyrHt(int imSizeW, int imSizeH, int filtsize)
{
  int height = 0;
  if (imSizeW < filtsize || imSizeH < filtsize) {
    height = 0;
  }
  else  {
    height = 1 + maxPyrHt(imSizeW >> 1, imSizeH >> 1, filtsize); 
  }
  return (height);
}

//This function is the main function which computes the VIF values for the input frames.
//Advanced Usage:
//  Users may want to modify the parameters in the code. 
//   (1) Modify sigma_nsq to find tune for your image dataset.
//   (2) MxM is the block size that denotes the size of a vector used in the
//   GSM model.
//   (3) subbands included in the computation, subands were modified for c++
//========================================================================
void DistortionMetricVIF::computeMetric (Frame* inp0, Frame* inp1)
{
  
  double sigma_nsq=0.4;
  //int subbands []={4 ,7, 10, 13, 16, 19, 22, 25}; //Matlab Subbands
#if spFilters5
  int subbands [] = {3 ,6, 9, 12, 15, 18, 21, 24}; //C++ sub bands
#else
  int subbands [] = {3 ,6, 9, 12, 15}; //C++ sub bands for sp1filter
#endif
  
  int sizeSubBand = sizeof(subbands )/sizeof(*subbands );
  int pYrSize = maxPyrHt(inp0->m_width[Y_COMP], inp0->m_height[Y_COMP],  sizeof(lo0filt) / sizeof(*lo0filt)   )-1;
  
  m_image0.resize(inp0->m_compSize[Y_COMP]); 
  m_image1.resize(inp1->m_compSize[Y_COMP]);
  
  //compute the non-constant luminace for each frames	
  for (int i = 0; i < inp0->m_compSize[Y_COMP]; i++) {
    m_image0[i]= rgbPQYuv ( (double)inp0->m_floatComp[0][i], (double)inp0->m_floatComp[1][i],(double)inp0->m_floatComp[2][i], m_vifYBitDepth, inp0->m_colorPrimaries);
    m_image1[i]= rgbPQYuv ( (double)inp1->m_floatComp[0][i], (double)inp1->m_floatComp[1][i],(double)inp1->m_floatComp[2][i], m_vifYBitDepth, inp1->m_colorPrimaries);
  }
  
  
  //variables names were chosen similar to the Matlab code
  m_hi0.resize    (inp0->m_compSize[Y_COMP]);
  m_lo0Img0.resize(inp0->m_compSize[Y_COMP]);
  m_lo0Img1.resize(inp1->m_compSize[Y_COMP]);
  m_temp.resize   (inp0->m_compSize[Y_COMP]);
  
  int xFDimHi0 = sizeof(hi0filt) / sizeof(*hi0filt);
  int yFDimHi0 = xFDimHi0;
  int xFDimLo0 = sizeof(lo0filt) / sizeof(*lo0filt);
  int y_fdim_lo0 = xFDimLo0;
  int xStart = 0;
  int yStart = 0;
  int xStep  = 1; 
  int yStep  = 1;
  int xStop  = inp0->m_width [Y_COMP];
  int yStop  = inp0->m_height[Y_COMP];
  int xDim   = inp0->m_width [Y_COMP];
  int yDim   = inp0->m_height[Y_COMP];
  int li0FiltSize = sizeof(lo0filt) / sizeof(*lo0filt);
  int bFiltsZ2    = sizeof(bfilts)  / sizeof(*bfilts);
  int subbandsNum = pYrSize * bFiltsZ2 + 2;
  vector<vector<double> > pyrImg0(pYrSize + 1);
  vector<vector<double> > pyrImg1(pYrSize + 1);
  vector<vector<double> > pyrImg0Arr (subbandsNum);
  vector<vector<double> > pyrImg1Arr (subbandsNum);  
  vector<double>          lo0FiltOneRow(li0FiltSize * li0FiltSize);
  vector<double>          hi0FiltOneRow(xFDimHi0 * xFDimHi0);
  vector<vector<int> >    lenWh(subbandsNum);
  vector<bool>            subbandUsed(subbandsNum);
  
  for (int i = 0;i < subbandsNum;i++)
    lenWh[i].resize(2);
  
  for (int i = 0; i < subbandsNum;i++)	{
    subbandUsed[i] = FALSE;
  }
  
  for (int i = 0; i < sizeSubBand;i++)	{
    subbandUsed[subbands[i]] = TRUE;
  }
  
  int elementPyrImg0  = subbandsNum - 1;
  int elementPyrImg1 = subbandsNum - 1;
  
  int k1 = elementPyrImg0;
  
  lenWh[k1  ][0] = xStop;  
  lenWh[k1--][1] = yStop;
  
  for (int i = 0; i < pYrSize; i++)	{
    int xLen = iRightShiftRound(xStop, i);
    int yLen = iRightShiftRound(yStop, i);
    for (int j = 0; j < bFiltsZ2; j++) {
      lenWh [k1][0] = xLen;
      lenWh [k1--][1] = yLen;
    }
  }
  lenWh [k1][0] = iRightShiftRound(xStop, pYrSize); 
  lenWh [k1][1] = iRightShiftRound(yStop, pYrSize);
  
  
  k1 = elementPyrImg0;
  for (int i = 0; i < subbandsNum;i++)  {
    if (subbandUsed[k1] == TRUE)   {
      pyrImg0Arr[k1].resize(lenWh[k1][0] * lenWh[k1][1]);
      pyrImg1Arr[k1].resize(lenWh[k1][0] * lenWh[k1][1]);
    }
    //else {
    //pyrImg0Arr[k1].resize (1);
    //pyrImg1Arr[k1].resize(1);
    //}
    k1--;
  }
  
  
  for (int i = 0; i < li0FiltSize ; i ++) {
    for (int j = 0; j < li0FiltSize ; j ++) {
      //reshape the filter into matrice
      lo0FiltOneRow[j + i * li0FiltSize]=	lo0filt[j][i] ;
    }
  }
  
  for (int i = 0; i < xFDimHi0 ; i ++)	{
    for (int j = 0; j < yFDimHi0 ; j ++) {
      hi0FiltOneRow[j + i * xFDimHi0]=	hi0filt[j][i] ;
    }
  }
  
  // Do wavelet decomposition. This requires the Steerable Pyramid. You can
  // use your own wavelet as long as the cell arrays org and dist contain
  // corresponding subbands from the reference and the distorted images respectively.
  // for the original !/ AMT: Note that there should be no notion of original vs distorted. 
  if (subbandUsed[elementPyrImg0]) {
    internalReduce(&m_image0[0], xDim, yDim, &hi0FiltOneRow[0], &m_temp[0],  xFDimHi0,  yFDimHi0,	xStart,  xStep,  xStop, yStart,  yStep,  yStop,	&pyrImg0Arr[elementPyrImg0][0]); //hio0
  }
  
  internalReduce(&m_image0[0], xDim,  yDim, &lo0FiltOneRow[0], &m_temp[0],  xFDimLo0,  y_fdim_lo0,xStart,  xStep,  xStop, yStart,  yStep,  yStop,	&m_lo0Img0[0]);
  
  elementPyrImg0--;
  buildSteerablePyramidLevels(&m_lo0Img0[0], pYrSize, inp0->m_width[Y_COMP],inp0->m_height[Y_COMP], pyrImg0, pYrSize,  elementPyrImg0, pyrImg0Arr, subbandUsed);
  
  // for the distorted
  if (subbandUsed[elementPyrImg1]) {
    internalReduce(&m_image1[0], xDim,  yDim, &hi0FiltOneRow[0], &m_temp[0], xFDimHi0, yFDimHi0, xStart, xStep, xStop, yStart, yStep, yStop, &pyrImg1Arr[elementPyrImg1][0]); //hio0
  }
  
  internalReduce(&m_image1[0], xDim, yDim, &lo0FiltOneRow[0], &m_temp[0], xFDimLo0, y_fdim_lo0, xStart, xStep, xStop, yStart, yStep, yStop, &m_lo0Img1[0]);
  
  elementPyrImg1--;
  buildSteerablePyramidLevels(&m_lo0Img1[0], pYrSize, inp1->m_width[Y_COMP],inp1->m_height[Y_COMP], pyrImg1,  pYrSize,  elementPyrImg1, pyrImg1Arr, subbandUsed);
  
  //================================
  // calculate the parameters of the distortion channel
  vector<vector<vector<double> > >   gAll     (subbandsNum);
  vector<vector<vector<double> > >   VvAll    (subbandsNum);
  vector<vector<int> >               lenWhGAll(subbandsNum);
  
  for (int i = 0; i < subbandsNum;i++)
    lenWhGAll[i].resize(2);
  
  for (int ii=0; ii<sizeSubBand;ii++)	{
    int sub = subbands [ii];
    int  newSizeYWidth = (lenWh[sub][0] / m_blockLength) * m_blockLength;
    int newSizeYHeight = (lenWh[sub][1] / m_blockLength) * m_blockLength;
    
    lenWhGAll [sub][0] = newSizeYWidth  / m_blockLength;  
    lenWhGAll [sub][1] = newSizeYHeight / m_blockLength; 
    gAll[sub].resize ( lenWhGAll [sub][0]);
    VvAll[sub].resize( lenWhGAll [sub][0]);
    for (int jj = 0;jj < lenWhGAll [sub][0]; jj++)		{
      gAll[sub][jj].resize( lenWhGAll [sub][1]);
      VvAll[sub][jj].resize( lenWhGAll [sub][1]);
    }
  }
  
  //calculate the parameters of the distortion channel
  vifSubEstM (pyrImg0Arr, pyrImg1Arr,  subbands,  lenWh, sizeSubBand, gAll,  VvAll, lenWhGAll);
  
  vector<vector<vector<double> > >  ssArr(subbandsNum);
  vector<vector<vector<double> > >  cuArr(subbandsNum);
  vector<vector<double> >           lArr (subbandsNum);
  
  for (int ii = 0;ii < sizeSubBand; ii++)  {
    int i = subbands[ii];
    int  newSizeYWidth = (lenWh[i][0] / m_blockLength) * m_blockLength;
    int newSizeYHeight = (lenWh[i][1] / m_blockLength) * m_blockLength;
    
    cuArr[i].resize(m_blockSize);
    ssArr[i].resize(newSizeYWidth / m_blockLength);
    lArr[i].resize (m_blockSize);
    
    for (int j = 0; j < m_blockSize; j++) {
      cuArr[i][j].resize( m_blockSize );
    }
    
    for (int j = 0;j < newSizeYWidth / m_blockLength; j++)  {
      ssArr[i][j].resize(newSizeYHeight / m_blockLength);
    }
  }
  
  //calculate the parameters of the reference image
  refParamsVecGSM (pyrImg0Arr, subbands, lenWh,  sizeSubBand,  ssArr,  lArr,  cuArr);
  
  // compute reference and distorted image information from each subband
  vector<double> num(sizeSubBand);
  vector<double> den(sizeSubBand);
  
  for (int i=0;i<sizeSubBand;i++)  {
    int sub=subbands[i];
    //how many eigenvalues to sum over. default is 1.
    //compute the size of the window used in the distortion channel estimation, and use it to calculate the offset from subband borders
    //we do this to avoid all coefficients that may suffer from boundary
    //effects
    int lev     = (int) ceil((sub)/6.0);
    int winSize = (1 << lev) + 1; 
    int offset  = (winSize - 1) >> 1;
    offset = (int) (ceil(offset / (double) m_blockLength));
    int sizeGWidth  = lenWhGAll[sub][0] - 2 * offset;
    int sizeGHeight = lenWhGAll[sub][1] - 2 * offset;
    
    vector<vector<double> >  g (sizeGWidth);
    vector<vector<double> >  vv(sizeGWidth);
    vector<vector<double> >  ss(sizeGWidth);
    
    for (int jj = 0; jj < sizeGWidth; jj++)    {
      g [jj].resize(sizeGHeight);
      vv[jj].resize(sizeGHeight);
      ss[jj].resize(sizeGHeight);
    }
    
    int w1 = 0;
    for (int w = offset; w < lenWhGAll[sub][0] - offset; w++)  {
      int h1=0;	
      for (int h = offset; h < lenWhGAll[sub][1] - offset; h++)  {
        // select only valid portion of the output.
        g [w1][h1] = gAll [sub][w][h];
        vv[w1][h1] = VvAll[sub][w][h];
        ss[w1][h1] = ssArr[sub][w][h];
        h1++;
      }
      w1++;
    }
    
    // VIF
    double temp1 = 0.0; 
    double temp2 = 0.0;
    for (int j = 0; j < m_blockSize; j++)  {
      temp1 += sumSumLog2GGSSLambdaDivVV  ( g, sizeGWidth, sizeGHeight,  ss, lArr[sub][j],  sigma_nsq,  vv);
      temp2 += sumSumLog2SSLambdaDivSigma (ss, sizeGWidth, sizeGHeight, lArr[sub][j], sigma_nsq); 
    }
    num[i] = temp1;
    den[i] = temp2;
  }
  //compute IFC and normalize to size of the image
  double ifc1 = 0;
  double sumNum = 0;
  double sumDen = 0;
  for (int j = 0;j < sizeSubBand; j++) {
    sumNum += num[j];
    sumDen += den[j];
    ifc1   += num[j] / (inp0->m_width[Y_COMP] * inp0->m_height[Y_COMP]);
  }
  
  m_vifFrame = sumNum / sumDen;
  m_vifFrameStats.updateStats(m_vifFrame);
}

void DistortionMetricVIF::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  printf("computeMetric DeltaE in One component is not possible\n");
}
//needs to be updated
void DistortionMetricVIF::reportMetric ()
{
  printf(" %0.6f  ",m_vifFrame);
}

void DistortionMetricVIF::reportSummary  ()
{
  printf(" %0.6f  ", m_vifFrameStats.getAverage());
}

void DistortionMetricVIF::reportMinimum  ()
{
  printf(" %0.6f  ", m_vifFrameStats.minimum);
}

void DistortionMetricVIF::reportMaximum  ()
{
  printf(" %0.6f  ", m_vifFrameStats.maximum);
}

void DistortionMetricVIF::printHeader()
{
  printf ("Frame_VIF  ");
}

void DistortionMetricVIF::printSeparator(){
  printf("-------------");
}
