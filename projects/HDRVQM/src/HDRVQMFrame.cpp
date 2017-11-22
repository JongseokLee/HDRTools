/* The copyright in this software is being made available under the BSD
* License, included below. This software may be subject to other third party
* and contributor rights, including patent rights, and no such rights are
* granted under this license.
*
* <OWNER> = Samsung Electronics, Apple Inc.
* <ORGANIZATION> = Samsung Electronics, Apple Inc.
* <YEAR> = 2016
*
* Copyright (c) 2016, Samsung Electronics, Apple Inc.
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
* \file HDRVQMFrame.cpp
*
* \brief
*    HDRVQMFrame class source files for computing VQM for a frame
*
* \author
*     - Kulbhushan Pachauri             <kb.pachauri@samsung.com>
*     - Alexis Michael Tourapis         <atourapis@apple.com>
*************************************************************************************
*/

#include <time.h>
#include <string.h>
#include <math.h>
#include <vector>
#include "HDRVQMFrame.H"

//-----------------------------------------------------------------------------
// constructor /de-constructor
//-----------------------------------------------------------------------------


HDRVQMFrame::HDRVQMFrame(ProjectParameters *inputParams)
{
  m_numberOfClips = 2;
  m_frameStore       = new Frame*         [m_numberOfClips];
  m_cropFrameStore   = new Frame*         [m_numberOfClips];
  m_windowFrameStore = new Frame*         [m_numberOfClips];
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
    m_windowFrameStore[index] = NULL;
    m_inputFrame[index]       = NULL;
    m_inputFile[index]        = &inputParams->m_inputFile[index];
    m_startFrame[index]       = m_inputFile[index]->m_startFrame;
    m_cropOffsetLeft[index]   = inputParams->m_cropOffsetLeft  [index];
    m_cropOffsetTop[index]    = inputParams->m_cropOffsetTop   [index];
    m_cropOffsetRight[index]  = inputParams->m_cropOffsetRight [index];
    m_cropOffsetBottom[index] = inputParams->m_cropOffsetBottom[index];
  }

  m_distortionMetric = new DistortionMetric*[DIST_METRICS];
  m_windowDistortionMetric = new DistortionMetric*[DIST_METRICS];

  m_enableWindow  = FALSE;
  m_windowMinPosX = 0;
  m_windowMaxPosX = 0;
  m_windowMinPosY = 0;
  m_windowMaxPosY = 0;

  m_windowWidth   = 0;
  m_windowHeight  = 0;
  // Copy input distortion parameters
  m_distortionParameters = inputParams->m_distortionParameters;

  for (int index = DIST_NULL; index < DIST_METRICS; index++) {
    m_distortionMetric[index] = NULL;
    m_windowDistortionMetric[index] = NULL;
  }
}

