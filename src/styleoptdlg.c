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
 * スタイル設定
 *****************************************/

#include <string.h>

#include "mDef.h"
#include "mStr.h"
#include "mDialog.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mWindow.h"
#include "mTab.h"
#include "mComboBox.h"
#include "mTextButtons.h"
#include "mTrans.h"
#include "mEvent.h"
#include "mList.h"
#include "mMessageBox.h"
#include "mSysDialog.h"
#include "mMenu.h"

#include "trgroup.h"
#include "trid.h"
#include "trid_menu.h"
#include "globaldata.h"
#include "mainwindow.h"
#include "style.h"
#include "stylelist.h"

#include "styleoptdlg_pv.h"


//--------------------

enum
{
	TRID_TITLE,
	TRID_EDIT_BTTS,

	TRID_TAB_BASIC = 100,
	TRID_TAB_FONT,
	TRID_TAB_CHAR,
};

enum
{
	WID_STYLELIST = 10000,
	WID_EDITBUTTONS,
	WID_TAB
};

//--------------------

typedef struct
{
	mWidget wg;
	mContainerData ct;
	mWindowData win;
	mDialogData dlg;
	//

	StyleOptDlgContents cdat;

	mList listStyle;
	StyleListItem *curstyle;
	mBool bChangeStyle;

	mWidget *ctcontents;
	mComboBox *cbsel;
	mTab *tab;
}StyleOptDlg;

//--------------------



//=====================
// sub
//=====================


/** スタイルのリストをセット */

static void _set_style_list(StyleOptDlg *p)
{
	mMenu *menu;
	const char *pc;
	int i,num;

	menu = GDAT->mainwin->main.menuStyle;

	//スタイルメニューからセット
	/* カレントスタイル以外は、選択された時に読み込むので、
	 * パラメータ値を 0 にしておく */

	num = mMenuGetNum(menu);

	for(i = 0; i < num; i++)
	{
		pc = mMenuGetTextByIndex(menu, i);

		mComboBoxAddItem(p->cbsel, pc,
			(mStrPathCompareEq(&p->curstyle->dat.strName, pc))? (intptr_t)p->curstyle: 0);
	}

	//選択

	mComboBoxSetSel_findParam(p->cbsel, (intptr_t)p->curstyle);
}

/** 現在のタブのデータをセット */

static void _set_contents_data(StyleOptDlg *p)
{
	if(p->cdat.setdata)
		(p->cdat.setdata)(p->cdat.data, &p->curstyle->dat);
}

/** 現在のタブのデータを取得 */

static void _get_contents_data(StyleOptDlg *p)
{
	if(p->cdat.getdata)
		(p->cdat.getdata)(p->cdat.data, &p->curstyle->dat);
}

/** 現在のタブ内容(データ)を破棄 */

static void _destroy_contents(StyleOptDlg *p)
{
	if(p->ctcontents->first)
	{
		_get_contents_data(p);

		mFree(p->cdat.data);

		memset(&p->cdat, 0, sizeof(StyleOptDlgContents));
	}
}

/** 各タブ内容作成 */

static void _create_contents(StyleOptDlg *p,int type)
{
	//現在の内容を削除

	_destroy_contents(p);

	//コンテナ破棄

	if(p->ctcontents->first)
		mWidgetDestroy(p->ctcontents->first);

	//作成

	M_TR_G(TRGROUP_DLG_STYLEOPT);

	switch(type)
	{
		case TRID_TAB_BASIC:
			StyleOptBasic_create(&p->cdat, p->ctcontents);
			break;
		case TRID_TAB_FONT:
			StyleOptFont_create(&p->cdat, p->ctcontents);
			break;
		case TRID_TAB_CHAR:
			StyleOptChar_create(&p->cdat, p->ctcontents);
			break;
	}

	//データセット

	_set_contents_data(p);

	//レイアウト

	mWidgetCalcHintSize(p->ctcontents);
	mWidgetLayout(p->ctcontents);
}

