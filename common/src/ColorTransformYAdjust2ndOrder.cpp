/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/ISO
 * <ORGANIZATION> = CISRA, Apple Inc, Ericsson
 * <YEAR> = 2015
 *
 * Copyright (c) 2016, CISRA, Apple Inc, Ericsson
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
 * \file ColorTransformYAdjust2ndOrder.cpp
 *
 * \class 
 *    ColorTransformYAdjust2ndOrder
 *
 * \brief
 *    2nd order approximation model method using LUTs for luma adjustment
 *    JCTVC-W0056: CE1-related: LUT-based luma sample adjustment
 *
 * \author
 *     - Vladimir Kolesnikov             <vladimir.kolesnikov@cisra.canon.com.au>
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *     - Jacob Strom                     <jacob.strom@ericsson.com>
 *     - Jonatan Samuelsson              <jonatan.samuelsson@ericsson.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "ColorTransformYAdjust2ndOrder.H"
#include <fstream>
#include <cassert>

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformYAdjust2ndOrder::ColorTransformYAdjust2ndOrder( ColorTransformParams *params ) 
{  
  m_mode = CTF_IDENTITY; 
  m_invMode = m_mode;
  
  setupParams(params);
  
  m_size = 0;
  m_tfDistance = TRUE;
  
  for (int index = 0; index < 4; index++) {
    m_floatComp[index] = NULL;
    m_compSize [index] = 0;       // number of samples in each color component
    m_height   [index] = 0;       // height of each color component
    m_width    [index] = 0;       // width of each color component
  }
  m_memoryAllocated = FALSE;
  m_isICtCp         = FALSE;  
  
  // Method supports mainly RGB to YCbCr conversion. LMS to ICtCp is partially supported but
  // due to monotonicity issues it might not always result in best performance.
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
      m_mode = CTF_LMSD_2_ICtCp;
      m_invMode = m_mode;
      m_modeRGB2XYZ = CTF_LMSD_2_XYZ;
      m_isICtCp = TRUE;
    }
    else {
    // Not supported properly yet
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
  m_transformRGBtoY = FWD_TRANSFORM[m_modeRGB2XYZ][1];
  
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
  m_luminanceRange = (double) (1 << m_bitDepth);

  m_transferFunction = TransferFunction::create(m_transferFunctions, TRUE, 1.0, params->m_oSystemGamma, params->m_minValue, params->m_maxValue, params->m_enableLUTs);
  setModelFile(params->m_yAdjustModelFile);
}

