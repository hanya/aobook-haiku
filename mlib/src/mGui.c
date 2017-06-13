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
 * [GUI 関連関数]
 *****************************************/

#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "mDef.h"

#include "mAppDef.h"
#include "mAppPrivate.h"
#include "mWindowDef.h"
#include "mSysCol.h"

#include "mGui.h"
#include "mUtil.h"
#ifndef OS_HAIKU
#include "mWidget.h"
#endif // OS_HAIKU
#include "mEvent.h"
#include "mPixbuf.h"
#include "mPixbuf_pv.h"
#include "mRectBox.h"
#ifndef OS_HAIKU
#include "mFont.h"
#endif // OS_HAIKU
#include "mStr.h"
#include "mUtilFile.h"

#include "mEventList.h"
#ifndef OS_HAIKU
#include "mWidget_pv.h"
#endif // OS_HAIKU

//--------------------------

mApp *g_mApp = NULL;
uint32_t g_mSysCol[MSYSCOL_NUM * 2];

//--------------------------
#ifndef OS_HAIKU
int __mAppInit(void);
void __mAppEnd(void);
void __mAppQuit(void);
int __mAppRun(mBool fwait);
#endif // OS_HAIKU
//--------------------------


//============================
// sub
//============================

#ifndef OS_HAIKU
/** 全ウィジェット削除 */

static void _app_DestroyAllWidget(void)
{
	mWidget *p;
	
	//全トップレベル削除
	/* ウィジェット削除時に次のウィジェットが削除されてしまうと
	 * 正しく処理できないので、削除直前の時点での次のウィジェットを使う。 */

	for(p = MAPP->widgetRoot->first; p; )
		p = mWidgetDestroy(p);
	
	//ルート

	mFree(MAPP->widgetRoot);
}
#endif
/** コマンドラインオプション
 *
 * ※ここで処理されたオプションは削除する。 */
#ifndef OS_HAIKU
static void _app_SetCmdlineOpt(int *argc,char **argv)
{
	int num,f,i,j;
	char *tmp[2];

	num = *argc;

	for(i = 1; i < num; i++)
	{
		//判定
		
		f = 0;
		
		if(strcmp(argv[i], "--debug-event") == 0)
		{
			MAPP->flags |= MAPP_FLAGS_DEBUG_EVENT;
			f = 1;
		}
		else if(strcmp(argv[i], "--disable-grab") == 0)
		{
			MAPP->flags |= MAPP_FLAGS_DISABLE_GRAB;
			f = 1;
		}
		else if(strcmp(argv[i], "--trfile") == 0)
		{
			if(i == num - 1)
				f = 1;
			else
			{
				MAPP->pv->trans_filename = argv[i + 1];
				f = 2;
			}
		}

		//argv を終端に移動

		if(f)
		{
			num -= f;

			for(j = 0; j < f; j++)
				tmp[j] = argv[i + j];

			for(j = i; j < num; j++)
				argv[j] = argv[j + 1];

			for(j = 0; j < f; j++)
				argv[num + j] = tmp[j];
		}
	}
	
	*argc = num;
}
#endif
#ifndef OS_HAIKU
/** デフォルトフォント作成 */

static int _app_CreateDefaultFont(void)
{
	mFontInfo info;

	info.mask   = MFONTINFO_MASK_SIZE | MFONTINFO_MASK_WEIGHT | MFONTINFO_MASK_SLANT;
	info.size   = 10;
	info.weight = MFONTINFO_WEIGHT_NORMAL;
	info.slant  = MFONTINFO_SLANT_ROMAN;

	MAPP->fontDefault = mFontCreate(&info);

	return (MAPP->fontDefault == NULL);
}
#endif
/** システムカラーセット */

