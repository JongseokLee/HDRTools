/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = DML UBC
 * <ORGANIZATION> = DML UBC
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, DML UBC
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
 * \file DistortionMetricVIF.h
 *
 * \brief
 *    DistortionMetricVIF header/class (main) definition for computing VIF distortion in HDR files
 * 
 *    This is a C++ implementation of the Visual Information Fidelity (VIF) measure.
 *    This implementation was prepared by the Digital Multimedia Lab (DML), 
 *    at the University of British Columbia, BC, Canada
 *    The original code in Matlab is available at:
 *          http://live.ece.utexas.edu/research/quality/VIF.htm*
 * \authors
 *     - Hamid Reza Tohidypour        <htohidyp@ece.ubc.ca>
 *     - Maryam Azimi                 <maryama@ece.ubc.ca>
 *     - Mahsa T. Pourazad            <mahsa.pourazad@gmail.com>
 *     - Panos Nasiopoulos            <panos.p.n@ieee.org>
 *************************************************************************************
 */

#ifndef __DistortionMetricVIF_H__
#define __DistortionMetricVIF_H__

#include "Global.H"
#include "Frame.H"
#include "DistortionMetric.H"
#include "TransferFunction.H"
#include <vector>
//#define FILE_REF_IN_NITS_DOUBLE     1.0


//The current version of VIF supports sp5filters and sp1filters. For more information please see http://live.ece.utexas.edu/research/quality/VIF.htm
//A macro is added for each of the set of filters.

#define spFilters5 1 //if 1 sp5Filters 5 are utilized otherwise !spFilters1 will be utilized
#define spFilters1 0


//spFilters 1
#if spFilters1
static const double lo0filt[9][9] = 
{
  { -8.701000e-05, -1.354280e-03, -1.601260e-03, -5.033700e-04,  2.524010e-03, -5.033700e-04, -1.601260e-03, -1.354280e-03, -8.701000e-05 },
  { -1.354280e-03,  2.921580e-03,  7.522720e-03,  8.224420e-03,  1.107620e-03,  8.224420e-03,  7.522720e-03,  2.921580e-03, -1.354280e-03 },
  { -1.601260e-03,  7.522720e-03, -7.061290e-03, -3.769487e-02, -3.297137e-02, -3.769487e-02, -7.061290e-03,  7.522720e-03, -1.601260e-03 },  
  { -5.033700e-04,  8.224420e-03, -3.769487e-02,  4.381320e-02,  1.811603e-01,  4.381320e-02, -3.769487e-02,  8.224420e-03, -5.033700e-04 },
  {  2.524010e-03,  1.107620e-03, -3.297137e-02,  1.811603e-01,  4.376250e-01,  1.811603e-01, -3.297137e-02,  1.107620e-03,  2.524010e-03 },
  { -5.033700e-04,  8.224420e-03, -3.769487e-02,  4.381320e-02,  1.811603e-01,  4.381320e-02, -3.769487e-02,  8.224420e-03, -5.033700e-04 },
  { -1.601260e-03,  7.522720e-03, -7.061290e-03, -3.769487e-02, -3.297137e-02, -3.769487e-02, -7.061290e-03,  7.522720e-03, -1.601260e-03 },
  { -1.354280e-03,  2.921580e-03,  7.522720e-03,  8.224420e-03,  1.107620e-03,  8.224420e-03,  7.522720e-03,  2.921580e-03, -1.354280e-03 },
  { -8.701000e-05, -1.354280e-03, -1.601260e-03, -5.033700e-04,  2.524010e-03, -5.033700e-04, -1.601260e-03, -1.354280e-03, -8.701000e-05 }
};

