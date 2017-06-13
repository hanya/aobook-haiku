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
 * ページ操作
 ********************************/

#ifndef AOLAYOUT_H
#define AOLAYOUT_H

#include "mListDef.h"

typedef struct _aoFont aoFont;
typedef struct _AO_STYLE AO_STYLE;
typedef struct _AO_PAGEINFO_ITEM AO_PAGEINDEX;
typedef struct _mPopupProgress mPopupProgress;

/** 見出しの項目 */

typedef struct
{
	mListItem i;

	uint8_t type;	//0:大 1:中 2:小
	int pageno;		//ページ番号
	char *text;		//タイトル
}AO_TITLEINFO;

/** レイアウト情報 */

typedef struct _AO_LAYOUT_INFO
{
	void *srcbuf;     //内部データバッファ
	uint32_t srcsize;

	AO_STYLE *style;
	aoFont *font,
		*fontRuby,
		*fontBold,
		*fontHalf,
		*fontInfo;
	const char *filepath;	//テキストのファイルパス (挿絵読み込み用)

	int pagenum,		//ページ数
		titleNum[3];	//各見出しごとの個数
	mList listPage,		//ページデータ
		listTitle;		//見出しデータ
}AO_LAYOUT_INFO;

#ifdef __cplusplus
extern "C" {
#endif
void aoFreeLayoutInfo(AO_LAYOUT_INFO *info);

void aoGetScreenSize(AO_LAYOUT_INFO *info,mSize *sz);

AO_PAGEINDEX *aoGetPageHomeEnd(AO_LAYOUT_INFO *info,mBool end);
AO_PAGEINDEX *aoGetPageIndex(AO_LAYOUT_INFO *info,int page);
AO_PAGEINDEX *aoGetPageIndexByLine(AO_LAYOUT_INFO *info,int line);
AO_PAGEINDEX *aoMovePage(AO_LAYOUT_INFO *info,AO_PAGEINDEX *page,int dir);

int aoGetPageNo(AO_PAGEINDEX *pi);
int aoGetPageLineNo(AO_PAGEINDEX *pi);

void aoSetLayoutFirst(AO_LAYOUT_INFO *info,mPopupProgress *poppg);
void aoDrawPage(AO_LAYOUT_INFO *info,mPixbuf *img,AO_PAGEINDEX *page);
#ifdef __cplusplus
}
#endif
#endif
