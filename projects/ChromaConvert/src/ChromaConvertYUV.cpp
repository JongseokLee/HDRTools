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
 * \file ChromaConvertYUV.cpp
 *
 * \brief
 *    ChromaConvertYUV class source files for performing HDR (YUV) format conversion
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

#include <time.h>
#include <string.h>
#include <math.h>
#include "ChromaConvertYUV.H"


ChromaConvertYUV::ChromaConvertYUV(ProjectParameters *inputParams) {
  m_oFrameStore       = NULL;
  m_pFrameStore       = NULL;
  m_iFrameStore       = NULL;
  
  m_inputFrame        = NULL;
  m_outputFrame       = NULL;

  m_convertFormat     = NULL;
  m_convertProcess    = NULL;

  
  m_inputFile         = &inputParams->m_inputFile;
  m_outputFile        = &inputParams->m_outputFile;
  m_startFrame        =  m_inputFile->m_startFrame;
}

//-----------------------------------------------------------------------------
// allocate memory - create objects
//-----------------------------------------------------------------------------
void ChromaConvertYUV::init (ProjectParameters *inputParams) {
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
  
  // Output file
  IOFunctions::openFile (m_outputFile, OPENFLAGS_WRITE, OPEN_PERMISSIONS);

  // Create output file
  m_outputFrame = Output::create(m_outputFile, output);
  
  // create frame memory as necessary
  // Input. This has the same format as the Input file.
  m_iFrameStore  = new Frame(m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_iFrameStore->clear();

  m_pFrameStore  = new Frame(m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP], m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
  m_pFrameStore->clear();

  m_oFrameStore  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], output->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_oFrameStore->clear();
  
  // Bit-depth conversion
  m_convertProcess = Convert::create(input, output);

  // Chroma subsampling
  // We may wish to create a single convert class that uses as inputs the output resolution as well the input and output chroma format, and the downsampling/upsampling method. That would make the code easier to handle.
  // To be done later.
  if (m_inputFrame->m_chromaFormat == output->m_chromaFormat) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, 0, m_inputFrame->m_chromaLocation, output->m_chromaLocation);
  }
  else if (m_inputFrame->m_chromaFormat == CF_444 && output->m_chromaFormat == CF_420) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, inputParams->m_chromaDownsampleFilter, m_inputFrame->m_chromaLocation, output->m_chromaLocation);
  }
  else if (m_inputFrame->m_chromaFormat == CF_422 && output->m_chromaFormat == CF_420) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, inputParams->m_chromaDownsampleFilter, m_inputFrame->m_chromaLocation, output->m_chromaLocation);
  }
  else if (m_inputFrame->m_chromaFormat == CF_420 && output->m_chromaFormat == CF_444) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, inputParams->m_chromaUpsampleFilter, m_inputFrame->m_chromaLocation, output->m_chromaLocation);
  }
  else if (m_inputFrame->m_chromaFormat == CF_420 && output->m_chromaFormat == CF_422) {
    m_convertFormat = ConvertColorFormat::create(output->m_width[Y_COMP], output->m_height[Y_COMP], m_inputFrame->m_chromaFormat, output->m_chromaFormat, inputParams->m_chromaUpsampleFilter, m_inputFrame->m_chromaLocation, output->m_chromaLocation);
  }
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------

void ChromaConvertYUV::destroy() {
  if (m_convertFormat != NULL){
    delete m_convertFormat;
    m_convertFormat = NULL;
  }
  if (m_convertProcess != NULL){
    delete m_convertProcess;
    m_convertProcess = NULL;
  }
  
  // output frame objects
  if (m_oFrameStore != NULL) {
    delete m_oFrameStore;
    m_oFrameStore = NULL;
  }
  // output frame objects
  if (m_pFrameStore != NULL) {
    delete m_pFrameStore;
    m_pFrameStore = NULL;
  }
  // input frame objects
  if (m_iFrameStore != NULL) {
    delete m_iFrameStore;
    m_iFrameStore = NULL;
  }
  
  if (m_inputFrame != NULL) {
    delete m_inputFrame;
    m_inputFrame = NULL;
  }
  
  if (m_outputFrame != NULL) {
    delete m_outputFrame;
    m_outputFrame = NULL;
  }
  
  
  IOFunctions::closeFile(m_inputFile);
  IOFunctions::closeFile(m_outputFile);
}

//-----------------------------------------------------------------------------
// main filtering function
//-----------------------------------------------------------------------------

void ChromaConvertYUV::process( ProjectParameters *inputParams ) {
  int frameNumber;
  int iCurrentFrameToProcess = 0;
  float fDistance0 = inputParams->m_source.m_frameRate / inputParams->m_output.m_frameRate;

  clock_t clk;  
  bool errorRead = FALSE;

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
    
    // Perform chroma conversion if needed
    m_convertFormat->process (m_pFrameStore, m_iFrameStore);
    // perform bit-depth conversion if needed
    m_convertProcess->process(m_oFrameStore, m_pFrameStore);

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
void ChromaConvertYUV::outputHeader(ProjectParameters *inputParams) {
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

void ChromaConvertYUV::outputFooter(ProjectParameters *inputParams) {
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

