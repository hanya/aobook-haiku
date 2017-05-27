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

/********************************
 * フォント
 ********************************/

#ifndef AOFONT_H
#define AOFONT_H

#include "mFont.h"

typedef struct _aoFont aoFont;

enum
{
	AOFONT_DRAWTEXT_F_VERT = 1<<0,
	AOFONT_DRAWTEXT_F_VERT_ROTATE = 1<<1,
	AOFONT_DRAWTEXT_F_NO_LEFT = 1<<2
};

//

mBool aoFontInit();
void aoFontEnd();

void aoFontFree(aoFont *font);
aoFont *aoFontCreate(const char *format,mBool bHalf);

int aoFontGetHeight(aoFont *font);
void aoFontSetCacheMax(aoFont *font,int max);

void aoFontDrawText(aoFont *font,mPixbuf *img,int x,int y,
	const uint32_t *text,int len,mRgbCol col,uint32_t flags);
void aoFontDrawText8(aoFont *font,mPixbuf *img,int x,int y,
	const uint8_t *text,int len,mRgbCol col);

int aoFontGetTextWidth(aoFont *font,const uint32_t *text,int len);
int aoFontGetTextWidth8(aoFont *font,const uint8_t *text,int len);

#endif
