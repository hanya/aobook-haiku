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
 * mFileListView
 *****************************************/

#include <string.h>

#include "mDef.h"

#include "mFileListView.h"
#include "mLVItemMan.h"

#include "mWidget.h"
#include "mEvent.h"
#include "mStr.h"
#include "mDirEntry.h"
#include "mUtilStr.h"


//----------------------

#define _FILETYPE_PARENT  0
#define _FILETYPE_DIR     1
#define _FILETYPE_FILE    2

//----------------------


/******************************//**

@defgroup filelistview mFileListView
@brief ファイルリストビュー

<h3>継承</h3>
mWidget \> mScrollView \> mListView \> mFileListView

@ingroup group_widget
@{

@file mFileListView.h
@def M_FILELISTVIEW(p)
@struct mFileListViewData
@struct mFileListView
@enum MFILELISTVIEW_STYLE
@enum MFILELISTVIEW_NOTIFY
@enum MFILELISTVIEW_FILETYPE

@var MFILELISTVIEW_STYLE::MFILELISTVIEW_S_MULTI_SEL
ファイルの複数選択を有効にする

@var MFILELISTVIEW_STYLE::MFILELISTVIEW_S_ONLY_DIR
ディレクトリのみ表示する

@var MFILELISTVIEW_NOTIFY::MFILELISTVIEW_N_SELECT_FILE
ファイルが選択された時

@var MFILELISTVIEW_NOTIFY::MFILELISTVIEW_N_DBLCLK_FILE
ファイルがダブルクリックされた時

@var MFILELISTVIEW_NOTIFY::MFILELISTVIEW_N_CHANGE_DIR
カレントディレクトリが変更された時

********************************/


//=============================
// sub
//=============================


/** ソート関数 */

static int _sortfunc_file(mListItem *item1,mListItem *item2,intptr_t param)
{
	mListViewItem *p1,*p2;

	p1 = M_LISTVIEWITEM(item1);
	p2 = M_LISTVIEWITEM(item2);

	if(p1->param < p2->param)
		return -1;
	else if(p1->param > p2->param)
		return 1;
	else
		return strcmp(p1->text, p2->text);
}

/** ファイルリストセット */

static void _setFileList(mFileListView *p)
{
	mDirEntry *dir;
	const char *fname;
	mStr str = MSTR_INIT;

	//クリア

	mListViewDeleteAllItem(M_LISTVIEW(p));

	//------ ファイル

	dir = mDirEntryOpen(p->flv.strDir.buf);
	if(!dir) goto NEXT;

	while(mDirEntryRead(dir))
	{
		fname = mDirEntryGetFileName(dir);
	
		if(mDirEntryIsDirectory(dir))
		{
			//--- ディレクトリ

			if(mDirEntryIsSpecName(dir))
			{
				//親に戻る (..)

				if(strcmp(fname, "..") == 0)
					mListViewAddItem(M_LISTVIEW(p), "..", 0, 0, _FILETYPE_PARENT);
			}
			else
			{
				//通常
			
				mStrSetChar(&str, '/');
				mStrAppendText(&str, fname);
				
				mListViewAddItem(M_LISTVIEW(p), str.buf, 0, 0, _FILETYPE_DIR);
			}
		}
		else
		{
			//--- ファイル

			//ディレクトリのみ時

			if(p->flv.style & MFILELISTVIEW_S_ONLY_DIR)
				continue;

			//フィルタ

			if(!mIsMatchStringSum(fname, p->flv.strFilter.buf, ';', TRUE))
				continue;

			//追加

			mListViewAddItem(M_LISTVIEW(p), fname, 0, 0, _FILETYPE_FILE);
		}
	}

	mStrFree(&str);

	mDirEntryClose(dir);

	//ソート

	mListViewSortItem(M_LISTVIEW(p), _sortfunc_file, 0);

NEXT:
	//幅セット

	mListViewSetWidthAuto(M_LISTVIEW(p), FALSE);
}

/** 通知 */

static void _notify(mFileListView *p,int type)
{
	mWidgetAppendEvent_notify(MWIDGET_NOTIFYWIDGET_RAW, M_WIDGET(p), type, 0, 0);
}


//=============================
//
//=============================


/** 解放処理 */

void mFileListViewDestroyHandle(mWidget *wg)
{
	mFileListView *p = M_FILELISTVIEW(wg);

	mStrFree(&p->flv.strDir);
	mStrFree(&p->flv.strFilter);

	//リストビュー

	mListViewDestroyHandle(wg);
}

/** 作成 */

mFileListView *mFileListViewNew(int size,mWidget *parent,uint32_t style)
{
	mFileListView *p;
	
	if(size < sizeof(mFileListView)) size = sizeof(mFileListView);
	
	p = (mFileListView *)mListViewNew(size, parent,
			(style & MFILELISTVIEW_S_MULTI_SEL)? MLISTVIEW_S_MULTI_SEL: 0,
			MSCROLLVIEW_S_HORZVERT_FRAME);
	if(!p) return NULL;

	p->wg.destroy = mFileListViewDestroyHandle;
	p->wg.event = mFileListViewEventHandle;
	p->wg.fLayout = MLF_EXPAND_WH;

	//エリアからの通知を自身で受ける
	p->wg.notifyTargetInterrupt = MWIDGET_NOTIFYTARGET_INT_SELF;
	
	p->flv.style = style;

	mWidgetSetInitSize_fontHeight(M_WIDGET(p), 30, 22);

	//

	mStrPathSetHomeDir(&p->flv.strDir);
	mStrSetChar(&p->flv.strFilter, '*');
	
	return p;
}

