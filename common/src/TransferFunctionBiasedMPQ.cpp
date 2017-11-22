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
 * \file TransferFunctionBiasedMPQ.cpp
 *
 * \brief
 *    TransferFunctionBiasedMPQ Class
 *    Note that this class gives a negative bias to Blue (cause of noise)
 *    An effect on this is that the gray value now also gets shifted.
 *    Is that of importance? I do not know.
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
#include "TransferFunctionBiasedMPQ.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

TransferFunctionBiasedMPQ::TransferFunctionBiasedMPQ(float inflectionPoint, float scale) {
  m1 = (2610.0        ) / (4096.0 * 4.0);
  m2 = (2523.0 * 128.0) / 4096.0;
  c1 = (3424.0        ) / 4096.0;
  c2 = (2413.0 *  32.0) / 4096.0;
  c3 = (2392.0 *  32.0) / 4096.0;
  
  m_normalFactor        = 1.0;
  m_invNormalFactor     = 1.0;
  m_inflectionPoint     = (double) inflectionPoint;
  m_invInflectionPoint  = inversePQ(m_inflectionPoint);
  m_scale               = scale;
  m_invInflectionPointDiv2 = m_invInflectionPoint / m_scale;
  
}

TransferFunctionBiasedMPQ::~TransferFunctionBiasedMPQ() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
double TransferFunctionBiasedMPQ::forwardPQ(double value) {
  double tempValue = pow(value, (1.0 / m2));
  return (pow(dMax(0.0, (tempValue - c1))/(c2 - c3 * tempValue),(1.0 / m1)));
}

double TransferFunctionBiasedMPQ::inversePQ(double value) {
  double tempValue = pow((double) value, m1);
  return (float) (pow(((c2 *(tempValue) + c1)/(1.0 + c3 *(tempValue))), m2));
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


double TransferFunctionBiasedMPQ::forward(double value) {
  if (value <= m_invInflectionPointDiv2) {
    double tempValue = pow(value * m_scale, (1.0 / m2));
    return(float) (pow(dMax(0.0, (tempValue - c1))/(c2 - c3 * tempValue),(1.0 / m1)));
  }
  else {
    double tempValue = pow((((value -  m_invInflectionPointDiv2) * (1.0 - m_invInflectionPoint))/(1.0 - m_invInflectionPointDiv2) +  m_invInflectionPoint ), (1.0 / m2));
    return (float) (pow(dMax(0.0, (tempValue - c1))/(c2 - c3 * tempValue),(1.0 / m1)));
  }
}

double TransferFunctionBiasedMPQ::inverse(double value) {
  double tempValue0 = pow((double) value, m1);
  double tempValue1 = (pow(((c2 *(tempValue0) + c1)/(1.0 + c3 *(tempValue0))), m2));
  if (value <= m_inflectionPoint) {
    return (float) tempValue1 / m_scale;
  }
  else {
    return (float) ((((tempValue1 - m_invInflectionPoint) * (1.0 - m_invInflectionPointDiv2)) / (1.0 - m_invInflectionPoint)) + m_invInflectionPointDiv2);
  }
}



void TransferFunctionBiasedMPQ::forward( Frame* out, const Frame *inp, int component ) {
  // In this scenario, we should likely copy the frame number externally
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      if (component == B_COMP) {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) forward((double) inp->m_floatComp[component][i]);
        }
      }
      else {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) forwardPQ((double) inp->m_floatComp[component][i]);
        }
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp, component);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      if (component == B_COMP) {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) (m_normalFactor * forward((double) inp->m_floatComp[component][i]));
        }
      }
      else {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) (m_normalFactor * forwardPQ((double) inp->m_floatComp[component][i]));
        }        
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp, component);
    }
  }
}

