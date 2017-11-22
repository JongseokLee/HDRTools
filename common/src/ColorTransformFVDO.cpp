/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = FastVDO Inc.
 * <ORGANIZATION> = FastVDO Inc.
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, FastVDO Inc.
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

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include "Global.H"
#include "ColorTransformFVDO.H"

//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ColorTransformFVDO::ColorTransformFVDO(ColorSpace iColorSpace, ColorSpace oColorSpace) {
  if ((iColorSpace == CM_RGB) && ((oColorSpace == CM_YFBFRV1) || (oColorSpace == CM_YFBFRV2) || (oColorSpace == CM_YFBFRV3) || (oColorSpace == CM_YFBFRV4)) ) {
    m_forward = TRUE; //indicates forward transform
  }
  else if ((oColorSpace == CM_RGB) && ((iColorSpace == CM_YFBFRV1) || (iColorSpace == CM_YFBFRV2) || (iColorSpace == CM_YFBFRV3) || (iColorSpace == CM_YFBFRV4)) ) {
    m_forward = FALSE; //indicates inverse transform
  }
}

ColorTransformFVDO::~ColorTransformFVDO() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ColorTransformFVDO::forward (Frame *out,  const Frame *inp)
{
  int p1, p2, p3;
  for (int i = 0; i < inp->m_compSize[0]; i++)  {
    if (inp->m_bitDepth > 8) {
      p3 = inp->m_ui16Comp[0][i]; // R
      p1 = inp->m_ui16Comp[1][i]; // G
      p2 = inp->m_ui16Comp[2][i]; // B
    }
    else {
      p3 = inp->m_comp[0][i]; // R
      p1 = inp->m_comp[1][i]; // G
      p2 = inp->m_comp[2][i]; // B
    }
    switch(out->m_colorSpace) {
      case CM_YFBFRV1:
        p3 -= p2; // p3 = R - B ; pixelV
        p2 += (p3 >> 1); //R/2 + B/2  ; B + (pixelV >> 1)
        p1 -= p2; // p1 = G - R/2 - B/2 ; pixelU
        p2 += ((p1 * 3) >> 3);//p2 = B + (pixelV >> 1) + 3*(pixelU >> 3) ; p2 = 3G/8 + 5B/16 + 5R/16 // .4,.3,.3
        p1  = iClip(((p1 + out->m_maxPelValue[2]) >> 1), out->m_minPelValue[2], out->m_maxPelValue[2]);
        p2  = iClip(p2, out->m_minPelValue[0], out->m_maxPelValue[0]);
        p3  = iClip(((p3 + out->m_maxPelValue[1]) >> 1), out->m_minPelValue[1], out->m_maxPelValue[1]);
        break;
      case CM_YFBFRV2:
        p3 -= p2; // p3 = R - B ; pixelV
        p2 += (p3 >> 1); //R/2 + B/2  ; B + (pixelV >> 1)
        p1 -= p2; // p1 = G - R/2 - B/2 ; pixelU
        p2 += ((p1 * 5) >> 3);//p2 = B + (pixelV >> 1) + 5*(pixelU >> 3) ; p2 =  5G/8 + 3B/16 + 3R/16 // .6,.2,.2
        p1  = iClip(((p1 + out->m_maxPelValue[2]) >> 1), out->m_minPelValue[2], out->m_maxPelValue[2]);
        p2  = iClip(p2, out->m_minPelValue[0], out->m_maxPelValue[0]);
        p3  = iClip(((p3 + out->m_maxPelValue[1]) >> 1), out->m_minPelValue[1], out->m_maxPelValue[1]);
        break;
      case CM_YFBFRV3:
        p3 -= p2; // p3 = R - B ; pixelV
        p2 += (p3 >> 1); //R/2 + B/2  ; B + (pixelV >> 1)
        p1 -= p2; // p1 = G - R/2 - B/2 ; pixelU
        p2 += ((p1 * 23) >> 5);//p2 = B + (pixelV >> 1) + 23*(pixelU >> 5) ; p2 =  23G/32 + 9B/64 + 9R/64
        p1  = iClip(((p1 + out->m_maxPelValue[2]) >> 1), out->m_minPelValue[2], out->m_maxPelValue[2]);
        p2  = iClip(p2, out->m_minPelValue[0], out->m_maxPelValue[0]);
        p3  = iClip(((p3 + out->m_maxPelValue[1]) >> 1), out->m_minPelValue[1], out->m_maxPelValue[1]);
        break;
      case CM_YFBFRV4:
        p3 -= p2; // p3 = R - B ; pixelV
        p2 += ((3 * p3) >> 2); // 3R/4 + B/4
        p1 -= p2; // G - 3R/4 - B/4
        p2 += ((p1 * 23) >> 5); // 23G/32 + 27R/128 + 9B/128 //.7,.225,.075
        p1  = iClip(((p1 + out->m_maxPelValue[2]) >> 1), out->m_minPelValue[2], out->m_maxPelValue[2]);
        p2  = iClip(p2, out->m_minPelValue[0], out->m_maxPelValue[0]);
        p3  = iClip(((p3 + out->m_maxPelValue[1]) >> 1), out->m_minPelValue[1], out->m_maxPelValue[1]);
        break;
      default:
        break;
    }
    if (out->m_bitDepth > 8) {
      out->m_ui16Comp[0][i] = p2; //Y
      out->m_ui16Comp[1][i] = p3; //Fb
      out->m_ui16Comp[2][i] = p1; //Fr
    }
    else {
      out->m_comp[0][i] = p2; //Y
      out->m_comp[1][i] = p3; //Fb
      out->m_comp[2][i] = p1; //Fr
    }
  }
}

