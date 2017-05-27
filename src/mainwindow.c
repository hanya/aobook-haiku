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
 * MainWindow [メインウィンドウ]
 *****************************************/

#include "mDef.h"

#include "mGui.h"
#include "mStr.h"
#include "mWidget.h"
#include "mWindow.h"
#include "mEvent.h"
#include "mMenuBar.h"
#include "mMenu.h"
#include "mAccelerator.h"
#include "mTrans.h"
#include "mPixbuf.h"
#include "mSysDialog.h"
#include "mDirEntry.h"
#include "mImageBuf.h"

#include "trgroup.h"
#include "trid.h"
#include "trid_menu.h"

#include "globaldata.h"
#include "style.h"
#include "mainwindow.h"
#include "bookmarkwin.h"
#include "mainfunc.h"
#include "envoptdlg.h"

#include "aoLayout.h"
#include "aoText.h"

#include "menudat.h"
#include "appicon.h"


//----------------------

#define _VERSION_TEXT  APPNAME " ver 1.0.3\n\nCopyright (C) 2014-2017 Azel"

#define CMDID_RECENTFILE  10000	//最近開いたファイル
#define CMDID_TOOL        11000	//ツール
#define CMDID_STYLE       12000	//スタイル

//----------------------

static int _event_handle(mWidget *wg,mEvent *ev);
static int _ondnd_handle(mWidget *wg,char **files);

static void _drawwg_draw_handle(mWidget *wg,mPixbuf *pixbuf);
static int _drawwg_event_handle(mWidget *wg,mEvent *ev);

//filedialog.c
mBool GetOpenTextFile(mWindow *owner,const char *initdir,mStr *strdst,int *retcode);

//captiondlg.c
int GetCaptionPageDlg();

//styleoptdlg.c
int StyleOptDialog(mWindow *owner);

//----------------------


//========================
// sub
//========================


/** アクセラレータのキーをセット */

static void _set_accelkey(MainWindow *p)
{
	mAccelerator *acc;
	uint32_t *ps;
	mStr str = MSTR_INIT;
	int i,key;

	acc = p->win.accelerator;

	mAcceleratorDeleteAll(acc);

	//メインメニュー

	for(ps = GDAT->shortcutkey; *ps; ps++)
		mAcceleratorAdd(acc, *ps & 0xffff, *ps >> 16, NULL);

	//ツール

	for(i = 0; i < TOOLITEM_NUM; i++)
	{
		if(mStrIsEmpty(GDAT->strTool + i)) break;

		mStrGetSplitText(&str, GDAT->strTool[i].buf, '\t', 2);
		key = mStrToInt(&str);
		
		if(key)
			mAcceleratorAdd(acc, CMDID_TOOL + i, key, NULL);
	}

	mStrFree(&str);
}

/** ショートカットキー変更時 */

static void _change_shortcutkey(MainWindow *p,uint32_t *newkey)
{
	uint32_t *curkey,*pcur,*pnew,id;
	mMenu *menu;

	menu = mWindowGetMenuInMenuBar(M_WINDOW(p));
	curkey = GDAT->shortcutkey;

	//新しいデータには存在しない ID を削除

	for(pcur = curkey; *pcur; pcur++)
	{
		id = *pcur & 0xffff;
		
		for(pnew = newkey; *pnew && id != (*pnew & 0xffff); pnew++);

		if(*pnew == 0)
			mMenuSetShortcutKey(menu, id, 0);
	}

	//追加または変更されたキーをセット

	for(pnew = newkey; *pnew; pnew++)
	{
		id = *pnew & 0xffff;

		for(pcur = curkey; *pcur && id != (*pcur & 0xffff); pcur++);

		if(*pcur == 0 || (*pnew >> 16) != (*pcur >> 16))
			mMenuSetShortcutKey(menu, id, *pnew >> 16);
	}

	//データを入れ替え

	mFree(GDAT->shortcutkey);

	GDAT->shortcutkey = newkey;
}

/** ツールのメニュー項目セット */

