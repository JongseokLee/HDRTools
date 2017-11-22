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
 ************************************************************************
 * \file TypeDefs.H
 *
 * \brief
 *    Type definitions for HDRTools
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 ************************************************************************
 */

#ifndef _TypeDefs_H_
#define _TypeDefs_H_

#include "Defines.H"

typedef enum {
  VIDEO_UNKNOWN = -1,
  VIDEO_YUV     =  0,
  VIDEO_RGB     =  1,
  VIDEO_XYZ     =  2,
  VIDEO_TIFF    =  3,
  VIDEO_AVI     =  4,
  VIDEO_EXR     =  5,
  VIDEO_Y4M     =  6,
  VIDEO_DPX     =  7,
  VIDEO_TOTAL
} VideoFileType;

typedef enum {
  CM_UNKNOWN   = -1,
  CM_YCbCr     =  0,
  CM_RGB       =  1,
  CM_XYZ       =  2,
  CM_YDZDX     =  3,
  CM_YCbCr_CL  =  4,
  CM_YFBFRV1   =  5, // Entries 5-8 correspond to FastVDO's proposed color spaces
  CM_YFBFRV2   =  6,
  CM_YFBFRV3   =  7,
  CM_YFBFRV4   =  8,
  CM_YUpVp     =  9,
  CM_ICtCp     = 10,
  CM_TOTAL
} ColorSpace;


typedef enum {
  CP_UNKNOWN = -1,
  CP_709     = 0,
  CP_2020    = 1,
  CP_P3D60   = 2,
  CP_P3D65   = 3,
  CP_XYZ     = 4,
  CP_NONE    = 5,
  CP_AMT     = 6,
  CP_YCOCG   = 7,
  CP_EXT     = 8,
  CP_LMSD    = 9, // LMS Dolby primaries
  CP_601     = 10,
  CP_TOTAL
} ColorPrimaries;

typedef enum {
  DIF_UNKNOWN = -1,
  DIF_PQPH10K = 0,
  DIF_PQ      = 1,
  DIF_DE2K    = 2,
  DIF_PQNS    = 3,
  DIF_NULL    = 4,
  DIF_HPQ     = 5,
  DIF_HPQ2    = 6,
  DIF_TOTAL      
} DistortionFunction;

typedef enum {
  CF_UNKNOWN = -1,     //!< Unknown color format
  CF_400     =  0,     //!< Monochrome
  CF_420     =  1,     //!< 4:2:0
  CF_422     =  2,     //!< 4:2:2
  CF_444     =  3,     //!< 4:4:4
  CF_TOTAL
} ChromaFormat;

typedef enum {
  PF_UNKNOWN = -1,     //!< Unknown color ordering
  PF_UYVY    =  0,     //!< UYVY
  PF_YUY2    =  1,     //!< YUY2
  PF_YUYV    =  2,     //!< YUYV
  PF_YVYU    =  3,     //!< YVYU
  PF_BGR     =  4,     //!< BGR
  PF_RGB     =  5,     //!< RGB
  PF_V210    =  6,     //!< V210
  PF_UYVY10  =  7,     //!< UYVY10
  PF_V410    =  8,     //!< V410
  PF_R210    =  9,     //!< R210
  PF_R10K    =  10,    //!< R10K
  PF_XYZ     =  11,    //!< XYZ
  PF_B64A    =  12,    //!< b64a
#ifdef __SIM2_SUPPORT_ENABLED__
  PF_SIM2    =  13,    //!< Sim2
#endif
  PF_TOTAL
} PixelFormat;

typedef enum  {
  UINT = 0,                           // Unsigned integer (32 bit) type
  HALF = 1,                           // Half floating point type (16 bits)
  FLOAT = 2,                          // Floating point (single precision - 32 bits)
  NUM_PIXEL_TYPES
} PixelTypes;

typedef enum {                        // Sample range
  SR_UNKNOWN    = -1,
  SR_STANDARD   =  0,                 // Standard/Limited/Video range (16-235) * k, 
                                      // i.e. Floor (219 * D * N + 16 * D + 0.5), with N = [0, 1]
  SR_FULL       =  1,                 // Full range (0 - 2^(k-1))
  SR_RESTRICTED =  2,                 // Restricted range. Similar to SDI but less restrictive
                                      // range (4 * D .. (1 << b) - 4 * D - 1) with D = 1<<(b-10)
  SR_SDI_SCALED =  3,                 // SDI range as currently used by Dolby's PQ TIFF File format. 
                                      // Floor(1015 * D * N + 4 * D + 0.5) with N = [0, 1] and D = 1<<(b-10). 
                                      // In this mode, values are always scaled to 16 bits.
  SR_SDI        =  4,                 // SDI range as currently used by Dolby's PQ TIFF File format. 
                                      // Floor(1015 * D * N + 4 * D + 0.5) with N = [0, 1] and D = 1<<(b-10)
  SR_TOTAL
} SampleRange;

