/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Technicolor
 * <ORGANIZATION> = Technicolor
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Technicolor
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
 * \file DistortionMetricDeltaE.cpp
 *
 * \brief
 *    DeltaE distortion computation Class
 *
 * \author
 *     - Christophe Chevance         <christophe.chevance@technicolor.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "DistortionMetricDeltaE.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

DistortionMetricDeltaE::DistortionMetricDeltaE(const FrameFormat *format, bool enableShowMSE, double maxSampleValue, double *whitePointDeltaE, int deltaEPointsEnable)
 : DistortionMetric()
{
  m_enableShowMSE = enableShowMSE;
  m_deltaEPointsEnable = deltaEPointsEnable;
  
  for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++) {
    if ((deltaEPointsEnable & ( 1 << wRef)) == 0)
      continue;
    m_deltaE[wRef] = 0.0;
    m_deltaEStats[wRef].reset();
    m_PsnrDE[wRef] = 0.0;
    m_PsnrDEStats[wRef].reset();
    
    m_maxDeltaE[wRef] = 0.0;
    m_maxDeltaEStats[wRef].reset();
    m_PsnrMaxDE[wRef] = 0.0;
    m_PsnrMaxDEStats[wRef].reset();
    
    m_whitePointDeltaE[wRef] =  whitePointDeltaE[wRef];
    
    m_PsnrL[wRef] = 0.0;
    m_PsnrLStats[wRef].reset();
  }
  m_maxValue = (double) maxSampleValue;
}

DistortionMetricDeltaE::~DistortionMetricDeltaE()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
double DistortionMetricDeltaE::trueLab(double r)
{
  // r should be smaller than 0.008856 after the first conditional. So unclear why the (r < 0.008856) was needed.
  // Equation agrees also with forward transformation to LaB
  //double result = (r >= 0.008856) ? pow(r, 1 / 3.0) : (r < 0.008856) * (7.78704 * r + 0.137931);
	//result = result + (r<0.008856)*(7.78704*r+0.137931);
  return (r >= 0.008856) ? pow(r, 1.0 / 3.0) : (7.78704 * r + 0.137931);
}

void DistortionMetricDeltaE::xyz2TrueLab(double srcX, double srcY, double srcZ, double *dstL, double *dstA, double *dstB, double whitePointDeltaE)
{
  // These quantities are constant per whitePointDeltaE and thus we could maybe save on computations
  // also, currently these use too many divides. We could change these into multiplications to again make things faster.
  double invYn = FILE_REF_IN_NITS_DOUBLE / whitePointDeltaE;
  double invXn = invYn / 0.95047;
  double invZn = invYn / 1.08883;

  //double xn = whitePointDeltaE * 0.95047 / FILE_REF_IN_NITS_DOUBLE;
  //double yn = whitePointDeltaE           / FILE_REF_IN_NITS_DOUBLE;
  //double zn = whitePointDeltaE * 1.08883 / FILE_REF_IN_NITS_DOUBLE;
  double yLab = trueLab(srcY * invYn);

	*dstL = 116.0 *  yLab - 16.0;
	*dstA = 500.0 * (trueLab(srcX * invXn) - yLab);
	*dstB = 200.0 * (yLab - trueLab(srcZ * invZn));
}

void DistortionMetricDeltaE::xyz2TrueLab(double srcX, double srcY, double srcZ, double *dstL, double *dstA, double *dstB, double invYn, double invXn, double invZn)
{
  double yLab = trueLab(srcY * invYn);
  
  *dstL = 116.0 *  yLab - 16.0;
  *dstA = 500.0 * (trueLab(srcX * invXn) - yLab);
  *dstB = 200.0 * (yLab - trueLab(srcZ * invZn));
}

