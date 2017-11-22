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
 * \file ImgToBufEndian.cpp
 *
 * \brief
 *    ImgToBufEndian Class to manipulate different I/O formats
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Global.H"
#include "ImgToBufEndian.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ImgToBufEndian::ImgToBufEndian() {
}

ImgToBufEndian::~ImgToBufEndian() {
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void ImgToBufEndian::process (
  imgpel* imgX,             //!< Pointer to image plane
  uint8* buf,               //!< Buffer for file output
  int sizeX,                //!< horizontal size of picture
  int sizeY,                //!< vertical size of picture
  int symbolSizeInBytes     //!< number of bytes in file used for one pixel
  )
{
  uint16 tmp16, ui16;
  unsigned long  tmp32, ui32;

  int j;
  
  // big endian
  switch (symbolSizeInBytes) {
  case 1:
    {
      for(j = 0; j < sizeY * sizeX; j++) {
        buf[j]= (uint8) imgX[j];
      }
      break;
    }
  case 2:
    {
      for(j = 0; j < sizeY * sizeX; j++) {
        tmp16 = (uint16) (imgX[j]);
        ui16  = (uint16) ((tmp16 >> 8) | ((tmp16&0xFF)<<8));
        memcpy(buf+(j * 2),&(ui16), 2);
      }
      break;
    }
  case 4:    
    {
      for(j = 0; j < sizeY * sizeX; j++) {
        tmp32 = (unsigned long) (imgX[j]);
        ui32  = (unsigned long) (((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24));
        memcpy(buf+(j * 4),&(ui32), 4);
      }
      break;
    }
  default:
    {
      fprintf(stderr, "reading only from formats of 8, 16 or 32 bit allowed on big endian architecture\n");
      exit(EXIT_FAILURE);
    }
  }
}

/*!
************************************************************************
* \brief
*      transfer from read buffer to image buffer for 16 bit data
*      note that this function is currently not correct for this  
*      transfer method
************************************************************************
*/

void ImgToBufEndian::process ( 
  uint16* imgX,             //!< Pointer to image plane
  uint8* buf,               //!< Buffer for file output
  int sizeX,                //!< horizontal size of picture
  int sizeY,                //!< vertical size of picture
  int symbolSizeInBytes     //!< number of bytes in file used for one pixel
  )
{
  int j;
  unsigned long  tmp32, ui32;
  uint16 tmp16, ui16;

  if (( sizeof (uint16) == symbolSizeInBytes)) {    
    for(j = 0; j < sizeY * sizeX; j++) {
      tmp16 = (uint16) (imgX[j]);
      ui16  = (uint16) ((tmp16 >> 8) | ((tmp16&0xFF)<<8));
      memcpy(buf+(j * 2),&(ui16), 2);
    }
  }
  else  {
    for(j = 0; j < sizeY * sizeX; j++) {
      tmp32 = (unsigned long) (imgX[j]);
      ui32  = (unsigned long) (((tmp32&0xFF00)<<8) | ((tmp32&0xFF)<<24) | ((tmp32&0xFF0000)>>8) | ((tmp32&0xFF000000)>>24));
      memcpy(buf+(j * 4),&(ui32), 4);
    }
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
