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
 * \file AVILib.H
 *
 * \brief
 *    AVILib header file
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __AVILib_H__
#define __AVILib_H__

//-----------------------------------------------------------------------------
// Include headers
//-----------------------------------------------------------------------------
#include <string.h>

//-----------------------------------------------------------------------------
// Class definition
//-----------------------------------------------------------------------------

#define AVI_MAX_TRACKS 8

typedef struct
{
  int64 key;
  int64 pos;
  int64 len;
} video_index_entry;

typedef struct
{
  int64 pos;
  int64 len;
  int64 tot;
} audio_index_entry;


// Index types
#define AVI_INDEX_OF_INDEXES 0x00             // when each entry in aIndex
                                              // array points to an index chunk
#define AVI_INDEX_OF_CHUNKS  0x01             // when each entry in aIndex
                                              // array points to a chunk in the file
#define AVI_INDEX_IS_DATA    0x80             // when each entry is aIndex is
                                              // really the data
                                              // bIndexSubtype codes for INDEX_OF_CHUNKS
                                              //
#define AVI_INDEX_2FIELD     0x01             // when fields within frames
                                              // are also indexed

enum AVIModes {
  AVI_MODE_WRITE = 0,
  AVI_MODE_READ  = 1
};

enum AVIErrorCodes {
  AVI_ERR_SIZELIM     = 1,   // Warning - Writing exceeds the maximum size of the AVI file. File may have been closed safely
  AVI_ERR_OPEN        = 2,   // Error opening the AVI file; wrong path name or file nor readable/writable
  AVI_ERR_READ        = 3,   // Error reading from AVI File
  AVI_ERR_WRITE       = 4,   // Error writing to AVI File; disk full or inaccessible
  AVI_ERR_WRITE_INDEX = 5,   // Could not write index to AVI file during close; file may still be usable
  AVI_ERR_CLOSE       = 6,   // Could not write header to AVI file or truncate the file during close; file is most probably corrupted
  AVI_ERR_NOT_PERM    = 7,   // Operation not permitted: trying to read from a file open for writing or vice versa
  AVI_ERR_NO_MEM      = 8,   // Memory allocation failed
  AVI_ERR_NO_AVI      = 9,   // Not an AVI file
  AVI_ERR_NO_HDRL     = 10,  // AVI file does not have a header list; file possibly corrupted
  AVI_ERR_NO_MOVI     = 11,  // AVI file does not have a MOVI list; file possibly corrupted
  AVI_ERR_NO_VIDS     = 12,  // AVI file does not contain any video data
  AVI_ERR_NO_IDX      = 13   // The file has been opened with getIndex==0, but an operation has been performed that needs an index
};

/* Possible Audio formats */

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_UNKNOWN             (0x0000)
#define WAVE_FORMAT_PCM                 (0x0001)
#define WAVE_FORMAT_ADPCM               (0x0002)
#define WAVE_FORMAT_IBM_CVSD            (0x0005)
#define WAVE_FORMAT_ALAW                (0x0006)
#define WAVE_FORMAT_MULAW               (0x0007)
#define WAVE_FORMAT_OKI_ADPCM           (0x0010)
#define WAVE_FORMAT_DVI_ADPCM           (0x0011)
#define WAVE_FORMAT_DIGISTD             (0x0015)
#define WAVE_FORMAT_DIGIFIX             (0x0016)
#define WAVE_FORMAT_YAMAHA_ADPCM        (0x0020)
#define WAVE_FORMAT_DSP_TRUESPEECH      (0x0022)
#define WAVE_FORMAT_GSM610              (0x0031)
#define IBM_FORMAT_MULAW                (0x0101)
#define IBM_FORMAT_ALAW                 (0x0102)
#define IBM_FORMAT_ADPCM                (0x0103)
#endif

// Maximum number of indices per stream
const int NR_IXNN_CHUNKS = 32;

const int NEW_RIFF_THRES = (1900*1024*1024);

typedef struct _AVISuperIndexEntry {
  uint64 qwOffset;           // absolute file offset
  uint32 dwSize;             // size of index chunk at this offset
  uint32 dwDuration;         // time span in stream ticks
} AVISuperIndexEntry;

typedef struct _AVIStandardIndexEntry {
  uint32 dwOffset;                // qwBaseOffset + this is absolute file offset
  uint32 dwSize;                  // bit 31 is set if this is NOT a keyframe
} AVIStandardIndexEntry;

