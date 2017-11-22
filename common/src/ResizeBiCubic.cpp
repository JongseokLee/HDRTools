/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Samsung Electronics
 * <ORGANIZATION> = Samsung Electronics
 * <YEAR> = 2015
 *
 * Copyright (c) 2015, Samsung Electronics
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
 * \file ResizeBiCubic.cpp
 *
 * \brief
 *       BiCubic resizer. This code was refactored from OpenCV
 *
 * \author
 *     - Sarvesh Sahota         <sa.sahota@samsung.com>
 *     - Kulbhushan Pachauri	  <kb.pachauri@samsung.com>
 *
 *       
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include "ResizeBiCubic.H"
#include <string.h>
#include <cassert>

#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
#include <emmintrin.h>
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
#if defined(ENABLE_SSE_OPT)
static int check_sse()
{
  int cpuid_data[4] = { 0, 0, 0, 0 };

#if defined _MSC_VER && (defined _M_IX86 || defined _M_X64)
  __cpuid(cpuid_data, 1);
#elif defined __GNUC__ && (defined __i386__ || defined __x86_64__)
#ifdef __x86_64__
  asm __volatile__
    (
    "movl $1, %%eax\n\t"
    "cpuid\n\t"
    :[eax]"=a"(cpuid_data[0]),[ebx]"=b"(cpuid_data[1]),[ecx]"=c"(cpuid_data[2]),[edx]"=d"(cpuid_data[3])
    :
  : "cc"
    );
#else
  asm volatile
    (
    "pushl %%ebx\n\t"
    "movl $1,%%eax\n\t"
    "cpuid\n\t"
    "popl %%ebx\n\t"
    : "=a"(cpuid_data[0]), "=c"(cpuid_data[2]), "=d"(cpuid_data[3])
    :
  : "cc"
    );
#endif
#endif 
  return ((cpuid_data[3] & (1<<25)) != 0);
}
#endif

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
static inline size_t alignsize(size_t sz, int n)
{
	assert((n & (n - 1)) == 0); // n is a power of 2
	return (sz + n-1) & -n;
}


static inline void interpolateCubic( float x, float* coeffs )
{
  //Changed to match the matlab resize results.
  const double A = -0.50;
  double xDouble = (double) x;
  double xDoubleP1 = xDouble + 1.0;
  double xDoubleM1 = 1.0 - xDouble;

  coeffs[0] = (float) (((A * xDoubleP1 - 5.0 * A) * xDoubleP1 + 8.0 * A) * xDoubleP1 - 4.0 * A);
  coeffs[1] = (float) (((A + 2.0) * xDouble   - (A + 3.0)) * xDouble   * xDouble   + 1.0);
  coeffs[2] = (float) (((A + 2.0) * xDoubleM1 - (A + 3.0)) * xDoubleM1 * xDoubleM1 + 1.0);
  coeffs[3] = (float) (1.0 - (double) coeffs[0] - (double) coeffs[1] - (double) coeffs[2]);
}