typedef enum{
  TF_NULL      = 0,
  TF_PQ        = 1,
  TF_PH        = 2,
  TF_HG        = 3,
  TF_APQ       = 4,
  TF_APH       = 5,
  TF_POWER     = 6,
  TF_MPQ       = 7,
  TF_APQS      = 8,
  TF_BiasedMPQ = 9,
  TF_AMPQ      = 10,
  TF_HLG       = 11,
  TF_NORMAL    = 12,
  TF_HPQ       = 13,
  TF_HPQ2      = 14,
  TF_SRGB      = 15,
  TF_ASRGB     = 16,
  TF_POWERSMP  = 17,
  TF_BT709     = 18,
  TF_DPXLOG    = 19,
  TF_CINLOG    = 20,
  TF_ST240     = 21,
#ifdef __SIM2_SUPPORT_ENABLED__
  TF_SIM2,
#endif
  TF_TOTAL
} TransferFunctions;

typedef enum{
  TM_NULL      = 0,
  TM_ROLL      = 1,
  TM_BT2390    = 2,
  TM_CIE1931   = 3,
  TM_CIE1976   = 4,
  TM_BT2390IPT = 5,
  TM_TOTAL
} ToneMapper;

typedef enum{
  DA_NULL      = 0,
  DA_HLG       = 1,
  DA_TOTAL
} DisplayAdjustment;

typedef enum{
  ADF_NULL      = 0,
  ADF_MULTI     = 1,
  ADF_CREDGE    = 2,
  ADF_CRBOUNDS  = 3,
  ADF_CRFBOUNDS = 4,
  ADF_TOTAL
} AdaptiveDownsampler;


typedef enum{
  CL_ZERO  = 0,   // middle aligned vertically, left aligned (co-sited) horizontally
  CL_ONE   = 1,   // center aligned vertically & horizontally
  CL_TWO   = 2,   // top aligned (co-sited) vertically, left aligned (co-sited) horizontally
  CL_THREE = 3,   // top aligned (co-sited) vertically, middle aligned horizontally 
  CL_FOUR  = 4,   // bottom aligned (co-sited) vertically, left aligned (co-sited) horizontally
  CL_FIVE  = 5,   // bottom aligned (co-sited) vertically, middle aligned horizontally
  CL_TOTAL
} ChromaLocation;

typedef enum{
  FP_TOP    = 0,
  FP_FRAME  = 0,
  FP_BOTTOM = 1,
  FP_TOTAL  
} FieldParity;

typedef enum  { 
  FT_NULL      = 0,
  FT_DEBLOCK   = 1,
  FT_WIENER2D  = 2,
  FT_2DSEP     = 3,
  FT_WIENER2DD = 4,
  FT_NLMEANS   = 5,
  FT_TOTAL
} FilterTypes;

typedef enum {
  F1D_NULL      =  0,
  F1D_BILINEAR  =  1,
  F1D_FLT0      =  2,
  F1D_FLT1      =  3,
  F1D_LZ2       =  4,
  F1D_FLT2      =  5,
  F1D_FLT3      =  6,
  F1D_FLT4      =  7,
  F1D_GS7       =  8,
  F1D_FLT5      =  9,
  F1D_FLT6      =  10,
  F1D_FLT7      =  11,
  F1D_FLT8      =  12,
  F1D_FLT9      =  13,
  F1D_TOTAL
} FiltersOneD;

typedef enum {
  CLT_NULL     = 0,
  CLT_BASE     = 1,
  CLT_BASE2    = 2,
  CLT_BASE3    = 3,
  CLT_BASE4    = 4,
  CLT_YADJ     = 5,
  CLT_YADJALT  = 6,
  CLT_CHROMA   = 7,
  CLT_YADJFST  = 8,
  CLT_RGB      = 9,
  CLT_Y        = 10,
  CLT_XYZ      = 11,
  CLT_YADJLFST = 12,
  CLT_YADJTELE = 13,
  CLT_FRGB     = 14,
  CLT_YADJFULL = 15,
  CLT_YLIN     = 16,
  CLT_YSUMLIN  = 17,
  CLT_YADJ2ORD = 18,
  CLT_YADJHLG  = 19,
  CLT_YAPPLEI  = 20,
  CLT_YAPPLEM  = 21,
  CLT_YAPPLEP  = 22,
  CLT_YAPPLEL  = 23,
  CLT_TOTAL
} ClosedLoopTrans;

#endif

