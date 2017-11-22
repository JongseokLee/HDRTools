/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = ITU/ISO
 * <ORGANIZATION> = Apple Inc
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, Apple Inc
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
 * \file ColorTransformRGBAdjust.cpp
 *
 * \brief
 *    ColorTransformRGBAdjust Class
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
#include "ColorTransformRGBAdjust.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformRGBAdjust::ColorTransformRGBAdjust( ColorTransformParams *params ) {
  
  m_mode = CTF_IDENTITY; 
  
  m_size = 0;
  m_tfDistance = TRUE;
  
  for (int index = 0; index < 4; index++) {
    m_floatComp[index] = NULL;
    
    m_compSize[index] = 0;       // number of samples in each color component
    m_height[index] = 0;         // height of each color component
    m_width[index] = 0;          // width of each color component
  }
  m_memoryAllocated = FALSE;
  
  // Method is only allowed for RGB to YCbCr conversion
  if (m_iColorSpace == CM_RGB && m_oColorSpace == CM_YCbCr) {
    if (m_iColorPrimaries == CP_709 && m_oColorPrimaries == CP_709) {
      m_mode = CTF_RGB709_2_YUV709;
    }
    else if (m_iColorPrimaries == CP_601 && m_oColorPrimaries == CP_601) {
      m_mode = CTF_RGB601_2_YUV601;
    }    
    else if (m_iColorPrimaries == CP_2020 && m_oColorPrimaries == CP_2020) {
      m_mode = CTF_RGB2020_2_YUV2020;
    }
    else if (m_iColorPrimaries == CP_P3D65 && m_oColorPrimaries == CP_P3D65) {
      m_mode = CTF_RGBP3D65_2_YUVP3D65;
    }
    else if (m_iColorPrimaries == CP_P3D60 && m_oColorPrimaries == CP_P3D60) {
      m_mode = CTF_RGBP3D60_2_YUVP3D60;
    }
    else if (m_iColorPrimaries == CP_EXT && m_oColorPrimaries == CP_EXT) {
      m_mode = CTF_RGBEXT_2_YUVEXT;
    }
    else if (m_oColorPrimaries == CP_AMT) {
      m_mode = CTF_RGB_2_AMT;
    }
    else if ( m_oColorPrimaries == CP_YCOCG) {
      m_mode = CTF_RGB_2_YCOCG;
    }
  }
  else {
    m_mode = CTF_IDENTITY;
  }

  // Forward Transform coefficients 
  m_transform0 = FWD_TRANSFORM[m_mode][Y_COMP];
  m_transform1 = FWD_TRANSFORM[m_mode][U_COMP];
  m_transform2 = FWD_TRANSFORM[m_mode][V_COMP];

  // Inverse Transform coefficients 
  m_invTransform0 = INV_TRANSFORM[m_mode][Y_COMP];
  m_invTransform1 = INV_TRANSFORM[m_mode][U_COMP];
  m_invTransform2 = INV_TRANSFORM[m_mode][V_COMP];

  m_fwdColorFormat = NULL;
  m_invColorFormat = NULL;
  m_fwdFrameStore  = NULL;
  m_invFrameStore  = NULL;

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

  m_transferFunction = TransferFunction::create(m_transferFunctions, TRUE, 1.0, params->m_oSystemGamma, params->m_minValue, params->m_maxValue, params->m_enableLUTs);
}

ColorTransformRGBAdjust::~ColorTransformRGBAdjust() {
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
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ColorTransformRGBAdjust::allocateMemory(Frame* out, const Frame *inp) {
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
    fprintf(stderr, "ColorTransformRGBAdjust: Not enough memory to create array m_floatData, of size %d", (int) m_size);
    exit(-1);
  }
  
  m_floatComp[Y_COMP] = &m_floatData[0];
  m_floatComp[U_COMP] = m_floatComp[Y_COMP] + m_compSize[Y_COMP];
  m_floatComp[V_COMP] = m_floatComp[U_COMP] + m_compSize[U_COMP];
  
  //printf("format %d %d\n", inp->m_chromaFormat, out->m_chromaFormat);
  m_fwdColorFormat = ConvertColorFormat::create(m_width[Y_COMP], m_height[Y_COMP], inp->m_chromaFormat, m_oChromaFormat, m_downMethod, (ChromaLocation *) &(inp->m_format.m_chromaLocation[0]), (ChromaLocation *)  &(out->m_format.m_chromaLocation[0]), FALSE, m_useMinMax);
  
  m_invColorFormat = ConvertColorFormat::create(m_width[Y_COMP], m_height[Y_COMP], m_oChromaFormat, inp->m_chromaFormat, m_upMethod, (ChromaLocation *) &(out->m_format.m_chromaLocation[0]), (ChromaLocation *)  &(inp->m_format.m_chromaLocation[0]), FALSE);
  
  m_fwdFrameStore  = new Frame(m_width[Y_COMP], m_height[Y_COMP], TRUE, out->m_colorSpace, out->m_colorPrimaries, m_oChromaFormat, out->m_sampleRange, out->m_bitDepthComp[Y_COMP], FALSE, TF_PQ, 1.0);      
  m_invFrameStore  = new Frame(m_width[Y_COMP], m_height[Y_COMP], TRUE, inp->m_colorSpace, inp->m_colorPrimaries, inp->m_chromaFormat, inp->m_sampleRange, inp->m_bitDepthComp[Y_COMP], FALSE, TF_PQ, 1.0);      
  

  m_memoryAllocated = TRUE;
}

