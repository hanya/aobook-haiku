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
 * BookmarkWin : しおりウィンドウ
 *****************************************/

#include "mDef.h"

#include "mStr.h"
#include "mList.h"
#include "mWindowDef.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mWindow.h"
#include "mEvent.h"
#include "mTrans.h"
#include "mTab.h"
#include "mTextButtons.h"
#include "mListView.h"
#include "mMessageBox.h"
#include "mSysDialog.h"
#include "mMenu.h"

#include "bookmarkwin.h"
#include "bookmarkdat.h"
#include "mainfunc.h"
#include "globaldata.h"
#include "trgroup.h"
#include "trid.h"


//----------------------

#define BOOKMARKWIN(p)  ((BookmarkWin *)(p))

struct _BookmarkWin
{
	mWidget wg;
	mContainerData ct;
	mWindowData win;

	mTab *tab;
	mListView *lv;
	mTextButtons *btt;
};

//----------------------

#define _TABNO_GLOBAL 0
#define _TABNO_LOCAL  1

enum
{
	TRID_BM_TITLE,
	TRID_BM_GLOBAL,
	TRID_BM_LOCAL,
	
	TRID_BM_BTT_GLOBAL_TOP = 10,
	TRID_BM_BTT_LOCAL_TOP = 20,

	TRID_BM_LMENU_MOVE = 100,
	TRID_BM_LMENU_EDIT,
	TRID_BM_LMENU_DEL
};

static int _event_handle(mWidget *wg,mEvent *ev);

//----------------------


//========================
// sub
//========================


/** ボタンをセット */

static void _set_buttons(BookmarkWin *p)
{
	mTextButtonsDeleteAll(p->btt);

	if(GDAT->bkmarkwin_tabno == _TABNO_GLOBAL)
		mTextButtonsAddButtonsTr(p->btt, TRID_BM_BTT_GLOBAL_TOP, 3);
	else
		mTextButtonsAddButtonsTr(p->btt, TRID_BM_BTT_LOCAL_TOP, 4);
}

/** ウィジェット作成 */

static void _create_widget(BookmarkWin *p)
{
	mTab *tab;

	mContainerSetPadding_one(M_CONTAINER(p), 3);

	p->ct.sepW = 6;

	//タブ

	tab = mTabNew(0, M_WIDGET(p), MTAB_S_TOP | MTAB_S_HAVE_SEP);
	p->tab = tab;

	tab->wg.fLayout |= MLF_EXPAND_W;

	mTabAddItemText(tab, M_TR_T(TRID_BM_GLOBAL));
	mTabAddItemText(tab, M_TR_T(TRID_BM_LOCAL));

	mTabSetSel_index(tab, GDAT->bkmarkwin_tabno);

	//ボタン

	p->btt = mTextButtonsNew(0, M_WIDGET(p), 0);

	_set_buttons(p);

	//リスト

	p->lv = mListViewNew(0, M_WIDGET(p),
				MLISTVIEW_S_AUTO_WIDTH,
				MSCROLLVIEW_S_HORZVERT | MSCROLLVIEW_S_FRAME);

	p->lv->wg.fLayout |= MLF_EXPAND_WH;
}

/** リストセット */

static void _set_list(BookmarkWin *p)
{
	BmItemGlobal *pg;
	BmItemLocal *pl;
	mStr str = MSTR_INIT;

	mListViewDeleteAllItem(p->lv);

	if(GDAT->bkmarkwin_tabno == _TABNO_GLOBAL)
	{
		//グローバル

		for(pg = BMITEM_G(GDAT->listBkmarkGlobal.top); pg; pg = BMITEM_G(pg->i.next))
		{
			BookmarkGlobal_getListStr(&str, pg);

			mListViewAddItem_textparam(p->lv, str.buf, (intptr_t)pg);
		}
	}
	else
	{
		//ローカル

		for(pl = BMITEM_L(GDAT->listBkmarkLocal.top); pl; pl = BMITEM_L(pl->i.next))
		{
			BookmarkLocal_getListStr(&str, pl);

			mListViewAddItem_textparam(p->lv, str.buf, (intptr_t)pl);
		}
	}

	mStrFree(&str);
}

/** ローカル・コメント入力 */

static mBool _local_input_comment(mStr *str)
{
	M_TR_G(TRGROUP_DIALOG);

	return mSysDlgInputText(M_WINDOW(GDAT->mainwin),
			M_TR_T(TRDLG_BMCOMMENT_TITLE), M_TR_T(TRDLG_BMCOMMENT_MES),
			str, 0);
}


//========================
//
//========================


/** 作成 */