static void _app_SetSysCol(void)
{
	int i;
	mRgbCol rgb[] = {
		0xffffff, //WHITE
		
		0xeaeaea, //FACE
		0xd3d6ee, //FACE_FOCUS
		0x5e84db, //FACE_SELECT
		0x8aa9ef, //FACE_SELECTLIGHT
		0xd8d8d8, //FACE_DARK
		0x888888, //FACE_DARKER
		0xffffff, //FACE_LIGHTEST
		
		0xa8a8a8, //FRAME
		0x3333ff, //FRAME_FOCUS
		0x666666, //FRAME_DARK
		0xcccccc, //FRAME_LIGHT

		0,        //TEXT
		0xffffff, //TEXT_REVERSE
		0x999999, //TEXT_DISABLE
		0xffffff, //TEXT_SELECT

		0x606060, //MENU_FACE
		0,        //MENU_FRAME
		0xa0a0a0, //MENU_SEP
		0x96a5ff, //MENU_SELECT
		0xffffff, //MENU_TEXT
		0x888888, //MENU_TEXT_DISABLE
		0xb0b0b0  //MENU_TEXT_SHORTCUT
	};

	for(i = 0; i < MSYSCOL_NUM; i++)
		g_mSysCol[i] = mRGBtoPix(rgb[i]);

	//RGB 値

	for(i = 0; i < MSYSCOL_NUM; i++)
		g_mSysCol[MSYSCOL_NUM + i] = rgb[i];
}
#ifndef OS_HAIKU
/** ループデータ初期化 */

static void _rundat_init(mAppRunDat *p)
{
	memset(p, 0, sizeof(mAppRunDat));
	
	p->runBack = MAPP_PV->runCurrent;
	
	MAPP_PV->runCurrent = p;
	MAPP_PV->runLevel++;
}
#endif

//=============================
// メイン関数
//=============================


/********************//**

@defgroup main mGui
@brief GUI 関連関数

@ingroup group_main
@{

@file mGui.h

*************************/

/** 終了処理
 *
 * すべてのウィジェットは自動で削除される。 */

void mAppEnd(void)
{
	mApp *p = MAPP;

	if(!p) return;
#ifndef OS_HAIKU
	//全ウィジェット削除
	
	if(p->widgetRoot)
		_app_DestroyAllWidget();
	
	//フォント削除
	
	mFontFree(p->fontDefault);
	
	//各システム終了処理
	
	if(p->sys)
		__mAppEnd();
	
	//mAppPrivate 関連

	if(p->pv)
	{
		//イベント削除
		
		mEventListEmpty();

		//翻訳データ削除

		mTranslationFree(&p->pv->transDef);
		mTranslationFree(&p->pv->transApp);
	}
#endif
	//パス文字列

	mFree(p->pathConfig);
	mFree(p->pathData);
	
#ifndef OS_HAIKU
	//メモリ解放
	
	mFree(p->pv);
	mFree(p->sys);
#endif
	mFree(p);
	
	MAPP = NULL;
}

/** アプリケーション初期化
 *
 * argc, argv は、mlib 側でオプション処理されたものは除去される。
 * 
 * @retval 0  成功
 * @retval -1 エラー */

int mAppInit(int *argc,char **argv)
{
	if(MAPP) return 0;

	//ロケール
	
	setlocale(LC_ALL, "");
	
	//mApp 確保
	
	MAPP = (mApp *)mMalloc(sizeof(mApp), TRUE);
	if(!MAPP) return -1;
	
#ifndef OS_HAIKU
	//mAppPrivate 確保
	
	MAPP_PV = (mAppPrivate *)mMalloc(sizeof(mAppPrivate), TRUE);
	if(!MAPP_PV) goto ERR;

	//ルートウィジェット作成
	
	MAPP->widgetRoot = mWidgetNew(0, (mWidget *)1);
	if(!MAPP->widgetRoot) goto ERR;

	//コマンドラインオプション
	
	_app_SetCmdlineOpt(argc, argv);
#endif
	//システム別の初期化
	
	if(__mAppInit()) goto ERR;
	
	//デフォルトフォント作成
#ifndef OS_HAIKU
	if(_app_CreateDefaultFont()) goto ERR;
#endif
	//RGB シフト数

	MAPP->r_shift_left = mGetBitOnPos(MAPP->maskR);
	MAPP->g_shift_left = mGetBitOnPos(MAPP->maskG);
	MAPP->b_shift_left = mGetBitOnPos(MAPP->maskB);

	MAPP->r_shift_right = 8 - mGetBitOffPos(MAPP->maskR >> MAPP->r_shift_left);
	MAPP->g_shift_right = 8 - mGetBitOffPos(MAPP->maskG >> MAPP->g_shift_left);
	MAPP->b_shift_right = 8 - mGetBitOffPos(MAPP->maskB >> MAPP->b_shift_left);
#ifndef OS_HAIKU
	//システムカラーセット

	_app_SetSysCol();
#endif
	return 0;

ERR:
	mAppEnd();
	return -1;
}


