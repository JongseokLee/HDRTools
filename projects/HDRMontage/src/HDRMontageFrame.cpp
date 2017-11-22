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
 * \file HDRMontageFrame.cpp
 *
 * \brief
 *    HDRMontageFrame class source files for performing HDR (EXR) format conversion
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

#include <assert.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "HDRMontageFrame.H"

//-----------------------------------------------------------------------------
// constructor /de-constructor
//-----------------------------------------------------------------------------


HDRMontageFrame::HDRMontageFrame(ProjectParameters *inputParams)
{
  
  if (inputParams->m_input0Only) {
    // Only use a single input
    m_numberOfClips = 1;
  }
  else {
    m_numberOfClips = 2;
  }

  m_frameStore       = new Frame*         [m_numberOfClips];
  m_cropFrameStore   = new Frame*         [m_numberOfClips];
  m_inputFrame       = new Input*         [m_numberOfClips];

  m_inputFile        = new IOVideo*       [m_numberOfClips];
  m_startFrame       = new int            [m_numberOfClips];
  m_cropOffsetLeft   = new int            [m_numberOfClips];
  m_cropOffsetTop    = new int            [m_numberOfClips];
  m_cropOffsetRight  = new int            [m_numberOfClips];
  m_cropOffsetBottom = new int            [m_numberOfClips];
  
  for (int index = 0; index < m_numberOfClips; index++) {
    m_frameStore[index]       = NULL;
    m_cropFrameStore[index]   = NULL;
    m_inputFrame[index]       = NULL;
    m_inputFile[index]        = &inputParams->m_inputFile[index];
    m_startFrame[index]       = m_inputFile[index]->m_startFrame;
    m_cropOffsetLeft[index]   = inputParams->m_cropOffsetLeft  [index];
    m_cropOffsetTop[index]    = inputParams->m_cropOffsetTop   [index];
    m_cropOffsetRight[index]  = inputParams->m_cropOffsetRight [index];
    m_cropOffsetBottom[index] = inputParams->m_cropOffsetBottom[index];
    m_destMinPosX[index]      = inputParams->m_destMinPosX     [index];
    m_destMaxPosX[index]      = inputParams->m_destMaxPosX     [index];
    m_destMinPosY[index]      = inputParams->m_destMinPosY     [index];
    m_destMaxPosY[index]      = inputParams->m_destMaxPosY     [index];
    m_inputFlip[index]        = inputParams->m_inputFlip       [index];
  }
  
  m_outputFrame      = NULL;
  m_oFrameStore       = NULL;
  m_outputFile       = &inputParams->m_outputFile;
  
  m_windowWidth   = 0;
  m_windowHeight  = 0;
}

HDRMontageFrame::~HDRMontageFrame()
{
  if (m_frameStore != NULL) {
    delete [] m_frameStore;
    m_frameStore = NULL;
  }
  
  // output frame objects
  if (m_oFrameStore != NULL) {
    delete m_oFrameStore;
    m_oFrameStore = NULL;
  }

  if (m_cropFrameStore != NULL) {
    delete [] m_cropFrameStore;
    m_cropFrameStore = NULL;
  }
  if (m_inputFrame != NULL) {
    delete [] m_inputFrame;
    m_inputFrame = NULL;
  }
  if (m_inputFile != NULL) {
    delete [] m_inputFile;
    m_inputFile = NULL;
  }
  if (m_startFrame != NULL) {
    delete [] m_startFrame;
    m_startFrame = NULL;
  }

  // Cropping parameters
  if (m_cropOffsetLeft != NULL) {
    delete [] m_cropOffsetLeft;
    m_cropOffsetLeft = NULL;
  }
  if (m_cropOffsetTop != NULL) {
    delete [] m_cropOffsetTop;
    m_cropOffsetTop = NULL;
  }
  if (m_cropOffsetRight != NULL) {
    delete [] m_cropOffsetRight;
    m_cropOffsetRight = NULL;
  }
  if (m_cropOffsetBottom != NULL) {
    delete [] m_cropOffsetBottom;
    m_cropOffsetBottom = NULL;
  }
  
}

