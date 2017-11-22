/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, Apple Inc.
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
 * \file ColorTransformClosedLoop.H
 *
 * \brief
 *    ColorTransformClosedLoop class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __ColorTransformClosedLoop_H__
#define __ColorTransformClosedLoop_H__

#include "Global.H"
#include "Frame.H"
#include "ColorTransform.H"
#include "ColorTransformGeneric.H"


class ColorTransformClosedLoop : public ColorTransform {
private:
  
  const double   *m_transform0;
  const double   *m_transform1;
  const double   *m_transform2;
  bool            m_clip;
  double          m_crDivider;
  double          m_cbDivider;
  
  vector<float>   m_floatData;
  float          *m_floatComp[4];
  int64           m_size;              // number of samples
  int             m_compSize[4];       // number of samples in each color component
  int             m_height[4];         // height of each color component
  int             m_width[4];          // width of each color component
  bool            m_memoryAllocated;
  
  int             m_mode;
  bool            m_isForward;
  ClosedLoopTrans m_closedLoopTransform;
   
public:
  // Construct/Deconstruct
  ColorTransformClosedLoop( ColorTransformParams *params );
  virtual ~ColorTransformClosedLoop();
  
  virtual void process(Frame *out,  const Frame *inp);
  
};

#endif