/** ユーザーアクション (キー、マウス) のイベントをブロックするか */

void mAppBlockUserAction(mBool on)
{
	if(on)
		MAPP->flags |= MAPP_FLAGS_BLOCK_USER_ACTION;
	else
		MAPP->flags &= ~MAPP_FLAGS_BLOCK_USER_ACTION;
}
#ifndef OS_HAIKU
/** メインループを抜ける */

void mAppQuit(void)
{
	if(MAPP_PV->runCurrent)
	{
		//すでにこのループの quit が実行されている場合は処理しない

		if(!(MAPP_PV->runCurrent->bQuit))
		{
			MAPP_PV->runCurrent->bQuit = TRUE;

			__mAppQuit();
		}
	}
}
#endif // OS_HAIKU
#ifndef OS_HAIKU
/** メインループ */

void mAppRun(void)
{
	mAppRunDat dat;

	_rundat_init(&dat);
	
	__mAppRun(TRUE);
	
	MAPP_PV->runCurrent = dat.runBack;
	MAPP_PV->runLevel--;
}
#endif // OS_HAIKU
#ifndef OS_HAIKU
/** メインループ (モーダルウィンドウ)
 *
 * 指定ウィンドウ以外はアクティブにならない。 */

void mAppRunModal(mWindow *modal)
{
	mAppRunDat dat;
	
	_rundat_init(&dat);

	dat.modal = modal;
	
	__mAppRun(TRUE);
	
	MAPP_PV->runCurrent = dat.runBack;
	MAPP_PV->runLevel--;
}

/** メインループ (ポップアップウィンドウ)
 *
 * 指定ウィンドウ外がクリックされるなどした場合はループを抜ける。 */

void mAppRunPopup(mWindow *popup)
{
	mAppRunDat dat;
	
	_rundat_init(&dat);

	dat.popup = popup;
	
	__mAppRun(TRUE);
	
	MAPP_PV->runCurrent = dat.runBack;
	MAPP_PV->runLevel--;
}

/** 現在実行中のモーダルのウィンドウ取得
 *
 * @return NULL でモーダルではない */

mWindow *mAppGetCurrentModalWindow()
{
	if(MAPP_PV->runCurrent)
		return MAPP_PV->runCurrent->modal;
	else
		return NULL;
}
#endif // OS_HAIKU

//=========================
// パス
//=========================


/**************//**

@name パス関連
@{

*******************/


/** "!/" など特殊なパス名の場合、実際のパスを取得
 *
 * @param path "!/" で始まっている場合はデータファイルディレクトリからの相対パス */

char *mAppGetFilePath(const char *path)
{
	if(path[0] == '!' && path[1] == '/')
	{
		mStr str = MSTR_INIT;

		mAppGetDataPath(&str, path + 2);
		return str.buf;
	}
	else
		return mStrdup(path);
}

/** データファイルのある場所をセット */

void mAppSetDataPath(const char *path)
{
	mFree(MAPP->pathData);

	MAPP->pathData = mStrdup(path);
}

/** データファイルのパスを取得 */

void mAppGetDataPath(mStr *str,const char *pathadd)
{
	mStrSetText(str, MAPP->pathData);
	mStrPathAdd(str, pathadd);
}

/** 設定ファイルのディレクトリのパスセット
 *
 * @param bHome TRUE でホームディレクトリからの相対パスを指定 */

