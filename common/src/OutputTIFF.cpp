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
 * \file OutputTIFF.cpp
 *
 * \brief
 *    OutputTIFF class C++ file for allowing input from TIFF files
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
#include "OutputTIFF.H"
#include "Global.H"
#include "IOFunctions.H"

//-----------------------------------------------------------------------------
// Macros/Defines
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

OutputTIFF::OutputTIFF(IOVideo *videoFile, FrameFormat *format) {
  // We currently do not support floating point data
  m_isFloat         = FALSE;
  format->m_isFloat = m_isFloat;
  videoFile->m_format.m_isFloat = m_isFloat;
  m_frameRate       = format->m_frameRate;
  
  m_memoryAllocated = FALSE;
  
  m_comp[Y_COMP]      = NULL;
  m_comp[U_COMP]      = NULL;
  m_comp[V_COMP]      = NULL;
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  m_ui16Comp[Y_COMP] = NULL;
  m_ui16Comp[U_COMP] = NULL;
  m_ui16Comp[V_COMP] = NULL;
  m_ui16Comp[A_COMP] = NULL;
  
  allocateMemory(format);
  
}

OutputTIFF::~OutputTIFF() {
  
  freeMemory();
  
  clear();
}

//-----------------------------------------------------------------------------
// Private methods
//-----------------------------------------------------------------------------

/*!
 ************************************************************************
 * \brief
 *   Set an unsigned short without swapping.
 *
 ************************************************************************
 */
static uint32 setU16 (Tiff * t, uint16 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  
  return 2;
}


/*!
 ************************************************************************
 * \brief
 *   Set an unsigned int32 without swapping.
 *
 ************************************************************************
 */
static uint32 setU32 (Tiff * t, uint32 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  *t->mp++ = *pIn++;
  
  return 4;
}


// Swap versions

/*!
 ************************************************************************
 * \brief
 *   Set an unsigned short and swap.
 *
 ************************************************************************
 */
static uint32 setSwappedU16 (Tiff * t, uint16 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = pIn[1];
  *t->mp++ = pIn[0];
  
  return 2;
}


/*!
 ************************************************************************
 * \brief
 *   Set an unsigned int32 and swap.
 *
 ************************************************************************
 */
static uint32 setSwappedU32 (Tiff * t, uint32 in)
{
  char *pIn = (char *) &in;
  *t->mp++ = pIn[3];
  *t->mp++ = pIn[2];
  *t->mp++ = pIn[1];
  *t->mp++ = pIn[0];
  
  return 2;
}


/*!
 ************************************************************************
 * \brief
 *   Set an array of (uint16|uint32|rational).
 *
 ************************************************************************
 */
uint32 OutputTIFF::setIntArray (Tiff * t, uint32 offset, TiffType type, uint32 a[], int n)
{
  uint32 count = 0;
  int    i;
  uint8  *mp = t->mp;  // save memory pointer
  
  t->mp = (uint8 *) &t->fileInMemory[offset];
  switch (type)  {
    case T_SHORT:
      for (i=0; i < n; ++i) {
        count += t->setU16( t, (uint16) a[i] );
      }
      break;
    case T_LONG:
      for (i=0; i < n; ++i) {
        count += t->setU32( t, a[i] );
      }
      break;
    case T_RATIONAL:
      for (i=0; i < 2*n; ++i)  {
        count += t->setU32( t, a[i]);
      }
      break;
    default:
      assert( type == T_SHORT || type == T_LONG || type == T_RATIONAL);
  }
  
  if (maxFramePosition < (int64) t->mp)
    maxFramePosition = (int64) t->mp;
  
  t->mp = mp;                           // restore memory pointer
  
  return count;
}

uint32 muxOffset ( Tiff * t, uint32 offset, uint32 inpValue) {
  if (t->setU32 == setU32)
    return (offset  << 16 ) | (0xFFFF & inpValue);
  else
    return (inpValue << 16) | (0xFFFF & offset);
}

/*!
 ************************************************************************
 * \brief
 *   Read DirectoryEntry and store important results in 't'.
 *
 ************************************************************************
 */