int ResizeBiCubic::verticalSSE(const byte** in, byte* out, const byte* _beta, int width )
{
#if defined(ENABLE_SSE_OPT)
  if( check_sse() )  {
    const float ** src = (const float**)in;
    const float * beta = (const float*)_beta;

    const float * S0 = src[0];
    const float * S1 = src[1];
    const float * S2 = src[2];
    const float * S3 = src[3];

    float * dst = (float*)out;

    int x = 0;

    __m128 rb0 = _mm_set1_ps(beta[0]);
    __m128 rb1 = _mm_set1_ps(beta[1]);
    __m128 rb2 = _mm_set1_ps(beta[2]);
    __m128 rb3 = _mm_set1_ps(beta[3]);

    for( ; x <= width - 8; x += 8 )
    {
      __m128 rx0, rx1, ry0, ry1, rs0, rs1;
      rx0 = _mm_loadu_ps(S0 + x);
      rx1 = _mm_loadu_ps(S0 + x + 4);
      ry0 = _mm_loadu_ps(S1 + x);
      ry1 = _mm_loadu_ps(S1 + x + 4);

      rs0 = _mm_mul_ps(rx0, rb0);
      rs1 = _mm_mul_ps(rx1, rb0);
      ry0 = _mm_mul_ps(ry0, rb1);
      ry1 = _mm_mul_ps(ry1, rb1);
      rs0 = _mm_add_ps(rs0, ry0);
      rs1 = _mm_add_ps(rs1, ry1);

      rx0 = _mm_loadu_ps(S2 + x);
      rx1 = _mm_loadu_ps(S2 + x + 4);
      ry0 = _mm_loadu_ps(S3 + x);
      ry1 = _mm_loadu_ps(S3 + x + 4);

      rx0 = _mm_mul_ps(rx0, rb2);
      rx1 = _mm_mul_ps(rx1, rb2);
      ry0 = _mm_mul_ps(ry0, rb3);
      ry1 = _mm_mul_ps(ry1, rb3);
      rs0 = _mm_add_ps(rs0, rx0);
      rs1 = _mm_add_ps(rs1, rx1);
      rs0 = _mm_add_ps(rs0, ry0);
      rs1 = _mm_add_ps(rs1, ry1);

      _mm_storeu_ps( dst + x, rs0);
      _mm_storeu_ps( dst + x + 4, rs1);
    }

    return x;
  }
  else
#endif
    return 0;
}

void ResizeBiCubic::horizontal(const float** src, float** dst, int count, const int* xofs, const float* alpha, int swidth, int dwidth, int cn, int xMin, int xMax ) 
{
  for( int k = 0; k < count; k++ )
  {
    const float * S = src[k];
    float * D = dst[k];
    int dx = 0, limit = xMin;
    for(;;) {
      for( ; dx < limit; dx++, alpha += 4 ) {
        int j, sx = xofs[dx] - cn;
        double v = 0.0;
        for( j = 0; j < 4; j++ ) {
          int sxj = sx + j*cn;
          if( (unsigned) sxj >= (unsigned)swidth ) {
            while( sxj < 0 )
              sxj += cn;
            while( sxj >= swidth )
              sxj -= cn;
					}
					v += (double) S[sxj] * (double) alpha[j];
				}
				D[dx] = (float) v;
			}
			
      if( limit == dwidth )
				break;
        
			for( ; dx < xMax; dx++, alpha += 4 ) {
				int sx = xofs[dx];
				D[dx] = (float) ((double) S[sx - cn] * (double) alpha[0] + (double) S[sx         ] * (double) alpha[1] +
					               (double) S[sx + cn] * (double) alpha[2] + (double) S[sx + cn * 2] * (double) alpha[3]);
			}
			limit = dwidth;
		}
		alpha -= dwidth * 4;
	}
}

void ResizeBiCubic::vertical(const float** src, float* dst, const float* beta, int width )
{
	double b0 = (double) beta[0];
	double b1 = (double) beta[1];
	double b2 = (double) beta[2];
	double b3 = (double) beta[3];

	const float * S0 = src[0];
	const float * S1 = src[1];
	const float * S2 = src[2];
	const float * S3 = src[3];

	int x = verticalSSE((const byte**)src, (byte*)dst, (const byte*)beta, width);
	for( ; x < width; x++ )
		dst[x] = (float) ((double) S0[x] * b0 + (double) S1[x] * b1 + (double) S2[x] * b2 + (double) S3[x] * b3);
}


