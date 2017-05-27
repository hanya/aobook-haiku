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

/***********************************
 * レイアウト操作関数
 ***********************************/

#include <string.h>

#include "_aolayout.h"
#include "aoLayout.h"

#include "mList.h"
#include "mPopupProgress.h"



//==========================
// sub
//==========================


/** AO_LAYOUT を初期化
 *
 * @param img 描画先。NULL で最初のレイアウトのみ。 */

static void _setLAYOUT(AO_LAYOUT *p,AO_LAYOUT_INFO *info,mPixbuf *img)
{
	AO_STYLE *st = info->style;

	memset(p, 0, sizeof(AO_LAYOUT));

	p->text = (uint8_t *)info->srcbuf;
	p->img = img;
	p->st = info->style;
	p->filepath = info->filepath;

	p->font = info->font;
	p->fontRuby = info->fontRuby;
	p->fontBold = info->fontBold;
	p->fontHalf = info->fontHalf;
	p->fontInfo = info->fontInfo;

	p->fontH = aoFontGetHeight(info->font);
	p->fontRubyH = aoFontGetHeight(info->fontRuby);
	p->fontBoldH = aoFontGetHeight(info->fontBold);
	p->fontHalfH = aoFontGetHeight(info->fontHalf);

	p->lineNext = p->fontH + p->fontRubyH + st->lineSpace;
	p->areaW = st->lines * p->lineNext;
	p->areaH = p->fontH * st->chars + st->charSpace * (st->chars - 1);
	p->rightTextX = p->areaW - p->lineNext;

	if(!img) p->plistTitle = &info->listTitle;
}

/** ページ位置の補正 */

static AO_PAGEINDEX *_page_adjust(AO_LAYOUT_INFO *info,AO_PAGEINDEX *pi)
{
	if(!pi)
		//NULL なら終端
		return aoGetPageHomeEnd(info, TRUE);
	else if(info->style->pages == 2 && pi && (pi->dat.pageNo & 1))
		//見開きの場合、奇数位置なら一つ戻る
		return (AO_PAGEINDEX *)pi->i.prev;
	else
		return pi;
}


//==========================
// 情報取得
//==========================


/** 画面サイズ取得 */

void aoGetScreenSize(AO_LAYOUT_INFO *info,mSize *sz)
{
	int fonth,rubyh;
	AO_STYLE *st = info->style;

	fonth = aoFontGetHeight(info->font);
	rubyh = aoFontGetHeight(info->fontRuby);

	sz->w = ((fonth + rubyh + st->lineSpace) * st->lines) * st->pages
				+ st->pageSpace * (st->pages - 1)
				+ st->margin.x1 + st->margin.x2;
	
	sz->h = (fonth + st->charSpace) * st->chars - st->charSpace
				+ st->margin.y1 + st->margin.y2;
}


//==========================
// ページ操作
//==========================


/** 先頭/終端のページ取得 */

AO_PAGEINDEX *aoGetPageHomeEnd(AO_LAYOUT_INFO *info,mBool end)
{
	AO_PAGEINDEX *pi;

	pi = (AO_PAGEINDEX *)((end)? info->listPage.bottom: info->listPage.top);

	return (pi)? _page_adjust(info, pi): NULL;
}

/** ページ番号からページ位置取得 */

AO_PAGEINDEX *aoGetPageIndex(AO_LAYOUT_INFO *info,int page)
{
	mListItem *pi;
	int i;

	for(i = 0, pi = info->listPage.top; pi && i < page; i++, pi = pi->next);

	return _page_adjust(info, (AO_PAGEINDEX *)pi);
}

/** 行番号からページを取得 */

AO_PAGEINDEX *aoGetPageIndexByLine(AO_LAYOUT_INFO *info,int line)
{
	AO_PAGEINDEX *pi,*next;

	for(pi = (AO_PAGEINDEX *)info->listPage.top; pi; pi = next)
	{
		next = (AO_PAGEINDEX *)pi->i.next;

		if(!next
			|| (pi->dat.lineno <= line && line < next->dat.lineno))
			break;
	}

	return _page_adjust(info, pi);
}

