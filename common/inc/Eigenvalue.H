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


//=============================
// https://people.sc.fsu.edu/~jburkardt/cpp_src/jacobiEigenvalue/jacobiEigenvalue.html


#ifndef __Eigenvalue_H__
#define __Eigenvalue_H__

#include "Global.H"
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h> 


class Eigenvalue {
private:
  void   r8MatDiagGetVector ( int n, double* a,  double* v);
  void   r8MatIdentity      ( int n, double* a  );
  double r8MatNormFro       ( int m, int n, double* a );
  // not used and could be removed
  double r8MatIsEigenRight  ( int n, int k, double* a, double* x, double* lambda);
  void   r8VecPrint         ( int n,  double* a, std::string title );
  void   r8MatPrint         ( int m, int n,  double* a, std::string title ); 
  void   r8MatPrintSome     ( int m, int n,  double* a, int ilo, int jlo, int ihi,   int jhi, std::string title );  
  void   timestamp          ( );

public:
  
  // Construct/Deconstruct
  virtual ~Eigenvalue() {}; 
  
  void jacobiEigenvalue ( int n, double* a, int itMax,double* v, 
                                 double*  d, int &itNum, int &rotNum );
  
};
#endif