ColorTransformYAdjust2ndOrder::~ColorTransformYAdjust2ndOrder() {
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
void ColorTransformYAdjust2ndOrder::allocateMemory(Frame* out, const Frame *inp) {
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
    fprintf(stderr, "ColorTransformYAdjust2ndOrder: Not enough memory to create array m_floatData, of size %d", (int) m_size);
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

void ColorTransformYAdjust2ndOrder::setModelFile(const char *modelFile) {
  
  m_modelFile.assign(modelFile); 
  
  int sz;
  ifstream is(m_modelFile.c_str());
  is >> sz;
  
  // Do some very basic error handling
  // This needs to be revised
  if (is.rdstate() == 0 && sz > 0) {   
    m_searchTable.resize(sz);
    
    for (int i = 0; i < sz; i++) {
      is >> m_searchTable[i].m_index;
      is >> m_searchTable[i].m_leaf;
    }
    
    is >> sz;
    m_coeffTable.resize(sz * m_modelCoeffs);
    for (int i = 0; i < sz * m_modelCoeffs; i+= m_modelCoeffs) {
      is >> m_coeffTable[i + 0];
      is >> m_coeffTable[i + 1];
      is >> m_coeffTable[i + 2];
      is >> m_coeffTable[i + 3];
      is >> m_coeffTable[i + 4];
      is >> m_coeffTable[i + 5];
      is >> m_coeffTable[i + 6];
    }
    
    if (is.rdstate() == 0)
    return;
  }
  
  // if we encountered an error, lets use defaults
    sz = 10;
    m_searchTable.resize(10);
    for (int i = 0; i < sz; i++) {
      m_searchTable[i].m_index = i;
      m_searchTable[i].m_leaf = TRUE;
    }
    
    m_coeffTable.resize(sz * m_modelCoeffs);
    for (int i = 0; i < sz * m_modelCoeffs; i+= m_modelCoeffs) {
      m_coeffTable[i + 0] = 1;
      m_coeffTable[i + 1] = 1;
      m_coeffTable[i + 2] = 1;
      m_coeffTable[i + 3] = 1;
      m_coeffTable[i + 4] = 1;
      m_coeffTable[i + 5] = 1;
      m_coeffTable[i + 6] = 1;
    }
}


double ColorTransformYAdjust2ndOrder::convertToYLinear(const double rComp, const double gComp, const double bComp) {
  return (m_transformRGBtoY[0] * m_transferFunction->getForward(rComp) + m_transformRGBtoY[1] * m_transferFunction->getForward(gComp) + m_transformRGBtoY[2] * m_transferFunction->getForward(bComp));
}

double ColorTransformYAdjust2ndOrder::convertToY(const double rComp, const double gComp, const double bComp) {
  return (m_transformRGBtoY[0] * rComp + m_transformRGBtoY[1] * gComp + m_transformRGBtoY[2] * bComp);
}

int ColorTransformYAdjust2ndOrder::tableSearch(unsigned comp0, unsigned comp1, unsigned comp2) const
{
  for (int offset = 0, bitno  = sizeof(unsigned) * 8 - 1; ; bitno--) {
    
    offset += ((comp0 >> (bitno - 2)) & 4) + ((comp1 >> (bitno - 1)) & 2) + ((comp2 >> bitno) & 1);
    
    if (m_searchTable[offset].m_leaf) {
      return offset;
    }
    
    offset = m_searchTable[offset].m_index;
  }
}

int ColorTransformYAdjust2ndOrder::calcYprime(int ylin, int cb, int cr, const double *coeffs)
{
  const double yd  = (double)(ylin * ylin);
  const double cbd = (double)(cb * cb);
  const double crd = (double)(cr * cr);
  
  return (int)(coeffs[0] * yd + coeffs[1] * ylin + coeffs[2] * cbd +
               coeffs[3] * cb + coeffs[4] * crd  + coeffs[5] * cr + coeffs[6] + 0.5);
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void ColorTransformYAdjust2ndOrder::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  // Current condition to perform this is that Frames are of same size and in 4:4:4
  // Can add more code to do the interpolation on the fly (and save memory/improve speed),
  // but this keeps our code more flexible for now.
  
  if (inp->m_compSize[Y_COMP] == out->m_compSize[Y_COMP] && inp->m_compSize[Y_COMP] == inp->m_compSize[U_COMP])  {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE)  {
      double yLinear;
      double uComp, vComp;
      float *floatComp[3];
      double scale = 1.0;
      double (ColorTransformYAdjust2ndOrder::*pt2Convert)(double, double, double) = NULL;
      
      if (inp->m_hasAlternate == TRUE) {
        floatComp[0] = inp->m_altFrame->m_floatComp[0];
        floatComp[1] = inp->m_altFrame->m_floatComp[1];
        floatComp[2] = inp->m_altFrame->m_floatComp[2];
        scale       = 1.0 / inp->m_altFrameNorm;
        pt2Convert  = &ColorTransformYAdjust2ndOrder::convertToY;
      }
      else {
        floatComp[0] = inp->m_floatComp[0];
        floatComp[1] = inp->m_floatComp[1];
        floatComp[2] = inp->m_floatComp[2];
        scale        = 1.0;        
        pt2Convert  = &ColorTransformYAdjust2ndOrder::convertToYLinear;
      }

      // Allocate memory. Note that current code does not permit change of resolution. TBDL
      if (m_memoryAllocated == FALSE) { 
        allocateMemory(out, inp);
      }              
      
      // First convert all components as per the described transform process 
      for (int i = 0; i < inp->m_compSize[0]; i++) {
        out->m_floatComp[0][i] = (float) (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i]);
        out->m_floatComp[1][i] = (float) (m_transform1[0] * (double) inp->m_floatComp[0][i] + m_transform1[1] * (double) inp->m_floatComp[1][i] + m_transform1[2] * (double) inp->m_floatComp[2][i]);
        out->m_floatComp[2][i] = (float) (m_transform2[0] * (double) inp->m_floatComp[0][i] + m_transform2[1] * (double) inp->m_floatComp[1][i] + m_transform2[2] * (double) inp->m_floatComp[2][i]);
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
        // Downscale (if needed)
        m_fwdColorFormat->process (m_fwdFrameStore, out);
        
        // Quantize/DeQuantize components
        // Luma only if adaptive upsampler is enabled
        if (m_useAdaptiveUpsampler == TRUE) {
          for (int i = 0; i < m_fwdFrameStore->m_compSize[0]; i++) {
            m_fwdFrameStore->m_floatComp[0][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[0][i] * m_lumaWeight) / m_lumaWeight);
          }          
        }
        // /Now also chroma - if m_useAdaptiveUpsampler is false, then we do not need to quantize that
        for (int i = 0; i < m_fwdFrameStore->m_compSize[1]; i++) {
          m_fwdFrameStore->m_floatComp[1][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[1][i] * m_chromaWeight) / m_chromaWeight);
          m_fwdFrameStore->m_floatComp[2][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[2][i] * m_chromaWeight) / m_chromaWeight);
        }
        
        // Now convert back to 4:4:4 (if needed)
        m_invColorFormat->process (m_invFrameStore, m_fwdFrameStore);
      }
      
      for (int i = 0; i < inp->m_compSize[0]; i++) {
        // get target y linear value
        yLinear = (*this.*pt2Convert)((double) floatComp[0][i] * scale, (double) floatComp[1][i] * scale,(double) floatComp[2][i] * scale);

        // apply OETF, quantize, get 1st LUT index
        double doubleIndex = m_transferFunction->getInverse(dClip((double) yLinear, 0.0, 1.0));       
        int yLinearIndex = (int)(doubleIndex * m_luminanceRange + 0.5);
        
        uComp = (double) m_invFrameStore->m_floatComp[1][i];
        vComp = (double) m_invFrameStore->m_floatComp[2][i];
                
        int cbIndex = (int) ((m_chromaWeight * uComp + m_chromaOffset) + 0.5);
        int crIndex = (int) ((m_chromaWeight * vComp + m_chromaOffset) + 0.5);
        
        unsigned comp0 = (yLinearIndex << 22);
        unsigned comp1 = (cbIndex << 22);
        unsigned comp2 = (crIndex << 22);
        
        int searchIndex = tableSearch(comp0, comp1, comp2);
        int modelIndex  = m_searchTable[searchIndex].m_index;
        
        int yprime = calcYprime(yLinearIndex, cbIndex, crIndex, &m_coeffTable[modelIndex * m_modelCoeffs]) - (int) m_lumaOffset;
        
        out->m_floatComp[0][i] = (float)((double) yprime / m_lumaWeight);
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
