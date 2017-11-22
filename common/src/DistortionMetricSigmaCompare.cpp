/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Chad Fogg
 * <ORGANIZATION> = Chad Fogg
 * <YEAR> = 2014
 *
 * Copyright (c) 2014, Chad Fogg
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
 * \file DistortionMetricSigmaCompare.cpp
 *
 * \brief
 *  DistortionMetricSigmaCompare class source files 
 *  "A Quality Metric for High Dynamic Range" by Gary Demos   (garyd@alumni.caltech.edu)
 *   algorithm description in: JCTVC-R0214:   
 *        http://phenix.int-evry.fr/jct/doc_end_user/current_document.php?id=9285
 *
 * \author
 *     - Chad Fogg 
 *     - Alexis Michael Tourapis
 *************************************************************************************
 */


#include "DistortionMetricSigmaCompare.H"

#include <iostream>
#include <string.h>
#include <math.h>


//-----------------------------------------------------------------------------
// Macros / Constants
//-----------------------------------------------------------------------------
static const char *c_name[] = {"red","grn","blu"};

//-----------------------------------------------------------------------------
// Non Class functions
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

#if 0
/***********************************************************************************************************/
{
  int firstFrame=0, last_frame=0;
  short h_reso = 1920; // can be overriden by file header
  short v_reso = 1080;
  int chroma_format_idc = CF_444;
  int transferCharacteristics = 16;  // PQ
  int bitDepth = 10;
  int videoFullRangeFlag = 0;
  int integerFileFrameFlag = 0;
  
  f32_stats_t f32SigmaStats[3];
  u16_stats_t u16SigmaStats[3];
  
  short dpx_in_file1=0, dpx_in_file2=0;
  float amplitudeFactor;
  
  f32_pic_t f32_src_pic, f32_test_pic;
  u16_pic_t u16_src_pic, u16_test_pic;
  
  // TODO: clear structures with 0
  
  amplitudeFactor = 1.0;
  
  for(int c=0;c<3;c++ )
    f32_src_pic.buf[c] = f32_test_pic.buf[c] = NULL;
    
    if (argc <= 2) {
      printf(" usage: %s original_files, test_comparison_files, firstFrame, last_frame, amplitudeFactor\n", argv[0]);
      printf(" note: use %% format for frame numbers, such as %%07d, within the file names\n");
      exit(1);
    }
  
  if (argc > 4) {
    firstFrame = atoi(argv[3]);
    last_frame = atoi(argv[4]);
    printf(" firstFrame = %d last_frame = %d\n", firstFrame, last_frame);
    
  } else { /* argc <= 4 */
    printf(" no frame range, one frame will be processed\n");
  } /* argc > 4 or not */
  
  if (argc > 5) {
    amplitudeFactor = atof(argv[5]);
    printf(" setting amplitudeFactor to %f\n", amplitudeFactor);
    /* can scale nits to unity relationship here
     e.g. if 1.0=100nits, use 100.0 here for the amplitudeFactor in order to see the printout relative to nits (sigma and average value), does not affect selfRelative
     e.g. also if looking at nits, but wish to see in terms of 100nits = 1.0, then use .01 for amplitudeFactor to range 100nits down to 1.0 (e.g. a nominal white at 1.0) */
    //    } else { /* argc <= 5 */
    //        printf(" using default amplitudeFactor = %f\n", amplitudeFactor);
  } /* argc > 5 or not */
  
  if (argc > 6) {
    h_reso = atof(argv[6]);
    printf(" setting h_reso to %d\n", h_reso );
  }
  if (argc > 7) {
    v_reso = atof(argv[7]);
    printf(" setting v_reso to %d\n", v_reso );
  }
  
  if (argc > 8) {
    chroma_format_idc = atof(argv[8]);
    printf(" chroma_format_idc to %d\n", chroma_format_idc );
  }
  
  if (argc > 9) {
    transferCharacteristics  = atof(argv[9]);
    printf(" transferCharacteristics to %d\n", transferCharacteristics );
  }
  
  if (argc > 10) {
    bitDepth  = atof(argv[10]);
    printf(" bitDepth to %d\n", bitDepth );
  }
  
  if (argc > 11) {
    videoFullRangeFlag  = atof(argv[11]);
    printf(" videoFullRangeFlag to %d\n", videoFullRangeFlag );
  }
  
  short num_chars1 = strlen(argv[1]); /* length of in_file_name string */
  short num_chars2 = strlen(argv[2]); /* length of in_file_name string */
  
  if( !strcmp(&argv[1][num_chars1-4],  ".yuv") && !strcmp(&argv[2][num_chars2-4],  ".yuv"))
  {
    integerFileFrameFlag = 1;
  }
  
  
  if( integerFileFrameFlag )
  {
    initPic( &u16_src_pic, h_reso, v_reso, bitDepth, chroma_format_idc, videoFullRangeFlag,  transferCharacteristics  );
    
    initPic( &u16_test_pic, h_reso, v_reso, bitDepth, chroma_format_idc, videoFullRangeFlag,  transferCharacteristics  );
    
    initPic( &f32_src_pic , h_reso, v_reso );
    initPic( &f32_test_pic , h_reso, v_reso );
  }
  // first step done
  for(int c=0;c<3;c++) {
    initSigmaStats( &(f32SigmaStats[c]) );
    
    if( integerFileFrameFlag )
      initSigmaStats( &(u16SigmaStats[c]) );
  }
  
  int f32_file_input1 = 0;
  int f32_file_input2 = 0;
  
  // PASS 1
  
  for (int iframe = firstFrame; iframe <= last_frame; iframe++) {
    char infile1[300], infile2[300];
    
    sprintf(infile1, argv[1], iframe);
    sprintf(infile2, argv[2], iframe);
    
    char *path1 = argv[1];
    char *path2 = argv[2];
    
    short num_chars = strlen(infile1); /* length of in_file_name string */
    
    f32_file_input1 = f32_file_input2 = 1;
    
    if( !strcmp(&path1[num_chars-4],  ".yuv"))
    {
      f32_file_input1 = 0;
    }
    else if ((!strcmp(&path1[num_chars-1],  "x"))  ||(!strcmp(&path1[num_chars-1],    "X")) || /* DPX file ending in ".dpx" (not recommended, 10bit linear has insufficient precision near black) */
             (!strcmp(&path1[num_chars-3],  "x32"))||(!strcmp(&path1[num_chars-3],  "X32")) || /* dpx32 or DPX32 */
             (!strcmp(&path1[num_chars-3],  "x16"))||(!strcmp(&path1[num_chars-3],  "X16")) || /* dpx16 or DPX16 (not recommended) */
             (!strcmp(&path1[num_chars-4], "xhlf"))||(!strcmp(&path1[num_chars-4], "XHLF"))) { /* dpxhlf or DPXHLF, 16-bit half-float (non-standard, improperly defined in dpx 2.0 standard) */
      dpx_in_file1 = 1;
    }
    num_chars = strlen(infile2); /* length of in_file_name string */
    
    if( !strcmp(&path2[num_chars-4],  ".yuv"))
    {
      f32_file_input2 = 0;
    }
    else if ((!strcmp(&path2[num_chars-1],  "x"))  ||(!strcmp(&infile2[num_chars-1],    "X")) || /* DPX file ending in ".dpx" (not recommended, 10bit linear has insufficient precision near black) */
             (!strcmp(&path2[num_chars-3],  "x32"))||(!strcmp(&path2[num_chars-3],  "X32")) || /* dpx32 or DPX32 */
             (!strcmp(&path2[num_chars-3],  "x16"))||(!strcmp(&path2[num_chars-3],  "X16")) || /* dpx16 or DPX16 (not recommended) */
             (!strcmp(&path2[num_chars-4], "xhlf"))||(!strcmp(&path2[num_chars-4], "XHLF"))) { /* dpxhlf or DPXHLF, 16-bit half-float (non-standard, improperly defined in dpx 2.0 standard) */
      dpx_in_file2 = 1;
    }
    
    if( f32_file_input2 )
      readOneFrame( &f32_test_pic, infile2, dpx_in_file2, &h_reso, &v_reso,  amplitudeFactor );
      else
      {
        readOneFrame( &u16_test_pic, infile2 );
        convertShortFrameToFloat( &f32_test_pic, &u16_test_pic, amplitudeFactor );
      }
    
    if( f32_file_input1 )
      readOneFrame( &f32_src_pic, infile1, dpx_in_file1, &h_reso, &v_reso,  amplitudeFactor );
      else {
        readOneFrame( &u16_src_pic, infile1 );
        convertShortFrameToFloat( &f32_src_pic, &u16_src_pic, amplitudeFactor );
      }
    
    for (int c=0; c<3; c++) {
      measureFrameSigmaPass1( &(f32SigmaStats[c]),  f32_src_pic.buf[c], f32_test_pic.buf[c], h_reso, v_reso );
      
      if( integerFileFrameFlag ){
        measureFrameSigmaPass1( &(u16SigmaStats[c]), &(u16_src_pic.plane[c]), &(u16_test_pic.plane[c]));
      }
    } /* c */
  } /* iframe */

  // Calculate and print stats.
  calcPass1Stats( &(f32SigmaStats[0]),  amplitudeFactor );
  printPass1Stats( &(f32SigmaStats[0]),  amplitudeFactor );
  
  if( integerFileFrameFlag ){
    calcPass1Stats( &(u16SigmaStats[0]) );
    printPass1Stats( &(u16SigmaStats[0]),  amplitudeFactor, bitDepth, videoFullRangeFlag );
  }
  
  // PASS 2
  
  printf("\n beginning second pass over frames (multiples of sigma, now that sigma for all frames has been determined)\n\n");
  
  for (int iframe = firstFrame; iframe <= last_frame; iframe++) {
    char infile1[300], infile2[300];
    
    sprintf(infile1, argv[1], iframe);
    sprintf(infile2, argv[2], iframe);
    
    if( f32_file_input2 )
      readOneFrame( &f32_test_pic, infile2, dpx_in_file2, &h_reso, &v_reso,  amplitudeFactor );
      else{
        readOneFrame( &u16_test_pic, infile2 );
        convertShortFrameToFloat( &f32_test_pic, &u16_test_pic, amplitudeFactor );
        
      }
    
    
    if( f32_file_input1 )
      readOneFrame( &f32_src_pic, infile1, dpx_in_file1, &h_reso, &v_reso,  amplitudeFactor );
      else{
        readOneFrame( &u16_src_pic, infile1 );
        convertShortFrameToFloat( &f32_src_pic, &u16_src_pic, amplitudeFactor );
        
      }
    
    for (int c=0; c<3; c++) {
      measureFrameSigmaPass2( &(f32SigmaStats[c]),  f32_src_pic.buf[c], f32_test_pic.buf[c], h_reso, v_reso );
      
      
      if( integerFileFrameFlag ){
        measureFrameSigmaPass2( &(u16SigmaStats[c]), &(u16_src_pic.plane[c]), &(u16_test_pic.plane[c]));
      }
    } /* c */
  } /* iframe */
  
  printPass1Stats( &(f32SigmaStats[0]),  amplitudeFactor );  // repeated for some reason in Gary's original
  
  // calc. and print self relative
  printPass2Stats( &(f32SigmaStats[0]),  amplitudeFactor );
  
  if( integerFileFrameFlag ){
    printPass1Stats( &(u16SigmaStats[0]),  amplitudeFactor, bitDepth, videoFullRangeFlag  );
    printPass2Stats( &(u16SigmaStats[0]),  amplitudeFactor );
  }
} /* main */
#endif




