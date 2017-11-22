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
 * \file GamutTestFrame.cpp
 *
 * \brief
 *    GamutTestFrame class for analyzing the color gamut of a video sequence
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

#include <time.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "GamutTestFrame.H"

//-----------------------------------------------------------------------------
// constructor /de-constructor
//-----------------------------------------------------------------------------


GamutTestFrame::GamutTestFrame(ProjectParameters *inputParams)
{
  m_frameStore       = NULL;
  m_cropFrameStore   = NULL;
  m_inputFrame       = NULL;
  m_inputFile        = &inputParams->m_inputFile;
  m_startFrame       = m_inputFile->m_startFrame;
  m_cropOffsetLeft   = inputParams->m_cropOffsetLeft  ;
  m_cropOffsetTop    = inputParams->m_cropOffsetTop   ;
  m_cropOffsetRight  = inputParams->m_cropOffsetRight ;
  m_cropOffsetBottom = inputParams->m_cropOffsetBottom;
  
  m_analyzeGamut     = new AnalyzeGamut();

}

GamutTestFrame::~GamutTestFrame()
{
  if (m_frameStore != NULL) {
    delete [] m_frameStore;
    m_frameStore = NULL;
  }
  if (m_cropFrameStore != NULL) {
    delete [] m_cropFrameStore;
    m_cropFrameStore = NULL;
  }  
  if (m_inputFrame != NULL) {
    delete [] m_inputFrame;
    m_inputFrame = NULL;
  }

  if (m_analyzeGamut != NULL) {
    delete m_analyzeGamut;
    m_analyzeGamut = NULL;
  }
}

//-----------------------------------------------------------------------------
// allocate memory - create frame stores
//-----------------------------------------------------------------------------
void GamutTestFrame::allocateFrameStores(Input *inputFrame, Frame **frameStore)
{
  // delete frame store if previous allocated
  if ((*frameStore) != NULL)
    delete (*frameStore);
  
  // create frame memory as necessary
  // Input. This has the same format as the Input file.
  (*frameStore)  = new Frame(inputFrame->m_width[Y_COMP], inputFrame->m_height[Y_COMP], inputFrame->m_isFloat, inputFrame->m_colorSpace, inputFrame->m_colorPrimaries, inputFrame->m_chromaFormat, inputFrame->m_sampleRange, inputFrame->m_bitDepthComp[Y_COMP], inputFrame->m_isInterlaced, inputFrame->m_transferFunction, inputFrame->m_systemGamma);
  (*frameStore)->clear();
  
  
}

