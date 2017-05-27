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
 * 見出し一覧ダイアログ
 *****************************************/


#include "mDef.h"
#include "mStr.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mWindow.h"
#include "mDialog.h"
#include "mListView.h"
#include "mEvent.h"
#include "mTrans.h"

#include "globaldata.h"
#include "trgroup.h"
#include "trid.h"

#include "aoLayout.h"


//-------------------

typedef struct
{
	mWidget wg;
	mContainerData ct;
	mWindowData win;
	mDialogData dlg;

	mListView *list;
}CaptionDlg;

//-------------------


/** イベント */

static int _event_handle(mWidget *wg,mEvent *ev)
{
	if(ev->type == MEVENT_NOTIFY
		&& ev->notify.widgetFrom->id == 100
		&& ev->notify.type == MLISTVIEW_N_ITEM_DBLCLK)
	{
		//ダブルクリック

		mDialogEnd(M_DIALOG(wg), TRUE);
		return 1;
	}
	else
		return mDialogEventHandle_okcancel(wg, ev);
}

/** 見出しリストセット */

static void _setlist(CaptionDlg *p)
{
	AO_TITLEINFO *pi;
	mStr str = MSTR_INIT;
	int toplevel,i;

	//大/中/小のうちのトップレベル

	for(toplevel = 0; toplevel < 3 && GDAT->layout->titleNum[toplevel] == 0; toplevel++);

	//

	for(pi = (AO_TITLEINFO *)GDAT->layout->listTitle.top; pi; pi = (AO_TITLEINFO *)pi->i.next)
	{
		mStrEmpty(&str);

		//インデント

		for(i = pi->type - toplevel; i > 0; i--)
			mStrAppendText(&str, "  ");

		//

		mStrAppendText(&str, pi->text);
	
		mListViewAddItem(p->list, str.buf, -1, 0, pi->pageno);
	}

	mStrFree(&str);

	//最初の項目を選択

	mListViewSetFocusItem_index(p->list, 0);
}

/** 作成 */

CaptionDlg *CaptionDlgNew(mWindow *owner)
{
	CaptionDlg *p;
	mListView *list;

	p = (CaptionDlg *)mDialogNew(sizeof(CaptionDlg), owner,
		MWINDOW_S_DIALOG_NORMAL | MWINDOW_S_NO_IM);
	if(!p) return NULL;

	p->wg.event = _event_handle;

	mContainerSetPadding_one(M_CONTAINER(p), 8);

	p->ct.sepW = 12;

	mWindowSetTitle(M_WINDOW(p), M_TR_T2(TRGROUP_DIALOG, TRDLG_CAPTION_TITLE));

	//リスト

	list = mListViewNew(0, M_WIDGET(p),
		0, MSCROLLVIEW_S_FRAME | MSCROLLVIEW_S_VERT);

	p->list = list;

	list->wg.id = 100;
	list->wg.fLayout = MLF_EXPAND_WH;

	mWidgetSetInitSize_fontHeight(M_WIDGET(list), 18, 18);

	//OK/Cancel

	mContainerCreateOkCancelButton(M_WIDGET(p));

	//リストセット

	if(GDAT->layout->listTitle.num)
		_setlist(p);

	return p;
}


/** 見出しダイアログ実行
 *
 * @return ジャンプするページ番号。-1 でキャンセル */

int GetCaptionPageDlg()
{
	CaptionDlg *p;
	int page;

	p = CaptionDlgNew(M_WINDOW(GDAT->mainwin));
	if(!p) return -1;

	mWindowMoveResizeShow_hintSize(M_WINDOW(p));

	if(!mDialogRun(M_DIALOG(p), FALSE))
		page = -1;
	else
		page = mListViewGetItemParam(p->list, -1);

	mWidgetDestroy(M_WIDGET(p));

	return page;
}