static void _set_tool_menu(MainWindow *p)
{
	mStr str = MSTR_INIT;
	int i,key;
	mMenu *menu;

	//メニュー

	menu = mWindowGetMenuInMenuBar(M_WINDOW(p));
	menu = mMenuGetSubMenu(menu, TRMENU_OPT_TOOL_SUB);

	//セット

	mMenuDeleteAll(menu);

	for(i = 0; i < TOOLITEM_NUM; i++)
	{
		if(mStrIsEmpty(GDAT->strTool + i)) break;

		//キー

		mStrGetSplitText(&str, GDAT->strTool[i].buf, '\t', 2);
		key = mStrToInt(&str);

		//ラベル

		mStrGetSplitText(&str, GDAT->strTool[i].buf, '\t', 0);

		//

		mMenuAddNormal(menu, CMDID_TOOL + i, str.buf, key, MMENUITEM_F_LABEL_COPY);
	}

	mStrFree(&str);
}

/** スタイルのメニュー項目をセット */

static void _set_style_menu(MainWindow *p)
{
	mMenu *menu;
	mDirEntry *dir;
	mStr str = MSTR_INIT;
	int cnt = 0;
	mBool bsel = FALSE;
	uint32_t flags;

	menu = p->main.menuStyle;

	mMenuDeleteAll(menu);

	//セット (style ディレクトリ内の .conf ファイル)

	mAppGetConfigPath(&str, "style");

	dir = mDirEntryOpen(str.buf);
	if(dir)
	{
		while(mDirEntryRead(dir) && cnt < STYLE_MAXNUM)
		{
			if(mDirEntryIsDirectory(dir)) continue;

			if(mDirEntryIsEqExt(dir, "conf"))
			{
				mStrPathGetFileNameNoExt(&str, mDirEntryGetFileName(dir));

				flags = MMENUITEM_F_LABEL_COPY | MMENUITEM_F_RADIO | MMENUITEM_F_AUTOCHECK;

				if(!bsel && mStrPathCompareEq(&str, GDAT->style->strName.buf))
				{
					flags |= MMENUITEM_F_CHECKED;
					bsel = TRUE;
				}

				mMenuAddNormal(menu, CMDID_STYLE + cnt, str.buf, 0, flags);

				cnt++;
			}
		}

		mDirEntryClose(dir);
	}

	mStrFree(&str);

	//設定ファイルが見つからなかった場合

	if(!bsel)
	{
		mMenuAddNormal(menu, CMDID_STYLE + cnt,
			GDAT->style->strName.buf, 0,
			MMENUITEM_F_LABEL_COPY | MMENUITEM_F_RADIO | MMENUITEM_F_AUTOCHECK | MMENUITEM_F_CHECKED);
	}
}


//=========================
// create - sub
//=========================


/** メニュー作成 */

static void _create_menu(MainWindow *p)
{
	mMenuBar *bar;
	mMenu *menu;
	uint32_t *ps;

	bar = mMenuBarNew(0, M_WIDGET(p), 0);

	p->win.menubar = bar;

	//データからセット

	M_TR_G(TRGROUP_MENU);

	mMenuBarCreateMenuTrArray16(bar, g_menudat, 1000);

	//ショートカットキーセット

	for(ps = GDAT->shortcutkey; *ps; ps++)
		mMenuSetShortcutKey(bar->mb.menu, *ps & 0xffff, *ps >> 16);

	//ファイル履歴サブメニュー

	menu = mMenuNew();
	p->main.menuRecentFile = menu;

	mMenuBarSetItemSubmenu(bar, TRMENU_FILE_RECENTFILE, menu);

	MainWindow_setRecentFileMenu(p);

	//ツールサブニュー

	_set_tool_menu(p);

	//スタイルメニュー

	menu = mWindowGetMenuInMenuBar(M_WINDOW(p));
	p->main.menuStyle = mMenuGetSubMenu(menu, TRMENU_OPT_STYLE_SUB);

	_set_style_menu(p);
}

/** アクセラレータ作成 */

static void _create_accel(MainWindow *p)
{
	p->win.accelerator = mAcceleratorNew();

	mAcceleratorSetDefaultWidget(p->win.accelerator, M_WIDGET(p));

	_set_accelkey(p);
}


//=========================
// main
//=========================


/** 解放処理 */

static void _destroy_handle(mWidget *p)
{
	mAcceleratorDestroy(M_WINDOW(p)->win.accelerator);
}

/** 作成 */

