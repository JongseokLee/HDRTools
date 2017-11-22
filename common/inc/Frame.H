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
 * \file Frame.H
 *
 * \brief
 *    Frame class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

#ifndef __Frame_H__
#define __Frame_H__

#include "Global.H"
#include "IOVideo.H"
#include <vector>


class Frame {
private:
  void copyArea(const float  *iComp, float  *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int oMinX, int oMinY, int iWidth, int iHeight, int oWidth, int oHeight);
  void copyArea(const imgpel *iComp, imgpel *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int oMinX, int oMinY, int iWidth, int iHeight, int oWidth, int oHeight);
  void copyArea(const uint16 *iComp, uint16 *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int oMinX, int oMinY, int iWidth, int iHeight, int oWidth, int oHeight);
  void copyArea(const float  *iComp, float  *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int oMinX, int oMinY, int iWidth, int iHeight, int oWidth, int oHeight, bool flip);
  void copyArea(const imgpel *iComp, imgpel *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int oMinX, int oMinY, int iWidth, int iHeight, int oWidth, int oHeight, bool flip);
  void copyArea(const uint16 *iComp, uint16 *oComp, int iMinY, int iMaxY, int iMinX, int iMaxX, int oMinX, int oMinY, int iWidth, int iHeight, int oWidth, int oHeight, bool flip);
  
  // Code from EE6 and jzhao@sharplabs.com
  //void clipComponent(imgpel *oComp, int compSize, int minPelValue, int maxPelValue);
  //void clipComponent(uint16 *oComp, int compSize, int minPelValue, int maxPelValue);
  template <typename ValueType>  void clipComponent(ValueType *oComp, int, int, int);

public:
  
  FrameFormat    m_format;            //!< Consolidated format parameters

  bool           m_isFloat;
  ColorSpace     m_colorSpace;        // Color spaces supported
                                      // CM_YCbCr =  0,
                                      // CM_RGB   =  1,
                                      // CM_XYZ   =  2
  ColorPrimaries m_colorPrimaries;    // Color primaries supported
                                      // CP_709   = 0,
                                      // CP_2020  = 1,
                                      // CP_P3D60 = 2,
                                      // CP_NONE  = 3. CP_NONE makes sense for XYZ
  SampleRange    m_sampleRange;       // Content sample range, 0: Standard, 1: Full, 2: SDI (only for fixed precision types)
  ChromaFormat   m_chromaFormat;      // 0: 420, 1:422, 2:444
  
  int            m_noComponents;
  int            m_bitDepth;          // data bit depth (common/max)
  int            m_bitDepthComp[4];   // component bitdepth

  int64          m_size;              // number of samples
  int            m_compSize[4];       // number of samples in each color component
  int            m_height[4];         // height of each color component
  int            m_width[4];          // width of each color component
  
  int            m_frameNo;           // frame number
  int            m_isAvailable;


  int            m_minPelValue[4];    // minimum value for pel component
  int            m_midPelValue[4];    // middle value for pel component
  int            m_maxPelValue[4];    // maximum value for pel component

  vector<imgpel> m_data;              // data array (vector)
  imgpel        *m_comp[4];          // pointer to the different color components

  // we could also use templates to do the same thing. TBD
  // For now, sticking with this solution though
  vector<uint16> m_ui16Data;          // data array (vector) for unsigned short types
  uint16        *m_ui16Comp[4];     // pointer to the different color components (for unsigned short data)

  // floating point data for openexr (or other float, i.e. half or single, types)
  vector<float>  m_floatData;        // data array (vector) for single precision float types
  float         *m_floatComp[4];    // pointer to the different color components (for unsigned short data)
  
  bool           m_hasAlternate;
  double         m_altFrameNorm;
  Frame         *m_altFrame;

  // Functions
  Frame(int width, int height, bool isFloat, ColorSpace colorSpace, ColorPrimaries colorPrimaries, ChromaFormat chromaFormat, SampleRange sampleRange, int bitDepth, bool isInterlaced, TransferFunctions transferFunction, float systemGamma);
  ~Frame();
  
  // Code from EE6 and jzhao@sharplabs.com
  void clipRange();

  void clear();
  void copy(Frame *f);
  void copy(Frame *f, int component);
  void copy(Frame *f, int sourceMinX, int sourceMinY, int sourceMaxX, int sourceMaxY, int targetX, int targetY, bool flip);
  void copy(Frame *f, int sourceMinX, int sourceMinY, int sourceMaxX, int sourceMaxY, int targetX, int targetY);
  void copy(Frame *f, int component, int sourceMinX, int sourceMinY, int sourceMaxX, int sourceMaxY, int targetX, int targetY);

  bool equalType    (Frame *f);
  bool equalTypePart(Frame *f);
  bool equalSize    (Frame *f);
};

#endif
