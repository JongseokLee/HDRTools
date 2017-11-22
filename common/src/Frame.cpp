/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Apple Inc.
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
 * \file Frame.cpp
 *
 * \brief
 *    Frame Class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Frame.H"
#include "Global.H"

#include <string.h> 
#include <math.h>
//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

Frame::Frame(int width, int height,  bool isFloat, ColorSpace colorSpace, ColorPrimaries colorPrimaries, ChromaFormat chromaFormat, SampleRange sampleRange, int bitDepth, bool isInterlaced, TransferFunctions transferFunction, float systemGamma)
{
  int c;
  
  m_isFloat          = isFloat;
  m_height[Y_COMP]   = height;
  m_width [Y_COMP]   = width;
  m_colorSpace       = colorSpace;
  m_chromaFormat     = chromaFormat;
  m_colorPrimaries   = colorPrimaries;
  m_sampleRange      = sampleRange;
  m_hasAlternate     = FALSE;
  m_altFrame         = NULL;
  m_altFrameNorm     = 1.0;
  
  switch (chromaFormat){
    case CF_400:
      m_width [U_COMP] = m_width [V_COMP] = 0;
      m_height[U_COMP] = m_height[V_COMP] = 0;
      m_noComponents = 1;
      break;
    case CF_420:
      m_width [U_COMP] = m_width [V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = m_height[Y_COMP] >> ONE;
      m_noComponents = 3;
      break;
    case CF_422:
      m_width [U_COMP] = m_width [V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = m_height[Y_COMP];
      m_noComponents = 3;
      break;
    case CF_444:
      m_width [U_COMP] = m_width [V_COMP] = m_width [Y_COMP];
      m_height[U_COMP] = m_height[V_COMP] = m_height[Y_COMP];
      m_noComponents = 3;
      break;
    default:
      fprintf(stderr, "\n Unsupported Chroma Subsampling Format %d\n", chromaFormat);
      exit(EXIT_FAILURE);
  }

  for (c = ZERO; c < 3; c++) {
    m_compSize[c] = m_width[c] * m_height[c];
  }

  m_size =  m_compSize[ZERO] + m_compSize[ONE] + m_compSize[TWO];

  // Set current pixel limits.
    m_bitDepth = bitDepth;
    for (c = ZERO; c < 3; c++) {
      m_bitDepthComp[c] = bitDepth;
      m_minPelValue[c] = 0;
      m_midPelValue[c] = (1 << (bitDepth - 1));
      m_maxPelValue[c] = (1 << bitDepth) - 1;
    }
 
  if (m_isFloat) {
    m_floatData.resize((unsigned int) m_size);
    if (m_floatData.size() != (unsigned int) m_size) {
      fprintf(stderr, "Frame.cpp: Frame(...) Not enough memory to create array m_floatData, of size %d", (int) m_size);
      exit(-1);
    }
    
    m_floatComp[Y_COMP] = &m_floatData[0];
    m_floatComp[U_COMP] = m_floatComp[Y_COMP] + m_compSize[Y_COMP];
    m_floatComp[V_COMP] = m_floatComp[U_COMP] + m_compSize[U_COMP];
    
    // Set ui8 pointers
    m_comp[Y_COMP] = NULL;
    m_comp[U_COMP] = NULL;
    m_comp[V_COMP] = NULL;

    // Set ui16 pointers
    m_ui16Comp[Y_COMP] = NULL;
    m_ui16Comp[U_COMP] = NULL;
    m_ui16Comp[V_COMP] = NULL;

  }
  else {
    if (m_bitDepth == 8) {
      m_data.resize((unsigned int) m_size);
      if (m_data.size() != (unsigned int) m_size) {
        fprintf(stderr, "Frame.cpp: Frame(...) Not enough memory to create array m_data, of size %d", (int) m_size);
        exit(-1);
      }
      
      m_comp[Y_COMP] = &m_data[0];
      m_comp[U_COMP] = m_comp[Y_COMP] + m_compSize[Y_COMP];
      m_comp[V_COMP] = m_comp[U_COMP] + m_compSize[U_COMP];
      
      // Set ui16 pointers
      m_ui16Comp[Y_COMP] = NULL;
      m_ui16Comp[U_COMP] = NULL;
      m_ui16Comp[V_COMP] = NULL;
      
      // Set float pointers
      m_floatComp[Y_COMP] = NULL;
      m_floatComp[U_COMP] = NULL;
      m_floatComp[V_COMP] = NULL;
    }
    else {
      m_ui16Data.resize((unsigned int) m_size);
      if (m_ui16Data.size() != (unsigned int) m_size) {
        fprintf(stderr, "Frame.cpp: Frame(...) Not enough memory to create array m_ui16Data, of size %d", (int) m_size);
        exit(-1);
      }
      
      m_ui16Comp[Y_COMP] = &m_ui16Data[0];
      m_ui16Comp[U_COMP] = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
      m_ui16Comp[V_COMP] = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
      
      // Set ui8 pointers
      m_comp[Y_COMP] = NULL;
      m_comp[U_COMP] = NULL;
      m_comp[V_COMP] = NULL;
      
      // Set float pointers
      m_floatComp[Y_COMP] = NULL;
      m_floatComp[U_COMP] = NULL;
      m_floatComp[V_COMP] = NULL;
    }
  }

  // setup format parameters
  m_format.m_isFloat        = m_isFloat;              //!< Floating point data
  m_format.m_isInterlaced   = isInterlaced;

  m_format.m_colorSpace       = m_colorSpace;           //!< ColorSpace (0: YUV, 1: RGB, 2: XYZ)
  m_format.m_chromaFormat     = m_chromaFormat;         //!< YUV format (0=4:0:0, 1=4:2:0, 2=4:2:2, 3=4:4:4)
  m_format.m_colorPrimaries   = m_colorPrimaries;       //!< Color primaries (0: BT.709, 1: BT.2020, 2:P3D60)
  m_format.m_sampleRange      = m_sampleRange;          //!< Sample range
  m_format.m_transferFunction = transferFunction;       //!< Transfer function
  m_format.m_systemGamma      = systemGamma;
  m_format.m_size             = m_size;                 //!< total image size (sum of m_compSize)
  for (int i = 0; i < 4; i++) {
    m_format.m_width[i]        = m_width[i];          //!< component frame width
    m_format.m_height[i]       = m_height[i];         //!< component frame height
    m_format.m_compSize[i]     = m_compSize[i];       //!< component sizes (width * height)
    m_format.m_bitDepthComp[i] = m_bitDepthComp[i];   //!< component bit depth
  }

  m_frameNo = ZERO;
  m_isAvailable = FALSE;
}

Frame::~Frame()
{
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  
  m_comp[Y_COMP] = NULL;
  m_comp[U_COMP] = NULL;
  m_comp[V_COMP] = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  
  // Here we do not delete the alternate frame. What we have is just a pointer.
  // It is very important that the alternate frame is cleared up at the appropriate
  // location.
  m_altFrame = NULL;
}

//-----------------------------------------------------------------------------
// Methods
//-----------------------------------------------------------------------------
void Frame::clear()
{
  if (m_isFloat) {
    // For floating point data, lets just set everything to 0 for now.
    memset(&m_floatData[0], ZERO, (int)m_size * sizeof(float));
  }
  else {
    // If RGB or XYZ, set all components to 0. Otherwise, we set to 0 luma info, and midvalue for chroma.
    if (m_bitDepth == 8) {
      if ( m_colorSpace == CM_RGB || m_colorSpace == CM_XYZ ) {
        memset(&m_data[0], ZERO, (int)m_size * sizeof(imgpel));
      }
      else{
        memset(m_comp[Y_COMP], ZERO, m_compSize[ZERO] * sizeof(imgpel));
        
        if (sizeof(imgpel) == 1){
          memset(m_comp[U_COMP], m_midPelValue[U_COMP], m_compSize[U_COMP] * sizeof(imgpel));
          memset(m_comp[V_COMP], m_midPelValue[V_COMP], m_compSize[V_COMP] * sizeof(imgpel));
        }
        else{
          int i, j;
          for (j = ONE; j < THREE; j++) {
            for (i = ZERO; i < m_compSize[j]; i++) {
              m_comp[j][i] = (imgpel) m_midPelValue[j];
            }
          }
        }
      } // if (m_colorSpace == CM_RGB || m_colorSpace == CM_XYZ)
    }
    else {
      if ( m_colorSpace == CM_RGB || m_colorSpace == CM_XYZ ) {
        memset(&m_ui16Data[0], ZERO, (int)m_size * sizeof(uint16));
      }
      else{
        int i, j;
        memset(m_ui16Comp[Y_COMP], ZERO, m_compSize[Y_COMP] * sizeof(uint16));
        
        for (j = ONE; j < THREE; j++) {
          for (i = ZERO; i < m_compSize[j]; i++) {
            m_ui16Comp[j][i] = (uint16) m_midPelValue[j];
          }
        }
      } // if (m_colorSpace == CM_RGB || m_colorSpace == CM_XYZ )
    }
  }
}

bool Frame::equalType(Frame *f)
{
  if (m_isFloat != f->m_isFloat)
    return FALSE;
  if (m_colorSpace != f->m_colorSpace)
    return FALSE;
  if (m_chromaFormat != f->m_chromaFormat)
    return FALSE;
  if (m_colorPrimaries != f->m_colorPrimaries)
    return FALSE;
  if (m_height[Y_COMP] != f->m_height[Y_COMP])
    return FALSE;
  if (m_width[Y_COMP] != f->m_width[Y_COMP])
    return FALSE;
  if (m_bitDepth != f->m_bitDepth)
     return FALSE;

  return TRUE;
}


bool Frame::equalSize(Frame *f)
{
  if (m_height[Y_COMP] != f->m_height[Y_COMP])
    return FALSE;
  if (m_width[Y_COMP] != f->m_width[Y_COMP])
    return FALSE;  
    
  return TRUE;
}

void Frame::copyArea(const float *iComp, float *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int iHeight, int iWidth, int oMinY, int oMinX, int oHeight, int oWidth)
{
  int destPosY = oMinY;
  int destPosX = oMinX;
  
  int posY, posX;
  
  for (posY = iMinY; posY <= iMaxY; posY++, destPosY++) {
    destPosX = oMinX;
    for (posX = iMinX; posX <= iMaxX; posX++, destPosX++) {
      if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
        if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
          oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
        }
        else if (posX >= iWidth || posY >= iHeight) {
          break;
        }
      }
      else if (destPosX >= oWidth || destPosY >= oHeight){
        break;
      }
    }
    if (destPosY >= m_height[Y_COMP] || posY >= iHeight) {
      break;
    }
  }
}

