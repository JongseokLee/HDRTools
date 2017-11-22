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
 * \file IOFunctions.H
 *
 * \brief
 *    IOFunctions class header. 
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __IOFunctions_H__
#define __IOFunctions_H__

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "IOVideo.H"

#ifdef WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

struct VIDEO_SIZE {
public:
  char name[64];
  int x_size;
  int y_size;
};


static const VIDEO_SIZE VideoRes[] = {
  { "qcif"  ,  176,  144},
  { "qqvga" ,  160,  128},
  { "qvga"  ,  320,  240},
  { "sif"   ,  352,  240},
  { "cif"   ,  352,  288},
  { "vga"   ,  640,  480},
  { "sd1"   ,  720,  480},
  { "sd2"   ,  704,  576},
  { "sd3"   ,  720,  576},
  { "720p"  , 1280,  720},
  { "1080p" , 1920, 1080},
  { "", 0, 0}
};

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

class IOFunctions
{
public:
  static FILE *openFile( char *filename, const char *mode );
  static int  openFile ( char *filename, int mode, int permissions );
  static void closeFile( FILE *f );
  static void closeFile( int *f );
  static void writeFile( int f, void *buf, int nbyte, char *filename );
  static void readFile ( int f, void *buf, int nbyte, char *filename );

  
  static void          openFrameFile    (IOVideo *inputFile, int FrameNumberInFile);
  static int           openFile         (IOVideo *inputFile);
  static int           openFile         (IOVideo *inputFile, int mode, int permissions);
  static void          closeFile        (IOVideo *inputFile);
  static int           parseFrameSize   (IOVideo *inputFile, int *xlen, int *ylen, float *fps);
  static void          parseFrameFormat (IOVideo *inputFile);
  static VideoFileType parseVideoType   (IOVideo *inputFile);
};
#endif

