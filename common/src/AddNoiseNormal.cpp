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
 * \file AddNoiseNormal.cpp
 *
 * \brief
 *    AddNoiseNormal Class
 *    Code for this function was taken from wikipedia, i.e.
 *    http://en.wikipedia.org/wiki/Box-Muller_transform
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
#include "AddNoiseNormal.H"

#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

AddNoiseNormal::AddNoiseNormal(double variance, double mean) {
  m_variance  = variance;
  m_mean      = mean;
  m_haveSpare = FALSE;
	m_rand1     = 0.0;
  m_rand2     = 0.0;
  
  // init noise
  srand (0);

}

AddNoiseNormal::~AddNoiseNormal() {
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

double AddNoiseNormal::generateGaussianNoise(const double &variance, const double mean)
{

	if(m_haveSpare)	{
		m_haveSpare = false;
		return (sqrt(variance * m_rand1) * sin(m_rand2) + mean) / 100.0;
	}
  
	m_haveSpare = true;
  
	m_rand1 = rand() / ((double) RAND_MAX);
	if (m_rand1 < 1e-100)
    m_rand1 = 1e-100;
	m_rand1 = -2 * log(m_rand1);
	m_rand2 = (rand() / ((double) RAND_MAX)) * TWO_PI;
  
  //  printf("noise value %10.7f %10.7f  %10.7f\n", variance, mean, sqrt(variance * m_rand1) * cos(m_rand2) + mean);
	return (sqrt(variance * m_rand1) * cos(m_rand2) + mean) / 100.0;
}

void AddNoiseNormal::addNoiseData (const uint16 *iData, uint16 *oData, int size, int maxSampleValue)
{
  for (int i = 0; i < size; i++) {
    *oData++ = iClip(*iData++ + (uint16) generateGaussianNoise(m_variance, m_mean), 0, maxSampleValue);
  }
}

void AddNoiseNormal::addNoiseData (const imgpel *iData, imgpel *oData, int size, int maxSampleValue)
{
  for (int i = 0; i < size; i++) {
    *oData++ = iClip(*iData++ + (imgpel) generateGaussianNoise(m_variance, m_mean), 0, maxSampleValue);
  }
}

void AddNoiseNormal::addNoiseData (const float *iData, float *oData, int size, double maxSampleValue)
{
  for (int i = 0; i < size; i++) {
    double noise = generateGaussianNoise(m_variance, m_mean);
    //printf("noise value %10.7f\n", noise);
    *oData++ = (float) dClip((double) *iData++ + noise, 0.0, maxSampleValue);
  }
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void AddNoiseNormal::process ( Frame* out, Frame *inp)
{
  out->m_frameNo = inp->m_frameNo;
  out->m_isAvailable = TRUE;
  
  if (inp->equalType(out)) {
    if (inp->m_isFloat == TRUE) {
      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_floatComp[c], out->m_floatComp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
    else if (inp->m_bitDepth > 8) {
      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_ui16Comp[c], out->m_ui16Comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
    else { // 8 bit data
      for (int c = Y_COMP; c < inp->m_noComponents; c++)
        addNoiseData (inp->m_comp[c], out->m_comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
    }
  }
  else {
    printf("AddNoiseNormal::Output frame buffer of different type than input frame buffer. Check your implementation\n");
  }
}

void AddNoiseNormal::process ( Frame *inp)
{
  if (inp->m_isFloat == TRUE) {
    for (int c = Y_COMP; c < inp->m_noComponents; c++)
      addNoiseData (inp->m_floatComp[c], inp->m_floatComp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
  }
  else if (inp->m_bitDepth > 8) {
    for (int c = Y_COMP; c < inp->m_noComponents; c++)
      addNoiseData (inp->m_ui16Comp[c], inp->m_ui16Comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
  }
  else { // 8 bit data
    for (int c = Y_COMP; c < inp->m_noComponents; c++)
      addNoiseData (inp->m_comp[c], inp->m_comp[c], inp->m_compSize[c], inp->m_maxPelValue[c]);
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
