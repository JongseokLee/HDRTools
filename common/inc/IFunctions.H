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
 ************************************************************************
 * \file IFunctions.H
 *
 * \brief
 *    Various inline functions. Code is based on the code that was
 *    contributed in the past to the JM, plus/minus some additions.
 *    May add some more in the future or modify some of the existing 
 *    ones.
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 ************************************************************************
 */



#ifndef __IFunctions_H__
#define __IFunctions_H__

#include "Defines.H"
#include <math.h>
#include <limits.h>

//-----------------------------------------------------------------------------
// Some basic functions, to be inlined
//-----------------------------------------------------------------------------

static inline int iMin(int a, int b) {
  return ((a) < (b)) ? (a) : (b);
}

static inline int iMax(int a, int b) {
  return ((a) > (b)) ? (a) : (b);
}

static inline short sMin(short a, short b) {
  return ((a) < (b)) ? (a) : (b);
}

static inline short sMax(short a, short b) {
  return ((a) > (b)) ? (a) : (b);
}

static inline float fMin(float a, float b) {
  return ((a) < (b)) ? (a) : (b);
}

static inline float fMax(float a, float b) {
  return ((a) > (b)) ? (a) : (b);
}

static inline double dMin(double a, double b) {
  return ((a) < (b)) ? (a) : (b);
}

static inline double dMax(double a, double b) {
  return ((a) > (b)) ? (a) : (b);
}

static inline int iClip(int x, int low, int high) {
  x = iMax(x, low);
  x = iMin(x, high);

  return x;
}

static inline int clip1(int x, int high) {
  x = iMax(x, ZERO);
  x = iMin(x, high);

  return x;
}

static inline short sClip(short x, short low, short high) {
  x = sMax(x, low);
  x = sMin(x, high);

  return x;
}

static inline float fClip(float x, float low, float high) {
  x = fMax(x, low);
  x = fMin(x, high);
  
  return x;
}

static inline double dClip(double x, double low, double high) {
  x = dMax(x, low);
  x = dMin(x, high);

  return x;
}

template <typename Type>
static inline Type setClosest(Type x, Type a, Type b) {
  if (abs(x - a) > abs(x - b)) {
    return b;
  }
  else {
    return a;
  }
}


static inline short sAbs(short x) {
  static const short SHORT_BITS = (sizeof(short) * CHAR_BIT) - 1;
  short y = (short) (x >> SHORT_BITS);
  return (short) ((x ^ y) - y);
}

static inline int iAbs(int x) {
  static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
  int y = x >> INT_BITS;
  return (x ^ y) - y;
}

static inline float fAbs(float x) {
  return ((x) < 0) ? -(x) : (x);
}

static inline double dAbs(double x) {
  return ((x) < 0) ? -(x) : (x);
}

static inline int64 i64Abs(int64 x) {
  static const int64 INT64_BITS = (sizeof(int64) * CHAR_BIT) - 1;
  int64 y = x >> INT64_BITS;
  return (x ^ y) - y;
}

static inline double dAbs2(double x) {
  return (x) * (x);
}

static inline int iAbs2(int x) {
  return (x) * (x);
}

static inline int iSign(int x) {
  return ((x) < 0) ? -1 : 1;
  // Following code should work if >> performs based on sign
  //static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
  //return (((x >> INT_BITS) | 0x01);
}

static inline float fSign(float x) {
  return ((x) < 0.0) ? -1.0f : 1.0f;
}


static inline double dSign(double x) {
  return ((x) < 0.0) ? -1.0 : 1.0;
}

static inline float fRoundLowBias(float x) {
  return (fSign(x) * floor((fAbs(x) + 0.475f)));
}

static inline float fRound(float x) {
  return (fSign(x) * floor((fAbs(x) + 0.5f)));
}

static inline double dRound(double x) {
  return (dSign(x) * floor((dAbs(x) + 0.5)));
}

static inline float fCeil(float x) {
  return (fSign(x) * floor((fAbs(x) + 1.0f)));
}

static inline double dCeil(double x) {
  return (dSign(x) * floor((dAbs(x) + 1.0)));
}


static inline int ceilingRescale(int val, int b) {
  return ((val + (1 << (b)) - 1) >> b)<<b;
}

static inline int ceilingRescaleDivide(int val, int b) {
  return ((val + b - 1) / b) * b;
}

static inline int iRightShiftRound(int val, int a)
{
  return (a > 0) ? ((val + (1 << (a-1) )) >> a) : (val << (-a));
}

static inline int iShiftRightRound(int val, int b) {
  return (val + (1 << (b-1))) >> b;
}

static inline uint16 uiShiftRightRound(uint16 val, uint16 b) {
  return (val + (1 << (b-1))) >> b;
}

static inline uint32 uiShiftRightRound(uint32 val, uint32 b) {
  return (val + (1 << (b-1))) >> b;
}

static inline int iShiftOffsetSafe(int val, int a, int b) {
  return ((val + a) >> b);
}

static inline int power2(int x) {
  return ONE << x;
}