// Standard index
class AVIStandardIndexChunk {
public:
  char    fcc[4];                 // ix##
  uint32  dwSize;                 // size of this chunk
  uint16  wLongsPerEntry;         // must be sizeof(aIndex[0])/sizeof(DWORD)
  uint8   bIndexSubType;          // must be 0
  uint8   bIndexType;             // must be AVI_INDEX_OF_CHUNKS
  uint32  nEntriesInUse;          //
  char    dwChunkId[4];           // '##dc' or '##db' or '##wb' etc..
  uint64  qwBaseOffset;       // all dwOffsets in aIndex array are relative to this
  uint32  dwReserved3;            // must be 0
  AVIStandardIndexEntry *m_aIndex;

  AVIStandardIndexChunk () {
    for (int index = 0; index < 4; index++) {
      fcc[index] = (char) 0;
      dwChunkId[index] = (char) 0;
    }
    dwSize = 0;                 // size of this chunk
    wLongsPerEntry = 0;         // must be sizeof(aIndex[0])/sizeof(DWORD)
    bIndexSubType = 0;          // must be 0
    bIndexType = 0;             // must be AVI_INDEX_OF_CHUNKS
    nEntriesInUse = 0;          //
    qwBaseOffset = 0;       // all dwOffsets in aIndex array are relative to this
    dwReserved3 = 0;            // must be 0
    m_aIndex = NULL;
  }
  ~AVIStandardIndexChunk () {
    if (m_aIndex != NULL) {
      delete [] m_aIndex;
      m_aIndex = NULL;
    }
  }
};


// Base Index Form 'indx'
class AVISuperIndexChunk {
public:
  char    fcc[4];
  uint32  dwSize;                 // size of this chunk
  uint16  wLongsPerEntry;         // size of each entry in aIndex array (must be 8 for us)
  uint8   bIndexSubType;          // future use. must be 0
  uint8   bIndexType;             // one of AVI_INDEX_* codes
  uint32  nEntriesInUse;          // index of first unused member in aIndex array
  char    dwChunkId[4];           // fcc of what is indexed
  uint32  dwReserved[3];          // meaning differs for each index type/subtype.
                                  // 0 if unused
  AVISuperIndexEntry *aIndex;           // where are the ix## chunks
  AVIStandardIndexChunk **stdindex;          // the ix## chunks itself (array)
  
  AVISuperIndexChunk(unsigned char *idxtag) {
    memcpy (fcc, "indx", 4);
    
    dwSize = 0; // size of this chunk
    wLongsPerEntry = 4;
    bIndexSubType = 0;
    bIndexType = AVI_INDEX_OF_INDEXES;
    nEntriesInUse = 0; // none are in use
    memcpy (dwChunkId, idxtag, 4);
    memset (dwReserved, 0, sizeof (dwReserved));
    
    // NR_IXNN_CHUNKS == allow 32 indices which means 32 GB files -- arbitrary
    aIndex = new AVISuperIndexEntry[wLongsPerEntry * NR_IXNN_CHUNKS];
    memset (aIndex, 0, wLongsPerEntry * NR_IXNN_CHUNKS * sizeof (AVISuperIndexEntry));
    stdindex = NULL;
    /*
    stdindex = new  AVIStandardIndexChunk*[NR_IXNN_CHUNKS];
    if (!sil->stdindex) {
      m_errorNumber = AVI_ERR_NO_MEM;
      return -1;
    }
    for (k = 0; k < NR_IXNN_CHUNKS; k++) {
      sil->stdindex[k] = malloc (sizeof (AVIStandardIndexChunk));
      // gets rewritten later
      sil->stdindex[k]->qwBaseOffset = (uint64_t)k * NEW_RIFF_THRES;
    }
     */
  }
  ~AVISuperIndexChunk() {
    if (aIndex != NULL) {
      delete [] aIndex;
      aIndex = NULL;
    }
    if (stdindex != NULL) {
      for (int index = 0; index < NR_IXNN_CHUNKS; index++) {
        if (stdindex[index] != NULL) {
          delete stdindex[index];
          stdindex[index] = NULL;
        }
      }
      delete [] stdindex;
      stdindex = NULL;
    }
  }
};


class AVItrack {
public:
  long   a_fmt;             /* Audio format, see #defines below */
  long   a_chans;           /* Audio channels, 0 for no audio */
  long   a_rate;            /* Rate in Hz */
  long   a_bits;            /* bits per audio sample */
  long   mp3rate;           /* mp3 bitrate kbs*/
  long   a_vbr;             /* 0 == no Variable BitRate */
  long   padrate;	      /* byte rate used for zero padding */
  