MainWindow *MainWindowNew()
{
	MainWindow *p;
	mWidget *wg;

	//ウィンドウ
	
	p = (MainWindow *)mWindowNew(sizeof(MainWindow), NULL,
			MWINDOW_S_TITLE | MWINDOW_S_BORDER | MWINDOW_S_CLOSE
			| MWINDOW_S_SYSMENU | MWINDOW_S_MINIMIZE | MWINDOW_S_NO_IM);
	if(!p) return NULL;
	
	GDAT->mainwin = p;

	p->wg.destroy = _destroy_handle;
	p->wg.event = _event_handle;
	p->wg.onDND = _ondnd_handle;

	p->wg.fState |= MWIDGET_STATE_ENABLE_DROP;
	p->wg.fOption |= MWIDGET_OPTION_NO_DRAW_BKGND;

	//タイトル

	mWindowSetTitle(M_WINDOW(p), APPNAME);

	//アイコン

	mWindowSetIconFromBufPNG(M_WINDOW(p), g_appicon, sizeof(g_appicon));

	//D&D有効

	mWindowEnableDND(M_WINDOW(p));

	//メニュー

	_create_menu(p);

	//アクセラレータ

	_create_accel(p);

	//----- 描画エリアのウィジェット作成

	wg = mWidgetNew(0, M_WIDGET(p));

	p->main.widgetDraw = wg;

	wg->fLayout = MLF_EXPAND_WH;
	wg->fEventFilter |= MWIDGET_EVENTFILTER_POINTER | MWIDGET_EVENTFILTER_SCROLL;
	wg->event = _drawwg_event_handle;
	wg->draw = _drawwg_draw_handle;

	return p;
}

/** 初期表示 */

void MainWindow_show(MainWindow *p,mPoint *pt,int maximize)
{
	mSize size;

	//描画用ウィジェットのサイズ

	aoGetScreenSize(GDAT->layout, &size);

	p->main.widgetDraw->hintW = size.w;
	p->main.widgetDraw->hintH = size.h;

	//ウィンドウサイズ

	mGuiCalcHintSize();

	mWidgetResize(M_WIDGET(p), p->wg.hintW, p->wg.hintH);

	//表示

	mWindowShowInitPos(M_WINDOW(p), pt, -1, -1, -10000, TRUE, maximize);
}

/** ファイル履歴のメニューをセット */

void MainWindow_setRecentFileMenu(MainWindow *p)
{
	int i,code,line;
	mStr str = MSTR_INIT,fname = MSTR_INIT;

	//クリア

	mMenuDeleteAll(p->main.menuRecentFile);

	//セット

	for(i = 0; i < RECENTFILE_NUM; i++)
	{
		if(mStrIsEmpty(GDAT->strRecentFile + i)) break;

		GlobalGetRecentFileInfo(i, &fname, &code, &line);

		mStrSetFormat(&str, "%s [%s] %d",
			fname.buf, aoTextGetCodeName(code), line + 1);
		
		mMenuAddText_copy(p->main.menuRecentFile, CMDID_RECENTFILE + i, str.buf);
	}

	mStrFree(&str);
	mStrFree(&fname);
}


//========================
// コマンド
//========================


/** 開く */

static void _cmd_open(mWidget *wg)
{
	mStr str = MSTR_INIT;
	int code;

	//ファイル名取得

	if(!GetOpenTextFile(M_WINDOW(wg), GDAT->strOpenPath.buf, &str, &code))
		return;

	//ディレクトリを記録

	mStrPathGetDir(&GDAT->strOpenPath, str.buf);

	//読み込み

	mfLoadTextFile(str.buf, code, 0, FALSE);

	mStrFree(&str);
}

/** ページ番号指定 */

static void _cmd_move_pageno(mWidget *wg)
{
	int no;
	mStr str = MSTR_INIT;

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	M_TR_G(TRGROUP_DIALOG);

	mStrSetFormat(&str, "%s (%d-%d)",
		M_TR_T(TRDLG_MOVEPAGENO_MES), 1, GDAT->layout->pagenum);
	
	if(mSysDlgInputNum(M_WINDOW(wg),
		M_TR_T(TRDLG_MOVEPAGENO_TITLE), str.buf,
		&no, 1, GDAT->layout->pagenum, 0))
	{
		mfMovePage(no - 1);
	}

	mStrFree(&str);
}

