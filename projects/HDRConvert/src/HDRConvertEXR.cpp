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
 * \file HDRConvertEXR.cpp
 *
 * \brief
 *    HDRConvertEXR class source files for performing HDR (EXR) format conversion
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

#include <time.h>
#include <string.h>
#include <math.h>
#include "HDRConvertEXR.H"
#include "FrameScaleHalf.H"


HDRConvertEXR::HDRConvertEXR(ProjectParameters *inputParams) {
  m_noOfFrameStores = 5;        // Number should only be increased to support more frame buffers for different processing
  m_oFrameStore       = NULL;
  m_iFrameStore       = NULL;
  m_dFrameStore       = NULL;
  
  for (int index = 0; index < m_noOfFrameStores; index++) {
    m_pFrameStore[index]     = NULL;
    m_pDFrameStore[index]    = NULL;
  }
  
  m_convertProcess           = NULL;
  m_colorTransform           = NULL;
  m_normalizeFunction        = NULL;
  m_outputTransferFunction   = NULL;
  m_inputTransferFunction    = NULL;
  m_inputFrame               = NULL;
  m_outputFrame              = NULL;
  m_convertTo420             = NULL;
  
  m_addNoise                 = NULL;
  m_colorSpaceConvert        = NULL;
  m_colorSpaceFrame          = NULL;
  m_scaledFrame              = NULL;
  m_frameScale               = NULL;
  m_chromaScale              = NULL;
  
  m_frameFilterNoise0        = NULL;
  m_frameFilterNoise1        = NULL;
  m_frameFilterNoise2        = NULL;
  
  m_useMinMax                = inputParams->m_useMinMax;
  
  m_filterInFloat            = inputParams->m_filterInFloat;
  
  m_inputFile                = &inputParams->m_inputFile;
  m_outputFile               = &inputParams->m_outputFile;
  m_startFrame               =  m_inputFile->m_startFrame;
  
  m_cropOffsetLeft           = inputParams->m_cropOffsetLeft;
  m_cropOffsetTop            = inputParams->m_cropOffsetTop;
  m_cropOffsetRight          = inputParams->m_cropOffsetRight;
  m_cropOffsetBottom         = inputParams->m_cropOffsetBottom;

  m_croppedFrameStore        = NULL;
  
  m_linearDownConversion     = inputParams->m_linearDownConversion;
  if (m_linearDownConversion == TRUE)
    inputParams->m_closedLoopConversion = CLT_NULL;
    
  m_rgbDownConversion        = inputParams->m_rgbDownConversion;
  m_bUseWienerFiltering      = inputParams->m_bUseWienerFiltering;
  m_bUse2DSepFiltering       = inputParams->m_bUse2DSepFiltering;
  m_bUseNLMeansFiltering     = inputParams->m_bUseNLMeansFiltering;

  m_b2DSepMode               = inputParams->m_b2DSepMode;
  
  m_srcDisplayGammaAdjust    = NULL;
  m_outDisplayGammaAdjust    = NULL;
  m_toneMapping              = NULL;
}