/** スタイル選択変更時 */

static void _change_style(StyleOptDlg *p,mListViewItem *lvi,mBool del)
{
	StyleListItem *pi;

	//現在のタブ内容を退避

	if(!del) _get_contents_data(p);

	//まだ読み込まれていない場合は、作成＆読み込み

	pi = (StyleListItem *)lvi->param;

	if(!pi)
	{
		pi = StyleListLoad(&p->listStyle, lvi->text);

		lvi->param = (intptr_t)pi;
	}

	//変更

	p->curstyle = pi;

	_set_contents_data(p);
}


//=====================
// command
//=====================


/** 新規スタイル */

static void _cmd_style_new(StyleOptDlg *p)
{
	mStr str = MSTR_INIT;
	StyleListItem *pi;
	mListViewItem *lvi;

	//名前

	M_TR_G(TRGROUP_DIALOG);

	if(!mSysDlgInputText(M_WINDOW(p),
			M_TR_T(TRDLG_STYLENEW_TITLE), M_TR_T(TRDLG_STYLENEW_MES),
			&str,
			MSYSDLG_INPUTTEXT_F_NOEMPTY))
		return;

	//同名のスタイルが存在するか

	for(lvi = mComboBoxGetTopItem(p->cbsel); lvi; lvi = M_LISTVIEWITEM(lvi->i.next))
	{
		if(mStrPathCompareEq(&str, lvi->text))
		{
			mMessageBoxErrTr(M_WINDOW(p),
				TRGROUP_MESSAGE, TRMES_STYLE_SAMENAME);
		
			mStrFree(&str);
			return;
		}
	}

	//追加

	pi = StyleListAppend(&p->listStyle, &str);
	if(pi)
	{
		lvi = mComboBoxAddItem(p->cbsel, str.buf, (intptr_t)pi);
		mComboBoxSetSelItem(p->cbsel, lvi);

		_change_style(p, lvi, FALSE);

		p->bChangeStyle = TRUE;
	}

	mStrFree(&str);
}

/** 削除 */

static void _cmd_style_del(StyleOptDlg *p)
{
	mListViewItem *lvi;

	//スタイルが一つなら削除できない

	if(mComboBoxGetItemNum(p->cbsel) == 1) return;

	//削除確認

	if(mMessageBox(M_WINDOW(p), NULL, M_TR_T2(TRGROUP_MESSAGE, TRMES_CONFIRM_DELETE),
			MMESBOX_YES | MMESBOX_NO, MMESBOX_YES)
		== MMESBOX_NO)
		return;

	//削除フラグをON (最後に削除する)

	p->curstyle->bDelete = TRUE;

	//コンボ削除＆選択変更

	lvi = mComboBoxDeleteItem_sel(p->cbsel);

	_change_style(p, lvi, TRUE);

	p->bChangeStyle = TRUE;
}


//=====================
//
//=====================


/** イベントハンドラ */

static int _event_handle(mWidget *wg,mEvent *ev)
{
	StyleOptDlg *p = (StyleOptDlg *)wg;
	int type;

	if(ev->type == MEVENT_NOTIFY)
	{
		type = ev->notify.type;
	
		switch(ev->notify.widgetFrom->id)
		{
			//スタイルリスト
			case WID_STYLELIST:
				if(type == MCOMBOBOX_N_CHANGESEL)
				{
					_change_style(p, (mListViewItem *)ev->notify.param1, FALSE);
				}
				return 1;
			//編集ボタン
			case WID_EDITBUTTONS:
				if(ev->notify.param1 == 0)
					_cmd_style_new(p);
				else if(ev->notify.param1 == 1)
					_cmd_style_del(p);
				return 1;
			//タブ
			case WID_TAB:
				if(type == MTAB_N_CHANGESEL)
					_create_contents(p, mTabGetItemParam_index(p->tab, -1));
				return 1;
		}
	}

	if(p->cdat.event)
		(p->cdat.event)(p->cdat.data, ev);

	return mDialogEventHandle_okcancel(wg, ev);
}