void BookmarkWinNew()
{
	BookmarkWin *p;
	mBox box;

	if(GDAT->bkmarkwin) return;

	//作成
	
	p = (BookmarkWin *)mWindowNew(sizeof(BookmarkWin), M_WINDOW(GDAT->mainwin),
			MWINDOW_S_NORMAL | MWINDOW_S_OWNER | MWINDOW_S_TABMOVE | MWINDOW_S_NO_IM);
	if(!p) return;

	p->wg.event = _event_handle;

	//

	M_TR_G(TRGROUP_BKMARKWIN);

	mWindowSetTitle(M_WINDOW(p), M_TR_T(TRID_BM_TITLE));

	//ウィジェット

	_create_widget(p);

	_set_list(p);

	//表示

	box.x = box.y = -1;
	box.w = 250;
	box.h = 300;

	mWindowShowInit(M_WINDOW(p), &GDAT->bkmarkwin_box, &box, -10000, TRUE, FALSE);

	//

	GDAT->bkmarkwin = p;
}

/** グローバルに追加 */

void BookmarkWinAddGlobal(BookmarkWin *p)
{
	BmItemGlobal *pi;

	//データ追加

	pi = BookmarkGlobal_add();
	if(!pi) return;

	//更新フラグ

	GDAT->fModify |= MODIFY_BKMARK_GLOBAL;

	//リスト追加

	if(p && GDAT->bkmarkwin_tabno == _TABNO_GLOBAL)
	{
		mStr str = MSTR_INIT;
		mListViewItem *item;

		BookmarkGlobal_getListStr(&str, pi);

		item = mListViewAddItem_textparam(p->lv, str.buf, (intptr_t)pi);

		mListViewSetFocusItem(p->lv, item);

		mStrFree(&str);
	}
}

/** ローカルに追加 */

void BookmarkWinAddLocal(BookmarkWin *p)
{
	mStr str = MSTR_INIT;
	BmItemLocal *pi;

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	//コメント入力

	if(!(GDAT->optflags & OPTFLAGS_LOCALBKM_NO_COMMENT))
	{
		if(!_local_input_comment(&str)) return;
	}

	//データ追加

	pi = BookmarkLocal_add(&str);

	mStrFree(&str);

	//リスト再セット (行番号順にソートされるので)

	if(pi && p && GDAT->bkmarkwin_tabno == _TABNO_LOCAL)
	{
		_set_list(p);

		mListViewSetFocusItem(p->lv,
			mListViewFindItemByParam(p->lv, (intptr_t)pi));
	}
}


//========================
// コマンド
//========================


/** データ削除 */

static void _delete_item(BookmarkWin *p,mListViewItem *pi)
{
	//データ削除

	mListDelete(
		(GDAT->bkmarkwin_tabno == _TABNO_GLOBAL)? &GDAT->listBkmarkGlobal: &GDAT->listBkmarkLocal,
		M_LISTITEM(pi->param));

	//リストアイテム削除

	mListViewDeleteItem_sel(p->lv, pi);
}

/** グローバル・呼び出し */

static void _global_call(mListViewItem *pi)
{
	BmItemGlobal *p = BMITEM_G(pi->param);

	if(mStrPathCompareEq(&GDAT->strFileName, p->fname))
		//現在のファイル
		mfMovePageToLine(p->lineno);
	else
		mfLoadTextFile(p->fname, -1, p->lineno, FALSE);
}

/** ローカル・読み込み */

static void _local_read(BookmarkWin *p)
{
	mStr str = MSTR_INIT;

	//ファイル名取得

	if(!mSysDlgOpenFile(M_WINDOW(GDAT->mainwin),
			"txt (*.txt)\t*.txt\tall file\t*", 0,
			GDAT->strBkmarkPath.buf, 0, &str))
		return;

	//ディレクトリ記録

	mStrPathGetDir(&GDAT->strBkmarkPath, str.buf);

	//読み込み

	BookmarkLocal_loadFile(str.buf);

	mStrFree(&str);

	//リスト

	_set_list(p);
}

/** ローカル・保存 */

static void _local_save()
{
	mStr str = MSTR_INIT;

	if(GDAT->listBkmarkLocal.num == 0) return;

	//ファイル名取得

	if(!mSysDlgSaveFile(M_WINDOW(GDAT->mainwin),
			"txt (*.txt)\t*.txt", 0,
			GDAT->strBkmarkPath.buf, 0, &str, NULL))
		return;

	mStrPathSetExt(&str, "txt");

	//ディレクトリ記録

	mStrPathGetDir(&GDAT->strBkmarkPath, str.buf);

	//保存

	BookmarkLocal_saveFile(str.buf);

	mStrFree(&str);
}

/** ローカル・メニュー */