void Frame::copyArea(const uint16 *iComp, uint16 *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int iHeight, int iWidth, int oMinY, int oMinX, int oHeight, int oWidth)
{
  int destPosY = oMinY;
  int destPosX = oMinX;
  
  int posY, posX;
  
  for (posY = iMinY; posY <= iMaxY; posY++, destPosY++) {
    destPosX = oMinX;
    for (posX = iMinX; posX <= iMaxX; posX++, destPosX++) {
      if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
        if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
          oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
        }
        else if (posX >= iWidth || posY >= iHeight) {
          break;
        }
      }
      else if (destPosX >= oWidth || destPosY >= oHeight){
        break;
      }
    }
    if (destPosY >= m_height[Y_COMP] || posY >= iHeight) {
      break;
    }
  }
}

void Frame::copyArea(const imgpel *iComp, imgpel *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int iHeight, int iWidth, int oMinY, int oMinX, int oHeight, int oWidth)
{
  int destPosY = oMinY;
  int destPosX = oMinX;
  
  int posY, posX;
  
  for (posY = iMinY; posY <= iMaxY; posY++, destPosY++) {
    destPosX = oMinX;
    for (posX = iMinX; posX <= iMaxX; posX++, destPosX++) {
      if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
        if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
          oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
        }
        else if (posX >= iWidth || posY >= iHeight) {
          break;
        }
      }
      else if (destPosX >= oWidth || destPosY >= oHeight){
        break;
      }
    }
    if (destPosY >= m_height[Y_COMP] || posY >= iHeight) {
      break;
    }
  }
}

