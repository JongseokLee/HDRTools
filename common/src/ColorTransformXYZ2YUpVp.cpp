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
 * \file ColorTransformXYZ2YUpVp.cpp
 *
 * \brief
 *    ColorTransformXYZ2YUpVp Class
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
#include "ColorTransformXYZ2YUpVp.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformXYZ2YUpVp::ColorTransformXYZ2YUpVp() {
}

ColorTransformXYZ2YUpVp::~ColorTransformXYZ2YUpVp() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ColorTransformXYZ2YUpVp::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  // Current condition to perform this is that Frames are of same size and in 4:4:4
  // Can add more code to do the interpolation on the fly (and save memory/improve speed),
  // but this keeps our code more flexible for now.
  
  if (inp->m_compSize[Y_COMP] == out->m_compSize[Y_COMP] && inp->m_compSize[Y_COMP] == inp->m_compSize[U_COMP])
  {
    {
      if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE)  {
        double denom;
        double scale;
        if (inp->m_colorSpace == CM_XYZ && out->m_colorSpace == CM_YUpVp) {
          for (int i = 0; i < inp->m_compSize[0]; i++) {
            out->m_floatComp[0][i] = inp->m_floatComp[1][i]; // Y => Y
            denom = ((double) inp->m_floatComp[0][i] + 15.0 * (double) inp->m_floatComp[1][i] + 3.0 * (double) inp->m_floatComp[2][i]);
            if (denom != 0.0) {
              scale = 1.0 / denom;
              
              out->m_floatComp[1][i] = fClip((float) (4.0 * (double) inp->m_floatComp[0][i] * scale), 0.0f, 1.0f); // u' - Should we clip from 0.001 to 0.625 as done for the Sim2? Maybe not for now
              out->m_floatComp[2][i] = fClip((float) (9.0 * (double) inp->m_floatComp[1][i] * scale), 0.0f, 1.0f); // v' - Should we clip from 0.001 to 0.625 as done for the Sim2? Maybe not for now
            }
            else {
              out->m_floatComp[1][i] = (float) 0.197830013378341; // zero case
              out->m_floatComp[2][i] = (float) 0.468319974939678; // zero case
            }
            
            //printf("(%d) Values (YUpVp) (%7.3f %7.3f %7.3f) => (%9.7f %9.7f %9.7f)\n", i, inp->m_floatComp[0][i], inp->m_floatComp[1][i], inp->m_floatComp[2][i], out->m_floatComp[0][i], out->m_floatComp[1][i], out->m_floatComp[2][i]);
          }
        }
        else if (out->m_colorSpace == CM_XYZ && inp->m_colorSpace == CM_YUpVp) {
          for (int i = 0; i < inp->m_compSize[0]; i++) {
            scale = (double) inp->m_floatComp[0][i] / (4.0 * (double) inp->m_floatComp[2][i]);
            
            out->m_floatComp[0][i] = fClip((float) (9.0 * (double) inp->m_floatComp[1][i] * scale), 0.0f, 1.0f); // X
            out->m_floatComp[1][i] = inp->m_floatComp[0][i]; // Y => Y
            out->m_floatComp[2][i] = fClip((float) ((12.0 - 3.0 * (double) inp->m_floatComp[1][i] - 20.0 * (double) inp->m_floatComp[2][i] )* scale), 0.0f, 1.0f); // Z
          }
        }
        else {
          // format not supported and we should not be reaching here
        }
      }
      else { // fixed precision, integer image data
             // Currently this is not supported. We always assume floating point values
      }
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