void mAppSetConfigPath(const char *path,mBool bHome)
{
	mFree(MAPP->pathConfig);

	if(!bHome)
		MAPP->pathConfig = mStrdup(path);
	else
	{
		mStr str = MSTR_INIT;

		mStrPathSetHomeDir(&str);
#ifdef OS_HAIKU
		mStrPathAdd(&str, "config");
		mStrPathAdd(&str, "settings");
#endif
		mStrPathAdd(&str, path);

		MAPP->pathConfig = mStrdup(str.buf);

		mStrFree(&str);
	}
}

/** 設定ファイルのディレクトリのパスを取得
 *
 * @param pathadd パスに追加する文字列 (NULL でなし) */

void mAppGetConfigPath(mStr *str,const char *pathadd)
{
	mStrSetText(str, MAPP->pathConfig);
	mStrPathAdd(str, pathadd);
}

/** 設定ファイルのディレクトリを作成
 *
 * @param pathadd パスに追加する文字列 (NULL でなし)
 * @return 0:作成された 1:失敗 -1:すでにディレクトリが存在している */

int mAppCreateConfigDir(const char *pathadd)
{
	mStr str = MSTR_INIT;
	int ret;

	if(!(MAPP->pathConfig)) return 1;

	mAppGetConfigPath(&str, pathadd);

	if(mIsFileExist(str.buf, TRUE))
		ret = -1;
	else
	{
		ret = mCreateDir(str.buf);
		ret = (ret)? 0: 1;
	}

	mStrFree(&str);

	return ret;
}

/** データディレクトリから設定ファイルディレクトリにファイルをコピー
 *
 * 設定ファイルディレクトリにファイルが存在していない場合。 */

void mAppCopyFile_dataToConfig(const char *path)
{
	mStr str_data = MSTR_INIT, str_conf = MSTR_INIT;

	mAppGetConfigPath(&str_conf, path);

	if(!mIsFileExist(str_conf.buf, FALSE))
	{
		mAppGetDataPath(&str_data, path);

		mCopyFile(str_data.buf, str_conf.buf, 0);

		mStrFree(&str_data);
	}
	
	mStrFree(&str_conf);
}

/* @} */


//=========================
// ほか
//=========================

/**

@name ほか
@{

*/
#ifndef OS_HAIKU
/** デフォルトフォントを作成してセット */

mBool mAppSetDefaultFont(const char *format)
{
	mFont *font;

	font = mFontCreateFromFormat(format);
	if(!font) return FALSE;

	//入れ替え

	mFontFree(MAPP->fontDefault);

	MAPP->fontDefault = font;

	return TRUE;
}

/** 翻訳データ読み込み
 *
 * コマンドラインで --trfile が指定されている場合は、そのファイルが読み込まれる。
 *
 * @param defdat 埋め込みデータ
 * @param lang 言語名(ja_JP など)。NULL でシステムの言語。
 * @param path ファイルの検索パス */

void mAppLoadTranslation(const void *defdat,const char *lang,const char *path)
{
	//埋め込みデータ

	mTranslationSetEmbed(&MAPP_PV->transDef, defdat);

	//翻訳ファイル読み込み

	if(MAPP->pv->trans_filename)
		mTranslationLoadFile(&MAPP_PV->transApp, MAPP->pv->trans_filename, TRUE);
	else
		mTranslationLoadFile_dir(&MAPP_PV->transApp, lang, path);
}

/** レイアウトサイズ計算 */

void mGuiCalcHintSize(void)
{
	mWidget *p = NULL;

	while(1)
	{
		p = __mWidgetGetTreeNext_follow_ui(p,
				MWIDGET_UI_FOLLOW_CALC, MWIDGET_UI_CALC);
		
		if(!p) break;
		
		__mWidgetCalcHint(p);
	}
}
#endif // OS_HAIKU
/** @fn int mKeyCodeToName(uint32_t c,char *buf,int bufsize)
 *
 * MKEY_* のキーコードからキー名の文字列を取得
 *
 * @return 格納された文字数 */

/* @} */

/* @} */

