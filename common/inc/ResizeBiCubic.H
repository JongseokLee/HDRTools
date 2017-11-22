/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Samsung Electronics
 * <ORGANIZATION> = Samsung Electronics
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, Samsung Electronics
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


/* 
*************************************************************************************
 * \file ResizeBiCubic.H
 *
 * \brief
 *    BiCubic resizer class Header
 *
 * \author
 *     - Sarvesh Sahota         <sa.sahota@samsung.com>
 *     - Kulbhushan Pachauri	  <kb.pachauri@samsung.com>
 *       
 *************************************************************************************
 */

#ifndef __RESIZE_BICUBIC_H__
#define __RESIZE_BICUBIC_H__

#include "Global.H"
#include "Frame.H"
#include <vector>

static const int MAX_ESIZE=16;



class ResizeBiCubic {
private:  
  static int  verticalSSE( const byte** in, byte* out, const byte* _beta, int width );
  static void vertical   ( const float** src, float* dst, const float* beta, int width );
  static void horizontal ( const float** src, float** dst, int count, const int* xofs, const float* alpha,  int swidth, int dwidth, int cn, int xmin, int xmax );
  static void resize     ( float * src, const int _src_w, const int src_h, float * dst, const int _dst_w, const int dst_h, 
	const int cn, const int* xofs, const void* _alpha, const int* yofs, const void* _beta, int xmin, int xmax, int ksize);
public:
  static void compute( float * _src, const int src_w, const int src_h, float * _dst, 
	const int dst_w, const int dst_h, const int cn );
  
  ResizeBiCubic(){};
  ~ResizeBiCubic(){};
};

#endif