void Frame::copyArea(const float *iComp, float *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int iHeight, int iWidth, int oMinY, int oMinX, int oHeight, int oWidth, bool flip)
{
  int destPosY = oMinY;
  int destPosX = oMinX;
  
  int posY, posX;
  
  for (posY = iMinY; posY <= iMaxY; posY++, destPosY++) {
    if (flip == false) {
      destPosX = oMinX;
      for (posX = iMinX; posX <= iMaxX; posX++, destPosX++) {
        if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
          if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
            oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
          }
          else if (posX >= iWidth || posY >= iHeight) {
            break;
          }
        }
        else if (destPosX >= oWidth || destPosY >= oHeight){
          break;
        }
      }
    }
    else {
      destPosX = oMinX + (iMaxX - iMinX);
      for (posX = iMinX; posX <= iMaxX; posX++, destPosX--) {
        if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
          if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
            oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
          }
          else if (posX >= iWidth || posY >= iHeight) {
            break;
          }
        }
        else if (destPosX >= oWidth || destPosY >= oHeight){
          break;
        }
      }
    }
    if (destPosY >= m_height[Y_COMP] || posY >= iHeight) {
      break;
    }
  }
}

void Frame::copyArea(const uint16 *iComp, uint16 *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int iHeight, int iWidth, int oMinY, int oMinX, int oHeight, int oWidth, bool flip)
{
  int destPosY = oMinY;
  int destPosX = oMinX;
  
  int posY, posX;
  
  for (posY = iMinY; posY <= iMaxY; posY++, destPosY++) {
    if (flip == false) {
      destPosX = oMinX;
      for (posX = iMinX; posX <= iMaxX; posX++, destPosX++) {
        if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
          if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
            oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
          }
          else if (posX >= iWidth || posY >= iHeight) {
            break;
          }
        }
        else if (destPosX >= oWidth || destPosY >= oHeight){
          break;
        }
      }
    }
    else {
      destPosX = oMinX + (iMaxX - iMinX);
      for (posX = iMinX; posX <= iMaxX; posX++, destPosX--) {
        if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
          if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
            oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
          }
          else if (posX >= iWidth || posY >= iHeight) {
            break;
          }
        }
        else if (destPosX >= oWidth || destPosY >= oHeight){
          break;
        }
      }
    }
    if (destPosY >= m_height[Y_COMP] || posY >= iHeight) {
      break;
    }
  }
}

