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
 * \file AnalyzeGamut.cpp
 *
 * \brief
 *    Analyze Gamut
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "AnalyzeGamut.H"
#include "ColorTransformGeneric.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
static char histFile[]  = "histFile.txt";

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

AnalyzeGamut::AnalyzeGamut()
{
  m_histBinsDiv3       = 2000;             // histogram
  m_totalComponents = TOTAL_COMPONENTS; // 3 for YCbCr, 3 for RGB, 3 for XYZ and three aggregators = 12
  
  for (int c = 0; c < m_totalComponents; c++) {
    m_cntMinRGB [c] = 0.0;
    m_cntMaxRGB [c] = 0.0;
    m_cntMinYUV [c] = 0.0;
    m_cntMaxYUV [c] = 0.0;
    m_minRGBStats[c].reset();
    m_maxRGBStats[c].reset();
    m_minYUVStats[c].reset();
    m_maxYUVStats[c].reset();
    m_minData [c] = 65535;
    m_maxData [c] = 0;
    m_minStats[c].reset();
    m_maxStats[c].reset();
    m_histRGB[c].resize(m_histBinsDiv3 * 3 + 1);
    m_pHistogram[c] = &m_histRGB[c][m_histBinsDiv3];
  }
  m_cntAllRGB = 0.0;
  m_allRGBStats.reset();
  m_counter = 0;
  
}

AnalyzeGamut::~AnalyzeGamut()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
float AnalyzeGamut::convertValue (const imgpel iComp, double weight, double offset) {
    return (float) ((weight * (double) (iComp)) - offset);
}

float AnalyzeGamut::convertValue (const uint16 iComp, double weight, double offset) {
//printf("value %d %10.7f\n", iComp, ((weight * (double) (iComp)) - offset));

    return (float) ((weight * (double) (iComp)) - offset);
}

float AnalyzeGamut::convertValue (const uint16 iComp,  int bitScale, double weight, double offset) {
    return (float) (weight * (((double) (iComp >> bitScale)) - offset));
}

float AnalyzeGamut::convertValue (const imgpel iComp, double weight, const uint16 offset) {
    return (float) ((weight * (double) (iComp - offset)));
}

float AnalyzeGamut::convertValue (const uint16 iComp, double weight, const uint16 offset) {
//printf("value %d %d %10.7f\n", iComp, iComp - offset,  ((weight * (double) (iComp - offset))));

    return (float) ((weight * (double) (iComp - offset)));
}