double DistortionMetricDeltaE::deltaE2000(double lRef, double aStarRef, double bStarRef, double lIn, double aStarIn, double bStarIn)
{
  // Compute C
  const double cRef = sqrt(aStarRef * aStarRef + bStarRef * bStarRef);
  const double cIn  = sqrt(aStarIn  * aStarIn  + bStarIn  * bStarIn );

  // these variables are not used but left here as reference
  const double lPRef = lRef;
  const double lPIn  = lIn;

  // Calculate G
  const double cm = (cRef + cIn) / 2.0;
  const double g   = 0.5 * ( 1.0 - sqrt( pow(cm, 7.0) / (pow(cm, 7.0) + pow(25.0, 7.0)) ) );
	
  const double aPRef = (1.0 + g) * aStarRef;
  const double aPIn  = (1.0 + g) * aStarIn;

  // these variables are not used but left here as reference
  const double bPRef = bStarRef;
  const double bPIn  = bStarIn;

  const double cPRef = sqrt(aPRef * aPRef + bPRef * bPRef);
  const double cPIn  = sqrt(aPIn  * aPIn  + bPIn  * bPIn );

  double hPRef = atan2(bPRef, aPRef);
  double hPIn  = atan2(bPIn , aPIn);

  // Calculate deltaL_p , deltaC_p , deltaH_p;
  double deltaLp = lPRef - lPIn;
  double deltaCp = cPRef - cPIn;
  double deltaHp = 2.0 * sqrt(cPRef * cPIn) * sin( (hPRef - hPIn) / 2.0 );


  //Calculate deltaE2000
  double lpm = (lPRef + lPIn) / 2.0;
  double cpm = (cPRef + cPIn) / 2.0;
  double hpm = (hPRef + hPIn) / 2.0;

  double rC = 2.0 * sqrt( pow(cpm, 7.0) / (pow(cpm, 7.0) + pow(25.0, 7.0)) );
  double deltaTheta = DEG30 * exp(-((hpm - DEG275) / DEG25) * ((hpm - DEG275) / DEG25));
  double rT = -sin(2.0 * deltaTheta) * rC;
  double t = 1.0 - 0.17 * cos(hpm - DEG30) + 0.24 * cos(2.0 * hpm) + 0.32 * cos(3.0 * hpm + DEG6) - 0.20 * cos(4.0 * hpm - DEG63);
  double sH = 1.0 + (0.015 * cpm * t);
  double sC = 1.0 + (0.045 * cpm);
  double sL = 1.0 + (0.015 * (lpm - 50.0) * (lpm - 50.0) / sqrt(20.0 + (lpm - 50.0) * (lpm - 50.0))  );

  // These quantities are all 1.0. So this multiplication seems not necessary and could be skipped
  // to speed up computations
  //double kL = 1.0;
  //double kC = 1.0;
  //double kH = 1.0;

  //deltaE2000 = sqrt(   (deltaLp / (kL * sL)) * (deltaLp / (kL * sL)) + (deltaCp / (kC * sC))*(deltaCp / (kC * sC)) + (deltaHp / (kH * sH)) * (deltaHp / (kH * sH)) + rT * (deltaCp / (kC * sC)) * (deltaHp / (kH * sH))  );
  double deltaLpSL = deltaLp / sL;
  double deltaCpSC = deltaCp / sC;
  double deltaHpSH = deltaHp / sH;
  
  return sqrt(   deltaLpSL * deltaLpSL + deltaCpSC * deltaCpSC + deltaHpSH * deltaHpSH + rT * deltaCpSC * deltaHpSH  );
}

double DistortionMetricDeltaE::getDeltaE2000(double x, double y, double z, double xRec, double yRec, double zRec, double whitePointDeltaE)
{
	double l, a, b, lRec, aRec, bRec;
	double d2000;

	xyz2TrueLab(x, y, z, &l, &a, &b, whitePointDeltaE);
	xyz2TrueLab(xRec, yRec, zRec, &lRec, &aRec, &bRec, whitePointDeltaE);
  
	d2000 = deltaE2000(l, a, b, lRec, aRec, bRec);
	return d2000;
}

