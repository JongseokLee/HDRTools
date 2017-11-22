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
 * \file ToneMappingRoll.cpp
 *
 * \brief
 *    ToneMappingRoll Class
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
#include "ToneMappingRoll.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ToneMappingRoll::ToneMappingRoll(double minValue, double maxValue, double targetValue, double gamma) {
  m_minValue    = minValue;
  m_maxValue    = maxValue;
  m_targetValue = targetValue;
  m_gamma       = 1.0 / gamma;
  m_range       = maxValue - minValue;
  m_mappedRange = m_targetValue - minValue;
}

ToneMappingRoll::~ToneMappingRoll() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void ToneMappingRoll::process ( Frame* frame) {
  if (frame->m_isFloat == TRUE ) {
    double value;
    for (int i = 0; i < frame->m_size; i++) {
      value = (double) frame->m_floatData[i];
      if (value > m_minValue) {
        frame->m_floatData[i] = (float) (pow(((value - m_minValue) / m_range), m_gamma) * m_mappedRange + m_minValue);
      }
    }
  }
}

void ToneMappingRoll::process ( Frame* out, const Frame *inp) {
  if (out == NULL) {
    // should only be done for pointers. This should help speeding up the code and reducing memory.
    out = (Frame *)inp;
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      double value;
      for (int i = 0; i < inp->m_size; i++) {
        value = (double) inp->m_floatData[i];
        if (value > m_minValue) {
          out->m_floatData[i] = (float) (pow(((value - m_minValue) / m_range), m_gamma) * m_mappedRange + m_minValue);
        }
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
