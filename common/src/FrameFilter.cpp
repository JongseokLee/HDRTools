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
 * \file FrameFilter.cpp
 *
 * \brief
 *    Base Class for Frame Scaling
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
#include "FrameFilter.H"
#include "FrameFilterNull.H"
#include "FrameFilterDeblock.H"
#include "FrameFilterWiener2D.H"
#include "FrameFilterWiener2DDark.H"
#include "FrameFilter2DSep.H"
#include "FrameFilterNLMeans.H"

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
FrameFilter *FrameFilter::create(int iWidth, int iHeight, int method, bool mode) {
  FrameFilter *result = NULL;
  
  if (method == FT_NULL) { // Do nothing
    result = new FrameFilterNull();
  }
  else if (method == FT_DEBLOCK) { 
    result = new FrameFilterDeblock(iWidth, iHeight);
  }
  else if (method == FT_WIENER2D) { 
    result = new FrameFilterWiener2D(iWidth, iHeight);
  }
  else if (method == FT_2DSEP) { 
    result = new FrameFilter2DSep(iWidth, iHeight, F1D_GS7, mode);
  }
  else if (method == FT_WIENER2DD) {
    result = new FrameFilterWiener2DDark(iWidth, iHeight);
  }
  else if (method == FT_NLMEANS) {
    result = new FrameFilterNLMeans(iWidth, iHeight);
  }
  
  return result;
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
