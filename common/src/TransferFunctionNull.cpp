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
 * \file TransferFunctionNull.cpp
 *
 * \brief
 *    TransferFunctionNull Class
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
#include "TransferFunctionNull.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

TransferFunctionNull::TransferFunctionNull() {
  m_enableLUT = FALSE;
}

TransferFunctionNull::~TransferFunctionNull() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
double TransferFunctionNull::forward(double value) {
  return value;
}

double TransferFunctionNull::inverse(double value) {
  return value;
}
/*

void TransferFunctionNull::forward ( Frame* out, const Frame *inp, int component) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
    for (int i = 0; i < inp->m_compSize[component]; i++) {
      out->m_floatComp[component][i] = inp->m_floatComp[component][i];
    }
  }
}

void TransferFunctionNull::forward ( Frame* out, const Frame *inp) {
  if (out == NULL) {
    // should only be done for pointers. This should help speeding up the code and reducing memory.
    out = (Frame *)inp;
  }
  else {
    out->m_frameNo = inp->m_frameNo;
    out->m_isAvailable = TRUE;
    out->copy((Frame *) inp);
  }
}

void TransferFunctionNull::inverse ( Frame* out, const Frame *inp, int component) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
    for (int i = 0; i < inp->m_compSize[component]; i++) {
      out->m_floatComp[component][i] = inp->m_floatComp[component][i];
    }
  }
}

void TransferFunctionNull::inverse ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  out->copy((Frame *) inp);
    for (int i = 0; i < inp->m_size; i++) {
          if (inp->m_floatData[i] > 1.0)
          printf("position %d %10.7f %10.7f\n", i, inp->m_floatData[i], out->m_floatData[i]);
          }

}
*/


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
