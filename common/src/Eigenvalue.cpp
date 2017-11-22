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

#include  "Global.H"
# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <cmath>
# include <ctime>
# include <cstring>
# include <vector>

# include "Eigenvalue.H"



//****************************************************************************80


//****************************************************************************80
//
//  Purpose:
//
//    jacobiEigenvalue carries out the Jacobi eigenvalue iteration.
//
//  Discussion:
//
//    This function computes the eigenvalues and eigenvectors of a
//    real symmetric matrix, using Rutishauser's modfications of the classical
//    Jacobi rotation method with threshold pivoting. 
//
//  Licensing:
//
//    The original code was distributed under the GNU LGPL license. 
//    Thad code can be found here:
//    https://people.sc.fsu.edu/~jburkardt/cpp_src/jacobiEigenvalue/jacobiEigenvalue.html
//  
//  Modified:
//
//    05 April 2016
//
//  Author:
//
//    C++ version by John Burkardt
//
//  Parameters:
//
//    Input, int N, the order of the matrix.
//
//    Input, double A[N*N], the matrix, which must be square, real,
//    and symmetric.
//
//    Input, int IT_MAX, the maximum number of iterations.
//
//    Output, double V[N*N], the matrix of eigenvectors.
//
//    Output, double D[N], the eigenvalues, in descending order.
//
//    Output, int &IT_NUM, the total number of iterations.
//
//    Output, int &ROT_NUM, the total number of rotations.
//
//****************************************************************************80
void Eigenvalue::jacobiEigenvalue ( int n, double*  a, int itMax, double*  v, double*  d, int &itNum, int &rotNum )
{
  vector<double> bw(n);
  vector<double> zw(n);
  double c, g, h;
  double gapq;
  int i, j, p, q;
  double s, t, tau;
  double term, termp, termq;
  double theta, thresh;
  
  r8MatIdentity ( n, v );
  r8MatDiagGetVector ( n, a, d );
  
  
  for ( i = 0; i < n; i++ )	{
    bw[i] = d[i];
    zw[i] = 0.0;
  }
  itNum = 0;
  rotNum = 0;
  
  while ( itNum < itMax )	{
    itNum++;
    //
    //  The convergence threshold is based on the size of the elements in
    //  the strict upper triangle of the matrix.
    //
    thresh = 0.0;
    for ( j = 0; j < n; j++ )		{
      for ( i = 0; i < j; i++ )			{
        thresh += a[i + j * n] * a[i + j * n];
      }
    }
    
    thresh = sqrt ( thresh ) / ( double ) ( 4 * n );
    
    if ( thresh == 0.0 )		{
      break;
    }
    
    for ( p = 0; p < n; p++ )		{
      for ( q = p + 1; q < n; q++ )			{
        gapq = 10.0 * dAbs ( a[p + q * n] );
        termp = gapq + dAbs ( d[p] );
        termq = gapq + dAbs ( d[q] );
        //
        //  Annihilate tiny offdiagonal elements.
        //
        if ( 4 < itNum &&	termp == dAbs ( d[p] ) &&	termq == dAbs ( d[q] ) )	{
          a[p + q * n] = 0.0;
        }
        //
        //  Otherwise, apply a rotation.
        //
        else if ( thresh <= dAbs ( a[p + q * n] ) )	{
          h = d[q] - d[p];
          term = dAbs ( h ) + gapq;
          
          if ( term == dAbs ( h ) )	{
            t = a[p + q * n] / h;
          }
          else {
            theta = 0.5 * h / a[p + q * n];
            t = 1.0 / ( dAbs ( theta ) + sqrt ( 1.0 + theta * theta ) );
            if ( theta < 0.0 ) {
              t = - t;
            }
          }
          c = 1.0 / sqrt ( 1.0 + t * t );
          s = t * c;
          tau = s / ( 1.0 + c );
          h = t * a[p + q * n];
          //
          //  Accumulate corrections to diagonal elements.
          //
          zw[p] -= h;                 
          zw[q] += h;
          d[p]  -= h;
          d[q]  += h;
          
          a[p + q * n] = 0.0;
          //
          //  Rotate, using information from the upper triangle of A only.
          //
          for ( j = 0; j < p; j++ )	{
            g = a[j + p * n];
            h = a[j + q * n];
            a[j + p * n] -= s * ( h + g * tau );
            a[j + q * n] += s * ( g - h * tau );
          }
          
          for ( j = p + 1; j < q; j++ )	{
            g = a[p + j * n];
            h = a[j + q * n];
            a[p + j * n] -= s * ( h + g * tau );
            a[j + q * n] += s * ( g - h * tau );
          }
          
          for ( j = q + 1; j < n; j++ )	{
            g = a[p + j * n];
            h = a[q + j * n];
            a[p + j * n] -= s * ( h + g * tau );
            a[q + j * n] += s * ( g - h * tau );
          }
          //
          //  Accumulate information in the eigenvector matrix.
          //
          for ( j = 0; j < n; j++ ) {
            g = v[j + p * n];
            h = v[j + q * n];
            v[j + p * n] -= s * ( h + g * tau );
            v[j + q * n] += s * ( g - h * tau );
          }
          rotNum = rotNum + 1;
        }
      }
    }
    
    for ( i = 0; i < n; i++ ) {
      bw[i] = bw[i] + zw[i];
      d[i] = bw[i];
      zw[i] = 0.0;
    }
  }
  //
  //  Restore upper triangle of input matrix.
  //
  for ( j = 0; j < n; j++ ) {
    for ( i = 0; i < j; i++ ) {
      a[i + j * n] = a[j + i * n];
    }
  }
  //
  //  Ascending sort the eigenvalues and eigenvectors.
  //Sorting is not necessary for VIF computation, so for VIF we disabled the following line of codes
#if 0
		for ( int k = 0; k < n - 1; k++ )	{
      int m = k;
      for ( int l = k + 1; l < n; l++ )	{
        if ( d[l] < d[m] ) {
          m = l;
        }
      }
      
      if ( m != k )	{
        t    = d[m];
        d[m] = d[k];
        d[k] = t;
        for ( i = 0; i < n; i++ )			{
          swap(v[i + m * n], v[i + k * n]);
        }
      }
    }
#endif
  
  return;
}