static const double lofilt[17][17] =
{
  { -4.350000e-05,  1.207800e-04, -6.771400e-04, -1.243400e-04, -8.006400e-04, -1.597040e-03, -2.516800e-04, -4.202000e-04,  1.262000e-03, -4.202000e-04, -2.516800e-04, -1.597040e-03, -8.006400e-04, -1.243400e-04, -6.771400e-04,  1.207800e-04, -4.350000e-05  },
  {  1.207800e-04,  4.460600e-04, -5.814600e-04,  5.621600e-04, -1.368800e-04,  2.325540e-03,  2.889860e-03,  4.287280e-03,  5.589400e-03,  4.287280e-03,  2.889860e-03,  2.325540e-03, -1.368800e-04,  5.621600e-04, -5.814600e-04,  4.460600e-04,  1.207800e-04  },
  { -6.771400e-04, -5.814600e-04,  1.460780e-03,  2.160540e-03,  3.761360e-03,  3.080980e-03,  4.112200e-03,  2.221220e-03,  5.538200e-04,  2.221220e-03,  4.112200e-03,  3.080980e-03,  3.761360e-03,  2.160540e-03,  1.460780e-03, -5.814600e-04, -6.771400e-04  },
  { -1.243400e-04,  5.621600e-04,  2.160540e-03,  3.175780e-03,  3.184680e-03, -1.777480e-03, -7.431700e-03, -9.056920e-03, -9.637220e-03, -9.056920e-03, -7.431700e-03, -1.777480e-03,  3.184680e-03,  3.175780e-03,  2.160540e-03,  5.621600e-04, -1.243400e-04  },
  { -8.006400e-04, -1.368800e-04,  3.761360e-03,  3.184680e-03, -3.530640e-03, -1.260420e-02, -1.884744e-02, -1.750818e-02, -1.648568e-02, -1.750818e-02, -1.884744e-02, -1.260420e-02, -3.530640e-03,  3.184680e-03,  3.761360e-03, -1.368800e-04, -8.006400e-04  },
  { -1.597040e-03,  2.325540e-03,  3.080980e-03, -1.777480e-03, -1.260420e-02, -2.022938e-02, -1.109170e-02,  3.955660e-03,  1.438512e-02,  3.955660e-03, -1.109170e-02, -2.022938e-02, -1.260420e-02, -1.777480e-03,  3.080980e-03,  2.325540e-03, -1.597040e-03  },
  { -2.516800e-04,  2.889860e-03,  4.112200e-03, -7.431700e-03, -1.884744e-02, -1.109170e-02,  2.190660e-02,  6.806584e-02,  9.058014e-02,  6.806584e-02,  2.190660e-02, -1.109170e-02, -1.884744e-02, -7.431700e-03,  4.112200e-03,  2.889860e-03, -2.516800e-04  },
  { -4.202000e-04,  4.287280e-03,  2.221220e-03, -9.056920e-03, -1.750818e-02,  3.955660e-03,  6.806584e-02,  1.445500e-01,  1.773651e-01,  1.445500e-01,  6.806584e-02,  3.955660e-03, -1.750818e-02, -9.056920e-03,  2.221220e-03,  4.287280e-03, -4.202000e-04  },
  {  1.262000e-03,  5.589400e-03,  5.538200e-04, -9.637220e-03, -1.648568e-02,  1.438512e-02,  9.058014e-02,  1.773651e-01,  2.120374e-01,  1.773651e-01,  9.058014e-02,  1.438512e-02, -1.648568e-02, -9.637220e-03,  5.538200e-04,  5.589400e-03,  1.262000e-03  },
  { -4.202000e-04,  4.287280e-03,  2.221220e-03, -9.056920e-03, -1.750818e-02,  3.955660e-03,  6.806584e-02,  1.445500e-01,  1.773651e-01,  1.445500e-01,  6.806584e-02,  3.955660e-03, -1.750818e-02, -9.056920e-03,  2.221220e-03,  4.287280e-03, -4.202000e-04  },
  { -2.516800e-04,  2.889860e-03,  4.112200e-03, -7.431700e-03, -1.884744e-02, -1.109170e-02,  2.190660e-02,  6.806584e-02,  9.058014e-02,  6.806584e-02,  2.190660e-02, -1.109170e-02, -1.884744e-02, -7.431700e-03,  4.112200e-03,  2.889860e-03, -2.516800e-04  },
  { -1.597040e-03,  2.325540e-03,  3.080980e-03, -1.777480e-03, -1.260420e-02, -2.022938e-02, -1.109170e-02,  3.955660e-03,  1.438512e-02,  3.955660e-03, -1.109170e-02, -2.022938e-02, -1.260420e-02, -1.777480e-03,  3.080980e-03,  2.325540e-03, -1.597040e-03  },
  { -8.006400e-04, -1.368800e-04,  3.761360e-03,  3.184680e-03, -3.530640e-03, -1.260420e-02, -1.884744e-02, -1.750818e-02, -1.648568e-02, -1.750818e-02, -1.884744e-02, -1.260420e-02, -3.530640e-03,  3.184680e-03,  3.761360e-03, -1.368800e-04, -8.006400e-04  },
  { -1.243400e-04,  5.621600e-04,  2.160540e-03,  3.175780e-03,  3.184680e-03, -1.777480e-03, -7.431700e-03, -9.056920e-03, -9.637220e-03, -9.056920e-03, -7.431700e-03, -1.777480e-03,  3.184680e-03,  3.175780e-03,  2.160540e-03,  5.621600e-04, -1.243400e-04  },
  { -6.771400e-04, -5.814600e-04,  1.460780e-03,  2.160540e-03,  3.761360e-03,  3.080980e-03,  4.112200e-03,  2.221220e-03,  5.538200e-04,  2.221220e-03,  4.112200e-03,  3.080980e-03,  3.761360e-03,  2.160540e-03,  1.460780e-03, -5.814600e-04, -6.771400e-04  },
  {  1.207800e-04,  4.460600e-04, -5.814600e-04,  5.621600e-04, -1.368800e-04,  2.325540e-03,  2.889860e-03,  4.287280e-03,  5.589400e-03,  4.287280e-03,  2.889860e-03,  2.325540e-03, -1.368800e-04,  5.621600e-04, -5.814600e-04,  4.460600e-04,  1.207800e-04  },
  { -4.350000e-05,  1.207800e-04, -6.771400e-04, -1.243400e-04, -8.006400e-04, -1.597040e-03, -2.516800e-04, -4.202000e-04,  1.262000e-03, -4.202000e-04, -2.516800e-04, -1.597040e-03  -8.006400e-04, -1.243400e-04, -6.771400e-04,  1.207800e-04, -4.350000e-05  }
  
};
static const double hi0filt[9][9] = 
{
  { -9.570000e-04, -2.424100e-04, -1.424720e-03, -8.742600e-04, -1.166810e-03, -8.742600e-04, -1.424720e-03, -2.424100e-04, -9.570000e-04 },
  { -2.424100e-04, -4.317530e-03,  8.998600e-04,  9.156420e-03,  1.098012e-02,  9.156420e-03,  8.998600e-04, -4.317530e-03, -2.424100e-04 },
  { -1.424720e-03,  8.998600e-04,  1.706347e-02,  1.094866e-02, -5.897780e-03,  1.094866e-02,  1.706347e-02,  8.998600e-04, -1.424720e-03 },
  { -8.742600e-04,  9.156420e-03,  1.094866e-02, -7.841370e-02, -1.562827e-01, -7.841370e-02,  1.094866e-02,  9.156420e-03, -8.742600e-04 },
  { -1.166810e-03,  1.098012e-02, -5.897780e-03, -1.562827e-01,  7.282593e-01, -1.562827e-01, -5.897780e-03,  1.098012e-02, -1.166810e-03 },
  { -8.742600e-04,  9.156420e-03,  1.094866e-02, -7.841370e-02, -1.562827e-01, -7.841370e-02,  1.094866e-02,  9.156420e-03, -8.742600e-04 },
  { -1.424720e-03,  8.998600e-04,  1.706347e-02,  1.094866e-02, -5.897780e-03,  1.094866e-02,  1.706347e-02,  8.998600e-04, -1.424720e-03 },
  { -2.424100e-04, -4.317530e-03,  8.998600e-04,  9.156420e-03,  1.098012e-02,  9.156420e-03,  8.998600e-04, -4.317530e-03, -2.424100e-04 },
  { -9.570000e-04, -2.424100e-04, -1.424720e-03, -8.742600e-04, -1.166810e-03, -8.742600e-04, -1.424720e-03, -2.424100e-04, -9.570000e-04 }
  
};
static const double bfilts [2][81] = 
{
  { -0.00612588,	0.0080526 ,	 0.02103714,   0.0153689 ,	0.01851466,	 0.0153689 ,   0.02103714,	0.0080526 ,	-0.00612588,
    0.01287416,	0.00961152, -0.01023569,  -0.00600945, -0.00187262, -0.00600945,  -0.01023569,	0.00961152,  0.01287416,	
    0.00564153, -0.0041684 ,  0.0238218 ,   0.05375324,	0.02076086,	 0.05375324,   0.0238218 , -0.0041684 ,  0.00564153,	  
    0.00895726,  0.00175117,	 0.01836909,  -0.1265655 , -0.2996168 , -0.1265655 ,   0.01836909,  0.00175117,	 0.00895726,
    0         ,	0         ,	 0         ,	0         ,	  0         ,	0         ,	   0         ,	0         ,	 0         ,
    -0.00895726, -0.00175117,	-0.01836909,  0.1265655 ,	0.2996168 ,	 0.1265655 ,   -0.01836909, -0.00175117,   -0.00895726,  -0.00564153,	0.0041684 ,
    -0.0238218 , -0.05375324,	-0.02076086,  -0.05375324, -0.0238218 ,  0.0041684 , -0.00564153,  -0.01287416, -0.00961152,    0.01023569,  0.00600945,    0.00187262,	  0.00600945,	0.01023569,
    -0.00961152, -0.01287416,  0.00612588,  -0.0080526 , -0.02103714, -0.0153689 , -0.01851466,  -0.0153689 , -0.02103714,   -0.0080526 ,	 0.00612588
  },
  { 0.00612588,	-0.01287416,	-0.00564153,	-0.00895726,	0,	0.00895726,	0.00564153,	0.01287416,	-0.00612588,
    -0.0080526 ,	-0.00961152,	0.0041684,	-0.00175117,	
    0,	0.00175117,	-0.0041684,	0.00961152,	0.0080526,	-0.02103714,	0.01023569,	-0.0238218,	-0.01836909,	
    0,	0.01836909,	0.0238218,	-0.01023569,	0.02103714,	-0.0153689,	0.00600945,	-0.05375324,	0.1265655,
    0,	-0.1265655,	0.05375324,	-0.00600945,	0.0153689,	-0.01851466,	0.00187262,	-0.02076086,	0.2996168,
    0,	-0.2996168,	0.02076086,	-0.00187262,	0.01851466,	-0.0153689,	0.00600945,	-0.05375324, 0.1265655,	
    0,	-0.1265655,	0.05375324,	-0.00600945,	0.0153689,	-0.02103714,	0.01023569,	-0.0238218,	-0.01836909,
    0,	0.01836909,	0.0238218,	-0.01023569,	0.02103714,	-0.0080526,	-0.00961152,	0.0041684,	-0.00175117,
    0,	0.00175117,	-0.0041684,	0.00961152,	0.0080526,	0.00612588,	-0.01287416,	-0.00564153,	-0.00895726,
    0,	0.00895726,	0.00564153,	0.01287416,	-0.00612588}
};


