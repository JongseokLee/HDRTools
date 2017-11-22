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
 * \file DisplayGammaAdjust.cpp
 *
 * \brief
 *    Display Gamma Adjustment Base Class
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
#include "DisplayGammaAdjust.H"
#include "DisplayGammaAdjustNull.H"
#include "DisplayGammaAdjustHLG.H"

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
DisplayGammaAdjust *DisplayGammaAdjust::create(int method, float scale, float systemGamma) {
  DisplayGammaAdjust *result = NULL;
    
  switch (method){
    case DA_NULL:
      result = new DisplayGammaAdjustNull();
      break;
    case DA_HLG:
      result = new DisplayGammaAdjustHLG(systemGamma, scale);
      break;
    default:
      fprintf(stderr, "\nUnsupported Display Gamma Adjustment Process value (%d)\n", method);
      exit(EXIT_FAILURE);
  }
  
  
  return result;
}


void DisplayGammaAdjust::setup(Frame *frame) {
};

void DisplayGammaAdjust::inverse(double &comp0, double &comp1, double &comp2) {
}

void DisplayGammaAdjust::forward(double &comp0, double &comp1, double &comp2) {
}

void DisplayGammaAdjust::forward(const double iComp0, const double iComp1, const double iComp2,  double *oComp0, double *oComp1, double *oComp2) {
  *oComp0 = iComp0;
  *oComp1 = iComp1;
  *oComp2 = iComp2;
};
void DisplayGammaAdjust::inverse(const double iComp0, const double iComp1, const double iComp2,  double *oComp0, double *oComp1, double *oComp2) {
  *oComp0 = iComp0;
  *oComp1 = iComp1;
  *oComp2 = iComp2;
};


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