DistortionMetricSigmaCompare::DistortionMetricSigmaCompare(const FrameFormat *format, double maxSampleValue, double amplitudeFactor )
: DistortionMetric()
{
  m_isFloat       = format->m_isFloat;
  m_colorSpace    = format->m_colorSpace;
  m_noPass        = 0;
    
  m_amplitudeFactor = amplitudeFactor;
  
  for (int c = 0; c < T_COMP; c++) {
    m_maxValue[c] = (double) maxSampleValue;
      
    initSigmaStats( &(m_f32SigmaStats[c]) );
    if( m_isFloat == FALSE )
      initSigmaStats( &(m_u16SigmaStats[c]) );
  }
}

DistortionMetricSigmaCompare::~DistortionMetricSigmaCompare()
{
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------
int DistortionMetricSigmaCompare::binValue( float sample )
{
  if ( sample  < 0.0) {
    return(-1);
  }
  else { /* col[c] >= 0 */
    
    float fltLog2 = (float) (log(fMax(1e-16f, sample))/ log(2.0));
    
    int j = (int) fltLog2; /* float to int */
    
    if (fltLog2 > 0.0) j = j + 1; /* must round one side of zero for continuity of integer j */
    
    j = (int) fMin(NUMBER_OF_F16_STOPS - 1, fMax(0.0f, j + NUMBER_OF_F16_STOPS / 2.0f)); /* split NUMBER_OF_STOPS half above 1.0 and half below 1.0 */
    return(j);
  } /* col[c] < 0 or not */
}

int DistortionMetricSigmaCompare::binValue( unsigned short sample )
{
  if ( sample  < 0.0) {
    return (-1);
  }
  else { /* col[c] >= 0 */
    float fltLog2 = (float) (log(fMax(1e-16f, (float)sample)) / log(2.0));
    int j = (int) fltLog2; /* float to int */
    
    //       if (fltLog2 > 0.0) j = j + 1; /* must round one side of zero for continuity of integer j */
    
    //       j = MIN(NUMBER_OF_U16_STOPS - 1, MAX(0, j + NUMBER_OF_U16_STOPS / 2)); /* split NUMBER_OF_STOPS half above 1.0 and half below 1.0 */
    return (j);
  } /* col[c] < 0 or not */
}

int DistortionMetricSigmaCompare::binValue( imgpel sample )
{
  if ( sample  < 0.0) {
    return (-1);
  }
  else { /* col[c] >= 0 */
    float fltLog2 = (float) (log(fMax(1e-16f, (float)sample)) / log(2.0));
    int j = (int) fltLog2; /* float to int */
    
    //       if (fltLog2 > 0.0) j = j + 1; /* must round one side of zero for continuity of integer j */
    
    //       j = MIN(NUMBER_OF_U16_STOPS - 1, MAX(0, j + NUMBER_OF_U16_STOPS / 2)); /* split NUMBER_OF_STOPS half above 1.0 and half below 1.0 */
    return (j);
  } /* col[c] < 0 or not */
}

// SMPTE FCD ST 2084
// non-linear to linear
float DistortionMetricSigmaCompare::PQ10000_f( float V)
{
  float L;
  // Lw, Lb not used since absolute Luma used for PQ
  // formula outputs normalized Luma from 0-1
  L = pow(fMax(pow(V, 1.0f/78.84375f) - 0.8359375f ,0.0f)/(18.8515625f - 18.6875f * pow(V, 1.0f/78.84375f)),1.0f/0.1593017578f);
  
  return L;
}

// SMPTE FCD ST 2084
// encode (V^gamma ish calculate L from V)
//  encode   V = ((c1+c2*Y**n))/(1+c3*Y**n))**m
float DistortionMetricSigmaCompare::PQ10000_r( float L)
{
  float V;
  // Lw, Lb not used since absolute Luma used for PQ
  // input assumes normalized luma 0-1
  V = pow((0.8359375f + 18.8515625f * pow((L),0.1593017578f))/(1.0f + 18.6875f * pow((L),0.1593017578f)),78.84375f);
  return V;
}


// BT.2020 , BT.601, BT.709 EOTF:  non-linear (gamma) --> linear
float DistortionMetricSigmaCompare::bt1886_f( float V, float gamma, float Lw, float Lb)
{
  // The reference EOTF specified in Rec. ITU-R BT.1886
  // L = a(max[(V+b),0])^g
  float a = pow( pow( Lw, 1.0f / gamma) - pow( Lb, 1.0f / gamma), gamma);
  float b = pow( Lb, 1.0f / gamma) / ( pow( Lw, 1.0f / gamma) - pow( Lb, 1.0f / gamma));
  float L = a * pow( fMax( V + b, 0.0f), gamma);
  return L;
}


// BT.2020 , BT.601, BT.709 OETF:  linear --> non-linear (gamma)
float DistortionMetricSigmaCompare::bt1886_r( float L, float gamma, float Lw, float Lb)
{
  // The reference EOTF specified in Rec. ITU-R BT.1886
  // L = a(max[(V+b),0])^g
  float a = pow( pow( Lw, 1.0f / gamma) - pow( Lb, 1.0f / gamma), gamma);
  float b = pow( Lb, 1.0f / gamma) / ( pow( Lw, 1.0f / gamma) - pow( Lb, 1.0f / gamma));
  float V = pow( fMax( L / a, 0.0f), 1.0f / gamma) - b;
  return V;
}


#if 0

void DistortionMetricSigmaCompare::convert_u16_plane_to_f32( float *dst_plane, u16_plane_t *p,  float amplitudeFactor )
{
  int plane_width = p->width;
  int plane_height = p->height;
  unsigned short *src_pic = p->buf;
  int minLevel = p->minLevel;
  int  maxLevel = p->maxLevel;
  //, int bitDepth,
  int transferCharacteristics = p->transferCharacteristics;
  
  int sample_range =  maxLevel - minLevel + 1;
  
  for(int y=0; y< plane_height; y++)
  {
    for(int x=0; x <  plane_width; x++)
    {
      
      short src_sample = src_pic[ y * plane_width + x];
      
      // order is R,G,B in the JCT adhoc's  ... not G,B,R like our internal format here
      
      src_sample = src_sample > maxLevel ? maxLevel : src_sample;
      src_sample = src_sample < minLevel ? minLevel  : src_sample;
      
      float dst_sample = ((float) (src_sample-minLevel) ) / sample_range;
      
      dst_sample  *= amplitudeFactor;
      
      
      dst_sample = dst_sample > 1.0 ? 1.0 : dst_sample;
      dst_sample = dst_sample < 0.0 ? 0.0:  dst_sample;
      
      if( transferCharacteristics == 16 )
        dst_sample = PQ10000_f( dst_sample ) * 10000;
      
      dst_plane[ y * plane_width + x] = dst_sample;
    }
  }
}

void DistortionMetricSigmaCompare::convertShortFrameToFloat( f32_pic_t *dst_pic, u16_pic_t *src_pic, float amplitudeFactor )
{
  for( int c = 0; c<3; c++ ){
    u16_plane_t *p = &(src_pic->plane[c]);
    convert_u16_plane_to_f32( dst_pic->buf[c], p, amplitudeFactor );
  }
}

#endif

void DistortionMetricSigmaCompare::calcSigma( f32_sigma_t * s )
{
  if (s->fStopCount > 0)  {
    s->MSE = s->squareSumError / s->fStopCount;
    s->sigma = sqrt( s->MSE);
    s->average = s->sumOrigVal  / s->fStopCount;
    s->selfRelative = s->sigma / s->average;
  } /* count[j] > 0 */
}

void DistortionMetricSigmaCompare::calcSigma( u16_sigma_t * s )
{
  
  if (s->fStopCount > 0)  {
    s->MSE = (double) s->squareSumError / (double)  s->fStopCount;
    s->sigma = sqrt(s->MSE);
    s->average = (double) s->sumOrigVal  / (double)  s->fStopCount;
    s->selfRelative = s->sigma / s->average;
  } /* count[j] > 0 */
}

#ifdef COMPUTE_LOCAL_SIGMA

float DistortionMetricSigmaCompare::calcLocalSigma( int width, int height, int x, int y, float *plane )
{
  double localMean = 0.0;
  double localSigma = 0.0;
  
  int radius = 1;
  
  // look at neighborhood
  int ymin = iMax(0, y - radius);
  int ymax = iMin(y + radius, height - 1);
  int xmin = iMax(0, x - radius);
  int xmax = iMin(x + radius, width - 1);
  
  // compute local mean
  int n = 0;
  
  for( int j = ymin; j <= ymax; j++ )  {
    for( int i = xmin; i <= xmax; i++ )   {
      localMean += plane[ j * width + i ];
      n++;
    }
  }
  
  localMean = localMean / (double) n;
  
  // compute local variance (unlike SSIM, this is non-weighted by a guassian)
  for( int j = ymin; j <= ymax; j++ )  {
    for( int i = xmin; i <= xmax; i++ )    {
      double diff = ( plane[ j * width + i ] - localMean );
      localSigma += diff * diff;
    }
  }
  
  // variance
  localSigma = localSigma / (double) n;
  
  // sigma (standard deviation)
  localSigma =  sqrt( localSigma );
  
  return( (float) localSigma );
}
#endif

/* turns k=0,1,2,3,4,5,6,7 into multiple=2,3,4,6,8,12,16 */
int DistortionMetricSigmaCompare::kMultiple( int k )
{
  int multiple = (1 << ((k>>1) + 1));
  multiple = multiple + (k & 1) * (multiple >> 1);
  
  return (multiple);
}

void DistortionMetricSigmaCompare::initSigmaStats( f32_stats_t *stat )
{
  memset(  &(stat->neg), 0, sizeof( f32_sigma_t ) );
  
  for (int j=0; j<NUMBER_OF_F16_STOPS; j++){
    memset(  &(stat->pos[j]), 0, sizeof( f32_sigma_t ) );
    
#ifdef COMPUTE_LOCAL_SIGMA
    for (int k=0; k<NUMBER_OF_F16_STOPS; k++)
      memset(  &(stat->lpos[k][j]), 0, sizeof( f32_sigma_t ) );
#endif
  }
}

void DistortionMetricSigmaCompare::initSigmaStats( u16_stats_t *stat )
{
  memset(  &(stat->neg), 0, sizeof( u16_sigma_t ) );
  
  for (int j=0; j<NUMBER_OF_U16_STOPS; j++){
    memset(  &(stat->pos[j]), 0, sizeof( u16_sigma_t ) );
    
#ifdef COMPUTE_LOCAL_SIGMA
    for (int k=0; k<NUMBER_OF_U16_STOPS; k++)
      memset(  &(stat->lpos[k][j]), 0, sizeof( u16_sigma_t ) );
#endif
  }
}

void DistortionMetricSigmaCompare::measureFrameSigmaPass1( f32_stats_t *sig,  float *src_pic, float *test_pic, int width, int height )
{
  for(int y=0; y< height; y++)  {
    for(int x=0; x< width; x++)    {
      int j;
      
      // note: m_amplitudeFactor is only used for float pictures
      float val = (float) (src_pic[ y * width + x] * m_amplitudeFactor);
      
      /* store difference from original in pixels2 */
      float dif = (float) (val - test_pic[ y * width + x] * m_amplitudeFactor);
      
#ifdef COMPUTE_LOCAL_SIGMA
      float localSigma = calcLocalSigma( width, height, x, y, src_pic );
      float localSigma_log2 = log2f( fMax(1e-16,localSigma));
      int k = localSigma_log2; /* float to int */
      
      if (localSigma_log2 > 0.0)
        k++; /* must round one side of zero for continuity of integer j */
#endif
      if (val < 0.0) {
        sig->neg.squareSumError += dif * dif;
        sig->neg.sumOrigVal += val;
        sig->neg.fStopCount++;
      }
      else { /* col[c] >= 0 */
        float fltLog2 = (float) (log(fMax (1e-16f, val)) / log(2.0));
        j = (int) fltLog2; /* float to int */
        if (fltLog2 > 0.0) j = j + 1; /* must round one side of zero for continuity of integer j */
        
        
        j = (int) fMin(NUMBER_OF_F16_STOPS - 1.0f, fMax(0.0f, j + NUMBER_OF_F16_STOPS / 2.0f)); /* split NUMBER_OF_STOPS half above 1.0 and half below 1.0 */
        
        if( dif != 0.0 ){
          sig->pos[j].squareSumError += dif * dif;
#ifdef COMPUTE_LOCAL_SIGMA
          sig->lpos[k][j].squareSumError += dif * dif;
#endif
        }
        
        sig->pos[j].sumOrigVal += val;
#ifdef COMPUTE_LOCAL_SIGMA
        sig->lpos[k][j].sumOrigVal += val;
#endif
        sig->pos[j].fStopCount++;
#ifdef COMPUTE_LOCAL_SIGMA
        sig->lpos[k][j].fStopCount++;
#endif
        
      } /* col[c] < 0 or not */
    } /* x */
  } /* y */
}

void DistortionMetricSigmaCompare::measureFrameSigmaPass1( u16_stats_t *sig,  uint16 *src_pic, uint16 *test_pic, int width, int height )
{
  int y, x, j;
  
  for(y=0; y< height; y++)  {
    for(x=0; x< width; x++)    {
      unsigned short val = src_pic[ y * width + x];
      
#ifdef COMPUTE_LOCAL_SIGMA
      float localMean = 0.0;
      float localSigma = 0.0;
      
      int radius = 1;
      
      // look at neighborhood
      int ymin = iMax(0, y - radius);
      int ymax = iMin(y + radius, height -1 );
      int xmin = iMax(0, x - radius);
      int xmax = iMin(x + radius, width - 1);
      
      // compute local mean
      int n = 0;
      for( int j=ymin; j<=ymax; j++ )      {
        for( int i=xmin; i<=xmax; i++ )        {
          localMean += src_pic[ j * width + i ];
          n++;
        }
      }
      
      localMean = localMean / (float) n;
      
      // compute local variance (unlike SSIM, this is non-weighted by a guassian)
      for( j=ymin; j<=ymax; j++ )  {
        for( i=xmin; i<=xmax; i++ )   {
          float diff = ( ((float) src_pic[ j * width + i ]) - localMean );
          localSigma += diff * diff;
        }
      }
      
      // MSE
      localSigma = localSigma / n;
      
      // RMSE (sigma)
      localSigma = sqrt( localSigma ) + 1.0;
      
      localSigma = localSigma < 1.0 ? 1.0 : localSigma;
#endif
      /* store difference from original in pixels2 */
      int dif = val - test_pic[ y * width + x];
      
      
      if (val < 0.0) { // TODO: update this to consider integer footroom as negative
        sig->neg.squareSumError += dif * dif;
        sig->neg.sumOrigVal += val;
        sig->neg.fStopCount++;
      }
      else { /* col[c] >= 0 */
        
        float fltLog2 = (float) (log(fMax(1e-16f, val)) / log(2.0));
        j = (int) fltLog2; /* float to int */
        
#ifdef COMPUTE_LOCAL_SIGMA
        float sig_log2 = log2f(fMax(0, localSigma ));
        int jd = sig_log2; /* float to int */
#endif
        if( dif != 0 ){
          sig->pos[j].squareSumError += dif * dif;
#ifdef COMPUTE_LOCAL_SIGMA
          sig->lpos[jd][j].squareSumError += dif * dif;
#endif
        }
        
        
        sig->pos[j].sumOrigVal += val;
#ifdef COMPUTE_LOCAL_SIGMA
        sig->lpos[jd][j].sumOrigVal += val;
#endif
        
        sig->pos[j].fStopCount++;
#ifdef COMPUTE_LOCAL_SIGMA
        sig->lpos[jd][j].fStopCount++;
#endif
        //       sig->local_fStopCount[j]++;
      } /* col[c] < 0 or not */
    } /* x */
  } /* y */
}

void DistortionMetricSigmaCompare::measureFrameSigmaPass1( u16_stats_t *sig,  imgpel *src_pic, imgpel *test_pic, int width, int height )
{
  for(int y=0; y< height; y++)  {
    for(int x=0; x< width; x++)    {
      
      unsigned short val = src_pic[ y * width + x];

#ifdef COMPUTE_LOCAL_SIGMA
        
      float localMean = 0.0;
      float localSigma = 0.0;
      
      int radius = 1;
      
      // look at neighborhood
      int ymin = iMax(0, y - radius);
      int ymax = iMin(y + radius, height -1 );
      int xmin = iMax(0, x - radius);
      int xmax = iMin(x + radius, width - 1);
      
      // compute local mean
      int n = 0;
      for( int j=ymin; j<=ymax; j++ )      {
        for( int i=xmin; i<=xmax; i++ )        {
          localMean += src_pic[ j * width + i ];
          n++;
        }
      }
      
      localMean = localMean / (float) n;
      
      // compute local variance (unlike SSIM, this is non-weighted by a guassian)
      for( j=ymin; j<=ymax; j++ )  {
        for( i=xmin; i<=xmax; i++ )   {
          float diff = ( ((float) src_pic[ j * width + i ]) - localMean );
          localSigma += diff * diff;
        }
      }
      
      // MSE
      localSigma = localSigma / n;
      
      // RMSE (sigma)
      localSigma = sqrt( localSigma ) + 1.0;
      
      localSigma = localSigma < 1.0 ? 1.0 : localSigma;
#endif
      /* store difference from original in pixels2 */
      int dif = val - test_pic[ y * width + x];
      
      
      if (val < 0.0) { // TODO: update this to consider integer footroom as negative
        sig->neg.squareSumError += dif * dif;
        sig->neg.sumOrigVal += val;
        sig->neg.fStopCount++;
      }
      else { /* col[c] >= 0 */
        
        float fltLog2 = (float) (log(fMax(1e-16f, val)) / log(2.0));
        int j = (int) fltLog2; /* float to int */

#ifdef COMPUTE_LOCAL_SIGMA
        float sig_log2 = log2f(fMax(0, localSigma ));
        int jd = sig_log2; /* float to int */
#endif
        if( dif != 0 ){
          sig->pos[j].squareSumError += dif * dif;
            
#ifdef COMPUTE_LOCAL_SIGMA
          sig->lpos[jd][j].squareSumError += dif * dif;
#endif
        }
        
        
        sig->pos[j].sumOrigVal += val;
#ifdef COMPUTE_LOCAL_SIGMA
        sig->lpos[jd][j].sumOrigVal += val;
#endif
        
        sig->pos[j].fStopCount++;
          
#ifdef COMPUTE_LOCAL_SIGMA
        sig->lpos[jd][j].fStopCount++;
#endif
        
        //       sig->local_fStopCount[j]++;
      } /* col[c] < 0 or not */
    } /* x */
  } /* y */
}


void DistortionMetricSigmaCompare::measureFrameSigmaPass2( f32_stats_t*sig,  float *src_pic, float *test_pic, int width, int height )
{
  for(int y=0; y< height; y++)  {
    for(int x=0; x< width; x++)  {
      int j = binValue( src_pic[ y * width + x ]  );
      
      if (j  >= 0)   { /* dont count negs */
        float dif;
        
        if (sig->pos[j].fStopCount > 0)  {
          dif = (float) ((src_pic[ y* width + x ] -  test_pic[y * width + x]) * m_amplitudeFactor);
          
          for(int k=0; k < SIGMA_MULTIPLES; k++) {
            short multiple = kMultiple(k);
            
            if (fAbs(dif) > (multiple * sig->pos[j].sigma))    {
              sig->pos[j].countMultipleSigma[k]++;
              
              if(k== (SIGMA_MULTIPLES-1)) {
                //  printf(" c=%d j=%d x=%d y=%d col_dif[%d]=%e\n", c, j, x, y, c, col_dif[c]);
                sig->pos[j].furthestOutlierSum += fAbs(dif);
              } /* k== (SIGMA_MULTIPLES-1) */
            } /* fAbs(col_dif[c]) > (multiple * sigma[j]) */
          } /* k */
        } /* count[j] > 0 */
      } /* bin_value[(c*v_reso + y) * h_reso + x] >= 0 */
      
    } /* x */
  } /* y */
}

void DistortionMetricSigmaCompare::measureFrameSigmaPass2( u16_stats_t *sig,  uint16 *src_pic, uint16 *test_pic, int width, int height )
{
  for(int y=0; y< height; y++)  {
    for(int x=0; x< width; x++)  {
      int j = binValue( src_pic[ y * width + x ]  );
      
      if (j  >= 0)   { /* dont count negs */
        float dif;
        
        if (sig->pos[j].fStopCount > 0)  {
          dif = (float) (src_pic[ y* width + x ] -  test_pic[y * width + x]);
          
          for(int k=0; k < SIGMA_MULTIPLES; k++) {
            short multiple = kMultiple(k);
            
            if (fAbs(dif) > (multiple * sig->pos[j].sigma))    {
              sig->pos[j].countMultipleSigma[k]++;
              
              if(k== (SIGMA_MULTIPLES-1)) {
                //  printf(" c=%d j=%d x=%d y=%d col_dif[%d]=%e\n", c, j, x, y, c, col_dif[c]);
                sig->pos[j].furthestOutlierSum += (int64) fAbs(dif);
              } /* k== (SIGMA_MULTIPLES-1) */
            } /* fAbs(col_dif[c]) > (multiple * sigma[j]) */
          } /* k */
        } /* count[j] > 0 */
      } /* bin_value[(c*v_reso + y) * h_reso + x] >= 0 */
      
    } /* x */
  } /* y */
}

void DistortionMetricSigmaCompare::measureFrameSigmaPass2( u16_stats_t *sig,  imgpel *src_pic, imgpel *test_pic, int width, int height )
{
  for(int y=0; y< height; y++)  {
    for(int x=0; x< width; x++)  {
      int j = binValue( src_pic[ y * width + x ]  );
      
      if (j  >= 0)   { /* dont count negs */
        float dif;
        
        if (sig->pos[j].fStopCount > 0)  {
          dif = (float) (src_pic[ y* width + x ] -  test_pic[y * width + x]);
          
          for(int k=0; k < SIGMA_MULTIPLES; k++) {
            short multiple = kMultiple(k);
            
            if (fAbs(dif) > (multiple * sig->pos[j].sigma))    {
              sig->pos[j].countMultipleSigma[k]++;
              
              if(k== (SIGMA_MULTIPLES-1)) {
                //  printf(" c=%d j=%d x=%d y=%d col_dif[%d]=%e\n", c, j, x, y, c, col_dif[c]);
                sig->pos[j].furthestOutlierSum += (int64) fAbs(dif);
              } /* k== (SIGMA_MULTIPLES-1) */
            } /* fAbs(col_dif[c]) > (multiple * sigma[j]) */
          } /* k */
        } /* count[j] > 0 */
      } /* bin_value[(c*v_reso + y) * h_reso + x] >= 0 */
      
    } /* x */
  } /* y */
}


void DistortionMetricSigmaCompare::calcPass1Stats( )
{
  for (int c=0; c<3; c++)  {
    f32_stats_t*sig = &(m_f32SigmaStats[c]);
    
    calcSigma( &(sig->neg));
    
    for (int j=0; j<NUMBER_OF_F16_STOPS; j++)    {
      calcSigma( &(sig->pos[j]) );
      
#ifdef COMPUTE_LOCAL_SIGMA
      for (int k=0; k<NUMBER_OF_F16_STOPS; k++)      {
        calcSigma( &(sig->lpos[k][j]) );
      }
#endif
    }
  }
  
  if (m_isFloat == FALSE) {
    for (int c=0; c<3; c++)  {
      u16_stats_t *sig = &(m_u16SigmaStats[c]);
      
      calcSigma( &(sig->neg));
      
      for (int j=0; j<NUMBER_OF_U16_STOPS; j++)    {
        calcSigma( &(sig->pos[j]) );
        
#ifdef COMPUTE_LOCAL_SIGMA
        for (int k=0; k<NUMBER_OF_U16_STOPS; k++)      {
          calcSigma( &(sig->lpos[k][j]) );
        }
#endif
      } /* j */
    } /* c */
  }
}


void DistortionMetricSigmaCompare::printPass1Stats()
{
    
  printf("\n amplitudeFactor = %f\n" , m_amplitudeFactor);
    
#ifdef COMPUTE_LOCAL_SIGMA
  
  printf("LOCAL SIGMA stats\n");
  
  for (int c=0; c<3; c++)  {
    f32_stats_t *sig = &(m_f32SigmaStats[c]);
    
    printf("\n============ %s: ==============\n",  c_name[c] );
    
    printf("self relative (rows: local pixel sigma class,  cols: frame global sigma)\n");
    
    for (int j=0; j<NUMBER_OF_F16_STOPS; j++)    {
      for (int k=0; k<NUMBER_OF_F16_STOPS; k++)      {
        printf("%f,", sig->lpos[k][j].selfRelative );
      }
      
      printf("\n");
    }
  }
  
  
#endif
  
  for (int c=0; c<3; c++)  {
    f32_stats_t*sig = &(m_f32SigmaStats[c]);
    
    printf("\n");
    if (sig->neg.fStopCount > 0)    {
      printf(" sigma_neg_%s = %e self_relatve = %f (%f%%) at average value = %e for %.0f pixels\n", c_name[c],
             sig->neg.sigma,
             sig->neg.selfRelative,
             100.0 * sig->neg.selfRelative,
             sig->neg.average,
             sig->neg.fStopCount);
      
    } /* count_neg[c] > 0 */
    
    for (int j=0; j<NUMBER_OF_F16_STOPS; j++) {
      if (sig->pos[j].fStopCount > 0)  {
        printf(" sigma_%s[%d] = %e selfRelative = %f (%f%%) at average value = %e for %.0f pixels\n", c_name[c],
               j, sig->pos[j].sigma, sig->pos[j].selfRelative, 100.0 * sig->pos[j].selfRelative, sig->pos[j].average, sig->pos[j].fStopCount);
        
      } /* count[j] > 0 */
    } /* j */
  } /* c */
  
  if (m_isFloat == FALSE) {
    //printf("\n INTEGER MEASURE\n amplitudeFactor = %f\n", amplitudeFactor);
    
    for (int c=0; c<3; c++)  {
#ifdef COMPUTE_LOCAL_SIGMA
      u16_stats_t*sig = &(m_u16SigmaStats[c]);
#endif
      printf("============ %s: ==============\n",  c_name[c] );
      
      printf("self relative:\n");
      
      for (int j=0; j<NUMBER_OF_U16_STOPS; j++)    {
          
#ifdef COMPUTE_LOCAL_SIGMA
        for (int k=0; k<NUMBER_OF_U16_STOPS; k++)      {
          printf(" %8.5f%%\t", 100.0 * sig->lpos[k][j].selfRelative );
        }
#endif
        printf("\n");
      }
    }
    
    for (int c=0; c<3; c++)  {
      u16_stats_t*sig = &(m_u16SigmaStats[c]);
      
      printf("\n");
      if (sig->neg.fStopCount > 0)    {
        printf(" sigmaNeg_%s = %e selfRelative = %f (%f%%) at average value = %e for %lld pixels\n", c_name[c],
               sig->neg.sigma,
               sig->neg.selfRelative,
               100.0 * sig->neg.selfRelative,
               sig->neg.average,
               sig->neg.fStopCount);
        
      } /* count_neg[c] > 0 */
      
      for (int j=0; j<NUMBER_OF_F16_STOPS; j++)    {
        if (sig != NULL && sig->pos[j].fStopCount > 0)      {
          printf(" sigma_%s[%d] = %e selfRelative = %f (%f%%) at average value = %e for %lld pixels\n", c_name[c],
                 j, sig->pos[j].sigma, sig->pos[j].selfRelative, 100.0 * sig->pos[j].selfRelative, sig->pos[j].average, sig->pos[j].fStopCount);
          
        } /* count[j] > 0 */
      } /* j */
    } /* c */
  }
}

void DistortionMetricSigmaCompare::printPass2Stats()
{
  short multiple;
  for (int c=0; c<3; c++) {
    f32_stats_t*sig = &(m_f32SigmaStats[c]);
    
    printf("\n");
    for (int j=0; j<NUMBER_OF_F16_STOPS; j++) {
      if ((sig->pos[j].fStopCount > 0) && (sig->pos[j].countMultipleSigma[0] > 0)) {
        printf("  sigma_%s[%d] = %e selfRelative = %f (%f%%) at average value = %e for %.0f pixels\n", c_name[c],
               j, sig->pos[j].sigma, sig->pos[j].selfRelative, 100.0 * sig->pos[j].selfRelative, sig->pos[j].average, sig->pos[j].fStopCount);
      } /* count[j] > 0 */
      for( int k=0; k < (SIGMA_MULTIPLES-1); k++)      {
        if ((sig->pos[j].countMultipleSigma[k] > 0) && (sig->pos[j].countMultipleSigma[k] > sig->pos[j].countMultipleSigma[k+1]))        {
          multiple = kMultiple( k );
          
          printf(" %dsigma_%s[%d] = %e selfRelative = %f (%f%%) at average value = %e for %.0f pixels which is %f %% of the %.0f pixels within a stop of this value\n",
                 multiple, c_name[c], j, multiple*sig->pos[j].sigma, multiple*sig->pos[j].selfRelative, 100.0 * multiple*sig->pos[j].selfRelative, sig->pos[j].average, sig->pos[j].countMultipleSigma[k], (sig->pos[j].countMultipleSigma[k] * 100.0)/(1.0 * sig->pos[j].fStopCount), sig->pos[j].fStopCount);
        } /* countMultipleSigma[k][j] > 0 */
      } /* k */
      
      int k = SIGMA_MULTIPLES-1;
      
      if (sig->pos[j].countMultipleSigma[k] > 0)      {
        sig->pos[j].furthestOutlierAvg = sig->pos[j].furthestOutlierSum / sig->pos[j].countMultipleSigma[k]; /* compute average from sum */
        multiple = (1 << ((k>>1) + 1));
        multiple = multiple + (k & 1) * (multiple >> 1); /* turns k=0,1,2,3,4,5,6,7 into multiple=2,3,4,6,8,12,16 */
        printf(" beyond %dsigma_%s[%d] furthestOutlierAvg dif = %e which is %f sigma (sigma being %e), selfRelative = %f (%f%%) at average value = %e for %.0f pixels which is %f %% of the %.0f pixels within a stop of this value\n",
               multiple, c_name[c], j, sig->pos[j].furthestOutlierAvg, sig->pos[j].furthestOutlierAvg / sig->pos[j].sigma, sig->pos[j].sigma, sig->pos[j].furthestOutlierAvg/ sig->pos[j].average, 100.0 * sig->pos[j].furthestOutlierAvg / sig->pos[j].average, sig->pos[j].average, sig->pos[j].countMultipleSigma[k], (sig->pos[j].countMultipleSigma[k]* 100.0)/(1.0 * sig->pos[j].fStopCount), sig->pos[j].fStopCount);
      } /* (countMultipleSigma[k][j] > 0) */
    } /* j */
  } /* c */
  
  printf("INTEGER PASS 2\n ");
  
  for (int c=0; c<3; c++) {
    u16_stats_t*sig = &(m_u16SigmaStats[c]);
    
    printf("\n");
    for (int j=0; j<NUMBER_OF_U16_STOPS; j++) {
      if ((sig->pos[j].fStopCount > 0) && (sig->pos[j].countMultipleSigma[0] > 0)) {
        printf("  sigma_%s[%d] = %e selfRelative = %f (%f%%) at average value = %e for %lld pixels\n", c_name[c],
               j, sig->pos[j].sigma, sig->pos[j].selfRelative, 100.0 * sig->pos[j].selfRelative, sig->pos[j].average, sig->pos[j].fStopCount);
      } /* count[j] > 0 */
      for( int k=0; k < (SIGMA_MULTIPLES-1); k++)      {
        if ((sig->pos[j].countMultipleSigma[k] > 0) && (sig->pos[j].countMultipleSigma[k] > sig->pos[j].countMultipleSigma[k+1]))        {
          multiple = kMultiple( k );
          
          printf(" %dsigma_%s[%d] = %e selfRelative = %f (%f%%) at average value = %e for %lld pixels which is %f %% of the %lld pixels within a stop of this value\n",
                 multiple, c_name[c], j, multiple*sig->pos[j].sigma, multiple*sig->pos[j].selfRelative, 100.0 * multiple*sig->pos[j].selfRelative, sig->pos[j].average, sig->pos[j].countMultipleSigma[k], (sig->pos[j].countMultipleSigma[k] * 100.0)/(1.0 * sig->pos[j].fStopCount), sig->pos[j].fStopCount);
        } /* countMultipleSigma[k][j] > 0 */
      } /* k */
      
      int k = SIGMA_MULTIPLES-1;
      
      if (sig->pos[j].countMultipleSigma[k] > 0)      {
        sig->pos[j].furthestOutlierAvg = (double) (sig->pos[j].furthestOutlierSum) / (double) (sig->pos[j].countMultipleSigma[k]); /* compute average from sum */
        multiple = (1 << ((k>>1) + 1));
        multiple = multiple + (k & 1) * (multiple >> 1); /* turns k=0,1,2,3,4,5,6,7 into multiple=2,3,4,6,8,12,16 */
        printf(" XXXbeyond %dsigma_%s[%d] furthestOutlierAvg dif = %e which is %f sigma (sigma being %e), selfRelative = %f (%f%%) at average value = %e for %lld pixels which is %f %% of the %lld pixels within a stop of this value\n",
               multiple, c_name[c], j, sig->pos[j].furthestOutlierAvg, sig->pos[j].furthestOutlierAvg / sig->pos[j].sigma, sig->pos[j].sigma, sig->pos[j].furthestOutlierAvg / sig->pos[j].average, 100.0 * sig->pos[j].furthestOutlierAvg / sig->pos[j].average, sig->pos[j].average, sig->pos[j].countMultipleSigma[k], (sig->pos[j].countMultipleSigma[k] * 100.0)/(1.0 * sig->pos[j].fStopCount), sig->pos[j].fStopCount);
      } /* (countMultipleSigma[k][j] > 0) */
    } /* j */
  } /* c */
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

void DistortionMetricSigmaCompare::computeMetric (Frame* inp0, Frame* inp1)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
      
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        if (m_noPass == 0)
          measureFrameSigmaPass1( &(m_f32SigmaStats[c]),  inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_width[c], inp0->m_height[c] );
        else
          measureFrameSigmaPass2( &(m_f32SigmaStats[c]),  inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_width[c], inp0->m_height[c] );
        //m_metric[c] = psnr(m_maxValue[c], inp0->m_compSize[c], m_sse[c]);
        //m_metricStats[c].updateStats(m_metric[c]);
      }
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        // quantity as well. TBD. Should this be done externally and carry around both quantites in the frame?
        // convert inpX->m_comp[c] to inpX->m_floatComp[c]
        //measureFrameSigmaPass1( &(m_f32SigmaStats[c]),  inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_width[c], inp0->m_height[c] );
        if (m_noPass == 0)
          measureFrameSigmaPass1( &(m_u16SigmaStats[c]),  inp0->m_comp[c], inp1->m_comp[c], inp0->m_width[c], inp0->m_height[c] );
        else
          measureFrameSigmaPass2( &(m_u16SigmaStats[c]),  inp0->m_comp[c], inp1->m_comp[c], inp0->m_width[c], inp0->m_height[c] );
      }
    }
    else { // 16 bit data
      for (int c = Y_COMP; c < inp0->m_noComponents; c++) {
        // ideally here we should also compute a floating representation of the image and compute the floating point
        // quantity as well. TBD. Should this be done externally and carry around both quantites in the frame?
        // convert inpX->m_ui16Comp[c] to inpX->m_floatComp[c]
        //measureFrameSigmaPass1( &(m_f32SigmaStats[c]),  inp0->m_floatComp[c], inp1->m_floatComp[c], inp0->m_width[c], inp0->m_height[c] );
        if (m_noPass == 0)
          measureFrameSigmaPass1( &(m_u16SigmaStats[c]),  inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_width[c], inp0->m_height[c] );
        else
          measureFrameSigmaPass2( &(m_u16SigmaStats[c]),  inp0->m_ui16Comp[c], inp1->m_ui16Comp[c], inp0->m_width[c], inp0->m_height[c] );
      }
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}
void DistortionMetricSigmaCompare::computeMetric (Frame* inp0, Frame* inp1, int component)
{
  // it is assumed here that the frames are of the same type
  if (inp0->equalType(inp1)) {
    if (inp0->m_isFloat == TRUE) {    // floating point data
    }
    else if (inp0->m_bitDepth == 8) {   // 8 bit data
    }
    else { // 16 bit data
    }
  }
  else {
    printf("Frames of different type being compared. Computation will not be performed for this frame.\n");
  }
}

void DistortionMetricSigmaCompare::reportMetric  ()
{
  printf("%7.3f ", m_metric[Y_COMP]);
}

void DistortionMetricSigmaCompare::reportSummary  ()
{
  if (m_noPass == 0) {
    calcPass1Stats();
    printPass1Stats();
    m_noPass = 1;
  }
  else {
    printPass2Stats();
    //printf("%7.3f ", m_metricStats[Y_COMP].getAverage());
  }
}

void DistortionMetricSigmaCompare::reportMinimum  ()
{
  printf("%7.3f ", m_metricStats[Y_COMP].minimum);
}

void DistortionMetricSigmaCompare::reportMaximum  ()
{
  printf("%7.3f ", m_metricStats[Y_COMP].maximum);
}

void DistortionMetricSigmaCompare::printHeader()
{
  if (m_isWindow == FALSE ) {
    printf("Sigma   "); // 8
  }
  else {
    printf("wSigma  "); // 8
  }
}

void DistortionMetricSigmaCompare::printSeparator(){
  printf("--------");
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