uint32 OutputTIFF::writeDirectoryEntry (Tiff * t, uint32 tag, TiffType type, uint32 count, uint32 offset)
{
  uint32 byteCounter = 0;
  uint32 offsetData = offset;
  
  byteCounter += t->setU16( t, tag);
  byteCounter += t->setU16( t, (uint16) type);
  byteCounter += t->setU32( t, count);
  
  switch (tag)  {
    case 256:                           // ImageWidth  SHORT or LONG
    case 257:                           // ImageLength    SHORT or LONG
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 258:                           // BitsPerSample  SHORT 8,8,8
      if (count == 1) {
        if (type == T_SHORT) {
          offsetData = muxOffset(t, offset, t->BitsPerSample[0]);
        }
        else
          offsetData = offset;
        
        byteCounter += t->setU32( t, offsetData);
      }
      else if (count != 3) {
        fprintf( stderr, "BitsPerSample (only [1] & [3] supported)\n");
        return 1;
      }
      else {
        byteCounter += t->setU32( t, offsetData);
        byteCounter += setIntArray( t, offset, type, t->BitsPerSample, count);
      }
      break;
    case 259:                           // Compression SHORT 1 or 32773
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 262:                           // PhotometricInterpretation SHORT 2
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 273:                           // StripOffsets  SHORT or LONG
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 274:                           // Orientation  SHORT
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 277:                           // SamplesPerPixel SHORT 3 or more
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 278:                           // RowsPerStrip  SHORT or LONG
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 279:                           // StripByteCounts LONG or SHORT
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 282:                           // XResolution  RATIONAL
      assert( count == 1);
      byteCounter += t->setU32( t, offsetData);
      byteCounter = setIntArray( t, offset, type, t->XResolution, 1);
      break;
    case 283:                           // YResolution  RATIONAL
      assert( count == 1);
      byteCounter += t->setU32( t, offsetData);
      byteCounter += setIntArray( t, offset, type, t->YResolution, 1);
      break;
    case 284:                           // PlanarConfiguration  SHORT
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      assert( offsetData == 1);
      break;
    case 296:                           // ResolutionUnit  SHORT 1, 2 or 3
      assert( count == 1);
      byteCounter += t->setU32( t, offset);
      break;
    case 305:                           // Software  ASCII
                                        // Not supported
      break;
    case 339:                           // SampleFormat  SHORT 1
    default:
      // Not supported
      break;
  }
  return count;
}


/*!
 ************************************************************************
 * \brief
 *   Write writeFileFromMemory into file named 'path' into memory buffer 't->fileInMemory'.
 *
 * \return
 *   0 if successful
 ************************************************************************
 */
int OutputTIFF::writeFileFromMemory (Tiff * t, int *fd, uint32 counter)
{
  long result;
  
  assert( t);
  
  //if (lseek( fd, 0, SEEK_SET) == -1L)   // reposition file at beginning
  //return 1;
  
  
  result = (long) mm_write ( *fd, (char *) &t->fileInMemory[0], counter);
  if (result != counter) {
    if (*fd != -1) {
      close( *fd);
      *fd = -1;
    }
    return 1;
  }
  
  
  return 0;
}

/*!
 ************************************************************************
 * \brief
 *    Read image data into 't->img'.
 *
 ************************************************************************
 */
uint32 OutputTIFF::writeImageData (Tiff * t)
{
  int     i, j, n;
  uint8  *mp, *s;
  uint16 *p;
  uint32 byteCounter = 0;
  
  switch (t->BitsPerSample[0]) {
    case 8:
      p = (uint16 *) &t->img[0];
      for (i=0; i < t->nStrips; ++i) {
        n = t->StripByteCounts[i];
        s = (uint8 *) &t->fileInMemory[t->StripOffsets[i]];
        byteCounter += n;
        for (j=0; j < n; ++j) {
          *s++ = (uint8) *p++;
        }
        if (maxFramePosition < (int64) s)
          maxFramePosition = (int64) s;
      }
      break;
    case 16:
      mp = t->mp;                       // save memory pointer
      p = (uint16 *) &t->img[0];
      for (i=0; i < t->nStrips; ++i) {
        n = t->StripByteCounts[i] / 2;
        t->mp = (uint8 *) &t->fileInMemory[t->StripOffsets[i]];
        for (j=0; j < n; ++j) {
          byteCounter += t->setU16( t, *p++);
        }
      }
      if (maxFramePosition < (int64) t->mp)
        maxFramePosition = (int64) t->mp;
      
      t->mp = mp;                       // restore memory pointer
      break;
  }
  return byteCounter;
}


/*!
 *****************************************************************************
 * \brief
 *    Write the ImageFileDirectory.
 *
 *****************************************************************************
 */