/** 行番号指定 */

static void _cmd_move_lineno(mWidget *wg)
{
	int no;

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	M_TR_G(TRGROUP_DIALOG);

	if(mSysDlgInputNum(M_WINDOW(wg),
		M_TR_T(TRDLG_MOVELINENO_TITLE), M_TR_T(TRDLG_MOVELINENO_MES),
		&no, 1, INT32_MAX, 0))
	{
		mfMovePageToLine(no - 1);
	}
}

/** 見出し */

static void _cmd_move_caption(mWidget *wg)
{
	int page;

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	page = GetCaptionPageDlg();
	if(page != -1)
		mfMovePage(page);
}

/** スタイル変更 */

static void _cmd_style_change(MainWindow *p,int no)
{
	mBool relayout;

	relayout = StyleChangeByName(mMenuGetTextByIndex(p->main.menuStyle, no));

	mfUpdateChangeStyle(relayout);
}

/** 環境設定 */

static void _cmd_opt_env(MainWindow *p)
{
	int f;
	uint32_t *newkey;

	f = EnvOptDialog(M_WINDOW(p), &newkey);

	//ショートカットキー

	if(f & ENVOPTDLG_UPDATE_KEY)
		_change_shortcutkey(p, newkey);

	//アクセラレータ

	if(f & (ENVOPTDLG_UPDATE_KEY | ENVOPTDLG_UPDATE_TOOL))
		_set_accelkey(p);

	//ツール・メニュー

	if(f & ENVOPTDLG_UPDATE_TOOL)
		_set_tool_menu(p);
}

/** スタイル設定 */

static void _cmd_opt_style(MainWindow *p)
{
	int ret;

	ret = StyleOptDialog(M_WINDOW(p));
	if(!ret) return;

	//メニュー変更

	if(ret & (1<<8))
	{
		_set_style_menu(p);

		ret &= ~(1<<8);
	}

	//更新

	mfUpdateChangeStyle((ret == 2));
}


//========================
// イベント
//========================


/** COMMAND イベント */

static void _event_command(mWidget *wg,mEvent *ev)
{
	int id = ev->cmd.id;

	//ファイル履歴

	if(id >= CMDID_RECENTFILE && id < CMDID_RECENTFILE + RECENTFILE_NUM)
	{
		mfLoadTextFileFromRecent(id - CMDID_RECENTFILE);
		return;
	}

	//ツール

	if(id >= CMDID_TOOL && id < CMDID_TOOL + TOOLITEM_NUM)
	{
		mfExecTool(id - CMDID_TOOL);
		return;
	}

	//スタイル

	if(id >= CMDID_STYLE && id < CMDID_STYLE + STYLE_MAXNUM)
	{
		_cmd_style_change(MAINWINDOW(wg), id - CMDID_STYLE);
		return;
	}

	//

	switch(id)
	{
		//開く
		case TRMENU_FILE_OPEN:
			_cmd_open(wg);
			break;
		//再読み込み
		case TRMENU_FILE_RELOAD:
			mfReloadFile();
			break;
		//次のファイル
		case TRMENU_FILE_NEXTFILE:
			mfLoadNextPrevFile(FALSE);
			break;
		//前のファイル
		case TRMENU_FILE_PREVFILE:
			mfLoadNextPrevFile(TRUE);
			break;
		//終了
		case TRMENU_FILE_EXIT:
			mAppQuit();
			break;

		//次のページ
		case TRMENU_MOVE_NEXT:
			mfMovePage(PAGENO_NEXT);
			break;
		//前のページ
		case TRMENU_MOVE_PREV:
			mfMovePage(PAGENO_PREV);
			break;
		//先頭ページ
		case TRMENU_MOVE_TOP:
			mfMovePage(PAGENO_HOME);
			break;
		//終端ページ
		case TRMENU_MOVE_BOTTOM:
			mfMovePage(PAGENO_END);
			break;
		//ページ番号指定
		case TRMENU_MOVE_PAGENO:
			_cmd_move_pageno(wg);
			break;
		//行番号指定
		case TRMENU_MOVE_LINENO:
			_cmd_move_lineno(wg);
			break;
		//見出し
		case TRMENU_MOVE_CAPTION:
			_cmd_move_caption(wg);
			break;

		//しおり一覧
		case TRMENU_BM_LIST:
			BookmarkWinNew();
			break;
		//グローバルに追加
		case TRMENU_BM_ADD_GLOBAL:
			BookmarkWinAddGlobal(GDAT->bkmarkwin);
			break;
		//ローカルに追加
		case TRMENU_BM_ADD_LOCAL:
			BookmarkWinAddLocal(GDAT->bkmarkwin);
			break;

		//環境設定
		case TRMENU_OPT_ENV:
			_cmd_opt_env(MAINWINDOW(wg));
			break;
		//スタイル設定
		case TRMENU_OPT_STYLE:
			_cmd_opt_style(MAINWINDOW(wg));
			break;

		//バージョン情報
		case TRMENU_HELP_ABOUT:
			mSysDlgAbout(M_WINDOW(wg), _VERSION_TEXT);
			break;
	}
}

