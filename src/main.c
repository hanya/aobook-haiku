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

/**********************************
 * main
 **********************************/

#include <stdio.h>
#include <string.h>

#include "mDef.h"
#include "mAppDef.h"
#include "mGui.h"
#include "mStr.h"
#include "mWidget.h"
#include "mWindow.h"
#include "mIniRead.h"
#include "mIniWrite.h"
#include "mKeyDef.h"

#include "globaldata.h"
#include "style.h"
#include "mainfunc.h"
#include "mainwindow.h"
#include "bookmarkwin.h"
#include "bookmarkdat.h"
#include "trid_menu.h"

#include "aoLayout.h"
#include "aoFont.h"

#include "deftrans.h"


//----------------------

#define CONFIGFILENAME  "aobook.conf"

GlobalData *g_globaldat;

//----------------------

#define _SCVAL(id,key,mod)  id | ((key | mod) << 16)

static uint32_t g_shortcutkey_def[] = {
	_SCVAL(TRMENU_FILE_OPEN, 'O', MACCKEY_CTRL),
	_SCVAL(TRMENU_FILE_RELOAD, MKEY_F5, 0),
	_SCVAL(TRMENU_FILE_NEXTFILE, 'N', MACCKEY_CTRL),
	_SCVAL(TRMENU_FILE_PREVFILE, 'P', MACCKEY_CTRL),
	_SCVAL(TRMENU_MOVE_NEXT, MKEY_LEFT, 0),
	_SCVAL(TRMENU_MOVE_PREV, MKEY_RIGHT, 0),
	_SCVAL(TRMENU_MOVE_TOP, MKEY_HOME, 0),
	_SCVAL(TRMENU_MOVE_BOTTOM, MKEY_END, 0),
	_SCVAL(TRMENU_MOVE_PAGENO, 'P', 0),
	_SCVAL(TRMENU_MOVE_LINENO, 'L', 0),
	_SCVAL(TRMENU_MOVE_CAPTION, 'C', 0),
	_SCVAL(TRMENU_BM_LIST, 'B', 0),
	_SCVAL(TRMENU_OPT_STYLE, 'S', 0),
	_SCVAL(TRMENU_OPT_ENV, 'E', 0),
	0
};

//----------------------

typedef struct
{
	mPoint ptMainWin;
	int show_bkmark,
		mainwin_maximize;
	mStr strStyle;
}ConfigData;

//----------------------


//===========================
// 設定ファイル書き込み
//===========================


/** 設定ファイル書き込み */

static void _save_config()
{
	FILE *fp;
	mPoint pt;
	GlobalData *p = GDAT;
	uint32_t *pu32;
	int i;

	fp = mIniWriteOpenFile2(MAPP->pathConfig, CONFIGFILENAME);
	if(!fp) return;

	//-------- メイン

	mIniWriteGroup(fp, "main");

	mIniWriteInt(fp, "ver", 1);

	//ウィンドウ

	mWindowGetFrameRootPos(M_WINDOW(p->mainwin), &pt);
	
	mIniWritePoint(fp, "mainwin", &pt);
	mIniWriteInt(fp, "mainwin_maximize", mWindowIsMaximized(M_WINDOW(p->mainwin)));

	//しおりウィンドウ

	if(p->bkmarkwin)
		mWindowGetSaveBox(M_WINDOW(p->bkmarkwin), &p->bkmarkwin_box);

	mIniWriteBox(fp, "bkmarkwin", &p->bkmarkwin_box);
	mIniWriteInt(fp, "bkmarkwin_show", (p->bkmarkwin != 0));
	mIniWriteInt(fp, "bkmarkwin_tab", p->bkmarkwin_tabno);

	//

	mIniWriteStr(fp, "openpath", &p->strOpenPath);
	mIniWriteStr(fp, "bkmarkpath", &p->strBkmarkPath);
	mIniWriteStr(fp, "style", &p->style->strName);
	mIniWriteStr(fp, "guifont", &p->strGUIFont);
	mIniWriteInt(fp, "optflags", p->optflags);

	//------ ファイル履歴

	mIniWriteGroup(fp, "recentfile");
	mIniWriteNoStrs(fp, 0, p->strRecentFile, RECENTFILE_NUM);

	//------ マウス操作

	mIniWriteGroup(fp, "mouse");

	for(i = 0; i < MOUSECTRL_BTT_NUM; i++)
		mIniWriteNoInt(fp, i, p->mousectrl[i]);
	
	//------- ショートカットキー

	mIniWriteGroup(fp, "shortcutkey");

	for(pu32 = p->shortcutkey; *pu32; pu32++)
		mIniWriteNoHex(fp, *pu32 & 0xffff, *pu32 >> 16);

	//------ ツール

	mIniWriteGroup(fp, "tool");
	mIniWriteNoStrs(fp, 0, p->strTool, TOOLITEM_NUM);

	fclose(fp);
}


//===========================
// 設定ファイル読み込み
//===========================


/** ショートカットキー読み込み */

static void _load_config_shortcutkey(mIniRead *p)
{
	int n;
	uint32_t num,*pd;

	if(!mIniReadSetGroup(p, "shortcutkey"))
	{
		//グループがない場合はデフォルト値をコピー

		GDAT->shortcutkey = (uint32_t *)mMemdup(g_shortcutkey_def, sizeof(g_shortcutkey_def));
	}
	else
	{
		//個数+1 を確保
		
		n = mIniReadGetGroupItemNum(p);

		GDAT->shortcutkey = (uint32_t *)mMalloc(sizeof(uint32_t) * (n + 1), FALSE);
		if(!GDAT->shortcutkey) return;

		//読み込み "id=key"

		pd = GDAT->shortcutkey;

		while(mIniReadGetNextItem_nonum32(p, &n, &num, TRUE))
			*(pd++) = n | (num << 16);

		*pd = 0;
	}
}