static inline int signPower2(int x) {
  return ( x < 0 ) ? 0 : (ONE << x);
}

// rounding down the the next power of 2
static inline int flp2(int y) {
  uint32 x = (uint32) y;
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  x |= ( x >> 16 );
  return (int) (x - (x >> 1));
}

static inline uint32 flp2(uint32 x) {
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  x |= ( x >> 16 );
  return (x - (x >> 1));
}

static inline uint16 flp2(uint16 x) {
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  return (x - (x >> 1));
}

static inline short flp2(short x) {
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  return (x - (x >> 1));
}

// rounding up the the next power of 2
static inline int clp2(int y) {
  uint32 x = ((uint32) y) - 1;
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  x |= ( x >> 16 );
  return (int) (x + 1);
}

static inline uint32 clp2(uint32 x) {
  x -= 1;
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  x |= ( x >> 16 );
  return (x + 1);
}

static inline uint16 clp2(uint16 x) {
  x -= 1;
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  return (x + 1);
}

static inline short clp2(short x) {
  x -= 1;
  x |= ( x >> 1 );
  x |= ( x >> 2 );
  x |= ( x >> 4 );
  x |= ( x >> 8 );
  return (x + 1);
}

static inline int median(int a, int b, int c) {
  //return a + b + c - iMin(iMin(a, b), c) - iMax(iMax(a, b), c);
  return iMax(iMin(iMax(a, b), c), iMin(a, b));
}

static inline int average3(int a, int b, int c) {
  return (int) floor((((double) a + (double) b + (double) c) / 3.0) + 0.5);
}

static inline uint16 average3(uint16 a, uint16 b, uint16 c) {
  return (uint16) floor((((double) a + (double) b + (double) c) / 3.0) + 0.5);
}

static inline uint32 average3(uint32 a, uint32 b, uint32 c) {
  return (uint32) floor((((double) a + (double) b + (double) c) / 3.0) + 0.5);
}

static inline float average3(float a, float b, float c) {
  return (float) (((double) a + (double) b + (double) c) / 3.0);
}


static inline uint16 sim2FilterAnchor(uint16 a, uint16 b, uint16 c) {
  return (uint16) (((int) a + 6 * (int) b + (int) c + 4) >> 3);
}

static inline uint32 sim2FilterAnchor(uint32 a, uint32 b, uint32 c) {
  return (uint32) (((int) a + 6 * (int) b + (int) c + 4) >> 3);
}

static inline int sim2FilterAnchor(int a, int b, int c) {
  return ((a + 6 * b + c + 4) >> 3);
}


static inline int float2int(float f) {
  return (int)(f < 0 ? f - 0.5: f + 0.5);
}

static inline int double2int(double x) {
  return (int)(x < 0 ? x - 0.5: x + 0.5);
}

static inline int double2IntCeil(double x) {
  return (((x) < 0.0 ) ? (((int)(x))) : (((int)(x))+1));
}

static inline int double2IntFloor(double x) {
  return (((x) < 0.0 ) ? (((int)(x))-1) : (((int)(x))));
}

static inline uint16 ceilDivide(uint16 x, uint16 y) {
  return (x / y + (x % y != 0));
}

static inline uint32 ceilDivide(uint32 x, uint32 y) {
  return (x / y + (x % y != 0));
}

static inline int ceilDivide(int x, int y) {
  return (x / y + (x % y != 0));
}

static inline double psnr(double maxSampleValue, int samples, double sseDistortion, double minLimit) {
  return (double) (10.0 * log10(maxSampleValue * maxSampleValue * (double) ((double) samples / (sseDistortion < minLimit ? minLimit : sseDistortion))));
}

template <typename Type>
static inline void swap(Type *x, Type *y) {
  Type temp = *x;
  *x = *y;
  *y = temp;
}


#ifdef JM_PSNR
static inline double psnr(double maxSampleValue, int samples, double sseDistortion) {
  return (double) (10.0 * log10((double) maxSampleValue * (double) maxSampleValue * (double) ((double) samples / (sseDistortion < 1e-20 ? 1e-20 : sseDistortion))));
}

static inline double psnr(int maxSampleValue, int samples, double sseDistortion) {
  return (double) (10.0 * log10((double) maxSampleValue * (double) maxSampleValue * (double) ((double) samples / (sseDistortion < 1.0 ? 1.0 : sseDistortion))));
}
#else
static inline double psnr(double maxSampleValue, int samples, double sseDistortion) {
  return (double) (sseDistortion == 0.0 ? 0.0 : (10.0 * log10((double) maxSampleValue * (double) maxSampleValue * (double) ((double) samples / sseDistortion))));
}
static inline double psnr(int maxSampleValue, int samples, double sseDistortion) {
  return (double) (sseDistortion == 0.0 ? 0.0 : (10.0 * log10((double) maxSampleValue * (double) maxSampleValue * (double) ((double) samples / sseDistortion))));
}
#endif

static inline double ssimSNR(double distortion) {
  return (double) (-10.0 * log10(((1 - distortion) < 1e-20 ? 1e-20 : (1 - distortion))));
}


#endif