void Frame::copyArea(const imgpel *iComp, imgpel *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int iHeight, int iWidth, int oMinY, int oMinX, int oHeight, int oWidth, bool flip)
{
  int destPosY = oMinY;
  int destPosX = oMinX;
  
  int posY, posX;
  
  for (posY = iMinY; posY <= iMaxY; posY++, destPosY++) {
    if (flip == false) {
      destPosX = oMinX;
      for (posX = iMinX; posX <= iMaxX; posX++, destPosX++) {
        if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
          if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
            oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
          }
          else if (posX >= iWidth || posY >= iHeight) {
            break;
          }
        }
        else if (destPosX >= oWidth || destPosY >= oHeight){
          break;
        }
      }
    }
    else {
      destPosX = oMinX + (iMaxX - iMinX);
      for (posX = iMinX; posX <= iMaxX; posX++, destPosX--) {
        if (destPosX >= 0 && destPosX < oWidth && destPosY >= 0 && destPosY < oHeight) {
          if (posX >= 0 && posX < iWidth && posY >= 0 && posY < iHeight) {
            oComp[destPosY * oWidth + destPosX] = iComp[posY * iWidth + posX];
          }
          else if (posX >= iWidth || posY >= iHeight) {
            break;
          }
        }
        else if (destPosX >= oWidth || destPosY >= oHeight){
          break;
        }
      }
      
    }
    if (destPosY >= m_height[Y_COMP] || posY >= iHeight) {
      break;
    }
  }
}