void ColorTransformFVDO::inverse (Frame *out,  const Frame *inp)
{
  int p1, p2, p3;
  for (int i = 0; i < inp->m_compSize[0]; i++)  {
    if (inp->m_bitDepth > 8) {
      p3 = inp->m_ui16Comp[0][i]; // Y
      p1 = inp->m_ui16Comp[1][i] - (1 << (inp->m_bitDepthComp[1] - 1)); // Fb
      p2 = inp->m_ui16Comp[2][i] - (1 << (inp->m_bitDepthComp[2] - 1)); // Fr
    }
    else {
      p3 = inp->m_comp[0][i]; // Y
      p1 = inp->m_comp[1][i] - (1 << (inp->m_bitDepthComp[1] - 1)); // Fb
      p2 = inp->m_comp[2][i] - (1 << (inp->m_bitDepthComp[2] - 1)); // Fr
    }
    switch(inp->m_colorSpace) {
      case CM_YFBFRV1:
        p1  = p1 << 1;
        p2  = p2 << 1;
        p3 -= ((p2 * 3) >> 3); // Y - 3*Fr/8
        p2 += p3; // Y + 5*Fr/8
        p3 -= (p1 >> 1); // Y - 3*Fr/8 - Fb/2
        p1 += p3; // Y -3*Fr/8 + Fb/2
        p1  = iClip(p1, out->m_minPelValue[0], out->m_maxPelValue[0]); //Y -3*Fr/8 + Fb/2  ...1 , 3/8
        p3  = iClip(p3, out->m_minPelValue[2], out->m_maxPelValue[2]); //Y - 3*Fr/8 - Fb/2
        p2  = iClip(p2, out->m_minPelValue[1], out->m_maxPelValue[1]); //Y + 5*Fr/8
        break;
      case CM_YFBFRV2:
        p1  = p1 << 1;
        p2  = p2 << 1;
        p3 -= ((p2 * 5) >> 3); // Y - 5*Fr/8
        p2 += p3; // Y + 3*Fr/8
        p3 -= (p1 >> 1); // Y - 5*Fr/8 - Fb/2
        p1 += p3; // Y -5*Fr/8 + Fb/2
        p1  = iClip(p1, out->m_minPelValue[0], out->m_maxPelValue[0]); //Y -5*Fr/8 + Fb/2
        p3  = iClip(p3, out->m_minPelValue[2], out->m_maxPelValue[2]); //Y - 5*Fr/8 - Fb/2
        p2  = iClip(p2, out->m_minPelValue[1], out->m_maxPelValue[1]); //Y + 3*Fr/8
        break;
      case CM_YFBFRV3:
        p1  = p1 << 1;
        p2  = p2 << 1;
        p3 -= ((p2 * 23) >> 5); // Y - 23*Fr/32
        p2 += p3; // Y + 9*Fr/32
        p3 -= (p1 >> 1); // Y - 23*Fr/32 - Fb/2
        p1 += p3; // Y -23*Fr/32 + Fb/2
        p1  = iClip(p1, out->m_minPelValue[0], out->m_maxPelValue[0]); //Y -23*Fr/32 + Fb/2
        p3  = iClip(p3, out->m_minPelValue[2], out->m_maxPelValue[2]); //Y - 23*Fr/32 - Fb/2
        p2  = iClip(p2, out->m_minPelValue[1], out->m_maxPelValue[1]); //Y + 9*Fr/32
        break;
      case CM_YFBFRV4:
        p1  = p1 << 1;
        p2  = p2 << 1;
        p3 -= ((p2 * 23) >> 5); // Y - 23*Fr/32
        p2 += p3; // Y + 9*Fr/32
        p3 -= ((3 * p1) >> 2); //  // Y - 23Fr/32 - 3Fb/4
        p1 += p3; // Y - 23Fr/32 + Fb/4
        p1  = iClip(p1, out->m_minPelValue[0], out->m_maxPelValue[0]); //Y - 23Fr/32 + Fb/4
        p3  = iClip(p3, out->m_minPelValue[2], out->m_maxPelValue[2]); //Y - 23Fr/32 - 3Fb/4
        p2  = iClip(p2, out->m_minPelValue[1], out->m_maxPelValue[1]); //Y + 9Fr/32
        break;
      default:
        break;
    }
    if (out->m_bitDepth > 8) {
      out->m_ui16Comp[0][i] = p1; //R
      out->m_ui16Comp[1][i] = p2; //G
      out->m_ui16Comp[2][i] = p3; //B
    }
    else {
      out->m_comp[0][i] = p1; //R
      out->m_comp[1][i] = p2; //G
      out->m_comp[2][i] = p3; //B
    }
  }
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
void ColorTransformFVDO::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  if (m_forward)
    forward(out, inp);
  else
    inverse(out, inp);
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
