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
 * \file Parameters.cpp
 *
 * \brief
 *    Parameters Class 
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */
//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "Parameters.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Local classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

Parameters *params; // contains global parameters (nasty global variable)

extern IntegerParameter intParameterList[];
extern DoubleParameter  doubleParameterList[];
extern FloatParameter   floatParameterList[];
extern StringParameter  stringParameterList[];
extern BoolParameter    boolParameterList[];

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

Parameters::Parameters() {
}

Parameters::~Parameters() {
}

/*!
 ************************************************************************
 * \brief
 *    Reads Input File Size 
 *
 ************************************************************************
 */
static int64 getVideoFileSize(int video_file)
{
   int64 fsize;   

   lseek(video_file, 0, SEEK_END); 
   fsize = tell((int) video_file); 
   lseek(video_file, 0, SEEK_SET); 

   return fsize;
}

/*!
 ************************************************************************
 * \brief
 *    Updates the number of frames to encode based on the file size
 *
 ************************************************************************
 */
int Parameters::getNumberOfFrames (const FrameFormat *source, IOVideo *inputFile, int start_frame) {
  int64 fsize = getVideoFileSize(inputFile->m_fileNum);
  int64 isize = source->m_size;
  int maxBitDepth = iMax(source->m_bitDepthComp[Y_COMP], source->m_bitDepthComp[U_COMP]);

  isize <<= (maxBitDepth > 8)? 1: 0;
  inputFile->m_numFrames = (int) ((fsize - inputFile->m_fileHeader)/ isize);
  return (int) (((fsize - inputFile->m_fileHeader)/ isize) - start_frame);
}

void Parameters::setupFormat(FrameFormat *format) {
  format->m_picUnitSizeOnDisk = (iMax(format->m_bitDepthComp[Y_COMP], format->m_bitDepthComp[U_COMP]) > 8) ? 16 : 8;
  format->m_picUnitSizeShift3 = format->m_picUnitSizeOnDisk >> 3;

  if (format->m_chromaFormat == CF_400) {// reset bitdepth of chroma for 400 content
    format->m_bitDepthComp[U_COMP] = 8;
    format->m_bitDepthComp[V_COMP] = 8;
    format->m_width[U_COMP]  = 0;
    format->m_width[V_COMP]  = 0;
    format->m_height[U_COMP] = 0;
    format->m_height[V_COMP] = 0;
  }
  else  {
    format->m_width[U_COMP]  = (format->m_width [Y_COMP] * mb_width_cr [format->m_chromaFormat]) >> 4;
    format->m_width[V_COMP]  =  format->m_width [U_COMP];
    format->m_height[U_COMP] = (format->m_height[Y_COMP] * mb_height_cr[format->m_chromaFormat]) >> 4;
    format->m_height[V_COMP] =  format->m_height[U_COMP];
  }
    
  // source size
  format->m_compSize[Y_COMP] = format->m_width[Y_COMP] * format->m_height[Y_COMP];
  format->m_compSize[U_COMP] = format->m_width[U_COMP] * format->m_height[U_COMP];
  format->m_compSize[V_COMP] = format->m_compSize[U_COMP];
  format->m_size             = format->m_compSize[Y_COMP] + format->m_compSize[U_COMP] + format->m_compSize[V_COMP];
  
  // For XYZ, YDzDx, or Yu'v' data we use the entire XYZ triangle
  if (format->m_colorSpace == CM_XYZ || format->m_colorSpace == CM_YDZDX || format->m_colorSpace == CM_YUpVp) {
    format->m_colorPrimaries = CP_XYZ;
  }
}


void Parameters::configure(char *filename, char **cl_params, int numCLParams, bool readConfig ) {
  // read configuration file
  if (readConfig == FALSE) {
    printf("Parsing default configuration file %s.\n", filename);
    readConfigFile(filename);
  }
  // now parse stored command line parameters as well
  parseCommandLineParams( cl_params, numCLParams );
  update();
}