//-----------------------------------------------------------------------------
// allocate memory - create frame stores
//-----------------------------------------------------------------------------
void HDRMontageFrame::allocateFrameStores(Input *inputFrame, Frame **frameStore, FrameFormat   *output)
{
  //int width, height;

  //width  = inputFrame->m_width[Y_COMP];
  //height = inputFrame->m_height[Y_COMP];

  // delete frame store if previous allocated
  if ((*frameStore) != NULL)
    delete (*frameStore);
  
  // create frame memory as necessary
  // Input. This has the same format as the Input file.
  (*frameStore)  = new Frame(inputFrame->m_width[Y_COMP], inputFrame->m_height[Y_COMP], inputFrame->m_isFloat, inputFrame->m_colorSpace, inputFrame->m_colorPrimaries, inputFrame->m_chromaFormat, inputFrame->m_sampleRange, inputFrame->m_bitDepthComp[Y_COMP], inputFrame->m_isInterlaced, inputFrame->m_transferFunction, inputFrame->m_systemGamma);
  (*frameStore)->clear();
  
  // Create output file
  IOFunctions::openFile (m_outputFile, OPENFLAGS_WRITE, OPEN_PERMISSIONS);

  m_outputFile->m_format.m_height[Y_COMP] = output->m_height[Y_COMP];
  m_outputFile->m_format.m_width [Y_COMP] = output->m_width [Y_COMP];
  
  m_outputFrame = Output::create(m_outputFile, output);
  
  m_oFrameStore  = new Frame(output->m_width[Y_COMP], output->m_height[Y_COMP], output->m_isFloat, output->m_colorSpace, output->m_colorPrimaries, output->m_chromaFormat, output->m_sampleRange, output->m_bitDepthComp[Y_COMP], output->m_isInterlaced, output->m_transferFunction, output->m_systemGamma);
  m_oFrameStore->clear();

}

