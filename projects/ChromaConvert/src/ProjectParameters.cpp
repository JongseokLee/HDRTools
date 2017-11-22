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
 *    ProjectParameters class functions for ChromaConvert project
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
#include "TransferFunction.H"
#include "ScaleFilter.H"

//-----------------------------------------------------------------------------
// Local classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ProjectParameters ccParams;
ProjectParameters *pParams = &ccParams;

FrameFormat *src  = &pParams->m_source;
FrameFormat *out  = &pParams->m_output;

static char def_logfile[]  = "log.txt";
static char def_out_file[] = "output_conv.yuv";

StringParameter stringParameterList[] = {
  { "SourceFile",          pParams->m_inputFile.m_fName,              NULL, "Source file name"           },
  { "OutputFile",          pParams->m_outputFile.m_fName,     def_out_file, "Output file name"           },
  { "LogFile",             pParams->m_logFile,                 def_logfile, "Output Log file name"       },
  { "",                    NULL,                                      NULL, "String Termination entry"   }
};

IntegerParameter intParameterList[] = {
  { "SourceWidth",             &src->m_width[0],                            176,           0,        65536,    "Input source width"                },
  { "SourceHeight",            &src->m_height[0],                           144,           0,        65536,    "Input source height"               },
  { "SourceChromaFormat",     (int *) &src->m_chromaFormat,              CF_420,      CF_400,       CF_444,    "Source Chroma Format"              },
  { "SourceFourCCCode",       (int *) &src->m_pixelFormat,              PF_UYVY,     PF_UYVY, PF_TOTAL - 1,    "Source Pixel Format"               },
  { "SourceBitDepthCmp0",     (int *) &src->m_bitDepthComp[Y_COMP],           8,           8,           16,    "Source Bitdepth Cmp0"              },
  { "SourceBitDepthCmp1",     (int *) &src->m_bitDepthComp[U_COMP],           8,           8,           16,    "Source Bitdepth Cmp1"              },
  { "SourceBitDepthCmp2",     (int *) &src->m_bitDepthComp[V_COMP],           8,           8,           16,    "Source Bitdepth Cmp2"              },
  { "SourceColorSpace",       (int *) &src->m_colorSpace,              CM_YCbCr,    CM_YCbCr,   CM_TOTAL-1,    "Source Color Space"                },
  { "SourceColorPrimaries",   (int *) &src->m_colorPrimaries,            CP_709,      CP_709,   CP_TOTAL-1,    "Source Color Primaries"            },
  { "SourceSampleRange",      (int *) &src->m_sampleRange,          SR_STANDARD, SR_STANDARD,       SR_SDI,    "Source Sample Range"               },

  // Currently we do not need rescaling so we can keep these parameters disabled (could be added in the future if needed).
  //{ "OutputWidth",             &out->m_width[0],                          176,           0,        65536,    "Output/Processing width"           },
  //{ "OutputHeight",            &out->m_height[0],                         144,           0,        65536,    "Output/Processing height"          },
  { "OutputChromaFormat",     (int *) &out->m_chromaFormat,              CF_420,      CF_400,       CF_444,    "Output Chroma Format"              },
  { "OutputBitDepthCmp0",     (int *) &out->m_bitDepthComp[Y_COMP],           8,           8,           16,    "Output Bitdepth Cmp0"              },
  { "OutputBitDepthCmp1",     (int *) &out->m_bitDepthComp[U_COMP],           8,           8,           16,    "Output Bitdepth Cmp1"              },
  { "OutputBitDepthCmp2",     (int *) &out->m_bitDepthComp[V_COMP],           8,           8,           16,    "Output Bitdepth Cmp2"              },

  //! Various Params
  { "NumberOfFrames",         &pParams->m_numberOfFrames,                     1,           1,      INT_INF,    "Number of Frames to process"       },
  { "InputFileHeader",        &pParams->m_inputFile.m_fileHeader,             0,           0,      INT_INF,    "Source Header (bytes)"             },
  { "StartFrame",             &pParams->m_inputFile.m_startFrame,             0,           0,      INT_INF,    "Source Start Frame"                },
  { "FrameSkip",              &pParams->m_frameSkip,                          0,           0,      INT_INF,    "Source Frame Skipping"             },

  { "ChromaDownsampleFilter", &pParams->m_chromaDownsampleFilter,         DF_F0,       DF_NN,        DF_F1,    "Chroma Downsampling Filter"        },
  { "ChromaUpsampleFilter",   &pParams->m_chromaUpsampleFilter,           UF_NN,       UF_NN,        UF_F0,    "Chroma Upsampling Filter"          },
  
  { "",                       NULL,                                           0,           0,            0,    "Integer Termination entry"         }
};

