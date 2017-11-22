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
 * \file ProjectParameters.cpp
 *
 * \brief
 *    ProjectParameters class functions for HDRVQM project
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "ProjectParameters.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Local classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ProjectParameters ccParams;
ProjectParameters *pParams = &ccParams;

FrameFormat *src0  = &pParams->m_source[0];
FrameFormat *src1  = &pParams->m_source[1];
DistortionParameters *dParams = &pParams->m_distortionParameters;
VQMParams   *vqm   = &pParams->m_distortionParameters.m_VQM;

static char def_logfile[]  = "distortion.txt";

StringParameter stringParameterList[] = {
  { "Input0File",          pParams->m_inputFile[0].m_fName,           NULL, "1st Input file name"           },
  { "Input1File",          pParams->m_inputFile[1].m_fName,           NULL, "2nd Input file name"           },
  { "LogFile",             pParams->m_logFile,                 def_logfile, "Output Log file name"       },
  { "",                    NULL,                                      NULL, "String Termination entry"   }
};

IntegerParameter intParameterList[] = {
  { "Input0Width",             &src0->m_width[0],                                  176,           0,      65536,    "1st Input source width"               },
  { "Input0Height",            &src0->m_height[0],                                 144,           0,      65536,    "1st Input source height"              },
  { "Input0ChromaFormat",     (int *) &src0->m_chromaFormat,                    CF_420,      CF_400,     CF_444,    "1st Input Chroma Format"              },
  { "Input0FourCCCode",       (int *) &src0->m_pixelFormat,                    PF_UYVY,     PF_UYVY, PF_TOTAL-1,    "1st Input Pixel Format"               },
  { "Input0BitDepthCmp0",     &src0->m_bitDepthComp[Y_COMP],                         8,           8,         16,    "1st Input Bitdepth Cmp0"              },
  { "Input0BitDepthCmp1",     &src0->m_bitDepthComp[U_COMP],                         8,           8,         16,    "1st Input Bitdepth Cmp1"              },
  { "Input0BitDepthCmp2",     &src0->m_bitDepthComp[V_COMP],                         8,           8,         16,    "1st Input Bitdepth Cmp2"              },
  { "Input0ColorSpace",       (int *) &src0->m_colorSpace,                    CM_YCbCr,    CM_YCbCr, CM_TOTAL-1,    "1st Input Color Space"                },
  { "Input0ColorPrimaries",   (int *) &src0->m_colorPrimaries,                  CP_709,      CP_709, CP_TOTAL-1,    "1st Input Color Primaries"            },
  { "Input0SampleRange",      (int *) &src0->m_sampleRange,                SR_STANDARD, SR_STANDARD,     SR_SDI,    "1st Input Sample Range"               },
  { "Input0FileHeader",       &pParams->m_inputFile[0].m_fileHeader,                 0,           0,    INT_INF,    "1st Input Header (bytes)"             },
  { "Input0StartFrame",       &pParams->m_inputFile[0].m_startFrame,                 0,           0,    INT_INF,    "1st Input Start Frame"                },
  { "Input0FrameSkip",        &pParams->m_frameSkip[0],                              0,           0,    INT_INF,    "1st Input Frame Skipping"             },
  { "Input0CropOffsetLeft",   &pParams->m_cropOffsetLeft[0],                         0,      -65536,      65536,    "1st Input Crop Offset Left position"  },
  { "Input0CropOffsetTop",    &pParams->m_cropOffsetTop[0],                          0,      -65536,      65536,    "1st Input Crop Offset Top position"   },
  { "Input0CropOffsetRight",  &pParams->m_cropOffsetRight[0],                        0,      -65536,      65536,    "1st Input Crop Offset Right position" },
  { "Input0CropOffsetBottom", &pParams->m_cropOffsetBottom[0],                       0,      -65536,      65536,    "1st Input Crop Offset Bottom position"},  
  // Currently we do not need rescaling so we can keep these parameters disabled (could be added in the future if needed).
  { "Input1Width",            &src1->m_width[0],                                   176,           0,      65536,    "2nd Input/Processing width"           },
  { "Input1Height",           &src1->m_height[0],                                  144,           0,      65536,    "2nd Input/Processing height"          },
  { "Input1ChromaFormat",     (int *) &src1->m_chromaFormat,                    CF_420,      CF_400,     CF_444,    "2nd Input Chroma Format"              },
  { "Input1FourCCCode",       (int *) &src1->m_pixelFormat,                    PF_UYVY,     PF_UYVY, PF_TOTAL-1,    "2nd Input Pixel Format"               },
  { "Input1BitDepthCmp0",     &src1->m_bitDepthComp[Y_COMP],                         8,           8,         16,    "2nd Input Bitdepth Cmp0"              },
  { "Input1BitDepthCmp1",     &src1->m_bitDepthComp[U_COMP],                         8,           8,         16,    "2nd Input Bitdepth Cmp1"              },
  { "Input1BitDepthCmp2",     &src1->m_bitDepthComp[V_COMP],                         8,           8,         16,    "2nd Input Bitdepth Cmp2"              },
  { "Input1ColorSpace",       (int *) &src1->m_colorSpace,                    CM_YCbCr,    CM_YCbCr,   CM_YUpVp,    "2nd Input Color Space"                },
  { "Input1ColorPrimaries",   (int *) &src1->m_colorPrimaries,                  CP_709,      CP_709, CP_TOTAL-1,    "2nd Input Color Primaries"            },
  { "Input1SampleRange",      (int *) &src1->m_sampleRange,                SR_STANDARD, SR_STANDARD,     SR_SDI,    "2nd Input Sample Range"               },
  { "Input1FileHeader",       &pParams->m_inputFile[1].m_fileHeader,                 0,           0,    INT_INF,    "2nd Input Header (bytes)"             },
  { "Input1StartFrame",       &pParams->m_inputFile[1].m_startFrame,                 0,           0,    INT_INF,    "2nd Input Start Frame"                },
  { "Input1FrameSkip",        &pParams->m_frameSkip[1],                              0,           0,    INT_INF,    "2nd Input Frame Skipping"             },
  { "Input1CropOffsetLeft",   &pParams->m_cropOffsetLeft[1],                         0,      -65536,      65536,    "2nd Input Crop Offset Left position"  },
  { "Input1CropOffsetTop",    &pParams->m_cropOffsetTop[1],                          0,      -65536,      65536,    "2nd Input Crop Offset Top position"   },
  { "Input1CropOffsetRight",  &pParams->m_cropOffsetRight[1],                        0,      -65536,      65536,    "2nd Input Crop Offset Right position" },
  { "Input1CropOffsetBottom", &pParams->m_cropOffsetBottom[1],                       0,      -65536,      65536,    "2nd Input Crop Offset Bottom position"},
  { "WindowMinPosX",          &pParams->m_windowMinPosX,                             0,      -65536,      65536,    "Minimum Window X position"            },
  { "WindowMaxPosX",          &pParams->m_windowMaxPosX,                             0,      -65536,      65536,    "Maximum Window X position"            },
  { "WindowMinPosY",          &pParams->m_windowMinPosY,                             0,      -65536,      65536,    "Minimum Window Y position"            },
  { "WindowMaxPosY",          &pParams->m_windowMaxPosY,                             0,      -65536,      65536,    "Maximum Window Y position"            },
  { "NumberOfFrames",         &pParams->m_numberOfFrames,                            1,           1,    INT_INF,    "Number of Frames to process"          },
  //VQM parameters
  { "AreaDisplay",            &vqm->m_displayArea,                                6100,        6100,       6100,    "Display Area"                         },
  { "RowsDisplay",            &vqm->m_rowsDisplay,                                1080,         512,       1080,    "Display Rows"                         },
  { "ColsDisplay",            &vqm->m_colsDisplay,                                1920,         896,       1920,    "Display Columns"                      },
  { "FFTColumns",             &vqm->m_columnsFFT,                                 1024,          64,      65536,    "VQM FFT Rows"                         },
  { "FFTRows",                &vqm->m_rowsFFT,                                     512,          64,      65536,    "VQM FFT Columns"                      },
  { "ViewingDistance",        &vqm->m_viewingDistance,                             178,         178,        178,    "Viewer's display distance (cm)"       },
  { "NumberOfScales",         &vqm->m_numberOfScale,                                 5,           5,          5,    "Log Gabor filter scales #"            },
  { "NumberOfOrientations",   &vqm->m_numberOfOrient,                                4,           4,          4,    "Log Gabor filter orientations #"      },
  { "NumberOfFrames",         &vqm->m_numberOfFrames,                                1,           1,    INT_INF,    "Number of Frames to process"          },
  { "",                       NULL,                                                  0,           0,          0,    "Integer Termination entry"            }
};

