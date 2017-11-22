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
 * \file ColorTransformYAdjust2ndOrder.H
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


#ifndef __ColorTransformYAdjust2ndOrder_H__
#define __ColorTransformYAdjust2ndOrder_H__

#include "Global.H"
#include "Frame.H"
#include "ColorTransform.H"
#include "ColorTransformGeneric.H"
#include "ConvertColorFormat.H"
#include "TransferFunction.H"
#include "Convert.H"

class YAdjustSearchTable 
{ 
public:
  int m_index; 
  bool m_leaf; 
  YAdjustSearchTable() {
    m_index = 0;
    m_leaf = FALSE;
  }
};

class ColorTransformYAdjust2ndOrder : public ColorTransform {
private:
  static const int           m_modelCoeffs = 7;
  
  const double              *m_transform0;
  const double              *m_transform1;
  const double              *m_transform2;
  const double              *m_invTransform0;
  const double              *m_invTransform1;
  const double              *m_invTransform2;
  const double              *m_transformRGBtoY;  // Y in XYZ

  vector<float>              m_floatData;
  float                     *m_floatComp[4];
  int64                      m_size;              // number of samples
  int                        m_compSize[4];       // number of samples in each color component
  int                        m_height[4];         // height of each color component
  int                        m_width[4];          // width of each color component
  bool                       m_memoryAllocated;
  bool                       m_tfDistance;
  
  int                        m_mode;
  int                        m_invMode;
  int                        m_modeRGB2XYZ;
  bool                       m_isICtCp;
  
  ConvertColorFormat        *m_fwdColorFormat;
  ConvertColorFormat        *m_invColorFormat;
    
  Convert                   *m_fwdConvertProcess; 
  Convert                   *m_invConvertProcess;
   
  Frame                     *m_fwdFrameStore2; 
  Frame                     *m_invFrameStore2; 
  Frame                     *m_fwdFrameStore;
  Frame                     *m_invFrameStore;
  double                     m_luminanceRange;
  double                     m_lumaWeight;
  double                     m_chromaWeight;
  double                     m_lumaOffset;
  double                     m_chromaOffset;
  TransferFunction          *m_transferFunction;

  // 2nd order approximation method related parameters
  string                     m_modelFile;
  vector<YAdjustSearchTable> m_searchTable;
  vector<double>             m_coeffTable;

  void                allocateMemory(Frame* out, const Frame *inp);
  void                setModelFile (const char *modelFile);
  
  double              convertToYLinear(const double rComp, const double gComp, const double bComp);
  double              convertToY      (const double rComp, const double gComp, const double bComp);

  int                 tableSearch(unsigned comp0, unsigned comp1, unsigned comp2) const;
  int                 calcYprime(int ylin, int cb, int cr, const double *coeffs);

public:
  // Construct/Deconstruct
  ColorTransformYAdjust2ndOrder( ColorTransformParams *params );
                        
  virtual ~ColorTransformYAdjust2ndOrder();
  
  virtual void process      (Frame *out,  const Frame *inp);
};

#endif
