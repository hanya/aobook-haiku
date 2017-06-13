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

#include <stdlib.h>
#include <stdint.h>

#include "mDef.h"
#include "mDefGui.h"

#include "mPixbuf.h"
#include "mPixbuf_pv.h"

#include <Bitmap.h>


typedef struct
{
	mPixbuf b;
	mPixbufPrivate p;
	
	BBitmap *bitmap;
} __mPixbufHaiku;

extern "C"
mPixbuf *__mPixbufAlloc(void)
{
	__mPixbufHaiku *p = (__mPixbufHaiku *)mMalloc(sizeof(__mPixbufHaiku), TRUE);
	p->bitmap = NULL;
	return (mPixbuf *)p;
}

extern "C"
mPixbuf *__mPixbufFree(mPixbuf *p)
{
	__mPixbufHaiku *pHaiku = (__mPixbufHaiku *)p;
	delete pHaiku->bitmap;
}

extern "C"
mPixbuf *__mPixbufCreate(mPixbuf *p, int w, int h)
{
	__mPixbufHaiku *pHaiku = (__mPixbufHaiku *)p;
	if (pHaiku->bitmap != NULL) {
		__mPixbufFree(p);
	}
	
	BBitmap *bitmap = new BBitmap(BRect(0, 0, w, h), 0, B_RGB32);
	
	pHaiku->bitmap = bitmap;
	
	p->w = w;
	p->h = h;
	p->bpp = 4;
	p->pitch = p->pitch_dir = (int)bitmap->BytesPerRow();
	p->buf = p->buftop = (unsigned char *)bitmap->Bits();
	return 0;
}
