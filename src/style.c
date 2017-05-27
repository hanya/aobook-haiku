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
 * スタイルデータ
 *****************************************/

#include <string.h>
#include <stdio.h>

#include "mDef.h"
#include "mGui.h"
#include "mIniRead.h"
#include "mIniWrite.h"
#include "mUtilCharCode.h"
#include "mImageBuf.h"
#include "mLoadImage.h"

#include "style.h"
#include "globaldata.h"

#include "aoFont.h"


//-------------------

static const char *g_str_nohead =
		"）〕］｝〉》」』】〙〗、。，．・：；？！?!‼⁇⁈⁉゜ヽヾゝゞ々〻ー’”゛〟"
		"ぁぃぅぇぉっゃゅょゎァィゥェォッャュョヮヵヶ",
	*g_str_nobottom = "（［｛〔〈《「『【〘〖〝‘“",
	*g_str_hanging = "、。，．",
	*g_str_nosep = "…‥";

//-------------------


//=======================
// sub
//=======================


/** 設定ファイルから UCS-4 文字列取得 */

uint32_t *_read_ucs4_str(mIniRead *ini,const char *key,const char *def)
{
	const char *pc;

	pc = mIniReadText(ini, key, def);

	if(!pc || !(*pc))
		//空なら NULL
		return NULL;
	else
		return mUTF8ToUCS4_alloc(pc, -1, NULL);
}

/** 設定ファイルに UCS-4 文字列保存 */

void _write_ucs4_str(FILE *fp,const char *key,uint32_t *text)
{
	char *utf8;

	utf8 = mUCS4ToUTF8_alloc(text, -1, NULL);

	mIniWriteText(fp, key, utf8);

	mFree(utf8);
}

/** 画像読み込み */

static mImageBuf *_load_image(mStr *strfname)
{
	mLoadImageSource src;
	mLoadImageFunc func;

	if(!mStrIsEmpty(strfname))
	{
		src.type = MLOADIMAGE_SOURCE_TYPE_PATH;
		src.filename = strfname->buf;

		if(mLoadImage_checkFormat(&src, &func,
			MLOADIMAGE_CHECKFORMAT_F_BMP | MLOADIMAGE_CHECKFORMAT_F_PNG | MLOADIMAGE_CHECKFORMAT_F_JPEG))
		{
			return mImageBufLoadImage(&src, (mDefEmptyFunc)func, 3, NULL);
		}
	}

	return NULL;
}


//=======================
// 解放
//=======================


/** フォント解放 */

void StyleFreeFont(StyleData *p)
{
	aoFontFree(p->fontText); p->fontText = NULL;
	aoFontFree(p->fontBold); p->fontBold = NULL;
	aoFontFree(p->fontRuby); p->fontRuby = NULL;
	aoFontFree(p->fontHalf); p->fontHalf = NULL;
	aoFontFree(p->fontInfo); p->fontInfo = NULL;
}

/** AO_STYLE データを解放 */

void StyleFreeAO(StyleData *p)
{
	M_FREE_NULL(p->b.strNoHead);
	M_FREE_NULL(p->b.strNoBottom);
	M_FREE_NULL(p->b.strHanging);
	M_FREE_NULL(p->b.strNoSep);
	M_FREE_NULL(p->b.strReplace);
}

/** データのみ解放 */

void StyleFreeData(StyleData *p)
{
	if(p)
	{
		StyleFreeAO(p);

		mStrFree(&p->strName);
		mStrFree(&p->strFontText);
		mStrFree(&p->strFontBold);
		mStrFree(&p->strFontRuby);
		mStrFree(&p->strFontInfo);
		mStrFree(&p->strBkgndFile);
	}
}

/** 解放 */

void StyleFree(StyleData *p)
{
	if(p)
	{
		StyleFreeData(p);
		StyleFreeFont(p);

		mImageBufFree(p->imgBkgnd);
		p->imgBkgnd = NULL;
	}
}

/** バッファ解放 (p も含めて解放) */

void StyleFreeBuf(StyleData *p)
{
	if(p)
	{
		StyleFree(p);

		mFree(p);
	}
}


