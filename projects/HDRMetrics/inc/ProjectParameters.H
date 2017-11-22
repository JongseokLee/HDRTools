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
 * \file ProjectParameters.H
 *
 * \brief
 *    ProjectParameters class (main) definition for HDRMetrics
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __ProjectParameters_H__
#define __ProjectParameters_H__

#include "Global.H"
#include "Parameters.H"
#include "DistortionMetric.H"

#define DEFAULTCONFIGFILENAME "HDRMetrics.cfg"

class ProjectParameters : public Parameters {
private:
public:
  ProjectParameters();
  /*input*/
  int                  m_numberOfClips;
  IOVideo              m_inputFile[2];
  FrameFormat          m_source[2];
  int                  m_frameSkip[2];     //! Frame skipping for input
  double               m_maxSampleValue;
  bool                 m_enableMetric[DIST_METRICS];
  bool                 m_enableWindowMetric[DIST_METRICS];
  bool                 m_enableWindow;
  int                  m_windowMinPosX;
  int                  m_windowMaxPosX;
  int                  m_windowMinPosY;
  int                  m_windowMaxPosY;
  // Cropping parameters
  int                  m_cropOffsetLeft  [2];
  int                  m_cropOffsetTop   [2];
  int                  m_cropOffsetRight [2];
  int                  m_cropOffsetBottom[2];

  DistortionParameters m_distortionParameters;
  
  virtual void update();
  virtual void refresh();
};

extern ProjectParameters ccParams;

#endif