void Frame::copy(Frame *f)
{
  // the copy function assumes that all other format parameters are the same except the
  // image data. 
  if (m_isFloat != f->m_isFloat) {
    fprintf(stderr, "Warning: trying to copy frames of different type. \n");
    return;
  }
  if (m_size != f->m_size) {
    fprintf(stderr, "Warning: trying to copy frames of different sizes (%" FORMAT_OFF_T " %" FORMAT_OFF_T "). \n",m_size,f->m_size);
    return;
  }
  if (m_isFloat == FALSE && m_bitDepth != f->m_bitDepth) {
    fprintf(stderr, "Warning: trying to copy frames of different bitdepth (%d %d). \n",m_bitDepth,f->m_bitDepth);
    return;
  }
  if (m_chromaFormat != f->m_chromaFormat) {
      fprintf(stderr, "Warning: trying to copy frames of different chroma format (%d %d). \n",m_chromaFormat,f->m_chromaFormat);
      return;
  }

  for (int c = ZERO; c < 3; c++) {
    m_minPelValue[c]  = f->m_minPelValue[c];
    m_midPelValue[c]  = f->m_midPelValue[c];
    m_maxPelValue[c]  = f->m_maxPelValue[c];
  }
  
  m_frameNo      = f->m_frameNo;
  m_isAvailable  = TRUE;

  if (m_isFloat) {
    m_floatData = f->m_floatData;
  }
  else if (m_bitDepth == 8) { 
    m_data = f->m_data;
  }
  else {
    m_ui16Data = f->m_ui16Data;
  }
}

void Frame::copy(Frame *f, int component)
{
  // the copy function assumes that all other format parameters are the same except the
  // image data.
  if (m_isFloat != f->m_isFloat) {
    fprintf(stderr, "Warning: trying to copy frames of different type. \n");
    return;
  }
  if (m_compSize[component] != f->m_compSize[component]) {
    fprintf(stderr, "Warning: trying to copy frames of different sizes (%d %d). \n",m_compSize[component],f->m_compSize[component]);
    return;
  }
  if (m_isFloat == FALSE && m_bitDepthComp[component] != f->m_bitDepthComp[component]) {
    fprintf(stderr, "Warning: trying to copy frames of different bitdepth (%d %d). \n",m_bitDepthComp[component],f->m_bitDepthComp[component]);
    return;
  }
  
  //if (m_chromaFormat != f->m_chromaFormat) {
  //    fprintf(stderr, "Warning: trying to copy frames of different chroma format (%d %d). \n",m_chromaFormat,f->m_chromaFormat);
  //    return;
  //}

  m_minPelValue[component]  = f->m_minPelValue[component];
  m_midPelValue[component]  = f->m_midPelValue[component];
  m_maxPelValue[component]  = f->m_maxPelValue[component];
  
  if (m_isFloat) {
    memcpy(m_floatComp[component], f->m_floatComp[component], (int) m_compSize[component] * sizeof(float));
  }
  else if (m_bitDepth == 8) {
    memcpy(m_comp[component], f->m_comp[component], (int) m_compSize[component] * sizeof(imgpel));
  }
  else {
    memcpy(m_ui16Comp[component], f->m_ui16Comp[component], (int) m_compSize[component] * sizeof(uint16));
  }
}

