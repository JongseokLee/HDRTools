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
 * \file HDRConvScalerYUV.cpp
 *
 * \brief
 *    HDRConvScalerYUV class source files for performing HDR (YUV) format conversion
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

#include <time.h>
#include <string.h>
#include <math.h>
#include "HDRConvScalerYUV.H"


HDRConvScalerYUV::HDRConvScalerYUV(ProjectParameters *inputParams) {
  m_noOfFrameStores   = 7;        // Number should only be increased to support more frame buffers for different processing
  m_oFrameStore       = NULL;
  m_iFrameStore       = NULL;

  for (int index = 0; index < m_noOfFrameStores; index ++) {
    m_pFrameStore[index] = NULL;
  }

  m_convertProcess           = NULL;
  m_colorTransform           = NULL;
  m_normalizeFunction        = NULL;
  m_outputTransferFunction   = NULL;
  m_inputTransferFunction    = NULL;
  m_inputFrame               = NULL;
  m_outputFrame              = NULL;
  m_convertFormat            = NULL;

  m_addNoise                 = NULL;
  m_colorSpaceConvert        = NULL;
  m_colorSpaceFrame          = NULL;
  m_scaledFrame              = NULL;
  m_chromaScale              = NULL;
  m_frameScale                = NULL;

  m_frameFilter              = NULL;
  m_frameFilterNoise0        = NULL;
  m_frameFilterNoise1        = NULL;

  m_convertFrameStore        = NULL;
  m_convertIQuantize         = NULL;
  m_changeColorPrimaries     = FALSE;
 
  m_filterInFloat            = inputParams->m_filterInFloat;
  
  m_inputFile                = &inputParams->m_inputFile;
  m_outputFile               = &inputParams->m_outputFile;
  m_startFrame               =  m_inputFile->m_startFrame;
  
  m_cropOffsetLeft           =  inputParams->m_cropOffsetLeft;
  m_cropOffsetTop            =  inputParams->m_cropOffsetTop;
  m_cropOffsetRight          =  inputParams->m_cropOffsetRight;
  m_cropOffsetBottom         =  inputParams->m_cropOffsetBottom;

  m_croppedFrameStore        = NULL;
  
  m_bUseChromaDeblocking   =  inputParams->m_bUseChromaDeblocking;
  m_bUseWienerFiltering    =  inputParams->m_bUseWienerFiltering;
  m_bUse2DSepFiltering     =  inputParams->m_bUse2DSepFiltering;
  m_b2DSepMode             =  inputParams->m_b2DSepMode;
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------

void HDRConvScalerYUV::deleteMemory() {
  // delete processing frame stores if previous allocated
  for (int i = 0; i < m_noOfFrameStores; i++) {
    if (m_pFrameStore[i] != NULL) {
      delete m_pFrameStore[i];
      m_pFrameStore[i] = NULL;
    }
  }

  // output frame objects
  if (m_oFrameStore != NULL) {
    delete m_oFrameStore;
    m_oFrameStore = NULL;
  }
  // input frame objects
  if (m_iFrameStore != NULL) {
    delete m_iFrameStore;
    m_iFrameStore = NULL;
  }

  if (m_colorTransform != NULL) {
    delete m_colorTransform;
    m_colorTransform = NULL;
  }
  
  if (m_convertFormat != NULL){
    delete m_convertFormat;
    m_convertFormat = NULL;
  }
  
  if (m_chromaScale != NULL) {
    delete m_chromaScale;
    m_chromaScale = NULL;
  }
  
  if (m_frameScale != NULL) {
    delete m_frameScale;
    m_frameScale = NULL;
  }

  if (m_convertProcess != NULL){
    delete m_convertProcess;
    m_convertProcess = NULL;
  }

  // Cropped frame store
  if (m_croppedFrameStore != NULL) {
    delete m_croppedFrameStore;
    m_croppedFrameStore = NULL;
  }
  
  if (m_colorSpaceConvert != NULL) {
    delete m_colorSpaceConvert;
    m_colorSpaceConvert = NULL;
  }
 

  if (m_scaledFrame != NULL) {
    delete m_scaledFrame;
    m_scaledFrame = NULL;
  }
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------

void HDRConvScalerYUV::destroy() {

  if (m_addNoise != NULL) {
    delete m_addNoise;
    m_addNoise = NULL;
  }

  if (m_inputFrame != NULL) {
    delete m_inputFrame;
    m_inputFrame = NULL;
  }

  if (m_outputFrame != NULL) {
    delete m_outputFrame;
    m_outputFrame = NULL;
  }
  
  if (m_normalizeFunction != NULL) {
    delete m_normalizeFunction;
    m_normalizeFunction = NULL;
  }
  if (m_outputTransferFunction != NULL) {
    delete m_outputTransferFunction;
    m_outputTransferFunction = NULL;
  }
  if (m_inputTransferFunction != NULL) {
    delete m_inputTransferFunction;
    m_inputTransferFunction = NULL;
  }
  
  if (m_convertIQuantize != NULL) {
    delete m_convertIQuantize;
    m_convertIQuantize = NULL;
  }
 
  if (m_colorSpaceFrame != NULL) {
    delete m_colorSpaceFrame;
    m_colorSpaceFrame = NULL;
  }
  
  if (m_frameFilter != NULL) {
    delete m_frameFilter;
    m_frameFilter = NULL;    
  }
  if (m_frameFilterNoise0 != NULL) {
    delete m_frameFilterNoise0;
    m_frameFilterNoise0 = NULL;    
  }
  if (m_frameFilterNoise1 != NULL) {
    delete m_frameFilterNoise1;
    m_frameFilterNoise1 = NULL;    
  }
  
  if (m_convertFrameStore != NULL) {
    delete m_convertFrameStore;
    m_convertFrameStore = NULL;
  }
  
  
  IOFunctions::closeFile(m_inputFile);
  IOFunctions::closeFile(m_outputFile);
}

//-----------------------------------------------------------------------------
// allocate memory - create objects
//-----------------------------------------------------------------------------
void HDRConvScalerYUV::init (ProjectParameters *inputParams) {
  int width, height;

  FrameFormat   *input  = &inputParams->m_source;
  FrameFormat   *output = &inputParams->m_output;
  
   // Input frame objects initialization
  IOFunctions::openFile (m_inputFile);
  
  if (m_inputFile->m_isConcatenated == FALSE && strlen(m_inputFile->m_fTail) == 0) {
    // Update number of frames to be processed
    inputParams->m_numberOfFrames = 1;
  }
  
  // create memory for reading the input filesource
  m_inputFrame = Input::create(m_inputFile, input, inputParams);
  
  // Create output file
  IOFunctions::openFile (m_outputFile, OPENFLAGS_WRITE, OPEN_PERMISSIONS);
  
  // create frame memory as necessary
  // Input. This has the same format as the Input file.
  m_iFrameStore  = new Frame(m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_iFrameStore->clear();
  
  if (m_cropOffsetLeft != 0 || m_cropOffsetTop != 0 || m_cropOffsetRight != 0 || m_cropOffsetBottom != 0) {
    width  = m_inputFrame->m_width[Y_COMP]  - m_cropOffsetLeft + m_cropOffsetRight;
    height = m_inputFrame->m_height[Y_COMP] - m_cropOffsetTop  + m_cropOffsetBottom;
    m_croppedFrameStore  = new Frame(width, height, m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
    m_croppedFrameStore->clear();
  }
  else {
    width  = m_inputFrame->m_width[Y_COMP];
    height = m_inputFrame->m_height[Y_COMP];
  }


  // Output (transfer function or denormalization). Since we don't support scaling, lets reset the width and height here.
  //m_outputFile->m_format.m_height[Y_COMP] = output->m_height[Y_COMP] = height;
  //m_outputFile->m_format.m_width [Y_COMP] = output->m_width [Y_COMP] = width;
  m_outputFile->m_format.m_height[Y_COMP] = output->m_height[Y_COMP];
  m_outputFile->m_format.m_width [Y_COMP] = output->m_width [Y_COMP];

    // Frame store for the inversion of PQ TF
  m_pFrameStore[1]   = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_pFrameStore[1]->clear();

  // Also creation of frame store for the transfer function processed images
  //m_pFrameStore[3]  = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], output->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_pFrameStore[3]  = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_pFrameStore[3]->clear();
  
  m_pFrameStore[4]   = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_pFrameStore[4]->clear();

  // Create output file
  m_outputFrame = Output::create(m_outputFile, output);
  
  // Chroma format conversion (if needed). Only difference is in chroma format
  if (output->m_chromaFormat != m_inputFrame->m_chromaFormat) {
    if (m_filterInFloat == TRUE) {
      m_pFrameStore[0]  = new Frame(width, height, TRUE, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, output->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
    }
    else {
      m_pFrameStore[0]  = new Frame(width, height, m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, output->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);      
    }

    m_pFrameStore[0]->clear();
  }
  else {
    m_pFrameStore[0] = NULL;
  }
  
  // Here we may convert to the output's format type (e.g. from integer to float).
  //  m_convertFrameStore  = new Frame(m_width, m_height, output->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries,output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_convertFrameStore  = new Frame(width, height, TRUE, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries,output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_convertFrameStore->clear();
  

  m_oFrameStore  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], output->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_oFrameStore->clear();
  
  
  // initiate the color transform process. Maybe we can move this in the process section though.
  // This does not currently handle YCbCr to YCbCr conversion cleanly (goes first to RGB which is not correct). Needs to be fixed.
  if (m_iFrameStore->m_colorSpace == CM_YCbCr) { // If YCbCr we need to first go to RGB
    if (m_oFrameStore->m_colorSpace == CM_YCbCr && m_iFrameStore->m_colorPrimaries == m_oFrameStore->m_colorPrimaries && input->m_iConstantLuminance == output->m_iConstantLuminance) {
      m_colorTransform = ColorTransform::create(m_iFrameStore->m_colorSpace, m_iFrameStore->m_colorPrimaries, m_oFrameStore->m_colorSpace, m_iFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, CLT_NULL, input->m_iConstantLuminance, output->m_iConstantLuminance);
      m_changeColorPrimaries = FALSE;
    }
    else {
      m_colorTransform = ColorTransform::create(m_iFrameStore->m_colorSpace, m_iFrameStore->m_colorPrimaries, CM_RGB, m_iFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, CLT_NULL, input->m_iConstantLuminance, 0);
      m_changeColorPrimaries = TRUE;
    }
    m_colorSpaceConvert = ColorTransform::create(CM_RGB, m_iFrameStore->m_colorPrimaries, m_oFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, inputParams->m_closedLoopConversion, 0, output->m_iConstantLuminance);
    
    // frame store for color format conversion
    //m_pFrameStore[2]  = new Frame(m_width, m_height, output->m_isFloat, CM_RGB, m_iFrameStore->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
    m_pFrameStore[2]  = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, CM_RGB, m_iFrameStore->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
    m_pFrameStore[2]->clear();
    
    m_colorSpaceFrame  = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, TF_NULL, 1.0);
    m_colorSpaceFrame->clear();
    
  }
  else {
    m_colorTransform = ColorTransform::create(m_iFrameStore->m_colorSpace, m_iFrameStore->m_colorPrimaries, m_oFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, CLT_NULL);
    m_colorSpaceConvert = ColorTransform::create(m_oFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, m_oFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, inputParams->m_closedLoopConversion);
    
    // frame store for color format conversion
    m_pFrameStore[2]  = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
    m_pFrameStore[2]->clear();
    
    m_colorSpaceFrame  = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, TF_NULL, 1.0);
    m_colorSpaceFrame->clear();
    
    m_changeColorPrimaries = FALSE;
  }

  // Chroma subsampling
  // We may wish to create a single convert class that uses as inputs the output resolution as well the input and output chroma format, and the downsampling/upsampling method. That would make the code easier to handle.
  // To be done later.
  if (input->m_chromaFormat == output->m_chromaFormat) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, 0, m_inputFrame->m_chromaLocation, output->m_chromaLocation);
  }
  else if (input->m_chromaFormat == CF_444 && output->m_chromaFormat == CF_420) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, inputParams->m_chromaDownsampleFilter, m_inputFrame->m_chromaLocation, output->m_chromaLocation, inputParams->m_useAdaptiveDownsampling, inputParams->m_useMinMax);
  }
  else if ((input->m_chromaFormat == CF_420 || input->m_chromaFormat == CF_422) && output->m_chromaFormat == CF_444) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, inputParams->m_chromaUpsampleFilter, m_inputFrame->m_chromaLocation, output->m_chromaLocation, inputParams->m_useAdaptiveUpsampling, inputParams->m_useMinMax);
  }

  if ( (input->m_colorSpace == output->m_colorSpace) && (input->m_colorPrimaries == output->m_colorPrimaries) && (input->m_transferFunction == output->m_transferFunction)) {
    m_useSingleTransferStep = TRUE;
    m_normalizeFunction = NULL;
    m_inputTransferFunction  = TransferFunction::create(TF_NULL, TRUE, inputParams->m_srcNormalScale, input->m_systemGamma, inputParams->m_srcMinValue, inputParams->m_srcMaxValue);    
    // Output transfer function picture store
    m_outputTransferFunction  = TransferFunction::create(TF_NULL, TRUE, inputParams->m_outNormalScale, output->m_systemGamma, inputParams->m_outMinValue, inputParams->m_outMaxValue);
  }
  else 
  {
    
  if ( (input->m_iConstantLuminance != 0 || output->m_iConstantLuminance != 0 )|| ( input->m_transferFunction != TF_NULL && input->m_transferFunction != TF_POWER && ( inputParams->m_useSingleTransferStep == FALSE || (input->m_transferFunction != TF_PQ && input->m_transferFunction != TF_HPQ && input->m_transferFunction != TF_HPQ2 && input->m_transferFunction != TF_APQ && input->m_transferFunction != TF_APQS && input->m_transferFunction != TF_MPQ && input->m_transferFunction != TF_AMPQ && input->m_transferFunction != TF_PH && input->m_transferFunction != TF_APH) ))) {
    m_useSingleTransferStep = FALSE;
    m_normalizeFunction = TransferFunction::create(TF_NORMAL, FALSE, inputParams->m_srcNormalScale, input->m_systemGamma, inputParams->m_srcMinValue, inputParams->m_srcMaxValue);
    m_inputTransferFunction  = TransferFunction::create(input->m_transferFunction, FALSE, inputParams->m_srcNormalScale, input->m_systemGamma, inputParams->m_srcMinValue, inputParams->m_srcMaxValue);
  }
  else {
    m_useSingleTransferStep = TRUE;
    m_normalizeFunction = NULL;
    m_inputTransferFunction  = TransferFunction::create(input->m_transferFunction, TRUE, inputParams->m_srcNormalScale, input->m_systemGamma, inputParams->m_srcMinValue, inputParams->m_srcMaxValue);
  }
    // Output transfer function picture store
    m_outputTransferFunction  = TransferFunction::create(output->m_transferFunction, TRUE, inputParams->m_outNormalScale, output->m_systemGamma, inputParams->m_outMinValue, inputParams->m_outMaxValue);
  }
  

  // Format conversion process
  m_convertIQuantize = Convert::create(&m_iFrameStore->m_format, &m_convertFrameStore->m_format);
  m_convertProcess   = Convert::create(&m_pFrameStore[4]->m_format, output);
  if (m_bUseChromaDeblocking == TRUE)
    m_frameFilter      = FrameFilter::create(width, height, FT_DEBLOCK);

  m_frameScale   = FrameScale::create(width, height, output->m_width[Y_COMP], output->m_height[Y_COMP], &inputParams->m_fsParams, inputParams->m_chromaDownsampleFilter, output->m_chromaLocation[FP_FRAME], inputParams->m_useMinMax);
  m_scaledFrame = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], TRUE, CM_RGB, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, TF_NORMAL, 1.0);

  if (m_bUseWienerFiltering == TRUE)
    m_frameFilterNoise0 = FrameFilter::create(width, height, FT_WIENER2D);
  if (m_bUse2DSepFiltering == TRUE)
    m_frameFilterNoise1 = FrameFilter::create(width, height, FT_2DSEP, m_b2DSepMode);

}

