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

/*****************************************
 * mList [双方向リスト]
 *****************************************/

#include "mDef.h"
#include "mList.h"


/************************//**

@defgroup list mList
@brief 双方向リスト

@ingroup group_data
@{

@file mList.h
@file mListDef.h
@struct _mList
@struct _mListItem

@def MLIST_INIT
mList 構造体の値を初期化

@def M_LISTITEM(p)
(mListItem *) に型変換

******************************/



/** アイテムのメモリを確保
 *
 * ゼロクリアされる。
 * 
 * @param size アイテム構造体の全サイズ */

mListItem *mListItemAlloc(int size,void (*destroy)(mListItem *))
{
	mListItem *p;

	if(size < sizeof(mListItem))
		size = sizeof(mListItem);
	
	p = (mListItem *)mMalloc(size, TRUE);
	if(!p) return NULL;
	
	p->destroy = destroy;
	
	return p;
}


//====================
// 追加
//====================


/** アイテムを新規作成し、最後に追加
 * 
 * @param size リストアイテムの構造体のサイズ */

mListItem *mListAppendNew(mList *list,int size,void (*destroy)(mListItem *))
{
	mListItem *pi;
	
	pi = mListItemAlloc(size, destroy);
	if(!pi) return NULL;

	mListLinkAppend(list, pi);

	list->num++;

	return pi;
}

/** アイテムを新規作成し、挿入 */

mListItem *mListInsertNew(mList *list,mListItem *pos,int size,void (*destroy)(mListItem *))
{
	mListItem *pi;
	
	pi = mListItemAlloc(size, destroy);
	if(!pi) return NULL;

	mListLinkInsert(list, pi, pos);

	list->num++;

	return pi;
}

/** すでに確保されたアイテムを追加 */

void mListAppend(mList *list,mListItem *item)
{
	if(item)
	{
		mListLinkAppend(list, item);
		list->num++;
	}
}

/** すでに確保されたアイテムを挿入 */

void mListInsert(mList *list,mListItem *pos,mListItem *item)
{
	if(item)
	{
		mListLinkInsert(list, item, pos);
		list->num++;
	}
}

/** アイテムを削除せずにリストから取り外す
 *
 * mList::num の値も変化する。 */

void mListRemove(mList *list,mListItem *item)
{
	if(item)
	{
		mListLinkRemove(list, item);
		list->num--;
	}
}

/** リスト全体を複製 (各アイテムは固定サイズ) */

mBool mListDup(mList *dst,mList *src,int itemsize)
{
	mListItem *ps,*pd;

	if(itemsize < sizeof(mListItem))
		itemsize = sizeof(mListItem);

	for(ps = src->top; ps; ps = ps->next)
	{
		pd = (mListItem *)mMemdup(ps, itemsize);
		if(!pd) return FALSE;

		mListAppend(dst, pd);
	}

	return TRUE;
}


//========================
// 削除
//========================


/** アイテムを全て削除 */

void mListDeleteAll(mList *list)
{
	mListItem *pi,*next;
	
	for(pi = list->top; pi; pi = next)
	{
		next = pi->next;
		
		if(pi->destroy)
			(pi->destroy)(pi);
		
		mFree(pi);
	}
	
	list->top = list->bottom = NULL;
	list->num = 0;
}

/** リストからアイテムを削除
 *
 * @param item NULL の場合は何もしない */

void mListDelete(mList *list,mListItem *item)
{
	if(item)
	{
		if(item->destroy)
			(item->destroy)(item);

		mListLinkRemove(list, item);
		
		mFree(item);
		
		list->num--;
	}
}

/** リストからアイテムを削除
 * 
 * アイテムの destroy() ハンドラは呼ばない。 */

void mListDeleteNoDestroy(mList *list,mListItem *item)
{
	mListLinkRemove(list, item);
	
	mFree(item);
	
	list->num--;
}

/** インデックス位置からアイテム削除
 * 
 * @return 削除されたか */

mBool mListDeleteByIndex(mList *list,int index)
{
	mListItem *p = mListGetItemByIndex(list, index);
	
	if(!p)
		return FALSE;
	else
	{
		mListDelete(list, p);
		return TRUE;
	}
}

/** 先頭から指定個数を削除 */

void mListDeleteTopNum(mList *list,int num)
{
	mListItem *p,*next;

	for(p = list->top; p && num > 0; num--, p = next)
	{
		next = p->next;

		mListDelete(list, p);
	}
}

/** 終端から指定個数を削除 */

void mListDeleteBottomNum(mList *list,int num)
{
	mListItem *p,*next;

	for(p = list->bottom; p && num > 0; num--, p = next)
	{
		next = p->prev;

		mListDelete(list, p);
	}
}


