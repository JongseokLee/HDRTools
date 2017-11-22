/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * <OWNER> = Apple Inc.
 * <ORGANIZATION> = Apple Inc.
 * <YEAR> = 2016
 *
 * Copyright (c) 2016, Apple Inc.
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
 * \file IODPX.H
 *
 * \brief
 *    Main IODPX header file
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef __IODPX_H__
#define __IODPX_H__
#include <string.h>


typedef enum {
  DPX_DS_USER        = 0,   // User-defined
  DPX_DS_RED         = 1,   // Red
  DPX_DS_GREEN       = 2,   // Green
  DPX_DS_BLUE        = 3,   // Blue
  DPX_DS_ALPHA       = 4,   // Alpha
  DPX_DS_LUMINANCE   = 6,   // Luminance
  DPX_DS_CHROMINANCE = 7,   // Chrominance
  DPX_DS_DEPTH       = 8,   // Depth
  DPX_DS_COMPOSITE   = 9,   // Composite video
  DPX_DS_RGB         = 50,  // RGB
  DPX_DS_RGBA        = 51,  // RGBA
  DPX_DS_ABGR        = 52,  // ABGR
  DPX_DS_CBYCRY      = 100, // CbYCrY
  DPX_DS_CBYACRYA    = 101, // CbYaCrYa
  DPX_DS_CBYCR       = 102, // CbYCr
  DPX_DS_CBYCRA      = 103, // CbYCra
  DPX_DS_2EL         = 150, // User-defined 2-component element
  DPX_DS_3EL         = 151, // User-defined 3-component element
  DPX_DS_4EL         = 152, // User-defined 4-component element
  DPX_DS_5EL         = 153, // User-defined 5-component element
  DPX_DS_6EL         = 154, // User-defined 6-component element
  DPX_DS_7EL         = 155, // User-defined 7-component element
  DPX_DS_8EL         = 156, // User-defined 8-component element
  DPX_DS_TOTAL       = 256
}DPXDescriptor;

class DPXGenericFileHeader {
public:
  uint32 m_magic;            /* Magic number */
  uint32 m_imageOffset;      /* Offset to start of image data in bytes */
  char   m_version[8];       /* Version stamp of header format */
  uint32 m_fileSize;         /* Total DPX file size in bytes */
  uint32 m_dittoKey;         /* Image content specifier */
  uint32 m_genericSize;      /* Generic section header length in bytes */
  uint32 m_industrySize;     /* Industry-specific header length in bytes */
  uint32 m_userSize;         /* User-defined data length in bytes */
  char   m_fileName[100];    /* Name of DPX file */
  char   m_timeDate[24];     /* Time and date of file creation */
  char   m_creator[100];     /* Name of file creator */
  char   m_project[200];     /* Name of project */
  char   m_copyright[200];   /* File contents copyright information */
  uint32 m_encryptKey;       /* Encryption key */
  byte   m_reserved[104];    /* Reserved field used for padding */
  DPXGenericFileHeader() {
    m_magic = 0x58504453; // init to little endian
    m_imageOffset = 4;
    strcpy(m_version, "v1.0"); // For v1.0 files, this field contains the ASCII string "V1.0".
    
    m_fileSize = 768;          // Filesize in bytes
    m_dittoKey = 1;            // indicates whether the image in the DPX file is the same as in the previous frame (a value of 0), or is a new frame (a value of 1)
    m_genericSize = 0;         // size of the generic header section in bytes
    m_industrySize = 0;        // total size of the industry-specific header section in bytes
    m_userSize = 0;            // total size of the user-defined data area in bytes. If no user-defined area is present, its value will be 0.
    memset(m_fileName, 0, 100 * sizeof(char)); 
    strcpy(m_timeDate, "0000:00:00:00:00:00:000"); // string in the form YYYY:MM:DD:HH:MM:SS:LTZ.
    memset(m_creator, 0, 100 * sizeof(char));
    memset(m_project, 0, 200 * sizeof(char)); 
    memset(m_copyright, 0, 200 * sizeof(char));
    m_encryptKey = 0xFFFFFFFF;  
    memset(m_reserved, 0, 104 * sizeof(byte));
  }
};

