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
 * \file BufToImgBasic.cpp
 *
 * \brief
 *    BufToImgBasic Class to manipulate different I/O formats
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
#include "BufToImgBasic.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

BufToImgBasic::BufToImgBasic() {
}

BufToImgBasic::~BufToImgBasic() {
}

//-----------------------------------------------------------------------------
// Private methods (non-MMX)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void BufToImgBasic::process ( imgpel* imgX,             //!< Pointer to image plane
                              unsigned char* buf,       //!< Buffer for file output
                              int sizeX,                //!< horizontal size of picture
                              int sizeY,                //!< vertical size of picture
                              int offsetSizeX,          //!< horizontal size of picture
                              int offsetSizeY,          //!< vertical size of picture
                              int symbolSizeInBytes,    //!< number of bytes in file used for one pixel
                              int bitShift              //!< variable for bitdepth expansion
)
{
  int i,j;
  unsigned char* tempBuffer = buf;

  if (( sizeof (imgpel) == symbolSizeInBytes)) {
    // imgpel == pixel_in_file -> simple copy
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
        memcpy(imgX, tempBuffer, sizeY * sizeX * sizeof(imgpel));
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
        memcpy(&imgX[(i + destOffsetY) * offsetSizeX + destOffsetX], &(tempBuffer[(i + offsetY) * sizeX + offsetX]), iMinWidth * sizeof(imgpel));
      }
    }
  }
  else if (( sizeof (unsigned short) == symbolSizeInBytes)) {    
    // imgpel == pixel_in_file -> simple copy
    unsigned short* temp_buf_16b = (unsigned short*)buf;
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
      memcpy(imgX, tempBuffer, sizeY * sizeX * sizeof(unsigned short));
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
      iMinWidth  =  ( (offsetX + iMinWidth ) > sizeX ) ? ( sizeX - offsetX ) : iMinWidth;
      iMinHeight =  ( (offsetY + iMinHeight) > sizeY ) ? ( sizeY - offsetY ) : iMinHeight;
      // destination
      iMinWidth  =  ( (destOffsetX + iMinWidth ) > offsetSizeX  ) ? ( offsetSizeX - destOffsetX ) : iMinWidth;
      iMinHeight =  ( (destOffsetY + iMinHeight) > offsetSizeY )  ? ( offsetSizeY - destOffsetY ) : iMinHeight;

      for (i=0; i<iMinHeight;i++) {
        memcpy(&imgX[(i + destOffsetY) * offsetSizeX + destOffsetX], &(temp_buf_16b[(i + offsetY) * sizeX + offsetX]), iMinWidth * sizeof(unsigned short));
      }
    }
  }
  else  {
    int j_pos;
    unsigned short ui16;
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
      for (j=0; j < offsetSizeY * offsetSizeX; j++) {
        ui16 = 0;          
        memcpy(&(ui16), buf + (j * symbolSizeInBytes), symbolSizeInBytes);
        imgX[j]= (imgpel) ui16;
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
      iMinWidth  =  ( (offsetX + iMinWidth ) > sizeX ) ? (sizeX - offsetX) : iMinWidth;
      iMinHeight =  ( (offsetY + iMinHeight) > sizeY ) ? (sizeY - offsetY) : iMinHeight;
      // destination
      iMinWidth  =  ( (destOffsetX + iMinWidth ) > offsetSizeX  ) ? (offsetSizeX - destOffsetX) : iMinWidth;
      iMinHeight =  ( (destOffsetY + iMinHeight) > offsetSizeY )  ? (offsetSizeY - destOffsetY) : iMinHeight;

      for (j = 0; j < iMinHeight; j++) {
        memcpy(&imgX[(j + destOffsetY) * offsetSizeX + destOffsetX], &(tempBuffer[(j + offsetY) * sizeX + offsetX]), iMinWidth * symbolSizeInBytes);
      }
      for (j=0; j < iMinHeight; j++) {        
        j_pos = (j + offsetY) * sizeX + offsetX;
        for (i=0; i < iMinWidth; i++) {
          ui16 = 0;
          memcpy(&(ui16), buf + ((i + j_pos) * symbolSizeInBytes), symbolSizeInBytes);
          imgX[(j + destOffsetY) * offsetSizeX + i + destOffsetX]= (imgpel) ui16;
        }
      }    
    }
  }
}

void BufToImgBasic::process ( uint16* imgX,             //!< Pointer to image plane
                              uint8* buf,               //!< Buffer for file output
                              int sizeX,                //!< horizontal size of picture
                              int sizeY,                //!< vertical size of picture
                              int offsetSizeX,          //!< horizontal size of picture
                              int offsetSizeY,          //!< vertical size of picture
                              int symbolSizeInBytes,    //!< number of bytes in file used for one pixel
                              int bitShift              //!< variable for bitdepth expansion
                            ) 
{
  int i,j;
  unsigned char* tempBuffer = buf;
  
  if (( sizeof (unsigned short) == symbolSizeInBytes)) {
    // unsigned short == pixel_in_file -> simple copy
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
        memcpy(imgX, tempBuffer, sizeY * sizeX * sizeof(unsigned short));
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
        memcpy(&imgX[(i + destOffsetY) * offsetSizeX + destOffsetX], &(tempBuffer[(i + offsetY) * sizeX + offsetX]), iMinWidth * sizeof(unsigned short));
      }
    }
  }
  else if (( sizeof (unsigned short) == symbolSizeInBytes)) {    
    // unsigned short == pixel_in_file -> simple copy
    unsigned short* temp_buf_16b = (unsigned short*)buf;
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
      memcpy(imgX, tempBuffer, sizeY * sizeX * sizeof(unsigned short));
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
      iMinWidth  =  ( (offsetX + iMinWidth ) > sizeX ) ? ( sizeX - offsetX ) : iMinWidth;
      iMinHeight =  ( (offsetY + iMinHeight) > sizeY ) ? ( sizeY - offsetY ) : iMinHeight;
      // destination
      iMinWidth  =  ( (destOffsetX + iMinWidth ) > offsetSizeX  ) ? (offsetSizeX - destOffsetX) : iMinWidth;
      iMinHeight =  ( (destOffsetY + iMinHeight) > offsetSizeY )  ? (offsetSizeY - destOffsetY) : iMinHeight;

      for (i=0; i<iMinHeight;i++) {
        memcpy(&imgX[(i + destOffsetY) * offsetSizeX + destOffsetX], &(temp_buf_16b[(i + offsetY) * sizeX + offsetX]), iMinWidth * sizeof(unsigned short));
      }
    }
  }
  else  {
    int j_pos;
    unsigned short ui16;
    if (sizeX == offsetSizeX && sizeY == offsetSizeY) {
      for (j=0; j < offsetSizeY * offsetSizeX; j++) {
        ui16 = 0;          
        memcpy(&(ui16), buf + (j * symbolSizeInBytes), symbolSizeInBytes);
        imgX[j]= (unsigned short) ui16;
      }    
    }
    else {
      int iMinWidth   = iMin(sizeX, offsetSizeX);
      int iMinHeight  = iMin(sizeY, offsetSizeY);
      int destOffsetX = 0, destOffsetY = 0;
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
          imgX[(j + destOffsetY) * offsetSizeX + i + destOffsetX]= (unsigned short) ui16;
        }
      }    
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
