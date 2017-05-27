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
 * 環境設定
 *****************************************/

#include <string.h>

#include "mDef.h"
#include "mStr.h"
#include "mDialog.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mWindow.h"
#include "mTab.h"
#include "mTrans.h"
#include "mEvent.h"

#include "trgroup.h"
#include "globaldata.h"

#include "envoptdlg.h"
#include "envoptdlg_pv.h"


//--------------------

enum
{
	TRID_TITLE,

	TRID_TAB_TOP,
	TRID_TAB_OPT = TRID_TAB_TOP,
	TRID_TAB_MOUSE,
	TRID_TAB_KEY,
	TRID_TAB_TOOL,
};

//--------------------

typedef struct
{
	mWidget wg;
	mContainerData ct;
	mWindowData win;
	mDialogData dlg;
	//

	EnvOptDlgContents cdat;
	mWidget *cth;
	mTab *tab;

	EnvOptDlgEditData edit;
}EnvOptDlg;

//--------------------


//*******************************
// EnvOptDlg
//*******************************


//=====================
// sub
//=====================


/** 編集用データ作成 */

static void _create_edit_data(EnvOptDlg *p)
{
	mStrCopy(&p->edit.strGUIFont, &GDAT->strGUIFont);

	p->edit.optflags = GDAT->optflags;

	memcpy(p->edit.mousectrl, GDAT->mousectrl, MOUSECTRL_BTT_NUM);

	p->edit.keybuf = EnvOptKey_createEditData();

	mStrArrayCopy(p->edit.strTool, GDAT->strTool, TOOLITEM_NUM);
}

/** 現在の内容を破棄 */

static void _destroy_contents(EnvOptDlg *p)
{
	if(p->cth->first != p->cth->last)
	{
		if(p->cdat.destroy)
			(p->cdat.destroy)(p->cdat.data);

		mFree(p->cdat.data);

		memset(&p->cdat, 0, sizeof(EnvOptDlgContents));
	}
}

/** 各タブ内容作成 */

static void _create_contents(EnvOptDlg *p,int type)
{
	//現在の内容を削除

	_destroy_contents(p);

	//コンテナ破棄

	if(p->cth->first != p->cth->last)
		mWidgetDestroy(p->cth->last);

	//作成

	M_TR_G(TRGROUP_DLG_ENVOPT);

	switch(type)
	{
		case TRID_TAB_OPT:
			EnvOptOpt_create(&p->cdat, p->cth, &p->edit);
			break;
		case TRID_TAB_MOUSE:
			EnvOptMouse_create(&p->cdat, p->cth, p->edit.mousectrl);
			break;
		case TRID_TAB_KEY:
			EnvOptKey_create(&p->cdat, p->cth, &p->edit.keybuf);
			break;
		case TRID_TAB_TOOL:
			EnvOptTool_create(&p->cdat, p->cth, p->edit.strTool);
			break;
	}

	//レイアウト

	mWidgetCalcHintSize(p->cth);
	mWidgetLayout(p->cth);
}


//=====================


/** イベントハンドラ */

static int _event_handle(mWidget *wg,mEvent *ev)
{
	EnvOptDlg *p = (EnvOptDlg *)wg;

	if(ev->type == MEVENT_NOTIFY
		&& ev->notify.widgetFrom == (mWidget *)p->tab
		&& ev->notify.type == MTAB_N_CHANGESEL)
	{
		//タブ選択変更

		_create_contents(p, mTabGetItemParam_index(p->tab, -1));
	}
	else
	{
		if(p->cdat.event)
			(p->cdat.event)(p->cdat.data, ev);

		return mDialogEventHandle_okcancel(wg, ev);
	}

	return 1;
}

/** 作成 */

EnvOptDlg *EnvOptDlgNew(mWindow *owner)
{
	EnvOptDlg *p;
	mWidget *ct;
	mTab *tab;
	int i;

	p = (EnvOptDlg *)mDialogNew(sizeof(EnvOptDlg), owner, MWINDOW_S_DIALOG_NORMAL);
	if(!p) return NULL;

	p->wg.event = _event_handle;

	//

	M_TR_G(TRGROUP_DLG_ENVOPT);

	mWindowSetTitle(M_WINDOW(p), M_TR_T(TRID_TITLE));

	//----- ウィジェット

	mContainerSetPadding_b4(M_CONTAINER(p), M_MAKE_DW4(4,8,8,8));

	p->ct.sepW = 15;

	//水平コンテナ

	ct = mContainerCreate(M_WIDGET(p), MCONTAINER_TYPE_HORZ, 0, 12, MLF_EXPAND_WH);
	p->cth = ct;

	//タブ

	tab = mTabNew(0, ct, MTAB_S_LEFT | MTAB_S_HAVE_SEP);
	p->tab = tab;

	tab->wg.fLayout = MLF_EXPAND_H;

	for(i = 0; i < 4; i++)
		mTabAddItem(tab, M_TR_T(TRID_TAB_TOP + i), -1, TRID_TAB_TOP + i);

	mTabSetSel_index(tab, 0);

	//ボタン

	mContainerCreateOkCancelButton(M_WIDGET(p));

	//-------------

	//編集中用のデータ作成

	_create_edit_data(p);

	//初期内容

	_create_contents(p, TRID_TAB_OPT);

	return p;
}


//*******************************
// 関数
//*******************************


/** 環境設定ダイアログ
 *
 * return: 更新フラグ */

int EnvOptDialog(mWindow *owner,uint32_t **ppnewkey)
{
	EnvOptDlg *p;
	mBool ret;
	int flag = 0;

	p = EnvOptDlgNew(owner);
	if(!p) return FALSE;

	mWindowMoveResizeShow(M_WINDOW(p),
		(p->wg.initW < 450)? 450: p->wg.initW,
		(p->wg.initH < 400)? 400: p->wg.initH);

	ret = mDialogRun(M_DIALOG(p), FALSE);

	//編集中の現在データを退避

	_destroy_contents(p);

	//各データ

	EnvOptOpt_finish(&p->edit, ret);

	EnvOptMouse_finish(p->edit.mousectrl, ret);

	if(EnvOptKey_finish(p->edit.keybuf, ppnewkey, ret))
		flag |= ENVOPTDLG_UPDATE_KEY;

	if(EnvOptTool_finish(p->edit.strTool, ret))
		flag |= ENVOPTDLG_UPDATE_TOOL;

	//

	mWidgetDestroy(M_WIDGET(p));

	return flag;
}