//-----------------------------------------------------------------------------
// main filtering function
//-----------------------------------------------------------------------------

void HDRConvScalerYUV::process( ProjectParameters *inputParams ) {
  int frameNumber;
  int iCurrentFrameToProcess = 0;
  float fDistance0 = inputParams->m_source.m_frameRate / inputParams->m_output.m_frameRate;
  //FrameFormat   *output = &inputParams->m_output;
  FrameFormat   *input  = &inputParams->m_source;

  clock_t clk;  
  bool errorRead = FALSE;
  
  Frame *currentFrame = NULL;

  // Now process all frames
  for (frameNumber = 0; frameNumber < inputParams->m_numberOfFrames; frameNumber ++) {
    clk = clock();
    iCurrentFrameToProcess = int(frameNumber * fDistance0);
    
    // read frames
    m_iFrameStore->m_frameNo = frameNumber;
    if (m_inputFrame->readOneFrame(m_inputFile, iCurrentFrameToProcess, m_inputFile->m_fileHeader, m_startFrame) == TRUE) {
      // Now copy input frame buffer to processing frame buffer for any subsequent processing
      m_inputFrame->copyFrame(m_iFrameStore);
    }
    else {
      inputParams->m_numberOfFrames = frameNumber;
      errorRead = TRUE;
      break;
    }
    
    if (errorRead == TRUE) {
      break;
    }
    else if (inputParams->m_silentMode == FALSE) {
      printf("%05d ", frameNumber );
    }
    
    currentFrame = m_iFrameStore;
    if (m_croppedFrameStore != NULL) {
      m_croppedFrameStore->copy(m_iFrameStore, m_cropOffsetLeft, m_cropOffsetTop, m_iFrameStore->m_width[Y_COMP] + m_cropOffsetRight, m_iFrameStore->m_height[Y_COMP] + m_cropOffsetBottom, 0, 0);

      currentFrame = m_croppedFrameStore;
    }
      
    if (m_iFrameStore->m_chromaFormat != m_oFrameStore->m_chromaFormat) {
      // Convert chroma format if needed (note that given the current code
      // we can always create m_cFrameStore if we want to and avoid the conditional,
      // while being penalized with memory copy operations.
      // An alternative could also be to set m_cFrameStore to be equal to m_iFrameStore
      // and have a "true" null process that does nothing. That would save
      // on both memory and memory copy operations. TBD
      
      // The resolution of the below frame stores is actually at 4:4:4 regardless if the data in it are 4:2:0. Code works as is, but should be fixed.
      if (m_filterInFloat == TRUE) {
        // Convert to different format if needed (integer to float)
        m_convertIQuantize->process (m_pFrameStore[0], currentFrame);
        if (m_bUseChromaDeblocking == TRUE) // Perform deblocking
          m_frameFilter->process(m_pFrameStore[0]);
        // Chroma conversion
        m_convertFormat->process(m_convertFrameStore, m_pFrameStore[0]);
      }
      else {      
        m_convertFormat->process (m_pFrameStore[0], currentFrame);
        // Convert to different format if needed (integer to float)
        m_convertIQuantize->process(m_convertFrameStore, m_pFrameStore[0]);
      }
    }
    else {
      // Convert to different format if needed (integer to float)
      m_convertIQuantize->process(m_convertFrameStore, currentFrame);
    }
    currentFrame = m_convertFrameStore;  
    
    // Apply denoising if specified (using Wiener2D currently)
    if (m_bUseWienerFiltering == TRUE)
      m_frameFilterNoise0->process(currentFrame);

    // Separable denoiser      
    if (m_bUse2DSepFiltering == TRUE)
      m_frameFilterNoise1->process(currentFrame);

    m_frameScale->process(m_scaledFrame, currentFrame);
    currentFrame = m_scaledFrame;

    // Now perform a color format conversion
    // Output to m_pFrameStore memory with appropriate color space conversion
    // Note that the name of "forward" may be a bit of a misnomer.
    
    if (!(input->m_iConstantLuminance != 0 && input->m_colorSpace == CM_YCbCr)) {
      m_colorTransform->process(m_pFrameStore[2], currentFrame);
      if ( m_useSingleTransferStep == FALSE ) {
        m_inputTransferFunction->forward(m_pFrameStore[3], m_pFrameStore[2]);
        m_normalizeFunction->forward(m_pFrameStore[1], m_pFrameStore[3]);
      }
      else {
        m_inputTransferFunction->forward(m_pFrameStore[1], m_pFrameStore[2]);
      }
    }
    else {
      m_colorTransform->process(m_pFrameStore[2], currentFrame);
      m_normalizeFunction->forward(m_pFrameStore[1], m_pFrameStore[2]);
    }    
    
    if (m_changeColorPrimaries == TRUE) {
      m_colorSpaceConvert->process(m_colorSpaceFrame, m_pFrameStore[1]);
      m_outputTransferFunction->inverse(m_pFrameStore[4], m_colorSpaceFrame);
    }
    else{
      // here we apply the output transfer function (to be fixed)
      m_outputTransferFunction->inverse(m_pFrameStore[4], m_pFrameStore[1]);
    }
    m_convertProcess->process(m_oFrameStore, m_pFrameStore[4]);

    // frame output
    m_outputFrame->copyFrame(m_oFrameStore);
    m_outputFrame->writeOneFrame(m_outputFile, frameNumber, m_outputFile->m_fileHeader, 0);
    
    clk = clock() - clk;
    if (inputParams->m_silentMode == FALSE){
      printf("%7.3f", 1.0 * clk / CLOCKS_PER_SEC);
      printf("\n");
      fflush(stdout);
    }
    else {
      printf("Processing Frame : %d\r", frameNumber);
      fflush(stdout);
    }
  } //end for frameNumber
}

