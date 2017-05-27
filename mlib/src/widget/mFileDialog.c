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
 * mFileDialog
 *****************************************/


#include <string.h>

#include "mDef.h"

#include "mFileDialog.h"
#include "mSysDialog.h"

#include "mStr.h"
#include "mWidget.h"
#include "mWindow.h"
#include "mEvent.h"
#include "mTrans.h"
#include "mPath.h"
#include "mUtilFile.h"
#include "mUtilStr.h"

#include "mContainer.h"
#include "mLabel.h"
#include "mButton.h"
#include "mLineEdit.h"
#include "mComboBox.h"
#include "mFileListView.h"
#include "mMessageBox.h"


//********************************


/** 開くファイル名取得ダイアログ
 *
 * 複数選択時は、"directory\tfile1\tfile2...\t" の文字列になる。
 * 
 * @param filter Ex:"png file\t*.png\tall file\t*"
 * @param deftype ファイル種類のデフォルトのインデックス番号
 * @ingroup sysdialog */

mBool mSysDlgOpenFile(mWindow *owner,const char *filter,int deftype,
	const char *initdir,uint32_t flags,mStr *strdst)
{
	mFileDialog *p;
	uint32_t style = 0;

	if(flags & MSYSDLG_OPENFILE_F_MULTI_SEL)
		style |= MFILEDIALOG_S_MULTI_SEL;

	//

	p = mFileDialogNew(0, owner, style, MFILEDIALOG_TYPE_OPENFILE);
	if(!p) return FALSE;

	mFileDialogInit(p, filter, deftype, initdir, strdst);

	mWindowMoveResizeShow_hintSize(M_WINDOW(p));

	return mDialogRun(M_DIALOG(p), TRUE);
}

/** 保存ファイル名取得ダイアログ
 *
 * @param strdst 空でない場合、ファイル名としてセットされる
 * @param filtertype 選択されたファイル種類のインデックス番号が入る
 * @ingroup sysdialog */

mBool mSysDlgSaveFile(mWindow *owner,const char *filter,int deftype,
	const char *initdir,uint32_t flags,mStr *strdst,int *filtertype)
{
	mFileDialog *p;
	uint32_t style = 0;

	if(flags & MSYSDLG_SAVEFILE_F_NO_OVERWRITE_MES)
		style |= MFILEDIALOG_S_NO_OVERWRITE_MES;

	//

	p = mFileDialogNew(0, owner, style, MFILEDIALOG_TYPE_SAVEFILE);
	if(!p) return FALSE;

	p->fdlg.filtertypeDst = filtertype;

	mFileDialogInit(p, filter, deftype, initdir, strdst);

	mWindowMoveResizeShow_hintSize(M_WINDOW(p));

	return mDialogRun(M_DIALOG(p), TRUE);
}

/** ディレクトリ名取得ダイアログ
 *
 * @ingroup sysdialog */

mBool mSysDlgSelectDir(mWindow *owner,const char *initdir,uint32_t flags,mStr *strdst)
{
	mFileDialog *p;

	p = mFileDialogNew(0, owner, 0, MFILEDIALOG_TYPE_DIR);
	if(!p) return FALSE;

	mFileDialogInit(p, NULL, 0, initdir, strdst);

	mWindowMoveResizeShow_hintSize(M_WINDOW(p));

	return mDialogRun(M_DIALOG(p), TRUE);
}



//********************************


enum
{
	WID_OK = 100000,
	WID_CANCEL,
	WID_EDIT_DIR,
	WID_HOME,
	WID_FILELIST,
	WID_TYPE
};


/*****************************//**

@defgroup filedialog mFileDialog
@brief ファイル選択ダイアログ

\ref sysdialog に、一つの関数で実行できるようにまとめてある。

<h3>継承</h3>
mWidget \> mContainer \> mWindow \> mDialog \> mFileDialog

@ingroup group_window
@{

@file mFileDialog.h
@def M_FILEDIALOG(p)
@struct _mFileDialog
@struct mFileDialogData
@enum MFILEDIALOG_STYLE
@enum MFILEDIALOG_TYPE

@var MFILEDIALOG_STYLE::MFILEDIALOG_S_MULTI_SEL
複数ファイル選択を有効にする

@var MFILEDIALOG_STYLE::MFILEDIALOG_S_NO_OVERWRITE_MES
保存ダイアログ時、上書き保存メッセージを表示しない

**********************************/


//============================
// sub - 初期化
//============================


/** ファイル種類コンボボックスセット
 *
 * param にはフィルタ文字列のポインタが入る */

