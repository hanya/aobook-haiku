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
 * ファイル開くダイアログ (テキスト)
 *****************************************/

#include "mDef.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mWindow.h"
#include "mFileDialog.h"
#include "mLabel.h"
#include "mComboBox.h"
#include "mTrans.h"

#include "trgroup.h"
#include "aoText.h"


//---------------

#define TRID_CHARCODE 0
#define TRID_AUTO     1
#define WID_CHARCODE  100

//---------------


/** OK/キャンセル時 */

static mBool _okcancel_handle(mFileDialog *p,mBool bOK,const char *path)
{
	//文字コード
	
	if(bOK)
	{
		mComboBox *cb;

		cb = (mComboBox *)mWidgetFindByID(M_WIDGET(p), WID_CHARCODE);

		*((int *)(p + 1)) = (cb)? mComboBoxGetItemParam(cb, -1): -1;
	}

	return TRUE;
}

/** 拡張ウィジェット作成 */

static void _create_widget(mFileDialog *p)
{
	mWidget *ct;
	mComboBox *cb;
	int i;

	M_TR_G(TRGROUP_FILEDIALOG);

	//水平コンテナ

	ct = mContainerCreate(M_WIDGET(p), MCONTAINER_TYPE_HORZ, 0, 4, 0);

	//ラベル

	mLabelCreate(ct, 0, MLF_MIDDLE, 0, M_TR_T(TRID_CHARCODE));

	//コンボボックス

	cb = mComboBoxCreate(ct, WID_CHARCODE, 0, MLF_MIDDLE, 0);

	mComboBoxAddItem(cb, M_TR_T(TRID_AUTO), -1);

	for(i = 0; i < AOTEXT_CODE_NUM; i++)
		mComboBoxAddItem(cb, aoTextGetCodeName(i), i);

	mComboBoxSetWidthAuto(cb);
	mComboBoxSetSel_index(cb, 0);
}

/** ファイル開くダイアログ */

mBool GetOpenTextFile(mWindow *owner,const char *initdir,mStr *strdst,int *retcode)
{
	mFileDialog *p;
	mBool ret;

	p = mFileDialogNew(sizeof(mFileDialog) + sizeof(int), owner, 0, MFILEDIALOG_TYPE_OPENFILE);
	if(!p) return FALSE;

	p->fdlg.onOkCancel = _okcancel_handle;

	mFileDialogInit(p,
		"all files (*)\t*\ttext file (*.txt)\t*.txt\tzip file (*.zip)\t*.zip", 0,
		initdir, strdst);

	_create_widget(p);

	mWindowMoveResizeShow_hintSize(M_WINDOW(p));

	ret = mDialogRun(M_DIALOG(p), FALSE);

	if(ret)
		*retcode = *((int *)(p + 1));

	mWidgetDestroy(M_WIDGET(p));

	return ret;
}

