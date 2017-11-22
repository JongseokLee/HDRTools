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
 * \file ImgToBufBasic.cpp
 *
 * \brief
 *    ImgToBufBasic Class to manipulate different I/O formats
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
#include "ImgToBufBasic.H"
#include <string.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor/destructor
//-----------------------------------------------------------------------------

ImgToBufBasic::ImgToBufBasic() {
}

ImgToBufBasic::~ImgToBufBasic() {
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------


void ImgToBufBasic::process ( uint8*  imgX,            //!< Pointer to image plane
                             unsigned char* buf,       //!< Buffer for file output
                             int sizeX,                //!< horizontal size of picture
                             int sizeY,                //!< vertical size of picture
                             int symbolSizeInBytes     //!< number of bytes in file used for one pixel
)
{
  // imgpel == pixel_in_file -> simple copy
  memcpy(buf, imgX, sizeY * sizeX * sizeof(imgpel));
}

void ImgToBufBasic::process ( uint16* imgX,             //!< Pointer to image plane
                              uint8* buf,               //!< Buffer for file output
                              int sizeX,                //!< horizontal size of picture
                              int sizeY,                //!< vertical size of picture
                              int symbolSizeInBytes     //!< number of bytes in file used for one pixel
)
{
  if (( sizeof (unsigned short) == symbolSizeInBytes)) {
    // unsigned short == pixel_in_file -> simple copy
    memcpy(buf, imgX, sizeY * sizeX * sizeof(unsigned short));
  }
  else  {
    int j;
    for (j=0; j < sizeY * sizeX; j++) {
      memcpy(buf + (j * symbolSizeInBytes), &(imgX[j]), symbolSizeInBytes);
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