static void _set_combo_type(mComboBox *pcb,const char *filter,int deftype)
{
	const char *pc,*pc2;

	if(!filter) return;

	for(pc = filter; *pc; )
	{
		pc2 = pc + strlen(pc) + 1;
		if(*pc2 == 0) break;

		mComboBoxAddItem(pcb, pc, (intptr_t)pc2);

		pc = pc2 + strlen(pc2) + 1;
	}

	mComboBoxSetSel_index(pcb, deftype);
}

/** メイン部分のウィジェット作成 */

static void _create_widget_main(mFileDialog *p)
{
	mWidget *cttop,*ct;
	uint32_t f;

	//垂直コンテナ

	cttop = mContainerCreate(M_WIDGET(p), MCONTAINER_TYPE_VERT, 0, 6, MLF_EXPAND_WH);

	//------ ディレクトリ

	ct = mContainerCreate(cttop, MCONTAINER_TYPE_HORZ, 0, 3, MLF_EXPAND_W);

	//入力

	p->fdlg.editdir = mLineEditCreate(ct, WID_EDIT_DIR,
		MLINEEDIT_S_NOTIFY_ENTER, MLF_EXPAND_W | MLF_MIDDLE, 0);

	//home

	mButtonCreate(ct, WID_HOME, MBUTTON_S_REAL_W, 0, 0, "home");

	//----- ファイルリスト

	f = 0;

	if(p->fdlg.type == MFILEDIALOG_TYPE_DIR) f |= MFILELISTVIEW_S_ONLY_DIR;

	if(p->fdlg.type == MFILEDIALOG_TYPE_OPENFILE
		&& (p->fdlg.style & MFILEDIALOG_S_MULTI_SEL))
		f |= MFILELISTVIEW_S_MULTI_SEL;

	p->fdlg.flist = mFileListViewNew(0, cttop, f);

	p->fdlg.flist->wg.id = WID_FILELIST;

	//------ ファイル名・種類

	ct = mContainerCreate(cttop, MCONTAINER_TYPE_GRID, 2, 0, MLF_EXPAND_W);

	M_CONTAINER(ct)->ct.gridSepCol = 5;
	M_CONTAINER(ct)->ct.gridSepRow = 6;

	//ファイル名 (保存時のみ)

	if(p->fdlg.type == MFILEDIALOG_TYPE_SAVEFILE)
	{
		mLabelCreate(ct, 0, MLF_RIGHT | MLF_MIDDLE, 0, M_TR_T(M_TRSYS_FILENAME));

		p->fdlg.editname = mLineEditCreate(ct, 0, 0, MLF_EXPAND_W, 0);
	}

	//種類 (ディレクトリ時は除く)

	if(p->fdlg.type != MFILEDIALOG_TYPE_DIR)
	{
		mLabelCreate(ct, 0, MLF_RIGHT | MLF_MIDDLE, 0, M_TR_T(M_TRSYS_TYPE));

		p->fdlg.cbtype = mComboBoxCreate(ct, WID_TYPE, 0, MLF_EXPAND_W, 0);
	}
}

/** ウィジェット作成 */

static void _create_widget(mFileDialog *p)
{
	mWidget *ct;
	mButton *btt;

	//トップレイアウト

	mContainerSetPadding_one(M_CONTAINER(p), 8);

	//メイン部分

	_create_widget_main(p);

	//---- ボタン

	ct = mContainerCreate(M_WIDGET(p), MCONTAINER_TYPE_HORZ, 0, 4, MLF_RIGHT);

	M_CONTAINER(ct)->ct.padding.top = 8;

	//開く or 保存

	btt = mButtonCreate(ct, WID_OK, 0, 0, 0,
		(p->fdlg.type == MFILEDIALOG_TYPE_SAVEFILE)? M_TR_T(M_TRSYS_SAVE): M_TR_T(M_TRSYS_OPEN));

	btt->wg.fState |= MWIDGET_STATE_ENTER_DEFAULT;

	//キャンセル

	mButtonCreate(ct, WID_CANCEL, 0, 0, 0, M_TR_T(M_TRSYS_CANCEL));
}


//============================
//
//============================


/** 解放処理 */

void mFileDialogDestroyHandle(mWidget *wg)
{
	mFree(M_FILEDIALOG(wg)->fdlg.filter);
}

/** 作成 */

mFileDialog *mFileDialogNew(int size,mWindow *owner,uint32_t style,int type)
{
	mFileDialog *p;
	
	if(size < sizeof(mFileDialog)) size = sizeof(mFileDialog);
	
	p = (mFileDialog *)mDialogNew(size, owner, MWINDOW_S_DIALOG_NORMAL);
	if(!p) return NULL;
	
	p->wg.destroy = mFileDialogDestroyHandle;
	p->wg.event = mFileDialogEventHandle;
	
	p->fdlg.style = style;
	p->fdlg.type = type;

	//ウィジェット作成

	M_TR_G(M_TRGROUP_SYS);

	_create_widget(p);
	
	return p;
}