uint32 OutputTIFF::writeImageFileDirectory (Tiff * t)
{
  uint32 count = 0;
  uint32 nEntries = 10; // Currently lets add all entries (trim later)
  count += t->setU16( t, nEntries);
  
  // Do not modify below entries. Only a limited set of options are supported at this point
  count += writeDirectoryEntry( t, 256, T_SHORT, 1, t->ImageWidth);  // ImageWidth
  count += writeDirectoryEntry( t, 257, T_SHORT, 1, t->ImageLength); // ImageLength
  count += writeDirectoryEntry( t, 258, T_SHORT, 3, 8); // bitdepth
  count += writeDirectoryEntry( t, 259, T_SHORT, 1, 1); // compression
  count += writeDirectoryEntry( t, 262, T_SHORT, 1, 2); // PhotometricInterpretation
  count += writeDirectoryEntry( t, 273, T_SHORT, t->nStrips, t->StripOffsets[0]); // StripOffsets
  count += writeDirectoryEntry( t, 274, T_SHORT, 1, (uint32) t->Orientation); // Orientation
  count += writeDirectoryEntry( t, 277, T_SHORT, 1, 3); // SamplesPerPixel
                                                        // count += writeDirectoryEntry( t, 278, T_SHORT, 1, t->RowsPerStrip); // RowsPerStrip
  count += writeDirectoryEntry( t, 279, T_LONG, 1, t->StripByteCounts[0]); // StripByteCounts
                                                                           // count += writeDirectoryEntry( t, 282, T_RATIONAL, 1, 0); // XResolution
                                                                           // count += writeDirectoryEntry( t, 283, T_RATIONAL, 1, 0); // YResolution
  count += writeDirectoryEntry( t, 284, T_SHORT, 1, 1); // PlanarConfiguration
                                                        // count += writeDirectoryEntry( t, 296, T_SHORT, 1, 1); // ResolutionUnit
                                                        // count += writeDirectoryEntry( t, 305, T_ASCII, 1, 1); // Software
                                                        // count += writeDirectoryEntry( t, 339, T_SHORT, 1, 1); // Unforseen
  
  if (maxFramePosition < (int64) t->mp)
    maxFramePosition = (int64) t->mp;
  
  return count;
}


/*!
 *****************************************************************************
 * \brief
 *    Read the ImageFileHeader.
 *
 *****************************************************************************
 */
uint32 OutputTIFF::writeImageFileHeader (Tiff * t)
{
  uint32 byteCounter = 0;
  t->mp = &t->fileInMemory[0];
  maxFramePosition = (int64) t->mp;
  t->ifh.byteOrder = 0x4949;
  byteCounter += t->setU16( t, t->ifh.byteOrder);
  t->ifh.arbitraryNumber = 42;
  byteCounter += t->setU16( t, t->ifh.arbitraryNumber);
  t->ifh.offset = 32;
  byteCounter += t->setU32( t, t->ifh.offset);
  
  // extend byteCounter in case we would like to start from a different position
  //byteCounter += t->ifh.offset - 8;
  t->mp = &t->fileInMemory[t->ifh.offset];
  maxFramePosition = (int64) t->mp;
  
  return byteCounter;
}

/*!
 *****************************************************************************
 * \brief
 *    Write the TIFF file.
 *
 *****************************************************************************
 */