  long   audioStreamNo;        /* Audio stream number */
  int64  audioBytes;       /* Total number of bytes of audio data */
  long   audioChunks;      /* Chunks of audio data in the file */
  
  char   audioTag[4];      /* Tag of audio data */
  long   audioPositionChunk;        /* Audio position: chunk */
  long   audioPositionByte;        /* Audio position: byte within chunk */
  
  int64  a_codech_off;       /* absolut offset of audio codec information */
  int64  a_codecf_off;       /* absolut offset of audio codec information */
  
  audio_index_entry *m_audioIndex;
  AVISuperIndexChunk *m_audioSuperIndex;
  
  AVItrack () {
    a_fmt = 0;             /* Audio format, see #defines below */
    a_chans = 0;           /* Audio channels, 0 for no audio */
    a_rate = 0;            /* Rate in Hz */
    a_bits = 0;            /* bits per audio sample */
    mp3rate = 0;           /* mp3 bitrate kbs*/
    a_vbr = 0;             /* 0 == no Variable BitRate */
    padrate = 0;	      /* byte rate used for zero padding */
    
    audioStreamNo = 0;        /* Audio stream number */
    audioBytes = 0;       /* Total number of bytes of audio data */
    audioChunks = 0;      /* Chunks of audio data in the file */
    
    for (int index = 0; index < 4; index++) {
      audioTag[index] = 0;      /* Tag of audio data */
    }
    
    audioPositionChunk = 0;        /* Audio position: chunk */
    audioPositionByte = 0;        /* Audio position: byte within chunk */
    
    a_codech_off = 0;       /* absolut offset of audio codec information */
    a_codecf_off = 0;       /* absolut offset of audio codec information */
    
    m_audioIndex = NULL;
    m_audioSuperIndex = NULL;
  }
};

typedef struct
{
  uint32  bitmapSize;
  uint32  bitmapWidth;
  uint32  bitmapHeight;
  uint16  bitmapPlanes;
  uint16  bitmapBitCount;
  uint32  bitmapCompression;
  uint32  bitmapSizeImage;
  uint32  bitmapXPixelsPerMeter;
  uint32  bitmapYPixelsPerMeter;
  uint32  bitmapClrUsed;
  uint32  bitmapClrImportant;
} alBITMAPINFOHEADER;

typedef struct 
{
  uint16  w_format_tag;
  uint16  n_channels;
  uint32  n_samples_per_sec;
  uint32  n_avg_bytes_per_sec;
  uint16  n_block_align;
  uint16  w_bits_per_sample;
  uint16  cb_size;
} alWAVEFORMATEX;


class AVIMainHeader {
public:
  uint32 m_dwMicroSecPerFrame;
  uint32 m_dwMaxBytesPerSec;
  uint32 m_dwPaddingGranularity;
  
  uint32 m_dwFlags;
  uint32 m_dwTotalFrames;
  uint32 m_dwInitialFrames;
  uint32 m_dwStreams;
  uint32 m_dwSuggestedBufferSize;
  
  uint32 m_dwWidth;
  uint32 m_dwHeight;
  
  uint32 dwReserved[4];
  
  AVIMainHeader () {
    m_dwMicroSecPerFrame = 0;
    m_dwMaxBytesPerSec = 0;
    m_dwPaddingGranularity = 0;
    
    m_dwFlags = 0;
    
    m_dwTotalFrames = 0;
    m_dwInitialFrames = 0;
    m_dwStreams = 0;
    m_dwSuggestedBufferSize = 0;
    
    m_dwWidth = 0;
    m_dwHeight = 0;
    
    for (int index = 0; index < 4; index++) {
      dwReserved[index] = 0;
    }
  }
};

typedef struct {
  uint8   m_indexEntry[16];
} aviIndexEntries;


// 2.1.3 The stream heder list element" strh
class AVIStreamHeader {
public: 
  uint32 m_fccType;
  uint32 m_fccHandler;

  uint32 m_dwFlags;
  
  uint32 m_dwCaps; // Not in stream header list
  uint16 m_wPriority;
  uint16 m_wLanguage;
  uint32 m_dwInitialFrames;

  uint32 m_dwScale;
  uint32 m_dwRate;     // m_dwRate/m_dwScale == samples/second
  uint32 m_dwStart;
  uint32 m_dwLength;   // in units above