int Parameters::readConfigFile(char *filename) {
  FILE *f;
  char line[MAX_LINE_LEN];
  int equal_pos;
  int i;
  char *p;
  int atoi_p;
  int matched;
  
  f = fopen(filename, "rt");
  if (!f)
    return ZERO;
  
  while (TRUE) {
    if (fgets(line, MAX_LINE_LEN, f) == NULL)
      break;
    
    if (feof(f))
      break;
    
    equal_pos = UNDEFINED;
    
    for (i = ZERO; i < MAX_LINE_LEN; i++) {
      if (line[i] == 0)
        break;
      else if (line[i] == '=') {
        line[i] = 0;
        equal_pos = i;
      }
      else if (line[i] == '#') {
        line[i] = 0;
        break;
      }
      else if (line[i] == 0x0d || line[i] == 0x0a)
        line[i] = 0;
      else if (line[i] == '"')
        line[i] = 0;
    }
    
    if (equal_pos != UNDEFINED) {
      matched = FALSE;
      // Check integer list
      for (i = 0; intParameterList[i].ptr != NULL; i++) {
        if (strcasecmp(line, intParameterList[i].name) == 0) {
          p = line+equal_pos+1;
          atoi_p = atoi(p);
          
          atoi_p = iClip(atoi_p, intParameterList[i].min_val, intParameterList[i].max_val);
          *(intParameterList[i].ptr) = atoi_p;
          matched = TRUE;
        }
      }
      // check bool list
      for (i = 0; boolParameterList[i].ptr != NULL; i++) {
        if (strcasecmp(line, boolParameterList[i].name) == 0) {
          p = line+equal_pos+1;
          atoi_p = atoi(p);

          // atoi_p = iClip(atoi_p, boolParameterList[i].min_val, boolParameterList[i].max_val);
          *(boolParameterList[i].ptr) =  (atoi_p ? TRUE : FALSE);
          matched = TRUE;
        }
      }
      // check floating parameter list
      for (i = 0; floatParameterList[i].ptr != NULL; i++) {
        if (strcasecmp(line, floatParameterList[i].name) == 0) {
          p = line+equal_pos+1;
          
          *(floatParameterList[i].ptr) = (float)atof(p);
          matched = TRUE;
        }
      }
      // check double parameter list
      for (i = 0; doubleParameterList[i].ptr != NULL; i++) {
        if (strcasecmp(line, doubleParameterList[i].name) == 0) {
          p = line+equal_pos+1;

          *(doubleParameterList[i].ptr) = (double)atof(p);
          matched = TRUE;
        }
      }
      // check string parameter list
      for (i = 0; stringParameterList[i].ptr != NULL; i++) {
        if (strcasecmp(line, stringParameterList[i].name) == 0) {
          p = line + equal_pos + 1;
          strcpy(stringParameterList[i].ptr, p+1);
          matched = TRUE;
        }
      }
      if (matched == FALSE) {
        printf("Unknown parameter <%s>\n", line);
      }
    }
  }
  
  fclose(f);
  
//#define PRINT_DEBUG_PARAMS

#ifdef PRINT_DEBUG_PARAMS
  for (i = 0; intParameterList[i].ptr != NULL; i++)
    printf("%s = %d\n", intParameterList[i].name, *(intParameterList[i].ptr));
  for (i = 0; boolParameterList[i].ptr != NULL; i++)
    printf("%s = %d\n", boolParameterList[i].name, *(boolParameterList[i].ptr));
  for (i = 0; floatParameterList[i].ptr != NULL; i++)
    printf("%s = %10.8f\n", floatParameterList[i].name, *(floatParameterList[i].ptr));
  for (i = 0; doubleParameterList[i].ptr != NULL; i++)
    printf("%s = %10.8f\n", doubleParameterList[i].name, *(doubleParameterList[i].ptr));
  for (i = 0; stringParameterList[i].ptr != NULL; i++)
    printf("%s = %d\n", stringParameterList[i].name, *(stringParameterList[i].ptr));
#endif

  return ONE;
}

