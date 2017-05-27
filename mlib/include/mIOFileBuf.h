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

#ifndef MLIB_IOFILEBUF_H
#define MLIB_IOFILEBUF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mIOFileBuf mIOFileBuf;

void mIOFileBuf_close(mIOFileBuf *p);

mIOFileBuf *mIOFileBuf_openRead_filename(const char *filename);
mIOFileBuf *mIOFileBuf_openRead_buf(const void *buf,uint32_t bufsize);
mIOFileBuf *mIOFileBuf_openRead_FILE(void *fp);

int mIOFileBuf_read(mIOFileBuf *p,void *buf,int size);
mBool mIOFileBuf_readEmpty(mIOFileBuf *p,int size);
mBool mIOFileBuf_readSize(mIOFileBuf *p,void *buf,int size);
mBool mIOFileBuf_readByte(mIOFileBuf *p,void *buf);

void mIOFileBuf_setPos(mIOFileBuf *p,uint32_t pos);
void mIOFileBuf_seekCur(mIOFileBuf *p,int seek);

#ifdef __cplusplus
}
#endif

#endif