float AnalyzeGamut::convertCompValue(uint16 inpValue, SampleRange sampleRange, ColorSpace colorSpace, int bitDepth, int component) {

  if (sampleRange == SR_FULL) {
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default:
          return convertValue (inpValue, 1.0 / ((1 << bitDepth) - 1.0), (const uint16) 0);
        case U_COMP:
        case V_COMP:
          return convertValue (inpValue, 1.0 / ((1 << bitDepth) - 1.0), (const uint16) ((1 << (bitDepth - 1))));
      }
    }
    else {
      convertValue (inpValue, 1.0 / ((1 << bitDepth) - 1.0), 0.0);
    }
  }
  else if (sampleRange == SR_STANDARD) {
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default:
          return convertValue (inpValue, 1.0 / ((1 << (bitDepth - 8)) * 219.0), 16.0 / 219.0);
        case U_COMP:
        case V_COMP:
          return convertValue (inpValue, 1.0 / ((1 << (bitDepth - 8)) * 224.0), 128.0 / 224.0);
      }
    }
    else {
      return convertValue (inpValue, 1.0 / ((1 << (bitDepth - 8)) * 219.0), 16.0 / 219.0);
    }
  }
  else if (sampleRange == SR_RESTRICTED) {
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default: {
          double weight = 1.0 / (double) ((1 << bitDepth) - (1 << (bitDepth - 7)));
          double offset = (double) (1 << (bitDepth - 8)) * weight;
          return convertValue (inpValue, weight, offset);
        }
        case U_COMP:
        case V_COMP: {
          double weight = 1.0 / (double) ((1 << bitDepth) - (1 << (bitDepth-7)));
          double offset = (double) (1 << (bitDepth-1)) * weight;
          return convertValue (inpValue, weight, offset);
        }
      }
    }
    else {
      double weight = 1.0 / (double) ((1 << bitDepth) - (1 << (bitDepth - 7)));
      double offset = (double) (1 << (bitDepth - 8)) * weight;
      return convertValue (inpValue, weight, offset);
    }
  }
  else if (sampleRange == SR_SDI_SCALED) {
    // SDI fixed mode for PQ encoded data, scaled by 16. Data were originally graded in "12" bits given the display, but stored
    // in the end in 16 bits. 
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default: {
          double weight = 1.0 / 4060.0;
          double offset = 16.0;
          return convertValue (inpValue, 4, weight, offset);
        }
        case U_COMP:
        case V_COMP: {
          double weight = 1.0 / 4048.0;
          double offset = 2048.0;
          return convertValue (inpValue, 4, weight, offset);
        }
      }
    }
    else {
      double weight = 1.0 / 4060.0;
      double offset = 16.0;
      return convertValue (inpValue, 4, weight, offset);
    }
  }
  else if (sampleRange == SR_SDI) {
    // SDI specifies values in the range of Floor(1015 * D * N + 4 * D + 0.5), where N is the nonlinear color value from zero to unity, and D = 2^(B-10).
    // Note that we are cheating a bit here since SDI only allows values from 10-16 but we modified the function a bit to also handle bitdepths of 8 and 9 (although likely we will not use those).
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default: {
          double weight = 1.0 / (253.75 * (double) (1 << (bitDepth - 8)));
          double offset = (double) (1 << (bitDepth-8)) * weight;            
          return convertValue (inpValue, weight, offset);
        }
        case U_COMP:
        case V_COMP: {
          double weight = 1.0 / (253.00 * (double) (1 << (bitDepth - 8)));
          double offset = (double) (1 << (bitDepth-1)) * weight;
          return convertValue (inpValue, weight, offset);
        }
      }
    }
    else {
      double weight = 1.0 / (253.75 * (double) (1 << (bitDepth - 8)));
      double offset = (double) (1 << (bitDepth-8)) * weight;            
      return convertValue (inpValue, weight, offset);
    }
  }
  return 0.0;
}

float AnalyzeGamut::convertCompValue(imgpel inpValue, SampleRange sampleRange, ColorSpace colorSpace, int bitDepth, int component) {
  if (sampleRange == SR_FULL) {
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default:
          return convertValue (inpValue, 1.0 / 255.0, (const uint16) 0);
        case U_COMP:
        case V_COMP:
          return convertValue (inpValue, 1.0 / 255.0, (const uint16) 128);
      }
    }
    else {
      convertValue (inpValue, 1.0 / 255.0, 0.0);
    }
  }
  else { // lump all other ranges here since commonly we only have FULL and Video range for 8 bit data
    if (colorSpace == CM_YCbCr || colorSpace == CM_ICtCp) {
      switch (component) {
        case Y_COMP:
        default:
          return convertValue (inpValue, 1.0 / 219.0, 16.0 / 219.0);
        case U_COMP:
        case V_COMP:
          return convertValue (inpValue, 1.0 / 224.0, 128.0 / 224.0);
      }
    }
    else {
      return convertValue (inpValue, 1.0 / 219.0, 16.0 / 219.0);
    }
  }

  return 0.0;
}


void AnalyzeGamut::convertToRGB(double *yuv, double *rgb, const double *transform0, const double *transform1, const double *transform2) {

  rgb[0] =  transform0[0] * yuv[Y_COMP] + transform0[1] * yuv[U_COMP] + transform0[2] * yuv[V_COMP];
  rgb[1] =  transform1[0] * yuv[Y_COMP] + transform1[1] * yuv[U_COMP] + transform1[2] * yuv[V_COMP];
  rgb[2] =  transform2[0] * yuv[Y_COMP] + transform2[1] * yuv[U_COMP] + transform2[2] * yuv[V_COMP];
  
}