//=======================


/** スタイルデータ確保 */

StyleData *StyleAlloc()
{
	return (StyleData *)mMalloc(sizeof(StyleData), TRUE);
}

/** 設定ファイルのパスを取得 */

void StyleGetFilePath(mStr *str,const char *name)
{
	mAppGetConfigPath(str, "style");
	mStrPathAdd(str, name);
	mStrAppendText(str, ".conf");
}

/** 設定ファイルからスタイルデータ読み込み */

void StyleLoadConf(StyleData *p,const char *name)
{
	mStr str = MSTR_INIT;
	mIniRead *ini;
	AO_STYLE *ao;

	//読み込み

	StyleGetFilePath(&str, name);

	ini = mIniReadLoadFile(str.buf);

	mStrFree(&str);

	if(!ini) return;

	//

	mIniReadSetGroup(ini, "style");

	if(mIniReadInt(ini, "ver", 0) != 1)
		mIniReadEmpty(ini);

	//AO_STYLE

	ao = &p->b;

	ao->pages = mIniReadInt(ini, "pages", 1);
	ao->chars = mIniReadInt(ini, "chars", 32);
	ao->lines = mIniReadInt(ini, "lines", 15);
	ao->charSpace = mIniReadInt(ini, "charspace", 0);
	ao->lineSpace = mIniReadInt(ini, "linespace", 2);
	ao->flags = mIniReadInt(ini, "flags", AOSTYLE_F_HANGING | AOSTYLE_F_ENABLE_PICTURE | AOSTYLE_F_DRAW_PAGENO | AOSTYLE_F_DASH_TO_LINE);
	ao->pageSpace = mIniReadInt(ini, "pagespace", 20);
	ao->margin.x1 = mIniReadInt(ini, "mgleft", 20);
	ao->margin.x2 = mIniReadInt(ini, "mgright", 20);
	ao->margin.y1 = mIniReadInt(ini, "mgtop", 30);
	ao->margin.y2 = mIniReadInt(ini, "mgbottom", 20);

	ao->colText = mIniReadHex(ini, "coltext", 0);
	ao->colRuby = mIniReadHex(ini, "colruby", 0x400000);
	ao->colInfo = mIniReadHex(ini, "colinfo", M_RGB(0,114,0));

	ao->strNoHead = _read_ucs4_str(ini, "nohead", g_str_nohead);
	ao->strNoBottom = _read_ucs4_str(ini, "nobottom", g_str_nobottom);
	ao->strHanging = _read_ucs4_str(ini, "hanging", g_str_hanging);
	ao->strNoSep = _read_ucs4_str(ini, "nosep", g_str_nosep);
	ao->strReplace = _read_ucs4_str(ini, "replace", NULL);

	//

	p->colBkgnd = mIniReadHex(ini, "colbkgnd", 0xfff8e0);

	mIniReadStr(ini, "bkgndfile", &p->strBkgndFile, NULL);

	p->bkgnd_imgtype = mIniReadInt(ini, "bkgndtype", 0);

	mIniReadStr(ini, "fonttext", &p->strFontText, "family=serif;size=12");
	mIniReadStr(ini, "fontbold", &p->strFontBold, "family=serif;size=12;weight=bold");
	mIniReadStr(ini, "fontruby", &p->strFontRuby, "family=serif;size=6");
	mIniReadStr(ini, "fontinfo", &p->strFontInfo, "family=serif;size=8");

	//

	mIniReadEnd(ini);

	//名前コピー

	mStrSetText(&p->strName, name);
}

/** 設定ファイルに保存 */