#else
#if spFilters5

//SP5filter values
static const double lo0filt[5][5] =
{
  { 0.0034161,-0.015512,-0.038482,-0.015512,0.0034161},
  { -0.015512,0.05587,0.15926,0.05587,-0.015512},
  { -0.038482,0.15926,0.40304,0.15926,-0.038482},
  { -0.015512,0.05587,0.15926,0.05587,-0.015512},
  { 0.0034161,-0.015512,-0.038482,-0.015512,0.0034161},
};
static const double lofilt[9][9] =
{
  { 0.0017081,-0.0048983,-0.0077562,-0.018889,-0.019241,-0.018889,-0.0077562,-0.0048983,0.0017081},
  { -0.0048983,-0.010466,-0.013222,0.008212,0.02006,0.008212,-0.013222,-0.010466,-0.0048983},
  { -0.0077562,-0.013222,0.027935,0.065541,0.079628,0.065541,0.027935,-0.013222,-0.0077562},
  { -0.018889,0.008212,0.065541,0.12853,0.16339,0.12853,0.065541,0.008212,-0.018889},
  { -0.019241,0.02006,0.079628,0.16339,0.20193,0.16339,0.079628,0.02006,-0.019241},
  { -0.018889,0.008212,0.065541,0.12853,0.16339,0.12853,0.065541,0.008212,-0.018889},
  { -0.0077562,-0.013222,0.027935,0.065541,0.079628,0.065541,0.027935,-0.013222,-0.0077562},
  { -0.0048983,-0.010466,-0.013222,0.008212,0.02006,0.008212,-0.013222,-0.010466,-0.0048983},
  { 0.0017081,-0.0048983,-0.0077562,-0.018889,-0.019241,-0.018889,-0.0077562,-0.0048983,0.0017081},
};
static const double hi0filt[9][9] =
{
  { -0.00033429,-0.0011309,-0.0017148,-0.0013354,-0.00080639,-0.0013354,-0.0017148,-0.0011309,-0.00033429},
  { -0.0011309,-0.0035002,-0.0024381,0.0063165,0.012612,0.0063165,-0.0024381,-0.0035002,-0.0011309},
  { -0.0017148,-0.0024381,-0.0029008,-0.0067348,-0.0098105,-0.0067348,-0.0029008,-0.0024381,-0.0017148},
  { -0.0013354,0.0063165,-0.0067348,-0.070277,-0.11436,-0.070277,-0.0067348,0.0063165,-0.0013354},
  { -0.00080639,0.012612,-0.0098105,-0.11436,0.8138,-0.11436,-0.0098105,0.012612,-0.00080639},
  { -0.0013354,0.0063165,-0.0067348,-0.070277,-0.11436,-0.070277,-0.0067348,0.0063165,-0.0013354},
  { -0.0017148,-0.0024381,-0.0029008,-0.0067348,-0.0098105,-0.0067348,-0.0029008,-0.0024381,-0.0017148},
  { -0.0011309,-0.0035002,-0.0024381,0.0063165,0.012612,0.0063165,-0.0024381,-0.0035002,-0.0011309},
  { -0.00033429,-0.0011309,-0.0017148,-0.0013354,-0.00080639,-0.0013354,-0.0017148,-0.0011309,-0.00033429},
};
static const double bfilts[6][49] =
{
  { 0.0027764,0.0049619,0.010267,0.014554,0.010267,0.0049619,0.0027764,-0.009869,-0.0089306,0.011899,0.027552,0.011899,-0.0089306,-0.009869,-0.010219,-0.030754,-0.082264,-0.11732,-0.082264,-0.030754,-0.010219,0,0,0,0,0,0,0,0.010219,0.030754,0.082264,0.11732,0.082264,0.030754,0.010219,0.009869,0.0089306,-0.011899,-0.027552,-0.011899,0.0089306,0.009869,-0.0027764,-0.0049619,-0.010267,-0.014554,-0.010267,-0.0049619,-0.0027764},
  { -0.0034325,-0.0064081,-0.00073141,0.011243,0.0018208,0.0028572,0.01167,-0.0035846,-0.019775,-0.040842,-0.0022822,0.039306,0.011612,0.00128,0.010477,0.014863,-0.048191,-0.12227,-0.053941,0.0085396,-0.0045903,0.0079041,0.044356,0.094542,0,-0.094542,-0.044356,-0.0079041,0.0045903,-0.0085396,0.053941,0.12227,0.048191,-0.014863,-0.010477,-0.00128,-0.011612,-0.039306,0.0022822,0.040842,0.019775,0.0035846,-0.01167,-0.0028572,-0.0018208,-0.011243,0.00073141,0.0064081,0.0034325},
  { 0.0034325,0.0035846,-0.010477,-0.0079041,-0.0045903,0.00128,0.01167,0.0064081,0.019775,-0.014863,-0.044356,0.0085396,0.011612,0.0028572,0.00073141,0.040842,0.048191,-0.094542,-0.053941,0.039306,0.0018208,-0.011243,0.0022822,0.12227,0,-0.12227,-0.0022822,0.011243,-0.0018208,-0.039306,0.053941,0.094542,-0.048191,-0.040842,-0.00073141,-0.0028572,-0.011612,-0.0085396,0.044356,0.014863,-0.019775,-0.0064081,-0.01167,-0.00128,0.0045903,0.0079041,0.010477,-0.0035846,-0.0034325},
  { -0.0027764,0.009869,0.010219,0,-0.010219,-0.009869,0.0027764,-0.0049619,0.0089306,0.030754,0,-0.030754,-0.0089306,0.0049619,-0.010267,-0.011899,0.082264,0,-0.082264,0.011899,0.010267,-0.014554,-0.027552,0.11732,0,-0.11732,0.027552,0.014554,-0.010267,-0.011899,0.082264,0,-0.082264,0.011899,0.010267,-0.0049619,0.0089306,0.030754,0,-0.030754,-0.0089306,0.0049619,-0.0027764,0.009869,0.010219,0,-0.010219,-0.009869,0.0027764},
  { -0.01167,-0.00128,0.0045903,0.0079041,0.010477,-0.0035846,-0.0034325,-0.0028572,-0.011612,-0.0085396,0.044356,0.014863,-0.019775,-0.0064081,-0.0018208,-0.039306,0.053941,0.094542,-0.048191,-0.040842,-0.00073141,-0.011243,0.0022822,0.12227,0,-0.12227,-0.0022822,0.011243,0.00073141,0.040842,0.048191,-0.094542,-0.053941,0.039306,0.0018208,0.0064081,0.019775,-0.014863,-0.044356,0.0085396,0.011612,0.0028572,0.0034325,0.0035846,-0.010477,-0.0079041,-0.0045903,0.00128,0.01167},
  { -0.01167,-0.0028572,-0.0018208,-0.011243,0.00073141,0.0064081,0.0034325,-0.00128,-0.011612,-0.039306,0.0022822,0.040842,0.019775,0.0035846,0.0045903,-0.0085396,0.053941,0.12227,0.048191,-0.014863,-0.010477,0.0079041,0.044356,0.094542,0,-0.094542,-0.044356,-0.0079041,0.010477,0.014863,-0.048191,-0.12227,-0.053941,0.0085396,-0.0045903,-0.0035846,-0.019775,-0.040842,-0.0022822,0.039306,0.011612,0.00128,-0.0034325,-0.0064081,-0.00073141,0.011243,0.0018208,0.0028572,0.01167}  
};