class DPXImageElement {
public:
  uint32  m_dataSign;          /* Data sign extension */
  uint32  m_lowData;           /* Reference low data code value */
  float   m_lowQuantity;       /* Reference low quantity represented */
  uint32  m_highData;          /* Reference high data code value */
  float   m_highQuantity;      /* Reference high quantity represented */
  byte    m_descriptor;        /* Descriptor for image element */
  byte    m_transfer;          /* Transfer characteristics for element */
  byte    m_colorimetric;      /* Colormetric specification for element */
  byte    m_bitSize;           /* Bit size for element */
  uint16  m_packing;           /* Packing for element */
  uint16  m_encoding;          /* Encoding for element */
  uint32  m_dataOffset;        /* Offset to data of element */
  uint32  m_endOfLinePadding;  /* End of line padding used in element */
  uint32  m_endOfImagePadding; /* End of image padding used in element */
  char    m_description[32];   /* Description of element */  
  DPXImageElement () {
    m_dataSign = 0;      
    m_lowData = 0;         
    m_lowQuantity = 0.0f;      
    m_highData = 1;         
    m_highQuantity = 0;     
    m_descriptor = 0;      
    m_transfer = 0;        
    m_colorimetric = 0;      
    m_bitSize = 0;          
    m_packing = 0;           
    m_encoding = 0;          
    m_dataOffset = 0;        
    m_endOfLinePadding = 0;  
    m_endOfImagePadding = 0; 
    memset(m_description, 0, 32 * sizeof(char));
  }
};

class DPXGenericImageHeader {
public:
  uint16          m_orientation;            /* Image orientation */
  uint16          m_numberElements;         /* Number of image elements */
  uint32          m_pixelsPerLine;          /* Pixels per line */
  uint32          m_linesPerElement;        /* Lines per image element */
  DPXImageElement m_imageElement[8];
  byte            m_reserved[52];            /* Reserved field used for padding */
  DPXGenericImageHeader() {
    m_orientation = 0;
    m_numberElements = 0;
    m_pixelsPerLine = 0;
    m_linesPerElement = 0;
    memset(m_reserved, 0, 52 * sizeof(byte));
  }
};


class DPXGenericOrientationHeader {
public:
  uint32 m_xOffset;           /* X offset */
  uint32 m_yOffset;           /* Y offset */
  float  m_xCenter;           /* X center */
  float  m_yCenter;           /* Y center */
  uint32 m_xOriginalSize;     /* X original size */
  uint32 m_yOriginalSize;     /* Y original size */
  char   m_fileName[100];     /* Source image file name */
  char   m_timeDate[24];      /* Source image date and time */
  char   m_inputName[32];     /* Input device name */
  char   m_inputSN[32];       /* Input device serial number */
  uint16 m_order[4];          /* Border validity (XL, XR, YT, YB) */
  uint32 m_aspectRatio[2];    /* Pixel aspect ratio (H:V) */
  byte   m_reserved[28];      /* Reserved field used for padding */
  
  DPXGenericOrientationHeader() {
    m_xOffset = 0;
    m_yOffset = 0;
    m_xCenter = 0.0f;
    m_yCenter = 0.0f;
    m_xOriginalSize = 0;
    m_yOriginalSize = 0;
    memset(m_fileName, 0, 100 * sizeof(char)); 
    strcpy(m_timeDate, "0000:00:00:00:00:00:000"); // string in the form YYYY:MM:DD:HH:MM:SS:LTZ.
    memset(m_inputName, 0, 32 * sizeof(char));
    memset(m_inputSN, 0, 32 * sizeof(char)); 
    memset(m_order, 0, 4 * sizeof(char));
    m_aspectRatio[0] = m_aspectRatio[1] = 1;
    memset(m_reserved, 0, 28 * sizeof(byte));
  }
};


class DPXIndustryFilmInfoHeader {
public:
  char   m_filmMfgId[2];      /* Film manufacturer ID code */
  char   m_filmType[2];       /* File type */
  char   m_offset[2];         /* Offset in perfs */
  char   m_prefix[6];         /* Prefix */
  char   m_count[4];          /* Count */
  char   m_format[32];        /* Format */
  uint32 m_framePosition;     /* Frame position in sequence */
  uint32 m_sequenceLen;       /* Sequence length in frames */
  uint32 m_heldCount;         /* Held count */
  float  m_frameRate;         /* Frame rate of original in frames/sec */
  float  m_shutterAngle;      /* Shutter angle of camera in degrees */
  char   m_frameId[32];       /* Frame identification */
  char   m_slateInfo[100];    /* Slate information */
  byte   m_reserved[56];      /* Reserved field used for padding */  
  DPXIndustryFilmInfoHeader() {
    memset(m_filmMfgId, 0, 2 * sizeof(char));
    memset(m_filmType, 0, 2 * sizeof(char)); 
    memset(m_offset, 0, 2 * sizeof(char));
    memset(m_prefix, 0, 6 * sizeof(char));
    memset(m_count, 0, 4 * sizeof(char));
    memset(m_format, 0, 32 * sizeof(char));
    m_framePosition = 0;
    m_sequenceLen = 0;
    m_heldCount = 0;
    m_frameRate = 30.0f;
    m_shutterAngle = 0.0f;
    memset(m_frameId, 0, 32 * sizeof(char));
    memset(m_slateInfo, 0, 100 * sizeof(char));
    memset(m_reserved, 0, 56 * sizeof(byte));
  }
};