//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_DIAG_GET_VECTOR gets the value of the diagonal of an R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    15 July 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of rows and columns of the matrix.
//
//    Input, double A[N*N], the N by N matrix.
//
//    Output, double V[N], the diagonal entries
//    of the matrix.
//
//****************************************************************************80
void Eigenvalue::r8MatDiagGetVector ( int n, double* a, double* v)
{
  for ( int i = 0; i < n; i++ )	{
    v[i] = a[i+i*n];
  }
  
  return;
}


//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_IDENTITY sets the square matrix A to the identity.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 December 2011
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the order of A.
//
//    Output, double A[N*N], the N by N identity matrix.
//
//****************************************************************************80
void Eigenvalue::r8MatIdentity ( int n, double* a )
{
  int k = 0;
  for ( int j = 0; j < n; j++ )	{
    for ( int i = 0; i < n; i++ )		{
      if ( i == j )			{
        a[k] = 1.0;
      }
      else			{
        a[k] = 0.0;
      }
      k++;
    }
  }
  
  return;
}

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_IS_EIGEN_RIGHT determines the error in a (right) eigensystem.
//
//  Discussion:
//
//    An R8MAT is a matrix of doubles.
//
//    This routine computes the Frobenius norm of
//
//      A * X - X * LAMBDA
//
//    where
//
//      A is an N by N matrix,
//      X is an N by K matrix (each of K columns is an eigenvector)
//      LAMBDA is a K by K diagonal matrix of eigenvalues.
//
//    This routine assumes that A, X and LAMBDA are all real.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    07 October 2010
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the order of the matrix.
//
//    Input, int K, the number of eigenvectors.
//    K is usually 1 or N.
//
//    Input, double A[N*N], the matrix.
//
//    Input, double X[N*K], the K eigenvectors.
//
//    Input, double LAMBDA[K], the K eigenvalues.
//
//    Output, double R8MAT_IS_EIGEN_RIGHT, the Frobenius norm
//    of the difference matrix A * X - X * LAMBDA, which would be exactly zero
//    if X and LAMBDA were exact eigenvectors and eigenvalues of A.
//
//****************************************************************************80
double Eigenvalue::r8MatIsEigenRight ( int n, int k,  double* a, double *x, double *lambda )
{
  vector<double> c(n * k);
  
  
  double errorFrobenius;
  int i;
  int j;
  int l;
  
  
  for ( j = 0; j < k; j++ )	{
    for ( i = 0; i < n; i++ )		{
      c[i + j * n] = 0.0;
      for ( l = 0; l < n; l++ )			{
        c[i + j * n] = c[i + j * n] + a[i+l*n] * x[l+j*n];
      }
    }
  }
  
  for ( j = 0; j < k; j++ )	{
    for ( i = 0; i < n; i++ )		{
      c[i + j * n] = c[i + j * n] - lambda[j] * x[i + j * n];
    }
  }
  
  errorFrobenius = r8MatNormFro ( n, k, &c[0] );
  
  return errorFrobenius;
}


//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_NORM_FRO returns the Frobenius norm of an R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//    The Frobenius norm is defined as
//
//      R8MAT_NORM_FRO = sqrt (
//        sum ( 1 <= I <= M ) sum ( 1 <= j <= N ) A(I,J)^2 )
//    The matrix Frobenius norm is not derived from a vector norm, but
//    is compatible with the vector L2 norm, so that:
//
//      r8vec_norm_l2 ( A * x ) <= r8MatNormFro ( A ) * r8vec_norm_l2 ( x ).
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    10 October 2005
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, the number of rows in A.
//
//    Input, int N, the number of columns in A.
//
//    Input, double A[M*N], the matrix whose Frobenius
//    norm is desired.
//
//    Output, double R8MAT_NORM_FRO, the Frobenius norm of A.
//
//****************************************************************************80
double Eigenvalue::r8MatNormFro ( int m, int n, double* a )
{
  int i;
  int j;
  double value;
  
  value = 0.0;
  for ( j = 0; j < n; j++ )	{
    for ( i = 0; i < m; i++ )		{
      value = value + pow ( a[i + j * m], 2 );
    }
  }
  value = sqrt ( value );
  
  return value;
}

