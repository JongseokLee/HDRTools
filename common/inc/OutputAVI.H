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
 * \file OutputAVI.H
 *
 * \brief
 *    OutputAVI class header file for AVI files
 *    Code is loosely based on avilib by Rainer Johanni <Rainer@Johanni.de>
 *    and the extensions by Thomas ÷streich. Therefore their copyright is also added
 *    here:
 *
 *    Copyright (C) Thomas ÷streich - June 2001
 *    multiple audio track support Copyright (C) 2002 Thomas ÷streich
 *
 *    Original code:
 *    Copyright (C) 1999 Rainer Johanni <Rainer@Johanni.de>
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __OutputAVI_H__
#define __OutputAVI_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------

#include "Output.H"
#include "AVILib.H"

#define HEADERBYTES 7684
#define AVI_MAX_LEN (UINT_MAX-(1<<20)*16-HEADERBYTES)

#define FRAME_RATE_SCALE 1000000

/* The Flags in AVI File header */
#define AVIF_HASINDEX           0x00000010      /* Index at end of file */
#define AVIF_MUSTUSEINDEX       0x00000020
#define AVIF_ISINTERLEAVED      0x00000100
#define AVIF_TRUSTCKTYPE        0x00000800      /* Use CKType to find key frames */
#define AVIF_WASCAPTUREFILE     0x00010000
#define AVIF_COPYRIGHTED        0x00020000


//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

class OutputAVI : public Output {
private:
  int64          m_errorNumber;
  IOVideo       *m_outputFile;
  AVIFile       *m_avi;
  byte          *m_header;
  FrameFormat    m_format;
  int            m_headerBytes;
  
  int     writeData (int vfile, uint8 *buf);
  int     writeData (int vfile, int framesizeInBytes, uint8 *buf);
  int64   getFrameSizeInBytes(FrameFormat *source, bool isInterleaved);
  int64   aviWrite (int fd, char *buffer, int64 len);
  int     updateHeader(FrameFormat *format);
  void    out4CC   (char *dest, const char *source, int64 *length);
  void    outLong  (char *dest, int32 value, int64 *length);
  void    outShort (char *dest, int32 value, int64 *length);
  void    outChar  (char *dest, int32 value, int64 *length);
  void    outMemory(char *dest, void *source, int64 *length, uint32 size);
  void    int32ToChar(char *string, int32 value);
  int     sampleSize(int j);
  int     writeData(char *data, unsigned long length, int audio, int keyframe);
  int     addChunk(unsigned char *tag, unsigned char *data, int length);
  int     addIndexEntry(unsigned char *tag, long flags, unsigned long pos, unsigned long len);
  int     addOdmlIndexEntry(unsigned char *tag, long flags, int64 pos, unsigned long len);
  int     addOdmlIndexEntryCore(long flags, int64_t pos, unsigned long len, AVIStandardIndexChunk *si);
  int     initSuperIndex(unsigned char *idxtag, AVISuperIndexChunk **si);
  int     addStandardIndex(unsigned char *idxtag, unsigned char *strtag, AVIStandardIndexChunk *stdil);
  int     ixnnEntry(AVIStandardIndexChunk *ch, AVISuperIndexEntry *en);
  
  int     closeOutputFile(FrameFormat *format);
  int     close(FrameFormat *format);
  
  void    reFormat(uint8** , uint8** , FrameFormat *, int  );

#ifdef __SIM2_SUPPORT_ENABLED__
  void    formatSim2(uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes);
  void    formatSim2CoSited(uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes);
  void    formatSim2F2(uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes);
  void    formatSim2CoSitedF2(uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes);
#endif

  void    formatRGB8(uint8** input, uint8** output, FrameFormat *source, int symbolSizeInBytes);

public:
  OutputAVI(IOVideo *inputFile, FrameFormat *format);
  virtual ~OutputAVI();
  virtual int writeOneFrame (IOVideo *inputFile, int frameNumber, int fileHeader, int frameSkip);
};

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
