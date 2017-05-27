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
 * グローバルデータ
 *****************************************/

#include "mDef.h"
#include "mStr.h"
#include "mUtilStr.h"

#include "globaldata.h"
#include "bookmarkdat.h"
#include "style.h"

#include "aoLayout.h"
#include "aoText.h"



//============================


/** ファイル履歴配列にセット */

static void _setFileRecentArray(mStr *array,int arraynum,const char *filename,mStr *strText)
{
	int i,find;
	mStr str = MSTR_INIT;

	//同じファイル名があるか

	for(i = 0, find = -1; i < arraynum; i++)
	{
		mStrGetSplitText(&str, array[i].buf, '\t', 0);
	
		if(mStrPathCompareEq(&str, filename))
		{
			find = i;
			break;
		}
	}

	mStrFree(&str);

	//セット

	mStrArraySetRecent(array, arraynum, find, strText->buf);
}


//============================



/** グローバルデータ解放 */

void GlobalDataFree(GlobalData *p)
{
	StyleFreeBuf(p->style);

	//レイアウトデータ

	aoFreeLayoutInfo(p->layout);
	mFree(p->layout);

	//しおりデータ

	BookmarkFree();

	//mStr

	mStrFree(&p->strFileName);
	mStrFree(&p->strOpenPath);
	mStrFree(&p->strBkmarkPath);
	mStrFree(&p->strLayoutFilePath);
	mStrFree(&p->strGUIFont);
	
	mStrArrayFree(p->strRecentFile, RECENTFILE_NUM);
	mStrArrayFree(p->strTool, TOOLITEM_NUM);

	//buf

	mFree(p->textbuf.buf);
	mFree(p->shortcutkey);

	//

	mFree(p);
}

/** グローバルデータ確保 */

mBool GlobalDataNew()
{
	GlobalData *p;

	//確保

	p = (GlobalData *)mMalloc(sizeof(GlobalData), TRUE);
	if(!p) return FALSE;

	GDAT = p;

	//レイアウトデータ

	p->layout = (AO_LAYOUT_INFO *)mMalloc(sizeof(AO_LAYOUT_INFO), TRUE);
	if(!p->layout) return FALSE;

	//スタイル

	p->style = StyleAlloc();

	return TRUE;
}

/** 現在のファイルを閉じる時 */

void GlobalCloseFile()
{
	char m[32];

	if(GLOBAL_ISNOT_HAVE_TEXT) return;

	//履歴に行数をセット

	mIntToStr(m, aoGetPageLineNo(GDAT->curpage));

	mStrReplaceSplitText(&GDAT->strRecentFile[0], '\t', 2, m);
}

/** 現在のテキストデータをクリア */

void GlobalEmptyTextData()
{
	GlobalData *p = GDAT;

	M_FREE_NULL(p->textbuf.buf);

	//ページ・タイトルデータ解放

	aoFreeLayoutInfo(p->layout);

	p->curpage = NULL;
}

/** テキストデータをセット */

void GlobalSetTextData(mBuf *buf)
{
	GDAT->textbuf = *buf;

	GDAT->layout->srcbuf  = buf->buf;
	GDAT->layout->srcsize = buf->size;
}

/** ファイルの履歴を追加 */

void GlobalAddRecentFile(const char *filename,int code,int line)
{
	mStr str = MSTR_INIT;

	//ファイル名 + '\t' + 文字コード名 + '\t' + 行数

	mStrSetFormat(&str, "%s\t%s\t%d",
		filename, aoTextGetCodeName(code), line);

	_setFileRecentArray(GDAT->strRecentFile, RECENTFILE_NUM,
		filename, &str);

	mStrFree(&str);
}

/** ファイル履歴から情報取得 */

void GlobalGetRecentFileInfo(int no,mStr *strfname,int *code,int *line)
{
	mStr *src,str = MSTR_INIT;
	int i;

	src = GDAT->strRecentFile + no;

	//ファイル名

	mStrGetSplitText(strfname, src->buf, '\t', 0);

	//文字コード

	mStrGetSplitText(&str, src->buf, '\t', 1);

	*code = -1;

	for(i = 0; i < AOTEXT_CODE_NUM; i++)
	{
		if(mStrCompareCaseEq(&str, aoTextGetCodeName(i)))
		{
			*code = i;
			break;
		}
	}

	//行数

	mStrGetSplitText(&str, src->buf, '\t', 2);

	*line = mStrToInt(&str);

	//

	mStrFree(&str);
}

/** スタイルデータをレイアウト用データにセット */

void GlobalSetStyleToLayout()
{
	StyleData *s = GDAT->style;
	AO_LAYOUT_INFO *l = GDAT->layout;

	l->style = &s->b;
	l->font = s->fontText;
	l->fontHalf = s->fontHalf;
	l->fontBold = s->fontBold;
	l->fontRuby = s->fontRuby;
	l->fontInfo = s->fontInfo;
}

/** レイアウトデータにファイルのパスをセット */

void GlobalSetLayoutFilePath()
{
	mStrPathGetDir(&GDAT->strLayoutFilePath, GDAT->strFileName.buf);

	GDAT->layout->filepath = GDAT->strLayoutFilePath.buf;
}