//========================


/** アイテムの位置を移動
 * 
 * @param dst 挿入位置。NULL で最後尾へ */

void mListMove(mList *list,mListItem *src,mListItem *dst)
{
	if(src == dst) return;
	
	if(!dst && list->bottom == src) return;
	
	mListLinkRemove(list, src);
	mListLinkInsert(list, src, dst);
}

/** アイテムを先頭へ移動 */

void mListMoveTop(mList *list,mListItem *item)
{
	if(list->top != item)
		mListMove(list, item, list->top);
}

/** アイテムを一つ上下に移動
 *
 * @return 移動したか */

mBool mListMoveUpDown(mList *list,mListItem *item,mBool up)
{
	if(!item) return FALSE;
	
	if(up)
	{
		if(item->prev)
		{
			mListMove(list, item, item->prev);
			return TRUE;
		}
	}
	else
	{
		if(item->next)
		{
			mListMove(list, item->next, item);
			return TRUE;
		}
	}

	return FALSE;
}

/** アイテムの位置を入れ替える */

void mListSwap(mList *list,mListItem *item1,mListItem *item2)
{
	mListItem *p = item1->next;
	
	mListMove(list, item1, item2->next);
	mListMove(list, item2, p);
}

/** ソート
 * 
 * @param param 比較関数に渡すパラメータ */

void mListSort(mList *list,int (*comp)(mListItem *,mListItem *,intptr_t),intptr_t param)
{
	int h,i,flag;
	mListItem *p1,*p2,*ptmp;
	
	//[コムソート]

	h = list->num;
	flag = 0;

	while(h > 1 || flag)
	{
		if(h > 1)
		{
			h = (h * 10) / 13;
			if(h == 9 || h == 10) h = 11;
		}

		//p2 = 先頭から +h の位置

		p1 = p2 = list->top;

		for(i = h; i > 0 && p2; i--, p2 = p2->next);

		for(flag = 0; p2; )
		{
			if((*comp)(p1, p2, param) > 0)
			{
				//入れ替え

				mListSwap(list, p1, p2);
				flag = 1;

				ptmp = p1;
				p1   = p2->next;
				p2   = ptmp->next;
			}
			else
			{
				p1 = p1->next;
				p2 = p2->next;
			}
		}
	}
}


//========================


/** 2つのアイテムの位置関係を得る
 * 
 * @retval -1  item1 @< item2
 * @retval 0   item1 == item2
 * @retval 1   item1 @> item2 */

int mListGetDir(mList *list,mListItem *item1,mListItem *item2)
{
	mListItem *p;
	
	if(item1 == item2) return 0;
	
	for(p = item1->next; p; p = p->next)
	{
		if(p == item2) return -1;
	}
	
	return 1;
}

/** インデックス位置からアイテム取得 */

mListItem *mListGetItemByIndex(mList *list,int index)
{
	mListItem *p;
	int cnt;
	
	for(p = list->top, cnt = 0; p && cnt != index; p = p->next, cnt++);
	
	return p;
}

/** アイテムのインデックス位置取得 */

int mListGetItemIndex(mList *list,mListItem *item)
{
	int pos;
	mListItem *p;
	
	for(p = list->top, pos = 0; p; p = p->next, pos++)
	{
		if(p == item) return pos;
	}
	
	return -1;
}


//================================
// リンク操作
//================================


/** リストの最後にリンクする
 *
 * @attention mList::num の値は操作されない */

void mListLinkAppend(mList *list,mListItem *item)
{
	if(list->bottom)
	{
		item->prev = list->bottom;
		(list->bottom)->next = item;
	}
	else
	{
		item->prev = NULL;
		list->top = item;
	}
	
	item->next = NULL;
	list->bottom = item;
}

/** リストの指定位置にリンクを挿入
 * 
 * @param pos  挿入位置。NULL で最後尾  */

void mListLinkInsert(mList *list,mListItem *item,mListItem *pos)
{
	if(!pos)
		mListLinkAppend(list, item);
	else
	{
		if(pos->prev)
			(pos->prev)->next = item;
		else
			list->top = item;
		
		item->prev = pos->prev;
		pos->prev = item;
		item->next = pos;
	}
}

/** リストからアイテムのリンクを外す */

void mListLinkRemove(mList *list,mListItem *item)
{
	if(item->prev)
		(item->prev)->next = item->next;
	else
		list->top = item->next;
	
	if(item->next)
		(item->next)->prev = item->prev;
	else
		list->bottom = item->prev;
}

/** @} */