HDRVQMFrame::~HDRVQMFrame()
{
  if (m_frameStore != NULL) {
    delete [] m_frameStore;
    m_frameStore = NULL;
  }
  if (m_cropFrameStore != NULL) {
    delete [] m_cropFrameStore;
    m_cropFrameStore = NULL;
  }
  if (m_windowFrameStore != NULL) {
    delete [] m_windowFrameStore;
    m_windowFrameStore = NULL;
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

  // Distortion parameters
  if (m_distortionMetric != NULL) {
    delete [] m_distortionMetric;
    m_distortionMetric = NULL;
  }
  if (m_windowDistortionMetric != NULL) {
    delete [] m_windowDistortionMetric;
    m_windowDistortionMetric = NULL;
  }
}

//-----------------------------------------------------------------------------
// allocate memory - create frame stores
//-----------------------------------------------------------------------------
void HDRVQMFrame::allocateFrameStores(Input *inputFrame, Frame **frameStore)
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
void HDRVQMFrame::init (ProjectParameters *inputParams)
{
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
      allocateFrameStores(m_inputFrame[index], &m_frameStore[index]);
    else
    {
      // Read first frame just to see if there is need to update any parameters. This is very important for OpenEXR files
      if (m_inputFrame[index]->readOneFrame(m_inputFile[index], 0, m_inputFile[index]->m_fileHeader, m_startFrame[index]) == TRUE) {
        allocateFrameStores(m_inputFrame[index], &m_frameStore[index]);
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

  if (m_frameStore[0]->equalType(m_frameStore[1]) == FALSE) {
    fprintf(stderr, "Error. Input sources of different type.\n");
    destroy();
    exit(EXIT_FAILURE);
  }

  for (int index = DIST_NULL; index < DIST_METRICS; index++) {
    m_enableMetric[index] = inputParams->m_enableMetric[index];
    if (m_enableMetric[index] == TRUE)
      m_distortionMetric[index] = DistortionMetric::create(&m_frameStore[0]->m_format, index, &m_distortionParameters, FALSE);
  }

  m_enableWindow  = inputParams->m_enableWindow;

  if (m_enableWindow) {
    m_windowMinPosX = inputParams->m_windowMinPosX;
    m_windowMaxPosX = inputParams->m_windowMaxPosX;
    m_windowMinPosY = inputParams->m_windowMinPosY;
    m_windowMaxPosY = inputParams->m_windowMaxPosY;

    m_windowWidth   = iAbs(m_windowMaxPosX - m_windowMinPosX + 1);
    m_windowHeight  = iAbs(m_windowMaxPosY - m_windowMinPosY + 1);

    m_windowFrameStore[0]  = new Frame(m_windowWidth, m_windowHeight, m_inputFrame[0]->m_isFloat, m_inputFrame[0]->m_colorSpace, m_inputFrame[0]->m_colorPrimaries, m_inputFrame[0]->m_chromaFormat, m_inputFrame[0]->m_sampleRange, m_inputFrame[0]->m_bitDepthComp[Y_COMP], m_inputFrame[0]->m_isInterlaced, m_inputFrame[0]->m_transferFunction, m_inputFrame[0]->m_systemGamma);
    m_windowFrameStore[0]->clear();
    m_windowFrameStore[1]  = new Frame(m_windowWidth, m_windowHeight, m_inputFrame[1]->m_isFloat, m_inputFrame[1]->m_colorSpace, m_inputFrame[1]->m_colorPrimaries, m_inputFrame[1]->m_chromaFormat, m_inputFrame[1]->m_sampleRange, m_inputFrame[1]->m_bitDepthComp[Y_COMP], m_inputFrame[1]->m_isInterlaced,  m_inputFrame[1]->m_transferFunction, m_inputFrame[1]->m_systemGamma);
    m_windowFrameStore[1]->clear();

    for (int index = DIST_NULL; index < DIST_METRICS; index++) {
      m_enableWindowMetric[index] = inputParams->m_enableWindowMetric[index];
      if (m_enableWindowMetric[index] == TRUE)
        m_windowDistortionMetric[index] = DistortionMetric::create(&m_windowFrameStore[0]->m_format, index, &m_distortionParameters, TRUE);
    }
  }
}

//-----------------------------------------------------------------------------
// deallocate memory - destroy objects
//-----------------------------------------------------------------------------
void HDRVQMFrame::destroy()
{
  for (int index = 0; index < m_numberOfClips; index++) {
    if (m_enableWindow) {
      if (m_windowFrameStore[index] != NULL) {
        delete m_windowFrameStore[index];
        m_windowFrameStore[index] = NULL;
      }
    }
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
    IOFunctions::closeFile(m_inputFile[index]);
  }

  for (int index = DIST_NULL; index < DIST_METRICS; index++) {
    if (m_distortionMetric[index] != NULL) {
      delete m_distortionMetric[index];
      m_distortionMetric[index] = NULL;
    }
    if (m_windowDistortionMetric[index] != NULL) {
      delete m_windowDistortionMetric[index];
      m_windowDistortionMetric[index] = NULL;
    }
  }
}

//-----------------------------------------------------------------------------
// main filtering function
//-----------------------------------------------------------------------------
void HDRVQMFrame::process( ProjectParameters *inputParams ) {
  int frameNumber;
  int iCurrentFrameToProcess = 0;

  vector <Frame *>  currentFrame(m_numberOfClips);
  VQMParams *paramsVQM = &inputParams->m_distortionParameters.m_VQM;

  int index;

  clock_t clk;  
  bool errorRead = FALSE;
  bool single_pass = FALSE;										// tells if single pass is done or not
  int mult_factor;

  if(paramsVQM->m_displayAdapt == FALSE) {
    single_pass = TRUE;
    mult_factor = (int) ceil(paramsVQM->m_frameRate * paramsVQM->m_fixationTime);
  }
  else  {
    mult_factor = 1;
  }


  for (frameNumber = 0; frameNumber < inputParams->m_numberOfFrames; frameNumber = frameNumber + mult_factor) {
    for(int iters = 0; (iters < mult_factor) && (frameNumber + iters < inputParams->m_numberOfFrames); iters++) {
      clk = clock();

      for (index = 0; index < m_numberOfClips; index++) {
        // Current frame to process depends on frame rate of current sequence versus frame rate of the first sequence
        //so, frames for 1st image read for even iterations of this for-loop. Baaki 2nd ke liye
        iCurrentFrameToProcess = int((frameNumber + iters) * inputParams->m_source[index % 2].m_frameRate / inputParams->m_source[0].m_frameRate);
        if (m_inputFrame[index]->readOneFrame(m_inputFile[index % 2], iCurrentFrameToProcess, m_inputFile[index % 2]->m_fileHeader, m_startFrame[index]) == TRUE) {
          // If the size of the images has changed, then reallocate space appropriately
          if ((m_inputFrame[index]->m_width[Y_COMP] != m_frameStore[index]->m_width[Y_COMP]) || (m_inputFrame[index]->m_height[Y_COMP] != m_frameStore[index]->m_height[Y_COMP])) {
            allocateFrameStores(m_inputFrame[index], &m_frameStore[index]);
          }

          // Now copy input frame buffer to processing frame buffer for any subsequent processing
          m_frameStore[index]->m_frameNo = frameNumber + iters;
          m_inputFrame[index]->copyFrame(m_frameStore[index]);
          currentFrame[index] = m_frameStore[index];
          if (m_cropFrameStore[index] != NULL) {
            m_cropFrameStore[index]->copy(m_frameStore[index], m_cropOffsetLeft[index], m_cropOffsetTop[index], m_frameStore[index]->m_width[Y_COMP] + m_cropOffsetRight[index], m_frameStore[index]->m_height[Y_COMP] + m_cropOffsetBottom[index], 0, 0);
            currentFrame[index] = m_cropFrameStore[index];
          }
          if (m_enableWindow == TRUE) {
            m_windowFrameStore[index]->copy(currentFrame[index], m_windowMinPosX, m_windowMinPosY, m_windowMaxPosX, m_windowMaxPosY, 0, 0);
          }
        }
        else {
          inputParams->m_numberOfFrames = frameNumber + iters;
          errorRead = TRUE;
          break;
        }
      }
      if (m_enableMetric[DIST_VQM] == TRUE) {
        m_distortionMetric[DIST_VQM]->computeMetric(currentFrame[0], currentFrame[1]);
      }

      if (currentFrame[0]->equalType(currentFrame[1]) == FALSE) {
        fprintf(stderr, "Error. Input sources of different type.\n");
        errorRead = TRUE;
        break;
      }

      if (errorRead == TRUE) {
        break;
      }
      else {
        if (inputParams->m_silentMode == FALSE) {
          printf("%06d ", frameNumber + iters );
          m_distortionMetric[DIST_VQM]->reportMetric();
        }
      }

      if((frameNumber + iters) == inputParams->m_numberOfFrames - 1  && !single_pass) {
        //alright, adapt_display done. Now scale the image and compute
        m_distortionMetric[DIST_VQM]->m_allFrames = true;
        //m_distortionMetric[DIST_VQM]->computeMetric(currentFrame[0], currentFrame[1]);
        //change this too. Mult_factor is for running the for-loop for 15frames to account for pooling
        
        single_pass = true;
        frameNumber = -1;
      }
      if (inputParams->m_silentMode == FALSE){
        clk = clock() - clk;
        printf("%7.3f", 1.0 * clk / CLOCKS_PER_SEC);
        printf("\n");
        fflush(stdout);
      }
      else {
        printf("Processing Frame : %d\r", frameNumber);
        fflush(stdout);
      }
    }
    if(single_pass)
    {
      mult_factor = (int) ceil(paramsVQM->m_frameRate * paramsVQM->m_fixationTime);
      if(frameNumber%mult_factor != 0)
        frameNumber = -mult_factor;
    }
  }
}

//-----------------------------------------------------------------------------
// header output
//-----------------------------------------------------------------------------
void HDRVQMFrame::outputHeader(ProjectParameters *inputParams) {
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
    if (index < m_numberOfClips - 1) {
      printf("----------------------------------------------------------------------------------------------------------------\n");
    }
    else {
      printf("================================================================================================================\n");
    }
  }

  if (m_enableWindow) {
    printf(" Cropped Window Statistics Enabled from rectangle position (%d,%d) and size (%dx%d)\n", iMin(m_windowMinPosX,m_windowMaxPosX),iMin(m_windowMinPosY,m_windowMaxPosY), m_windowWidth, m_windowHeight);
    printf("================================================================================================================\n");
  }

  if (inputParams->m_silentMode == FALSE) {
    for (int index = DIST_NULL; index < DIST_METRICS; index++) {
      if (m_enableMetric[index] == TRUE)
        m_distortionMetric[index]->printSeparator();
    }
    if (m_enableWindow) {
      printf("---");
      for (int index = DIST_NULL; index < DIST_METRICS; index++) {
        if (m_enableWindowMetric[index] == TRUE)
          m_windowDistortionMetric[index]->printSeparator();
      }
    }


    // Separators for 'Frame#  Time'
    printf("---------------------------------------------------------\n");
    printf("Frame#  ");  // 8
    for (int index = DIST_NULL; index < DIST_METRICS; index++) {
      if (m_enableMetric[index] == TRUE)
        m_distortionMetric[index]->printHeader();
    }
    if (m_enableWindow) {
      printf(" | ");
      for (int index = DIST_NULL; index < DIST_METRICS; index++) {
        if (m_enableWindowMetric[index] == TRUE)
          m_windowDistortionMetric[index]->printHeader();
      }
    }
    printf("  Time "); // 7


    printf("\n---------------------------------------------------------\n");
  }
}

//-----------------------------------------------------------------------------
// footer output
//-----------------------------------------------------------------------------
void HDRVQMFrame::outputFooter(ProjectParameters *inputParams) {
  int j;
  clock_t clk = clock();
  FILE* f = IOFunctions::openFile(inputParams->m_logFile, "at");

  if (f != NULL) {
    fprintf(f, "%s ", inputParams->m_inputFile[0].m_fName);
    fprintf(f, "%s ", inputParams->m_inputFile[1].m_fName);
    fprintf(f, "%d ", inputParams->m_numberOfFrames);
    fprintf(f, "%f \n", 1.0 * clk / inputParams->m_numberOfFrames / CLOCKS_PER_SEC);
  }
  //if (inputParams->m_silentMode == FALSE){
  if (1) {
    for (j = DIST_NULL; j < DIST_METRICS; j++) {
      if (m_enableMetric[j] == TRUE)
        m_distortionMetric[j]->printSeparator();
    }
    if (m_enableWindow) {
      printf("---");
      for (int index = DIST_NULL; index < DIST_METRICS; index++) {
        if (m_enableWindowMetric[index] == TRUE)
          m_windowDistortionMetric[index]->printSeparator();
      }
    }

    // Separators for 'Frame#  Time'
    printf("---------------------------------------------------------\n");

    //printf("%06d ", inputParams->m_numberOfFrames);
    printf("HDR-VQM   ");

    for (j = DIST_NULL; j < DIST_METRICS; j++) {
      if (m_enableMetric[j] == TRUE)
        m_distortionMetric[j]->reportSummary();
    }
    if (m_enableWindow) {
      printf("  |");
      for (int index = DIST_NULL; index < DIST_METRICS; index++) {
        if (m_enableWindowMetric[index] == TRUE)
          m_windowDistortionMetric[index]->reportSummary();
      }
    }
    //printf("%7.3f", clk / ((float) inputParams->m_numberOfFrames * (float) CLOCKS_PER_SEC));
  }

  // Separators for 'Frame#  Time'
  printf("\n---------------------------------------------------------");

  printf("\nTotal of %d frames processed in %5.3f seconds\n",inputParams->m_numberOfFrames, 1.0 * clk / CLOCKS_PER_SEC);
  printf("Processing Speed: %3.2ffps\n", (float) inputParams->m_numberOfFrames * CLOCKS_PER_SEC / clk);
  for (j = DIST_NULL; j < DIST_METRICS; j++) {
    if (m_enableMetric[j] == TRUE)
      m_distortionMetric[j]->printSeparator();
  }
  if (m_enableWindow) {
    printf("---");
    for (int index = DIST_NULL; index < DIST_METRICS; index++) {
      if (m_enableWindowMetric[index] == TRUE)
        m_windowDistortionMetric[index]->printSeparator();
    }
  }
  printf("---------------------------------------------------------\n");

  IOFunctions::closeFile(f);
}