  uint32 m_dwSuggestedBufferSize;
  uint32 m_dwQuality;
  uint32 m_dwSampleSize;
  
  // rectangle coordinates (rcFrame)
  uint16 m_rcFrameLeft;
  uint16 m_rcFrameTop;
  uint16 m_rcFrameRight;
  uint16 m_rcFrameBottom;
  
  AVIStreamHeader () {
    m_fccType = 0;
    m_fccHandler = 0;
    
    m_dwFlags = 0;
    //    m_dwCaps = 0;
    m_wPriority = 0;
    m_wLanguage = 0;
    m_dwInitialFrames = 0;

    m_dwScale = 0;
    m_dwRate = 0;
    m_dwStart = 0;
    m_dwLength = 0;

    m_dwSuggestedBufferSize = 0;
    
    m_dwQuality = 0;
    m_dwSampleSize = 0;
    m_rcFrameLeft = 0;
    m_rcFrameTop = 0;
    m_rcFrameRight = 0;
    m_rcFrameBottom = 0;
  }
};

class AVIFile {
public:
  
  int     m_fileNum;           /* File descriptor of AVI file */
  int     m_mode;              /* 0 for reading, 1 for writing */
  int     m_width;             /* Width  of a video frame */
  int     m_height;            /* Height of a video frame */
  double  m_frameRate;         /* Frame rate*/
  char    m_compressor [8];    /* Type of compressor, 4 bytes + padding for 0 byte */
  char    m_compressor2[8];    /* Type of compressor, 4 bytes + padding for 0 byte */
  long    m_videoStreamNo;     /* Video stream number */
  long    m_videoFrames;       /* Number of video frames */
  char    m_videoTag[4];       /* Tag of video data */
  long    m_videoPosition;     /* Number of next frame to be read (if index present) */
  
  AVIStreamHeader m_videoStreamHeader;
  
  uint32   m_maxLength;    /* maximum video chunk present */
  AVItrack m_track[AVI_MAX_TRACKS];  // up to AVI_MAX_TRACKS audio tracks supported
  
  int64    m_pos;                 /* position in file */
  long     m_noIndex;             /* number of index entries actually filled */
  long     m_maxIndex;            /* number of index entries actually allocated */
  
  int64    m_videoCodecStrhOffset;      /* absolute offset of video codec (strh) info */
  int64    m_videoCodecStrfOffset;      /* absolute offset of video codec (strf) info */
  
  aviIndexEntries *m_index;
  
  video_index_entry *m_videoIndex;
  AVISuperIndexChunk *m_videoSuperIndex;  /* index of indices */
  int      m_isOpenDML;           /* set to 1 if this is an odml file with multiple index chunks */
  
  int64    m_lastPosition;          /* Position of last frame written */
  uint32   m_lastLength;   /* Length of last frame written */
  int      m_mustUseIndex;       /* Flag if frames are duplicated */
  int64    m_moviStart;
  int      m_totalFrames;         /* total number of frames if dmlh is present */
  
  int      m_numberAudioTracks;            // total number of audio tracks
  int      m_currentAudioTrack;            // current audio working track
  int      m_commentFilePos;       // Read avi header comments from this position
  char     *m_indexFile;    // read the avi index from this file
  
  alBITMAPINFOHEADER *m_bitmapInfoHeader;
  alWAVEFORMATEX     *m_waveFormatEx[AVI_MAX_TRACKS];
  AVIStreamHeader     m_streamHeaders[AVI_MAX_TRACKS];
  
  void         *m_extraData;
  unsigned long	m_extraDataSize;

  AVIFile () {
    m_fileNum = -1;         /* File descriptor of AVI file */
    m_mode = 0;             /* 0 for reading, 1 for writing */
    
    m_width = 0;             /* Width  of a video frame */
    m_height = 0;            /* Height of a video frame */
    m_frameRate = 0.0;               /* Frames per second */
    for (int index = 0; index < 8; index++) {
      m_compressor[index] = 0;
      m_compressor2[index] = 0;
    }
    m_videoStreamNo = 0;
    m_videoFrames = 0;
    for (int index = 0; index < 4; index++) {
      m_videoTag[index] = 0;
    }
    m_videoPosition = 0;
    m_maxLength = 0;
    
    m_pos = 0;
    m_noIndex = 0;
    m_maxIndex = 0;
    
    m_videoCodecStrhOffset = 0;
    m_videoCodecStrfOffset = 0;
    
    m_index = NULL;
    
    m_videoIndex = NULL;
    m_videoSuperIndex = NULL;
    m_isOpenDML = 0;
    m_lastPosition = 0;
    m_lastLength = 0;
    m_mustUseIndex = 0;
    m_moviStart = 0;
    m_totalFrames = 0;
    
    m_numberAudioTracks = 0;
    m_currentAudioTrack = 0;
    m_commentFilePos = 0;
    m_indexFile = NULL;
    m_bitmapInfoHeader = NULL;

    for (int index = 0; index < AVI_MAX_TRACKS; index++) {
      m_waveFormatEx[index] = NULL;
    }
    
    m_extraData = NULL;
    m_extraDataSize = 0;
  }
  