static void _local_menu(BookmarkWin *p,mListViewItem *item)
{
	mMenu *menu;
	mMenuItemInfo *pi;
	mPoint pt;
	int id;
	BmItemLocal *bmitem = BMITEM_L(item->param);

	//メニュー

	menu = mMenuNew();

	M_TR_G(TRGROUP_BKMARKWIN);

	mMenuAddTrTexts(menu, TRID_BM_LMENU_MOVE, 3);

	mWidgetGetCursorPos(NULL, &pt);

	pi = mMenuPopup(menu, NULL, pt.x, pt.y, 0);

	id = (pi)? pi->id: -1;

	mMenuDestroy(menu);

	//結果

	switch(id)
	{
		//ページ移動
		case TRID_BM_LMENU_MOVE:
			mfMovePageToLine(bmitem->lineno);
			break;
		//編集
		case TRID_BM_LMENU_EDIT:
			{
			mStr str = MSTR_INIT;

			mStrSetText(&str, bmitem->comment);

			if(_local_input_comment(&str))
			{
				M_FREE_NULL(bmitem->comment);

				if(!mStrIsEmpty(&str))
					bmitem->comment = mStrdup(str.buf);

				//リスト

				BookmarkLocal_getListStr(&str, bmitem);

				mListViewSetItemText(p->lv, item, str.buf);
			}

			mStrFree(&str);
			}
			break;
		//削除
		case TRID_BM_LMENU_DEL:
			_delete_item(p, item);
			break;
	}
}


//========================
// イベント
//========================


/** ボタン処理 */

static void _cmd_button(BookmarkWin *p,int bttno)
{
	mListViewItem *pi;

	//選択アイテム

	pi = mListViewGetFocusItem(p->lv);

	//

	if(GDAT->bkmarkwin_tabno == 0)
	{
		//グローバル

		switch(bttno)
		{
			//追加
			case 0:
				BookmarkWinAddGlobal(p);
				break;
			//削除
			case 1:
				if(pi)
				{
					_delete_item(p, pi);

					GDAT->fModify |= MODIFY_BKMARK_GLOBAL;
				}
				break;
			//呼び出し
			case 2:
				if(pi)
					_global_call(pi);
				break;
		}
	}
	else
	{
		//ローカル

		switch(bttno)
		{
			//追加
			case 0:
				BookmarkWinAddLocal(p);
				break;
			//読み込み
			case 1:
				_local_read(p);
				break;
			//保存
			case 2:
				_local_save();
				break;
			//クリア
			case 3:
				if(GDAT->listBkmarkLocal.num == 0) break;
			
				if(mMessageBox(M_WINDOW(GDAT->mainwin), NULL,
						M_TR_T2(TRGROUP_MESSAGE, TRMES_CONFIRM_ALLCLEAR),
						MMESBOX_YES | MMESBOX_NO, MMESBOX_YES)
					== MMESBOX_YES)
				{
					mListDeleteAll(&GDAT->listBkmarkLocal);
					mListViewDeleteAllItem(p->lv);
				}
				break;
		}
	}
}

/** 通知 */

static void _event_notify(BookmarkWin *p,mEvent *ev)
{
	mWidget *from;
	int type;

	from = ev->notify.widgetFrom;
	type = ev->notify.type;

	if(from == M_WIDGET(p->lv))
	{
		//------ リスト

		if(type == MLISTVIEW_N_ITEM_DBLCLK)
		{
			//ダブルクリック

			if(GDAT->bkmarkwin_tabno == _TABNO_GLOBAL)
				_global_call(M_LISTVIEWITEM(ev->notify.param1));
			else
				mfMovePageToLine(BMITEM_L(ev->notify.param2)->lineno);
		}
		else if(type == MLISTVIEW_N_ITEM_RCLK)
		{
			//右クリック

			if(ev->notify.param1)
				_local_menu(p, (mListViewItem *)ev->notify.param1);
		}
	}
	else if(from == M_WIDGET(p->btt))
	{
		//------ ボタン

		_cmd_button(p, ev->notify.param1);
	}
	else if(from == M_WIDGET(p->tab))
	{
		//------ タブ

		if(type == MTAB_N_CHANGESEL)
		{
			GDAT->bkmarkwin_tabno = ev->notify.param1;

			//ボタン変更

			M_TR_G(TRGROUP_BKMARKWIN);

			_set_buttons(p);

			//リスト再セット

			_set_list(p);

			//再レイアウト (ボタンの幅が変わるので)

			mWidgetLayout(M_WIDGET(p));
		}
	}
}

/** イベント */

int _event_handle(mWidget *wg,mEvent *ev)
{
	switch(ev->type)
	{
		case MEVENT_NOTIFY:
			_event_notify(BOOKMARKWIN(wg), ev);
			break;
	
		//閉じる -> 破棄
		case MEVENT_CLOSE:
			mWindowGetSaveBox(M_WINDOW(wg), &GDAT->bkmarkwin_box);

			mWidgetDestroy(wg);

			GDAT->bkmarkwin = NULL;
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

