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
 * \file Output.cpp
 *
 * \brief
 *    Output Class relating to I/O operations
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */


//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Output.H"
#include "OutputAVI.H"
#include "OutputEXR.H"
#include "OutputTIFF.H"
#include "OutputY4M.H"
#include "OutputYUV.H"
#include "Global.H"

#include <stdlib.h>
#include <string.h>

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
static uint32 setU16 (const char *&buf, uint16 in)
{
  char *pIn = (char *) &in;
  *((char *) buf++) = *pIn++;
  *((char *) buf++) = *pIn++;
  
  return 2;
}

/*!
 ************************************************************************
 * \brief
 *   Set an unsigned short and swap.
 *
 ************************************************************************
 */
static uint32 setSwappedU16 (const char *&buf, uint16 in)
{
  char *pIn = (char *) &in;
  *((char *) buf++) = pIn[1];
  *((char *) buf++) = pIn[0];
  
  return 2;
}


void Output::reinterleaveYUV420(
  uint8** input,         //!< input buffer
  uint8** output,        //!< output buffer
  FrameFormat *source,   //!< format of source buffer
  int symbolSizeInBytes  //!< number of bytes per symbol
  ) 
{
  int i;  
  // final buffer (interleaved)
  uint8 *ocmp  = *output;
  // original buffer (planar)
  uint8 *icmp0 = *input;
  uint8 *icmp1 = icmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *icmp2 = icmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    memcpy(ocmp, icmp1, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp1 += symbolSizeInBytes;
    memcpy(ocmp, icmp0, 2 * symbolSizeInBytes);
    ocmp  += 2 * symbolSizeInBytes;
    icmp0 += 2 * symbolSizeInBytes;
    memcpy(ocmp, icmp2, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp2 += symbolSizeInBytes;
    memcpy(ocmp, icmp0, 2 * symbolSizeInBytes);
    ocmp  += 2 * symbolSizeInBytes;
    icmp0 += 2 * symbolSizeInBytes;
  }

  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

void Output::reinterleaveYUV444(
  uint8** input,       //!< input buffer
  uint8** output,      //!< output buffer
  FrameFormat *format,         //!<  buffer format
  int symbolSizeInBytes     //!< number of bytes per symbol
  ) 
{
  int i;  
  // final buffer
  uint8 *ocmp  = *output;
  // original buffer
  uint8 *icmp0 = *input;

  uint8 *icmp1 = icmp0 + symbolSizeInBytes * format->m_compSize[Y_COMP];
  uint8 *icmp2 = icmp1 + symbolSizeInBytes * format->m_compSize[U_COMP];

  // reverse buffers if RGB format
  if (format->m_pixelFormat == PF_RGB) {
    icmp0 = icmp2;
    icmp2 = *input;
  }
  
  for (i = 0; i < format->m_compSize[Y_COMP]; i++) {
    memcpy(ocmp, icmp0, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp0 += symbolSizeInBytes;
    memcpy(ocmp, icmp1, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp1 += symbolSizeInBytes;
    memcpy(ocmp, icmp2, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp2 += symbolSizeInBytes;
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

void Output::reinterleaveYUYV (
  uint8** input,               //!< input buffer
  uint8** output,              //!< output buffer
  FrameFormat *source,         //!< format of source buffer
  int symbolSizeInBytes     //!< number of bytes per symbol
  ) 
{
  int i;  
  // final buffer
  uint8 *ocmp = *output;
  // original buffer
  uint8 *icmp0 = *input;

  uint8 *icmp1 = icmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *icmp2 = icmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    // Y
    memcpy(ocmp, icmp0, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp0 += symbolSizeInBytes;
    // U
    memcpy(ocmp, icmp1, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp1 += symbolSizeInBytes;
    // Y
    memcpy(ocmp, icmp0, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp0 += symbolSizeInBytes;
    // V
    memcpy(ocmp, icmp2, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp2 += symbolSizeInBytes;
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

void Output::reinterleaveYVYU (
  uint8** input,       //!< input buffer
  uint8** output,      //!< output buffer
  FrameFormat *source,         //!< format of source buffer
  int symbolSizeInBytes     //!< number of bytes per symbol
  ) 
{
  int i;  
  // final buffer
  uint8 *ocmp  = *output;
  // original buffer
  uint8 *icmp0 = *input;
  uint8 *icmp1 = icmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *icmp2 = icmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    // Y
    memcpy(ocmp, icmp0, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp0 += symbolSizeInBytes;
    // V
    memcpy(ocmp, icmp2, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp2 += symbolSizeInBytes;
    // Y
    memcpy(ocmp, icmp0, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp0 += symbolSizeInBytes;
    // U
    memcpy(ocmp, icmp1, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp1 += symbolSizeInBytes;
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

void Output::reinterleaveUYVY (
  uint8** input,         //!< input buffer
  uint8** output,        //!< output buffer
  FrameFormat *source,   //!< format of source buffer
  int symbolSizeInBytes  //!< number of bytes per symbol
  ) 
{
  int i;  
  // final buffer
  uint8 *ocmp  = *output;
  // original buffer
  uint8 *icmp0 = *input;
  uint8 *icmp1 = icmp0 + symbolSizeInBytes * source->m_compSize[Y_COMP];
  uint8 *icmp2 = icmp1 + symbolSizeInBytes * source->m_compSize[U_COMP];

  for (i = 0; i < source->m_compSize[U_COMP]; i++) {
    // U
    memcpy(ocmp, icmp1, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp1 += symbolSizeInBytes;
    // Y
    memcpy(ocmp, icmp0, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp0 += symbolSizeInBytes;
    // V
    memcpy(ocmp, icmp2, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp2 += symbolSizeInBytes;
    // Y
    memcpy(ocmp, icmp0, symbolSizeInBytes);
    ocmp  += symbolSizeInBytes;
    icmp0 += symbolSizeInBytes;
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}


void Output::reinterleaveV210(uint8** input,       //!< input buffer
                           uint8** output,      //!< output buffer
                           FrameFormat *source,         //!< format of source buffer
                           int symbolSizeInBytes     //!< number of bytes per symbol
)
{
  int i;
  // final buffer
  uint8 *ocmp  = NULL;
  // original buffer
  
  uint32  *ui32cmp = (uint32 *) *output;
  uint16 *ui16cmp0 = (uint16 *) *input;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i+= 6) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // Cr 0                Y 0                 Cb 0
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *(ui32cmp++)    = (((uint32) *ui16cmp2 & 0x3ff) << 20) | (((uint32) *ui16cmp0  & 0x3ff) << 10) | (((uint32) *ui16cmp1 & 0x3ff) );
    // Byte 7          Byte 6          Byte 5          Byte 4
    // Y 2                 Cb 1                Y 1
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *(ui32cmp++)    = (((uint32) *(ui16cmp0 + 2) & 0x3ff) << 20) | (((uint32) *(ui16cmp1 + 1) & 0x3ff) << 10) | (((uint32) *(ui16cmp0 + 1) & 0x3ff) );
    
    // Byte 11         Byte 10         Byte 9          Byte 8
    // Cb 2                Y 3                 Cr 1
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *(ui32cmp++)    = (((uint32) *(ui16cmp1 + 2) & 0x3ff) << 20) | (((uint32) *(ui16cmp0 + 3) & 0x3ff) << 10) | (((uint32) *(ui16cmp2 + 1) & 0x3ff));
    
    // Byte 15         Byte 14         Byte 13         Byte 12
    // Y 5                 Cr 2                Y 4
    // X X 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
    *(ui32cmp++)    = (((uint32) *(ui16cmp0 + 5) & 0x3ff) << 20) | (((uint32) *(ui16cmp2 + 2) & 0x3ff) << 10) | (((uint32) *(ui16cmp0 + 4)  & 0x3ff));
    
    ui16cmp0 += 6;
    ui16cmp1 += 3;
    ui16cmp2 += 3;
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

void Output::reinterleaveV410(uint8** input,       //!< input buffer
                              uint8** output,      //!< output buffer
                              FrameFormat *source,         //!< format of source buffer
                              int symbolSizeInBytes     //!< number of bytes per symbol
)
{
  int i;
  // final buffer
  uint8 *ocmp  = NULL;
  // original buffer
  
  uint32  *ui32cmp = (uint32 *) *output;
  uint16 *ui16cmp0 = (uint16 *) *input;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // Cr                  Y                  Cb 
    // 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 X X
    *(ui32cmp++)    = (((uint32) *(ui16cmp2++) & 0x3ff) << 22) | (((uint32) *(ui16cmp0++)  & 0x3ff) << 12) | (((uint32) *(ui16cmp1++) & 0x3ff) << 2 );
    
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

void Output::reinterleaveR10K(uint8** input,       //!< input buffer
                              uint8** output,      //!< output buffer
                              FrameFormat *source,         //!< format of source buffer
                              int symbolSizeInBytes     //!< number of bytes per symbol
)
{
  int i;
  // final buffer
  uint8 *ocmp  = NULL;
  // original buffer
  
  uint32  *ui32cmp = (uint32 *) *output;
  uint16 *ui16cmp0 = (uint16 *) *input;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // R                   G                   B 
    // 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 X X
    *(ui32cmp++)    = (((uint32) *(ui16cmp1++) & 0x3FF) | (((uint32) *(ui16cmp2++) & 0x3FF) << 10) | (((uint32) *(ui16cmp0++) & 0x3FF) << 20)) ;
/*
    *ui32cmp = 0;
        
    *(((uint8 *) ui32cmp) + 0)    = (uint8) (((*(ui16cmp1  ) & 0x0ff) >> 0)); 
    *(((uint8 *) ui32cmp) + 1)    = (uint8) (((*(ui16cmp1  ) & 0x300) >> 8)) | (uint8) (((*(ui16cmp2  ) & 0x3F) << 2)); 
    *(((uint8 *) ui32cmp) + 2)    = (uint8) (((*(ui16cmp2  ) & 0x3C0) >> 6)) | (uint8) (((*(ui16cmp0  ) & 0x0F) << 4)); 
    *(((uint8 *) ui32cmp) + 3)    = (uint8) (((*(ui16cmp0  ) & 0x3F0) >> 4)); 
    ui32cmp++;

    ui16cmp0++;
    ui16cmp1++;
    ui16cmp2++;
*/    
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

void Output::reinterleaveB64A(uint8** input,       //!< input buffer
                              uint8** output,      //!< output buffer
                              FrameFormat *source,         //!< format of source buffer
                              int symbolSizeInBytes     //!< number of bytes per symbol
)
{
  int i;
  // final buffer
  uint8 *ocmp  = NULL;
  // output buffer
  const char  *ui8cmp = (const char *) *output;

  uint16 *ui16cmp0 = (uint16 *) *input;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    *((char *)ui8cmp++) = 0;
    *((char *)ui8cmp++) = 0;            // A
    mSetU16(ui8cmp, *ui16cmp1++);       // R
    mSetU16(ui8cmp, *ui16cmp2++);       // G
    mSetU16(ui8cmp, *ui16cmp0++);       // B
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}


void Output::reinterleaveR210(uint8** input,       //!< input buffer
                              uint8** output,      //!< output buffer
                              FrameFormat *source,         //!< format of source buffer
                              int symbolSizeInBytes     //!< number of bytes per symbol
)
{
  int i;
  // final buffer
  uint8 *ocmp  = NULL;
  // original buffer
  
  uint32  *ui32cmp = (uint32 *) *output;
  uint16 *ui16cmp0 = (uint16 *) *input;
  uint16 *ui16cmp1 = ui16cmp0 + source->m_compSize[Y_COMP];
  uint16 *ui16cmp2 = ui16cmp1 + source->m_compSize[U_COMP];
  
  for (i = 0; i < source->m_compSize[Y_COMP]; i++) {
    // Byte 3          Byte 2          Byte 1          Byte 0
    // Blo             Glo         Bhi Rlo     Ghi     X X Rhi
    // 7 6 5 4 3 2 1 0 5 4 3 2 1 0 9 8 3 2 1 0 9 8 7 6 x x 9 8 7 6 5 4
    *(ui32cmp++)    = (((uint32) *(ui16cmp0) & 0x0ff) << 24) |  // Blo
                      (((uint32) *(ui16cmp2) & 0x03f) << 18) |  // Glo
                      (((uint32) *(ui16cmp0) & 0x300) <<  8) |  // Bhi
                      (((uint32) *(ui16cmp1) & 0x00F) << 12) |  // Rlo
                      (((uint32) *(ui16cmp2) & 0x3C0) <<  2) |  // Ghi
                      (((uint32) *(ui16cmp1) & 0x3F0) >>  4);   // Rhi
    ui16cmp0++;
    ui16cmp1++;
    ui16cmp2++;
  }
  // flip buffers
  ocmp    = *input;
  *input  = *output;
  *output = ocmp;
}

//-----------------------------------------------------------------------------
// Public methods
//-----------------------------------------------------------------------------
Output *Output::create(IOVideo *outputFile, FrameFormat *format) {
  Output *result = NULL;
  FrameFormat *source = &outputFile->m_format;

  switch (outputFile->m_videoType) {
  case VIDEO_YUV:
  case VIDEO_RGB:
    if (outputFile->m_isConcatenated == TRUE)
      result = new OutputYUV(outputFile, format);
    else {
      fprintf(stderr, "Only concatenated file types currently supported\n");
      exit(EXIT_FAILURE);
    }
    break;
    case VIDEO_Y4M:
      result = new OutputY4M(outputFile, format);
      break;
    case VIDEO_EXR:
      result = new OutputEXR(outputFile, format);
      break;
    case VIDEO_TIFF:
      result = new OutputTIFF(outputFile, format);
      break;
    case VIDEO_AVI:
      result = new OutputAVI(outputFile, format);
      break;
  default:
    fprintf(stderr, "Not supported format type %d\n", outputFile->m_videoType);
    exit(EXIT_FAILURE);
    break;
  }
  
  // I/O buffer to image mapping functions
  result->m_imgToBuf = ImgToBuf::create(source);

  return result;
}

Output::Output () {
  m_isFloat = FALSE;
  m_isInterleaved = FALSE;
  m_isInterlaced = FALSE;
  m_size = 0;
  m_compSize[0] = m_compSize[1] = m_compSize[2] = m_compSize[3] = 0;
  m_height[0] = m_height[1] = m_height[2] = m_height[3] = 0;
  m_width[0] = m_width[1] = m_width[2] = m_width[3] = 0;
  m_colorSpace = CM_YCbCr;
  m_colorPrimaries = CP_709;
  m_sampleRange = SR_STANDARD;
  m_chromaFormat = CF_420;
  m_pixelFormat = PF_UNKNOWN;
  m_bitDepthComp[0] = m_bitDepthComp[1] = m_bitDepthComp[2] = m_bitDepthComp[3] = 8;
  m_pixelType[0] = m_pixelType[1] = m_pixelType[2] = m_pixelType[3] = UINT;
  m_frameRate = 24.0;
  m_picUnitSizeOnDisk = 8;
  m_picUnitSizeShift3 = m_picUnitSizeOnDisk >> 3;
  m_comp[0] = m_comp[1] = m_comp[2] = m_comp[3] = NULL;
  m_ui16Comp[0] = m_ui16Comp[1] = m_ui16Comp[2] = m_ui16Comp[3] = NULL;
  m_floatComp[0] = m_floatComp[1] = m_floatComp[2] = m_floatComp[3] = NULL;
  m_imgToBuf  = NULL;
  m_cositedSampling = FALSE;
  m_improvedFilter  = FALSE;
  m_useFloatRound = FALSE;
  
  // Check endianess
  int endian = 1;
  int machineLittleEndian = (*( (char *)(&endian) ) == 1) ? 1 : 0;
  if (machineLittleEndian == 0)  { // endianness of machine matches b64a files
    mSetU16 = setU16;
  }
  else {                           // endianness of machine does not match b64a file
    mSetU16 = setSwappedU16;
  }

}

Output::~Output() {
  if(m_imgToBuf != NULL) {
    delete m_imgToBuf;
    m_imgToBuf = NULL;
  }
}

void Output::clear() {
}

// copy data from frame buffer to the output buffer
void Output::copyFrame(Frame *frm) {

  if (m_isFloat) {
    // Copying floating point data from the frame buffer to the output buffer
    m_floatData = frm->m_floatData;
  }
  else {
    if (frm->m_bitDepth == 8)
      m_data = frm->m_data;
    else
      m_ui16Data = frm->m_ui16Data;
  }
}


/*!
************************************************************************
* \brief
*    Reinterleave source picture structure to file write buffer
************************************************************************
*/
void Output::reInterleave ( uint8** input,         //!< input buffer
                            uint8** output,        //!< output buffer
                            FrameFormat *format,   //!< format of source buffer
                            int symbolSizeInBytes  //!< number of bytes per symbol
                          )
{
  switch (format->m_chromaFormat) {
    case CF_420:
      reinterleaveYUV420(input, output, format, symbolSizeInBytes);
      break;
    case CF_422:
      switch (format->m_pixelFormat) {
        case PF_YUYV:
        case PF_YUY2:
          reinterleaveYUYV(input, output, format, symbolSizeInBytes);
          break;
        case PF_YVYU:
          reinterleaveYVYU(input, output, format, symbolSizeInBytes);
          break;
        case PF_UYVY:
          reinterleaveUYVY(input, output, format, symbolSizeInBytes);
          break;
        case PF_V210:
          reinterleaveV210(input, output, format, symbolSizeInBytes);
          break;
        default:
          fprintf(stderr, "Unsupported pixel format.\n");
          exit(EXIT_FAILURE);
          break;
      }
      break;
    case CF_444:
      if (format->m_pixelFormat == PF_V410) {
        reinterleaveV410(input, output, format, symbolSizeInBytes);
      }
      else if (format->m_pixelFormat == PF_B64A) {
        reinterleaveB64A(input, output, format, symbolSizeInBytes);
      }
      else if (format->m_pixelFormat == PF_R10K) {
        reinterleaveR10K(input, output, format, symbolSizeInBytes);
      }
      else if (format->m_pixelFormat == PF_R210) {
        reinterleaveR210(input, output, format, symbolSizeInBytes);
      }
      else
#ifdef __SIM2_SUPPORT_ENABLED__
      if (format->m_pixelFormat == PF_SIM2) {
        if (format->m_cositedSampling) {
          if (format->m_improvedFilter)
            reinterleaveSim2CoSitedF2( input, output, format, symbolSizeInBytes );
          else
            reinterleaveSim2CoSited( input, output, format, symbolSizeInBytes );
        }
        else {
          if (format->m_improvedFilter)
            reinterleaveSim2F2 ( input, output, format, symbolSizeInBytes );
          else
            reinterleaveSim2 ( input, output, format, symbolSizeInBytes );
        }
      }
      else
#endif
      {
        reinterleaveYUV444(input, output, format, symbolSizeInBytes);
      }
      break;
    default:
      fprintf(stderr, "Unknown Chroma Format type %d\n", format->m_chromaFormat);
      exit(EXIT_FAILURE);
      break;
  }
}

/*!
************************************************************************
* \brief
*    Reinterleave file read buffer to source picture structure
************************************************************************
*/
void Output::imageReformat ( uint8* buf,           //!< input buffer
                             uint8* out,           //!< output buffer
                             FrameFormat *format,  //!< format of source buffer
                             int symbolSizeInBytes //!< number of bytes per symbol
                           ) 
{
  bool rgb_input = (bool) (m_colorSpace == CM_RGB && m_chromaFormat == CF_444);
  const int bytesY  = m_compSize[Y_COMP] * symbolSizeInBytes;
  const int bytesUV = m_compSize[U_COMP] * symbolSizeInBytes;

  if(rgb_input) {
    if (format->m_pixelFormat == PF_RGB  && m_isInterleaved == FALSE) {
      m_imgToBuf->process(m_comp[U_COMP], buf + bytesY, m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes);
    }
    else {
      m_imgToBuf->process(m_comp[Y_COMP], buf + bytesY, m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes);
    }
  }
  else
    m_imgToBuf->process(m_comp[Y_COMP], buf, m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes);

  if (m_chromaFormat != CF_400) {
    if(rgb_input) {
      if (format->m_pixelFormat == PF_RGB  && m_isInterleaved == FALSE) {
        m_imgToBuf->process(m_comp[V_COMP], buf + bytesY + bytesUV, m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes);
        m_imgToBuf->process(m_comp[Y_COMP], buf, m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes);
      }
      else {
        m_imgToBuf->process(m_comp[U_COMP], buf + bytesY + bytesUV, m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes);
        m_imgToBuf->process(m_comp[V_COMP], buf, m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes);
      }
    }
    else {
      m_imgToBuf->process(m_comp[U_COMP], buf + bytesY, m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes);
      m_imgToBuf->process(m_comp[V_COMP], buf + bytesY + bytesUV, m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes);
    }
  }
}

/*!
************************************************************************
* \brief
*    Reinterleave file read buffer to source picture structure
************************************************************************
*/
void Output::imageReformatUInt16 (
  uint8* buf,                //!< input buffer
  FrameFormat *format,       //!< format of source buffer
  int symbolSizeInBytes   //!< number of bytes per symbol
  ) 
{
  // Reinterleave data
  bool rgb_input = (bool) (m_colorSpace == CM_RGB && m_chromaFormat == CF_444);
  const int bytesY  = m_compSize[Y_COMP] * symbolSizeInBytes;
  const int bytesUV = m_compSize[U_COMP] * symbolSizeInBytes;
  
  if(rgb_input) {
    if (format->m_pixelFormat == PF_RGB  && m_isInterleaved == FALSE) {
      m_imgToBuf->process(m_ui16Comp[U_COMP], buf + bytesY, m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes);
    }
    else {
      m_imgToBuf->process(m_ui16Comp[Y_COMP], buf + bytesY, m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes);
    }
  }
  else
    m_imgToBuf->process(m_ui16Comp[Y_COMP], buf, m_width[Y_COMP], m_height[Y_COMP], symbolSizeInBytes);
  
  if (m_chromaFormat != CF_400) {
    if(rgb_input) {
      if (format->m_pixelFormat == PF_RGB  && m_isInterleaved == FALSE) {
        m_imgToBuf->process(m_ui16Comp[V_COMP], buf + bytesY + bytesUV, m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes);
        m_imgToBuf->process(m_ui16Comp[Y_COMP], buf, m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes);
      }
      else {
        m_imgToBuf->process(m_ui16Comp[U_COMP], buf + bytesY + bytesUV, m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes);
        m_imgToBuf->process(m_ui16Comp[V_COMP], buf, m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes);
      }
    }
    else {
      m_imgToBuf->process(m_ui16Comp[U_COMP], buf + bytesY, m_width[U_COMP], m_height[U_COMP], symbolSizeInBytes);
      m_imgToBuf->process(m_ui16Comp[V_COMP], buf + bytesY + bytesUV, m_width[V_COMP], m_height[V_COMP], symbolSizeInBytes);
    }
  }
}


//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
