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
 * \file HDRMetrics.cpp
 *
 * \brief
 *    HDRMetrics main project class
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *************************************************************************************
 */


//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Global.H"
#include "ProjectParameters.H"
#include "Parameters.H"
#include "HDRMetrics.H"
#include "HDRMetricsFrame.H"

//-----------------------------------------------------------------------------
// Local functions
//-----------------------------------------------------------------------------

/*!
 ***********************************************************************
 * \brief
 *   print help message and exit
 ***********************************************************************
 */
void HDRMetricsExit (char *func_name) {
    printf("Usage: %s [-h] {[-H] [-s] [-m]} [-f config.cfg] "
    "{[-p Param1=Value1]..[-p ParamM=ValueM]}\n\n"

    "Options:\n"
    "   -h :  Help mode (this info)\n"
    "   -H :  Help mode (long format)\n"
    "   -s :  Silent mode\n"
    "   -f :  Read <config.cfg> for reseting selected parameters.\n"
    "   -p :  Set parameter <ParamM> to <ValueM>.\n"
    "         See default config.cfg file for description of all parameters.\n\n"
    
    "## Supported video file formats\n"
    "   RAW:  .yuv -> YUV 4:2:0\n\n"
    
    "## Examples of usage:\n"
    "   %s\n"
    "   %s  -h\n"
    "   %s  -H\n"
    "   %s  -f config.cfg\n"
    "   %s  -f config.cfg -p SourceFile=\"seq.yuv\" -p width=176 -p height=144\n" 
    ,func_name,func_name,func_name,func_name,func_name,func_name);
}

HDRMetrics *HDRMetrics::create(ProjectParameters *inputParams) {
  HDRMetrics *result = NULL;
    result = new HDRMetricsFrame(inputParams);
  return result;
}


//-----------------------------------------------------------------------------
// Main function
//-----------------------------------------------------------------------------

int main(int argc, char **argv) {
  int helpMode = 0;
  int a;
  HDRMetrics* hdrProcess;
  int numCLParams = 0, par;
  char **cl_params = new char* [MAX_CL_PARAMS];
  char *parfile= new char[FILE_NAME_SIZE];
  bool readConfig = FALSE;
  
  params = &ccParams;
  params->refresh();
  
  strcpy(parfile, DEFAULTCONFIGFILENAME );
  for ( par = 0; par < MAX_CL_PARAMS; par++ ) {
    cl_params[par] = new char [MAX_CL_PARAM_LENGTH];
    //Lets reset this to make sure no garbage remains
    memset(cl_params[par],0, MAX_CL_PARAM_LENGTH * sizeof(char));
  }
  
  // start here
  params->m_silentMode = FALSE;
  
  for (a = 1; a < argc; a++) {
    if (argv[a][ZERO] == '-') {
      if (strcmp(argv[a], "-h") == ZERO)
        helpMode = 1;
      else if (strcmp(argv[a], "-H") == ZERO)
        helpMode = 2;
      else if (strcasecmp (argv[a], "-v") == ZERO) {
        printf("%s ",argv[ZERO]);
        printf("V." VERSION ": compiled " __DATE__ " " __TIME__ "\n");
        
        exit(EXIT_FAILURE);
      }
      else if (strcasecmp(argv[a], "-s") == ZERO)
        params->m_silentMode = TRUE;
      else if (strcasecmp(argv[a], "-p") == ZERO) {
        // copy parameter to buffer
        if ( (a + 1) < argc ) {
          strncpy( cl_params[ numCLParams ], argv[a + 1],strlen(argv[a + 1]));
          numCLParams++;
          // jump ahead or else the loop will break and the cfg filename will not be reached
          // since it assumes it always finds "-"
          a++;
        }
        else {
          exit(EXIT_FAILURE);
        }
      }
      else if (strcasecmp(argv[a], "-f") == ZERO) {
        // input parameter file
        if ( (a + 1) < argc ) {
          strcpy(parfile, argv[a + 1] );
          printf("Parsing configuration file %s.\n", parfile);
          params->readConfigFile(parfile);
          readConfig = TRUE;
          a++;
        }
        else {
          exit(EXIT_FAILURE);
        }
      }
      else
        helpMode = 3;
    }
    else
      break;
  }
  
  if (helpMode != 0)
    params->m_silentMode = FALSE;
  
  if (params->m_silentMode == FALSE) {
    printf("---------------------------------------------------------\n");
    printf(" HDR Video Quality Analysis tool - Version %s (%s)\n",HDR_METRIC_VERSION, VERSION);
    printf("---------------------------------------------------------\n");
  }
  
  if (helpMode != 0) {
    HDRMetricsExit(argv[ZERO]);
    
    if (helpMode == 2)
      params->printParams();
    
    for (par = 0; par < MAX_CL_PARAMS; par++){
      delete [] cl_params[par];
    }
    
    delete [] cl_params;
    delete [] parfile;
    
    if (helpMode == 3)
      exit(EXIT_FAILURE);
    else
      exit(EXIT_SUCCESS);

  }
  
  // Prepare parameters
  params->configure(parfile, cl_params, numCLParams, readConfig );
  
  hdrProcess = HDRMetrics::create((ProjectParameters *) params);
  
  hdrProcess->init         ((ProjectParameters *) params);
  hdrProcess->outputHeader ((ProjectParameters *) params);
  hdrProcess->process      ((ProjectParameters *) params);
  hdrProcess->outputFooter ((ProjectParameters *) params);
  hdrProcess->destroy();
  
  delete hdrProcess;
  
  for (par = 0; par < MAX_CL_PARAMS; par++){
    delete [] cl_params[par];
  }
  
  delete [] cl_params;
  delete [] parfile;
  
  return EXIT_SUCCESS;  
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
