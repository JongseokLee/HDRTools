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
 * \file InputTIFF.cpp
 *
 * \brief
 *    InputTIFF class C++ file for allowing input from TIFF files
 *    Original code taken from the JM (io_tiff.c), which was written by
 *    Mr. Larry Luther.
 *    Part of code was based on libtiff (see http://www.libtiff.org/)
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include <string.h>
#include <assert.h>
#include "InputTIFF.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------
//#define __PRINT_INPUT_TIFF__

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

InputTIFF::InputTIFF(IOVideo *videoFile, FrameFormat *format) {
  // We currently only support floating point data
  m_isFloat         = FALSE;
  format->m_isFloat = m_isFloat;
  videoFile->m_format.m_isFloat = m_isFloat;
  m_frameRate       = format->m_frameRate;
  
  m_memoryAllocated = FALSE;
  m_size      = 0;
  m_prevSize  = -1;
  
  m_buf       = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  m_ui16Comp[A_COMP] = NULL;
  
  // Color space is explicitly specified here. We could do a test inside the code though to at least check if
  // RGB is RGB and YUV is YUV, but not important currently
  m_colorSpace       = format->m_colorSpace;
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;
  
}

InputTIFF::~InputTIFF() {
  
  freeMemory();
  
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------
/*!
 ************************************************************************
 * \brief
 *   Get an unsigned short without swapping.
 *
 ************************************************************************
 */
static uint16 getU16 (Tiff * t)
{
  uint16 in;
  
  *(((char *) &in) + 0) = *t->mp++;
  *(((char *) &in) + 1) = *t->mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an unsigned int32 without swapping.
 *
 ************************************************************************
 */
static uint32 getU32 (Tiff * t)
{
  uint32 in;
  
  *(((char *) &in) + 0) = *t->mp++;
  *(((char *) &in) + 1) = *t->mp++;
  *(((char *) &in) + 2) = *t->mp++;
  *(((char *) &in) + 3) = *t->mp++;
  
  return in;
  
}


// Swap versions

/*!
 ************************************************************************
 * \brief
 *   Get an unsigned short and swap.
 *
 ************************************************************************
 */
static uint16 getSwappedU16 (Tiff * t)
{
  uint16 in;
  
  *(((char *) &in) + 1) = *t->mp++;
  *(((char *) &in) + 0) = *t->mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an unsigned int32 and swap.
 *
 ************************************************************************
 */
static uint32 getSwappedU32 (Tiff * t)
{
  uint32 in;
  
  *(((char *) &in) + 3) = *t->mp++;
  *(((char *) &in) + 2) = *t->mp++;
  *(((char *) &in) + 1) = *t->mp++;
  *(((char *) &in) + 0) = *t->mp++;
  
  return in;
}


/*!
 ************************************************************************
 * \brief
 *   Get an array of (uint16|uint32|rational).
 *
 ************************************************************************
 */
void InputTIFF::getIntArray (Tiff * t, uint32 offset, TiffType type, uint32 a[], int n)
{
  int    i;
  uint8  *mp = t->mp;                           // save memory pointer
  
  t->mp = (uint8 *) &t->fileInMemory[offset];
  
  switch (type)  {
    case T_SHORT:
      for (i=0; i < n; ++i) {
        a[i] = t->getU16( t);
      }
      break;
    case T_LONG:
      for (i=0; i < n; ++i) {
        a[i] = t->getU32( t);
      }
      break;
    case T_RATIONAL:
      for (i=0; i < 2*n; ++i)  {
        a[i] = t->getU32( t);
      }
      break;
    default:
      assert( type == T_SHORT || type == T_LONG || type == T_RATIONAL);
  }
  t->mp = mp;                           // restore memory pointer
}


/*!
 ************************************************************************
 * \brief
 *   Read DirectoryEntry and store important results in 't'.
 *
 ************************************************************************
 */
int InputTIFF::readDirectoryEntry (Tiff * t)
{
  uint16   tag    = t->getU16( t);
  TiffType type   = (TiffType) t->getU16( t);
  uint32   count  = t->getU32( t);
  uint32   offset = t->getU32( t);
  
  uint32   offsetData = offset;
  
  if (type == T_SHORT) {
    if (t->getU32 == getU32) {
      offsetData = offset & 0xFFFF;
    }
    else
      offsetData = offset >> 16;
  }
  
  switch (tag)  {
    case 256:                           // ImageWidth  SHORT or LONG
      assert( count == 1);
      t->ImageWidth = offsetData;
#ifdef  __PRINT_INPUT_TIFF__
      printf( "256:  ImageWidth          = %u\n", t->ImageWidth);
#endif
      
      break;
    case 257:                           // ImageLength    SHORT or LONG
      assert( count == 1);
      t->ImageLength = offsetData;
#ifdef  __PRINT_INPUT_TIFF__
      printf( "257:  ImageLength         = %u\n", t->ImageLength);
#endif
      
      if (t->ImageLength  > YRES) {
        fprintf( stderr, "readDirectoryEntry:  ImageLength (%d) exceeds builtin maximum of %d\n", offset, YRES);
        return 1;
      }
      break;
    case 258:                           // BitsPerSample  SHORT 8,8,8
      if (count == 1)
        t->BitsPerSample[0] = t->BitsPerSample[1] = t->BitsPerSample[2] = offsetData;
      else if (count != 3) {
        fprintf( stderr, "BitsPerSample (only [3] supported)\n");
        return 1;
      }
      else {
        getIntArray( t, offset, type, t->BitsPerSample, count);
      }
#ifdef  __PRINT_INPUT_TIFF__
      printf( "258:  BitsPerSample[%d]    = %u,%u,%u\n", count, t->BitsPerSample[0], t->BitsPerSample[1], t->BitsPerSample[2]);
#endif
      if (t->BitsPerSample[0] != t->BitsPerSample[1] || t->BitsPerSample[0] != t->BitsPerSample[2])  {
        fprintf( stderr, "BitsPerSample must be the same for all samples\n");
        return 1;
      }
      if (t->BitsPerSample[0] != 8 && t->BitsPerSample[0] != 16) {
        fprintf( stderr, "Only 8 or 16 BitsPerSample is supported %d\n", t->BitsPerSample[0] );
        return 1;
      }
      break;
    case 259:                           // Compression SHORT 1 or 32773
      assert( count == 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "259:  Compression         = %u\n", offsetData);
#endif
      if (offsetData != 1) {
        fprintf( stderr, "Only uncompressed TIFF files supported. Format %d\n", offsetData );
        return 1;
      }
      break;
    case 262:                           // PhotometricInterpretation SHORT 2
      assert( count == 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "262:  PhotometricInterpretation = %u\n", offsetData);
#endif
      assert( offsetData == 2);
      break;
    case 273:                           // StripOffsets  SHORT or LONG
      if (count == 1)
        t->StripOffsets[0] = offsetData;
      else
        getIntArray( t, offset, type, t->StripOffsets, count);
      t->nStrips = count;
#ifdef  __PRINT_INPUT_TIFF__
      printf( "273:  StripOffsets[%d] = %u\n", count, t->StripOffsets[0]);
#endif
      break;
    case 274:                           // Orientation  SHORT
      assert( count == 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "274:  Orientation         = %u\n", offsetData);
#endif
      t->Orientation = (uint16) offsetData;
      if (t->Orientation != 1) {
        fprintf( stderr, "Only Orientation 1 is supported\n");
        return 1;
      }
      break;
    case 277:                           // SamplesPerPixel SHORT 3 or more
      assert( count == 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "277:  SamplesPerPixel     = %u\n", offsetData);
#endif
      assert( offsetData == 3);
      break;
    case 278:                           // RowsPerStrip  SHORT or LONG
      assert( count == 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "278:  RowsPerStrip        = %u\n", offsetData);
#endif
      t->RowsPerStrip = offsetData;
      break;
    case 279:                           // StripByteCounts LONG or SHORT
#ifdef  __PRINT_INPUT_TIFF__
      printf( "279:  StripByteCounts[%u] = %u %d\n", count, offsetData, type);
#endif
      if (count == 1)
        t->StripByteCounts[0] = offsetData;
      else
        getIntArray( t, offset, type, t->StripByteCounts, count);
      break;
    case 282:                           // XResolution  RATIONAL
      assert( count == 1);
      getIntArray( t, offset, type, t->XResolution, 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "282:  XResolution (%d)        = %u/%u\n", offset, t->XResolution[0], t->XResolution[1]);
#endif
      break;
    case 283:                           // YResolution  RATIONAL
      assert( count == 1);
      getIntArray( t, offset, type, t->YResolution, 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "283:  YResolution (%d)        = %u/%u\n", offset, t->YResolution[0], t->YResolution[1]);
#endif
      break;
    case 284:                           // PlanarConfiguration  SHORT
      assert( count == 1);
#ifdef  __PRINT_INPUT_TIFF__
      printf( "284:  PlanarConfiguration = %u\n", offsetData);
#endif
      assert( offsetData == 1);
      break;
    case 296:                           // ResolutionUnit  SHORT 1, 2 or 3
#ifdef  __PRINT_INPUT_TIFF__
      printf( "296:  ResolutionUnit      = %u\n", offsetData);
#endif
      assert( count == 1);
      break;
    case 305:                           // Software  ASCII
#ifdef  __PRINT_INPUT_TIFF__
      printf( "305:  Software            = %s\n", &t->fileInMemory[offset]);
#endif
      break;
    case 339:                           // SampleFormat  SHORT 1
    default:
#ifdef  __PRINT_INPUT_TIFF__
      printf( "%3d:  Unforseen           = %u\n", tag, offset);
#endif
      break;
  }
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *   Read file named 'path' into memory buffer 't->fileInMemory'.
 *
 * \return
 *   0 if successful
 ************************************************************************
 */
int InputTIFF::readFileIntoMemory (Tiff * t, int *fd)
{
  long cnt, result;
  uint16 byteOrder;
  int endian = 1;
  int machineLittleEndian = (*( (char *)(&endian) ) == 1) ? 1 : 0;
  
  assert( t);
  
  cnt = (long) lseek( *fd, 0, SEEK_END); // TIFF files by definition cannot exceed 2^32
  if (cnt == -1L)
    return 1;
  
  if (lseek( *fd, 0, SEEK_SET) == -1L)   // reposition file at beginning
    return 1;
  
  t->fileInMemory.resize(cnt);
  
  // The following will never happen. So we can comment out this code
  //if (&t->fileInMemory[0] == 0) {
  //  if (*fd != -1) {
  //    close( *fd);
  //    *fd = -1;
  //  }
  //  return 1;
  //}
  
  result = (long) mm_read( *fd, (char *) &t->fileInMemory[0], cnt);
  if (result != cnt) {
    if (*fd != - 1) {
      close( *fd);
      *fd = -1;
    }
    return 1;
  }
  
  byteOrder = (t->fileInMemory[0] << 8) | t->fileInMemory[1];
  switch (byteOrder) {
    case 0x4949:                        // little endian file
      t->le = 1;
      //printf( "Little endian file\n");
      break;
    case 0x4D4D:                        // big endian file
      t->le = 0;
      //printf( "Big endian file\n");
      break;
    default:
      fprintf( stderr, "First two bytes indicate:  Not a TIFF file\n");
      return 1;
  }
  if (t->le == machineLittleEndian)  { // endianness of machine matches file
    t->getU16 = getU16;
    t->getU32 = getU32;
  }
  else {                               // endianness of machine does not match file
    t->getU16 = getSwappedU16;
    t->getU32 = getSwappedU32;
  }
  t->mp = (uint8 *) &t->fileInMemory[0];
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    Read image data into 't->img'.
 *
 ************************************************************************
 */
int InputTIFF::readImageData (Tiff * t)
{
  int     i, j, n;
  uint8  *mp, *s;
  uint16 *p;
  uint32  size;
  
  assert( t);
  
  size = t->ImageWidth * t->ImageLength * 3 * sizeof(uint16);
  
  t->img.resize(size);
    
  switch (t->BitsPerSample[0]) {
    case 8:
      p = (uint16 *) &t->img[0];
      for (i=0; i < t->nStrips; ++i) {
        n = t->StripByteCounts[i];
        s = (uint8 *) &t->fileInMemory[t->StripOffsets[i]];
        for (j=0; j < n; ++j) {
          *p++ = *s++;
        }
      }
      break;
    case 16:
      mp = t->mp;                       // save memory pointer
      p = (uint16 *) &t->img[0];
      for (i=0; i < t->nStrips; ++i) {
        n = t->StripByteCounts[i] / 2;
        t->mp = (uint8 *) &t->fileInMemory[t->StripOffsets[i]];
        for (j=0; j < n; ++j) {
          *p++ = (uint16) t->getU16( t);
        }
      }
      t->mp = mp;                       // restore memory pointer
      break;
  }
  return 0;
}


/*!
 *****************************************************************************
 * \brief
 *    Read the ImageFileDirectory.
 *
 *****************************************************************************
 */
int InputTIFF::readImageFileDirectory (Tiff * t)
{
  uint32 i;
  uint16 nEntries = t->getU16( t);
  
  for (i=0; i < nEntries; ++i) {
    readDirectoryEntry( t);
  }
  return 0;
}


/*!
 *****************************************************************************
 * \brief
 *    Read the ImageFileHeader.
 *
 *****************************************************************************
 */
int InputTIFF::readImageFileHeader (Tiff * t)
{
  t->ifh.byteOrder = (uint16) t->getU16( t);
  t->ifh.arbitraryNumber = (uint16) t->getU16( t);
  t->ifh.offset = t->getU32( t);
  
  if (t->ifh.arbitraryNumber != 42) {
    fprintf( stderr, "ImageFileHeader.arbitrary (%d) != 42\n", t->ifh.arbitraryNumber);
    return 1;
  }
  t->mp = &t->fileInMemory[t->ifh.offset];
  return 0;
}

/*!
 *****************************************************************************
 * \brief
 *    Read the TIFF file named 'path' into 't'.
 *
 *****************************************************************************
 */
int InputTIFF::readTiff (FrameFormat *format, int *fd) {
  assert( &m_tiff);
  
  if (readFileIntoMemory( &m_tiff, fd))
    goto Error;
  
  if (readImageFileHeader( &m_tiff))
    goto Error;
  
  if (readImageFileDirectory( &m_tiff))
    goto Error;
  
  allocateMemory(format);
  
  if (readImageData( &m_tiff))
    goto Error;
  
  return 1;
  
Error:
  return 0;
}


/*!
 ************************************************************************
 * \brief
 *    Open file containing a single frame
 ************************************************************************
 */
int InputTIFF::openFrameFile( IOVideo *inputFile, int FrameNumberInFile)
{
  char infile [FILE_NAME_SIZE], in_number[16];
  int length = 0;
  in_number[length]='\0';
  length = (int) strlen(inputFile->m_fHead);
  strncpy(infile, inputFile->m_fHead, length);
  infile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(inputFile->m_fTail) != 0) {
    if (inputFile->m_zeroPad == TRUE)
      snprintf(in_number, 16, "%0*d", inputFile->m_numDigits, FrameNumberInFile);
    else
      snprintf(in_number, 16, "%*d", inputFile->m_numDigits, FrameNumberInFile);
    
    //strncat(infile, in_number, sizeof(in_number));
    strncat(infile, in_number, strlen(in_number));
    length += sizeof(in_number);
    infile[length]='\0';
    strncat(infile, inputFile->m_fTail, strlen(inputFile->m_fTail));
    length += (int) strlen(inputFile->m_fTail);
    infile[length]='\0';
  }
  
  if ((inputFile->m_fileNum = open(infile, OPENFLAGS_READ)) == -1)  {
    printf ("openFrameFile: cannot open file %s\n", infile);
    //exit(EXIT_FAILURE);
    return -1;
  }
  
  return inputFile->m_fileNum;
}

void InputTIFF::allocateMemory(FrameFormat *format)
{
  format->m_height[Y_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_tiff.ImageLength;
  format->m_width [Y_COMP] = format->m_width [U_COMP] = format->m_width [V_COMP] = m_tiff.ImageWidth;
  
  m_height[Y_COMP] = format->m_height[Y_COMP];
  m_width [Y_COMP] = format->m_width [Y_COMP];
  
  // We are only dealing with 4:4:4 data here
  m_height [V_COMP] = m_height [U_COMP] = m_height [Y_COMP];
  m_width  [V_COMP] = m_width  [U_COMP] = m_width  [Y_COMP];
  
  format->m_compSize[Y_COMP] = m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  format->m_compSize[U_COMP] = m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  format->m_compSize[V_COMP] = m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];
  
  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  
  format->m_height[A_COMP] = m_height[A_COMP];
  format->m_width [A_COMP] = m_width [A_COMP];
  format->m_compSize[A_COMP] = m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
  
  
  // Note that we do not read alpha but discard it
  format->m_size = m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  //format->m_colorSpace   = m_colorSpace   = CM_RGB;
  format->m_chromaFormat = m_chromaFormat = CF_444;
  
  format->m_bitDepthComp[Y_COMP] = m_bitDepthComp[Y_COMP] = m_tiff.BitsPerSample[Y_COMP];
  format->m_bitDepthComp[U_COMP] = m_bitDepthComp[U_COMP] = m_tiff.BitsPerSample[U_COMP];
  format->m_bitDepthComp[V_COMP] = m_bitDepthComp[V_COMP] = m_tiff.BitsPerSample[V_COMP];
  format->m_bitDepthComp[A_COMP] = m_bitDepthComp[A_COMP] = 16;
  
  m_isInterleaved = FALSE;
  m_isInterlaced  = FALSE;
  
  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];    
  }

  
  if (m_memoryAllocated == FALSE || m_size != m_prevSize) {
    if (format->m_bitDepthComp[Y_COMP] == 8) {
      m_data.resize((unsigned int) m_size);
      m_comp[Y_COMP]      = &m_data[0];
      m_comp[U_COMP]      = m_comp[Y_COMP] + m_compSize[Y_COMP];
      m_comp[V_COMP]      = m_comp[U_COMP] + m_compSize[U_COMP];
      
      m_ui16Comp[Y_COMP]  = NULL;
      m_ui16Comp[U_COMP]  = NULL;
      m_ui16Comp[V_COMP]  = NULL;
    }
    else {
      m_comp[Y_COMP]      = NULL;
      m_comp[U_COMP]      = NULL;
      m_comp[V_COMP]      = NULL;

      m_ui16Data.resize((unsigned int) m_size);
      m_ui16Comp[Y_COMP]  = &m_ui16Data[0];
      m_ui16Comp[U_COMP]  = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
      m_ui16Comp[V_COMP]  = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
    }
  }
  
  m_prevSize          = m_size;
  m_ui16Comp[A_COMP]  = NULL;
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  
  m_floatComp[A_COMP] = NULL;

  m_memoryAllocated = TRUE;
}

void InputTIFF::freeMemory()
{
  m_comp[Y_COMP]     = NULL;
  m_comp[U_COMP]     = NULL;
  m_comp[V_COMP]     = NULL;
  m_comp[A_COMP]     = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  m_ui16Comp[A_COMP] = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
}

int InputTIFF::readAttributeInfo( int vfile, FrameFormat *source)
{
  return 1;
}

/*!
 ************************************************************************
 * \brief
 *    Read Header data
 ************************************************************************
 */
int InputTIFF::readHeaderData( int vfile, FrameFormat *source)
{
  return 0;
}

int InputTIFF::readData (int vfile,  FrameFormat *source, uint8 *buf) {
  return 1;
}

int InputTIFF::reformatData () {
  int i, k;
  
  if (m_tiff.BitsPerSample[0] == 8) {
    imgpel *comp0 = NULL;
    imgpel *comp1 = NULL;
    imgpel *comp2 = NULL;
    
    uint16 *curBuf = (uint16 *) &m_tiff.img[0];
    
    // Unpack the data appropriately (interleaving is done at the row level).
    for (k = 0; k < m_height[Y_COMP]; k++) {
      comp0   = &m_comp[0][k * m_width[0]];
      comp1   = &m_comp[1][k * m_width[1]];
      comp2   = &m_comp[2][k * m_width[2]];
      for (i = 0; i < m_width[0]; i++) {
        *comp0++ = (imgpel) *curBuf++;
        *comp1++ = (imgpel) *curBuf++;
        *comp2++ = (imgpel) *curBuf++;
      }
    }
  }
  else {
    uint16 *comp0 = NULL;
    uint16 *comp1 = NULL;
    uint16 *comp2 = NULL;
    
    uint16 *curBuf = (uint16 *) &m_tiff.img[0];
    
    // Unpack the data appropriately (interleaving is done at the row level).
    for (k = 0; k < m_height[Y_COMP]; k++) {
      comp0   = &m_ui16Comp[0][k * m_width[0]];
      comp1   = &m_ui16Comp[1][k * m_width[1]];
      comp2   = &m_ui16Comp[2][k * m_width[2]];
      for (i = 0; i < m_width[0]; i++) {
        *comp0++ = *curBuf++;
        *comp1++ = *curBuf++;
        *comp2++ = *curBuf++;
      }
    }
  }
  
  return 1;
}


//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------

/*!
 ************************************************************************
 * \brief
 *    Reads one new frame from a single TIFF file
 *
 * \param inputFile
 *    Input file to read from
 * \param frameNumber
 *    Frame number in the source file
 * \param fileHeader
 *    Number of bytes in the source file to be skipped
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int InputTIFF::readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileRead = 1;
  int *vfile = &inputFile->m_fileNum;
  FrameFormat *source = &inputFile->m_format;
  
  if (openFrameFile( inputFile, frameNumber + frameSkip) != -1) {
    
    fileRead = readTiff( source, vfile);
    reformatData ();
    
    if (*vfile != -1) {
      close(*vfile);
      *vfile = -1;
    }
    
    return fileRead;
  }
  else
    return 0;
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