/** 指定方向に指定数移動したページを取得 */

AO_PAGEINDEX *aoMovePage(AO_LAYOUT_INFO *info,AO_PAGEINDEX *pi,int dir)
{
	int i;

	dir *= info->style->pages;

	if(dir < 0)
	{
		//前方向
		for(i = -dir; pi->i.prev && i > 0; i--, pi = (AO_PAGEINDEX *)pi->i.prev);
	}
	else
	{
		//次方向
		for(i = dir; pi->i.next && i > 0; i--, pi = (AO_PAGEINDEX *)pi->i.next);
	}

	return _page_adjust(info, pi);
}

/** ページ番号取得 */

int aoGetPageNo(AO_PAGEINDEX *pi)
{
	return (pi)? pi->dat.pageNo: 0;
}

/** ページの先頭のテキスト行番号取得 */

int aoGetPageLineNo(AO_PAGEINDEX *pi)
{
	return (pi)? pi->dat.lineno: 0;
}


//==========================
// レイアウト＆描画
//==========================


/** レイアウト情報解放 */

void aoFreeLayoutInfo(AO_LAYOUT_INFO *info)
{
	mListDeleteAll(&info->listPage);
	mListDeleteAll(&info->listTitle);
}

/** 最初のレイアウト処理
 *
 * ページの情報を作成 + 見出しの抽出 */

void aoSetLayoutFirst(AO_LAYOUT_INFO *info,mPopupProgress *poppg)
{
	AO_LAYOUT dat;
	AO_LAYOUT_FIRST lf;
	AO_PAGEINFO_ITEM *pi;
	uint8_t *topbuf;

	//現在のデータを解放

	aoFreeLayoutInfo(info);

	//info からデータセット

	_setLAYOUT(&dat, info, NULL);

	dat.lf = &lf;

	//見出し文字列用バッファ

	mMemAutoAlloc(&dat.memTitle, 1024, 1024);

	//AO_LAYOUT_FIRST 初期化

	memset(&lf, 0, sizeof(AO_LAYOUT_FIRST));

	lf.cur.src = dat.text;

	//フォントキャッシュ数セット

	aoFontSetCacheMax(dat.font, 100);
	aoFontSetCacheMax(dat.fontRuby, 50);

	//各ページ情報セット

	topbuf = dat.text;

	while(layoutPage(&dat, &lf))
	{
		//ページ追加
		
		pi = (AO_PAGEINFO_ITEM *)mListAppendNew(&info->listPage,
				sizeof(AO_PAGEINFO_ITEM), NULL);

		if(pi)
		{
			lf.cur.pageNo = lf.pagenum;
		
			pi->dat = lf.cur;
		
			lf.pagenum++;
		}

		lf.cur = lf.next;

		//進捗

		mPopupProgressThreadSetPos(poppg,
			(int)((double)(dat.text - topbuf) / info->srcsize * 100 + 0.5));
	}

	//ページ数

	info->pagenum = lf.pagenum;

	//見出し個数

	memcpy(info->titleNum, dat.titleNum, sizeof(int) * 3);

	//解放

	mMemAutoFree(&dat.memTitle);
}

/** ページを描画 */

void aoDrawPage(AO_LAYOUT_INFO *info,mPixbuf *img,AO_PAGEINDEX *page)
{
	AO_LAYOUT dat;
	AO_PAGEINDEX *next;

	if(!page) page = (AO_PAGEINFO_ITEM *)info->listPage.top;
	if(!page) return;

	_setLAYOUT(&dat, info, img);

	dat.pagenum = info->pagenum;

	//描画

	if(info->style->pages == 1)
		layoutDrawPage(&dat, &page->dat, -1);
	else
	{
		layoutDrawPage(&dat, &page->dat, 0);

		next = (AO_PAGEINDEX *)page->i.next;
		if(next) layoutDrawPage(&dat, &next->dat, 1);
	}
}