/** イベントハンドラ */

int _event_handle(mWidget *wg,mEvent *ev)
{
	switch(ev->type)
	{
		//コマンド
		case MEVENT_COMMAND:
			_event_command(wg, ev);
			break;
	
		//閉じるボタン
		case MEVENT_CLOSE:
			mAppQuit();
			break;
		
		default:
			return FALSE;
	}

	return TRUE;
}

/** D&D */

static int _ondnd_handle(mWidget *wg,char **files)
{
	if(*files)
		mfLoadTextFile(*files, -1, 0, FALSE);

	return 1;
}


//==============================
// 描画エリアのウィジェット
//==============================


/** マウス/ホイール */

static void _drawwg_event_pointer(mEvent *ev)
{
	int btt = -1;

	//ボタン番号 取得

	if(ev->type == MEVENT_SCROLL)
	{
		if(ev->scr.dir == MEVENT_SCROLL_DIR_UP)
			btt = MOUSECTRL_BTT_SCROLL_UP;
		else if(ev->scr.dir == MEVENT_SCROLL_DIR_DOWN)
			btt = MOUSECTRL_BTT_SCROLL_DOWN;
	}
	else if(ev->pt.type == MEVENT_POINTER_TYPE_PRESS
		|| ev->pt.type == MEVENT_POINTER_TYPE_DBLCLK)
	{
		if(ev->pt.btt == M_BTT_LEFT)
			btt = MOUSECTRL_BTT_LEFT;
		else if(ev->pt.btt == M_BTT_RIGHT)
			btt = MOUSECTRL_BTT_RIGHT;
	}

	if(btt == -1) return;

	//コマンド

	switch(GDAT->mousectrl[btt])
	{
		case MOUSECTRL_CMD_NEXTPAGE:
			mfMovePage(PAGENO_NEXT);
			break;
		case MOUSECTRL_CMD_PREVPAGE:
			mfMovePage(PAGENO_PREV);
			break;
	}
}

/** イベントハンドラ */

int _drawwg_event_handle(mWidget *wg,mEvent *ev)
{
	switch(ev->type)
	{
		case MEVENT_POINTER:
			_drawwg_event_pointer(ev);
			break;
		case MEVENT_SCROLL:
			_drawwg_event_pointer(ev);
			break;
	
		default:
			return FALSE;
	}

	return TRUE;
}

/** 描画ハンドラ */

void _drawwg_draw_handle(mWidget *wg,mPixbuf *pixbuf)
{
	GlobalData *g = GDAT;

	if(g->bNowThread) return;

	//背景

	if(!g->style->imgBkgnd)
		mPixbufFillBox(pixbuf, 0, 0, wg->w, wg->h, mRGBtoPix(g->style->colBkgnd));
	else
	{
		if(g->style->bkgnd_imgtype == 0)
			mPixbufScaleImageBuf_oversamp(pixbuf, 0, 0, wg->w, wg->h, g->style->imgBkgnd, 5);
		else
			mPixbufTileImageBuf(pixbuf, 0, 0, wg->w, wg->h, g->style->imgBkgnd);
	}

	//ページ描画

	aoDrawPage(g->layout, pixbuf, g->curpage);
}