void AnalyzeGamut::setColorConversion(int colorPrimaries, const double **transform0, const double **transform1, const double **transform2) {
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
  
  *transform0 = INV_TRANSFORM[mode][Y_COMP];
  *transform1 = INV_TRANSFORM[mode][U_COMP];
  *transform2 = INV_TRANSFORM[mode][V_COMP];
}

void AnalyzeGamut::compute(Frame* inp)
{
  const double compLimitsMin[3] = { 0.0, -0.5, -0.5 };
  const double compLimitsMax[3] = { 1.0, +0.5, +0.5 };
  
  double rgbDouble[3], yCbCrDouble[3];
  
  for (int c = 0; c < m_totalComponents; c++) {
    m_cntMinRGB [c] = 0.0;
    m_cntMaxRGB [c] = 0.0;
    m_cntMinYUV [c] = 0.0;
    m_cntMaxYUV [c] = 0.0;
    m_minData   [c] = 65535;
    m_maxData   [c] = 0;
  }
  m_cntAllRGB = 0.0;
  m_counter++;
  
  if (inp->m_colorSpace == CM_RGB) { // In this case, we only check for negative and positive values
    for (int i = 0; i < inp->m_compSize[Y_COMP]; i++) {
      for (int comp = R_COMP; comp <= B_COMP; comp++) {
        if (inp->m_bitDepth == 8) {
          if (m_minData[comp] > inp->m_comp[comp][i])
            m_minData[comp] = inp->m_comp[comp][i];
          if (m_maxData[comp] < inp->m_comp[comp][i])
            m_maxData[comp] = inp->m_comp[comp][i];
          rgbDouble[comp] = convertCompValue(inp->m_comp[comp][i], inp->m_sampleRange, CM_RGB, 8, comp);
          }
        else {
          if (m_minData[comp] > inp->m_ui16Comp[comp][i])
            m_minData[comp] = inp->m_ui16Comp[comp][i];
          if (m_maxData[comp] < inp->m_ui16Comp[comp][i])
            m_maxData[comp] = inp->m_ui16Comp[comp][i];

          rgbDouble[comp] = convertCompValue(inp->m_ui16Comp[comp][i], inp->m_sampleRange, CM_RGB, 8, comp);
          }
        
        m_cntMinRGB[comp] += (rgbDouble[comp] < compLimitsMin[0]);
        m_cntMaxRGB[comp] += (rgbDouble[comp] > compLimitsMax[0]);
      }
    }
  }
  else {
    
    const double *transform0 = NULL;
    const double *transform1 = NULL;
    const double *transform2 = NULL;
    
    setColorConversion(inp->m_colorPrimaries, &transform0, &transform1, &transform2);
    
    int heightScale = inp->m_height[Y_COMP] / inp->m_height[U_COMP];
    int widthScale  = inp->m_width[Y_COMP] / inp->m_width[U_COMP];
    for (int j = 0; j < inp->m_height[Y_COMP]; j++) {
      for (int i = 0; i < inp->m_width[Y_COMP]; i++) {
        if (inp->m_bitDepth == 8) {
          imgpel sampleValue = inp->m_comp[Y_COMP][j * inp->m_width[Y_COMP] + i];
          if (m_minData[Y_COMP] > sampleValue)
            m_minData[Y_COMP] = sampleValue;
          if (m_maxData[Y_COMP] < sampleValue)
            m_maxData[Y_COMP] = sampleValue;
          
          yCbCrDouble[Y_COMP] = convertCompValue(sampleValue, inp->m_sampleRange, inp->m_colorSpace, 8, Y_COMP);
        }
        else {
          uint16 sampleValue = inp->m_ui16Comp[Y_COMP][j * inp->m_width[Y_COMP] + i];
          if (m_minData[Y_COMP] > sampleValue)
            m_minData[Y_COMP] = sampleValue;
          if (m_maxData[Y_COMP] < sampleValue)
            m_maxData[Y_COMP] = sampleValue;
          
          yCbCrDouble[Y_COMP] = convertCompValue(sampleValue, inp->m_sampleRange, inp->m_colorSpace, inp->m_bitDepth, Y_COMP);
        }
        m_cntMinYUV[Y_COMP] += (yCbCrDouble[Y_COMP] < compLimitsMin[Y_COMP]) ? 1 : 0;
        m_cntMaxYUV[Y_COMP] += (yCbCrDouble[Y_COMP] > compLimitsMax[Y_COMP]) ? 1 : 0;
        
        for (int comp = U_COMP; comp <= V_COMP; comp++) {
          if (inp->m_bitDepth == 8) {
            imgpel sampleValue = inp->m_comp[comp][(j / heightScale) * inp->m_width[comp] + (i / widthScale)];
            if (m_minData[comp] > sampleValue)
              m_minData[comp] = sampleValue;
            if (m_maxData[comp] < sampleValue)
              m_maxData[comp] = sampleValue;
            
            yCbCrDouble[comp] = convertCompValue(sampleValue, inp->m_sampleRange, inp->m_colorSpace, 8, comp);
          }
          else {
            uint16 sampleValue = inp->m_ui16Comp[comp][(j / heightScale) * inp->m_width[comp] + (i / widthScale)];
            if (m_minData[comp] > sampleValue)
              m_minData[comp] = sampleValue;
            if (m_maxData[comp] < sampleValue)
              m_maxData[comp] = sampleValue;
            
            yCbCrDouble[comp] = convertCompValue(sampleValue, inp->m_sampleRange, inp->m_colorSpace, inp->m_bitDepth, comp);
          }
          
          m_cntMinYUV[comp] += (yCbCrDouble[comp] < compLimitsMin[comp]) ? 1 : 0;
          m_cntMaxYUV[comp] += (yCbCrDouble[comp] > compLimitsMax[comp]) ? 1 : 0;
        }
        
        m_cntMinYUV[3] += ((yCbCrDouble[Y_COMP] < compLimitsMin[Y_COMP]) || (yCbCrDouble[U_COMP] < compLimitsMin[U_COMP]) || (yCbCrDouble[V_COMP] < compLimitsMin[V_COMP])) ? 1 : 0;
        
        m_cntMaxYUV[3] += ((yCbCrDouble[Y_COMP] > compLimitsMax[Y_COMP]) || (yCbCrDouble[U_COMP] > compLimitsMax[U_COMP]) || (yCbCrDouble[V_COMP] > compLimitsMax[V_COMP])) ? 1 : 0;
        
        for (int comp = Y_COMP; comp <= V_COMP; comp++) {
          //yCbCrDouble[comp] = dClip(yCbCrDouble[comp], compLimitsMin[comp], compLimitsMax[comp]);
        }
        
        convertToRGB(yCbCrDouble, rgbDouble, transform0, transform1, transform2);
        for (int comp = R_COMP; comp <= B_COMP; comp++) {
          m_cntMinRGB[comp] += (rgbDouble[comp] < compLimitsMin[0]) ? 1 : 0;
          m_cntMaxRGB[comp] += (rgbDouble[comp] > compLimitsMax[0]) ? 1 : 0;
          m_pHistogram[comp][iMax(-m_histBinsDiv3, iMin(2*m_histBinsDiv3, (int) dRound(rgbDouble[comp] * m_histBinsDiv3)))] += 1;
        }
        
        int hasNegative = ((rgbDouble[R_COMP] < compLimitsMin[0]) || (rgbDouble[G_COMP] < compLimitsMin[0]) || (rgbDouble[B_COMP] < compLimitsMin[0])) ? 1 : 0;
        int hasPositive = ((rgbDouble[R_COMP] > compLimitsMax[0]) || (rgbDouble[G_COMP] > compLimitsMax[0]) || (rgbDouble[B_COMP] > compLimitsMax[0])) ? 1 : 0;
        
        m_cntMinRGB[3] += (double) hasNegative;
        m_cntMaxRGB[3] += (double) hasPositive;
        
        m_cntAllRGB += (double) (hasNegative + hasPositive);
        
      }
    }
  }
  for (int c = 0; c < m_totalComponents; c++) {
    m_minStats[c].updateStats(m_minData[c]);
    m_maxStats[c].updateStats(m_maxData[c]);
   
    m_minRGBStats[c].updateStats(m_cntMinRGB[c]);
    m_maxRGBStats[c].updateStats(m_cntMaxRGB[c]);
    m_minYUVStats[c].updateStats(m_cntMinYUV[c]);
    m_maxYUVStats[c].updateStats(m_cntMaxYUV[c]);
  }
  m_allRGBStats.updateStats(m_cntAllRGB);
  
  m_totalPixels = inp->m_compSize[Y_COMP];
}



