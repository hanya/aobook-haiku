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

/*******************************************
 * [イベントリスト]
 *******************************************/

#include "mDef.h"

#include "mAppDef.h"
#include "mAppPrivate.h"
#include "mList.h"
#include "mEvent.h"


//------------------------

typedef struct _mEventListItem
{
	mListItem i;
	mEvent event;
}mEventListItem;

#define EVLIST         (&(MAPP_PV->listEvent))
#define EVLISTITEM(p)  ((mEventListItem *)(p))

//------------------------


static void _item_destroy(mListItem *p)
{
	if(EVLISTITEM(p)->event.data)
		mFree(EVLISTITEM(p)->event.data);
}


/** イベントをすべて削除 */

void mEventListEmpty(void)
{
	mListDeleteAll(EVLIST);
}

/** イベントがあるか */

mBool mEventListPending(void)
{
	return (EVLIST->top != NULL);
}

/** イベント追加 */

void mEventListAppend(mEvent *ev)
{
	mEventListItem *p;

	p = (mEventListItem *)mListAppendNew(EVLIST, sizeof(mEventListItem), _item_destroy);
	if(!p) return;
	
	p->event = *ev;
}

/** ウィジェットとイベントタイプを指定して追加 */

mEvent *mEventListAppend_widget(mWidget *widget,int type)
{
	mEventListItem *p;

	p = (mEventListItem *)mListAppendNew(EVLIST, sizeof(mEventListItem), _item_destroy);
	if(!p) return NULL;
	
	p->event.type   = type;
	p->event.widget = widget;
	
	return &p->event;
}

/** リスト内に同じイベントがあった場合はそれを返す。なければ追加 */

mEvent *mEventListAppend_only(mWidget *widget,int type)
{
	mEventListItem *pi;

	//後ろから検索

	for(pi = EVLISTITEM(EVLIST->bottom); pi; pi = EVLISTITEM(pi->i.prev))
	{
		if(pi->event.widget == widget
			&& pi->event.type == type)
			return &pi->event;
	}

	//見つからないので追加

	return mEventListAppend_widget(widget, type);
}

/** イベント取り出し */

mBool mEventListGetEvent(mEvent *ev)
{
	if(!EVLIST->top)
		return FALSE;
	else
	{
		*ev = EVLISTITEM(EVLIST->top)->event;

		/* アイテムの destroy() ハンドラを呼び出さない
		 * (ev->data のメモリを解放しない) */
		
		mListDeleteNoDestroy(EVLIST, EVLIST->top);
		
		return TRUE;
	}
}

/** 指定ウィジェットのイベントをすべて削除 */

void mEventListDeleteWidget(mWidget *widget)
{
	mList *list = EVLIST;
	mListItem *p,*next;
	
	for(p = list->top; p; p = next)
	{
		next = p->next;
	
		if(EVLISTITEM(p)->event.widget == widget)
			mListDelete(list, p);
	}
}
