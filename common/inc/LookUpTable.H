/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Apple Inc.
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
 * \file LookUpTable.H
 *
 * \brief
 *    LookUpTable class Header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#ifndef __LookUpTable_H__
#define __LookUpTable_H__

#include "Global.H"
#include "Frame.H"
#include <functional>
#include <ostream>

class LookUpTable {
private:
  // LUT related
  bool                    m_enable;
  uint32                  m_bins;
  uint32                  m_binsExt;

  void init(int bins, int elements, double (*pFunction) (double));
  void init(int bins, int elements);
  
public:
  // setters
  void   setBins(uint32 uiValue) { m_bins = uiValue; }
  uint32 getBins()    const      { return m_bins;    }

  vector<uint32>          m_elements;
  vector<double>          m_bound;
  vector<double>          m_multiplier;
  vector<vector<double> > m_data;
  
  // Note that we could also have used the std:function class but it seems that
  // maybe we would create some portability issues then. So we are sticking
  // with a function pointer
  LookUpTable(bool enable, int bins, int elements, double (*pFunction) (double));
  LookUpTable(bool enable, int bins, int elements);
  ~LookUpTable();
  
  double getLUTValue(double value);
  double compute    (double value);
};

#endif