BoolParameter boolParameterList[] = {
  { "Input0Interleaved",      &pParams->m_inputFile[0].m_isInterleaved,   FALSE,      FALSE,      TRUE,    "1st Input Interleaved Source"                  },
  { "Input1Interleaved",      &pParams->m_inputFile[1].m_isInterleaved,   FALSE,      FALSE,      TRUE,    "2nd Input Interleaved Source"                  },
  { "Input0Interlaced",       &src0->m_isInterlaced,                      FALSE,      FALSE,      TRUE,    "1st Input Interleaved Source"                  },
  { "Input1Interlaced",       &src1->m_isInterlaced,                      FALSE,      FALSE,      TRUE,    "2nd Input Interleaved Source"                  },
  { "SilentMode",             &pParams->m_silentMode,                      TRUE,      FALSE,      TRUE,    "Silent mode"                                   },
  { "ClipInputValues",        &dParams->m_clipInputValues,                FALSE,      FALSE,      TRUE,    "Clip input during distortion comp."            },
  { "EnableHDRVQM",           &pParams->m_enableMetric[DIST_VQM],          TRUE,       TRUE,      TRUE,    "Enable HDRVQM reporting"                       },
  { "DisplayAdapt",			      &vqm->m_displayAdapt,                        TRUE,      FALSE,      TRUE,    "Enable VQM display adaptation"                 },
  { "",                       NULL,                                       FALSE,      FALSE,     FALSE,    "Boolean Termination entry"                     }
};

