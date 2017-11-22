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
 * \file Output.H
 *
 * \brief
 *    Output master class header
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __Output_H__
#define __Output_H__

#include "Frame.H"
#include "Parameters.H"
#include "ImgToBuf.H"


class Output {
protected:  
  unsigned char *m_buf, *m_iBuf;
  vector<unsigned char>  m_buffer;
  vector<unsigned char>  m_iBuffer;
  void imageReformat       ( unsigned char* inp,  unsigned char* out,   FrameFormat *source, int symbolSizeInBytes);
  void imageReformatUInt16 ( unsigned char* buf, FrameFormat *source, int symbolSizeInBytes);
  uint32 (*mSetU16) (const char *& buf, uint16 value);

private:
  void reinterleaveYUV420 ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveYUV444 ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveYUYV   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveYVYU   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveUYVY   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveV210   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveV410   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveR10K   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveR210   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveB64A   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
#ifdef __SIM2_SUPPORT_ENABLED__
  void reinterleaveSim2          ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveSim2CoSited   ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveSim2F2        ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
  void reinterleaveSim2CoSitedF2 ( uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes );
#endif
  
public:
  bool              m_isFloat;
  bool              m_isInterleaved;
  bool              m_isInterlaced;
  int64             m_size;                         //!< number of samples
  int64             m_bufSize;                      ///!< buffer samples (may include alpha)
  int               m_compSize[4];                  //!< number of samples in specific component
  int               m_height[4];                    //!< height of a specific component in pixels
  int               m_width [4];                    //!< width of a specific component in pixels
  ColorSpace        m_colorSpace;                   //!< Color Space
  ColorPrimaries    m_colorPrimaries;               //!< Color primaries
  SampleRange       m_sampleRange;                  //!< Sample range
  ChromaFormat      m_chromaFormat;                 //!< 0: 420, 1:422, 2:444
  ChromaLocation    m_chromaLocation[FP_TOTAL];     //!< Chroma location type for subsampled ChromaFormats. 
                                                    //!< Note that we keep two elements for top and bottom fields.

  TransferFunctions m_transferFunction;             //!< output transfer function
  PixelFormat       m_pixelFormat;
  float             m_systemGamma;
  int               m_bitDepthComp[4];
  int32             m_pixelType[4];                 //!< pixel type (for OpenEXR)
  float             m_frameRate;
  int               m_picUnitSizeOnDisk;            //!< picture sample unit size on storage medium
  int               m_picUnitSizeShift3;            //!< m_picUnitSizeOnDisk >> 3


  vector<imgpel>    m_data;                         //!< data array for 8 bit data
  imgpel           *m_comp[4];                      //!< pointers to the different components of the data array
  
  vector<uint16>    m_ui16Data;                     //!< data array for 16 bit data
  uint16           *m_ui16Comp[4];                  //!< pointers to the different components of the data array
  
  vector<float>     m_floatData;                    //!< data array for floating point data
  float            *m_floatComp[4];                 //!< pointers to the different components of the data array

  ImgToBuf         *m_imgToBuf;
  
  // Sim2 parameters
  bool              m_cositedSampling;
  bool              m_improvedFilter;
  
  // EXR rounding
  bool              m_useFloatRound;


  Output ();
  virtual ~Output();
  static Output *create(IOVideo *inputFile, FrameFormat *output);
  void clear();

  virtual int writeOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) = 0;
  // copy, crop, center w/ black borders (aspect ratio and pixel values preserved; no filtering)
  void reInterleave        ( unsigned char** inp, unsigned char** outp, FrameFormat *source, int symbolSizeInBytes);
  void copyFrame(Frame *frm);
};


#endif
