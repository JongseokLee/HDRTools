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
 * \file Win32.H
 *
 * \brief
 *    Header file relating to Windows compilation. 
 *    Also sets up some functions for other operating systems relating to I/O
 *    Should be extended for Win64 as well.
 *
 * \author
 *     - Alexis Michael Tourapis         <atourapis@apple.com>
 *
 *************************************************************************************
 */

#ifndef _WIN32_H_
#define _WIN32_H_

#include <fcntl.h>
#include <stdio.h>
#define ENABLE_GROUP_PERMISSIONS 1

#ifdef WIN32
# include <io.h>
# include <sys/types.h>
# include <sys/stat.h>

//! Lets define some params that are not defined under windows
typedef __int64   int64;
# define FORMAT_OFF_T "I64d"

# define  M_PI   3.14159265358979323846f

# define _S_IRWXU        (_S_IREAD | _S_IWRITE | _S_IEXEC)
# define _S_IXUSR         _S_IEXEC        /* read permission, user */
# define _S_IWUSR         _S_IWRITE       /* write permission, user */
# define _S_IRUSR         _S_IREAD        /* execute/search permission, user */
# define _S_IRWXG        (_S_IRWXU >> 3)
# define _S_IXGRP        (_S_IXUSR >> 3)  /* read permission, group */
# define _S_IWGRP        (_S_IWUSR >> 3)  /* write permission, group */
# define _S_IRGRP        (_S_IRUSR >> 3)  /* execute/search permission, group */
# define _S_IRWXO        (_S_IRWXG >> 3)
# define _S_IXOTH        (_S_IXGRP >> 3)  /* read permission, other */
# define _S_IWOTH        (_S_IWGRP >> 3)  /* write permission, other */
# define _S_IROTH        (_S_IRGRP >> 3)  /* execute/search permission, other */


# define  snprintf  _snprintf
# define  open      _open
# define  close     _close
# define  mm_read   _read
# define  mm_write  _write
# define  lseek     _lseeki64
# define  fsync     _commit
# define  tell      _telli64
# define  strcasecmp _strcmpi
# define  ftruncate  _chsize_s

# define  OPENFLAGS_WRITE  _O_WRONLY|_O_CREAT|_O_BINARY|_O_TRUNC
# define  OPENFLAGS_READ   _O_RDONLY|_O_BINARY

# define fseeko(fm, pos, type) _fseeki64(fm, (int64) (pos), (type))
# define ftello(fm) _ftelli64(fm)

# if ENABLE_GROUP_PERMISSIONS
#   define  OPEN_PERMISSIONS _S_IRUSR | _S_IWUSR | _S_IRGRP | _S_IROTH
# else
#   define  OPEN_PERMISSIONS _S_IRUSR | _S_IWUSR
# endif

#else
# include <unistd.h>
typedef long long int64;
# define  mm_read  read
# define  mm_write write
# define  tell(fd) lseek(fd, 0, SEEK_CUR)
# define  FORMAT_OFF_T "lld"
# define  OPENFLAGS_WRITE  O_WRONLY|O_CREAT|O_TRUNC
# define  OPENFLAGS_READ   O_RDONLY

# if ENABLE_GROUP_PERMISSIONS
#   define  OPEN_PERMISSIONS S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
# else
#   define  OPEN_PERMISSIONS S_IRUSR | S_IWUSR
# endif

#endif


#endif
