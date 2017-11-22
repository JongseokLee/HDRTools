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
 * \file Input.H
 *
 * \brief
 *    Input master class header 
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __Input_H__
#define __Input_H__

#include "Frame.H"
#include "Parameters.H"
#include "BufToImg.H"


class Input {
protected:  
  unsigned char *m_buf, *m_iBuf;
  vector<unsigned char>  m_buffer;
  vector<unsigned char>  m_iBuffer;

  void deInterleave        ( unsigned char** inp, unsigned char** outp, FrameFormat *source, int symbolSizeInBytes);
  void imageReformat       ( unsigned char* inp,  unsigned char* out,   FrameFormat *source, int symbolSizeInBytes);
  void imageReformatUInt16 ( unsigned char* buf, FrameFormat *source, int symbolSizeInBytes);
 
private:
  // de-interleaving functions for a variety of input files
  void deInterleaveYUV420 (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveYUV444 (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveYUYV   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveYVYU   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveUYVY   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveV210   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveV410   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveR10K   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveR210   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveB64A   (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  void deInterleaveUYVY10 (uint8** input, uint8** output, FrameFormat *source, int symbol_size_in_bytes);
  uint16 (*mGetU16)       (const char *&);


public:
  bool              m_isFloat;                      //!< Floating point input
  bool              m_isInterleaved;                //!< Interleaced input
  bool              m_isInterlaced;                 //!< Interlaced input
  int               m_components;
  int64             m_size;                         //!< number of samples
  int64             m_bufSize;                      //!< buffer size
  int               m_compSize[4];                  //!< number of samples in specific component
  int               m_height[4];                    //!< height of a specific component in pixels
  int               m_width [4];                    //!< width of a specific component in pixels
  ColorSpace        m_colorSpace;                   //!< Color space
  ColorPrimaries    m_colorPrimaries;               //!< Color primaries
  ChromaFormat      m_chromaFormat;                 //!< 0: 420, 1:422, 2:444
  ChromaLocation    m_chromaLocation[FP_TOTAL];     //!< Chroma location type for subsampled ChromaFormats. 
                                                    //!< Note that we keep two elements for top and bottom fields.
  SampleRange       m_sampleRange;                  //!< 0: standard, 1: full, 2: SDI
  TransferFunctions m_transferFunction;             //!< Supported transfer functions
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
  
  BufToImg         *m_bufToImg;
  
  Input ();
  virtual      ~Input();
  static Input *create       (IOVideo *inputFile, FrameFormat *output, Parameters *inputParams);
  virtual int   readOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip) = 0;
  void          copyFrame    (Frame *frm);
  void          clear        ();
};


#endif
