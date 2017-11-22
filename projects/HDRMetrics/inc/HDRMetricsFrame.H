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
 * \file HDRMetricsFrame.H
 *
 * \brief
 *    HDRMetricsFrame class (main) definition for computing distortion in HDR files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __HDRMetricsFrame_H__
#define __HDRMetricsFrame_H__

#include "Global.H"
#include "HDRMetrics.H"
#include "Input.H"
#include "Output.H"
#include "Frame.H"
#include "IOFunctions.H"
#include "DistortionMetric.H"


class HDRMetricsFrame : public HDRMetrics {
private:
  bool                   m_enableMetric[DIST_METRICS];
  bool                   m_enableWindow;
  bool                   m_enableWindowMetric[DIST_METRICS];
  DistortionMetric     **m_distortionMetric;     // distortion array
  DistortionMetric     **m_windowDistortionMetric; // window distortion array
  DistortionParameters   m_distortionParameters;
  int                    m_numberOfClips;
  int                   *m_startFrame;
  
  Input                **m_inputFrame;           // input frames
  Frame                **m_frameStore;           // picture storage for input frames
  Frame                **m_cropFrameStore;       // picture storage for cropped input frames
  Frame                **m_windowFrameStore;     // windowed frame store
  
  // Cropping parameters
  int                   *m_cropOffsetLeft;
  int                   *m_cropOffsetTop;
  int                   *m_cropOffsetRight;
  int                   *m_cropOffsetBottom;

  // Window parameters
  int                    m_windowMinPosX;
  int                    m_windowMaxPosX;
  int                    m_windowMinPosY;
  int                    m_windowMaxPosY;
  int                    m_windowWidth;
  int                    m_windowHeight;
  
   void                 allocateFrameStores(Input *inputFrame, Frame **frameStore);
public:
  HDRMetricsFrame                          (ProjectParameters *inputParams);
  virtual ~HDRMetricsFrame                 ();
  
  virtual void outputHeader                (ProjectParameters *inputParams);
  virtual void outputFooter                (ProjectParameters *inputParams);
  virtual void init                        (ProjectParameters *inputParams);
  virtual void process                     (ProjectParameters *inputParams);
  virtual void destroy                     ();
  
};

#endif
