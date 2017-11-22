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
 * \file ConvertBitDepth.cpp
 *
 * \brief
 *    ConvertBitDepth Class
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
#include "ConvertBitDepth.H"

#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ConvertBitDepth::ConvertBitDepth() {
}

ConvertBitDepth::~ConvertBitDepth() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void ConvertBitDepth::convert (const uint16 *iData, uint16 *oData, int size, int bitShift) {
  int i;
  if (bitShift < 0) {
  for (i = 0; i < size; i++) {
    *oData++ = uiShiftRightRound(*iData++, -bitShift);
  }
  }
  else if (bitShift > 0){
    for (i = 0; i < size; i++) {
      *oData++ = *iData++ << bitShift;
    }
  }
  else
    memcpy(oData, iData, size * sizeof(uint16));
}

void ConvertBitDepth::convert (const imgpel *iData, uint16 *oData, int size, int bitShift) {
  int i;
  for (i = 0; i < size; i++) {
    *oData++ = *iData++ << bitShift;
  }
}

void ConvertBitDepth::convert (const uint16 *iData, imgpel *oData, int size, int bitShift) {
  int i;
  for (i = 0; i < size; i++) {
    *oData++ = (imgpel) uiShiftRightRound(*iData++, -bitShift);
  }
}

void ConvertBitDepth::convert (const imgpel *iData, imgpel *oData, int size) {
  memcpy(oData, iData, size * sizeof(imgpel));
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void ConvertBitDepth::process ( Frame* out, const Frame *inp) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  if ( inp->m_bitDepth > 8 ) {
    if ( out->m_bitDepth > 8) // both output and input use more than 8 bits
      convert (&inp->m_ui16Data[0], &out->m_ui16Data[0], (int) inp->m_size, out->m_bitDepth - inp->m_bitDepth);
    else  // output is in 8 bits
      convert (&inp->m_ui16Data[0], &out->m_data[0], (int) inp->m_size, out->m_bitDepth - inp->m_bitDepth);
  }
  else {
    if (out->m_bitDepth > 8) // output needs more than 8 bits
      convert (&inp->m_data[0], &out->m_ui16Data[0], (int) inp->m_size, out->m_bitDepth - inp->m_bitDepth);
    else // both are 8 bit data and thus there is no conversion
      convert (&inp->m_data[0], &out->m_data[0], (int) inp->m_size);
  }
}



//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
