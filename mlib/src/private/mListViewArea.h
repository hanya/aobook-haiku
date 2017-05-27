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

#ifndef MLIB_LISTVIEWAREA_H
#define MLIB_LISTVIEWAREA_H

#include "mScrollViewArea.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_LISTVIEWAREA(p)  ((mListViewArea *)(p))

typedef struct
{
	mLVItemMan *manager;
	mWidget *owner;
	uint32_t style,
		styleLV;
	int itemH;
}mListViewAreaData;

typedef struct _mListViewArea
{
	mWidget wg;
	mScrollViewAreaData sva;
	mListViewAreaData lva;
}mListViewArea;


enum MLISTVIEWAREA_STYLE
{
	MLISTVIEWAREA_S_POPUP = 1
};

enum MLISTVIEWAREA_NOTIFY
{
	MLISTVIEWAREA_N_POPUPEND = 10000
};


#define MLISTVIEW_DRAW_ITEM_MARGIN    3
#define MLISTVIEW_DRAW_CHECKBOX_SIZE  13
#define MLISTVIEW_DRAW_CHECKBOX_SPACE 4


/*------*/

mListViewArea *mListViewAreaNew(int size,mWidget *parent,
	uint32_t style,uint32_t styleLV,
	mLVItemMan *manager,mWidget *owner);

void mListViewArea_scrollToFocus(mListViewArea *p,int dir);

#ifdef __cplusplus
}
#endif

#endif