double DistortionMetricDeltaE::getDeltaE2000(double x, double y, double z, double xRec, double yRec, double zRec, double invYn, double invXn, double invZn)
{
  double l, a, b, lRec, aRec, bRec;
  
  xyz2TrueLab(x, y, z, &l, &a, &b, invYn, invXn, invZn);
  xyz2TrueLab(xRec, yRec, zRec, &lRec, &aRec, &bRec, invYn, invXn, invZn);
  
  return deltaE2000(l, a, b, lRec, aRec, bRec);
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
//***********
//RGB --> XYZ
//***********

static const float RGB2XYZ_REC[5][9] =
{
  {  // RBG BT.709
    0.4124f, 0.3576f, 0.1805f,
    0.2126f, 0.7152f, 0.0722f,
    0.0193f, 0.1192f, 0.9504f,
  },
  {  // RBG BT.2020
    0.6370f, 0.1446f, 0.1689f,
    0.2627f, 0.6780f, 0.0593f,
    0.0000f, 0.0281f, 1.0610f,
  },
  { // P3 D60
    0.5049495342f, 0.2646814889f, 0.1830150515f,
    0.2376233102f, 0.6891706692f, 0.0732060206f,
    0.0000000000f, 0.0449459132f, 0.9638792711f
  },
  { // P3 D65
    0.486571f, 0.265668f, 0.198217f,
    0.228975f, 0.691739f, 0.079287f,
    0.000000f, 0.045113f, 1.043944f
  },
  { // XYZ
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f
  }
};

void DistortionMetricDeltaE::computeMetric (Frame* inp0, Frame* inp1)
{
  double x0, y0, z0, x1, y1, z1;
  //double deltaE2000;
  //double l0, a0, b0, l1, a1, b1;
  double meanDeltaL = 0.0;
  double deltaE = 0.0;
  double currentDeltaE = 0.0, maxDeltaE = 0.0;

  int cPrim0 = inp0->m_colorPrimaries;
  int cPrim1 = inp1->m_colorPrimaries;

  if (cPrim0 == CP_UNKNOWN || cPrim1 == CP_UNKNOWN) {
    printf("DeltaE not supported for unknown color primaries/spaces.\n");
  }
  else if (inp0->m_colorSpace == CM_YCbCr ||  inp1->m_colorSpace == CM_YCbCr) {
    printf("DeltaE not supported for YUV data.\n");
  }
  else if (inp0->m_colorSpace == CM_ICtCp ||  inp1->m_colorSpace == CM_ICtCp) {
    printf("DeltaE not supported for ICtCp data.\n");
  }
  else if (inp0->m_colorSpace >= CM_YFBFRV1 ||  inp1->m_colorSpace >= CM_YFBFRV1) {
    printf("DeltaE not supported for YFBFR data.\n");
  }
  else {
    // it is assumed here that the frames are of the same type
    if (inp0->equalType(inp1)) {
      if (inp0->m_isFloat == TRUE) {
        const float *rec0RGB2XYZ = &RGB2XYZ_REC[cPrim0][0];
        const float *rec1RGB2XYZ = &RGB2XYZ_REC[cPrim1][0];

        // we could reorder the for loops to do image pass first, followed by white point
        // unclear how much that could gain since we now can compute the scaling parameters
        // once at the frame level. Could be investigated.
        for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++) {
          if ((m_deltaEPointsEnable & ( 1 << wRef)) == 0)
            continue;

          deltaE     = 0.0;
          meanDeltaL = 0.0;

          double invYn = FILE_REF_IN_NITS_DOUBLE / m_whitePointDeltaE[wRef];
          double invXn = invYn / 0.95047;
          double invZn = invYn / 1.08883;
  
          float *floatImg0Comp0 = &inp0->m_floatComp[0][0];
          float *floatImg0Comp1 = &inp0->m_floatComp[1][0];
          float *floatImg0Comp2 = &inp0->m_floatComp[2][0];
          float *floatImg1Comp0 = &inp1->m_floatComp[0][0];
          float *floatImg1Comp1 = &inp1->m_floatComp[1][0];
          float *floatImg1Comp2 = &inp1->m_floatComp[2][0];
          // floating point data
          for (int i = 0; i < inp0->m_compSize[0]; i++) {
            // =================  Method 2 : RGB to XYZ followed by PQ curve on XYZ, followed by X'Y'Z' to Y'DzDx ================
            // RGB to XYZ conversion
            x0 = ( rec0RGB2XYZ[0] * (double)(*floatImg0Comp0  ) + rec0RGB2XYZ[1] * (double)(*floatImg0Comp1  ) + rec0RGB2XYZ[2] * (double)(*floatImg0Comp2  ) );
            y0 = ( rec0RGB2XYZ[3] * (double)(*floatImg0Comp0  ) + rec0RGB2XYZ[4] * (double)(*floatImg0Comp1  ) + rec0RGB2XYZ[5] * (double)(*floatImg0Comp2  ) );
            z0 = ( rec0RGB2XYZ[6] * (double)(*floatImg0Comp0++) + rec0RGB2XYZ[7] * (double)(*floatImg0Comp1++) + rec0RGB2XYZ[8] * (double)(*floatImg0Comp2++) );

            x1 = ( rec1RGB2XYZ[0] * (double)(*floatImg1Comp0  ) + rec1RGB2XYZ[1] * (double)(*floatImg1Comp1  ) + rec1RGB2XYZ[2] * (double)(*floatImg1Comp2  ) );
            y1 = ( rec1RGB2XYZ[3] * (double)(*floatImg1Comp0  ) + rec1RGB2XYZ[4] * (double)(*floatImg1Comp1  ) + rec1RGB2XYZ[5] * (double)(*floatImg1Comp2  ) );
            z1 = ( rec1RGB2XYZ[6] * (double)(*floatImg1Comp0++) + rec1RGB2XYZ[7] * (double)(*floatImg1Comp1++) + rec1RGB2XYZ[8] * (double)(*floatImg1Comp2++) );
            
            //deltaE2000 = getDeltaE2000(x0, y0, z0, x1, y1, z1, m_whitePointDeltaE[wRef]);
            //deltaE += deltaE2000;
            currentDeltaE = getDeltaE2000(x0, y0, z0, x1, y1, z1, invYn, invXn, invZn);
            maxDeltaE = dMax(maxDeltaE, currentDeltaE);
            deltaE += currentDeltaE;
            
            // L, a, b
            //xyz2TrueLab(x0, y0, z0, &l0, &a0, &b0, m_whitePointDeltaE[wRef]);
            //xyz2TrueLab(x1, y1, z1, &l1, &a1, &b1, m_whitePointDeltaE[wRef]);
            // new version
            //xyz2TrueLab(x0, y0, z0, &l0, &a0, &b0, invYn, invXn, invZn);
            //xyz2TrueLab(x1, y1, z1, &l1, &a1, &b1, invYn, invXn, invZn);
            //l0 = 116.0 *  trueLab(y0 * invYn);
            //l1 = 116.0 *  trueLab(y1 * invYn);
            //meanDeltaL += dAbs(l0-l1);
            
            // fast version
            meanDeltaL += dAbs(116.0 *  (trueLab(y0 * invYn) - trueLab(y1 * invYn)));

          }
          
          m_deltaE[wRef] = deltaE / (double) inp0->m_compSize[0];
          m_deltaEStats[wRef].updateStats(m_deltaE[wRef]);
          m_PsnrDE[wRef] = 10.0 * log10( m_maxValue / m_deltaE[wRef] );
          m_PsnrDEStats[wRef].updateStats(m_PsnrDE[wRef]);

          m_maxDeltaE[wRef] = maxDeltaE;
          m_maxDeltaEStats[wRef].updateStats(m_maxDeltaE[wRef]);
          m_PsnrMaxDE[wRef] = 10.0 * log10( m_maxValue / m_maxDeltaE[wRef] );
          m_PsnrMaxDEStats[wRef].updateStats(m_PsnrMaxDE[wRef]);

          meanDeltaL /= (double) inp0->m_compSize[0];
          m_PsnrL[wRef] = 10.0 * log10( m_maxValue / meanDeltaL );
          m_PsnrLStats[wRef].updateStats(m_PsnrL[wRef]);
        }
      }
      else if (inp0->m_bitDepth == 8) {   // 8 bit data
        printf("DeltaE not implemented for 8 bits input.\n");
      }
      else { // 16 bit data
        printf("DeltaE not implemented for 16 bits input.\n");
      }
    }
    else {
      printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
    }
  }
}