int OutputTIFF::writeTiff (FrameFormat *format, int *fd) {
  assert( &m_tiff);
  
  if (m_memoryAllocated == FALSE) {
    allocateMemory(format);
  }
  
  writeImageFileHeader( &m_tiff);
  writeImageFileDirectory( &m_tiff);
  writeImageData( &m_tiff);
  
  if (writeFileFromMemory( &m_tiff, fd, (uint32) (maxFramePosition - (int64) &m_tiff.fileInMemory[0])))
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
int OutputTIFF::openFrameFile( IOVideo *outputFile, int FrameNumberInFile)
{
  char outFile [FILE_NAME_SIZE], outNumber[16];
  int length = 0;
  outNumber[length]='\0';
  length = (int) strlen(outputFile->m_fHead);
  strncpy(outFile, outputFile->m_fHead, length);
  outFile[length]='\0';
  
  // Is this a single frame file? If yes, m_fTail would be of size 0.
  if (strlen(outputFile->m_fTail) != 0) {
    if (outputFile->m_zeroPad == TRUE)
      snprintf(outNumber, 16, "%0*d", outputFile->m_numDigits, FrameNumberInFile);
    else
      snprintf(outNumber, 16, "%*d", outputFile->m_numDigits, FrameNumberInFile);
    
    strncat(outFile, outNumber, strlen(outNumber));
    length += sizeof(outNumber);
    outFile[length]='\0';
    strncat(outFile, outputFile->m_fTail, strlen(outputFile->m_fTail));
    length += (int) strlen(outputFile->m_fTail);
    outFile[length]='\0';
  }
  
  if ((outputFile->m_fileNum = open(outFile, OPENFLAGS_WRITE, OPEN_PERMISSIONS)) == -1)  {
    printf ("openFrameFile: cannot open file %s\n", outFile);
    exit(EXIT_FAILURE);
  }
  
  return outputFile->m_fileNum;
}

void OutputTIFF::allocateMemory(FrameFormat *format)
{
  m_memoryAllocated = TRUE;
  m_chromaFormat     = format->m_chromaFormat;
  
  m_tiff.ImageLength = m_height[Y_COMP] = format->m_height[Y_COMP];
  m_tiff.ImageWidth  = m_width[Y_COMP]  = format->m_width [Y_COMP];
  
  switch (m_chromaFormat){
    case CF_400:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = 0;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = 0;
      break;
    case CF_420:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP] >> ONE;
      break;
    case CF_422:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP] >> ONE;
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      break;
    case CF_444:
      m_width [U_COMP] = m_width [V_COMP] = format->m_height[U_COMP] = format->m_height[V_COMP] = m_width [Y_COMP];
      m_height[U_COMP] = m_height[V_COMP] = format->m_width[U_COMP] = format->m_width[V_COMP] = m_height[Y_COMP];
      break;
    default:
      fprintf(stderr, "\n Unsupported Chroma Subsampling Format %d\n", m_chromaFormat);
      exit(EXIT_FAILURE);
  }
  m_height [A_COMP] = 0;
  m_width  [A_COMP] = 0;
  
  m_compSize[Y_COMP] = format->m_compSize[Y_COMP] = m_height[Y_COMP] * m_width[Y_COMP];
  m_compSize[U_COMP] = format->m_compSize[U_COMP] = m_height[U_COMP] * m_width[U_COMP];
  m_compSize[V_COMP] = format->m_compSize[V_COMP] = m_height[V_COMP] * m_width[V_COMP];
  m_compSize[A_COMP] = format->m_compSize[A_COMP] = m_height[A_COMP] * m_width[A_COMP];
  
  
  m_size = format->m_size = m_compSize[Y_COMP] + m_compSize[U_COMP] + m_compSize[V_COMP] + m_compSize[A_COMP];
  
  // Color space is explicitly specified here. We could do a test inside the code though to at least check if
  // RGB is RGB and YUV is YUV, but not important currently
  m_colorSpace       = format->m_colorSpace;
  m_colorPrimaries   = format->m_colorPrimaries;
  m_sampleRange      = format->m_sampleRange;
  
  //format->m_colorSpace   = m_colorSpace   = CM_RGB;
  m_transferFunction = format->m_transferFunction;
  m_systemGamma      = format->m_systemGamma;
  
  m_bitDepthComp[Y_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[Y_COMP] > 8) ? 16 : format->m_bitDepthComp[Y_COMP];
  m_bitDepthComp[U_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[U_COMP] > 8) ? 16 : format->m_bitDepthComp[U_COMP];
  m_bitDepthComp[V_COMP] = (m_sampleRange == SR_SDI_SCALED && format->m_bitDepthComp[U_COMP] > 8) ? 16 : format->m_bitDepthComp[V_COMP];
  
  m_tiff.BitsPerSample[Y_COMP] = (m_bitDepthComp[Y_COMP] > 8 ? 16 : 8);
  m_tiff.BitsPerSample[U_COMP] = (m_bitDepthComp[U_COMP] > 8 ? 16 : 8);
  m_tiff.BitsPerSample[V_COMP] = (m_bitDepthComp[V_COMP] > 8 ? 16 : 8);
  
  
  m_bitDepthComp[A_COMP] = format->m_bitDepthComp[A_COMP];
  
  // Only orientation 1 is supported
  m_tiff.Orientation = 1;
  
  m_isInterleaved = FALSE;
  m_isInterlaced  = FALSE;
  
  m_chromaLocation[FP_TOP]    = format->m_chromaLocation[FP_TOP];
  m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM];
  
  if (m_isInterlaced == FALSE && m_chromaLocation[FP_TOP] != m_chromaLocation[FP_BOTTOM]) {
    printf("Progressive Content. Chroma Type Location needs to be the same for both fields.\n");
    printf("Resetting Bottom field chroma location from type %d to type %d\n", m_chromaLocation[FP_BOTTOM], m_chromaLocation[FP_TOP]);
    m_chromaLocation[FP_BOTTOM] = format->m_chromaLocation[FP_BOTTOM] = m_chromaLocation[FP_TOP];
  }
  
  
  // init size of file based on image data to write (without headers
  m_tiffSize = m_size * (m_tiff.BitsPerSample[Y_COMP] > 8 ? 2 : 1);
  
  //uint32  size;
  //assert( t);
  //size = t->ImageWidth * t->ImageLength * 3 * sizeof(uint16);
  
  // memory buffer for the image data
  m_tiff.img.resize((unsigned int) m_tiffSize);
  // Assign a rather large buffer
  m_tiff.fileInMemory.resize((long) m_size * 4);
  
  if (format->m_bitDepthComp[Y_COMP] == 8) {
    m_data.resize((unsigned int) m_size);
    m_comp[Y_COMP]      = &m_data[0];
    m_comp[U_COMP]      = m_comp[Y_COMP] + m_compSize[Y_COMP];
    m_comp[V_COMP]      = m_comp[U_COMP] + m_compSize[U_COMP];
    m_comp[A_COMP]      = NULL;
    m_ui16Comp[Y_COMP]  = NULL;
    m_ui16Comp[U_COMP]  = NULL;
    m_ui16Comp[V_COMP]  = NULL;
    m_ui16Comp[A_COMP]  = NULL;
  }
  else {
    m_comp[Y_COMP]      = NULL;
    m_comp[U_COMP]      = NULL;
    m_comp[V_COMP]      = NULL;
    m_ui16Data.resize((unsigned int) m_size);
    m_ui16Comp[Y_COMP]  = &m_ui16Data[0];
    m_ui16Comp[U_COMP]  = m_ui16Comp[Y_COMP] + m_compSize[Y_COMP];
    m_ui16Comp[V_COMP]  = m_ui16Comp[U_COMP] + m_compSize[U_COMP];
    m_ui16Comp[A_COMP]  = NULL;
  }
  
  m_floatComp[Y_COMP] = NULL;
  m_floatComp[U_COMP] = NULL;
  m_floatComp[V_COMP] = NULL;
  m_floatComp[A_COMP] = NULL;
  
  int endian = 1;
  int machineLittleEndian = (*( (char *)(&endian) ) == 1) ? 1 : 0;
  
  m_tiff.le = 1;
  if (m_tiff.le == machineLittleEndian)  { // endianness of machine matches file
    m_tiff.setU16 = setU16;
    m_tiff.setU32 = setU32;
  }
  else {                               // endianness of machine does not match file
    m_tiff.setU16 = setSwappedU16;
    m_tiff.setU32 = setSwappedU32;
  }
  m_tiff.mp = (uint8 *) &m_tiff.fileInMemory[0];
  
  m_tiff.nStrips = 1;
  m_tiff.StripOffsets[0] = 256;
  m_tiff.StripByteCounts[0] = (uint32) m_tiffSize;
  m_tiff.Orientation = 1;
  m_tiff.RowsPerStrip = m_height[Y_COMP];
  
}

