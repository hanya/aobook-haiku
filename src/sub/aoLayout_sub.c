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
 * レイアウト処理 サブ
 **********************************/

#include "_aolayout.h"
#include "aoLayout.h"

#include "mList.h"
#include "mUtilCharCode.h"



/** AO_TITLEINFO 解放関数 */

static void _destroy_titleinfo(mListItem *p)
{
	if(((AO_TITLEINFO *)p)->text)
		mFree(((AO_TITLEINFO *)p)->text);
}


/** c が match 文字列内に存在するか */

mBool isMatchChar(uint32_t *match,uint32_t c)
{
	if(match)
	{
		for(; *match; match++)
		{
			if(*match == c) return TRUE;
		}
	}

	return FALSE;
}

/** 縦中横の対象文字か */

mBool isHorzInVertChar(uint32_t c)
{
	return ((c >= '0' && c <= '9') || c == '!' || c == '?');
}

/** 行末禁則/分割禁止 文字か */

mBool isNoBottomChar(AO_LAYOUT *dat,AO_LAYOUT_CHAR *pi)
{
	uint32_t c = pi->code;

	//行末禁則

	if(isMatchChar(dat->st->strNoBottom, c))
		return TRUE;

	//分割禁止

	if(isMatchChar(dat->st->strNoSep, c) && pi->i.next
		&& ((AO_LAYOUT_CHAR *)pi->i.next)->code == c)
		return TRUE;

	//くの字点上

	if(c == 0x3033 || c == 0x3034) return TRUE;

	return FALSE;
}

/** 置き換えリストを元に文字を置換え
 *
 * @param text 置き換えリスト (並んだ２つが対となる。半角スペースは無視)
 * @param pc   置き換え対象の文字
 * @return 置き換えが行われたか */

mBool replaceChar(uint32_t *text,uint32_t *pc)
{
	uint32_t c,cc,csrc,cdst;

	if(!text) return FALSE;

	c = *pc;

	while(*text)
	{
		//置換元と置換後の文字取得 (半角スペースはスキップ)

		csrc = cdst = 0;

		while(*text)
		{
			cc = *(text++);
		
			if(cc == ' ')
				continue;
			else if(!csrc)
				csrc = cc;
			else if(!cdst)
			{
				cdst = cc;
				break;
			}
		}

		if(!csrc || !cdst) break;

		//判定

		if(c == csrc)
		{
			*pc = cdst;
			return TRUE;
		}
	}

	return FALSE;
}

/** ルビの親文字が折り返しているかと、親文字の高さ取得 */

int getRubyLeftInfo(AO_LAYOUT *dat,AO_LAYOUT_RUBY *pir,int *pwrap)
{
	AO_LAYOUT_CHAR *pic;
	int i,h = 0,wrap;

	wrap = 0;

	for(pic = pir->charTop, i = pir->charlen; i > 0; i--, pic = (AO_LAYOUT_CHAR *)pic->i.next)
	{
		if(pic->flags & AO_LAYOUT_CHAR_F_WRAP)
		{
			if(pic == pir->charTop)
				wrap |= 1;
			else
				wrap |= 2;
		}

		h += pic->height;
	}

	*pwrap = wrap;

	return h + (pir->charlen - 1) * dat->st->charSpace;
}

/** ルビの親文字が連続しているか */

mBool isRubyConnect(AO_LAYOUT_RUBY *pi,AO_LAYOUT_RUBY *next)
{
	AO_LAYOUT_CHAR *pic;
	int i;

	if(!pi) return FALSE;

	for(pic = pi->charTop, i = pi->charlen; i > 0; i--, pic = (AO_LAYOUT_CHAR *)pic->i.next);

	return (next && pic == next->charTop);
}

/** 見出し終わり時、見出しリストに追加 */

void aolAppendTitleList(AO_LAYOUT *dat)
{
	AO_TITLEINFO *pi;
	uint8_t *ps;

	if(dat->memTitle.curpos == 0) return;

	pi = (AO_TITLEINFO *)mListAppendNew(dat->plistTitle,
			sizeof(AO_TITLEINFO), _destroy_titleinfo);

	if(pi)
	{
		ps = dat->memTitle.buf;

		pi->type = *(ps++);
		pi->pageno = dat->lf->pagenum;
		pi->text = mUCS4ToUTF8_alloc((uint32_t *)ps, (dat->memTitle.curpos - 1) >> 2, NULL);

		//各タイプの個数を加算
		
		(dat->titleNum[pi->type])++;
	}

	//文字列リセット

	mMemAutoReset(&dat->memTitle);
}
