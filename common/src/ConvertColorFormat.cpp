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
 * \file ConvertColorFormat.cpp
 *
 * \brief
 *    Base Class for Color Format conversion
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
#include "ScaleFilter.H"
#include "ConvertColorFormat.H"
#include "ConvertColorFormatNull.H"
#include "Conv420to422Generic.H"
#include "Conv420to444NN.H"
#include "Conv420to444Adaptive.H"
#include "Conv420to444CrBounds.H"
#include "Conv420to444Generic.H"
#include "Conv422to444NN.H"
#include "Conv422to444Generic.H"
#include "Conv444to420NN.H"
#include "Conv444to422NN.H"
#include "Conv444to420BI.H"
#include "Conv444to420Generic.H"
#include "Conv444to420Adaptive.H"
#include "Conv444to420CrEdge.H"
#include "Conv444to420CrBounds.H"
#include "Conv444to420CrFBounds.H"
#include "Conv444to422Generic.H"
#include "Conv422to420Generic.H"
#include "Conv444to422GenericOld.H"

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
ConvertColorFormat *ConvertColorFormat::create(int width, int height, ChromaFormat iChromaFormat, ChromaFormat oChromaFormat, int method, ChromaLocation *iChromaLocationType, ChromaLocation *oChromaLocationType, int useAdaptiveFilter, int useMinMax) {
  ConvertColorFormat *result = NULL;

  if (iChromaFormat == oChromaFormat) { // Do nothing
    if ((iChromaFormat == CF_422 || iChromaFormat == CF_400) && iChromaLocationType[FP_FRAME] != oChromaLocationType[FP_FRAME]) {
      printf("Change in Chroma Sample Location type is currently not supported. \n");
      printf("Reverting to the input Chroma Sample Location type (%d).\n", iChromaLocationType[FP_FRAME]);
    }
    result = new ConvertColorFormatNull();
  }
  else if (iChromaFormat == CF_444 && oChromaFormat == CF_422) {// Downsample to 422 from 444
    switch (method) {
      case DF_NN: // Nearest Neighbor
        result = new Conv444to422NN();
        break;
      case DF_F0: // Generic
      case DF_F1:
#if 0
        result = new Conv444to422GenericOld(width, height, method);
#else
      case DF_TM:
      case DF_FV:
      case DF_GS:
      case DF_WCS:
      case DF_SVC:
      case DF_LZW:
      case DF_SNW:
      case DF_LZ2:
      case DF_LZ3:
      case DF_LZ4:
      case DF_SN2:
      case DF_SN3:
      case DF_SNW3:
      case DF_SNW7:
      case DF_SNW11:
      case DF_SNW15:
      case DF_SSW3:
      case DF_SSW5:
      case DF_SSW7:
      case DF_SSW11:
      case DF_SSW15:
        result = new Conv444to422Generic(width, height, method, oChromaLocationType, useMinMax);
#endif
        break;
      default:
        fprintf(stderr, "Not supported chroma downsampling method %d\n", method);
        exit(EXIT_FAILURE);
        break;
    }
  }
  else if (iChromaFormat == CF_444 && oChromaFormat == CF_420) {// Downsample
    switch (method) {
      case DF_NN: // Nearest Neighbor
        result = new Conv444to420NN();
        break;
      case DF_BI: // Bilinear
        result = new Conv444to420BI(width, height);
        break;
        // case DF_F0 ... (DF_TOTAL - 1): // Generic
      case DF_F0:
      case DF_F1:
      case DF_TM:
      case DF_FV:
      case DF_GS:
      case DF_WCS:
      case DF_SVC:
      case DF_LZW:
      case DF_SNW:
      case DF_LZ2:
      case DF_LZ3:
      case DF_LZ4:
      case DF_SN2:
      case DF_SN3:
      case DF_SNW3:
      case DF_SNW7:
      case DF_SNW11:
      case DF_SNW15:
      case DF_SSW3:
      case DF_SSW5:
      case DF_SSW7:
      case DF_SSW11:
      case DF_SSW15:
        if (useAdaptiveFilter == (int) ADF_MULTI)
          result = new Conv444to420Adaptive(width, height, method, oChromaLocationType);
        else if (useAdaptiveFilter == (int) ADF_CREDGE)
          result = new Conv444to420CrEdge(width, height, method, oChromaLocationType);
        else if (useAdaptiveFilter == (int) ADF_CRBOUNDS)
          result = new Conv444to420CrBounds(width, height, method, oChromaLocationType);        
        else if (useAdaptiveFilter == (int) ADF_CRFBOUNDS)
          result = new Conv444to420CrFBounds(width, height, method, oChromaLocationType);        
        else      
          result = new Conv444to420Generic (width, height, method, oChromaLocationType, useMinMax);
        break;
      default:
        fprintf(stderr, "Not supported chroma downsampling method %d\n", method);
        exit(EXIT_FAILURE);
        break;
    }
  }
  else if (iChromaFormat == CF_422 && oChromaFormat == CF_420) {// Downsample
    if (iChromaLocationType[FP_FRAME] != oChromaLocationType[FP_FRAME]) {
      printf("Change in Chroma Sample Location type is currently not supported. \n");
      printf("Reverting to the input Chroma Sample Location type (%d).\n", iChromaLocationType[FP_FRAME]);
    }

    switch (method) {
      default:
      case 0:
        result = new Conv422to420Generic(width, height, method);
        break;
    }
  }
  else if (iChromaFormat == CF_420 && oChromaFormat == CF_422) {// Upsample 420 to 422
    if (iChromaLocationType[FP_FRAME] != oChromaLocationType[FP_FRAME]) {
      printf("Change in Chroma Sample Location type is currently not supported. \n");
      printf("Reverting to the input Chroma Sample Location type (%d).\n", iChromaLocationType[FP_FRAME]);
    }

    switch (method) {
      default:
      case 0:
        result = new Conv420to422Generic(width, height, method);
        break;
    }
  }
  else if (iChromaFormat == CF_420 && oChromaFormat == CF_444) { // Upsample 420 to 444
    switch (method) {
      case UF_NN: // Nearest Neighbor
        result = new Conv420to444NN();
        break;
      case UF_F0:
      case UF_FV:
      case UF_GS:
      case UF_LS3:
      case UF_LS4:
      case UF_LS5:
      case UF_LS6:
      case UF_TM:
        if (useAdaptiveFilter == (int) ADF_NULL)
          result = new Conv420to444Generic(width, height, method, iChromaLocationType);
        else if (useAdaptiveFilter == (int) ADF_MULTI)
          result = new Conv420to444Adaptive(width, height, method, iChromaLocationType);
        else //if (useAdaptiveFilter == (int) ADF_CRBOUNDS) {
          result = new Conv420to444CrBounds(width, height, method, iChromaLocationType);
        break;
      default:
        fprintf(stderr, "Not supported chroma upsampling method %d\n", method);
        exit(EXIT_FAILURE);
        break;
    }
  }
  else if (iChromaFormat == CF_422 && oChromaFormat == CF_444) { // Upsample 422 to 444
    switch (method) {
      case UF_NN: // Nearest Neighbor
        result = new Conv422to444NN();
        break;
      case UF_F0:
      case UF_FV:
      case UF_GS:
      case UF_LS3:
      case UF_LS4:
      case UF_LS5:
      case UF_LS6:
      case UF_TM:
          result = new Conv422to444Generic(width, height, method, iChromaLocationType);
        break;
      default:
        fprintf(stderr, "Not supported chroma upsampling method %d\n", method);
        exit(EXIT_FAILURE);
        break;
    }
  }
  
  return result;
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
