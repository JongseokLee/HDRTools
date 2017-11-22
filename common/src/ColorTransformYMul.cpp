/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/ISO
 * <ORGANIZATION> = Apple Inc
 * <YEAR> = 2017
 *
 * Copyright (c) 2017, Apple Inc
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
 * \file ColorTransformYMultiply.cpp
 *
 * \class 
 *    ColorTransformYMultiply
 *
 * \brief
 *    Closed-form luma adjustment method using a weighted adjustment
 *
 * \date
 *    $Date: 03/15/2017
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "ColorTransformYMultiply.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformYMultiply::ColorTransformYMultiply( ColorTransformParams *params ) {
  
  m_mode = CTF_IDENTITY; 
  m_invMode = m_mode;
  setupParams(params);

  m_size = 0;
  m_tfDistance = TRUE;
  
  // These are the weights for the distortion computation
  m_xWeight = 0.0;
  m_yWeight = 1.0;
  m_zWeight = 0.0;
  
  m_isICtCp = FALSE;

  for (int index = 0; index < 4; index++) {
    m_floatComp[index] = NULL;
    
    m_compSize[index] = 0;       // number of samples in each color component
    m_height[index] = 0;         // height of each color component
    m_width[index] = 0;          // width of each color component
  }
  m_memoryAllocated = FALSE;
  
  // Method is only allowed for RGB to YCbCr conversion.
  // This is not properly supported for ICtCp given the different behavior of the 
  // transform. Please do not use.
  if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr) {
    if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_YUV709;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_RGB709_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_YUV601;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_RGB601_2_XYZ;
    }    
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
      if (params->m_useHighPrecision == 0) {
        m_mode = CTF_RGB2020_2_YUV2020;
        m_invMode = m_mode;
      }
      else if (params->m_useHighPrecision == 1) {
        m_mode = CTF_RGB2020_2_YUV2020;
        m_invMode = CTF_RGB2020_2_YUV2020_HP;
      }
      else {
        m_mode = CTF_RGB2020_2_YUV2020_HP;
        m_invMode = CTF_RGB2020_2_YUV2020;
      }
      m_modeRGB2XYZ = CTF_RGB2020_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_YUVP3D65;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_RGBP3D65_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGBP3D60_2_YUVP3D60;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_RGBP3D60_2_XYZ;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBEXT_2_YUVEXT;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_RGBEXT_2_XYZ;
    }
    else if (m_oColorPrimaries == CP_AMT) {
      m_mode = CTF_RGB_2_AMT;
      m_invMode = m_mode;
      if (m_iColorPrimaries == CP_709) {
        m_modeRGB2XYZ = CTF_RGB709_2_XYZ;        
      }
      else if (m_iColorPrimaries == CP_601) {
        m_modeRGB2XYZ = CTF_RGB601_2_XYZ;
      }
      else if (m_iColorPrimaries == CP_2020) {
        m_modeRGB2XYZ = CTF_RGB2020_2_XYZ;        
      }
      else if (m_iColorPrimaries == CP_P3D65) {
        m_modeRGB2XYZ = CTF_RGBP3D65_2_XYZ;        
      }
      else if (m_iColorPrimaries == CP_P3D60) {
        m_modeRGB2XYZ = CTF_RGBP3D60_2_XYZ;        
      }
    }
    else if (m_oColorPrimaries == CP_YCOCG) {
      m_mode = CTF_RGB_2_YCOCG;
      m_invMode = m_mode;
      if (m_iColorPrimaries == CP_709) {
        m_modeRGB2XYZ = CTF_RGB709_2_XYZ;        
      }
      else if (m_iColorPrimaries == CP_601) {
        m_modeRGB2XYZ = CTF_RGB601_2_XYZ;
      }
      else if (m_iColorPrimaries == CP_2020) {
        m_modeRGB2XYZ = CTF_RGB2020_2_XYZ;        
      }
      else if (m_iColorPrimaries == CP_P3D65) {
        m_modeRGB2XYZ = CTF_RGBP3D65_2_XYZ;        
      }
      else if (m_iColorPrimaries == CP_P3D60) {
        m_modeRGB2XYZ = CTF_RGBP3D60_2_XYZ;        
      }
    }
  }
  else if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_ICtCp) {
    if (m_iColorPrimaries == CP_LMSD && m_oColorPrimaries == CP_LMSD) {
      printf("Conversion not supported for ICtCp space\n");
      m_mode = CTF_LMSD_2_ICtCp;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_LMSD_2_XYZ;
      m_isICtCp     = TRUE;
    }
    else {
      m_mode = CTF_IDENTITY;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_IDENTITY;
    }
  }
  else {
    m_mode = CTF_IDENTITY;
    m_invMode = m_mode;
    m_modeRGB2XYZ = CTF_IDENTITY;
  }
  
  // Forward Transform coefficients 
  m_transform0 = FWD_TRANSFORM[m_mode][Y_COMP];
  m_transform1 = FWD_TRANSFORM[m_mode][U_COMP];
  m_transform2 = FWD_TRANSFORM[m_mode][V_COMP];

  // Inverse Transform coefficients 
  m_invTransform0 = INV_TRANSFORM[m_invMode][Y_COMP];
  m_invTransform1 = INV_TRANSFORM[m_invMode][U_COMP];
  m_invTransform2 = INV_TRANSFORM[m_invMode][V_COMP];
  
  // Transform coefficients for conversion to Y in XYZ
  m_transformRGBtoX = FWD_TRANSFORM[m_modeRGB2XYZ][0];
  m_transformRGBtoY = FWD_TRANSFORM[m_modeRGB2XYZ][1];
  m_transformRGBtoZ = FWD_TRANSFORM[m_modeRGB2XYZ][2];

  m_fwdColorFormat = NULL;
  m_invColorFormat = NULL;
  m_fwdFrameStore  = NULL;
  m_invFrameStore  = NULL;
  
  m_fwdConvertProcess = NULL; 
  m_invConvertProcess = NULL; 
  m_fwdFrameStore2  = NULL; 
  m_invFrameStore2  = NULL;

  if (m_range == SR_STANDARD) {
    m_lumaWeight   = (double) (1 << (m_bitDepth - 8)) * 219.0;
    m_lumaOffset   = (double) (1 << (m_bitDepth - 8)) * 16.0;
    m_chromaWeight = (double) (1 << (m_bitDepth - 8)) * 224.0;
    m_chromaOffset = (double) (1 << (m_bitDepth - 1));
  }
  else if (m_range == SR_FULL) {
    m_lumaWeight   = (double) (1 << (m_bitDepth)) - 1.0;
    m_lumaOffset   = 0.0;
    m_chromaWeight = (double) (1 << (m_bitDepth)) - 1.0;
    m_chromaOffset = 0.0;
  }
  else if (m_range == SR_SDI) {
    m_lumaWeight   = (double) (1 << (m_bitDepth - 8)) * 253.75;
    m_lumaOffset   = (double) (1 << (m_bitDepth - 8));
    m_chromaWeight = (double) (1 << (m_bitDepth - 8)) * 253.0;
    m_chromaOffset = (double) (1 << (m_bitDepth - 1));
  }

  m_iLumaWeight = (int) m_lumaWeight;
  if (m_transferFunctions != TF_HLG)
    m_useAlternate = TRUE;
  else
    m_useAlternate = FALSE;

  m_transferFunction = TransferFunction::create(m_transferFunctions, TRUE, 1.0, params->m_oSystemGamma, params->m_minValue, params->m_maxValue, params->m_enableLUTs);
}