// parse buffered command line parameters
int Parameters::parseCommandLineParams( char **cl_params, int numCLParams ) {
  int par, i, matched = FALSE;
  char *line = NULL, *p = NULL, *source = NULL;
  int atoi_p, equal_pos = UNDEFINED, param_length;
  char *pline = new char[80]; 

  for ( par = 0; par < numCLParams; par++ ) {
    memset(pline, 0, 80 * sizeof(char));
    param_length = 0;
    matched = FALSE;
    source = cl_params[par];
    line = source;

    while (*source != '\0') {
      if (*source == '=')  { // The Parser expects whitespace before and after '='
        break;
      } else
        param_length ++;
      source++;
    }

    strncpy(pline, line, iMin(80, param_length));
    pline[param_length + 1] = '\0';

    for (i = 0; intParameterList[i].ptr != NULL; i++) {
      if (strcasecmp(pline, intParameterList[i].name) == 0) {
        equal_pos = param_length;

        p = line + equal_pos + 1;
        atoi_p = atoi(p);
        atoi_p = iClip(atoi_p, intParameterList[i].min_val, intParameterList[i].max_val);
        *(intParameterList[i].ptr) = atoi_p;
        matched = TRUE;
        // printout message
        printf ("Parsing command line string '%s'\n", line);
      }
    }
    for (i = 0; boolParameterList[i].ptr != NULL; i++) {
      if (strcasecmp(pline, boolParameterList[i].name) == 0) {
        equal_pos = param_length;
        
        p = line + equal_pos + 1;
        atoi_p = atoi(p);
        
        // atoi_p = iClip(atoi_p, boolParameterList[i].min_val, boolParameterList[i].max_val);
        *(boolParameterList[i].ptr) = (atoi_p ? TRUE : FALSE);
        matched = TRUE;
        // printout message
        printf ("Parsing command line string '%s'\n", line);
      }
    }
    for (i = 0; floatParameterList[i].ptr != NULL; i++) {
      if (strcasecmp(pline, floatParameterList[i].name) == 0) {
        // assuming param=X get the position of the "=" sign
        equal_pos = param_length;

        p = line + equal_pos + 1;
        *(floatParameterList[i].ptr) = (float)atof(p);
        matched = TRUE;
        // printout message
        printf ("Parsing command line string '%s'\n", line);
      }
    }
    for (i = 0; doubleParameterList[i].ptr != NULL; i++) {
      if (strcasecmp(pline, doubleParameterList[i].name) == 0) {
        equal_pos = param_length;

        p = line + equal_pos + 1;
        *(doubleParameterList[i].ptr) = (double)atof(p);
        matched = TRUE;
        // printout message
        printf ("Parsing command line string '%s'\n", line);
      }
    }
    for (i = 0; stringParameterList[i].ptr != NULL; i++) {
      if (strcasecmp(pline, stringParameterList[i].name) == 0) {
        // assuming param=X get the position of the "=" sign
        equal_pos = param_length;

        p = line + equal_pos + 1;
        strcpy(stringParameterList[i].ptr, p); // + 1 because of """ character???
        matched = TRUE;
        // printout message
        printf ("Parsing command line string '%s'\n", line);
      }
    }
    if (matched == FALSE) {
      printf("Unknown parameter <%s>\n", line);
      printf("Please put all parameters as -p Param=X between executable name and -f file.cfg.\n Do not use quotes when X is a string.\n");
    }
  }

  delete[] pline;
  return ONE;
}

// parse buffered command line parameters
void Parameters::printParams() {
  int i;
  printf("Supported Parameters\n");
  printf("====================\n\n");
  printf("Integet type parameters:\n");
  printf("------------------------\n");
  printf("Name                                  Default         Min         Max   Description\n");
  printf("===================================================================================\n");
  for (i = 0; intParameterList[i].ptr != NULL; i++) {
    printf("%-32s = %8d\t{%8d, %10d} %s\n",intParameterList[i].name,
           intParameterList[i].default_val,
           intParameterList[i].min_val,
           intParameterList[i].max_val,
           intParameterList[i].description);
  }
  printf("\n");
  printf("Boolean type parameters:\n");
  printf("------------------------\n");
  printf("Name                                  Default         Min         Max   Description\n");
  printf("===================================================================================\n");
  for (i = 0; boolParameterList[i].ptr != NULL; i++) {
    printf("%-32s = %8d\t{%8d, %10d} %s\n",boolParameterList[i].name,
           boolParameterList[i].default_val,
           boolParameterList[i].min_val,
           boolParameterList[i].max_val,
           boolParameterList[i].description);
  }
  printf("\n");
  printf("Float type parameters:\n");
  printf("----------------------\n");
  printf("Name                                  Default      Min            Max      Description\n");
  printf("======================================================================================\n");
  for (i = 0; floatParameterList[i].ptr != NULL; i++) {
    printf("%-32s = %13.5f {%7.5f, %14.5f} %s\n",floatParameterList[i].name,
           floatParameterList[i].default_val,
           floatParameterList[i].min_val,
           floatParameterList[i].max_val,
           floatParameterList[i].description);
  }
  printf("\n");
  printf("Double type parameters:\n");
  printf("-----------------------\n");
  printf("Name                                  Default      Min            Max      Description\n");
  printf("======================================================================================\n");
  for (i = 0; doubleParameterList[i].ptr != NULL; i++) {
    printf("%-32s = %13.5f {%7.5f, %14.5f} %s\n",doubleParameterList[i].name,
           doubleParameterList[i].default_val,
           doubleParameterList[i].min_val,
           doubleParameterList[i].max_val,
           doubleParameterList[i].description);
  }
  printf("\n");
  printf("String type parameters:\n");
  printf("-----------------------\n");
  printf("Name                                                    Default         Description\n");
  printf("===================================================================================\n");
  for (i = 0; stringParameterList[i].ptr != NULL; i++) {
    printf("%-32s = %32s\t%s\n",stringParameterList[i].name,
           stringParameterList[i].default_val,
           stringParameterList[i].description);
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