void ColorTransformRGBAdjust::convertToRGB(const double yComp, const double uComp, const double vComp, double *rComp, double *gComp, double *bComp) {
  *rComp = dClip((m_invTransform0[0] * yComp + m_invTransform0[1] * uComp + m_invTransform0[2] * vComp), 0.0, 1.0);
  *gComp = dClip((m_invTransform1[0] * yComp + m_invTransform1[1] * uComp + m_invTransform1[2] * vComp), 0.0, 1.0);
  *bComp = dClip((m_invTransform2[0] * yComp + m_invTransform2[1] * uComp + m_invTransform2[2] * vComp), 0.0, 1.0);
}

double ColorTransformRGBAdjust::convertToYLinear(const double rComp, const double gComp, const double bComp) {
  return (m_transform0[0] * m_transferFunction->forward(rComp) + m_transform0[1] * m_transferFunction->forward(gComp) + m_transform0[2] * m_transferFunction->forward(bComp));
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ColorTransformRGBAdjust::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  // Current condition to perform this is that Frames are of same size and in 4:4:4
  // Can add more code to do the interpolation on the fly (and save memory/improve speed),
  // but this keeps our code more flexible for now.
  
  if (inp->m_compSize[Y_COMP] == out->m_compSize[Y_COMP] && inp->m_compSize[Y_COMP] == inp->m_compSize[U_COMP])  {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE)  {
      double yLinear, yConv, yConvMin = 0.0, yConvMax = 0.0;
      double yComp, uComp, vComp;
      double rComp, gComp, bComp; 
      double rOrig, gOrig, bOrig;
      double rMin = 0.0, gMin = 0.0, bMin = 0.0;
      double rMax = 99999.999, gMax = 99999.999, bMax = 99999.999;
      // Allocate memory. Note that current code does not permit change of resolution. TBDL
      if (m_memoryAllocated == FALSE) { 
        allocateMemory(out, inp);
      }              
      
      // First convert all components as per the described transform process 
      for (int i = 0; i < inp->m_compSize[0]; i++) {
        out->m_floatComp[0][i] = (float) (m_transform0[0] * (double) inp->m_floatComp[0][i] + m_transform0[1] * (double) inp->m_floatComp[1][i] + m_transform0[2] * (double) inp->m_floatComp[2][i]);
        out->m_floatComp[1][i] = (float) (m_transform1[0] * (double) inp->m_floatComp[0][i] + m_transform1[1] * (double) inp->m_floatComp[1][i] + m_transform1[2] * inp->m_floatComp[2][i]);
        out->m_floatComp[2][i] = (float) (m_transform2[0] * (double) inp->m_floatComp[0][i] + m_transform2[1] * (double) inp->m_floatComp[1][i] + m_transform2[2] * (double) inp->m_floatComp[2][i]);
      }
      
      m_fwdColorFormat->process (m_fwdFrameStore, out);
      // Quantize/DeQuantize components
      // Luma
      for (int i = 0; i < m_fwdFrameStore->m_compSize[0]; i++) {
        m_fwdFrameStore->m_floatComp[0][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[0][i] * m_lumaWeight) / m_lumaWeight);
      }
      // Chroma
      for (int i = 0; i < m_fwdFrameStore->m_compSize[1]; i++) {
        m_fwdFrameStore->m_floatComp[1][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[1][i] * m_chromaWeight) / m_chromaWeight);
        m_fwdFrameStore->m_floatComp[2][i] = (float) (dRound((double) m_fwdFrameStore->m_floatComp[2][i] * m_chromaWeight) / m_chromaWeight);
      }
      
      // Now convert back to 4:4:4
      m_invColorFormat->process (m_invFrameStore, m_fwdFrameStore);

      for (int i = 0; i < inp->m_compSize[0]; i++) {
        // First compute the linear value of the target Y (given original data)
        rOrig = (double) inp->m_floatComp[0][i];
        gOrig = (double) inp->m_floatComp[1][i];
        bOrig = (double) inp->m_floatComp[2][i];
        yLinear = convertToYLinear(rOrig, gOrig, bOrig);
        
        yComp = (double) m_invFrameStore->m_floatComp[0][i];
        uComp = (double) m_invFrameStore->m_floatComp[1][i];
        vComp = (double) m_invFrameStore->m_floatComp[2][i];
        //yComp = 0.0;
        
        
        double yCompMin = 0.0; // minimum allowed value
        double yCompMax = 1.0; // maximum allowed value
        
        //convertToRGB(yComp, uComp, vComp, &rComp, &gComp, &bComp);
        
        //yConv = convertToYLinear(rComp, gComp, bComp);
        
        //printf("values %10.8f %10.8f %10.8f (%d)", yLinear, yComp, yConv, (int) dRound(yComp * m_lumaWeight));
        
        // Given reconstruction convert also inverse data
        for (int j = 0; j < m_maxIterations; j++) {
          convertToRGB(yComp, uComp, vComp, &rComp, &gComp, &bComp);
          
          yConv = convertToYLinear(rComp, gComp, bComp);
          //printf("%d values %10.8f %10.8f %10.8f %10.8f %10.8f\n", j, yLinear, yComp, yConv, yConvMin, yConvMax);
          
          if (yConv == yLinear) {
            yConvMin = yConv;
            yConvMax = yConv;
            yCompMin = yComp;
            yCompMax = yComp;
            rMax = rMin = rComp;
            gMax = gMin = gComp;
            bMax = bMin = bComp;
            break;
          }
          else if (yConv < yLinear) { 
            convertToRGB(yCompMax, uComp, vComp, &rMax, &gMax, &bMax);
            
            yConvMax = convertToYLinear(rMax, gMax, bMax);
            yCompMin = yComp;
            yConvMin = yConv;
            rMin = rComp;
            gMin = gComp;
            bMin = bComp;
            yComp = dRound(((yComp + yCompMax) / 2.0) * m_lumaWeight) / m_lumaWeight;
            //printf("increase %10.8f %10.8f %10.8f %10.8f (%10.8f %10.8f %10.8f)\n", yComp, yCompMax, yConvMin, yConvMax, rComp, gComp, bComp);
            
            if (yComp == yCompMax) {
              break;
            }
          }
          else {
            convertToRGB(yCompMin, uComp, vComp, &rMin, &gMin, &bMin);
            
            yConvMin = convertToYLinear(rMin, gMin, bMin);
            yCompMax = yComp;
            yConvMax = yConv;
            rMax = rComp;
            gMax = gComp;
            bMax = bComp;

            yComp = dRound(((yComp + yCompMin) / 2.0) * m_lumaWeight) / m_lumaWeight;
            //printf("decrease %10.8f %10.8f %10.8f %10.8f (%10.8f %10.8f %10.8f)\n", yComp, yCompMin, yConvMin, yConvMax, rComp, gComp, bComp);
            if (yComp == yCompMax) {
              break;
            }
          }
        }
        
        if (yConvMin == yConvMax)
          out->m_floatComp[0][i] = (float) yCompMin;
        else if(m_tfDistance == FALSE) {
          if (dAbs(yConvMin - yLinear) < dAbs(yConvMax - yLinear))
            out->m_floatComp[0][i] = (float) yCompMin;
          else
            out->m_floatComp[0][i] = (float) yCompMax;
        }
        else if(m_tfDistance == TRUE) {
          double yTFMin = m_transferFunction->inverse(yConvMin);
          double yTFMax = m_transferFunction->inverse(yConvMax);
          double yTF    = m_transferFunction->inverse(yLinear);
          if (dAbs(yTFMin - yTF) + dAbs(rMin - rOrig)  + dAbs(gMin - gOrig) + dAbs(bMin - bOrig) < dAbs(yTFMax - yTF)  + dAbs(rMax - rOrig)  + dAbs(gMax - gOrig) + dAbs(bMax - bOrig))
            out->m_floatComp[0][i] = (float) yCompMin;
          else
            out->m_floatComp[0][i] = (float) yCompMax;
        }
        
        //printf(" end %10.8f (%10.8f) %10.8f %10.8f (%d) \n", out->m_floatComp[0][i] , - (m_invFrameStore->m_floatComp[0][i] - out->m_floatComp[0][i]) * m_lumaWeight, yConvMin, yConvMax, (int) dRound(out->m_floatComp[0][i] * m_lumaWeight));
        
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