/** 初期化 */

void mFileDialogInit(mFileDialog *p,const char *filter,int deftype,
	const char *initdir,mStr *strdst)
{
	int n;

	p->fdlg.strdst = strdst;

	//タイトル

	switch(p->fdlg.type)
	{
		case MFILEDIALOG_TYPE_SAVEFILE:
			n = M_TRSYS_TITLE_SAVEFILE;
			break;
		case MFILEDIALOG_TYPE_DIR:
			n = M_TRSYS_TITLE_SELECTDIR;
			break;
		default:
			n = M_TRSYS_TITLE_OPENFILE;
			break;
	}

	mWindowSetTitle(M_WINDOW(p), M_TR_T(n));

	//ファイル名 & フォーカス

	if(p->fdlg.type == MFILEDIALOG_TYPE_SAVEFILE)
	{
		//ファイル保存時

		mLineEditSetText(p->fdlg.editname, strdst->buf);

		mWidgetSetFocus(M_WIDGET(p->fdlg.editname));
		mLineEditSelectAll(p->fdlg.editname);
	}
	else
		mWidgetSetFocus(M_WIDGET(p->fdlg.flist));

	//種類

	if(p->fdlg.type != MFILEDIALOG_TYPE_DIR)
	{
		//フィルタ文字列、\t を 0 に変換

		p->fdlg.filter = mGetSplitCharReplaceStr(filter, '\t');

		//

		_set_combo_type(p->fdlg.cbtype, p->fdlg.filter, deftype);

		mFileListViewSetFilter(p->fdlg.flist,
			(const char *)mComboBoxGetItemParam(p->fdlg.cbtype, -1));
	}

	//ディレクトリセット
	//(指定なし、または指定ディレクトリが存在しない場合、ホームディレクトリ)

	if(initdir && !mIsFileExist(initdir, TRUE))
		initdir = NULL;
	
	mFileListViewSetDirectory(p->fdlg.flist, initdir);

	mLineEditSetText(p->fdlg.editdir, (p->fdlg.flist)->flv.strDir.buf);
}


//========================
// イベント
//========================


/** ディレクトリ変更時 */

static void _onChangeDir(mFileDialog *p)
{
	mLineEditSetText(p->fdlg.editdir, (p->fdlg.flist)->flv.strDir.buf);
	mLineEditCursorToRight(p->fdlg.editdir);
}

/** ディレクトリ名 ENTER 時 */

static void _dir_enter(mFileDialog *p)
{
	mStr str = MSTR_INIT;

	mLineEditGetTextStr(p->fdlg.editdir, &str);

	mFileListViewSetDirectory(p->fdlg.flist, str.buf);

	mStrFree(&str);

	_onChangeDir(p);
}

/** 選択ファイル変更時 */

static void _onChangeSelFile(mFileDialog *p)
{
	int type;
	mStr str = MSTR_INIT;

	type = mFileListViewGetSelectFileName(p->fdlg.flist, &str, FALSE);

	if(type == MFILELISTVIEW_FILETYPE_FILE)
	{
		//ファイル名セット
		
		if(p->fdlg.type == MFILEDIALOG_TYPE_SAVEFILE)
			mLineEditSetText(p->fdlg.editname, str.buf);

		//ハンドラ

		if(p->fdlg.onSelectFile)
			(p->fdlg.onSelectFile)(p, str.buf);
	}

	mStrFree(&str);
}

/** 終了 (キャンセル) */

static void _end_cancel(mFileDialog *p)
{
	if(p->fdlg.onOkCancel)
	{
		if(!(p->fdlg.onOkCancel)(p, FALSE, NULL))
			return;
	}

	mDialogEnd(M_DIALOG(p), FALSE);
}

/** 終了 (OK) */