//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void AnalyzeGamut::process (Frame* inp)
{
  // it is assumed here that the frames are of the same type
  if (inp->m_isFloat == FALSE) {    // non floating point data
    compute(inp);
  }
  else {
    printf("Computation not supported for floating point inputs.\n");
  }
}

void AnalyzeGamut::reportMetric  ()
{
  printf("%8.0f ", m_totalPixels);
  printf("%8d %8d %8d %8d %8d %8d ", m_minData[Y_COMP], m_maxData[Y_COMP], m_minData[U_COMP], m_maxData[U_COMP], m_minData[V_COMP],   m_maxData[V_COMP]);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_cntMinYUV[Y_COMP], m_cntMaxYUV[Y_COMP], m_cntMinYUV[U_COMP], m_cntMaxYUV[U_COMP], m_cntMinYUV[V_COMP], m_cntMaxYUV[V_COMP], m_cntMinYUV[3],  m_cntMaxYUV[3]);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_cntMinRGB[R_COMP], m_cntMaxRGB[R_COMP], m_cntMinRGB[G_COMP], m_cntMaxRGB[G_COMP], m_cntMinRGB[B_COMP], m_cntMaxRGB[B_COMP], m_cntMinRGB[3], m_cntMaxRGB[3], m_cntAllRGB);
}

