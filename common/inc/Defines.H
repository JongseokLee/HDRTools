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
 * \file Defines.H
 *
 * \brief
 *    Simple defines used throughout the entire system
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __Defines_H__
#define __Defines_H__

#include <stdint.h>

#define VERSION "0.16"
#define UNDEFINED 0x7fffffff
#define INT_INF 0x7fffffff
#define FLT_INF 1e30F

#define ENABLE_SSE_OPT      0


#define MAKE_TEXT(x) #x

#if (defined(WIN32) || defined(WIN64)) && !defined(__GNUC__)
typedef unsigned char       imgpel;                 //!<  Pixel type definition (8 bit)
typedef unsigned char       byte;
typedef unsigned __int8     uint8;
typedef unsigned __int16    uint16;
typedef unsigned __int32    uint32;
typedef unsigned __int64    uint64;
typedef          __int8     int8;
typedef          __int16    int16;
typedef          __int32    int32;
typedef          __int64    int64;
# ifndef INT64_MIN
#  define INT64_MIN        (-9223372036854775807i64 - 1i64)
# endif
#else
typedef unsigned char       imgpel;                 //!<  Pixel type definition (8 bit)
typedef unsigned char       byte;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;
typedef          char       int8;
typedef          short      int16;
typedef          int        int32;
typedef          long long  int64;
# ifndef INT64_MIN
#  define INT64_MIN        (-9223372036854775807LL - 1LL)
# endif
#endif



// lets define half as float16 and make it ot type unsigned short 
typedef unsigned short float16;

// boolean constants
#ifndef FALSE
enum {
  FALSE = 0,
  TRUE  = 1
};
#endif

// integer constants (helps with searches in code)
enum {
  ZERO  = 0,
  ONE   = 1,
  TWO   = 2,
  THREE = 3,
  FOUR  = 4,
  FIVE  = 5
};

// Color components
enum {
  Y_COMP  = 0,    // Y Component
  R_COMP  = 0,    // R Component
  U_COMP  = 1,    // U Component
  Cb_COMP = 1,
  UP_COMP = 1,    // U' Component
  G_COMP  = 1,    // G Component
  V_COMP  = 2,    // V Component
  Cr_COMP = 2,
  VP_COMP = 2,    // V' Component
  B_COMP  = 2,    // B Component
  A_COMP  = 3,    // Alpha Component
  T_COMP
};



// File format definitions
enum{
  FF_RAW = 0,
  FF_EXR = 1,
  FF_TIF = 2
};

// command line parameter parsing
enum {
  MAX_CL_PARAM_LENGTH = 2000,
  MAX_CL_PARAMS       = 200
};

// various
enum {
  FILE_NAME_SIZE = 255,
  MAX_LINE_LEN = 2000, // maximum length of line in config file
  MAX_FILT_PIC = 6     // maximum number of frame filters
};

#endif