class DPXIndustryTelevisionInfoHeader {
public:
  uint32  m_timeCode;            /* SMPTE time code */
  uint32  m_userBits;            /* SMPTE user bits */
  byte    m_interlace;           /* Interlace */
  byte    m_fieldNumber;         /* Field number */
  byte    m_videoSignal;         /* Video signal standard */
  byte    m_padding;             /* Structure alignment padding */
  float   m_horzSampleRate;      /* Horizontal sampling rate in Hz */
  float   m_vertSampleRate;      /* Vertical sampling rate in Hz */
  float   m_frameRate;           /* Temporal sampling rate or frame rate in Hz */
  float   m_timeOffset;          /* Time offset from sync to first pixel */
  float   m_gamma;               /* Gamma value */
  float   m_blackLevel;          /* Black level code value */
  float   m_blackGain;           /* Black gain */
  float   m_breakpoint;          /* Breakpoint */
  float   m_whiteLevel;          /* Reference white level code value */
  float   m_integrationTimes;    /* Integration time(s) */
  byte    m_reserved[76];        /* Reserved field used for padding */  
  DPXIndustryTelevisionInfoHeader() {
    m_timeCode         = 0;
    m_userBits         = 0;
    m_interlace        = 0;
    m_fieldNumber      = 0;
    m_videoSignal      = 0;
    m_padding          = 0;
    m_horzSampleRate   = 0.0f;
    m_vertSampleRate   = 0.0f;
    m_frameRate        = 30.0f;
    m_timeOffset       = 0.0f;
    m_gamma            = 0.0f;
    m_blackLevel       = 0.0f;
    m_blackGain        = 0.0f;
    m_breakpoint       = 0.0f;
    m_whiteLevel       = 0.0f;
    m_integrationTimes = 0.0f;
    memset(m_reserved, 0, 76 * sizeof(byte));
  }
};


class DPXUserDefinedData {
public:
  char  m_userId[32];    /* User-defined identification string */
  byte *m_data;          /* User-defined data */
  
  DPXUserDefinedData() {
    memset(m_userId, 0, 32 * sizeof(char));
    m_data = NULL;
  }
  ~DPXUserDefinedData() {
    if (m_data != NULL) {
      delete [] m_data;
      m_data = NULL;
    }
  }
};

typedef unsigned char BYTE4 [4];

class DPXImageDataElement {
public:
  BYTE4 *m_data;
  DPXImageDataElement() {
    m_data = NULL;
  }
  ~DPXImageDataElement() {
    if (m_data != NULL) {
      delete [] m_data;
      m_data = NULL;
    }
  }
};

class DPXFileFormat {
public:
  DPXGenericFileHeader            m_fileHeader;
  DPXGenericImageHeader           m_imageHeader;
  DPXGenericOrientationHeader     m_orientHeader;
  DPXIndustryFilmInfoHeader       m_filmHeader;
  DPXIndustryTelevisionInfoHeader m_tvHeader;
  DPXUserDefinedData             *m_userData;  /* NULL if data not present */
  DPXImageDataElement            *m_imageData; /* Array of 1 to 8 DPXImageDataElement structures */
  DPXFileFormat() {
    m_userData = NULL;
    m_imageData = NULL;
  }
  ~DPXFileFormat() {
    if (m_userData != NULL) {
      delete [] m_userData;
      m_userData = NULL;
    }
    if (m_imageData != NULL) {
      delete [] m_imageData;
      m_imageData = NULL;
    }
  }
};

//! DPX file data
class DPXFileData {
public:
  vector<uint16>  m_img;                  //!< Image data
  vector<uint8>   m_buffer;         //!< The file will be read into memory in one gulp here.
  uint8               *m_mp;                              //!< Memory pointer.
  int                  m_le;                               //!< Little endian - 0 FALSE, 1 - TRUE
  DPXFileFormat        m_fileFormat;
  
  uint32             (*setU16)   (DPXFileData *, uint16 value);
  uint32             (*setU32)   (DPXFileData *, uint32 value);
  float              (*setFloat) (DPXFileData *, float value);
  uint16             (*getU16)   (DPXFileData *);
  uint32             (*getU32)   (DPXFileData *);
  float              (*getFloat) (DPXFileData *);
} ;

#endif
//-----------------------------------------------------------------------------
// End of file
//-----------------------------------------------------------------------------
