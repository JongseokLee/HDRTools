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
 * \file Convert.cpp
 *
 * \brief
 *    Convert Class from one frame format to another based on pixel precision/type
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "Convert.H"
#include "ConvertBitDepth.H"
#include "ConvFloatToFixed.H"
#include "ConvFixedToFloat.H"
#include "ConvertNull.H"

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
Convert *Convert::create(const FrameFormat *iFormat, const FrameFormat *oFormat, const ConvertParams *params) {
  Convert *result = NULL;
  
  // Ideally this function should examine the input and output formats, and make appropriate decisions
  // of what processing to perform based on that, and possibly additional setup parameters that unify
  // all conversion functions. To be decided/done at a later stage maybe.
  if (iFormat->m_isFloat == oFormat->m_isFloat && iFormat->m_isFloat == TRUE) {
    result = new ConvertNull();
  }
  else {
    if (iFormat->m_isFloat == FALSE && oFormat->m_isFloat == TRUE)
      result = new ConvFixedToFloat();
    else if (iFormat->m_isFloat == FALSE && oFormat->m_isFloat == FALSE)
      result = new ConvertBitDepth();
    else if (iFormat->m_isFloat == TRUE && oFormat->m_isFloat == FALSE)
      result = new ConvFloatToFixed();
    else {
      fprintf(stderr, "Unsupported Conversion %d %d (%d %d %d %d)\n", iFormat->m_chromaFormat, oFormat->m_chromaFormat, iFormat->m_width[Y_COMP], iFormat->m_height[Y_COMP], oFormat->m_width[Y_COMP], oFormat->m_height[Y_COMP]);
      exit(EXIT_FAILURE);
    }
  }

  return result;
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------


