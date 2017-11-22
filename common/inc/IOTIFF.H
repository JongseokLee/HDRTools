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
 * \file IOTIFF.H
 *
 * \brief
 *    Main IOTIFF header file
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __IOTIFF_H__
#define __IOTIFF_H__
// Maximum number of rows in image
#define YRES 4320

//! TIFF Field Type Code
typedef enum TiffType {
  T_BYTE      = 1,   //!< 8-bit unsigned integer.
  T_ASCII     = 2,   //!< 8-bit byte that contains a 7-bit ASCII code; the last byte must be NUL (binary zero).
  T_SHORT     = 3,   //!< 16-bit (2-byte) unsigned integer.
  T_LONG      = 4,   //!< 32-bit (4-byte) unsigned integer.
  T_RATIONAL  = 5,   //!< Two LONGs:  the first represents the numerator of a fraction; the second, the denominator.
  T_SBYTE     = 6,   //!< An 8-bit signed (twos-complement) integer.
  T_UNDEFINED = 7,   //!< An 8-bit byte that may contain anything, depending on the definition of the field.
  T_SSHORT    = 8,   //!< A 16-bit (2-byte) signed (twos-complement) integer.
  T_SLONG     = 9,   //!< A 32-bit (4-byte) signed (twos-complement) integer.
  T_SRATIONAL = 10,  //!< Two SLONGís:  the first represents the numerator of a fraction, the second the denominator.
  T_FLOAT     = 11,  //!< Single precision (4-byte) IEEE format.
  T_DOUBLE    = 12   //!< Double precision (8-byte) IEEE format.
} TiffType;


typedef struct TiffDirectoryEntry {
  uint16  tag;                                //!< The tag that identifies the field.
  uint16  type;                               //!< The field type.
  uint32  count;                              //!< Number of values of the indicated type.
  uint32  offset;                             //!< Value or offset.
} TiffDirectoryEntry;


//! TIFF Image File Directory
typedef struct TiffIFD {
  uint16             nEntries;
  TiffDirectoryEntry *directoryEntry;
} TiffIFD;


typedef struct TiffImageFileHeader {
  uint16 byteOrder;                          //!< "II" (4949H) or "MM" (4D4DH)
  uint16 arbitraryNumber;                    //!< 42
  uint32 offset;                             //!< Offset of the 0th IFD
} TiffImageFileHeader;


//! TIFF file data
typedef struct Tiff {
  vector<uint16>  img;                  //!< Image data
  vector<uint8>   fileInMemory;         //!< The file will be read into memory in one gulp here.
  uint8    *mp;                              //!< Memory pointer.
  int      le;                               //!< Little endian - 0 FALSE, 1 - TRUE
  int      nStrips;
  TiffImageFileHeader ifh;
  // Information from TAGs
  uint16   Orientation;
  uint32   BitsPerSample[3];
  uint32   RowsPerStrip;
  uint32   ImageLength;
  uint32   ImageWidth;
  uint32   StripByteCounts[YRES];
  uint32   StripOffsets[YRES];
  uint32   XResolution[2];
  uint32   YResolution[2];

  uint32 (*setU16) (struct Tiff *, uint16 value);
  uint32 (*setU32) (struct Tiff *, uint32 value);
  uint16 (*getU16) (struct Tiff *);
  uint32 (*getU32) (struct Tiff *);
} Tiff;


#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
