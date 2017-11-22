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
 * \file IOFunctions.cpp
 *
 * \brief
 *    IOFunctions Class relating to I/O operations
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


#include "IOFunctions.H"
#include "Defines.H"
#include "Global.H"
#include "IOVideo.H"

#include <stdio.h>
#include <string.h>

/*!
 ************************************************************************
 * \brief
 *    Parse Size from from file name
 *
 ************************************************************************
 */
int IOFunctions::parseFrameSize (IOVideo *inputFile, int *x_size, int *y_size, float *fps) {
  char *p1, *p2, *tail;
  char *fn = inputFile->m_fName;
  char c;
  int i = 0;

  *x_size = *y_size = -1;
  p1 = p2 = fn;

  while (p1 != NULL && p2 != NULL) {
    // Search for first '_'
    p1 = strstr( p1, "_");
    if (p1 == NULL)
      break;

    // Search for end character of x_size (first 'x' after last '_')
    p2 = strstr( p1, "x");

    // If no 'x' is found, exit
    if (p2 == NULL)    
      break;

    // Try conversion of number
    *p2 = 0;
    *x_size = (int) strtol( p1 + 1, &tail, 10);

    // If there are characters left in the string, or the string is null, discard conversion
    if (*tail != '\0' || *(p1 + 1) == '\0') {
      *p2 = 'x';
      p1 = tail;
      continue;
    }

    // Conversion was correct. Restore string
    *p2 = 'x';

    // Search for end character of y_size (first '_' or '.' after last 'x')
    p1 = strpbrk( p2 + 1, "_.");
    // If no '_' or '.' is found, try again from current position
    if (p1 == NULL) {
      p1 = p2 + 1;
      continue;
    }

    // Try conversion of number
    c = *p1;
    *p1 = 0;
    *y_size = (int) strtol( p2 + 1, &tail, 10);

    // If there are characters left in the string, or the string is null, discard conversion
    if (*tail != '\0' || *(p2 + 1) == '\0') {
      *p1 = c;
      p1 = tail;
      continue;
    }

    // Conversion was correct. Restore string
    *p1 = c;

    // Search for end character of y_size (first 'i' or 'p' after last '_')
    p2 = strstr( p1 + 1, "i");
    if (p2 == NULL)      
      p2 = strstr( p1 + 1, "p");

    // If no 'i' or 'p' is found, exit
    if (p2 == NULL) {
      *p2 = c;
      break;
    }

    // Try conversion of number
    c = *p2;
    *p2 = 0;
    *fps = (float) strtod( p1 + 1, &tail);

    // If there are characters left in the string, or the string is null, discard conversion
    if (*tail != '\0' || *(p1 + 1) == '\0') {
      *p2 = c;
      p1 = tail;
      continue;
    }
    // Conversion was correct. Restore string
    *p2 = c;
    break;
  }

  // Now lets test some common video file formats
  if (p1 == NULL || p2 == NULL)  {    
    //for (i = 0; VideoRes[i].name != NULL; i++) {
    for (i = 0; i < 64; i++) {
      if (strcasecmp (fn, VideoRes[i].name)) {
        *x_size = VideoRes[i].x_size;
        *y_size = VideoRes[i].y_size;       
        // Should add frame rate support as well
        break;
      }
    }
  }

  return (*x_size == -1 || *y_size == -1) ? 0 : 1; 
}

/*!
 ************************************************************************
 * \brief
 *    Parse Size from file name
 *
 ************************************************************************
 */