#ifndef OS_HAIKU
//=================================
// <mTrans.h> 翻訳
//=================================


/********************//**

@defgroup apptrans mTrans
@brief 翻訳データ

@ingroup group_main
@{

@file mTrans.h

@def M_TR_G(id)
mTransSetGroup() の短縮形

@def M_TR_T(id)
mTransGetText() の短縮形

@def M_TR_T2(gid,id)
mTransGetText2() の短縮形

@def M_TRGROUP_SYS
システム関連の文字列グループ ID

@enum M_TRSYS
システム関連の文字列 ID

************************/


/** 翻訳のカレントグループセット */

void mTransSetGroup(uint16_t groupid)
{
	mTranslationSetGroup(&MAPP_PV->transDef, groupid);
	mTranslationSetGroup(&MAPP_PV->transApp, groupid);
}

/** 翻訳のカレントグループから文字列取得
 *
 * @return 見つからなかった場合、空文字列 */

const char *mTransGetText(uint16_t strid)
{
	const char *pc;

	pc = mTranslationGetText(&MAPP_PV->transApp, strid);
	if(!pc)
	{
		pc = mTranslationGetText(&MAPP_PV->transDef, strid);
		if(!pc) pc = "";
	}

	return pc;
}

/** 翻訳のカレントグループから文字列取得
 *
 * @return 見つからなかった場合、NULL */

const char *mTransGetTextRaw(uint16_t strid)
{
	const char *pc;

	pc = mTranslationGetText(&MAPP_PV->transApp, strid);

	if(!pc)
		pc = mTranslationGetText(&MAPP_PV->transDef, strid);

	return pc;
}

/** 翻訳のカレントグループから埋め込みのデフォルト文字列取得
 *
 * @return 見つからなかった場合、空文字列 */

const char *mTransGetTextDef(uint16_t strid)
{
	const char *pc;

	pc = mTranslationGetText(&MAPP_PV->transDef, strid);

	return (pc)? pc: "";
}

/** 翻訳から指定グループの文字列取得
 *
 * カレントグループは変更されない。 */

const char *mTransGetText2(uint16_t groupid,uint16_t strid)
{
	const char *pc;

	pc = mTranslationGetText2(&MAPP_PV->transApp, groupid, strid);
	if(!pc)
	{
		pc = mTranslationGetText2(&MAPP_PV->transDef, groupid, strid);
		if(!pc) pc = "";
	}

	return pc;
}

/** 翻訳から指定グループの文字列取得
 *
 * @return 見つからなかった場合、NULL */

const char *mTransGetText2Raw(uint16_t groupid,uint16_t strid)
{
	const char *pc;

	pc = mTranslationGetText2(&MAPP_PV->transApp, groupid, strid);

	if(!pc)
		pc = mTranslationGetText2(&MAPP_PV->transDef, groupid, strid);

	return pc;
}

/* @} */
#endif // OS_HAIKU
#ifndef OS_HAIKU
//=================================
// sub - イベント処理後
//=================================


/**********//**

@addtogroup main
@{

**************/


/** ウィジェットの描画処理を行う */

void mGuiDraw(void)
{
	mWidget *p = NULL;
	mPixbuf *pixbuf;
	mRect clip;
	
	while(1)
	{
		//次のウィジェット
	
		p = __mWidgetGetTreeNext_follow_uidraw(p);
		if(!p) break;
		
		//非表示

		if(!mWidgetIsVisibleReal(p)) continue;

		//-----------

		//クリッピング範囲

		if(!__mWidgetGetClipRect(p, &clip)) continue;

		//
		
		pixbuf = (p->toplevel)->win.pixbuf;
		
		mPixbufSetOffset(pixbuf, p->absX, p->absY, NULL);
		__mPixbufSetClipMaster(pixbuf, &clip);
	
		//描画ハンドラ
	
		if(p->draw)
			(p->draw)(p, pixbuf);

		//

		mPixbufSetOffset(pixbuf, 0, 0, NULL);
		__mPixbufSetClipMaster(pixbuf, NULL);
	}
}

