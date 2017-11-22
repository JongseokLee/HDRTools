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


/* 
 *************************************************************************************
 * \file FFT.cpp
 *
 * \brief
 *    Basic FFT class Source code
 *    Code was ported from Paul Bourke's (paul.bourke@gmail.com) original code 
 *    found at this link:
 *    http://paulbourke.net/miscellaneous/dft/
 *
 *    The current FFT is a radix-2 implementation, i.e. it only works with data
 *    that have a power of 2 points. We will try to extend this in the future,
 *    likely using the chirp-z transform method. A discussion for this can be
 *    found here:
 *    http://math.stackexchange.com/questions/77118/non-power-of-2-ffts
 *   
 *    This code would do for now in our opinion.
 *
 * \author
 *     - Alexis Michael Tourapis <atourapis@apple.com>
 *       
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include <cassert>
#include "FFT.H"

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
/*!
 ************************************************************************
 *
 * \name
 *      powerOf2
 * \brief
 *      Computes m as the closest but lower power of two of a number
 * \return
 *      0, if twopm = 2**m != n=n
 *      1, otherwise
 ************************************************************************
 */
int FFT::powerOf2(int n, int *m, int *twopm)
{
  if (n <= 1) {
    *m = 0;
    *twopm = 1;
    return 0;
  }

  *m = 1;
  *twopm = 2;
  do {
    (*m)++;
    (*twopm) *= 2;
  } while (2*(*twopm) <= n);

  if (*twopm != n)
    return 0;
  else
    return 1;
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
/*!
 ************************************************************************
 *
 * \name
 *      compute
 * \brief
 *      This function computes an in-place complex-to-complex FFT (1D)
 *      x and y are the real and imaginary arrays of 2^m points.
 *      Note that we have removed the normalization from this function
 *      since that was not computed in other implementations (e.g. fftw)
 *      dir =  1 results in the forward transform
 *      dir = -1 results in the inverse transform
 *
 *   Formula: forward
 *                 N-1
 *                 ---
 *                 \          - j k 2 pi n / N
 *     X(n) =       >   x(k) e                    = forward transform
 *                 /                                n=0..N-1
 *                 ---
 *                 k=0
 *
 *    Formula: inverse
 *                 N-1
 *                 ---
 *                 \          j k 2 pi n / N
 *     X(n) =       >   x(k) e                    = inverse transform
 *                 /                                n=0..N-1
 *                 ---
 *                 k=0
 *
 * \return
 *      0, if computation failed
 *      1, if computation was succesful
 ************************************************************************
 */
int FFT::compute(int dir, int m, double *x, double *y)
{
  long nn,i,i1,j = 0, k, i2, l, l1, l2 = 1;
  double c1 = -1.0, c2 = 0.0, t1,t2,u1,u2,z;

  /* Calculate the number of points */
  nn = ((long) 1 << m);
  /* Do the bit reversal */
  i2 = nn >> 1;
  for (i = 0; i < nn - 1; i++) {
    if (i < j) {
      swap(&x[i], &x[j]);
      swap(&y[i], &y[j]);
    }
    k = i2;
    while (k <= j) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }

  /* Compute the FFT */
  for (l = 0; l < m; l++) {
    l1 = l2;
    l2 <<= 1;
    u1 = 1.0;
    u2 = 0.0;
    for (j = 0; j < l1; j++) {
      for (i = j; i < nn; i += l2) {
        i1 = i + l1;
        t1 = u1 * x[i1] - u2 * y[i1];
        t2 = u1 * y[i1] + u2 * x[i1];
        x[i1] = x[i] - t1;
        y[i1] = y[i] - t2;
        x[i] += t1;
        y[i] += t2;
      }
      z =  u1 * c1 - u2 * c2;
      u2 = u1 * c2 + u2 * c1;
      u1 = z;
    }
    c2 = sqrt((1.0 - c1) / 2.0);
    if (dir == 1)
      c2 = -c2;
    c1 = sqrt((1.0 + c1) / 2.0);
  }

  // For this implementation, we do not need the scaling and we can remove this code
  /* Scaling for forward transform */
  /*
  if (dir == 1) {
  for (i = 0; i < nn;i++) {
  x[i] /= (double) nn;
  y[i] /= (double) nn;
  }
  }
  */

  return 1;
}


/*!
 ************************************************************************
 *
 * \name
 *      compute2D
 * \brief
 *      Perform a 2D FFT inplace given a complex array of size (width * height)
 *      This array has been folded into 1D using height chunks of stride size.
 *      For simplicity, we handle non power of 2 resolutions with zero-padding.
 *      This is obviously not the right thing to do, and it is not recommended. 
 *      We will update this in the future to use the chirp-z transform method
 *      or some other technique. Done only to avoid code crashing.
 *      dir =  1 results in the forward transform
 *      dir = -1 results in the inverse transform
 * \return
 *      0 if the dimensions are not powers of 2
 ************************************************************************
 */
int FFT::compute2D(Complex *c, int height, int width, int dir, int stride)
{
  assert(stride >= width); // make sure that the stride is larger or equal than width
  if (height == 0 || width == 0)
    return 0;

  int i, j;
  int m, twopm;
  vector<double> real;
  vector<double> imag;
  Complex *pCurr;

  // Transform the rows
  if (!powerOf2(height, &m, &twopm)) {
    // return 0;
    m++;
    twopm = 1 << m;
  }

  real.resize(twopm);
  imag.resize(twopm);

  for (j = 0; j < width; j++) {
    pCurr = &c[j];
    for (i = 0; i < height; i++) {
      real[i] = (double) pCurr[i * stride].real;
      imag[i] = (double) pCurr[i * stride].imag;
    }
    // Here the padding happens
    for (i = height; i < twopm; i++) {
      real[i] = 0.0;
      imag[i] = 0.0;
    }
    compute(dir, m, &real[0], &imag[0]);
    for (i = 0; i < height; i++) {
      pCurr[i * stride].real = (float) real[i];
      pCurr[i * stride].imag = (float) imag[i];
    }
  }

  // Transform the columns
  if (!powerOf2(width, &m, &twopm)) {
    //return 0;
    m++;
    twopm = 1 << m;
  }

  real.resize(twopm);
  imag.resize(twopm);

  for (i = 0; i < height; i++) {
    pCurr = &c[i * stride];

    for (j = 0; j < width; j++) {
      real[j] = (double) pCurr[j].real;
      imag[j] = (double) pCurr[j].imag;
    }
    // Pad with zeros
    for (j = width; j < twopm; j++) {
      real[j] = 0.0;
      imag[j] = 0.0;
    }

    compute(dir, m, &real[0], &imag[0]);
    for (j = 0; j < width; j++) {
      pCurr[j].real = (float) real[j];
      pCurr[j].imag = (float) imag[j];
    }
  }

  return 1;
}

