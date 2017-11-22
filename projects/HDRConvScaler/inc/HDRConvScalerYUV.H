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
 * \file HDRConvScalerYUV.H
 *
 * \brief
 *    HDRConvScalerYUV class (main) definition for converting YUV files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __HDRConvScalerYUV_H__
#define __HDRConvScalerYUV_H__

#include "Global.H"
#include "HDRConvScaler.H"
#include "Input.H"
#include "Output.H"
#include "Frame.H"
#include "IOFunctions.H"
#include "Convert.H"
#include "ConvertColorFormat.H"
#include "ColorTransform.H"
#include "TransferFunction.H"
#include "FrameFilter.H"
#include "AddNoise.H"
#include "FrameScale.H"


class HDRConvScalerYUV : public HDRConvScaler {
private:
  bool                  m_useSingleTransferStep;
  int                   m_filterInFloat;
  int                   m_startFrame;
  int                   m_noOfFrameStores;

  Input                *m_inputFrame;             // input frames
  Output               *m_outputFrame;            // output frames
  Frame                *m_oFrameStore;            // picture storage for output frames
  Frame                *m_iFrameStore;            // picture storage for input frames
  
  Frame                *m_pFrameStore[7];         // picture storage for processing frames
  Frame                *m_convertFrameStore;      // pixel type conversion frame store
  Frame                *m_croppedFrameStore;      // cropped frame store
  Frame                *m_colorSpaceFrame;
  Frame                *m_scaledFrame;
  FrameFilter          *m_frameFilter;

  FrameFilter          *m_frameFilterNoise0;
  FrameFilter          *m_frameFilterNoise1;
  
  Convert              *m_convertProcess;         // Final Conversion process
  Convert              *m_convertIQuantize;       // Initial Conversion process
  ColorTransform       *m_colorTransform;         // Color space conversion
  ColorTransform       *m_colorSpaceConvert;      // Color space conversion (second)

  ConvertColorFormat   *m_convertFormat;          // Chroma format conversion
  
  AddNoise             *m_addNoise;                 // Noise Addition
  FrameScale           *m_chromaScale;              // Frame downscale for linear chroma conversion
  
  FrameScale           *m_frameScale;                // Image rescaling

  TransferFunction     *m_normalizeFunction;        // Data normalization for OpenEXR inputs with linear light data
  TransferFunction     *m_outputTransferFunction;   // Output Transfer function
  TransferFunction     *m_inputTransferFunction;    // Input Transfer function
    
  // Cropping information
  int                   m_cropOffsetLeft;
  int                   m_cropOffsetTop;
  int                   m_cropOffsetRight;
  int                   m_cropOffsetBottom;
  
  bool                  m_changeColorPrimaries;
  bool                  m_bUseChromaDeblocking;
  bool                  m_bUseWienerFiltering;
  bool                  m_bUse2DSepFiltering;
  bool                  m_b2DSepMode;

  void                  deleteMemory();

public:
  HDRConvScalerYUV(ProjectParameters *inputParams);
  
  virtual void outputHeader (ProjectParameters *inputParams);
  virtual void outputFooter (ProjectParameters *inputParams);
  virtual void init         (ProjectParameters *inputParams);
  virtual void process      (ProjectParameters *inputParams);
  virtual void destroy();  
};

#endif