void TransferFunctionBiasedMPQ::forward( Frame* out, const Frame *inp ) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      for (int i = 0; i < inp->m_compSize[R_COMP]; i++) {
        out->m_floatComp[R_COMP][i] = (float) forward((double) inp->m_floatComp[R_COMP][i]);
      }
      for (int i = 0; i < inp->m_compSize[G_COMP]; i++) {
        out->m_floatComp[G_COMP][i] = (float) forward((double) inp->m_floatComp[G_COMP][i]);
      }
      for (int i = 0; i < inp->m_compSize[B_COMP]; i++) {
        out->m_floatComp[B_COMP][i] = (float) forwardPQ((double) inp->m_floatComp[B_COMP][i]);
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      for (int i = 0; i < inp->m_compSize[R_COMP]; i++) {
        out->m_floatComp[R_COMP][i] = (float) (m_normalFactor * forwardPQ((double) inp->m_floatComp[R_COMP][i]));
      }
      for (int i = 0; i < inp->m_compSize[G_COMP]; i++) {
        out->m_floatComp[G_COMP][i] = (float) (m_normalFactor * forwardPQ((double) inp->m_floatComp[G_COMP][i]));
      }
      for (int i = 0; i < inp->m_compSize[B_COMP]; i++) {
        out->m_floatComp[B_COMP][i] = (float) (m_normalFactor * forward((double) inp->m_floatComp[B_COMP][i]));
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
}

void TransferFunctionBiasedMPQ::inverse( Frame* out, const Frame *inp, int component ) {
  // In this scenario, we should likely copy the frame number externally
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      if (component == B_COMP) {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) inverse((double) inp->m_floatComp[component][i]);
        }
      }
      else {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) inversePQ((double) inp->m_floatComp[component][i]);
        }
      }      
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp, component);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_compSize[component] == out->m_compSize[component]) {
      if (component == B_COMP) {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) inverse(dMax(0.0, dMin((double) inp->m_floatComp[component][i]/m_normalFactor, 1.0)));
        }
      }
      else {
        for (int i = 0; i < inp->m_compSize[component]; i++) {
          out->m_floatComp[component][i] = (float) inversePQ(dMax(0.0, dMin((double) inp->m_floatComp[component][i]/m_normalFactor, 1.0)));
        }        
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp, component);
    }
  }
}

void TransferFunctionBiasedMPQ::inverse( Frame* out, const Frame *inp ) {
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  if (m_normalFactor == 1.0) {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      for (int i = 0; i < inp->m_compSize[R_COMP]; i++) {
        out->m_floatComp[R_COMP][i] = (float) inverse((double) inp->m_floatComp[R_COMP][i]);
      }
      for (int i = 0; i < inp->m_compSize[G_COMP]; i++) {
        out->m_floatComp[G_COMP][i] = (float) inverse((double) inp->m_floatComp[G_COMP][i]);
      }
      for (int i = 0; i < inp->m_compSize[B_COMP]; i++) {
        out->m_floatComp[B_COMP][i] = (float) inversePQ((double) inp->m_floatComp[B_COMP][i]);
      }
    }
    else if (inp->m_isFloat == FALSE && out->m_isFloat == FALSE && inp->m_size == out->m_size && inp->m_bitDepth == out->m_bitDepth) {
      out->copy((Frame *) inp);
    }
  }
  else {
    if (inp->m_isFloat == TRUE && out->m_isFloat == TRUE && inp->m_size == out->m_size) {
      for (int i = 0; i < inp->m_compSize[R_COMP]; i++) {
        out->m_floatComp[R_COMP][i] = (float) inverse(dMax(0.0, dMin((double) inp->m_floatComp[R_COMP][i]/m_normalFactor, 1.0)));
      }
      for (int i = 0; i < inp->m_compSize[G_COMP]; i++) {
        out->m_floatComp[G_COMP][i] = (float) inverse(dMax(0.0, dMin((double) inp->m_floatComp[G_COMP][i]/m_normalFactor, 1.0)));
      }
      for (int i = 0; i < inp->m_compSize[R_COMP]; i++) {
        out->m_floatComp[B_COMP][i] = (float) inversePQ(dMax(0.0, dMin((double) inp->m_floatComp[B_COMP][i]/m_normalFactor, 1.0)));
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
