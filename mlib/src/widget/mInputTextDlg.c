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
 * mInputTextDlg
 *****************************************/

#include <stdlib.h>

#include "mDef.h"

#include "mDialog.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mWindow.h"
#include "mLabel.h"
#include "mLineEdit.h"
#include "mStr.h"
#include "mEvent.h"
#include "mSysDialog.h"


//----------------------

typedef struct
{
	mLineEdit *edit;
	uint32_t flags;
}mInputTextDlgData;

typedef struct _mInputTextDlg
{
	mWidget wg;
	mContainerData ct;
	mWindowData win;
	mDialogData dlg;
	mInputTextDlgData it;
}mInputTextDlg;

//----------------------


//******************************
// mInputTextDlg
//******************************


/** イベント */

static int _event_handle(mWidget *wg,mEvent *ev)
{
	if(ev->type == MEVENT_NOTIFY
		&& ev->notify.widgetFrom->id == M_WID_OK)
	{
		mInputTextDlg *p = (mInputTextDlg *)wg;
	
		//空かどうか
		
		if((p->it.flags & MSYSDLG_INPUTTEXT_F_NOEMPTY)
			&& mLineEditIsEmpty(p->it.edit))
		{
			return 1;
		}
	}

	return mDialogEventHandle_okcancel(wg, ev);
}

/** 作成 */

mInputTextDlg *mInputTextDlgNew(mWindow *owner,
	const char *title,const char *message,const char *text,uint32_t flags)
{
	mInputTextDlg *p;
	mWidget *wg;
	
	p = (mInputTextDlg *)mDialogNew(sizeof(mInputTextDlg), owner, MWINDOW_S_DIALOG_NORMAL);
	if(!p) return NULL;
	
	p->wg.event = _event_handle;

	p->it.flags = flags;

	//

	mContainerSetPadding_one(M_CONTAINER(p), 10);
	p->ct.sepW = 6;

	mWindowSetTitle(M_WINDOW(p), title);

	//ウィジェット

	mLabelCreate(M_WIDGET(p), 0, 0, 0, message);

	p->it.edit = mLineEditCreate(M_WIDGET(p), 0, 0, MLF_EXPAND_W, 0);
	
	(p->it.edit)->wg.initW = mWidgetGetFontHeight(M_WIDGET(p)) * 20;

	//初期テキスト

	mLineEditSetText(p->it.edit, text);

	mWidgetSetFocus(M_WIDGET(p->it.edit));
	mLineEditSelectAll(p->it.edit);

	//OK/Cancel

	wg = mContainerCreateOkCancelButton(M_WIDGET(p));
	M_CONTAINER(wg)->ct.padding.top = 10;
	
	return p;
}


//******************************
// 関数
//******************************


/** 文字列入力ダイアログ
 *
 * str の内容が初期テキストとして表示される。
 *
 * @param flags \b MSYSDLG_INPUTTEXT_F_NOEMPTY : 空文字列は許可しない
 * 
 * @ingroup sysdialog */

mBool mSysDlgInputText(mWindow *owner,
	const char *title,const char *message,mStr *str,uint32_t flags)
{
	mInputTextDlg *p;
	mBool ret;

	p = mInputTextDlgNew(owner, title, message, str->buf, flags);
	if(!p) return FALSE;

	mWindowMoveResizeShow_hintSize(M_WINDOW(p));

	ret = mDialogRun(M_DIALOG(p), FALSE);

	if(ret)
		mLineEditGetTextStr(p->it.edit, str);

	mWidgetDestroy(M_WIDGET(p));

	return ret;
}

/** 数値入力ダイアログ
 *
 * @param flags \b MSYSDLG_INPUTNUM_F_DEFAULT : *dst の値をデフォルト値としてセット
 * 
 * @ingroup sysdialog */

mBool mSysDlgInputNum(mWindow *owner,
	const char *title,const char *message,int *dst,int min,int max,uint32_t flags)
{
	mStr str = MSTR_INIT;
	int num;
	mBool ret;

	//デフォルト値

	if(flags & MSYSDLG_INPUTNUM_F_DEFAULT)
		mStrSetInt(&str, *dst);

	//

	ret = mSysDlgInputText(owner, title, message, &str, 0);

	if(ret)
	{
		num = atoi(str.buf);

		if(num < min) num = min;
		else if(num > max) num = max;

		*dst = num;
	}

	mStrFree(&str);

	return ret;
}