void IOFunctions::parseFrameFormat (IOVideo *inputFile) {
  
  char *fn         = inputFile->m_fName;
  char *p1 = fn, *p2 = fn, *tail;  
  char *fhead      = inputFile->m_fHead;
  char *ftail      = inputFile->m_fTail;
  bool *zeroPad    = &inputFile->m_zeroPad;
  int  *numDigits  = &inputFile->m_numDigits;

  *zeroPad = FALSE;
  *numDigits = -1;
  
  while (p1 != NULL && p2 != NULL)  {
    // Search for first '_'
    p1 = strstr( p1, "%");
    
    if (p1 == NULL) {
      if (strlen(fhead) == 0 && strlen(fn) !=0 ) {
        strncpy(fhead, fn, strlen(fn));
      }
      break;
    }

    strncpy(fhead, fn, p1 - fn);
    
    // Search for end character of x_size (first 'x' after last '_')
    p2 = strstr( p1, "d");

    // If no 'x' is found, exit
    if (p2 == NULL)    
      break;
    
    // Try conversion of number
    *p2 = 0;

    if (*(p1 + 1) == '0')
      *zeroPad = TRUE;

    *numDigits = (int) strtol( p1 + 1, &tail, 10);

    // If there are characters left in the string, or the string is null, discard conversion
    if (*tail != '\0' || *(p1 + 1) == '\0')  {
      *p2 = 'd';
      p1 = tail;
      continue;
    }

    // Conversion was correct. Restore string
    *p2 = 'd';

    tail++;
    strncpy(ftail, tail, (int) strlen(tail));
    break;
  }
  
  if (inputFile->m_videoType == VIDEO_TIFF || inputFile->m_videoType == VIDEO_EXR)  {
    inputFile->m_isConcatenated = FALSE;
  }
  else {
    inputFile->m_isConcatenated = (*numDigits == -1) ? TRUE : FALSE;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Open file containing a single frame
 ************************************************************************
 */
void IOFunctions::openFrameFile( IOVideo *inputFile, int FrameNumberInFile) {
  char infile [FILE_NAME_SIZE], in_number[16];
  int length = 0;
  in_number[length]='\0';
  length = (int) strlen(inputFile->m_fHead);
  strncpy(infile, inputFile->m_fHead, length);
  infile[length]='\0';
  if (inputFile->m_zeroPad)       
    snprintf(in_number, 16, "%0*d", inputFile->m_numDigits, FrameNumberInFile);
  else
    snprintf(in_number, 16, "%*d", inputFile->m_numDigits, FrameNumberInFile);

  // strncat(infile, in_number, sizeof(in_number));
  strncat(infile, in_number, sizeof(infile) - strlen(infile) - 1);
  length += sizeof(in_number);
  infile[length]='\0';
  strncat(infile, inputFile->m_fTail, strlen(inputFile->m_fTail));
  length += (int) strlen(inputFile->m_fTail);
  infile[length]='\0';

  if ((inputFile->m_fileNum = open(infile, OPENFLAGS_READ)) == -1) {
    printf ("openFrameFile: cannot open file %s\n", infile);
    exit(EXIT_FAILURE);
  }    
}

FILE* IOFunctions::openFile(char *filename, const char *mode) {
  FILE *f;

  if (filename[ZERO] == ZERO)
    return NULL; // if empty filename, simply return NULL
  else {
    f = fopen(filename, mode);
    if (f == NULL) {
      fprintf(stderr, "Could not open file [%s].\n", filename);
      exit(EXIT_FAILURE);
    }
  }

  return f;
}

int IOFunctions::openFile(char *filename, int mode, int permissions) {
  int f;

  if (filename[ZERO] == ZERO)
    return -1; // if empty filename, simply return -1
  else {
    f = open(filename, mode, permissions);
    if (f == -1) {
      fprintf(stderr, "Could not open file [%s].\n", filename);
      exit(EXIT_FAILURE);
    }
  }
  return f;
}

void IOFunctions::closeFile(FILE *f)
{
  if (f != NULL) {
    fclose(f);
    f = NULL;
  }
}

void IOFunctions::closeFile(int *f) {
  if (*f != -1) {
    close(*f);
    *f = -1;
  }
}

/*!
 ************************************************************************
 * \brief
 *    Open file(s) containing the entire frame sequence
 ************************************************************************
 */
int IOFunctions::openFile( IOVideo *file ) {
  if (file->m_isConcatenated == TRUE)  {
    if ((int) strlen(file->m_fName) == 0) {
      fprintf(stderr, "No file name was provided. Please check settings.\n");
      exit(EXIT_FAILURE);
    }

    if ((file->m_fileNum = open(file->m_fName, OPENFLAGS_READ)) == -1)   {
      fprintf(stderr, "Input file %s does not exist\n", file->m_fName);
      exit(EXIT_FAILURE);
    }
    return file->m_fileNum;
  }
  return -1;
}

int IOFunctions::openFile( IOVideo *file, int mode, int permissions ) {
  if (file->m_isConcatenated == TRUE)  {
    if ((int) strlen(file->m_fName) == 0) {
      fprintf(stderr, "No file name was provided. Please check settings.\n");
      exit(EXIT_FAILURE);
    }
    
    if ((file->m_fileNum = open(file->m_fName, mode, permissions)) == -1)   {
      fprintf(stderr, "Could not open file %s", file->m_fName);
      exit(EXIT_FAILURE);
    }
    return file->m_fileNum;
  }
  return -1;
}

/*!
 ************************************************************************
 * \brief
 *    Close input file
 ************************************************************************
 */
void IOFunctions::closeFile(IOVideo *currentFile) {
  if (currentFile->m_fileNum != -1) {
    close(currentFile->m_fileNum);
    currentFile->m_fileNum = -1;
  }
}


void IOFunctions::writeFile( int f, void *buf, int nbyte, char *filename ) {
  if ( -1 == mm_write( f, buf, nbyte ) )  {
    fprintf(stderr, "Cannot write to file [%s].\n", filename);
    exit(EXIT_FAILURE);
  }
}

void IOFunctions::readFile( int f, void *buf, int nbyte, char *filename ) {
  if ( nbyte != mm_read( f, buf, nbyte ) )  {
    fprintf(stderr, "Cannot read from file [%s].\n", filename);
    exit(EXIT_FAILURE);
  }
}

/* ==========================================================================
 *
 * parseVideoType
 *
 * ==========================================================================
*/
VideoFileType IOFunctions::parseVideoType (IOVideo *inputFile) {
  char *format  = inputFile->m_fName + (int) strlen(inputFile->m_fName) - 3;
  char *format2 = inputFile->m_fName + (int) strlen(inputFile->m_fName) - 4;

  if (strcasecmp (format, "y4m") == 0) {
    inputFile->m_videoType = VIDEO_Y4M;
    inputFile->m_format.m_chromaFormat = CF_420;
    inputFile->m_avi = NULL;
  }
  else if (strcasecmp (format, "yuv") == 0) {
    inputFile->m_videoType = VIDEO_YUV;
    inputFile->m_format.m_chromaFormat = CF_420;
    inputFile->m_avi = NULL;
  }
  else if (strcasecmp (format, "dpx") == 0) {
    inputFile->m_videoType = VIDEO_DPX;
    inputFile->m_format.m_chromaFormat = CF_444;
    inputFile->m_avi = NULL;
  }
  else if (strcasecmp (format, "rgb") == 0) {
    inputFile->m_videoType = VIDEO_RGB;
    inputFile->m_format.m_chromaFormat = CF_444;
    inputFile->m_avi = NULL;
  }
  else if (strcasecmp (format, "raw") == 0) {
    inputFile->m_videoType = VIDEO_RGB;
    inputFile->m_format.m_chromaFormat = CF_444;
    inputFile->m_avi = NULL;
  }
  else if ((strcasecmp (format, "tif") == 0) || (strcasecmp (format2, "tiff") == 0)) {
    inputFile->m_videoType = VIDEO_TIFF;
    inputFile->m_format.m_chromaFormat = CF_444;
    inputFile->m_avi = NULL;
  }
  else if (strcasecmp (format, "exr") == 0) {
    inputFile->m_videoType = VIDEO_EXR;
    inputFile->m_format.m_chromaFormat = CF_444;
    inputFile->m_avi = NULL;
  }
  else if (strcasecmp (format, "avi") == 0) {
  // Currently format is not supported
    inputFile->m_videoType = VIDEO_AVI;
    inputFile->m_format.m_chromaFormat = CF_444;
  }
  else  { // RAW formats
    inputFile->m_videoType = VIDEO_YUV;
    inputFile->m_format.m_chromaFormat = CF_420;
    inputFile->m_avi = NULL;
  }

  return inputFile->m_videoType;
}
