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
 * \file ChromaConvertYUV.H
 *
 * \brief
 *    ChromaConvertYUV class (main) definition for converting YUV files
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __ChromaConvertYUV_H__
#define __ChromaConvertYUV_H__

#include "Global.H"
#include "ChromaConvert.H"
#include "Input.H"
#include "Output.H"
#include "Frame.H"
#include "IOFunctions.H"
#include "Convert.H"
#include "ConvertColorFormat.H"
#include "ColorTransform.H"
#include "TransferFunction.H"


class ChromaConvertYUV : public ChromaConvert {
private:
  Input              *m_inputFrame;             // input frames
  Output             *m_outputFrame;            // output frames
  Frame              *m_oFrameStore;            // picture storage for output frames
  Frame              *m_pFrameStore;            // picture storage for intermediate processed frames
  Frame              *m_iFrameStore;            // picture storage for input frames
  
  ConvertColorFormat *m_convertFormat;          // Chroma format conversion
  Convert            *m_convertProcess;         // Bitdepth Conversion process

  
  int                 m_startFrame;
public:
  ChromaConvertYUV(ProjectParameters *inputParams);
  //virtual ~ChromaConvertYUV();
  
  virtual void outputHeader (ProjectParameters *inputParams);
  virtual void outputFooter (ProjectParameters *inputParams);
  virtual void init         (ProjectParameters *inputParams);
  virtual void process      (ProjectParameters *inputParams);
  virtual void destroy();
  
};

#endif
