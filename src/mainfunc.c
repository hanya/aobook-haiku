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
 * メイン処理関数
 *****************************************/

#include "mDef.h"
#include "mStr.h"
#include "mGui.h"
#include "mWidget.h"
#include "mWindow.h"
#include "mMessageBox.h"
#include "mPopupProgress.h"
#include "mFileList.h"
#include "mUtilFile.h"
#include "mUtilSys.h"

#include "globaldata.h"
#include "mainfunc.h"
#include "mainwindow.h"
#include "trgroup.h"
#include "trid.h"

#include "aoText.h"
#include "aoLayout.h"


//----------------------

enum
{
	_LOADTEXT_ERR_LOADFILE = -1,
	_LOADTEXT_ERR_CONVCODE = -2,
	_LOADTEXT_ERR_BUF = -3
};

//unzip.c
mBool extractZipFile(const char *filename,mBuf *dstbuf);

//----------------------


//==========================
// sub
//==========================


/** ZIP ファイル判定 */

static mBool _is_zipfile(const char *filename)
{
	uint8_t b[4];

	if(!mReadFileHead(filename, b, 4)) return FALSE;

	//ローカルファイルヘッダまたはセントラルディレクトリヘッダ

	if(b[0] == 0x50 && b[1] == 0x4b)
	{
		if((b[2] == 0x03 && b[3] == 0x04)
			|| (b[2] == 0x01 && b[3] == 0x02))
			return TRUE;
	}

	return FALSE;
}

/** テキストファイル読み込み
 *
 * @return >= 0 で実際の文字コード。< 0 でエラーコード */

int _loadTextFile(const char *filename,int code)
{
	mBuf tbuf,unibuf,cbuf;
	mBool ret;
	int rescode;

	//-------- データ変換

	//ソーステキスト読み込み

	if(_is_zipfile(filename))
		ret = extractZipFile(filename, &tbuf);
	else
		ret = mReadFileFull(filename, 0, &tbuf);

	if(!ret)
		return _LOADTEXT_ERR_LOADFILE;

	//Unicode に変換

	rescode = aoTextConvUnicode(&tbuf, &unibuf, code);

	mFree(tbuf.buf);
	
	if(rescode < 0)
		return _LOADTEXT_ERR_CONVCODE;

	//内部データに変換 (unibuf は自動で解放される)

	if(!aoTextConvert(&unibuf, &cbuf))
		return _LOADTEXT_ERR_BUF;

	//--------- データセット

	GlobalCloseFile();
	GlobalEmptyTextData();

	GlobalSetTextData(&cbuf);

	return rescode;
}

/** レイアウトセットのスレッド処理 */

static void _thread_setlayout(mThread *th)
{
	aoSetLayoutFirst(GDAT->layout, (mPopupProgress *)th->param);

	mPopupProgressThreadEnd();
}

/** 最初のレイアウト情報セット (読み込み/スタイル変更時) */

static void _setFirstLayout()
{
	mPopupProgress *p;
	int w;
	mBox box;

	w = GDAT->mainwin->wg.w;
	if(w > 200) w = 200;

	p = mPopupProgressNew(0, w, MPROGRESSBAR_S_FRAME);
	if(!p) return;

	mWidgetGetRootBox(M_WIDGET(GDAT->mainwin), &box);

	GDAT->bNowThread = TRUE;
	mPopupProgressRun(p, box.x, box.y + box.h - p->wg.h, _thread_setlayout);
	GDAT->bNowThread = FALSE;

	mWidgetDestroy(M_WIDGET(p));
}


//==========================
// メイン
//==========================


/** ファイル再読み込み */

void mfReloadFile()
{
	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	mfLoadTextFile(GDAT->strFileName.buf,
		GDAT->charcode, aoGetPageNo(GDAT->curpage), TRUE);
}

/** テキストファイル読み込み
 *
 * @param linepage reload = TRUE の場合、ページ数。ほかは行数 */

void mfLoadTextFile(const char *filename,int code,int linepage,mBool reload)
{
	int ret,n;

	//テキスト読み込み

	ret = _loadTextFile(filename, code);

	if(ret < 0)
	{
		//------ エラー

		switch(ret)
		{
			case _LOADTEXT_ERR_CONVCODE: n = TRMES_ERR_CONVCODE; break;
			case _LOADTEXT_ERR_BUF: n = TRMES_ERR_BUF; break;
			default: n = TRMES_ERR_LOADFILE; break;
		}

		mMessageBoxErrTr(M_WINDOW(GDAT->mainwin), TRGROUP_MESSAGE, n);
	}
	else
	{
		//-------- OK

		if(!reload)
		{
			//ファイル情報セット

			mStrSetText(&GDAT->strFileName, filename);

			GDAT->charcode = ret;

			GlobalSetLayoutFilePath();

			//履歴セット

			GlobalAddRecentFile(filename, code, linepage);

			MainWindow_setRecentFileMenu(GDAT->mainwin);
		}

		//レイアウト

		_setFirstLayout();

		//ページ位置

		if(reload)
			GDAT->curpage = aoGetPageIndex(GDAT->layout, linepage);
		else
			GDAT->curpage = aoGetPageIndexByLine(GDAT->layout, linepage);

		//更新

		mfSetWindowTitle();

		mfUpdateScreen();
	}
}