/** ウィジェット作成 */

static void _create_widget(StyleOptDlg *p)
{
	int i;
	mWidget *ct;
	mComboBox *cb;
	mTextButtons *btts;
	mTab *tab;

	//---- スタイル選択+ボタン

	//水平コンテナ

	ct = mContainerCreate(M_WIDGET(p), MCONTAINER_TYPE_HORZ, 0, 8, MLF_EXPAND_W);

	//コンボボックス

	cb = mComboBoxCreate(ct, WID_STYLELIST, 0, MLF_EXPAND_W, 0);
	p->cbsel = cb;

	_set_style_list(p);

	//ボタン

	btts = mTextButtonsNew(0, ct, 0);
	btts->wg.id = WID_EDITBUTTONS;

	mTextButtonsAddButtonsTr(btts, TRID_EDIT_BTTS, 2);

	//----- タブ

	tab = mTabCreate(M_WIDGET(p), WID_TAB, MTAB_S_TOP | MTAB_S_HAVE_SEP, MLF_EXPAND_W, 0);
	p->tab = tab;

	for(i = 0; i < 3; i++)
		mTabAddItem(tab, M_TR_T(TRID_TAB_BASIC + i), -1, TRID_TAB_BASIC + i);

	mTabSetSel_index(tab, 0);

	//----- コンテンツ用コンテナ

	p->ctcontents = mContainerCreate(M_WIDGET(p), MCONTAINER_TYPE_VERT, 0, 0, MLF_EXPAND_WH);

	//----- OK/Cancel

	mContainerCreateOkCancelButton(M_WIDGET(p));
}

/** 作成 */

StyleOptDlg *StyleOptDlgNew(mWindow *owner)
{
	StyleOptDlg *p;

	p = (StyleOptDlg *)mDialogNew(sizeof(StyleOptDlg), owner, MWINDOW_S_DIALOG_NORMAL);
	if(!p) return NULL;

	p->wg.event = _event_handle;

	//

	M_TR_G(TRGROUP_DLG_STYLEOPT);

	mWindowSetTitle(M_WINDOW(p), M_TR_T(TRID_TITLE));

	mContainerSetPadding_one(M_CONTAINER(p), 8);
	p->ct.sepW = 10;

	//------ データ作成

	p->curstyle = StyleListCreate(&p->listStyle);

	//----- ウィジェット

	_create_widget(p);

	//----- 初期内容

	_create_contents(p, TRID_TAB_BASIC);

	return p;
}


/** スタイル設定ダイアログ実行
 *
 * @return [0]更新なし [1]描画更新のみ [2]再レイアウト [8bit]メニュー変更 */

int StyleOptDialog(mWindow *owner)
{
	StyleOptDlg *p;
	int ret = 0;

	p = StyleOptDlgNew(owner);
	if(!p) return 0;

	mWindowMoveResizeShow_hintSize(M_WINDOW(p));

	ret = mDialogRun(M_DIALOG(p), FALSE);
	
	//編集中の現在データを退避

	_destroy_contents(p);

	//適用
	
	if(ret)
	{
		//選択が変更されたか

		if(!mStrPathCompareEq(&GDAT->style->strName, p->curstyle->dat.strName.buf))
			p->bChangeStyle = TRUE;
	
		//ファイルに保存

		StyleListSaveAndDelete(&p->listStyle);
	
		//現在のスタイルに適用

		if(StyleChange(GDAT->style, &p->curstyle->dat))
			ret = 2;

		//スタイルメニュー変更

		if(p->bChangeStyle) ret |= 1<<8;
	}

	//削除

	mListDeleteAll(&p->listStyle);

	mWidgetDestroy(M_WIDGET(p));

	return ret;
}