void OutputTIFF::freeMemory()
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

int OutputTIFF::writeAttributeInfo( int vfile, FrameFormat *source)
{
  return 1;
}

/*!
 ************************************************************************
 * \brief
 *    Read Header data
 ************************************************************************
 */
int OutputTIFF::writeHeaderData( int vfile, FrameFormat *source)
{
  return 0;
}

int OutputTIFF::writeData (int vfile,  FrameFormat *source, uint8 *buf) {
  return 1;
}

int OutputTIFF::reformatData () {
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
        *curBuf++ = *comp0++;
        *curBuf++ = *comp1++;
        *curBuf++ = *comp2++;
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
        *curBuf++ = *comp0++;
        *curBuf++ = *comp1++;
        *curBuf++ = *comp2++;
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
 *    Writes one new frame into a single Tiff file
 *
 * \param inputFile
 *    Output file to write into
 * \param frameNumber
 *    Frame number in the source file
 * \param fileHeader
 *    Number of bytes in the source file to be skipped
 * \param frameSkip
 *    Start position in file
 ************************************************************************
 */
int OutputTIFF::writeOneFrame (IOVideo *outputFile, int frameNumber, int fileHeader, int frameSkip) {
  int fileWrite = 1;
  int *vfile = &outputFile->m_fileNum;
  FrameFormat *format = &outputFile->m_format;
  openFrameFile( outputFile, frameNumber + frameSkip);
  reformatData ();
  
  fileWrite = writeTiff( format, vfile);
  
  if (*vfile != -1) {
    close(*vfile);
    *vfile = -1;
  }
  
  return fileWrite;
}
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------

