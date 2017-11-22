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
 * \file Filter1D.H
 *
 * \brief
 *    Filter1D class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __Filter1D_H__
#define __Filter1D_H__

#include "Global.H"
#include "Frame.H"


// Filters are specified as follows
// pos0 => number of taps
// pos1 = pos0+1 .. pos2=(pos1 + pos0) => tap values
// pos2 => offset
// pos3 = pos2+1 => shift  
static const double Filters1D[24][24] = {
    { 1.0, +256.0,                                                                                     +128.0, 8.0}, // No filter
    { 2.0, +256.0,  +256.0,                                                                            +256.0, 9.0}, // Average
    { 3.0,  +64.0,  +384.0,  +64.0,                                                                    +256.0, 9.0}, // [1 6 1]/8
    { 3.0, +128.0,  +256.0, +128.0,                                                                    +256.0, 9.0}, // [1 2 1]/4
    { 4.0,  -16.0,  +144.0, +144.0,   -16.0,                                                           +128.0, 8.0}, // LZ2
    { 5.0,   +17.0,  +62.0,  +98.0,   +62.0,  +17.0,                                                   +128.0, 8.0},
    { 6.0,   +6.0,   -34.0, +156.0,  +156.0,  -34.0,   +6.0,                                           +128.0, 8.0},
    { 6.0,  +21.0,   -52.0, +159.0,  +159.0,  -52.0,  +21.0,                                           +128.0, 8.0},
    { 7.0,   +1.0,   +14.0,  +62.0,  +102.0,  +62.0,  +14.0,   +1.0,                                   +128.0, 8.0}, // GS 7 tap - 9th filter
    { 8.0,   -3.0,   +15.0,  -43.0,  +159.0, +159.0,  -43.0,  +15.0,  -3.0,                            +128.0, 8.0},
    { 10.0,  +2.0,    -8.0,  +21.0,   -47.0, +160.0, +160.0, -47.0,  +21.0,  -8.0, +2.0,               +128.0, 8.0},
    { 12.0,  -1.0,    +5.0,  -12.0,   +24.0,  -49.0, +161.0, +161.0, -49.0, +24.0, -12.0,  +5.0, -1.0, +128.0, 8.0},
    {11.0,  +21.0,    +0.0,  -52.0,    +0.0, +159.0, +256.0, +159.0,  +0.0, -52.0,   0.0, +21.0,       +256.0, 9.0},
    {12.0,   +5.0,   +11.0,  -21.0,   -37.0,  +70.0, +228.0, +228.0, +70.0, -37.0, -21.0, +11.0, +5.0, +256.0, 9.0},
};



class Filter1D {
public:
  int     m_numberOfTaps;
  int     m_positionOffset;
  float  *m_floatFilter;
  float   m_floatOffset;
  float   m_floatScale;
  int32  *m_i32Filter;
  int32   m_i32Offset;
  int32   m_i32Shift;
  bool    m_clip;
  
  //template<typename ValueType>  double filterH(ValueType *inp, int pos_x, int width,             float minValue, float maxValue);  
  //template<typename ValueType>  double filterV(ValueType *inp, int pos_y, int width, int height, float minValue, float maxValue);

  double filterH(double *inp, int pos_x, int width,             float minValue, float maxValue);  
  double filterV(double *inp, int pos_y, int width, int height, float minValue, float maxValue);  
  double filterH(float  *inp, int pos_x, int width,             float minValue, float maxValue);  
  double filterV(float  *inp, int pos_y, int width, int height, float minValue, float maxValue);
  double filterH(imgpel *inp, int pos_x, int width,             float minValue, float maxValue);  
  double filterV(imgpel *inp, int pos_y, int width, int height, float minValue, float maxValue);
  double filterH(uint16 *inp, int pos_x, int width,             float minValue, float maxValue);  
  double filterV(uint16 *inp, int pos_y, int width, int height, float minValue, float maxValue);

  
  Filter1D(int filter, int paramsMode, int inOffset, int inShift, int *outOffset, int *outShift, bool verbose = FALSE);
  ~Filter1D();
};

#endif
