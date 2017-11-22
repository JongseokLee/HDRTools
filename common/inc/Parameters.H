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
 * \file Parameters.H
 *
 * \brief
 *    Parameters Input class (main) 
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __Parameters_H__
#define __Parameters_H__

#include "Global.H"
#include "IOVideo.H"

static const int mb_width_cr[4] = {0,8, 8,16};
static const int mb_height_cr[4]= {0,8,16,16};

struct IntegerParameter {
public:
  char name[64];
  int *ptr;
  int  default_val;
  int  min_val;
  int  max_val;
  char description[256];
};

struct FloatParameter {
public:
  char   name[64];
  float *ptr;
  float  default_val;
  float  min_val;
  float  max_val;
  char   description[256];
};

struct DoubleParameter {
public:
  char    name[64];
  double *ptr;
  double  default_val;
  double  min_val;
  double  max_val;
  char    description[256];
};

struct StringParameter {
public:
  char  name[64];
  char *ptr;
  char *default_val;
  char  description[256];
};

struct BoolParameter {
public:
  char name[64];
  bool *ptr;
  bool default_val;
  bool min_val;
  bool max_val;
  char description[256];
};

class Parameters {
private:
  int   parseCommandLineParams( char **cl_params, int numCLParams );

public:
  int  m_numberOfFrames;
  bool m_silentMode;       //! Silent mode (reduced output)
  bool m_enableLegacy;
  
  // Log file
  char m_logFile[MAX_LINE_LEN];

  Parameters();
  virtual  ~Parameters() = 0;

  
  void  printParams           ();
  void  setupFormat           ( FrameFormat *format );
  int   getNumberOfFrames     ( const FrameFormat *source, IOVideo *inputFile, int start_frame );
  void  configure             ( char *filename, char **cl_params, int numCLParams, bool readConfig );
  int   readConfigFile        ( char *filename );
  
  virtual void update()  = 0;
  virtual void refresh() = 0;
};

extern Parameters *params;

#endif