void AnalyzeGamut::reportHistogram  ()
{
  FILE* f = IOFunctions::openFile(histFile, "w+t");
  
  if (f != NULL) {
    fprintf(f, "Histogram Stats\n");
    fprintf(f, "=======================\n");
    //    fprintf(f, "color\t index\t count  \n");
    //    for (int c = 0; c < 3; c++) {
    //      for (int i = -m_histBinsDiv3; i < 2 * m_histBinsDiv3 + 1; i++) {
    //        fprintf(f, "%d\t %+6.5f\t %d\n", c, (double) i / m_histBinsDiv3, (int) m_pHistogram[c][i]);
    //      }
    //    }
    fprintf(f, "RGBvalue\t   R-count\t   G-count\t   B-count\n");
    fprintf(f, "===============================================================\n");
      for (int i = -m_histBinsDiv3; i < 2 * m_histBinsDiv3 + 1; i++) {
        fprintf(f, "%+6.5f\t %9d\t %9d\t %9d\n", (double) i / m_histBinsDiv3, (int) m_pHistogram[0][i] , (int) m_pHistogram[1][i], (int) m_pHistogram[2][i]);
        //fprintf(f, "%+6.5f\t %9.2f\t %9.2f\t %9.2f\n", (double) i / m_histBinsDiv3, m_pHistogram[0][i] / (double) m_counter, m_pHistogram[1][i] / (double) m_counter, m_pHistogram[2][i] / (double) m_counter);
      }
    fprintf(f, "===============================================================\n");
    
  }
  
  IOFunctions::closeFile(f);
  
}


