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
 * \file LookUpTable.cpp
 *
 * \brief
 *    Generic Look Up Table Implementation
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
#include "LookUpTable.H"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
LookUpTable::LookUpTable(bool enable, int bins, int elements, double (*pFunction) (double)) {
  m_enable = enable;
  init(bins, elements, pFunction);
}

LookUpTable::LookUpTable(bool enable, int bins, int elements) {
  m_enable = enable;
  init(bins, elements);
}

LookUpTable::~LookUpTable() {
  m_enable = FALSE;
  m_bins = 0;
}


//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
void LookUpTable::init(int bins, int elements, double (*pFunction) (double)){
  if (m_enable == FALSE || bins < 0 || elements < 0) {
    m_bins = 0;
  }
  else {
    //printf("Initializing LUT\n");
    uint32 i, j;
    m_bins = bins;
    m_binsExt = m_bins + 2;
    m_elements.resize    (m_binsExt);
    m_multiplier.resize  (m_binsExt);
    m_bound.resize       (m_binsExt + 1);
    m_data.resize        (m_binsExt);
    
    m_bound[0] = 0.0;
    for (i = 0; i < m_binsExt; i++) {
      m_elements[i] = elements; // Size of each bin.
                                                      // Could be different for each bin, but for now lets set this to be the same.
      m_bound[i + 1] = 1 / pow( 10.0 , (double) (m_bins - i - 1)); // upper bin boundary
      double stepSize = (m_bound[i + 1] -  m_bound[i]) / (m_elements[i] - 1);
      m_multiplier[i] = (double) (m_elements[i] - 1) / (m_bound[i + 1] -  m_bound[i]);
      
      
      // Allocate also memory for the derivative LUT
      m_data[i].resize(m_elements[i]);
      for (j = 0; j < m_elements[i] ; j++) {
        double curValue = m_bound[i] + (double) j * stepSize;
        m_data[i][j] = (*pFunction)(curValue);
      }
    }
  }
}

void LookUpTable::init(int bins, int elements){
  if (m_enable == FALSE || bins < 0 || elements < 0) {
    m_bins = 0;
  }
  else {
    uint32 i, j;
    m_bins = bins;
    m_binsExt = m_bins + 2;
    m_elements.resize    (m_binsExt);
    m_multiplier.resize  (m_binsExt);
    m_bound.resize       (m_binsExt + 1);
    m_data.resize  (m_binsExt);
    
    m_bound[0] = 0.0;
    for (i = 0; i < m_binsExt; i++) {
      m_elements[i] = elements; // Size of each bin.
                                                      // Could be different for each bin, but for now lets set this to be the same.
      m_bound[i + 1] = 1 / pow( 10.0 , (double) (m_bins - i - 1)); // upper bin boundary
      //double stepSize = (m_bound[i + 1] -  m_bound[i]) / (m_elements[i] - 1);
      m_multiplier[i] = (double) (m_elements[i] - 1) / (m_bound[i + 1] -  m_bound[i]);


      
      // Allocate also memory for the derivative LUT
      m_data[i].resize(m_elements[i]);
      // initialize
      for (j = 0; j < m_elements[i] ; j++) {
        //double curValue = m_bound[i] + (double) j * stepSize;
        m_data[i][j] = 0.0; //(*pFunction)(curValue);
      }
    }
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
double LookUpTable::compute(double value) {
  if (m_bins == 0)
    return 0.0;
  else if (value <= 0.0)
    return m_data[0][0];
  else if (value >= 1.0  ) {
    // top value, most likely 1.0
    return m_data[m_bins - 1][m_elements[m_bins - 1] - 1];
  }
  else {
    // now search for value in the table
    for (uint32 i = 0; i < m_bins; i++) {
      if (value < m_bound[i + 1]) { // value located
        double satValue     = (value - m_bound[i]) * m_multiplier[i];
        int    valuePlus    = (int) dCeil(satValue) ;
        double distancePlus = (double) valuePlus - satValue;
        
        return (m_data[i][valuePlus - 1] * distancePlus + m_data[i][valuePlus] * (1.0 - distancePlus));
      }
    }
  }
  return 0.0;
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