DoubleParameter doubleParameterList[] = {
  { "MaxSampleValue",        &dParams->m_maxSampleValue,                  10000.0,     0.001, 99999999.0,   "Maximum sample value for floats"             },
  { "",                      NULL,                                            0.0,       0.0,        0.0,   "Double Termination entry"                    }
};

FloatParameter floatParameterList[] = {
  { "Input0Rate",            &src0->m_frameRate,                           24.00F,    0.01F,     120.00F,    "1st Input Source Frame Rate"                },
  { "Input1Rate",            &src1->m_frameRate,                           24.00F,    0.01F,     120.00F,    "2nd Input Source Frame Rate"                },
  //VQM paramters
  { "PoolPercent",           &vqm->m_poolingPerc,                            0.3F,     0.1F,        1.0F,    "Pooling Percentage for HDRVQM Calculation"  },
  { "MaxDisplay",            &vqm->m_maxDisplay,                          4500.0F,  1000.0F,    10000.0F,    "Maximum luminance that can be displayed"    },
  { "MinDisplay",            &vqm->m_minDisplay,                            0.03F,    0.01F,        1.0F,    "Minimum luminance that can be displayed"    },
  { "FixationTime",          &vqm->m_fixationTime,                           0.6F,     0.6F,        0.6F,    "Fixation Duration in seconds for calculation of spatio-temporal tubes"},
  { "Input0Rate",            &vqm->m_frameRate,                            24.00F,    0.01F,     120.00F,    "1st Input Source Frame Rate"                },
  { "",                      NULL,                                         0.00F,     0.00F,       0.00F,    "Float Termination entry"                    }
};


//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
ProjectParameters::ProjectParameters()
{
  for (int i = DIST_NULL; i < DIST_METRICS; i++) {
    m_enableMetric[i] = FALSE;
  }
  for (int i = DIST_NULL; i < DIST_METRICS; i++) {
    m_enableWindowMetric[i] = FALSE;
  }
  m_enableWindow = FALSE;
}

void ProjectParameters::refresh() {
  int i;

  for (i = ZERO; intParameterList[i].ptr != NULL; i++)
    *(intParameterList[i].ptr) = intParameterList[i].default_val;

  for (i = ZERO; boolParameterList[i].ptr != NULL; i++)
    *(boolParameterList[i].ptr) = boolParameterList[i].default_val;

  for (i = ZERO; floatParameterList[i].ptr != NULL; i++)
    *(floatParameterList[i].ptr) = floatParameterList[i].default_val;

  for (i = ZERO; doubleParameterList[i].ptr != NULL; i++) {
    *(doubleParameterList[i].ptr) = doubleParameterList[i].default_val;  
  }
  
  for (i = ZERO; stringParameterList[i].ptr != NULL; i++) {
    if (stringParameterList[i].default_val != NULL)
      strcpy(stringParameterList[i].ptr, stringParameterList[i].default_val);
    else
      stringParameterList[i].ptr[0] = 0;    
  }
}

void ProjectParameters::update() {
  IOFunctions::parseVideoType   (&m_inputFile[0]);
  IOFunctions::parseFrameFormat (&m_inputFile[0]);
  IOFunctions::parseVideoType   (&m_inputFile[1]);
  IOFunctions::parseFrameFormat (&m_inputFile[1]);
  
  // Read resolution from file name
  if (m_source[0].m_width[0] == 0 || m_source[0].m_height[0] == 0) {
    if (IOFunctions::parseFrameSize (&m_inputFile[0], m_source[0].m_width, m_source[0].m_height, &(m_source[0].m_frameRate)) == 0) {
      fprintf(stderr, "File name does not contain resolution information.\n");
      exit(EXIT_FAILURE);
    }
  }
    
  setupFormat(&m_source[0]);
  setupFormat(&m_source[1]);
    
  if (m_numberOfFrames == -1)  {
    IOFunctions::openFile (&m_inputFile[0]);
    getNumberOfFrames (&m_source[0], &m_inputFile[0], m_inputFile[0].m_startFrame);
    IOFunctions::closeFile (&m_inputFile[0]);
    IOFunctions::openFile (&m_inputFile[1]);
    getNumberOfFrames (&m_source[1], &m_inputFile[1], m_inputFile[1].m_startFrame);
    IOFunctions::closeFile (&m_inputFile[1]);
  }

  m_inputFile[0].m_format = m_source[0];
  m_inputFile[1].m_format = m_source[1];
  
  for (int i = DIST_NULL; i < DIST_METRICS; i++) {
    if (m_enableWindowMetric[i] == TRUE) {
      m_enableWindow = TRUE;
      break;
    }
  }
}



//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
