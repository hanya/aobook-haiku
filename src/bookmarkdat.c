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
 * しおりデータ
 *****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mDef.h"

#include "mGui.h"
#include "mList.h"
#include "mStr.h"
#include "mUtilStdio.h"
#include "mTextRead.h"

#include "bookmarkdat.h"
#include "globaldata.h"
#include "aoLayout.h"



/** アイテム破棄ハンドラ */

static void _global_destroy(mListItem *p)
{
	mFree(BMITEM_G(p)->fname);
}

/** アイテム破棄ハンドラ */

static void _local_destroy(mListItem *p)
{
	mFree(BMITEM_L(p)->comment);
}


/** データを解放 */

void BookmarkFree()
{
	mListDeleteAll(&GDAT->listBkmarkGlobal);
	mListDeleteAll(&GDAT->listBkmarkLocal);
}


//========================
// global
//========================


/** グローバルに追加 */

BmItemGlobal *BookmarkGlobal_add()
{
	BmItemGlobal *pi;

	if(GLOBAL_ISNOT_HAVE_TEXT) return NULL;

	//最大数

	if(GDAT->listBkmarkGlobal.num == 100)
		return NULL;

	//追加

	pi = (BmItemGlobal *)mListAppendNew(&GDAT->listBkmarkGlobal,
			sizeof(BmItemGlobal), _global_destroy);
	if(!pi) return NULL;

	pi->fname = mStrdup(GDAT->strFileName.buf);
	pi->lineno = aoGetPageLineNo(GDAT->curpage);

	return pi;
}

/** リスト表示用文字列取得 */

void BookmarkGlobal_getListStr(mStr *str,BmItemGlobal *pi)
{
	mStr fname = MSTR_INIT;

	mStrPathGetFileName(&fname, pi->fname);

	mStrSetFormat(str, "<%d> %t", pi->lineno + 1, &fname);

	mStrFree(&fname);
}

/** ファイルに保存 */

void BookmarkGlobal_saveFile()
{
	FILE *fp;
	BmItemGlobal *pi;
	mStr str = MSTR_INIT;

	//更新された時のみ保存

	if(!(GDAT->fModify & MODIFY_BKMARK_GLOBAL)) return;

	//開く

	mAppGetConfigPath(&str, "bookmark.txt");

	fp = mFILEopenUTF8(str.buf, "wt");

	mStrFree(&str);

	if(!fp) return;

	//

	for(pi = BMITEM_G(GDAT->listBkmarkGlobal.top); pi; pi = BMITEM_G(pi->i.next))
		fprintf(fp, "%d %s\n", pi->lineno + 1, pi->fname);

	fclose(fp);
}

/** ファイルから読み込み */

void BookmarkGlobal_loadFile()
{
	mTextRead *p;
	char *pc,*split;
	mStr str = MSTR_INIT;
	BmItemGlobal *item;

	mAppGetConfigPath(&str, "bookmark.txt");

	p = mTextRead_readFile(str.buf);

	mStrFree(&str);

	if(!p) return;

	//

	while(1)
	{
		pc = mTextReadGetLine_skipEmpty(p);
		if(!pc) break;

		//空白で区切る

		split = strchr(pc, ' ');
		if(!split) continue;

		*(split++) = 0;

		//追加

		item = (BmItemGlobal *)mListAppendNew(&GDAT->listBkmarkGlobal,
				sizeof(BmItemGlobal), _global_destroy);
		if(item)
		{
			item->lineno = atoi(pc) - 1;
			item->fname = mStrdup(split);
		}
	}

	mTextReadEnd(p);
}


//========================
// local
//========================


/** ソート関数 */

static int _local_sort(mListItem *p1,mListItem *p2,intptr_t param)
{
	int n1,n2;

	n1 = BMITEM_L(p1)->lineno;
	n2 = BMITEM_L(p2)->lineno;

	if(n1 == n2)
		return 0;
	else if(n1 < n2)
		return -1;
	else
		return 1;
}

/** ローカルに追加 */

BmItemLocal *BookmarkLocal_add(mStr *strcomment)
{
	BmItemLocal *pi;

	//最大数

	if(GDAT->listBkmarkLocal.num == 100)
		return NULL;

	//追加

	pi = (BmItemLocal *)mListAppendNew(&GDAT->listBkmarkLocal,
			sizeof(BmItemLocal), _local_destroy);
	if(!pi) return NULL;

	pi->lineno = aoGetPageLineNo(GDAT->curpage);
	pi->comment = (mStrIsEmpty(strcomment))? NULL: mStrdup(strcomment->buf);

	//行番号順にソート

	mListSort(&GDAT->listBkmarkLocal, _local_sort, 0);

	return pi;
}

/** リスト表示用文字列取得 */

void BookmarkLocal_getListStr(mStr *str,BmItemLocal *pi)
{
	if(pi->comment)
		mStrSetFormat(str, "%dL : %s", pi->lineno + 1, pi->comment);
	else
		mStrSetFormat(str, "%dL", pi->lineno + 1);
}

/** ファイルに保存 */

void BookmarkLocal_saveFile(const char *fname)
{
	FILE *fp;
	BmItemLocal *pi;

	fp = mFILEopenUTF8(fname, "wt");
	if(!fp) return;

	for(pi = BMITEM_L(GDAT->listBkmarkLocal.top); pi; pi = BMITEM_L(pi->i.next))
	{
		if(pi->comment)
			fprintf(fp, "%d %s\n", pi->lineno + 1, pi->comment);
		else
			fprintf(fp, "%d\n", pi->lineno + 1);
	}

	fclose(fp);
}

/** ファイルから読み込み */

void BookmarkLocal_loadFile(const char *fname)
{
	mTextRead *p;
	char *pc,*split;
	BmItemLocal *item;

	mListDeleteAll(&GDAT->listBkmarkLocal);

	//

	p = mTextRead_readFile(fname);
	if(!p) return;

	while(1)
	{
		pc = mTextReadGetLine_skipEmpty(p);
		if(!pc) break;

		//空白で区切る (なければコメントなし)

		split = strchr(pc, ' ');

		if(split)
			*(split++) = 0;

		//追加

		item = (BmItemLocal *)mListAppendNew(&GDAT->listBkmarkLocal,
				sizeof(BmItemLocal), _local_destroy);
		if(item)
		{
			item->lineno = atoi(pc) - 1;
			item->comment = (split && *split)? mStrdup(split): NULL;
		}
	}

	mTextReadEnd(p);
}