//-----------------------------------------------------------------------------
// initialize
//-----------------------------------------------------------------------------
void HDRMontageFrame::init (ProjectParameters *inputParams)
{
  FrameFormat   *output = &inputParams->m_output;

  vector <int>  width (m_numberOfClips);
  vector <int>  height(m_numberOfClips);
  
  for (int index = 0; index < m_numberOfClips; index++) {
    // Input frame objects initialization
    IOFunctions::openFile (m_inputFile[index]);
    
    if (m_inputFile[index]->m_isConcatenated == FALSE && strlen(m_inputFile[index]->m_fTail) == 0) {
      // Update number of frames to be processed
      inputParams->m_numberOfFrames = 1;
    }
    
    // create memory for reading the input filesource
    
    m_inputFrame[index] = Input::create(m_inputFile[index], &inputParams->m_source[index], inputParams);
    
    if (m_inputFile[index]->m_videoType == VIDEO_YUV)
      allocateFrameStores(m_inputFrame[index], &m_frameStore[index], output);
    else
    {
      // Read first frame just to see if there is need to update any parameters. This is very important for OpenEXR files
      if (m_inputFrame[index]->readOneFrame(m_inputFile[index], 0, m_inputFile[index]->m_fileHeader, m_startFrame[index]) == TRUE) {
        allocateFrameStores(m_inputFrame[index], &m_frameStore[index], output);
      }
      else {
        fprintf(stderr, "Error reading input file. Process exiting.\n");
        destroy();
        exit(EXIT_FAILURE);
      }
    }
    
    if (m_cropOffsetLeft[index] != 0 || m_cropOffsetTop[index] != 0 || m_cropOffsetRight[index] != 0 || m_cropOffsetBottom[index] != 0) {
      width[index]  = m_inputFrame[index]->m_width[Y_COMP]  - m_cropOffsetLeft[index] + m_cropOffsetRight[index];
      height[index] = m_inputFrame[index]->m_height[Y_COMP] - m_cropOffsetTop[index]  + m_cropOffsetBottom[index];
      m_cropFrameStore[index]  = new Frame(width[index], height[index], m_inputFrame[index]->m_isFloat, m_inputFrame[index]->m_colorSpace, m_inputFrame[index]->m_colorPrimaries, m_inputFrame[index]->m_chromaFormat, m_inputFrame[index]->m_sampleRange, m_inputFrame[index]->m_bitDepthComp[Y_COMP], m_inputFrame[index]->m_isInterlaced, m_inputFrame[index]->m_transferFunction, m_inputFrame[index]->m_systemGamma);
      m_cropFrameStore[index]->clear();
    }
    else {
      width[index]  = m_inputFrame[index]->m_width[Y_COMP];
      height[index] = m_inputFrame[index]->m_height[Y_COMP];
    }
  }
  
  if (m_numberOfClips > 1 && m_frameStore[0]->equalType(m_frameStore[1]) == FALSE) {
      fprintf(stderr, "Error. Input sources of different type.\n");
      destroy();
      exit(EXIT_FAILURE);
  }
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------
void HDRMontageFrame::destroy()
{
  for (int index = 0; index < m_numberOfClips; index++) {
    if (m_frameStore[index] != NULL) {
      delete m_frameStore[index];
      m_frameStore[index] = NULL;
    }
    
    if (m_cropFrameStore[index] != NULL) {
      delete m_cropFrameStore[index];
      m_cropFrameStore[index] = NULL;
    }
    
    if (m_inputFrame[index] != NULL) {
      delete m_inputFrame[index];
      m_inputFrame[index] = NULL;
    }
    if (m_outputFrame != NULL) {
      delete m_outputFrame;
      m_outputFrame = NULL;
    }

    IOFunctions::closeFile(m_inputFile[index]);
    IOFunctions::closeFile(m_outputFile);
  }
}

//-----------------------------------------------------------------------------
// main filtering function
//-----------------------------------------------------------------------------
void HDRMontageFrame::process( ProjectParameters *inputParams ) {
  int frameNumber;
  int iCurrentFrameToProcess = 0;
  //float fDistance0 = inputParams->m_source[0].m_frameRate / inputParams->m_source[1].m_frameRate;
  vector <Frame *>  currentFrame(m_numberOfClips);
  int index;
  
  clock_t clk;  
  bool errorRead = FALSE;

  // Now process all frames
  for (frameNumber = 0; frameNumber < inputParams->m_numberOfFrames; frameNumber ++) {
    clk = clock();
    // read frames
    for (index = 0; index < m_numberOfClips; index++) {
      // Current frame to process depends on frame rate of current sequence versus frame rate of the first sequence
      iCurrentFrameToProcess = int(frameNumber * inputParams->m_source[index].m_frameRate / inputParams->m_source[0].m_frameRate);
      if (m_inputFrame[index]->readOneFrame(m_inputFile[index], iCurrentFrameToProcess, m_inputFile[index]->m_fileHeader, m_startFrame[index]) == TRUE) {
        FrameFormat   *output = &inputParams->m_output;

        // If the size of the images has changed, then reallocate space appropriately
        if ((m_inputFrame[index]->m_width[Y_COMP] != m_frameStore[index]->m_width[Y_COMP]) || (m_inputFrame[index]->m_height[Y_COMP] != m_frameStore[index]->m_height[Y_COMP])) {
          allocateFrameStores(m_inputFrame[index], &m_frameStore[index], output);
        }
        
        // Now copy input frame buffer to processing frame buffer for any subsequent processing
        m_frameStore[index]->m_frameNo = frameNumber;
        m_inputFrame[index]->copyFrame(m_frameStore[index]);
        currentFrame[index] = m_frameStore[index];
        // Not the most elegant way of copying. Currently we do this process in two steps.
        // Need to make a function that does this in a single step, thus being faster and easier to understand
        if (m_cropFrameStore[index] != NULL) {
          m_cropFrameStore[index]->copy(m_frameStore[index], m_cropOffsetLeft[index], m_cropOffsetTop[index], m_frameStore[index]->m_width[Y_COMP] + m_cropOffsetRight[index], m_frameStore[index]->m_height[Y_COMP] + m_cropOffsetBottom[index], 0, 0);
          currentFrame[index] = m_cropFrameStore[index];
        }
        m_oFrameStore->copy(currentFrame[index], 0, 0, m_destMaxPosX[index] - m_destMinPosX[index], m_destMaxPosY[index] - m_destMinPosY[index], m_destMinPosX[index], m_destMinPosY[index], m_inputFlip[index]);
      }
      else {
        inputParams->m_numberOfFrames = frameNumber;
        errorRead = TRUE;
        break;
      }
    }

    
    if (m_numberOfClips > 1 && currentFrame[0]->equalType(currentFrame[1]) == FALSE) {
      fprintf(stderr, "Error. Input sources of different type.\n");
      errorRead = TRUE;
      break;
    }
    
    if (errorRead == TRUE) {
      break;
    }
    else if (inputParams->m_silentMode == FALSE) {
      printf("%06d ", frameNumber );
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
void HDRMontageFrame::outputHeader(ProjectParameters *inputParams) {
  IOVideo *OutFile = &inputParams->m_outputFile;

  printf("================================================================================================================\n");
  for (int index = 0; index < m_numberOfClips; index++) {
    printf("Input%d: %s\n", index, inputParams->m_inputFile[index].m_fName);
    
    printf("W x H:  (%dx%d) ~ (%dx%d - %dx%d) => (%dx%d)\n", m_inputFrame[index]->m_width[Y_COMP], m_inputFrame[index]->m_height[Y_COMP],
           m_cropOffsetLeft[index], m_cropOffsetTop[index], m_cropOffsetRight[index], m_cropOffsetBottom[index],
           m_inputFrame[index]->m_width[Y_COMP] - m_cropOffsetLeft[index] + m_cropOffsetRight[index], m_inputFrame[index]->m_height[Y_COMP] - m_cropOffsetTop[index] + m_cropOffsetBottom[index]);
           
    if (m_inputFrame[index]->m_isFloat == TRUE)
      printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, Type: %s\n", COLOR_FORMAT[m_inputFrame[index]->m_chromaFormat + 1], INTERLACED_TYPE[m_inputFrame[index]->m_isInterlaced], COLOR_SPACE[m_inputFrame[index]->m_colorSpace + 1], COLOR_PRIMARIES[m_inputFrame[index]->m_colorPrimaries + 1], TRANSFER_CHAR[m_inputFrame[index]->m_transferFunction], m_inputFrame[index]->m_frameRate, PIXEL_TYPE[m_inputFrame[index]->m_pixelType[Y_COMP]]);
    else
      printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, BitDepth: %d, Range: %s\n", COLOR_FORMAT[m_inputFrame[index]->m_chromaFormat + 1], INTERLACED_TYPE[m_inputFrame[index]->m_isInterlaced], COLOR_SPACE[m_inputFrame[index]->m_colorSpace + 1], COLOR_PRIMARIES[m_inputFrame[index]->m_colorPrimaries + 1], TRANSFER_CHAR[m_inputFrame[index]->m_transferFunction], m_inputFrame[index]->m_frameRate, m_inputFrame[index]->m_bitDepthComp[Y_COMP], SOURCE_RANGE_TYPE[m_inputFrame[index]->m_sampleRange + 1]);
    printf("----------------------------------------------------------------------------------------------------------------\n");
  }
  printf("----------------------------------------------------------------------------------------------------------------\n");
  printf("Output: %s\n", OutFile->m_fName);
  printf("W x H:  (%dx%d) \n", m_outputFrame->m_width[Y_COMP], m_outputFrame->m_height[Y_COMP]);
  if (m_outputFrame->m_isFloat == TRUE)
    printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, Type: %s\n", COLOR_FORMAT[m_outputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_outputFrame->m_isInterlaced], COLOR_SPACE[m_outputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_outputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_outputFrame->m_transferFunction], m_outputFrame->m_frameRate, PIXEL_TYPE[m_outputFrame->m_pixelType[Y_COMP]]);
  else
    printf("Format: %s%s, ColorSpace: %s (%s/%s), FrameRate: %7.3f, BitDepth: %d, Range: %s\n", COLOR_FORMAT[m_outputFrame->m_chromaFormat + 1], INTERLACED_TYPE[m_outputFrame->m_isInterlaced], COLOR_SPACE[m_outputFrame->m_colorSpace + 1], COLOR_PRIMARIES[m_outputFrame->m_colorPrimaries + 1], TRANSFER_CHAR[m_outputFrame->m_transferFunction], m_outputFrame->m_frameRate, m_outputFrame->m_bitDepthComp[Y_COMP], SOURCE_RANGE_TYPE[m_outputFrame->m_sampleRange + 1]);
  printf("================================================================================================================\n");

  
  if (inputParams->m_silentMode == FALSE) {
    // Separators for 'Frame#  Time'
    printf("---------------\n");

    printf("Frame#  ");  // 8
    printf("  Time "); // 7
    // Separators for 'Frame#  Time'
    printf("\n");
  }

}

//-----------------------------------------------------------------------------
// footer output
//-----------------------------------------------------------------------------
void HDRMontageFrame::outputFooter(ProjectParameters *inputParams) {
  clock_t clk = clock();
  FILE* f = IOFunctions::openFile(inputParams->m_logFile, "at");

  if (f != NULL) {
    fprintf(f, "%s ", inputParams->m_inputFile[0].m_fName);
    fprintf(f, "%s ", inputParams->m_inputFile[1].m_fName);
    fprintf(f, "%d ", inputParams->m_numberOfFrames);
    fprintf(f, "%f \n", 1.0 * clk / inputParams->m_numberOfFrames / CLOCKS_PER_SEC);
  }
  
  printf("\nTotal of %d frames processed in %5.3f seconds\n",inputParams->m_numberOfFrames, 1.0 * clk / CLOCKS_PER_SEC);
  printf("Processing Speed: %3.2ffps\n", (float) inputParams->m_numberOfFrames * CLOCKS_PER_SEC / clk);

  IOFunctions::closeFile(f);
}

