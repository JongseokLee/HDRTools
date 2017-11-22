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
 * \file BufToImgEndian.cpp
 *
 * \brief
 *    BufToImgEndian Class to manipulate different I/O formats
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
#include "BufToImgEndian.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

BufToImgEndian::BufToImgEndian() {
}

BufToImgEndian::~BufToImgEndian() {
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void BufToImgEndian::process ( 
  imgpel* imgX,             //!< Pointer to image plane
  uint8* buf,       //!< Buffer for file output
  int sizeX,               //!< horizontal size of picture
  int sizeY,               //!< vertical size of picture
  int offsetSizeX,             //!< horizontal size of picture
  int offsetSizeY,             //!< vertical size of picture
  int symbolSizeInBytes, //!< number of bytes in file used for one pixel
  int bitShift              //!< variable for bitdepth expansion
  ) 
{
  int j;

  uint16 tmp16, ui16;
  unsigned long  tmp32, ui32;

  if (sizeX != offsetSizeX || sizeY != offsetSizeY) {
    fprintf(stderr, "Rescaling not supported in big endian architectures.\n");
    exit(EXIT_FAILURE);
  }

  // big endian
  switch (symbolSizeInBytes) {
  case 1:
    {
      for(j = 0; j < sizeY * sizeX; j++) {
        imgX[j]= (imgpel) buf[j];
      }
      break;
    }
  case 2:
    {
      uint16 *imgX_16b = ( uint16 *)imgX;
      for(j = 0; j < sizeY * sizeX; j++) {
        memcpy(&tmp16, buf + (j * 2), 2);
        ui16  = (tmp16 >> 8) | ((tmp16 & 0xFF) << 8);
        imgX_16b[j] = ui16;
      }
      break;
    }
  case 4:    
    {
      uint32 *imgX_32b = (uint32 *)imgX;
      for(j = 0; j < sizeY * sizeX; j++) {
        memcpy(&tmp32, buf + (j * 4), 4);
        ui32  = ((tmp32 & 0xFF00) << 8) | ((tmp32 & 0xFF) << 24) | ((tmp32 & 0xFF0000) >> 8) | ((tmp32 & 0xFF000000) >> 24);
        imgX_32b[j] = (uint32) ui32;
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

void BufToImgEndian::process ( 
  uint16* imgX,             //!< Pointer to image plane
  uint8* buf,               //!< Buffer for file output
  int sizeX,               //!< horizontal size of picture
  int sizeY,               //!< vertical size of picture
  int offsetSizeX,             //!< horizontal size of picture
  int offsetSizeY,             //!< vertical size of picture
  int symbolSizeInBytes, //!< number of bytes in file used for one pixel
  int bitShift              //!< variable for bitdepth expansion
  ) 
{
  int i,j;
  uint8* tempBuffer = buf;

  if (( sizeof (uint16) == symbolSizeInBytes)) {    
    // uint16 == pixel_in_file -> simple copy
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
      memcpy(imgX, tempBuffer, sizeY * sizeX * sizeof(uint16));
    }
    else {
      int iMinWidth   = iMin(sizeX, offsetSizeX);
      int iMinHeight  = iMin(sizeY, offsetSizeY);
      int destOffsetX  = 0, destOffsetY = 0;
      int offsetX = 0, offsetY = 0; // currently not used

      // determine whether we need to center the copied frame or crop it
      if ( offsetSizeX >= sizeX ) 
        destOffsetX = ( offsetSizeX - sizeX  ) >> 1;

      if (offsetSizeY >= sizeY) 
        destOffsetY = ( offsetSizeY - sizeY ) >> 1;

      // check copied area to avoid copying memory garbage
      // source
      iMinWidth  =  ( (offsetX + iMinWidth ) > sizeX ) ? (sizeX - offsetX) : iMinWidth;
      iMinHeight =  ( (offsetY + iMinHeight) > sizeY ) ? (sizeY - offsetY) : iMinHeight;
      // destination
      iMinWidth  =  ( (destOffsetX + iMinWidth ) > offsetSizeX  ) ? (offsetSizeX - destOffsetX) : iMinWidth;
      iMinHeight =  ( (destOffsetY + iMinHeight) > offsetSizeY )  ? (offsetSizeY - destOffsetY) : iMinHeight;

      for (i=0; i<iMinHeight;i++) {
        memcpy(&imgX[(i + destOffsetY) * offsetSizeX + destOffsetX], &(tempBuffer[(i + offsetY) * sizeX + offsetX]), iMinWidth * sizeof(uint16));
      }
    }
  }
  else if (( sizeof (uint16) == symbolSizeInBytes)) {    
    // uint16 == pixel_in_file -> simple copy
    uint16* temp_buf_16b = (uint16*)buf;
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
      memcpy(imgX, tempBuffer, sizeY * sizeX * sizeof(uint16));
    }
    else {
      int iMinWidth   = iMin(sizeX, offsetSizeX);
      int iMinHeight  = iMin(sizeY, offsetSizeY);
      int destOffsetX  = 0, destOffsetY = 0;
      int offsetX = 0, offsetY = 0; // currently not used

      // determine whether we need to center the copied frame or crop it
      if ( offsetSizeX >= sizeX ) 
        destOffsetX = ( offsetSizeX  - sizeX  ) >> 1;

      if (offsetSizeY >= sizeY) 
        destOffsetY = ( offsetSizeY - sizeY ) >> 1;

      // check copied area to avoid copying memory garbage
      // source
      iMinWidth  =  ( (offsetX + iMinWidth ) > sizeX ) ? (sizeX  - offsetX) : iMinWidth;
      iMinHeight =  ( (offsetY + iMinHeight) > sizeY ) ? (sizeY - offsetY) : iMinHeight;
      // destination
      iMinWidth  =  ( (destOffsetX + iMinWidth ) > offsetSizeX  ) ? (offsetSizeX  - destOffsetX) : iMinWidth;
      iMinHeight =  ( (destOffsetY + iMinHeight) > offsetSizeY )  ? (offsetSizeY - destOffsetY) : iMinHeight;

      for (i=0; i<iMinHeight;i++) {
        memcpy(&imgX[(i + destOffsetY) * offsetSizeX + destOffsetX], &(temp_buf_16b[(i + offsetY) * sizeX + offsetX]), iMinWidth * sizeof(uint16));
      }
    }
  }
  else  {
    int j_pos;
    uint16 ui16;
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
      for (j=0; j < offsetSizeY * offsetSizeX; j++) {
        ui16 = 0;          
        memcpy(&(ui16), buf + (j * symbolSizeInBytes), symbolSizeInBytes);
        imgX[j]= (uint16) ui16;
      }    
    }
    else {
      int iMinWidth   = iMin(sizeX, offsetSizeX);
      int iMinHeight  = iMin(sizeY, offsetSizeY);
      int destOffsetX  = 0, destOffsetY = 0;
      int offsetX = 0, offsetY = 0; // currently not used

      // determine whether we need to center the copied frame or crop it
      if ( offsetSizeX >= sizeX ) 
        destOffsetX = ( offsetSizeX  - sizeX  ) >> 1;

      if (offsetSizeY >= sizeY) 
        destOffsetY = ( offsetSizeY - sizeY ) >> 1;

      // check copied area to avoid copying memory garbage
      // source
      iMinWidth  =  ( (offsetX + iMinWidth ) > sizeX ) ? (sizeX  - offsetX) : iMinWidth;
      iMinHeight =  ( (offsetY + iMinHeight) > sizeY ) ? (sizeY - offsetY) : iMinHeight;
      // destination
      iMinWidth  =  ( (destOffsetX + iMinWidth ) > offsetSizeX  ) ? (offsetSizeX  - destOffsetX) : iMinWidth;
      iMinHeight =  ( (destOffsetY + iMinHeight) > offsetSizeY )  ? (offsetSizeY - destOffsetY) : iMinHeight;

      for (j = 0; j < iMinHeight; j++) {
        memcpy(&imgX[(j + destOffsetY) * offsetSizeX + destOffsetX], &(tempBuffer[(j + offsetY) * sizeX + offsetX]), iMinWidth * symbolSizeInBytes);
      }
      for (j=0; j < iMinHeight; j++) {        
        j_pos = (j + offsetY) * sizeX + offsetX;
        for (i=0; i < iMinWidth; i++) {
          ui16 = 0;
          memcpy(&(ui16), buf + ((i + j_pos) * symbolSizeInBytes), symbolSizeInBytes);
          imgX[(j + destOffsetY) * offsetSizeX + i + destOffsetX]= (uint16) ui16;
        }
      }    
    }
  }
}

//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
