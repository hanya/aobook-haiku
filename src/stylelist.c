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
 * スタイル設定時用のリストデータ
 *****************************************/

#include "mDef.h"
#include "mList.h"
#include "mUtilFile.h"

#include "globaldata.h"
#include "style.h"
#include "stylelist.h"


//----------------------

#define _ITEM(p)  ((StyleListItem *)(p))

//----------------------


//========================
// sub
//========================


/** アイテム削除時 */

static void _item_destroy(mListItem *p)
{
	StyleFreeData(&_ITEM(p)->dat);
}

/** アイテム追加 */

static StyleListItem *_add_item(mList *list)
{
	return (StyleListItem *)mListAppendNew(list,
				sizeof(StyleListItem), _item_destroy);
}


//========================


/** 編集用にスタイルのリストを作成
 *
 * return: 現在のスタイルのアイテム */

StyleListItem *StyleListCreate(mList *list)
{
	StyleListItem *pi;

	//現在のスタイルを複製

	pi = _add_item(list);

	StyleCopyData(&pi->dat, GDAT->style);

	return pi;
}

/** スタイル新規追加 */

StyleListItem *StyleListAppend(mList *list,mStr *strname)
{
	StyleListItem *pi;

	//削除チェックがONのデータに同名のものがある場合、データ削除

	for(pi = _ITEM(list->top); pi; pi = _ITEM(pi->i.next))
	{
		if(pi->bDelete && mStrPathCompareEq(&pi->dat.strName, strname->buf))
		{
			mListDelete(list, M_LISTITEM(pi));
			break;
		}
	}

	//新規追加

	pi = _add_item(list);
	if(pi)
	{
		StyleSetDefault(&pi->dat);

		mStrCopy(&pi->dat.strName, strname);
	}

	return pi;
}

/** 指定スタイルを設定ファイルから読み込み */

StyleListItem *StyleListLoad(mList *list,const char *name)
{
	StyleListItem *pi;

	pi = _add_item(list);
	if(pi)
		StyleLoadConf(&pi->dat, name);

	return pi;
}

/** スタイルを設定ファイルに書き込み&ファイル削除 */

void StyleListSaveAndDelete(mList *list)
{
	StyleListItem *pi;
	mStr str = MSTR_INIT;

	for(pi = _ITEM(list->top); pi; pi = _ITEM(pi->i.next))
	{
		if(!pi->bDelete)
			StyleSaveConf(&pi->dat);
		else
		{
			//ファイル削除

			StyleGetFilePath(&str, pi->dat.strName.buf);

			mDeleteFile(str.buf);
		}
	}

	mStrFree(&str);
}