void ResizeBiCubic::resize( float * src, const int _srcW, const int srcH, float * dst, const int _dstW, const int dstH, 
	                               const int cn, const int* xofs, const void* _alpha, const int* yofs, 
                                 const void* _beta, int xMin, int xMax, int kSize)
{
	const float* beta = (const float*)_beta;
	int srcW = _srcW * cn;
	int dstW = _dstW * cn;
	int dy;
	
	int bufstep = (int) alignsize(dstW, 16);

  vector<float> buffer(bufstep * kSize);

	const float * alpha = (const float *) _alpha;
	const float* srows[MAX_ESIZE] = {0};
	float* rows[MAX_ESIZE] = {0};
	int prevSy[MAX_ESIZE];

	int start = 0;
	int end = dstH;
	int src_s = srcW;
	int dst_s = dstW;

	xMin *= cn;
	xMax *= cn;

	for(int k = 0; k < kSize; k++ )	{
		prevSy[k] = -1;
		rows[k] = (float*) &buffer[bufstep * k];
	}

	beta += kSize * start;

	for( dy = start; dy < end; dy++, beta += kSize )	{
		int sy0 = yofs[dy], k0 = kSize, k1 = 0, kSize2 = (kSize >> 1);

		for(int k = 0; k < kSize; k++ )		{
			int sy = iClip(sy0 - kSize2 + 1 + k, 0, srcH - 1);
			for( k1 = iMax(k1, k); k1 < kSize; k1++ ) {
				if( sy == prevSy[k1] ) {
					if( k1 > k )
						memcpy( rows[k], rows[k1], bufstep * sizeof(rows[0][0]) );
					break;
				}
			}
			if( k1 == kSize )
				k0 = iMin(k0, k); 
			srows[k] = (float*)(src + src_s * sy);
			prevSy[k] = sy;
		}

		if( k0 < kSize )
			horizontal( (const float**)(srows + k0), (float**)(rows + k0), kSize - k0, xofs, (const float*)(alpha), srcW, dstW, cn, xMin, xMax );
	
    vertical( (const float**)rows, (float*)(dst + dst_s*dy), beta, dstW );
	}
}

void ResizeBiCubic::compute( float * _src, const int srcW, const int srcH, float * _dst, 
	const int dstW, const int dstH, const int cn )
{
	if( srcW <= 0 || srcH <= 0 )	{
		printf("Invalid source width/height %d %d\n", srcW, srcH);
		return;
	}
	if( dstW <= 0 || dstH <= 0)	{
		printf("Invalid destination width/height %d %d\n", dstW, dstH);
		return;
	}

	double invScaleX = (double) dstW / (double) srcW;
	double invScaleY = (double) dstH / (double) srcH;

	double scaleX = 1.0 / invScaleX;
  double scaleY = 1.0 / invScaleY;
	int    k, sx, sy, dx, dy;

	int    xMin = 0, xMax = dstW, width = dstW*cn;
	float  fx, fy;

	int kSize  = 4;
	int kSize2 = kSize >> 1;

  vector<byte> buffer(((width + dstH)*(sizeof(int) + sizeof(float) * kSize)));

	int*   xofs  = (int*) &buffer[0];
	int*   yofs  = xofs + width;
	float* alpha = (float*)(yofs + dstH);
	float* beta  = alpha + width * kSize;
	float  cbuf[MAX_ESIZE];

	for( dx = 0; dx < dstW; dx++ )	{
		fx = (float)(((double) dx + 0.5) * scaleX - 0.5);
		sx = (int) floor(fx);
		fx -= sx;

		if( sx < kSize2 - 1 ) {
			xMin = dx + 1;
			if( sx < 0 ) {
				fx = 0.0f;
        sx = 0;
      }
		}

		if( sx + kSize2 >= srcW ) {
			xMax = iMin( xMax, dx );
			if( sx >= srcW-1 ) {
				fx = 0.0f; 
        sx = srcW - 1;
      }
		}

		for( k = 0, sx *= cn; k < cn; k++ )
			xofs[dx * cn + k] = sx + k;

		interpolateCubic( fx, cbuf );

		for( k = 0; k < kSize; k++ )
			alpha[dx * cn * kSize + k] = cbuf[k];
		for( ; k < cn * kSize; k++ )
			alpha[dx * cn * kSize + k] = alpha[dx * cn * kSize + k - kSize];
	}

	for( dy = 0; dy < dstH; dy++ ) {
		fy = (float)((dy + 0.5) * scaleY - 0.5);
		sy = (int) floor(fy);
		fy -= sy;

		yofs[dy] = sy;

		interpolateCubic( fy, cbuf );

		for( k = 0; k < kSize; k++ )
			beta[dy*kSize + k] = cbuf[k];
	}

	resize( _src, srcW, srcH, _dst, dstW, dstH, cn, xofs, (void*) alpha, yofs, (void*) beta, xMin, xMax, kSize );
}