static void _end_ok(mFileDialog *p)
{
	mStr str = MSTR_INIT,str2 = MSTR_INIT;
	int n;

	/* str に結果文字列をセット。
	 * str がセットされていない場合、終了しない */

	switch(p->fdlg.type)
	{
		//ディレクトリ
		/* ディレクトリが選択されていればそのパス */
		case MFILEDIALOG_TYPE_DIR:
			n = mFileListViewGetSelectFileName(p->fdlg.flist, &str, TRUE);

			if(n != MFILELISTVIEW_FILETYPE_DIR)
				mStrCopy(&str, &(p->fdlg.flist)->flv.strDir);
			break;
		//開く
		case MFILEDIALOG_TYPE_OPENFILE:
			if(p->fdlg.style & MFILEDIALOG_S_MULTI_SEL)
			{
				n = mFileListViewGetSelectMultiName(p->fdlg.flist, &str2);
				if(n == 0) break;
			}
			else
			{
				n = mFileListViewGetSelectFileName(p->fdlg.flist, &str2, TRUE);
				if(n != MFILELISTVIEW_FILETYPE_FILE) break;
			}

			mStrCopy(&str, &str2);
			break;
		//保存
		case MFILEDIALOG_TYPE_SAVEFILE:
			//ファイル名
			
			mLineEditGetTextStr(p->fdlg.editname, &str2);
			if(mStrIsEmpty(&str2)) break;

			//ファイル名が正しいか

			if(!mPathIsEnableFilename(str2.buf))
			{
				mMessageBoxErrTr(M_WINDOW(p), M_TRGROUP_SYS, M_TRSYS_MES_FILENAME_INCORRECT);
				break;
			}

			//絶対パス

			mStrCopy(&str, &(p->fdlg.flist)->flv.strDir);
			mStrPathAdd(&str, str2.buf);

			//上書き確認

			if(!(p->fdlg.style & MFILEDIALOG_S_NO_OVERWRITE_MES)
				&& mIsFileExist(str.buf, FALSE))
			{
				if(mMessageBox(M_WINDOW(p), NULL,
						M_TR_T2(M_TRGROUP_SYS, M_TRSYS_MES_OVERWRITE),
						MMESBOX_YES | MMESBOX_NO, MMESBOX_NO) == MMESBOX_NO)
				{
					mStrFree(&str);
					break;
				}
			}

			//ファイル種類

			if(p->fdlg.filtertypeDst)
				*(p->fdlg.filtertypeDst) = mComboBoxGetSelItemIndex(p->fdlg.cbtype);
			break;
	}

	mStrFree(&str2);

	if(!str.buf) return;

	//ハンドラ

	if(p->fdlg.onOkCancel)
	{
		if(!(p->fdlg.onOkCancel)(p, TRUE, str.buf))
		{
			mStrFree(&str);
			return;
		}
	}

	//結果パス

	mStrCopy(p->fdlg.strdst, &str);
	mStrFree(&str);

	//ダイアログ終了

	mDialogEnd(M_DIALOG(p), TRUE);
}

/** NOTIFY */

static void _event_notify(mFileDialog *p,mEvent *ev)
{
	int id = ev->notify.widgetFrom->id,
		type = ev->notify.type;

	if(id == WID_FILELIST)
	{
		//ファイルリスト

		switch(type)
		{
			case MFILELISTVIEW_N_SELECT_FILE:
				_onChangeSelFile(p);
				break;
			case MFILELISTVIEW_N_CHANGE_DIR:
				_onChangeDir(p);
				break;
			case MFILELISTVIEW_N_DBLCLK_FILE:
				_end_ok(p);
				break;
		}
	}
	else
	{
		switch(id)
		{
			//ディレクトリ名
			case WID_EDIT_DIR:
				if(type == MLINEEDIT_N_ENTER)
					_dir_enter(p);
				break;
			//種類
			case WID_TYPE:
				if(type == MCOMBOBOX_N_CHANGESEL)
				{
					mFileListViewSetFilter(p->fdlg.flist,
						(const char *)mComboBoxGetItemParam(p->fdlg.cbtype, -1));

					mFileListViewUpdateList(p->fdlg.flist);

					//種類変更時ハンドラ

					if(p->fdlg.onChangeType)
						(p->fdlg.onChangeType)(p, mComboBoxGetSelItemIndex(p->fdlg.cbtype));
				}
				break;
			//home
			case WID_HOME:
				mFileListViewSetDirectory(p->fdlg.flist, NULL);
				_onChangeDir(p);
				break;
			//OK
			case WID_OK:
				_end_ok(p);
				break;
			//キャンセル
			case WID_CANCEL:
				_end_cancel(p);
				break;
		}
	}
}

/** イベント */

int mFileDialogEventHandle(mWidget *wg,mEvent *ev)
{
	if(ev->type == MEVENT_NOTIFY)
		_event_notify(M_FILEDIALOG(wg), ev);
	else if(ev->type == MEVENT_CLOSE)
	{
		//閉じるボタン
		
		_end_cancel(M_FILEDIALOG(wg));
		return TRUE;
	}

	return mDialogEventHandle(wg, ev);
}

/* @} */