void DistortionMetricDeltaE::computeMetric (Frame* inp0, Frame* inp1, int component)
{
   printf("computeMetric DeltaE in One component is not possible\n");
}

void DistortionMetricDeltaE::reportMetric ()
{
  for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++) {
    if ((m_deltaEPointsEnable & ( 1 << wRef)) == 0)
      continue;
    printf("%10.3f   ", m_PsnrDE[wRef]);
    printf("%10.3f   ", m_PsnrMaxDE[wRef]);
    printf("%9.3f   ", m_PsnrL[wRef]);
    if (m_enableShowMSE == TRUE) {
      printf("%11.4f   ", m_deltaE[wRef]);
      printf("%11.4f   ", m_maxDeltaE[wRef]);
    }
  }
}

void DistortionMetricDeltaE::reportSummary  ()
{
  for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++)  {
    if ((m_deltaEPointsEnable & ( 1 << wRef)) == 0)
      continue;
    printf("%10.3f   ", m_PsnrDEStats[wRef].getAverage());
    printf("%10.3f   ", m_PsnrMaxDEStats[wRef].getAverage());
    printf("%9.3f   ", m_PsnrLStats[wRef].getAverage());
    if (m_enableShowMSE == TRUE) {
      printf("%11.4f   ", m_deltaEStats[wRef].getAverage());
      printf("%11.4f   ", m_maxDeltaEStats[wRef].getAverage());
    }
  }
}