/** ファイル履歴から読み込み */

void mfLoadTextFileFromRecent(int no)
{
	mStr str = MSTR_INIT;
	int code,line;

	GlobalGetRecentFileInfo(no, &str, &code, &line);

	mfLoadTextFile(str.buf, code, line, FALSE);

	mStrFree(&str);
}

/** 次/前のファイル読み込み */

void mfLoadNextPrevFile(mBool prev)
{
	mList list = MLIST_INIT;
	mStr dir = MSTR_INIT,fname = MSTR_INIT;
	mFileListItem *pi;

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	//ディレクトリとファイル名

	mStrPathGetDir(&dir, GDAT->strFileName.buf);
	mStrPathGetFileName(&fname, GDAT->strFileName.buf);

	//ファイルリスト作成

	mFileListGetList(&list, dir.buf, mFileListFunc_notdir);

	mFileListSortName(&list);

	//現在のファイルを検索、前後のファイル名を取得

	pi = mFileListFindByName(&list, fname.buf);

	if(pi)
	{
		pi = (mFileListItem *)((prev)? pi->i.prev: pi->i.next);

		if(pi) mStrPathAdd(&dir, pi->fname);
	}

	mListDeleteAll(&list);
	mStrFree(&fname);

	//読み込み

	if(pi)
		mfLoadTextFile(dir.buf, -1, 0, FALSE);

	//

	mStrFree(&dir);
}

/** 再レイアウト (スタイル変更時) */

void mfReLayout()
{
	int line;

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	line = aoGetPageLineNo(GDAT->curpage);

	//再レイアウト

	_setFirstLayout();

	//ページ位置

	GDAT->curpage = aoGetPageIndexByLine(GDAT->layout, line);

	//更新

	mfUpdateScreen();
}

/** 画面更新 */

void mfUpdateScreen()
{
	mWidgetUpdate(GDAT->mainwin->main.widgetDraw);
}

/** スタイル変更後の更新 */

void mfUpdateChangeStyle(mBool relayout)
{
	MainWindow *p = GDAT->mainwin;
	mWidget *wgdraw;
	mSize size;

	//画面サイズ変更

	wgdraw = p->main.widgetDraw;

	aoGetScreenSize(GDAT->layout, &size);

	if(wgdraw->w != size.w || wgdraw->h != size.h)
	{
		wgdraw->hintW = size.w;
		wgdraw->hintH = size.h;

		mWidgetCalcHintSize(wgdraw);
		
		mGuiCalcHintSize();

		mWidgetResize(M_WIDGET(p), p->wg.hintW, p->wg.hintH);
	}

	//再レイアウト

	if(relayout)
		mfReLayout();

	mfUpdateScreen();
}

/** ウィンドウタイトルセット */

void mfSetWindowTitle()
{
	mStr str = MSTR_INIT;

	if(mStrIsEmpty(&GDAT->strFileName))
		mStrSetText(&str, APPNAME);
	else
	{
		//ファイル名+文字コード
		
		mStrPathGetFileName(&str, GDAT->strFileName.buf);
		mStrAppendText(&str, " [");
		mStrAppendText(&str, aoTextGetCodeName(GDAT->charcode));
		mStrAppendChar(&str, ']');
	}

	mWindowSetTitle(M_WINDOW(GDAT->mainwin), str.buf);

	mStrFree(&str);
}

/** ページ移動 (ページ番号、またはコマンド) */

void mfMovePage(int page)
{
	AO_LAYOUT_INFO *layout = GDAT->layout;
	AO_PAGEINDEX *p;

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	if(page >= 0)
		p = aoGetPageIndex(layout, page);
	else
	{
		switch(page)
		{
			case PAGENO_NEXT:
			case PAGENO_PREV:
				p = aoMovePage(layout, GDAT->curpage, (page == PAGENO_NEXT)? 1: -1);
				break;
			case PAGENO_HOME:
			case PAGENO_END:
				p = aoGetPageHomeEnd(layout, (page == PAGENO_END));
				break;
		}
	}

	GDAT->curpage = p;

	mfUpdateScreen();
}

/** ページ移動 (行番号) */

void mfMovePageToLine(int no)
{
	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	GDAT->curpage = aoGetPageIndexByLine(GDAT->layout, no);

	mfUpdateScreen();
}

/** ツールを実行 */

void mfExecTool(int no)
{
	mStr str = MSTR_INIT,str2 = MSTR_INIT,strrep[3];

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	//コマンド文字列

	mStrGetSplitText(&str, GDAT->strTool[no].buf, '\t', 1);

	//置き換え対象

	mStrArrayInit(strrep, 3);

	mStrEscapeCmdline(&str2, GDAT->strFileName.buf);

	mStrSetFormat(strrep, "f%s", str2.buf);
	mStrSetFormat(strrep + 1, "l%d", aoGetPageLineNo(GDAT->curpage) + 1);
	mStrSetFormat(strrep + 2, "c%s", aoTextGetCodeName_conv(GDAT->charcode));

	mStrFree(&str2);

	//% の部分を置換え

	mStrReplaceParams(&str, '%', strrep, 3);
	mStrArrayFree(strrep, 3);

	//実行

	if(!mStrIsEmpty(&str))
		mExec(str.buf);
	
	//

	mStrFree(&str);
}