void StyleSaveConf(StyleData *p)
{
	mStr str = MSTR_INIT;
	FILE *fp;
	AO_STYLE *ao = &p->b;

	StyleGetFilePath(&str, p->strName.buf);

	fp = mIniWriteOpenFile(str.buf);

	mStrFree(&str);
	
	if(!fp) return;

	//

	mIniWriteGroup(fp, "style");

	mIniWriteInt(fp, "ver", 1);

	//AO_STYLE

	mIniWriteInt(fp, "pages", ao->pages);
	mIniWriteInt(fp, "chars", ao->chars);
	mIniWriteInt(fp, "lines", ao->lines);
	mIniWriteInt(fp, "charspace", ao->charSpace);
	mIniWriteInt(fp, "linespace", ao->lineSpace);
	mIniWriteInt(fp, "flags", ao->flags);
	mIniWriteInt(fp, "pagespace", ao->pageSpace);
	mIniWriteInt(fp, "mgleft", ao->margin.x1);
	mIniWriteInt(fp, "mgright", ao->margin.x2);
	mIniWriteInt(fp, "mgtop", ao->margin.y1);
	mIniWriteInt(fp, "mgbottom", ao->margin.y2);

	mIniWriteHex(fp, "coltext", ao->colText);
	mIniWriteHex(fp, "colruby", ao->colRuby);
	mIniWriteHex(fp, "colinfo", ao->colInfo);

	_write_ucs4_str(fp, "nohead", ao->strNoHead);
	_write_ucs4_str(fp, "nobottom", ao->strNoBottom);
	_write_ucs4_str(fp, "hanging", ao->strHanging);
	_write_ucs4_str(fp, "nosep", ao->strNoSep);
	_write_ucs4_str(fp, "replace", ao->strReplace);

	//

	mIniWriteHex(fp, "colbkgnd", p->colBkgnd);
	mIniWriteStr(fp, "bkgndfile", &p->strBkgndFile);
	mIniWriteInt(fp, "bkgndtype", p->bkgnd_imgtype);

	mIniWriteStr(fp, "fonttext", &p->strFontText);
	mIniWriteStr(fp, "fontbold", &p->strFontBold);
	mIniWriteStr(fp, "fontruby", &p->strFontRuby);
	mIniWriteStr(fp, "fontinfo", &p->strFontInfo);

	fclose(fp);
}

/** 起動時の初期化 */

void StyleInit(StyleData *p)
{
	//フォント

	StyleCreateFont(p);

	GlobalSetStyleToLayout();

	//背景画像

	p->imgBkgnd = _load_image(&p->strBkgndFile);
}

/** デフォルト設定をセット */

void StyleSetDefault(StyleData *p)
{
	AO_STYLE *ao;

	//------ AO_STYLE

	ao = &p->b;

	ao->pages = 1;
	ao->chars = 32;
	ao->lines = 15;
	ao->charSpace = 0;
	ao->lineSpace = 2;
	ao->flags = AOSTYLE_F_HANGING | AOSTYLE_F_ENABLE_PICTURE | AOSTYLE_F_DRAW_PAGENO | AOSTYLE_F_DASH_TO_LINE;
	ao->pageSpace = 20;
	ao->margin.x1 = ao->margin.x2 = ao->margin.y2 = 20;
	ao->margin.y1 = 30;

	ao->colText = 0;
	ao->colRuby = 0x330000;
	ao->colInfo = M_RGB(0,114,0);

	StyleSetUTF32Text(&ao->strNoHead, g_str_nohead);
	StyleSetUTF32Text(&ao->strNoBottom, g_str_nobottom);
	StyleSetUTF32Text(&ao->strHanging, g_str_hanging);
	StyleSetUTF32Text(&ao->strNoSep, g_str_nosep);

	//--------

	p->colBkgnd = 0xfff8e0;

	mStrSetText(&p->strFontText, "family=serif;size=12");
	mStrSetText(&p->strFontBold, "family=serif;size=12;weight=bold");
	mStrSetText(&p->strFontRuby, "family=serif;size=6");
	mStrSetText(&p->strFontInfo, "family=serif;size=8");
}

/** データのみを複製 */

