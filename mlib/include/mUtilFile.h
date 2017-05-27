/*$
Copyright (c) 2014-2017, Azel
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
$*/

#ifndef MLIB_UTIL_FILE_H
#define MLIB_UTIL_FILE_H

typedef struct _mFileStat mFileStat;

enum MREADFILEFULL_FLAGS
{
	MREADFILEFULL_FILENAME_LOCALE = 1<<0,
	MREADFILEFULL_ADD_NULL = 1<<1,
	MREADFILEFULL_ACCEPT_EMPTY = 1<<2
};

#ifdef __cplusplus
extern "C" {
#endif

mBool mIsFileExist(const char *filename,mBool bDirectory);
mBool mGetFileStat(const char *filename,mFileStat *dst);
mBool mCreateDir(const char *path);
mBool mCreateDirHome(const char *pathadd);
mBool mDeleteFile(const char *filename);
mBool mDeleteDir(const char *path);

mBool mReadFileFull(const char *filename,uint8_t flags,mBuf *dstbuf);
mBool mReadFileHead(const char *filename,void *buf,int size);

mBool mCopyFile(const char *src,const char *dst,int bufsize);

#ifdef __cplusplus
}
#endif

#endif