void Frame::copy(Frame *f, int sourceMinX, int sourceMinY, int sourceMaxX, int sourceMaxY, int targetX, int targetY)
{
  // the copy function assumes that all other format parameters are the same except the
  // image data.
  if (m_isFloat != f->m_isFloat) {
    fprintf(stderr, "Warning: trying to copy frames of different type. \n");
    return;
  }
  if (m_isFloat == FALSE && m_bitDepth != f->m_bitDepth) {
    fprintf(stderr, "Warning: trying to copy frames of different bitdepth (%d %d). \n",m_bitDepth,f->m_bitDepth);
    return;
  }
  if (m_chromaFormat != f->m_chromaFormat) {
      fprintf(stderr, "Warning: trying to copy frames of different chroma format (%d %d). \n",m_chromaFormat,f->m_chromaFormat);
      return;
  }
  
  for (int c = ZERO; c < 3; c++) {
    m_minPelValue[c]  = f->m_minPelValue[c];
    m_midPelValue[c]  = f->m_midPelValue[c];
    m_maxPelValue[c]  = f->m_maxPelValue[c];
  }
  
  m_frameNo      = f->m_frameNo;
  m_isAvailable  = TRUE;

  int origMinX = iMin(sourceMinX, sourceMaxX);
  int origMaxX = iMax(sourceMinX, sourceMaxX);
  int origMinY = iMin(sourceMinY, sourceMaxY);
  int origMaxY = iMax(sourceMinY, sourceMaxY);

  if (m_isFloat) {
    copyArea(f->m_floatComp[Y_COMP], m_floatComp[Y_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[Y_COMP], f->m_width[Y_COMP], targetY, targetX, m_height[Y_COMP], m_width[Y_COMP]);
    if (m_chromaFormat == CF_444) {
      copyArea(f->m_floatComp[U_COMP], m_floatComp[U_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY, targetX, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_floatComp[V_COMP], m_floatComp[V_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY, targetX, m_height[V_COMP], m_width[V_COMP]);
    }
    else if (m_chromaFormat == CF_422) {
      copyArea(f->m_floatComp[U_COMP], m_floatComp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_floatComp[V_COMP], m_floatComp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX, m_height[V_COMP], m_width[V_COMP]);
    }
    else if (m_chromaFormat == CF_420) {
      copyArea(f->m_floatComp[U_COMP], m_floatComp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX >> 1, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_floatComp[V_COMP], m_floatComp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX >> 1, m_height[V_COMP], m_width[V_COMP]);
    }
  }
  else if (m_bitDepth == 8) {
    copyArea(f->m_comp[Y_COMP], m_comp[Y_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[Y_COMP], f->m_width[Y_COMP], targetY, targetX, m_height[Y_COMP], m_width[Y_COMP]);
    if (m_chromaFormat == CF_444) {
      copyArea(f->m_comp[U_COMP], m_comp[U_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY, targetX, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_comp[V_COMP], m_comp[V_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY, targetX, m_height[V_COMP], m_width[V_COMP]);
    }
    else if (m_chromaFormat == CF_422) {
      copyArea(f->m_comp[U_COMP], m_comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_comp[V_COMP], m_comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX, m_height[V_COMP], m_width[V_COMP]);
    }
    else if (m_chromaFormat == CF_420) {
      copyArea(f->m_comp[U_COMP], m_comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX >> 1, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_comp[V_COMP], m_comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX >> 1, m_height[V_COMP], m_width[V_COMP]);
    }
  }
  else {
    copyArea(f->m_ui16Comp[Y_COMP], m_ui16Comp[Y_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[Y_COMP], f->m_width[Y_COMP], targetY, targetX, m_height[Y_COMP], m_width[Y_COMP]);
    if (m_chromaFormat == CF_444) {
      copyArea(f->m_ui16Comp[U_COMP], m_ui16Comp[U_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY, targetX, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_ui16Comp[V_COMP], m_ui16Comp[V_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY, targetX, m_height[V_COMP], m_width[V_COMP]);
    }
    else if (m_chromaFormat == CF_422) {
      copyArea(f->m_ui16Comp[U_COMP], m_ui16Comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_ui16Comp[V_COMP], m_ui16Comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX, m_height[V_COMP], m_width[V_COMP]);
    }
    else if (m_chromaFormat == CF_420) {
      copyArea(f->m_ui16Comp[U_COMP], m_ui16Comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX >> 1, m_height[U_COMP], m_width[U_COMP]);
      copyArea(f->m_ui16Comp[V_COMP], m_ui16Comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX >> 1, m_height[V_COMP], m_width[V_COMP]);
    }
  }
}

void Frame::copy(Frame *f, int sourceMinX, int sourceMinY, int sourceMaxX, int sourceMaxY, int targetX, int targetY, bool flip)
{
  // the copy function assumes that all other format parameters are the same except the
  // image data.
  if (m_isFloat != f->m_isFloat) {
    fprintf(stderr, "Warning: trying to copy frames of different type. \n");
    return;
  }
  if (m_isFloat == FALSE && m_bitDepth != f->m_bitDepth) {
    fprintf(stderr, "Warning: trying to copy frames of different bitdepth (%d %d). \n",m_bitDepth,f->m_bitDepth);
    return;
  }
  if (m_chromaFormat != f->m_chromaFormat) {
    fprintf(stderr, "Warning: trying to copy frames of different chroma format (%d %d). \n",m_chromaFormat,f->m_chromaFormat);
    return;
  }
  
  for (int c = ZERO; c < 3; c++) {
    m_minPelValue[c]  = f->m_minPelValue[c];
    m_midPelValue[c]  = f->m_midPelValue[c];
    m_maxPelValue[c]  = f->m_maxPelValue[c];
  }
  
  m_frameNo      = f->m_frameNo;
  m_isAvailable  = TRUE;
  
  int origMinX = iMin(sourceMinX, sourceMaxX);
  int origMaxX = iMax(sourceMinX, sourceMaxX);
  int origMinY = iMin(sourceMinY, sourceMaxY);
  int origMaxY = iMax(sourceMinY, sourceMaxY);
  
  if (m_isFloat) {
    copyArea(f->m_floatComp[Y_COMP], m_floatComp[Y_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[Y_COMP], f->m_width[Y_COMP], targetY, targetX, m_height[Y_COMP], m_width[Y_COMP], flip);
    if (m_chromaFormat == CF_444) {
      copyArea(f->m_floatComp[U_COMP], m_floatComp[U_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY, targetX, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_floatComp[V_COMP], m_floatComp[V_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY, targetX, m_height[V_COMP], m_width[V_COMP], flip);
    }
    else if (m_chromaFormat == CF_422) {
      copyArea(f->m_floatComp[U_COMP], m_floatComp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_floatComp[V_COMP], m_floatComp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX, m_height[V_COMP], m_width[V_COMP], flip);
    }
    else if (m_chromaFormat == CF_420) {
      copyArea(f->m_floatComp[U_COMP], m_floatComp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX >> 1, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_floatComp[V_COMP], m_floatComp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX >> 1, m_height[V_COMP], m_width[V_COMP], flip);
    }
  }
  else if (m_bitDepth == 8) {
    copyArea(f->m_comp[Y_COMP], m_comp[Y_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[Y_COMP], f->m_width[Y_COMP], targetY, targetX, m_height[Y_COMP], m_width[Y_COMP], flip);
    if (m_chromaFormat == CF_444) {
      copyArea(f->m_comp[U_COMP], m_comp[U_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY, targetX, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_comp[V_COMP], m_comp[V_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY, targetX, m_height[V_COMP], m_width[V_COMP], flip);
    }
    else if (m_chromaFormat == CF_422) {
      copyArea(f->m_comp[U_COMP], m_comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_comp[V_COMP], m_comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX, m_height[V_COMP], m_width[V_COMP], flip);
    }
    else if (m_chromaFormat == CF_420) {
      copyArea(f->m_comp[U_COMP], m_comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX >> 1, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_comp[V_COMP], m_comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX >> 1, m_height[V_COMP], m_width[V_COMP], flip);
    }
  }
  else {
    copyArea(f->m_ui16Comp[Y_COMP], m_ui16Comp[Y_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[Y_COMP], f->m_width[Y_COMP], targetY, targetX, m_height[Y_COMP], m_width[Y_COMP], flip);
    if (m_chromaFormat == CF_444) {
      copyArea(f->m_ui16Comp[U_COMP], m_ui16Comp[U_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY, targetX, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_ui16Comp[V_COMP], m_ui16Comp[V_COMP], origMinY, origMaxY, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY, targetX, m_height[V_COMP], m_width[V_COMP], flip);
    }
    else if (m_chromaFormat == CF_422) {
      copyArea(f->m_ui16Comp[U_COMP], m_ui16Comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_ui16Comp[V_COMP], m_ui16Comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX, origMaxX, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX, m_height[V_COMP], m_width[V_COMP], flip);
    }
    else if (m_chromaFormat == CF_420) {
      copyArea(f->m_ui16Comp[U_COMP], m_ui16Comp[U_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[U_COMP], f->m_width[U_COMP], targetY >> 1, targetX >> 1, m_height[U_COMP], m_width[U_COMP], flip);
      copyArea(f->m_ui16Comp[V_COMP], m_ui16Comp[V_COMP], origMinY >> 1, origMaxY >> 1, origMinX >> 1, origMaxX >> 1, f->m_height[V_COMP], f->m_width[V_COMP], targetY >> 1, targetX >> 1, m_height[V_COMP], m_width[V_COMP], flip);
    }
  }
}


void Frame::copy(Frame *f, int component, int sourceMinX, int sourceMinY, int sourceMaxX, int sourceMaxY, int targetX, int targetY)
{
  // the copy function assumes that all other format parameters are the same except the
  // image data.
  if (m_isFloat != f->m_isFloat) {
    fprintf(stderr, "Warning: trying to copy frames of different type. \n");
    return;
  }
  if (m_bitDepthComp[component] != f->m_bitDepthComp[component]) {
    fprintf(stderr, "Warning: trying to copy frames of different bitdepth (%d %d). \n",m_bitDepthComp[component],f->m_bitDepthComp[component]);
    return;
  }
  if (m_chromaFormat != f->m_chromaFormat) {
    fprintf(stderr, "Warning: trying to copy frames of different chroma format (%d %d). \n",m_chromaFormat,f->m_chromaFormat);
    return;
  }
  
  m_minPelValue[component]  = f->m_minPelValue[component];
  
  int origMinX = iMin(sourceMinX, sourceMaxX);
  int origMaxX = iMax(sourceMinX, sourceMaxX);
  int origMinY = iMin(sourceMinY, sourceMaxY);
  int origMaxY = iMax(sourceMinY, sourceMaxY);
  
  if (component == U_COMP || component == V_COMP) {
    if (m_chromaFormat == CF_422 || m_chromaFormat == CF_420) {
      origMinY >>= 1;
      origMaxY >>= 1;
      targetY  >>= 1;
    }
    if (m_chromaFormat == CF_420) {
      origMinX >>= 1;
      origMaxX >>= 1;
      targetX  >>= 1;
    }
    if (m_chromaFormat == CF_400) {
      fprintf(stderr, "Warning: trying to copy chroma samples of a 4:0:0 image. \n");
      return;
    }
  }
  
  if (m_isFloat) {
    copyArea(f->m_floatComp[component], m_floatComp[component], origMinY, origMaxY, origMinX, origMaxX, f->m_height[component], f->m_width[component], targetY, targetX, m_height[component], m_width[component]);
  }
  else if (m_bitDepth == 8) {
    copyArea(f->m_comp[component], m_comp[component], origMinY, origMaxY, origMinX, origMaxX, f->m_height[component], f->m_width[component], targetY, targetX, m_height[component], m_width[component]);
  }
  else {
    copyArea(f->m_ui16Comp[component], m_ui16Comp[component], origMinY, origMaxY, origMinX, origMaxX, f->m_height[component], f->m_width[component], targetY, targetX, m_height[component], m_width[component]);
  }
}


template <typename ValueType>  void Frame::clipComponent(ValueType *oComp, int compSize, int minPelValue, int maxPelValue) {
  for (int i = 0; i < compSize; i++) {
    *oComp = (ValueType) iClip((int) *oComp, minPelValue, maxPelValue);
    oComp++;
  }
}


// Enforce clipping of the data according to the specified sample range
// Original code from jzhao@sharplabs.com. Modified and Extended a bit.
void Frame::clipRange() {
  const int rangeMin[3][3] = {
    { 16, 16, 16 },
    { 16, 16, 16 },
    {  4,  4,  4 }
  };
  const int rangeMax[3][3] = {
    {  235,  235,  235 },
    {  235,  240,  240 },
    { 1019, 1019, 1019 }
  };
  
  if (m_isFloat == TRUE || m_sampleRange != SR_STANDARD)
    return;
  
  if (m_bitDepth == 8) {
  
    if (m_sampleRange != SR_STANDARD)
      return;
    
    int compIdx = (m_colorSpace == CM_RGB || m_colorSpace == CM_XYZ) ? 0 : 1;
    for (int comp = 0; comp < m_noComponents; comp++)
      clipComponent(m_comp[comp], m_compSize[comp], rangeMin[compIdx][comp], rangeMax[compIdx][comp]);
  }
  else {
    if (m_sampleRange != SR_STANDARD) {
      if (m_bitDepth < 10 || m_colorSpace != CM_RGB)
        return;
      for (int comp = 0; comp < m_noComponents; comp++)
        clipComponent(m_ui16Comp[comp], m_compSize[comp], rangeMin[2][comp] << (m_bitDepthComp[comp] - 10), rangeMax[2][comp] << (m_bitDepthComp[comp] - 10));
    }
    else {
      int compIdx = (m_colorSpace == CM_RGB || m_colorSpace == CM_XYZ) ? 0 : 1;
      for (int comp = 0; comp < m_noComponents; comp++)
        clipComponent(m_ui16Comp[comp], m_compSize[comp], rangeMin[compIdx][comp] << (m_bitDepthComp[comp] - 8), rangeMax[compIdx][comp] << (m_bitDepthComp[comp] - 8));
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