/** マウス操作読み込み */

static void _load_config_mouse(mIniRead *p)
{
	uint8_t def[] = {
		MOUSECTRL_CMD_NEXTPAGE, MOUSECTRL_CMD_PREVPAGE,
		MOUSECTRL_CMD_PREVPAGE, MOUSECTRL_CMD_NEXTPAGE
	};
	int key;
	uint32_t num;

	if(!mIniReadSetGroup(p, "mouse"))
	{
		//グループがない場合はデフォルト値をコピー

		memcpy(GDAT->mousectrl, def, 4);
	}
	else
	{
		while(mIniReadGetNextItem_nonum32(p, &key, &num, FALSE))
		{
			if(key < MOUSECTRL_BTT_NUM)
				GDAT->mousectrl[key] = num;
		}
	}
}

/** 設定ファイル読み込み */

static void _load_config(ConfigData *dat)
{
	mIniRead *p;
	GlobalData *gd = GDAT;

	p = mIniReadLoadFile2(MAPP->pathConfig, CONFIGFILENAME);
	if(!p) return;

	mIniReadSetGroup(p, "main");

	//------- main

	//バージョン

	if(mIniReadInt(p, "ver", 0) != 1)
		mIniReadEmpty(p);

	//ウィンドウ

	mIniReadPoint(p, "mainwin", &dat->ptMainWin, -10000, -10000);
	dat->mainwin_maximize = mIniReadInt(p, "mainwin_maximize", 0);

	//しおりウィンドウ

	mIniReadBox(p, "bkmarkwin", &gd->bkmarkwin_box, -10000, -10000, -10000, -10000);
	dat->show_bkmark = mIniReadInt(p, "bkmarkwin_show", 0);
	gd->bkmarkwin_tabno = mIniReadInt(p, "bkmarkwin_tab", 0);

	//

	mIniReadStr(p, "openpath", &gd->strOpenPath, NULL);
	mIniReadStr(p, "bkmarkpath", &gd->strBkmarkPath, NULL);
	mIniReadStr(p, "style", &dat->strStyle, "default");
	mIniReadStr(p, "guifont", &gd->strGUIFont, NULL);

	gd->optflags = mIniReadInt(p, "optflags", 0);

	//-------- ファイル履歴

	mIniReadSetGroup(p, "recentfile");
	mIniReadNoStrs(p, 0, gd->strRecentFile, RECENTFILE_NUM);

	//------ マウス操作

	_load_config_mouse(p);

	//------- ショートカットキー

	_load_config_shortcutkey(p);

	//----- ツール

	if(mIniReadSetGroup(p, "tool"))
		mIniReadNoStrs(p, 0, GDAT->strTool, TOOLITEM_NUM);
	
	mIniReadEnd(p);
}


//===========================
// 初期化
//===========================


/** 初期化 */

static mBool _init(int argc,char **argv)
{
	MainWindow *mainwin;
	ConfigData dat;

	//設定ファイル用ディレクトリ作成

	mAppCreateConfigDir(NULL);
	mAppCreateConfigDir("style");

	//aoFont 初期化

	if(!aoFontInit()) return FALSE;

	//グローバルデータ確保

	if(!GlobalDataNew()) return FALSE;

	//設定ファイル・スタイル読み込み

	mMemzero(&dat, sizeof(ConfigData));

	_load_config(&dat);

	StyleLoadConf(GDAT->style, dat.strStyle.buf);

	mStrFree(&dat.strStyle);

	//スタイル初期化

	StyleInit(GDAT->style);

	//グローバルしおり読み込み

	BookmarkGlobal_loadFile();

	//GUI フォント作成

	if(!mStrIsEmpty(&GDAT->strGUIFont))
		mAppSetDefaultFont(GDAT->strGUIFont.buf);

	//メインウィンドウ作成

	mainwin = MainWindowNew();
	if(!mainwin) return FALSE;

	//ウィンドウ表示

	MainWindow_show(mainwin, &dat.ptMainWin, dat.mainwin_maximize);

	//しおりウィンドウ表示

	if(dat.show_bkmark)
		BookmarkWinNew();

	//ファイルを開く

	if(argc >= 2)
	{
		//引数のファイル

		mStr str = MSTR_INIT;

		mStrSetTextLocal(&str, argv[1], -1);
		
		mfLoadTextFile(str.buf, -1, 0, FALSE);

		mStrFree(&str);
	}
	else if((GDAT->optflags & OPTFLAGS_OPEN_LASTFILE)
				&& !mStrIsEmpty(GDAT->strRecentFile))
		//前回のファイル
		mfLoadTextFileFromRecent(0);

	return TRUE;
}


//===========================
// メイン
//===========================


/** 終了処理 */

static void _finish()
{
	//現在のファイルを閉じる

	GlobalCloseFile();

	//設定ファイル保存

	_save_config();

	BookmarkGlobal_saveFile();

	//解放

	GlobalDataFree(g_globaldat);

	aoFontEnd();
}

/** メイン */

int main(int argc,char **argv)
{
	if(mAppInit(&argc, argv)) return 1;

	//パス

	mAppSetConfigPath(".aobook", TRUE);

	//現在はGUI用のデータが必要ないので、セットしない
	//mAppSetResourcePath(PACKAGE_DATA_DIR);

	//翻訳データ

	mAppLoadTranslation(g_deftransdat, NULL, PACKAGE_DATA_DIR);

	//初期化

	if(!_init(argc, argv))
	{
		fputs("! failed initialize\n", stderr);
		return -1;
	}

	//実行

	mAppRun();

	//終了

	_finish();

	mAppEnd();

	return 0;
}