void AnalyzeGamut::reportSummary  ()
{
  printf("%8.0f ", m_totalPixels);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minStats[Y_COMP].getAverage(), m_maxStats[Y_COMP].getAverage(), m_minStats[U_COMP].getAverage(), m_maxStats[U_COMP].getAverage(), m_minStats[V_COMP].getAverage(),  m_maxStats[V_COMP].getAverage());
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minYUVStats[Y_COMP].getAverage(), m_maxYUVStats[Y_COMP].getAverage(), m_minYUVStats[U_COMP].getAverage(), m_maxYUVStats[U_COMP].getAverage(), m_minYUVStats[V_COMP].getAverage(), m_maxYUVStats[V_COMP].getAverage(), m_minYUVStats[3].getAverage(), m_maxYUVStats[3].getAverage());
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minRGBStats[R_COMP].getAverage(), m_maxRGBStats[R_COMP].getAverage(), m_minRGBStats[G_COMP].getAverage(), m_maxRGBStats[G_COMP].getAverage(), m_minRGBStats[B_COMP].getAverage(), m_maxRGBStats[B_COMP].getAverage(), m_minRGBStats[3].getAverage(), m_maxRGBStats[3].getAverage(), m_allRGBStats.getAverage());
}

void AnalyzeGamut::reportMinimum  ()
{
  printf("%8.0f ", m_totalPixels);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minStats[Y_COMP].minimum, m_maxStats[Y_COMP].minimum, m_minStats[U_COMP].minimum, m_maxStats[U_COMP].minimum, m_minStats[V_COMP].minimum, m_maxStats[V_COMP].minimum);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minYUVStats[Y_COMP].minimum, m_maxYUVStats[Y_COMP].minimum, m_minYUVStats[U_COMP].minimum, m_maxYUVStats[U_COMP].minimum, m_minYUVStats[V_COMP].minimum, m_maxYUVStats[V_COMP].minimum, m_minYUVStats[3].minimum, m_maxYUVStats[3].minimum);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minRGBStats[R_COMP].minimum, m_maxRGBStats[R_COMP].minimum, m_minRGBStats[G_COMP].minimum, m_maxRGBStats[G_COMP].minimum, m_minRGBStats[B_COMP].minimum, m_maxRGBStats[B_COMP].minimum, m_minRGBStats[3].minimum, m_maxRGBStats[3].minimum, m_allRGBStats.minimum);
}

void AnalyzeGamut::reportMaximum  ()
{
  printf("%8.0f ", m_totalPixels);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minStats[Y_COMP].maximum, m_maxStats[Y_COMP].maximum, m_minStats[U_COMP].maximum, m_maxStats[U_COMP].maximum, m_minStats[V_COMP].maximum, m_maxStats[V_COMP].maximum);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minYUVStats[Y_COMP].maximum, m_maxYUVStats[Y_COMP].maximum, m_minYUVStats[U_COMP].maximum, m_maxYUVStats[U_COMP].maximum, m_minYUVStats[V_COMP].maximum, m_maxYUVStats[V_COMP].maximum, m_minYUVStats[3].maximum, m_maxYUVStats[3].maximum);
  printf("%8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f %8.0f ", m_minRGBStats[R_COMP].maximum, m_maxRGBStats[R_COMP].maximum, m_minRGBStats[G_COMP].maximum, m_maxRGBStats[G_COMP].maximum, m_minRGBStats[B_COMP].maximum, m_maxRGBStats[B_COMP].maximum, m_minRGBStats[3].maximum, m_maxRGBStats[3].maximum, m_allRGBStats.maximum);
  reportHistogram  ();
}

void AnalyzeGamut::printHeader()
{
  printf("Pixels   "); // 9
  printf("  D0min  "); // 9
  printf("  D0max  "); // 9
  printf("  D1min  "); // 9
  printf("  D1max  "); // 9
  printf("  D2min  "); // 9
  printf("  D2max  "); // 9
  printf("  Ymin   "); // 9
  printf("  Ymax   "); // 9
  printf("  Umin   "); // 9
  printf("  Umax   "); // 9
  printf("  Vmin   "); // 9
  printf("  Vmax   "); // 9
  printf("TYUVmin  "); // 9
  printf("TYUVmax  "); // 9
  printf("  Rmin   "); // 9
  printf("  Rmax   "); // 9
  printf("  Gmin   "); // 9
  printf("  Gmax   "); // 9
  printf("  Bmin   "); // 9
  printf("  Bmax   "); // 9
  printf("TRGBmin  "); // 9
  printf("TRGBmax  "); // 9
  printf(" AllRGB  "); // 9
}

void AnalyzeGamut::printSeparator(){
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
  printf("---------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
