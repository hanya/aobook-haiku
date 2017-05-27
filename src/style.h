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
 * スタイルデータ
 ********************************/

#ifndef STYLE_H
#define STYLE_H

#include "mStr.h"
#include "aoStyle.h"

typedef struct _aoFont aoFont;

typedef struct _StyleData
{
	AO_STYLE b;

	mStr strName,
		strFontText,
		strFontBold,
		strFontRuby,
		strFontInfo,
		strBkgndFile;

	mRgbCol colBkgnd;	//背景色
	int bkgnd_imgtype;

	//

	aoFont *fontText,
		*fontBold,
		*fontRuby,
		*fontHalf,
		*fontInfo;
	mImageBuf *imgBkgnd;
}StyleData;


void StyleFree(StyleData *p);
void StyleFreeBuf(StyleData *p);
void StyleFreeData(StyleData *p);
void StyleFreeFont(StyleData *p);
void StyleFreeAO(StyleData *p);

StyleData *StyleAlloc();

void StyleGetFilePath(mStr *str,const char *name);
void StyleLoadConf(StyleData *p,const char *name);
void StyleSaveConf(StyleData *p);
void StyleInit(StyleData *p);

void StyleSetDefault(StyleData *p);
void StyleCopyData(StyleData *dst,StyleData *src);
mBool StyleChange(StyleData *pd,StyleData *ps);
mBool StyleChangeByName(const char *name);

void StyleCreateFont(StyleData *p);
void StyleSetUTF32Text(uint32_t **ppbuf,const char *text);

#endif