void HDRConvertEXR::deleteMemory() {
  
  // delete processing frame stores if previous allocated
  for (int i = 0; i < m_noOfFrameStores; i++) {
    if (m_pFrameStore[i] != NULL) {
      delete m_pFrameStore[i];
      m_pFrameStore[i] = NULL;
    }
    if (m_pDFrameStore[i] != NULL) {
      delete m_pDFrameStore[i];
      m_pDFrameStore[i] = NULL;
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
  
  if (m_convertTo420 != NULL){
    delete m_convertTo420;
    m_convertTo420 = NULL;
  }
  
  if (m_dFrameStore != NULL) {
    delete m_dFrameStore;
    m_dFrameStore = NULL;
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
  
  if (m_colorSpaceFrame != NULL) {
    delete m_colorSpaceFrame;
    m_colorSpaceFrame = NULL;
  }

  if (m_srcDisplayGammaAdjust != NULL) {
    delete m_srcDisplayGammaAdjust;
    m_srcDisplayGammaAdjust = NULL;
  }

  if (m_outDisplayGammaAdjust != NULL) {
    delete m_outDisplayGammaAdjust;
    m_outDisplayGammaAdjust = NULL;
  }
  
  if (m_toneMapping != NULL) {
    delete m_toneMapping;
    m_toneMapping = NULL;
  }
  
  if (m_scaledFrame != NULL) {
    delete m_scaledFrame;
    m_scaledFrame = NULL;
  }
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------

void HDRConvertEXR::destroy() {
  
  deleteMemory();
  
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
  
  if (m_frameFilterNoise0 != NULL) {
    delete m_frameFilterNoise0;
    m_frameFilterNoise0 = NULL;
  }
  
  if (m_frameFilterNoise1 != NULL) {
    delete m_frameFilterNoise1;
    m_frameFilterNoise1 = NULL;
  }

  if (m_frameFilterNoise2 != NULL) {
    delete m_frameFilterNoise2;
    m_frameFilterNoise2 = NULL;
  }
  
  IOFunctions::closeFile(m_inputFile);
  IOFunctions::closeFile(m_outputFile);
}

//-----------------------------------------------------------------------------
// allocate memory - create objects
//-----------------------------------------------------------------------------
void HDRConvertEXR::allocateFrameStores(ProjectParameters *inputParams, FrameFormat   *input, FrameFormat   *output) {

  int width, height;
  
  deleteMemory();
 
  // Create output file
  IOFunctions::openFile (m_outputFile, OPENFLAGS_WRITE, OPEN_PERMISSIONS);
  
  // create frame memory as necessary
  // Input. This has the same format as the Input file.
  m_iFrameStore  = new Frame(m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_iFrameStore->clear();
  // update input parameters to avoid any possible errors later on
  *input = m_iFrameStore->m_format;
  // frame rate is not passed into the frame store so lets copy this here back to the input
  input->m_frameRate = m_inputFrame->m_frameRate;

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

  // Processing also happens here in the same space (for now). Likely we should change this into floats, or have this depending on the process we are doing (int to floats or floats to ints).
  // Currently lets set this to the same format as the input store.
  m_pFrameStore[0]  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_pFrameStore[0]->clear();
  
  // Also creation of frame store for the normalized images
  m_pFrameStore[1]  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_pFrameStore[1]->clear();
  // and of the transfer function converted pictures
  m_pFrameStore[2]  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, output->m_transferFunction, m_inputFrame->m_systemGamma);
  m_pFrameStore[2]->clear();
  

  // Noise addition frame store. This is of the same type as the input since we would be adding noise at that stage
  m_pFrameStore[4]  = new Frame(width, height, m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_pFrameStore[4]->clear();

  
  // Output. Since we don't support scaling, lets reset the width and height here.
  
  m_outputFrame = Output::create(m_outputFile, output);
  
  // Frame store for altering color space
  if (m_inputFrame->m_colorSpace == CM_XYZ && output->m_colorSpace != CM_XYZ && output->m_colorPrimaries != CP_NONE ) {
    m_colorSpaceFrame   = new Frame(width, height, TRUE, CM_RGB, output->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  }
  else {
    m_colorSpaceFrame   = new Frame(width, height, TRUE, m_inputFrame->m_colorSpace, output->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  }
  m_colorSpaceFrame->clear();


  m_oFrameStore  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], output->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_oFrameStore->clear();

  if (output->m_chromaFormat != m_inputFrame->m_chromaFormat) {
    if (m_filterInFloat == TRUE) {
  //m_outputFile->m_format.m_height[Y_COMP] = output->m_height[Y_COMP] = height;
  //m_outputFile->m_format.m_width [Y_COMP] = output->m_width [Y_COMP] = width;
  m_outputFile->m_format.m_height[Y_COMP] = output->m_height[Y_COMP];
  m_outputFile->m_format.m_width [Y_COMP] = output->m_width [Y_COMP];

  m_pFrameStore[3]  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);      
    }
    else {
      m_pFrameStore[3]  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], output->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, m_inputFrame->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
    }
    m_pFrameStore[3]->clear();
  }
  else {
    m_pFrameStore[3] = NULL;
  }

  // Format conversion process
  m_convertProcess = Convert::create(&m_iFrameStore->m_format, output, &inputParams->m_cvParams);
  
  
  // initiate the color transform process. Maybe we can move this in the process section though
  
  if (m_iFrameStore->m_colorSpace == CM_XYZ && m_oFrameStore->m_colorSpace != CM_XYZ && m_oFrameStore->m_colorPrimaries != CP_NONE) {
    m_colorSpaceConvert = ColorTransform::create(m_iFrameStore->m_colorSpace, m_iFrameStore->m_colorPrimaries, CM_RGB, m_oFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, CLT_NULL, 0, 0);
    m_colorTransform = ColorTransform::create(CM_RGB, m_oFrameStore->m_colorPrimaries, m_oFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, inputParams->m_closedLoopConversion, 0, output->m_iConstantLuminance);
  }
  else {
    m_colorSpaceConvert = ColorTransform::create(m_iFrameStore->m_colorSpace, m_iFrameStore->m_colorPrimaries, m_iFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, CLT_NULL, input->m_iConstantLuminance, 0);
    m_colorTransform = ColorTransform::create(m_iFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, m_oFrameStore->m_colorSpace, m_oFrameStore->m_colorPrimaries, inputParams->m_transformPrecision, inputParams->m_useHighPrecisionTransform, inputParams->m_closedLoopConversion, 0, output->m_iConstantLuminance, output->m_transferFunction, output->m_bitDepthComp[Y_COMP], output->m_sampleRange, inputParams->m_chromaDownsampleFilter, inputParams->m_chromaUpsampleFilter, inputParams->m_useAdaptiveDownsampling, inputParams->m_useAdaptiveUpsampling, inputParams->m_useMinMax, inputParams->m_closedLoopIterations, output->m_chromaFormat, output->m_chromaLocation, inputParams->m_filterInFloat, inputParams->m_enableTFLUTs, &inputParams->m_ctParams);
  }
  
  // Chroma subsampling
  // We may wish to create a single convert class that uses as inputs the output resolution as well the input and output chroma format, and the downsampling/upsampling method. That would make the code easier to handle.
  // To be done later.
    m_convertTo420 = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, inputParams->m_chromaDownsampleFilter,  m_inputFrame->m_chromaLocation, output->m_chromaLocation, inputParams->m_useAdaptiveDownsampling, inputParams->m_useMinMax);
    
  if (output->m_chromaFormat != CF_420) {
    m_linearDownConversion = FALSE;
    m_rgbDownConversion = FALSE;
  }
    
  // To simplify things we always create the frame store for linear downconversion regardless of the target when this flag is enabled (To be cleaned later)
  if (m_linearDownConversion == TRUE) {
    int dWidth  = output->m_width [Y_COMP] / 2;
    int dHeight = output->m_height[Y_COMP] / 2;
    m_pDFrameStore[3]  = new Frame(output->m_width [Y_COMP], output->m_height[Y_COMP], TRUE, CM_RGB, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, TF_NORMAL, 1.0); 

    m_dFrameStore  = new Frame(dWidth, dHeight, TRUE, CM_RGB, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, TF_NORMAL, 1.0); 
    m_pDFrameStore[4]  = new Frame(dWidth, dHeight, TRUE, CM_RGB, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, TF_NULL, 1.0); 
    m_chromaScale   = FrameScale::create(output->m_width[Y_COMP], output->m_height[Y_COMP], dWidth, dHeight, &inputParams->m_fsParams, inputParams->m_chromaDownsampleFilter, output->m_chromaLocation[FP_FRAME], inputParams->m_useMinMax);
    
    m_pDFrameStore[0]  = new Frame(dWidth, dHeight, TRUE, CM_RGB, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, 1.0);
    m_pDFrameStore[1]  = new Frame(dWidth, dHeight, TRUE, output->m_colorSpace, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, 1.0);
    //m_pDFrameStore[2]  = new Frame(dWidth, dHeight, TRUE, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, 1.0);

  }
  else if (m_rgbDownConversion == TRUE) {
    int dWidth  = output->m_width [Y_COMP] / 2;
    int dHeight = output->m_height[Y_COMP] / 2;
    
    m_dFrameStore  = new Frame(dWidth, dHeight, TRUE, CM_RGB, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, 1.0); 
    m_chromaScale   = FrameScale::create(output->m_width[Y_COMP], output->m_height[Y_COMP], dWidth, dHeight, &inputParams->m_fsParams, inputParams->m_chromaDownsampleFilter, output->m_chromaLocation[FP_FRAME], inputParams->m_useMinMax);
    
    m_pDFrameStore[0]  = new Frame(dWidth, dHeight, TRUE, output->m_colorSpace, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, 1.0);
  }  
  m_frameScale  = FrameScale::create(width, height, output->m_width[Y_COMP], output->m_height[Y_COMP], &inputParams->m_fsParams, inputParams->m_chromaDownsampleFilter, output->m_chromaLocation[FP_FRAME], inputParams->m_useMinMax);
  m_scaledFrame = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], TRUE, CM_RGB, output->m_colorPrimaries, CF_444, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, TF_NORMAL, 1.0);
  
  if (m_bUseWienerFiltering == TRUE) {
    m_frameFilterNoise0 = FrameFilter::create(output->m_width[Y_COMP], output->m_height[Y_COMP], FT_WIENER2DD);
  }
  
  if (m_bUse2DSepFiltering == TRUE) {
    m_frameFilterNoise1 = FrameFilter::create(output->m_width[Y_COMP], output->m_height[Y_COMP], FT_2DSEP, m_b2DSepMode);
  }
  
  if (m_bUseNLMeansFiltering == TRUE) {
    m_frameFilterNoise2 = FrameFilter::create(output->m_width[Y_COMP], output->m_height[Y_COMP], FT_NLMEANS);
  }

}

//-----------------------------------------------------------------------------
// allocate memory - create objects
//-----------------------------------------------------------------------------
void HDRConvertEXR::init (ProjectParameters *inputParams) {
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
  
  // Read first frame just to see if there is need to update any parameters. This is very important for OpenEXR files
  if (m_inputFrame->readOneFrame(m_inputFile, 0, m_inputFile->m_fileHeader, m_startFrame) == TRUE) {
    allocateFrameStores(inputParams, input, output);
  }
  else {
    fprintf(stderr, "Error reading input file. Process exiting.\n");
    destroy();
    exit(EXIT_FAILURE);
  }
    
  m_addNoise = AddNoise::create(inputParams->m_addNoise, inputParams->m_noiseVariance, inputParams->m_noiseMean);
  m_inputTransferFunction  = TransferFunction::create(input->m_transferFunction, TRUE, inputParams->m_srcNormalScale, input->m_systemGamma, inputParams->m_srcMinValue, inputParams->m_srcMaxValue, inputParams->m_enableTFunctionLUT);

  if ( output->m_iConstantLuminance !=0 || (output->m_transferFunction != TF_NULL && output->m_transferFunction != TF_POWER && ( inputParams->m_useSingleTransferStep == FALSE || (output->m_transferFunction != TF_PQ && output->m_transferFunction != TF_HPQ && output->m_transferFunction != TF_HPQ2 && output->m_transferFunction != TF_APQ && output->m_transferFunction != TF_APQS && output->m_transferFunction != TF_MPQ && output->m_transferFunction != TF_AMPQ  && output->m_transferFunction != TF_PH  && output->m_transferFunction != TF_APH && output->m_transferFunction != TF_HLG && output->m_transferFunction != TF_NORMAL)) )) {
    m_useSingleTransferStep = FALSE;
    m_normalizeFunction = TransferFunction::create(TF_NORMAL, FALSE, inputParams->m_outNormalScale, output->m_systemGamma, inputParams->m_outMinValue, inputParams->m_outMaxValue);
    m_outputTransferFunction  = TransferFunction::create(output->m_transferFunction, FALSE, inputParams->m_outNormalScale, output->m_systemGamma, inputParams->m_outMinValue, inputParams->m_outMaxValue, inputParams->m_enableTFunctionLUT);
  }
  else {
    m_useSingleTransferStep = TRUE;
    //m_normalizeFunction = NULL;
    m_normalizeFunction = TransferFunction::create(TF_NORMAL, FALSE, inputParams->m_outNormalScale, output->m_systemGamma, inputParams->m_outMinValue, inputParams->m_outMaxValue);

    m_outputTransferFunction  = TransferFunction::create(output->m_transferFunction, TRUE, inputParams->m_outNormalScale, output->m_systemGamma, inputParams->m_outMinValue, inputParams->m_outMaxValue, inputParams->m_enableTFunctionLUT);
  }
  
  m_srcDisplayGammaAdjust = DisplayGammaAdjust::create(input->m_displayAdjustment,  m_useSingleTransferStep ? inputParams->m_srcNormalScale : 1.0f, input->m_systemGamma);
  m_outDisplayGammaAdjust = DisplayGammaAdjust::create(output->m_displayAdjustment, m_useSingleTransferStep ? inputParams->m_outNormalScale : 1.0f, output->m_systemGamma);
  
  if (input->m_displayAdjustment == DA_HLG) {
    m_inputTransferFunction->setNormalFactor(1.0);
  }
  if (output->m_displayAdjustment == DA_HLG) {
    m_outputTransferFunction->setNormalFactor(1.0);
  }

  
  m_toneMapping = ToneMapping::create(inputParams->m_toneMapping, &inputParams->m_tmParams);
}



//-----------------------------------------------------------------------------
// main filtering function
//-----------------------------------------------------------------------------

void HDRConvertEXR::process( ProjectParameters *inputParams ) {

  int frameNumber;
  int iCurrentFrameToProcess = 0;
  float fDistance0 = inputParams->m_source.m_frameRate / inputParams->m_output.m_frameRate;
  Frame *currentFrame, *processFrame, *linearFrame;
  FrameFormat   *input  = &inputParams->m_source;
  FrameFormat   *output = &inputParams->m_output;

  clock_t clk;  
  bool errorRead = FALSE;

    // Now process all frames
  for (frameNumber = 0; frameNumber < inputParams->m_numberOfFrames; frameNumber ++) {
    clk = clock();
    iCurrentFrameToProcess = int(frameNumber * fDistance0);
    
    // read frames
    m_iFrameStore->m_frameNo = frameNumber;
    if (m_inputFrame->readOneFrame(m_inputFile, iCurrentFrameToProcess, m_inputFile->m_fileHeader, m_startFrame) == TRUE) {
      // If the size of the images has changed, then reallocate space appropriately
      if ((m_inputFrame->m_width[Y_COMP] != m_iFrameStore->m_width[Y_COMP]) || (m_inputFrame->m_height[Y_COMP] != m_iFrameStore->m_height[Y_COMP])) {
        // Since we do not support scaling, width and height are also reset here
        output->m_height[Y_COMP] = m_inputFrame->m_height[Y_COMP];
        output->m_width [Y_COMP] = m_inputFrame->m_width [Y_COMP];

        allocateFrameStores(inputParams, input, output) ;
      }
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
    // remove any embedded transfer function if any

    if (inputParams->m_enableLegacy == FALSE)
      m_inputTransferFunction->forward(currentFrame);
    
    // Convert to appropriate color space (XYZ to RGB or different primaries) if needed
    processFrame = m_colorSpaceFrame;
    m_colorSpaceConvert->process(processFrame, currentFrame);
    currentFrame = processFrame;

    processFrame = m_pFrameStore[4];

    // Add noise
    m_addNoise->process(processFrame, currentFrame);
    currentFrame = processFrame;
    
    // Apply denoising if specified (using Wiener2D currently)
    if (m_bUseWienerFiltering == TRUE)
      m_frameFilterNoise0->process(currentFrame);

    // Separable denoiser      
    if (m_bUse2DSepFiltering == TRUE)
      m_frameFilterNoise1->process(currentFrame);

    // NLMeans denoiser      
    if (m_bUseNLMeansFiltering == TRUE)
      m_frameFilterNoise2->process(currentFrame);
    
    m_toneMapping->process(currentFrame);

    m_frameScale->process(m_scaledFrame, currentFrame);
    currentFrame = m_scaledFrame;
    
    linearFrame = currentFrame;
    
    if (m_linearDownConversion == TRUE) {
      // m_normalizeFunction->inverse(m_pDFrameStore[3], currentFrame);
      // m_chromaScale->process(m_dFrameStore, m_pDFrameStore[3]);
      // m_normalizeFunction->forward(m_pDFrameStore[4], m_dFrameStore);
      // m_outputTransferFunction->inverse(m_pDFrameStore[0], m_pDFrameStore[4]);
      m_chromaScale->process(m_dFrameStore, currentFrame);
      m_outDisplayGammaAdjust->inverse(m_dFrameStore);
	  
      m_outputTransferFunction->inverse(m_pDFrameStore[0], m_dFrameStore);
      m_colorTransform->process(m_pDFrameStore[1], m_pDFrameStore[0]);
      
      if (!(output->m_iConstantLuminance != 0 && (output->m_colorSpace == CM_YCbCr || output->m_colorSpace == CM_ICtCp))) {
        // Apply transfer function
        if ( m_useSingleTransferStep == FALSE ) {
          processFrame = m_pFrameStore[1];
          m_normalizeFunction->inverse(processFrame, currentFrame);
          currentFrame = processFrame;
          processFrame = m_pFrameStore[2];
          m_outDisplayGammaAdjust->inverse(currentFrame);
          m_outputTransferFunction->inverse (processFrame, currentFrame);
        }
        else {
          processFrame = m_pFrameStore[2];
          m_outDisplayGammaAdjust->inverse(currentFrame);
          m_outputTransferFunction->inverse(processFrame, currentFrame);
        }
        currentFrame = processFrame;
      } 
      else {
        processFrame = m_pFrameStore[1];
        m_normalizeFunction->inverse(processFrame, currentFrame);
        currentFrame = processFrame;      
      }
      // Output to m_pFrameStore[0] memory with appropriate color space conversion
      processFrame = m_pFrameStore[0];
      m_colorTransform->process(processFrame, currentFrame);
      currentFrame = processFrame;   
      
      processFrame = m_pFrameStore[3];
      processFrame->copy(currentFrame, Y_COMP);      
      currentFrame = processFrame;   
      
      currentFrame->copy(m_pDFrameStore[1], U_COMP);
      currentFrame->copy(m_pDFrameStore[1], V_COMP);
            
      // Now perform appropriate conversion to the output format (from float to fixed or vice versa)
      processFrame = m_oFrameStore;
      m_convertProcess->process(processFrame, currentFrame);
    }
    else {  //  (m_linearDownConversion == FALSE)
      if (m_rgbDownConversion == TRUE) {
        if (!(output->m_iConstantLuminance != 0 && (output->m_colorSpace == CM_YCbCr || output->m_colorSpace == CM_ICtCp))) {
          // Apply transfer function
          if ( m_useSingleTransferStep == FALSE ) {
            processFrame = m_pFrameStore[1];
            m_normalizeFunction->inverse(processFrame, currentFrame);
            currentFrame = processFrame;
            processFrame = m_pFrameStore[2];
            m_outDisplayGammaAdjust->inverse(currentFrame);
            m_outputTransferFunction->inverse (processFrame, currentFrame);
          }
          else {
            processFrame = m_pFrameStore[2];
            m_outDisplayGammaAdjust->inverse(currentFrame);
            m_outputTransferFunction->inverse(processFrame, currentFrame);
          }
          currentFrame = processFrame;
        } 
        else {
          processFrame = m_pFrameStore[1];
          m_normalizeFunction->inverse(processFrame, currentFrame);
          currentFrame = processFrame;      
        }

        // At this stage also create the downconverted RGB frame
        m_chromaScale->process(m_dFrameStore, currentFrame);
        // And the YCbCr downscaled
        m_colorTransform->process(m_pDFrameStore[0], m_dFrameStore);

        // Output to m_pFrameStore[0] memory with appropriate color space conversion
        processFrame = m_pFrameStore[0];
        m_colorTransform->process(processFrame, currentFrame);
        currentFrame = processFrame;   
                
        processFrame = m_pFrameStore[3];
        
        processFrame->copy(currentFrame, Y_COMP);      
        currentFrame = processFrame;   
        
        currentFrame->copy(m_pDFrameStore[0], U_COMP);
        currentFrame->copy(m_pDFrameStore[0], V_COMP);
        
        processFrame = m_oFrameStore;
        
        m_convertProcess->process(processFrame, currentFrame);        
      }
      else {
        if (!(output->m_iConstantLuminance != 0 && (output->m_colorSpace == CM_YCbCr || output->m_colorSpace == CM_ICtCp))) {
          // Apply transfer function
          if ( m_useSingleTransferStep == FALSE ) {
            processFrame = m_pFrameStore[1];
            m_normalizeFunction->inverse(processFrame, currentFrame);
            currentFrame = processFrame;
            processFrame = m_pFrameStore[2];
            m_outDisplayGammaAdjust->inverse(currentFrame);
            m_outputTransferFunction->inverse (processFrame, currentFrame);
          }
          else {
            processFrame = m_pFrameStore[2];
            m_outDisplayGammaAdjust->inverse(currentFrame);
//KW_KYH	1.inverse EOTF
//영상 클래스 Frame
//신호값 접근 멤버 변수 m_floatData[]
//LUT_KYH(m_floatData[index]) 의 형식을 가지도록 함수 설계
            m_outputTransferFunction->inverse(processFrame, currentFrame);
          }
          currentFrame = processFrame;
        } 
        else {
          processFrame = m_pFrameStore[1];
          m_normalizeFunction->inverse(processFrame, currentFrame);
          currentFrame = processFrame;      
        }
        currentFrame->m_hasAlternate = TRUE;
        currentFrame->m_altFrame = linearFrame;  
        currentFrame->m_altFrameNorm = inputParams->m_outNormalScale;      

        // Output to m_pFrameStore[0] memory with appropriate color space conversion
        processFrame = m_pFrameStore[0];

//KW_KYH	2. Color conersion: R’G’B’ to *NCL Y’CbCr
//m_transform0 RGB -> Y 변환 상수
//m_transform1 RGB -> Cb 변환 상수
//m_transform2 RGB -> Cr  변환 상수
        m_colorTransform->process(processFrame, currentFrame);

        currentFrame = processFrame;   
                
        if (m_oFrameStore->m_chromaFormat != currentFrame->m_chromaFormat) {
          // Now perform appropriate conversion to the output format (from float to fixed or vice versa)
          processFrame = m_pFrameStore[3];
          if (m_filterInFloat == TRUE) {
            m_convertTo420->process  (processFrame, currentFrame);

            currentFrame = processFrame;
            processFrame = m_oFrameStore;
                        
            m_convertProcess->process(processFrame, currentFrame);        
          }
          else {

//KW_KYH	3. Float(16 bits) to Quant 10bits integer
            m_convertProcess->process(processFrame, currentFrame);

            currentFrame = processFrame;
            processFrame = m_oFrameStore;
            m_convertTo420->process  (processFrame, currentFrame);
          }
        }
        else {
          // Now perform appropriate conversion to the output format (from float to fixed or vice versa)
          processFrame = m_oFrameStore;
         
          m_convertProcess->process(processFrame, currentFrame);
        }
      }
    }

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
void HDRConvertEXR::outputHeader(ProjectParameters *inputParams) {
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

void HDRConvertEXR::outputFooter(ProjectParameters *inputParams) {
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