//-----------------------------------------------------------------------------
// header output
//-----------------------------------------------------------------------------
void HDRConvScalerYUV::outputHeader(ProjectParameters *inputParams) {
  IOVideo *InpFile = &inputParams->m_inputFile;
  IOVideo *OutFile = &inputParams->m_outputFile;
  printf("================================================================================================================\n");
  printf("Source: %s\n", InpFile->m_fName);
  printf("W x H:  (%dx%d) \n", m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP]);
  if (m_inputFrame->m_isFloat == TRUE)
    printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, Type: %s\n", COLOR_FORMAT[m_inputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_inputFrame->m_isInterlaced], COLOR_SPACE[m_inputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_inputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_inputFrame->m_transferFunction], m_inputFrame->m_frameRate, PIXEL_TYPE[m_inputFrame->m_pixelType[Y_COMP]]);
  else
    printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, BitDepth: %d, Range: %s\n", COLOR_FORMAT[m_inputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_inputFrame->m_isInterlaced], COLOR_SPACE[m_inputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_inputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_inputFrame->m_transferFunction], m_inputFrame->m_frameRate, m_inputFrame->m_bitDepthComp[Y_COMP], SOURCE_RANGE_TYPE[m_inputFrame->m_sampleRange + 1]);

  printf("----------------------------------------------------------------------------------------------------------------\n");
  printf("Output: %s\n", OutFile->m_fName);
  printf("W x H:  (%dx%d) \n", m_outputFrame->m_width[Y_COMP], m_outputFrame->m_height[Y_COMP]);
  if (m_outputFrame->m_isFloat == TRUE)
    printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, Type: %s\n", COLOR_FORMAT[m_outputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_outputFrame->m_isInterlaced], COLOR_SPACE[m_outputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_outputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_outputFrame->m_transferFunction], m_outputFrame->m_frameRate, PIXEL_TYPE[m_outputFrame->m_pixelType[Y_COMP]]);
  else
    printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, BitDepth: %d, Range: %s\n", COLOR_FORMAT[m_outputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_outputFrame->m_isInterlaced], COLOR_SPACE[m_outputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_outputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_outputFrame->m_transferFunction], m_outputFrame->m_frameRate, m_outputFrame->m_bitDepthComp[Y_COMP], SOURCE_RANGE_TYPE[m_outputFrame->m_sampleRange + 1]);
  printf("================================================================================================================\n");
}

//-----------------------------------------------------------------------------
// footer output
//-----------------------------------------------------------------------------

void HDRConvScalerYUV::outputFooter(ProjectParameters *inputParams) {
  clock_t clk = clock();
  FILE* f = IOFunctions::openFile(inputParams->m_logFile, "at");

  if (f != NULL) {
    fprintf(f, "%s ", inputParams->m_inputFile.m_fName);
    fprintf(f, "%s ", inputParams->m_outputFile.m_fName);
    fprintf(f, "%d ", inputParams->m_numberOfFrames);
    fprintf(f, "%f \n", 1.0 * clk / inputParams->m_numberOfFrames / CLOCKS_PER_SEC);
  }

  printf("\n================================================================================================================\n");
  printf("Total of %d frames processed in %7.3f seconds\n",inputParams->m_numberOfFrames, 1.0 * clk / CLOCKS_PER_SEC);
  printf("Conversion Speed: %3.2ffps\n", (float) inputParams->m_numberOfFrames * CLOCKS_PER_SEC / clk);

  IOFunctions::closeFile(f);
}