#endif
#endif

static const double harmonics = {1};
static const double mtx[2][2] = 
{
  {1, 0},
  {0, 1}
};

class DistortionMetricVIF : public DistortionMetric {
private:
  int               m_blockLength;
  int               m_blockSize;
  int               m_vifYBitDepth; 
  double            m_vifFrame; 
  MetricStatistics  m_vifFrameStats;
  
  vector<double>    m_hi0;
  vector<double>    m_lo0Img0;
  vector<double>    m_lo0Img1;
  vector<double>    m_temp;
  vector<double>    m_image0; 
  vector<double>    m_image1;
  
  TransferFunction   *m_transferFunction;
  
  int internalReduce(double* image, int x_idim, int y_idim, const double *filt, double *temp, int xFDim, int yFDim, int xStart, int xStep, int xStop, int yStart, int yStep, int yStop, double *result);
  int  reflect1( const double * filt,int xDim,int yDim,int xPos,int yPos,double* result,int rOrE);
  void innerProduct(int XCNR,int YCNR,int xDim, int xFDim, int filtSize, int resPos, double* result, double* temp, double* image);
  void inverseMatrix(vector<vector<double> > &input, int n, vector<vector<double> > &output);

  
  void convertOneRowVar            (const double** input, double * output); 
  void refParamsVecGSM             (vector<vector<double> > &org,  int* subbands, vector<vector<int> > &lenWh, int sizeSubBand, vector<vector<vector<double> > >   &ssArr, vector<vector<double> > &lArr, vector<vector<vector<double> > >   &cuArr);
  void vifSubEstM                  (vector<vector<double> >  &org,  vector<vector<double> >  &dist, int* subbands, vector<vector<int> > &lenWh,int sizeSubBand, vector<vector<vector<double> > >   &gAll, vector<vector<vector<double> > > &VvAll, vector<vector<int> > &lenWhGAll); 
  void multiply                    (double* input1, double* input2, int size, double num,  double* output );
  void difference                  (double* input1, double* input2, int size,   double* output );
  void mean                        (vector<vector<double> >  &input, int w, int h, double* mcu);
  void setCompare                  (double* input, int size, double compVal, double setVal, bool iseq , double* output);
  void addFix                      (double* input, int size, double num, double* output );
  void divFix                      (double* input, int size, double num, double* output );
  void div                         (double* input, double* input2, int size, double* output );
  void setCompareWithOther         (double* compInput, double* setInput, int size, double compVal, bool iseq, double * output );
  void repmat                      (double* input, int w, int h, vector<vector<double> >  &output);
  void differenceVector            (vector<vector<double> > &input1, vector<vector<double> >  &input2, int w, int h, vector<vector<double> >  &output);
  void vectorMultiplication        (vector<vector<double> > &input1, int w1, int h1, vector<vector<double> > &input2, int w2,int h2, vector<vector<double> > &output);
  void vectorMax                   (int* input, int size , int imax);
  void vectorMultiplicationSameSize(vector<vector<double> > &input1, int w1, int h1, vector<vector<double> > &input2, vector<vector<double> >  &output);
  void vectorMultiplicationInverse (vector<vector<double> > &input1, int w1, int h1, vector<vector<double> > &input2, vector<vector<double> > &output);
  void sumSSTempMM                 (vector<vector<double> > &input1, int w1, int h1, vector<vector<double> > &input2, double input3, vector<vector<double> >  &output);
  double sumSumLog2GGSSLambdaDivVV (vector<vector<double> > &g, int w1, int h1, vector<vector<double> >  &ss, double lambda, double eps, vector<vector<double> >  &vv);
  double sumSumLog2SSLambdaDivSigma(vector<vector<double> > &ssg, int w1, int h1,double lambda, double eps);
  void vectorDivisionSameSize      (vector<vector<double> > &input1, int w1, int h1, vector<vector<double> > &input2, double input3, vector<vector<double> > &output);
  void vectorMultiplyFixNum        (vector<vector<double> > &input1, int w1, int h1, double input2,  vector<vector<double> > &output);
  void vectorAddFixNum             (vector<vector<double> > &input1, int w1, int h1, double input2,  vector<vector<double> > &output);
  void invertVector                (vector<vector<double> > &input1, int w, int h, vector<vector<double> > &output);
  void reshape                     (vector<vector<double> > &input, int newW, int newH , vector<vector<double> >  &s);
  int maxPyrHt                     (int imsize_w, int imsize_h,int filtsize); 
  int buildSteerablePyramidLevels                (double* lo0, int ht, int w, int h, vector<vector<double> > &pyr, int max_ht,  int pyrElement, vector<vector<double> >  &pyrArr, vector<bool> &subbandUsed); 
  
public:
  // Construct/Deconstruct
  DistortionMetricVIF(const FrameFormat *format, VIFParams *vifParams);
  
  virtual ~DistortionMetricVIF();
  virtual void   computeMetric (Frame* inp0, Frame* inp1); // Compute metric for only one component
  virtual void   computeMetric (Frame* inp0, Frame* inp1, int component);                // Compute metric for all components
  
  
  virtual double rgbPQYuv (double r, double g, double b, int bitdepth,  ColorPrimaries cp);
  
  virtual void   reportMetric  ();                                        // report frame level results
  virtual void   reportSummary ();                                        // report summary results
  virtual void   reportMinimum ();
  virtual void   reportMaximum ();
  virtual void   printHeader   ();
  virtual void   printSeparator();
};


#endif