/** 更新処理
 * 
 * 更新範囲のイメージをウインドウに転送する */

mBool mGuiUpdate(void)
{
	mWindow *p;
	mBox box;
	mBool ret = FALSE;
	
	for(p = M_WINDOW(MAPP->widgetRoot->first); p; p = M_WINDOW(p->wg.next))
	{
		if(p->wg.fUI & MWIDGET_UI_UPDATE)
		{
			if((p->wg.fState & MWIDGET_STATE_VISIBLE)
				&& mRectClipBox_d(&p->win.rcUpdate, 0, 0, p->wg.w, p->wg.h))
			{
				mRectToBox(&box, &p->win.rcUpdate);
				
				mPixbufRenderWindow(p->win.pixbuf, p, &box);

				ret = TRUE;
			}

			p->wg.fUI &= ~MWIDGET_UI_UPDATE;
		}
	}

	return ret;
}

/* @} */


/** イベント処理後
 *
 * @return ウィンドウ内容が更新された */

mBool __mAppAfterEvent(void)
{
	mGuiDraw();

	return mGuiUpdate();
}
#endif // OS_HAIKU

//=============================
// <mDef.h> 色変換
//=============================

/**********//**

@addtogroup default
@{

**************/


/** RGB 値をピクセル値に変換
 *
 * @param c (uint32_t)-1 の場合はそのまま返す */

mPixCol mRGBtoPix(mRgbCol c)
{
	if(c == (uint32_t)-1)
		return c;
	else if(MAPP->depth >= 24)
	{
		return (M_GET_R(c) << MAPP->r_shift_left) |
			(M_GET_G(c) << MAPP->g_shift_left) |
			(M_GET_B(c) << MAPP->b_shift_left);
	}
	else
	{
		return ((M_GET_R(c) >> MAPP->r_shift_right) << MAPP->r_shift_left) |
			((M_GET_G(c) >> MAPP->g_shift_right) << MAPP->g_shift_left) |
			((M_GET_B(c) >> MAPP->b_shift_right) << MAPP->b_shift_left);
	}
}

/** RGB 値をピクセル値に変換 */

mPixCol mRGBtoPix2(uint8_t r,uint8_t g,uint8_t b)
{
	if(MAPP->depth >= 24)
	{
		return ((uint32_t)r << MAPP->r_shift_left) |
			(g << MAPP->g_shift_left) |
			(b << MAPP->b_shift_left);
	}
	else
	{
		return ((r >> MAPP->r_shift_right) << MAPP->r_shift_left) |
			((g >> MAPP->g_shift_right) << MAPP->g_shift_left) |
			((b >> MAPP->b_shift_right) << MAPP->b_shift_left);
	}
}

/** グレイスケールからピクセル値を取得 */

mPixCol mGraytoPix(uint8_t c)
{
	if(MAPP->depth >= 24)
	{
		return ((uint32_t)c << MAPP->r_shift_left) |
			(c << MAPP->g_shift_left) |
			(c << MAPP->b_shift_left);
	}
	else
	{
		return ((c >> MAPP->r_shift_right) << MAPP->r_shift_left) |
			((c >> MAPP->g_shift_right) << MAPP->g_shift_left) |
			((c >> MAPP->b_shift_right) << MAPP->b_shift_left);
	}
}

/** ピクセル値を RGB 値に変換 */

mRgbCol mPixtoRGB(mPixCol c)
{
	uint32_t r,g,b;
	
	r = (c & MAPP->maskR) >> MAPP->r_shift_left;
	g = (c & MAPP->maskG) >> MAPP->g_shift_left;
	b = (c & MAPP->maskB) >> MAPP->b_shift_left;

	if(MAPP->depth <= 16)
	{
		r = (r << MAPP->r_shift_right) + (1 << MAPP->r_shift_right) - 1;
		g = (g << MAPP->g_shift_right) + (1 << MAPP->g_shift_right) - 1;
		b = (b << MAPP->b_shift_right) + (1 << MAPP->b_shift_right) - 1;
	}
	
	return (r << 16) | (g << 8) | b;
}

/* @} */
