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
 * \file OpenEXR.H
 *
 * \brief
 *    Main OpenEXR header file
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __OpenEXR_H__
#define __OpenEXR_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------


enum Compression {
  NO_COMPRESSION          = 0,        // No compression. Only this mode is currently
                                      // supported.
  RLE_COMPRESSION         = 1,        // Run Length encoding
  ZIPS_COMPRESSION        = 2,        // Scan line Zlib compression
  ZIP_COMPRESSION         = 3,        // 16 scan line block Zlib compression
  PIZ_COMPRESSION         = 4,        // PIZ-based wavelet compression
  PXR24_COMPRESSION       = 5,        // Lossy 24-bit float compression
  B44_COMPRESSION         = 6,        // Lossy 4-by-4 pixel block compression,
  B44A_COMPRESSION        = 7,        // Lossy 4-by-4 pixel block compression,
  NUM_COMPRESSION_METHODS             // Total number of compression methods
};

struct box2i {
  int32 xMin;
  int32 yMin;
  int32 xMax;
  int32 yMax;
  
  box2i () {
    xMin = yMin = xMax = yMax = 0;
  }
};

struct channelInfo {
  char    name[255];        // Shall be one of R, G, B.
  int32   pixelType;
  uint32  pLinear;
  int32   xSampling;
  int32   ySampling;
  
  // fixed values, shall be {1, 0, 1, 1}
  channelInfo()
  {
    pixelType = HALF;
    xSampling = ySampling = 1;
    pLinear = 0;
  }
};


struct attributeEXR {
public:
  char    *name;
  char    *type;
  int     size;
  char    *value;
};


#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