ColorTransformYMultiply::~ColorTransformYMultiply() {
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  // Color format conversion elements
  if (m_fwdColorFormat) {
    delete m_fwdColorFormat;
    m_fwdColorFormat = NULL;
  }
  if (m_invColorFormat) {
    delete m_invColorFormat;
    m_invColorFormat = NULL;
  }
  
   // Color format conversion frame stores
  if (m_fwdFrameStore) {
    delete m_fwdFrameStore;
    m_fwdFrameStore = NULL;
  }
  if (m_invFrameStore) {
    delete m_invFrameStore;
    m_invFrameStore = NULL;
  }
  
  if (m_transferFunction) {
    delete m_transferFunction;
    m_transferFunction = NULL;
  }
  
  // Delete buffers for integer processing if they were allocated
  if(m_fwdConvertProcess) {
    delete m_fwdConvertProcess;
    m_fwdConvertProcess = NULL;
  }
  if(m_invConvertProcess) {
    delete m_invConvertProcess;
    m_invConvertProcess = NULL;
  }
  if (m_fwdFrameStore2) {
    delete m_fwdFrameStore2;
    m_fwdFrameStore2 = NULL;
  }
  if (m_invFrameStore2) {
    delete m_invFrameStore2;
    m_invFrameStore2 = NULL;
  }
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ColorTransformYMultiply::allocateMemory(Frame* out, const Frame *inp) {
  m_width [Y_COMP] = inp->m_width[ Y_COMP];
  m_width [Y_COMP] = inp->m_width[ U_COMP];
  m_width [Y_COMP] = inp->m_width[ V_COMP];
  
  for (int index = ZERO; index < 3; index++) {
    m_width[index]  = inp->m_width[index];
    m_height[index] = inp->m_height[index];
    
    m_compSize[index] = inp->m_compSize[index];
  }
  
  m_size =  m_compSize[ZERO] + m_compSize[ONE] + m_compSize[TWO];
  m_floatData.resize((unsigned int) m_size);
  if (m_floatData.size() != (unsigned int) m_size) {
    fprintf(stderr, "ColorTransformYMultiply: Not enough memory to create array m_floatData, of size %d", (int) m_size);
    exit(-1);
  }
  
  m_floatComp[Y_COMP] = &m_floatData[0];
  m_floatComp[U_COMP] = m_floatComp[Y_COMP] + m_compSize[Y_COMP];
  m_floatComp[V_COMP] = m_floatComp[U_COMP] + m_compSize[U_COMP];
  
  m_fwdColorFormat = ConvertColorFormat::create(m_width[Y_COMP], m_height[Y_COMP], inp->m_chromaFormat, m_oChromaFormat, m_downMethod, (ChromaLocation *) &(inp->m_format.m_chromaLocation[0]), (ChromaLocation *)  &(m_oChromaLocation[0]), m_useAdaptiveDownsampler, m_useMinMax);
  
  m_invColorFormat = ConvertColorFormat::create(m_width[Y_COMP], m_height[Y_COMP], m_oChromaFormat, inp->m_chromaFormat, m_upMethod, (ChromaLocation *) &(m_oChromaLocation[0]), (ChromaLocation *)  &(inp->m_format.m_chromaLocation[0]), m_useAdaptiveUpsampler);
  
  if (m_useFloatPrecision == FALSE) {
    // Format conversion process
    FrameFormat inFormat  = out->m_format;
    inFormat.m_colorSpace = m_oColorSpace;
    FrameFormat outFormat = out->m_format;
    outFormat.m_isFloat = FALSE;
    outFormat.m_bitDepthComp[0] = m_bitDepth;
    outFormat.m_bitDepthComp[1] = m_bitDepth;
    outFormat.m_bitDepthComp[2] = m_bitDepth;
    outFormat.m_colorSpace = m_oColorSpace;
    
    m_fwdConvertProcess = Convert::create(&inFormat, &outFormat);
    m_invConvertProcess = Convert::create(&outFormat, &inFormat);
    
    m_fwdFrameStore  = new Frame(m_width[Y_COMP], m_height[Y_COMP], FALSE, m_oColorSpace, out->m_colorPrimaries, m_oChromaFormat, out->m_sampleRange, m_bitDepth, FALSE, m_transferFunctions, 1.0);      
    m_fwdFrameStore2  = new Frame(m_width[Y_COMP], m_height[Y_COMP], FALSE, m_oColorSpace, out->m_colorPrimaries, inp->m_chromaFormat, m_range, m_bitDepth, FALSE, m_transferFunctions, 1.0);      
    
    m_invFrameStore  = new Frame(m_width[Y_COMP], m_height[Y_COMP], TRUE, inp->m_colorSpace, inp->m_colorPrimaries, inp->m_chromaFormat, m_range, inp->m_bitDepthComp[Y_COMP], FALSE, m_transferFunctions, 1.0);      
    m_invFrameStore2  = new Frame(m_width[Y_COMP], m_height[Y_COMP], FALSE, m_oColorSpace, inp->m_colorPrimaries, inp->m_chromaFormat, m_range, m_bitDepth, FALSE, m_transferFunctions, 1.0);      
  }
  else {
    m_fwdFrameStore  = new Frame(m_width[Y_COMP], m_height[Y_COMP], TRUE, out->m_colorSpace, out->m_colorPrimaries, m_oChromaFormat, out->m_sampleRange, out->m_bitDepthComp[Y_COMP], FALSE, m_transferFunctions, 1.0);      
    m_invFrameStore  = new Frame(m_width[Y_COMP], m_height[Y_COMP], TRUE, inp->m_colorSpace, inp->m_colorPrimaries, inp->m_chromaFormat, inp->m_sampleRange, inp->m_bitDepthComp[Y_COMP], FALSE, m_transferFunctions, 1.0);      
  }  

  m_memoryAllocated = TRUE;
}

void ColorTransformYMultiply::computeColorImpact(const double uComp, const double vComp, double *rColor, double *gColor, double *bColor) {
  *rColor = m_invTransform0[1] * uComp + m_invTransform0[2] * vComp;
  *gColor = m_invTransform1[1] * uComp + m_invTransform1[2] * vComp;
  *bColor = m_invTransform2[1] * uComp + m_invTransform2[2] * vComp;
}

void ColorTransformYMultiply::computeColorImpactBasic(const double uComp, const double vComp, double *rColor, double *gColor, double *bColor) {
  // all the values used apart from vComp and uComp are constants and thus we could precompute those (resulting in just 4 multiplies and 1 addition
  // TBD
  *rColor = 2 * (1 - m_transform0[0]) * vComp;
  *bColor = 2 * (1 - m_transform0[2]) * uComp;
  *gColor = ((*bColor * m_transform0[2] +  *rColor * m_transform0[0]) / (1 - m_transform0[0] - m_transform0[2]));
}

void ColorTransformYMultiply::convertToXYZLinear(double rComp, double gComp, double bComp, double *xComp,  double *yComp, double *zComp) {
  rComp = m_transferFunction->forward(rComp);
  gComp = m_transferFunction->forward(gComp);
  bComp = m_transferFunction->forward(bComp);
  *xComp = (m_transformRGBtoX[0] * rComp + m_transformRGBtoX[1] * gComp + m_transformRGBtoX[2] * bComp);
  *yComp = (m_transformRGBtoY[0] * rComp + m_transformRGBtoY[1] * gComp + m_transformRGBtoY[2] * bComp);
  *zComp = (m_transformRGBtoZ[0] * rComp + m_transformRGBtoZ[1] * gComp + m_transformRGBtoZ[2] * bComp);
}

void  ColorTransformYMultiply::convertToYCbCr(double rComp, double gComp, double bComp, float *yComp,  float *uComp, float *vComp) {
  *yComp = (float) (m_transform0[0] * rComp + m_transform0[1] * gComp + m_transform0[2] * bComp);
  *uComp = (float) (m_transform1[0] * rComp + m_transform1[1] * gComp + m_transform1[2] * bComp);
  *vComp = (float) (m_transform2[0] * rComp + m_transform2[1] * gComp + m_transform2[2] * bComp);
}

void ColorTransformYMultiply::convertToXYZ(double rComp, double gComp, double bComp, double *xComp,  double *yComp, double *zComp) {
  *xComp = (m_transformRGBtoX[0] * rComp + m_transformRGBtoX[1] * gComp + m_transformRGBtoX[2] * bComp);
  *yComp = (m_transformRGBtoY[0] * rComp + m_transformRGBtoY[1] * gComp + m_transformRGBtoY[2] * bComp);
  *zComp = (m_transformRGBtoZ[0] * rComp + m_transformRGBtoZ[1] * gComp + m_transformRGBtoZ[2] * bComp);
}

void ColorTransformYMultiply::convertToXYZLinear(const double yValue, const double rColor, const double gColor, const double bColor, double *xComp,  double *yComp, double *zComp) {
  if (yValue == 0.0) {
    *xComp = 0.0;
    *yComp = 0.0;
    *zComp = 0.0;
        }
  else  {
    const double rComp = m_transferFunction->forward(dClip((m_invTransform0[0] * yValue + rColor), 0.0, 1.0));
    const double gComp = m_transferFunction->forward(dClip((m_invTransform1[0] * yValue + gColor), 0.0, 1.0));
    const double bComp = m_transferFunction->forward(dClip((m_invTransform2[0] * yValue + bColor), 0.0, 1.0));
    
    *xComp = (m_transformRGBtoX[0] * rComp + m_transformRGBtoX[1] * gComp + m_transformRGBtoX[2] * bComp);
    *yComp = (m_transformRGBtoY[0] * rComp + m_transformRGBtoY[1] * gComp + m_transformRGBtoY[2] * bComp);
    *zComp = (m_transformRGBtoZ[0] * rComp + m_transformRGBtoZ[1] * gComp + m_transformRGBtoZ[2] * bComp);
  }
}

void ColorTransformYMultiply::calcBoundsFast(int &ypBufLowPix, int &ypBufHighPix, double yLinear, const double rColor, const double gColor, const double bColor)
{
  double yTF = m_transferFunction->inverse(yLinear);
  double boundR = m_invTransform0[0] * yTF - rColor;
  double boundG = m_invTransform1[0] * yTF - gColor;
  double boundB = m_invTransform2[0] * yTF - bColor;
  
  double ypLowest0To1 = dMin(dMin(boundR, boundG), boundB);
  int ypLowestQuant = iClip(double2IntFloor(m_lumaWeight * ypLowest0To1), 0, m_iLumaWeight);
  
  // Calculate ypHighest0To1 = yTF, but we have to go to integer, round up and go back again.
  int ypHighestQuant = iClip(double2IntCeil(m_lumaWeight * yTF), 0, m_iLumaWeight);
  double ypHighest0To1 = (double) ypHighestQuant / m_lumaWeight;
  
  double testYpHighestR = m_invTransform0[0] * ypHighest0To1 - rColor;
  double testYpHighestG = m_invTransform1[0] * ypHighest0To1 - gColor;
  double testYpHighestB = m_invTransform2[0] * ypHighest0To1 - bColor;
  
  if(testYpHighestR > 1.0 || testYpHighestG > 1.0 || testYpHighestB > 1.0)  {
    // Instead use a "looser" bound that is safe.
    double ypLargest0To1 = dMax(dMax(boundR, boundG), boundB);
    ypHighestQuant = iClip(double2IntCeil(m_lumaWeight * ypLargest0To1), 0, m_iLumaWeight);
  }
  
  // The lowest possible and highest possible value for Yprime
  ypBufLowPix  = ypLowestQuant;
  ypBufHighPix = ypHighestQuant;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ColorTransformYMultiply::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  // Current condition to perform this is that Frames are of same size and in 4:4:4
  // Can add more code to do the interpolation on the fly (and save memory/improve speed),
  // but this keeps our code more flexible for now.
  
  if (inp->m_compSize[Y_COMP] == out->m_compSize[Y_COMP] && inp->m_compSize[Y_COMP] == inp->m_compSize[U_COMP])  {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE)  {
      double xLinear, yLinear, zLinear;
      double xConv, yConv, zConv;
      double rColor, gColor, bColor;
      double yComp, uComp, vComp;
      //double rComp, gComp, bComp;
      int    iYCompMin, iYCompMax; 
      float *floatComp[3];
      double scale = 1.0;
      void (ColorTransformYMultiply::*pt2Convert)(double, double, double, double*, double*, double*) = NULL;
      float *red   = inp->m_floatComp[0];
      float *green = inp->m_floatComp[1];
      float *blue  = inp->m_floatComp[2];

      if (m_useAlternate && inp->m_hasAlternate == TRUE) {
        floatComp[0] = inp->m_altFrame->m_floatComp[0];
        floatComp[1] = inp->m_altFrame->m_floatComp[1];
        floatComp[2] = inp->m_altFrame->m_floatComp[2];
        scale        = 1.0 / inp->m_altFrameNorm;
        pt2Convert   = &ColorTransformYMultiply::convertToXYZ;
      }
      else {
        floatComp[0] = inp->m_floatComp[0];
        floatComp[1] = inp->m_floatComp[1];
        floatComp[2] = inp->m_floatComp[2];   
        scale        = 1.0;        
        pt2Convert  = &ColorTransformYMultiply::convertToXYZLinear;     
      }

      // Allocate memory. Note that current code does not permit change of resolution. TBDL
      if (m_memoryAllocated == FALSE) { 
        allocateMemory(out, inp);
      }              
      
      // First convert all components as per the described transform process 
      for (int i = 0; i < inp->m_compSize[0]; i++) {
        convertToYCbCr((double) inp->m_floatComp[0][i], (double) inp->m_floatComp[1][i], (double) inp->m_floatComp[2][i], &(out->m_floatComp[0][i]), &(out->m_floatComp[1][i]), &(out->m_floatComp[2][i]));
      }
      
      if (m_useFloatPrecision == FALSE) {       // Integer conversion
                                                // convert float to fixed
        // convert bitdepth
        m_fwdConvertProcess->process(m_fwdFrameStore2, out);
        
        // Downscale (if needed)
        m_fwdColorFormat->process (m_fwdFrameStore, m_fwdFrameStore2);
        
        // Now convert back to 4:4:4 (if needed)
        m_invColorFormat->process (m_invFrameStore2, m_fwdFrameStore);
        
        // fixed to float
        m_invConvertProcess->process(m_invFrameStore, m_invFrameStore2);
      }
      else {   // Floating conversion
               // Downconvert if needed
        m_fwdColorFormat->process (m_fwdFrameStore, out);
        
        // Luma only if adaptive upsampler is enabled
        if (m_useAdaptiveUpsampler == TRUE) {
          for (int i = 0; i < m_fwdFrameStore->m_compSize[0]; i++) {
            m_fwdFrameStore->m_floatComp[0][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[0][i] * m_lumaWeight) / m_lumaWeight);
          }          
        }
        
        // Quantize/DeQuantize Chroma components
        for (int i = 0; i < m_fwdFrameStore->m_compSize[1]; i++) {
          m_fwdFrameStore->m_floatComp[1][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[1][i] * m_chromaWeight) / m_chromaWeight);
          m_fwdFrameStore->m_floatComp[2][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[2][i] * m_chromaWeight) / m_chromaWeight);
        }
        
        // Now convert back to 4:4:4
        m_invColorFormat->process (m_invFrameStore, m_fwdFrameStore);
      }
      
      double uvDenom = 1 / (1 - m_transform0[0] - m_transform0[2]);
      double uScale  = (2.0 * (1 - m_transform0[2]));
      double vScale  = (2.0 * (1 - m_transform0[0]));
      
      for (int i = 0; i < inp->m_compSize[0]; i++) {
        yComp  = (double) m_invFrameStore->m_floatComp[0][i];
        uComp  = (double) m_invFrameStore->m_floatComp[1][i];
        vComp  = (double) m_invFrameStore->m_floatComp[2][i];       
        
        double vOffset = vComp * vScale;
        double uOffset = uComp * uScale;
        double gOffset = ((uOffset * m_transform0[2] +  vOffset * m_transform0[0]) * uvDenom);
        
        double yValueR = red  [i] - vOffset;
        double yValueB = blue [i] - uOffset;
        double yValueG = green[i] + gOffset;
        
        int yValueRQ = (int) dRound(yValueR * m_lumaWeight);
        int yValueBQ = (int) dRound(yValueB * m_lumaWeight);
        int yValueGQ = (int) dRound(yValueG * m_lumaWeight);
        
        // If all values agree, no need to perform a refinement
        if (yValueRQ == yValueGQ && yValueRQ == yValueBQ) {
          out->m_floatComp[0][i] = (float) ((double) yValueRQ / m_lumaWeight);
        }
        else {          
          // Compute the linear value of the target Y (given original data)
          (*this.*pt2Convert)((double)floatComp[0][i] * scale, (double) floatComp[1][i] * scale,(double) floatComp[2][i] * scale, &xLinear, &yLinear, &zLinear);

          int yValueMin = iMin(yValueRQ, iMin(yValueBQ, yValueGQ));
          int yValueMax = iMax(yValueRQ, iMax(yValueBQ, yValueGQ));
          
          computeColorImpact(uComp, vComp, &rColor, &gColor, &bColor);          
          if (m_isICtCp == FALSE) {
            calcBoundsFast(iYCompMin, iYCompMax, yLinear, rColor, gColor, bColor);
            
            iYCompMin = iMax(iYCompMin, yValueMin);
            iYCompMax = iMin(iYCompMax, yValueMax);
          }
          else {
            iYCompMin = 0;
            iYCompMax = (int) m_lumaWeight;
          }
          
          if (iYCompMin == iYCompMax) {
            //convertToXYZLinear((double) iYCompMin / m_lumaWeight, rColor, gColor, bColor, &xConvMin, &yConvMin, &zConvMin);
            out->m_floatComp[0][i] = (float) ((double) iYCompMin / m_lumaWeight);            
          }
          else {
            // Given reconstruction convert also inverse data
            if(m_tfDistance == TRUE) {
            
              double yTF = m_transferFunction->inverse(yLinear);

              convertToXYZLinear(yComp, rColor, gColor, bColor, &xConv, &yConv, &zConv);
              
              if (yConv != 0.0 && yLinear != 0.0) {
                double scaleX = (yTF + 0.0000000001) / ( m_transferFunction->inverse(yConv) + 0.0000000001);
                out->m_floatComp[0][i] = (float) dMin(yComp * scaleX, 1.0);
              }
              else {
                out->m_floatComp[0][i] = (float) yComp;
              }
            }
            else {
              convertToXYZLinear(yComp, rColor, gColor, bColor, &xConv, &yConv, &zConv);
              
              if (yConv != 0.0 && yLinear != 0.0) {
                scale = (yLinear + 0.000001) / ( yConv + 0.000001);
                out->m_floatComp[0][i] = (float) dMin(yComp * scale, 1.0);
              }
              else {
                out->m_floatComp[0][i] = (float) yComp;
              }
            }
          }
        }
      }
    }
    else { 
      // fixed precision, integer image data. 
      // We should always ideally have inp->m_bitDepth == out->m_bitDepth. Still, the code currently does not
      // perform any enforcing of that condition, but maybe we should.
      if (inp->m_bitDepth == out->m_bitDepth) {
        if (inp->m_bitDepth > 8) {
          // In this scenario, we always clip
          for (int i = 0; i < inp->m_compSize[0]; i++) {
            // Note that since the input and output may be either RGB or YUV, it might be "better" not to use Y_COMP/R_COMP here to avoid confusion.
            out->m_ui16Comp[0][i] = (uint16) fClip((float) (m_transform0[0] * (float) inp->m_ui16Comp[0][i] + m_transform0[1] * (float) inp->m_ui16Comp[1][i] + m_transform0[2] * (float) inp->m_ui16Comp[2][i] + 0.5), (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0]);
            out->m_ui16Comp[1][i] = (uint16) fClip((float) (m_transform1[0] * (float) inp->m_ui16Comp[0][i] + m_transform1[1] * (float) inp->m_ui16Comp[1][i] + m_transform1[2] * (float) inp->m_ui16Comp[2][i] + 0.5), (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1]);
            out->m_ui16Comp[2][i] = (uint16) fClip((float) (m_transform2[0] * (float) inp->m_ui16Comp[0][i] + m_transform2[1] * (float) inp->m_ui16Comp[1][i] + m_transform2[2] * (float) inp->m_ui16Comp[2][i] + 0.5), (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2]);
          }
        }
        else {
          // In this scenario, we always clip
          for (int i = 0; i < inp->m_compSize[0]; i++) {
            // Note that since the input and output may be either RGB or YUV, it might be "better" not to use Y_COMP/R_COMP here to avoid confusion.
            out->m_comp[0][i] = (imgpel) fClip((float) (m_transform0[0] * (float) inp->m_comp[0][i] + m_transform0[1] * (float) inp->m_comp[1][i] + m_transform0[2] * (float) inp->m_comp[2][i] + 0.5), (float) out->m_minPelValue[0], (float) out->m_maxPelValue[0]);
            out->m_comp[1][i] = (imgpel) fClip((float) (m_transform1[0] * (float) inp->m_comp[0][i] + m_transform1[1] * (float) inp->m_comp[1][i] + m_transform1[2] * (float) inp->m_comp[2][i] + 0.5), (float) out->m_minPelValue[1], (float) out->m_maxPelValue[1]);
            out->m_comp[2][i] = (imgpel) fClip((float) (m_transform2[0] * (float) inp->m_comp[0][i] + m_transform2[1] * (float) inp->m_comp[1][i] + m_transform2[2] * (float) inp->m_comp[2][i] + 0.5), (float) out->m_minPelValue[2], (float) out->m_maxPelValue[2]);
          }
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