void StyleCopyData(StyleData *dst,StyleData *src)
{
	//AO_STYLE

	StyleFreeAO(dst);

	memcpy(&dst->b, &src->b, sizeof(AO_STYLE));

	dst->b.strNoHead = mUCS4StrDup(src->b.strNoHead);
	dst->b.strNoBottom = mUCS4StrDup(src->b.strNoBottom);
	dst->b.strHanging = mUCS4StrDup(src->b.strHanging);
	dst->b.strNoSep = mUCS4StrDup(src->b.strNoSep);
	dst->b.strReplace = mUCS4StrDup(src->b.strReplace);

	//mStr

	mStrCopy(&dst->strName, &src->strName);
	mStrCopy(&dst->strFontText, &src->strFontText);
	mStrCopy(&dst->strFontBold, &src->strFontBold);
	mStrCopy(&dst->strFontRuby, &src->strFontRuby);
	mStrCopy(&dst->strFontInfo, &src->strFontInfo);
	mStrCopy(&dst->strBkgndFile, &src->strBkgndFile);

	//

	dst->colBkgnd = src->colBkgnd;
	dst->bkgnd_imgtype = src->bkgnd_imgtype;
}

/** スタイルを変更する (pd に ps をセットする)
 *
 * @return 再レイアウトを行うか */

mBool StyleChange(StyleData *pd,StyleData *ps)
{
	mBool ret = FALSE,changefont,changechars;

	//フォントが変わったか

	changefont = (!mStrCompareEq(&ps->strFontText, pd->strFontText.buf)
			|| !mStrCompareEq(&ps->strFontBold, pd->strFontBold.buf)
			|| !mStrCompareEq(&ps->strFontRuby, pd->strFontRuby.buf)
			|| !mStrCompareEq(&ps->strFontInfo, pd->strFontInfo.buf));

	//文字郡が変わったか

	changechars = (mUCS4Compare(ps->b.strNoHead, pd->b.strNoHead)
			|| mUCS4Compare(ps->b.strNoBottom, pd->b.strNoBottom)
			|| mUCS4Compare(ps->b.strHanging, pd->b.strHanging)
			|| mUCS4Compare(ps->b.strNoSep, pd->b.strNoSep)
			|| mUCS4Compare(ps->b.strReplace, pd->b.strReplace));

	//再レイアウトを行うか

	if(ps->b.pages != pd->b.pages
		|| ps->b.chars != pd->b.chars
		|| ps->b.lines != pd->b.lines
		|| (ps->b.flags & (AOSTYLE_F_HANGING|AOSTYLE_F_ENABLE_PICTURE)) != (pd->b.flags & (AOSTYLE_F_HANGING|AOSTYLE_F_ENABLE_PICTURE))
		|| changefont
		|| changechars)
		ret = TRUE;

	//背景画像読み込み

	if(!mStrPathCompareEq(&ps->strBkgndFile, pd->strBkgndFile.buf))
	{
		mImageBufFree(pd->imgBkgnd);

		pd->imgBkgnd = _load_image(&ps->strBkgndFile);
	}

	//データコピー

	StyleCopyData(pd, ps);

	//フォント再作成

	if(changefont)
	{
		StyleCreateFont(pd);

		GlobalSetStyleToLayout();
	}

	return ret;
}

/** 名前からスタイル変更 */

mBool StyleChangeByName(const char *name)
{
	StyleData *ps;
	mBool ret;

	//読み込み

	ps = StyleAlloc();
	if(!ps) return FALSE;

	StyleLoadConf(ps, name);

	//変更

	ret = StyleChange(GDAT->style, ps);

	StyleFreeBuf(ps);

	return ret;
}

/** フォント作成 */

void StyleCreateFont(StyleData *p)
{
	StyleFreeFont(p);

	p->fontText = aoFontCreate(p->strFontText.buf, FALSE);
	p->fontHalf = aoFontCreate(p->strFontText.buf, TRUE);
	p->fontBold = aoFontCreate(p->strFontBold.buf, FALSE);
	p->fontRuby = aoFontCreate(p->strFontRuby.buf, FALSE);
	p->fontInfo = aoFontCreate(p->strFontInfo.buf, FALSE);
}

/** UTF-8 から UTF-32 の文字列をセット (文字郡用) */

void StyleSetUTF32Text(uint32_t **ppbuf,const char *text)
{
	//解放
	
	M_FREE_NULL(*ppbuf);

	//空文字列なら NULL

	if(!text || !(*text)) return;

	//セット

	*ppbuf = mUTF8ToUCS4_alloc(text, -1, NULL);
}