  ~AVIFile () {
    if (m_commentFilePos>0)
      close(m_commentFilePos);
    m_commentFilePos = -1;
    if (m_fileNum != -1)
      close(m_fileNum);
    m_fileNum = -1;
    
    if (m_index != NULL) {
      free(m_index);
      m_index = NULL;
    }
    
    if(m_videoIndex != NULL) {
      delete [] m_videoIndex;
      m_videoIndex = NULL;
    }
    if(m_videoSuperIndex != NULL) {
      delete m_videoSuperIndex;
      m_videoSuperIndex = NULL;
    }
    
    for (int j=0; j < m_numberAudioTracks; j++) {
      if(m_track[j].m_audioIndex  != NULL) {
        delete [] m_track[j].m_audioIndex;
        m_track[j].m_audioIndex = NULL;
      }
      if(m_track[j].m_audioSuperIndex != NULL) {
        if(m_track[j].m_audioSuperIndex->aIndex != NULL) {
          delete [] m_track[j].m_audioSuperIndex->aIndex;
          m_track[j].m_audioSuperIndex->aIndex = NULL;
        }
        delete [] m_track[j].m_audioSuperIndex;
        m_track[j].m_audioSuperIndex = NULL;
      }
    }
    
    if (m_bitmapInfoHeader) {
      delete [] m_bitmapInfoHeader;
      m_bitmapInfoHeader = NULL;
    }
    for (int j = 0; j < AVI_MAX_TRACKS; j++) {
      if (m_waveFormatEx[j]) {
        delete m_waveFormatEx[j];
        m_waveFormatEx[j] = NULL;
      }
    }
  }
};


AVIFile* AVI_open_output_file(char * filename);
void AVI_set_video(AVIFile *AVI, int width, int height, double fps, char *compressor);
int  AVI_write_frame(AVIFile *AVI, char *data, long bytes, int keyframe);
int  AVI_dup_frame(AVIFile *AVI);
long AVI_bytes_remain(AVIFile *AVI);
int  AVI_close(AVIFile *AVI);
long AVI_bytes_written(AVIFile *AVI);

AVIFile *AVI_open_input_file(char *filename, int getIndex);
AVIFile *AVI_open_input_indexfile(char *filename, int getIndex, char *indexfile);
AVIFile *AVI_open_fd(int fd, int getIndex);
AVIFile *AVI_open_indexfd(int fd, int getIndex, char *indexfile);
int avi_parse_input_file(AVIFile *AVI, int getIndex);
int avi_parse_index_from_file(AVIFile *AVI, char *filename);

long AVI_videoFrames(AVIFile *AVI);
int  AVI_video_width(AVIFile *AVI);
int  AVIVideoHeight(AVIFile *AVI);
double AVI_frame_rate(AVIFile *AVI);
char* AVI_video_compressor(AVIFile *AVI);

long AVI_max_video_chunk(AVIFile *AVI);

long AVI_frame_size(AVIFile *AVI, long frame);

int  AVISeekStart(AVIFile *AVI);
int  AVI_set_video_position(AVIFile *AVI, long frame);
long AVI_get_video_position(AVIFile *AVI, long frame);
long AVI_read_frame(AVIFile *AVI, char *vidbuf, int *keyframe);


long AVI_video_codech_offset(AVIFile *AVI);
long AVI_video_codecf_offset(AVIFile *AVI);

int  AVI_read_data(AVIFile *AVI, char *vidbuf, long max_vidbuf,
                   char *audbuf, long max_audbuf,
                   long *len);

void AVI_print_error(char *str);
char *AVI_strerror(void);
char *AVI_syserror(void);

int AVI_scan(char *name);
int AVI_dump(char *name, int mode);

char *AVI_codec2str(short cc);

void AVI_info(AVIFile *avifile);
uint64 AVI_max_size(void);



#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