//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_PRINT prints an R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//    Entry A(I,J) is stored as A[I+J*M]
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    10 September 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, the number of rows in A.
//
//    Input, int N, the number of columns in A.
//
//    Input, double A[M*N], the M by N matrix.
//
//    Input, string TITLE, a title.
//
//****************************************************************************80
void Eigenvalue::r8MatPrint ( int m, int n, double* a, std::string title )
{
  //r8MatPrintSome ( m, n, a, 1, 1, m, n, title );
  
  return;
}


//****************************************************************************80
//
//  Purpose:
//
//    R8MAT_PRINT_SOME prints some of an R8MAT.
//
//  Discussion:
//
//    An R8MAT is a doubly dimensioned array of R8 values, stored as a vector
//    in column-major order.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    26 June 2013
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int M, the number of rows of the matrix.
//    M must be positive.
//
//    Input, int N, the number of columns of the matrix.
//    N must be positive.
//
//    Input, double A[M*N], the matrix.
//
//    Input, int ILO, JLO, IHI, JHI, designate the first row and
//    column, and the last row and column to be printed.
//
//    Input, string TITLE, a title.
//
//****************************************************************************80
void Eigenvalue::r8MatPrintSome ( int m, int n, double*a  , int ilo, int jlo, int ihi, int jhi, std::string title )
{
# define INCX 5
  int i;
  int i2hi;
  int i2lo;
  int j;
  int j2hi;
  int j2lo;
  
  cout << "\n";
  cout << title << "\n";
  
  if ( m <= 0 || n <= 0 )	{
    cout << "\n";
    cout << "  (None)\n";
    return;
  }
  //
  //  Print the columns of the matrix, in strips of 5.
  //
  for ( j2lo = jlo; j2lo <= jhi; j2lo = j2lo + INCX )	{
    j2hi = j2lo + INCX - 1;
    if ( n < j2hi )		{
      j2hi = n;
    }
    if ( jhi < j2hi )		{
      j2hi = jhi;
    }
    cout << "\n";
    //
    //  For each column J in the current range...
    //
    //  Write the header.
    //
    cout << "  Col:    ";
    for ( j = j2lo; j <= j2hi; j++ )		{
      cout << setw(7) << j - 1 << "       ";
    }
    cout << "\n";
    cout << "  Row\n";
    cout << "\n";
    //
    //  Determine the range of the rows in this strip.
    //
    if ( 1 < ilo )		{
      i2lo = ilo;
    }
    else		{
      i2lo = 1;
    }
    if ( ihi < m )		{
      i2hi = ihi;
    }
    else		{
      i2hi = m;
    }
    
    for ( i = i2lo; i <= i2hi; i++ )		{
      //
      //  Print out (up to) 5 entries in row I, that lie in the current strip.
      //
      cout << setw(5) << i - 1 << ": ";
      for ( j = j2lo; j <= j2hi; j++ )			{
        cout << setw(12) << a[i - 1 + (j - 1) * m] << "  ";
      }
      cout << "\n";
    }
  }
  
  return;
# undef INCX
}

//****************************************************************************80
//
//  Purpose:
//
//    R8VEC_PRINT prints an R8VEC.
//
//  Discussion:
//
//    An R8VEC is a vector of R8's.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    16 August 2004
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of components of the vector.
//
//    Input, double A[N], the vector to be printed.
//
//    Input, string TITLE, a title.
//
//****************************************************************************80
void Eigenvalue::r8VecPrint ( int n,  double* a, std::string title )
{
  int i;
  
  cout << "\n";
  cout << title << "\n";
  cout << "\n";
  for ( i = 0; i < n; i++ )
  {
    cout << "  " << setw(8)  << i
    << ": " << setw(14) << a[i]  << "\n";
  }
  
  return;
}


//****************************************************************************80
//
//  Purpose:
//
//    TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//
//    31 May 2001 09:45:54 AM
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    08 July 2009
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    None
//
//****************************************************************************80
void Eigenvalue::timestamp ( )
{
# define TIME_SIZE 40
  
  static char time_buffer[TIME_SIZE];
  const struct std::tm *tm_ptr;
  //size_t len;
  std::time_t now;
  
  now = std::time ( NULL );
  tm_ptr = std::localtime ( &now );
  
  //len = std::strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm_ptr );
  std::strftime ( time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm_ptr );
    
  std::cout << time_buffer << "\n";
  
  return;
# undef TIME_SIZE
}

