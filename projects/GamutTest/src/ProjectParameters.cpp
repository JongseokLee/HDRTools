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
 * \file ProjectParameters.cpp
 *
 * \brief
 *    ProjectParameters class functions for GamutTest project
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
#include "ConvertColorFormat.H"

//-----------------------------------------------------------------------------
// Local classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
ProjectParameters ccParams;
ProjectParameters *pParams = &ccParams;

FrameFormat          *src     = &pParams->m_source;
DistortionParameters *dParams = &pParams->m_distortionParameters;
VIFParams            *vif     = &dParams->m_VIF;
SSIMParams           *ssim    = &dParams->m_SSIM;
PSNRParams           *snr     = &dParams->m_PSNR;


static char def_logfile[]  = "distortion.txt";

StringParameter stringParameterList[] = {
  { "VideoFile",          pParams->m_inputFile.m_fName,           NULL, "Input file name"           },
  { "LogFile",             pParams->m_logFile,                 def_logfile, "Output Log file name"       },
  { "",                    NULL,                                      NULL, "String Termination entry"   }
};

IntegerParameter intParameterList[] = {
  { "Width",             &src->m_width[0],                            176,           0,      65536,    "Input source width"               },
  { "Height",            &src->m_height[0],                           144,           0,      65536,    "Input source height"              },
  { "ChromaFormat",     (int *) &src->m_chromaFormat,              CF_420,      CF_400,     CF_444,    "Input Chroma Format"              },
  { "FourCCCode",       (int *) &src->m_pixelFormat,              PF_UYVY,     PF_UYVY, PF_TOTAL-1,    "Input Pixel Format"               },
  { "BitDepthCmp0",     &src->m_bitDepthComp[Y_COMP],                   8,           8,         16,    "Input Bitdepth Cmp0"              },
  { "BitDepthCmp1",     &src->m_bitDepthComp[U_COMP],                   8,           8,         16,    "Input Bitdepth Cmp1"              },
  { "BitDepthCmp2",     &src->m_bitDepthComp[V_COMP],                   8,           8,         16,    "Input Bitdepth Cmp2"              },
  { "ColorSpace",       (int *) &src->m_colorSpace,              CM_YCbCr,    CM_YCbCr, CM_TOTAL-1,    "Input Color Space"                },
  { "ColorPrimaries",   (int *) &src->m_colorPrimaries,            CP_709,      CP_709, CP_TOTAL-1,    "Input Color Primaries"            },
  { "SampleRange",      (int *) &src->m_sampleRange,          SR_STANDARD, SR_STANDARD,     SR_SDI,    "Input Sample Range"               },
  { "FileHeader",       &pParams->m_inputFile.m_fileHeader,           0,           0,    INT_INF,    "Input Header (bytes)"             },
  { "StartFrame",       &pParams->m_inputFile.m_startFrame,           0,           0,    INT_INF,    "Input Start Frame"                },
  { "FrameSkip",        &pParams->m_frameSkip,                        0,           0,    INT_INF,    "Input Frame Skipping"             },
  { "CropOffsetLeft",   &pParams->m_cropOffsetLeft,                   0,      -65536,      65536,    "Input Crop Offset Left position"  },
  { "CropOffsetTop",    &pParams->m_cropOffsetTop,                    0,      -65536,      65536,    "Input Crop Offset Top position"   },
  { "CropOffsetRight",  &pParams->m_cropOffsetRight,                  0,      -65536,      65536,    "Input Crop Offset Right position" },
  { "CropOffsetBottom", &pParams->m_cropOffsetBottom,                 0,      -65536,      65536,    "Input Crop Offset Bottom position"},
  
  { "NumberOfFrames",         &pParams->m_numberOfFrames,                      1,           1,    INT_INF,    "Number of Frames to process"          },
  // SSIM parameters
  { "SSIMBlockDistance",      &ssim->m_blockDistance,                          1,           1,        128,    "Block Distance for SSIM computation"  },
  { "SSIMBlockSizeX",         &ssim->m_blockSizeX,                             4,           4,        128,    "Block Width for SSIM computation"     },
  { "SSIMBlockSizeY",         &ssim->m_blockSizeY,                             4,           4,        128,    "Block Height for SSIM computation"    },
  
  { "TFPSNRDistortion",       (int *) &snr->m_tfDistortion,          DIF_PQPH10K,  DIF_PQPH10K, DIF_TOTAL-1,  "TF for tPSNR and other metrics"       },
  { "TFPSNRDistortion",       (int *) &ssim->m_tfDistortion,         DIF_PQPH10K,  DIF_PQPH10K, DIF_TOTAL-1,  "TF for tSSIM and other metrics"       },
  { "DeltaEPointsEnable",      &dParams->m_deltaEPointsEnable,                 1,           1,          7,    "Delta E points to Enable"             },
  { "RPSNRBlockDistanceX",     &snr->m_rPSNROverlapX,                          4,           1,      65536,    "Block Horz. Distance for rPSNR"       },
  { "RPSNRBlockDistanceY",     &snr->m_rPSNROverlapY,                          4,           1,      65536,     "Block Vert. Distance for rPSNR"       },
  { "RPSNRBlockSizeX",         &snr->m_rPSNRBlockSizeX,                        4,           4,      65536,     "Block Width for rPSNR"                },
  { "RPSNRBlockSizeY",         &snr->m_rPSNRBlockSizeY,                        4,           4,      65536,     "Block Height for rPSNR"               },
  // FIV parameters
  { "VIFBitDepthY",            &vif->m_vifBitDepth,                            8,           8,         16,    "VIF YUV Bitdepth control param"       },
  
  { "",                       NULL,                                            0,           0,          0,    "Integer Termination entry"            }
};

BoolParameter boolParameterList[] = {
  { "Interleaved",         &pParams->m_inputFile.m_isInterleaved,          FALSE,  FALSE,  TRUE,    "Input Interleaved Source"                  },
  { "Interlaced",          &src->m_isInterlaced,                             FALSE,  FALSE,  TRUE,    "Input Interleaved Source"                  },
  { "SilentMode",                &pParams->m_silentMode,                             TRUE,  FALSE,  TRUE,    "Silent mode"                                   },
  { "",                          NULL,                                              FALSE,  FALSE, FALSE,    "Boolean Termination entry"                     }
};

DoubleParameter doubleParameterList[] = {
  { "MaxSampleValue",        &dParams->m_maxSampleValue,                             10000.0,      0.001, 99999999.0,    "Maximum sample value for floats"                            },
  { "",                      NULL,                                                       0.0,        0.0,        0.0,    "Double Termination entry"                                    }
};

FloatParameter floatParameterList[] = {
  { "FrameRate",            &src->m_frameRate,                        24.00F,    0.01F,     120.00F,    "Input Source Frame Rate"            },
  { "",                      NULL,                                      0.00F,     0.00F,       0.00F,    "Float Termination entry"                 }
};


//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------
ProjectParameters::ProjectParameters()
{
  for (int i = DIST_NULL; i < DIST_METRICS; i++) {
    m_enableMetric[i] = FALSE;
  }
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
  IOFunctions::parseVideoType   (&m_inputFile);
  IOFunctions::parseFrameFormat (&m_inputFile);
  
  // Read resolution from file name
  if (m_source.m_width[0] == 0 || m_source.m_height[0] == 0) {
    if (IOFunctions::parseFrameSize (&m_inputFile, m_source.m_width, m_source.m_height, &(m_source.m_frameRate)) == 0) {
      fprintf(stderr, "File name does not contain resolution information.\n");
      exit(EXIT_FAILURE);
    }
  }
    
  setupFormat(&m_source);
  
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