//-----------------------------------------------------------------------------
// initialize
//-----------------------------------------------------------------------------
void GamutTestFrame::init (ProjectParameters *inputParams)
{
  int width, height;
  
  // Input frame objects initialization
  IOFunctions::openFile (m_inputFile);
  
  if (m_inputFile->m_isConcatenated == FALSE && strlen(m_inputFile->m_fTail) == 0) {
    // Update number of frames to be processed
    inputParams->m_numberOfFrames = 1;
  }
  
  // create memory for reading the input filesource
  
  m_inputFrame = Input::create(m_inputFile, &inputParams->m_source, inputParams);
  
  if (m_inputFile->m_videoType == VIDEO_YUV)
    allocateFrameStores(m_inputFrame, &m_frameStore);
  else
  {
    // Read first frame just to see if there is need to update any parameters. This is very important for OpenEXR files
    if (m_inputFrame->readOneFrame(m_inputFile, 0, m_inputFile->m_fileHeader, m_startFrame) == TRUE) {
      allocateFrameStores(m_inputFrame, &m_frameStore);
    }
    else {
      fprintf(stderr, "Error reading input file. Process exiting.\n");
      destroy();
      exit(EXIT_FAILURE);
    }
  }
  
  if (m_cropOffsetLeft != 0 || m_cropOffsetTop != 0 || m_cropOffsetRight != 0 || m_cropOffsetBottom != 0) {
    width  = m_inputFrame->m_width[Y_COMP]  - m_cropOffsetLeft + m_cropOffsetRight;
    height = m_inputFrame->m_height[Y_COMP] - m_cropOffsetTop  + m_cropOffsetBottom;
    m_cropFrameStore  = new Frame(width, height, m_inputFrame->m_isFloat, m_inputFrame->m_colorSpace, m_inputFrame->m_colorPrimaries, m_inputFrame->m_chromaFormat, m_inputFrame->m_sampleRange, m_inputFrame->m_bitDepthComp[Y_COMP], m_inputFrame->m_isInterlaced, m_inputFrame->m_transferFunction, m_inputFrame->m_systemGamma);
    m_cropFrameStore->clear();
  }
  else {
    width  = m_inputFrame->m_width[Y_COMP];
    height = m_inputFrame->m_height[Y_COMP];
  }
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------
void GamutTestFrame::destroy()
{
    if (m_frameStore != NULL) {
      delete m_frameStore;
      m_frameStore = NULL;
    }
    
    if (m_cropFrameStore != NULL) {
      delete m_cropFrameStore;
      m_cropFrameStore = NULL;
    }
    
    if (m_inputFrame != NULL) {
      delete m_inputFrame;
      m_inputFrame = NULL;
    }
    IOFunctions::closeFile(m_inputFile);
}

//-----------------------------------------------------------------------------
// main filtering function
//-----------------------------------------------------------------------------
void GamutTestFrame::process( ProjectParameters *inputParams ) {
  int frameNumber;
  int iCurrentFrameToProcess = 0;
  //float fDistance0 = inputParams->m_source[0].m_frameRate / inputParams->m_source[1].m_frameRate;
  Frame *  currentFrame;
  
  clock_t clk;  
  bool errorRead = FALSE;

  // Now process all frames
  for (frameNumber = 0; frameNumber < inputParams->m_numberOfFrames; frameNumber ++) {
    clk = clock();
    // read frames
      // Current frame to process depends on frame rate of current sequence versus frame rate of the first sequence
      iCurrentFrameToProcess = int(frameNumber * inputParams->m_source.m_frameRate / inputParams->m_source.m_frameRate);
      if (m_inputFrame->readOneFrame(m_inputFile, iCurrentFrameToProcess, m_inputFile->m_fileHeader, m_startFrame) == TRUE) {
        // If the size of the images has changed, then reallocate space appropriately
        if ((m_inputFrame->m_width[Y_COMP] != m_frameStore->m_width[Y_COMP]) || (m_inputFrame->m_height[Y_COMP] != m_frameStore->m_height[Y_COMP])) {
          allocateFrameStores(m_inputFrame, &m_frameStore);
        }
        
        // Now copy input frame buffer to processing frame buffer for any subsequent processing
        m_frameStore->m_frameNo = frameNumber;
        m_inputFrame->copyFrame(m_frameStore);
        currentFrame = m_frameStore;
        if (m_cropFrameStore != NULL) {
          m_cropFrameStore->copy(m_frameStore, m_cropOffsetLeft, m_cropOffsetTop, m_frameStore->m_width[Y_COMP] + m_cropOffsetRight, m_frameStore->m_height[Y_COMP] + m_cropOffsetBottom, 0, 0);
          currentFrame = m_cropFrameStore;
        }
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
      printf("%06d ", frameNumber );
    }
    m_analyzeGamut->process(currentFrame);
    m_analyzeGamut->reportMetric();
    
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
void GamutTestFrame::outputHeader(ProjectParameters *inputParams) {
  printf("================================================================================================================\n");
    printf("Input = %s\n", inputParams->m_inputFile.m_fName);
    
    printf("W x H:  (%dx%d) ~ (%dx%d - %dx%d) => (%dx%d)\n", m_inputFrame->m_width[Y_COMP], m_inputFrame->m_height[Y_COMP],
           m_cropOffsetLeft, m_cropOffsetTop, m_cropOffsetRight, m_cropOffsetBottom,
           m_inputFrame->m_width[Y_COMP] - m_cropOffsetLeft + m_cropOffsetRight, m_inputFrame->m_height[Y_COMP] - m_cropOffsetTop + m_cropOffsetBottom);
    if (m_inputFrame->m_isFloat == TRUE)
      printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, Type: %s\n", COLOR_FORMAT[m_inputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_inputFrame->m_isInterlaced], COLOR_SPACE[m_inputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_inputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_inputFrame->m_transferFunction], m_inputFrame->m_frameRate, PIXEL_TYPE[m_inputFrame->m_pixelType[Y_COMP]]);
    else
      printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, BitDepth: %d, Range: %s\n", COLOR_FORMAT[m_inputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_inputFrame->m_isInterlaced], COLOR_SPACE[m_inputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_inputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_inputFrame->m_transferFunction], m_inputFrame->m_frameRate, m_inputFrame->m_bitDepthComp[Y_COMP], SOURCE_RANGE_TYPE[m_inputFrame->m_sampleRange + 1]);
  printf("================================================================================================================\n");

  
  if (inputParams->m_silentMode == FALSE) {
    // Separators for 'Frame#  Time'
    m_analyzeGamut->printSeparator();
    printf("---------------\n");

    printf("Frame#  ");  // 8
    m_analyzeGamut->printHeader();
    printf("  Time "); // 7
    // Separators for 'Frame#  Time'
    printf("\n---------------");
    m_analyzeGamut->printSeparator();
    printf("\n");
  }

}

//-----------------------------------------------------------------------------
// footer output
//-----------------------------------------------------------------------------
void GamutTestFrame::outputFooter(ProjectParameters *inputParams) {
  clock_t clk = clock();
  FILE* f = IOFunctions::openFile(inputParams->m_logFile, "at");

  if (f != NULL) {
    fprintf(f, "%s ", inputParams->m_inputFile.m_fName);
    fprintf(f, "%d ", inputParams->m_numberOfFrames);
    fprintf(f, "%f \n", 1.0 * clk / inputParams->m_numberOfFrames / CLOCKS_PER_SEC);
  }
  //if (inputParams->m_silentMode == FALSE){
  
    // Separators for 'Frame#  Time'
    m_analyzeGamut->printSeparator();
    printf("---------------\n");
  
    
    //printf("%06d ", inputParams->m_numberOfFrames);
    printf("D_Avg  ");
    m_analyzeGamut->reportSummary();

    printf("%7.3f", clk / ((float) inputParams->m_numberOfFrames * (float) CLOCKS_PER_SEC));
    printf("\n---------------");
    m_analyzeGamut->printSeparator();

    // Minimum
    printf("\nD_Min  ");
    m_analyzeGamut->reportMinimum();
    // Minimum
    printf("\nD_Max  ");
    m_analyzeGamut->reportMaximum();
  
  // Separators for 'Frame#  Time'
  printf("\n---------------");
    m_analyzeGamut->printSeparator();

  
  printf("\nTotal of %d frames processed in %5.3f seconds\n",inputParams->m_numberOfFrames, 1.0 * clk / CLOCKS_PER_SEC);
  printf("Processing Speed: %3.2ffps\n", (float) inputParams->m_numberOfFrames * CLOCKS_PER_SEC / clk);
  m_analyzeGamut->printSeparator();
  printf("---------------\n");

  IOFunctions::closeFile(f);
}