BoolParameter boolParameterList[] = {
  { "SourceInterleaved",      &pParams->m_inputFile.m_isInterleaved,      FALSE,      FALSE,         TRUE,    "Interleaved Source"                },
  { "SourceInterlaced",       &src->m_isInterlaced,                       FALSE,      FALSE,         TRUE,    "Interlaced Source"                 },
  { "SilentMode",             &pParams->m_silentMode,                     FALSE,      FALSE,         TRUE,    "Silent mode"                       },
  { "SetOutputSinglePrec",    &pParams->m_outputSinglePrecision,          FALSE,      FALSE,         TRUE,    "OpenEXR Output data precision"     },
  
  { "",                       NULL,                                           0,          0,            0,    "Boolean Termination entry"         }
};

DoubleParameter doubleParameterList[] = {
  { "",                      NULL,                                          0.0,        0.0,          0.0,    "Double Termination entry"          }
};


FloatParameter floatParameterList[] = {
  { "SourceRate",             &src->m_frameRate,                         24.00F,      0.01F,      120.00F,    "Input Source Frame Rate"             },
  { "OutputRate",             &out->m_frameRate,                         24.00F,      0.01F,      120.00F,    "Image Output Frame Rate"             },
  { "",                       NULL,                                       0.00F,      0.00F,        0.00F,    "Float Termination entry"             }
};


//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------


void ProjectParameters::refresh() {
  int i;

  for (i = ZERO; intParameterList[i].ptr != NULL; i++)
    *(intParameterList[i].ptr) = intParameterList[i].default_val;

  for (i = ZERO; boolParameterList[i].ptr != NULL; i++)
    *(boolParameterList[i].ptr) = boolParameterList[i].default_val;

  for (i = ZERO; floatParameterList[i].ptr != NULL; i++)
    *(floatParameterList[i].ptr) = floatParameterList[i].default_val;

  for (i = ZERO; doubleParameterList[i].ptr != NULL; i++)
    *(doubleParameterList[i].ptr) = doubleParameterList[i].default_val;  

  for (i = ZERO; stringParameterList[i].ptr != NULL; i++) {
    if (stringParameterList[i].default_val != NULL)
      strcpy(stringParameterList[i].ptr, stringParameterList[i].default_val);
    else
      stringParameterList[i].ptr[0] = 0;    
  }
}

void ProjectParameters::update() {
  IOFunctions::parseVideoType   (&m_inputFile);
  IOFunctions::parseFrameFormat (&m_inputFile);
  IOFunctions::parseVideoType   (&m_outputFile);
  IOFunctions::parseFrameFormat (&m_outputFile);
  
  // Read resolution from file name
  if (m_source.m_width[0] == 0 || m_source.m_height[0] == 0) {
    if (IOFunctions::parseFrameSize (&m_inputFile, m_source.m_width, m_source.m_height, &(m_source.m_frameRate)) == 0) {
      fprintf(stderr, "File name does not contain resolution information.\n");
      exit(EXIT_FAILURE);
    }
  }
  
  // currently lets force the same resolution
  m_output.m_width[0]  = m_source.m_width[0];
  m_output.m_height[0] = m_source.m_height[0];

  m_output.m_colorSpace     = m_source.m_colorSpace;
  m_output.m_colorPrimaries = m_source.m_colorPrimaries;
  m_output.m_sampleRange    = m_source.m_sampleRange;
  m_output.m_isInterlaced   = m_source.m_isInterlaced;
  
  
  //m_output.m_width[0]   = ceilingRescale(m_output.m_width[0], 3);
  //m_output.m_height[0]  = ceilingRescale(m_output.m_height[0],3);
  
  setupFormat(&m_source);
  setupFormat(&m_output);
  
  
  if (m_outputSinglePrecision == 0) {
    m_output.m_pixelType[Y_COMP] = m_output.m_pixelType[U_COMP] =
    m_output.m_pixelType[V_COMP] = m_output.m_pixelType[A_COMP] = HALF;
  }
  else {
    m_output.m_pixelType[Y_COMP] = m_output.m_pixelType[U_COMP] =
    m_output.m_pixelType[V_COMP] = m_output.m_pixelType[A_COMP] = FLOAT;
  }
  
  if (m_numberOfFrames == -1)  {
    IOFunctions::openFile (&m_inputFile);
    getNumberOfFrames (&m_source, &m_inputFile, m_inputFile.m_startFrame);
    IOFunctions::closeFile (&m_inputFile);
  }

  m_inputFile.m_format = m_source;
}



//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