/** ディレクトリセット
 *
 * @param path NULL または空文字列でホームディレクトリ */

void mFileListViewSetDirectory(mFileListView *p,const char *path)
{
	if(!path || !(*path))
		mStrPathSetHomeDir(&p->flv.strDir);
	else
	{
		mStrSetText(&p->flv.strDir, path);
		mStrPathRemoveBottomPathSplit(&p->flv.strDir);
	}

	_setFileList(p);
}

/** フィルタ文字列セット
 *
 * ワイルドカード有効。複数指定する場合は ';' で区切る。NULL で '*'。
 *
 * @param filter NULL で '*' にセットする */

void mFileListViewSetFilter(mFileListView *p,const char *filter)
{
	if(filter)
		mStrSetText(&p->flv.strFilter, filter);
	else
		mStrSetChar(&p->flv.strFilter, '*');
}

/** リストを更新 */

void mFileListViewUpdateList(mFileListView *p)
{
	_setFileList(p);
}

/** 現在選択されているファイルの名前を取得
 *
 * @return MFILELISTVIEW_FILETYPE_* */

int mFileListViewGetSelectFileName(mFileListView *p,mStr *str,mBool bFullPath)
{
	mListViewItem *pi;
	const char *fname;

	mStrEmpty(str);

	pi = p->lv.manager->itemFocus;

	if(!pi || pi->param == _FILETYPE_PARENT)
		return MFILELISTVIEW_FILETYPE_NONE;

	//パスセット

	fname = pi->text;
	if(pi->param == _FILETYPE_DIR) fname++;

	if(bFullPath)
	{
		mStrCopy(str, &p->flv.strDir);
		mStrPathAdd(str, fname);
	}
	else
		mStrSetText(str, fname);

	return (pi->param == _FILETYPE_FILE)?
		MFILELISTVIEW_FILETYPE_FILE: MFILELISTVIEW_FILETYPE_DIR;
}

/** 複数選択時、すべての選択ファイル名を取得
 *
 * @param str \c "directory\tfile1\tfile2\t...\t"
 *
 * @return ファイル数 */

int mFileListViewGetSelectMultiName(mFileListView *p,mStr *str)
{
	mListViewItem *pi;
	int num = 0;

	//ディレクトリ

	mStrCopy(str, &p->flv.strDir);
	mStrAppendChar(str, '\t');

	//ファイル

	for(pi = M_LISTVIEWITEM(p->lv.manager->list.top); pi; pi = M_LISTVIEWITEM(pi->i.next))
	{
		if((pi->flags & MLISTVIEW_ITEM_F_SELECTED) && pi->param == _FILETYPE_FILE)
		{
			mStrAppendText(str, pi->text);
			mStrAppendChar(str, '\t');

			num++;
		}
	}

	return num;
}


//========================
// ハンドラ
//========================


/** ダブルクリック時 */

static void _dblclk(mFileListView *p)
{
	mListViewItem *pi;
	mStr *pstrdir = &p->flv.strDir;

	pi = p->lv.manager->itemFocus;
	if(!pi) return;

	switch(pi->param)
	{
		//親へ
		case _FILETYPE_PARENT:
			mStrPathRemoveFileName(pstrdir);

			_setFileList(p);
			_notify(p, MFILELISTVIEW_N_CHANGE_DIR);
			break;
		//ディレクトリ
		case _FILETYPE_DIR:
			mStrPathAdd(pstrdir, pi->text + 1);  //先頭の'/'は除く

			_setFileList(p);
			_notify(p, MFILELISTVIEW_N_CHANGE_DIR);
			break;
		//ファイル
		case _FILETYPE_FILE:
			_notify(p, MFILELISTVIEW_N_DBLCLK_FILE);
			break;
	}
}

/** イベント */

int mFileListViewEventHandle(mWidget *wg,mEvent *ev)
{
	mFileListView *p = M_FILELISTVIEW(wg);
	mListViewItem *pi;

	//mListView の通知イベント

	if(ev->type == MEVENT_NOTIFY
		&& ev->notify.widgetFrom == wg)
	{
		switch(ev->notify.type)
		{
			//フォーカスアイテム変更
			case MLISTVIEW_N_CHANGE_FOCUS:
				pi = p->lv.manager->itemFocus;

				if(pi && pi->param == _FILETYPE_FILE)
					_notify(p, MFILELISTVIEW_N_SELECT_FILE);
				break;
			//ダブルクリック
			case MLISTVIEW_N_ITEM_DBLCLK:
				_dblclk(p);
				break;
		}
	
		return TRUE;
	}

	return mListViewEventHandle(wg, ev);
}

/** @} */
