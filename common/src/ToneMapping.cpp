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
 * \file ToneMapping.cpp
 *
 * \brief
 *    Base Class for Tone Mapping
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
#include "ToneMapping.H"
#include "ToneMappingNull.H"
#include "ToneMappingRoll.H"
#include "ToneMappingBT2390.H"
#include "ToneMappingBT2390IPT.H"
#include "ToneMappingCIE1931.H"
#include "ToneMappingCIE1976.H"

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
ToneMapping *ToneMapping::create(int method, ToneMappingParams *tmParams) {
  ToneMapping *result = NULL;
    
  switch (method){
    case TM_NULL:
      result = new ToneMappingNull();
      break;
    case TM_ROLL:
      result = new ToneMappingRoll( tmParams->m_minValue, tmParams->m_maxValue, tmParams->m_targetValue, tmParams->m_gamma);
      break;
    case TM_BT2390:
      result = new ToneMappingBT2390( tmParams->m_minValue, tmParams->m_maxValue, tmParams->m_targetValue, tmParams->m_scaleGammut);
      break;
    case TM_BT2390IPT:
      result = new ToneMappingBT2390IPT( tmParams->m_minValue, tmParams->m_maxValue, tmParams->m_targetValue, tmParams->m_scaleGammut);
      break;
    case TM_CIE1931:
      result = new ToneMappingCIE1931( tmParams->m_minValue, tmParams->m_maxValue, tmParams->m_targetValue, tmParams->m_scaleGammut);
      break;
    case TM_CIE1976:
      result = new ToneMappingCIE1976( tmParams->m_minValue, tmParams->m_maxValue, tmParams->m_targetValue, tmParams->m_scaleGammut);
      break;
    default:
      fprintf(stderr, "\nUnsupported Tone Mapping Function %d\n", method);
      exit(EXIT_FAILURE);
  }
  
  
  return result;
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
