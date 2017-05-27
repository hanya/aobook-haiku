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

/********************************
 * グローバル定義
 ********************************/

#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include "mStrDef.h"
#include "mListDef.h"

/*---- macro ----*/

#define GDAT           g_globaldat
#define APPNAME        "aobook"
#define RECENTFILE_NUM 10
#define TOOLITEM_NUM   10
#define STYLE_MAXNUM   50

#define GLOBAL_ISNOT_HAVE_TEXT  (!GDAT->textbuf.buf)

/*---- enum ----*/

enum
{
	MOUSECTRL_BTT_LEFT,
	MOUSECTRL_BTT_RIGHT,
	MOUSECTRL_BTT_SCROLL_UP,
	MOUSECTRL_BTT_SCROLL_DOWN,

	MOUSECTRL_BTT_NUM
};

enum
{
	MOUSECTRL_CMD_NONE,
	MOUSECTRL_CMD_NEXTPAGE,
	MOUSECTRL_CMD_PREVPAGE,

	MOUSECTRL_CMD_NUM
};

enum
{
	MODIFY_BKMARK_GLOBAL = 1
};

enum
{
	OPTFLAGS_OPEN_LASTFILE = 1<<0,			//起動時、前回のファイル読み込む
	OPTFLAGS_LOCALBKM_NO_COMMENT = 1<<1		//ローカルしおりの追加時、コメントを入力しない
};

/*---- struct ----*/

typedef struct _MainWindow MainWindow;
typedef struct _BookmarkWin BookmarkWin;
typedef struct _StyleData StyleData;
typedef struct _AO_LAYOUT_INFO AO_LAYOUT_INFO;
typedef struct _AO_PAGEINFO_ITEM AO_PAGEINDEX;


typedef struct
{
	MainWindow *mainwin;
	BookmarkWin *bkmarkwin;

	mBox bkmarkwin_box;
	int bkmarkwin_tabno;

	mList listBkmarkGlobal,
		listBkmarkLocal;

	mStr strFileName,
		strOpenPath,
		strBkmarkPath,
		strLayoutFilePath,
		strGUIFont,
		strRecentFile[RECENTFILE_NUM],
		strTool[TOOLITEM_NUM];

	uint32_t optflags,
		*shortcutkey;
	uint8_t fModify,
		mousectrl[MOUSECTRL_BTT_NUM];

	//

	mBuf textbuf;	//テキストデータ
	int charcode;	//文字エンコード

	StyleData *style;
	AO_LAYOUT_INFO *layout;
	AO_PAGEINDEX *curpage;

	mBool bNowThread;  //スレッド中かどうか
}GlobalData;


extern GlobalData *g_globaldat;

/*---- func ----*/

void GlobalDataFree(GlobalData *p);
mBool GlobalDataNew();

void GlobalCloseFile();
void GlobalEmptyTextData();
void GlobalSetTextData(mBuf *buf);

void GlobalAddRecentFile(const char *filename,int code,int line);
void GlobalGetRecentFileInfo(int no,mStr *strfname,int *code,int *line);

void GlobalSetStyleToLayout();
void GlobalSetLayoutFilePath();

#endif