void DistortionMetricDeltaE::reportMinimum  ()
{
  for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++)  {
    if ((m_deltaEPointsEnable & ( 1 << wRef)) == 0)
      continue;
    printf("%10.3f   ", m_PsnrDEStats[wRef].minimum );
    printf("%10.3f   ", m_PsnrMaxDEStats[wRef].minimum );
    printf("%9.3f   ", m_PsnrLStats[wRef].minimum);
    if (m_enableShowMSE == TRUE) {
      printf("%11.4f   ", m_deltaEStats[wRef].minimum );
      printf("%11.4f   ", m_maxDeltaEStats[wRef].minimum );
    }
  }
}

void DistortionMetricDeltaE::reportMaximum  ()
{
  for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++)  {
    if ((m_deltaEPointsEnable & ( 1 << wRef)) == 0)
      continue;
    printf("%10.3f   ", m_PsnrDEStats[wRef].maximum);
    printf("%10.3f   ", m_PsnrMaxDEStats[wRef].maximum);
    printf("%9.3f   ", m_PsnrLStats[wRef].maximum);
    if (m_enableShowMSE == TRUE) {
      printf("%11.4f   ", m_deltaEStats[wRef].maximum);
      printf("%11.4f   ", m_maxDeltaEStats[wRef].maximum);
    }
  }
}

void DistortionMetricDeltaE::printHeader()
{
  for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++) {
    if ((m_deltaEPointsEnable & ( 1 << wRef)) == 0)
      continue;
    
    if (m_isWindow == FALSE ) {
      printf("PSNR_DE%04d  ",(int)m_whitePointDeltaE[wRef]); // 13
      printf("PSNR_MD%04d  ",(int)m_whitePointDeltaE[wRef]); // 13
      printf("PSNR_L%04d  ",(int)m_whitePointDeltaE[wRef]); // 12
      if (m_enableShowMSE == TRUE) {
        printf(" A_DeltaE%04d ",(int)m_whitePointDeltaE[wRef]); // 14
        printf(" M_DeltaE%04d ",(int)m_whitePointDeltaE[wRef]); // 14
      }
    }
    else {
      printf("wPSNR_DE%04d ",(int)m_whitePointDeltaE[wRef]); // 13
      printf("wPSNR_MD%04d ",(int)m_whitePointDeltaE[wRef]); // 13
      printf("wPSNR_L%04d ",(int)m_whitePointDeltaE[wRef]); // 12
      if (m_enableShowMSE == TRUE) {
        printf("wA_DeltaE%04d ",(int)m_whitePointDeltaE[wRef]); // 14
        printf("wM_DeltaE%04d ",(int)m_whitePointDeltaE[wRef]); // 14
      }
    }
  }
}

void DistortionMetricDeltaE::printSeparator(){
  for(int wRef = 0 ; wRef < NB_REF_WHITE ; wRef++)  {
    if ((m_deltaEPointsEnable & ( 1 << wRef)) == 0)
      continue;

    printf("-------------");
    printf("-------------");
    printf("------------");
    if (m_enableShowMSE == TRUE) {
      printf("--------------");
      printf("--------------");
    }
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
