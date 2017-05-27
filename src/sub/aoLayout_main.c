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
 * レイアウト メイン処理 (作業用)
 ***********************************/

#include <string.h>

#include "_aolayout.h"
#include "_aobufdef.h"

#include "mList.h"


//==============================
// 本文文字位置セット
//==============================


/** 現在の字下げ/地上げの Y 位置取得
 *
 * dat->jisageY     : 先頭行の字下げ
 * dat->jisageWrapY : 折り返し後の字下げ
 * dat->jiageBottom : 字上げの下端 */

static void _getTopBottomY(AO_LAYOUT *dat)
{
	int top,wrap,bottom,charh,max;

	//字下げ数 (行単位を優先)

	if(dat->linestate.jisage)
		top = wrap = dat->linestate.jisage;
	else
	{
		top = dat->blockstate.jisage;
		wrap = dat->blockstate.jisageWrap;
	}

	//字上げ数

	if(dat->linestate.jitsuki || dat->blockstate.jitsuki)
		bottom = ((dat->blockstate.jitsuki)? dat->blockstate.jitsuki: dat->linestate.jitsuki) - 1;
	else
		bottom = 0;

	//最大値調整

	max = dat->st->chars - 1;

	if(top > max) top = max;
	if(wrap > max) wrap = max;
	if(bottom > max) bottom = max;

	if(top + bottom > dat->st->chars) top = 0;
	if(wrap + bottom > dat->st->chars) wrap = 0;

	//セット

	charh = dat->fontH + dat->st->charSpace;

	if(bottom)
		bottom = bottom * dat->fontH + (bottom - 1) * dat->st->charSpace;

	dat->jisageY = top * charh;
	dat->jisageWrapY = wrap * charh;
	dat->jiageBottom = dat->areaH - bottom;
}

/** １行分の本文文字の描画位置セット
 *
 * 地付き/地上げは Y 位置は上端からの位置のままになっているので、描画時に処理する。
 *
 * @param ptcur  現在の行の px 位置。次の行の位置が返る
 * @param prevWrapNum  前ページの前行からの折り返しで、前ページの分の行数
 * @param center_diffx ページ左右中央の場合の X 位置差分 (描画時)
 * @return 現在の行が次ページに渡って折り返す場合、先頭から何行ずらすか */

static int _setTextPos(AO_LAYOUT *dat,mPoint *ptcur,int prevWrapNum,int center_diffx)
{
	AO_LAYOUT_CHAR *p,*pnext;
	int x,y,ch,chnext,charsp,wrap,bottomY[2];
	int wrapCnt = 0,	//折り返し行数
		nextWrapNum = 0;	//次ページで折り返しをスキップする行数
	mBool bwrapdown = FALSE, //次がぶら下げか
		bJiageStart = 0;

	//字下げ/字上げの Y 位置

	_getTopBottomY(dat);

	//下端位置 ([0]通常時 [1]地付き/字上げ時)

	bottomY[0] = dat->areaH;
	bottomY[1] = dat->jiageBottom;

	//------------

	x = ptcur->x + dat->lineNext * prevWrapNum; //前ページの折り返し分をずらす
	y = dat->jisageY;

	charsp = dat->st->charSpace;

	for(p = (AO_LAYOUT_CHAR *)dat->listChar.top; p; p = pnext)
	{
		pnext = (AO_LAYOUT_CHAR *)p->i.next;
	
		ch = p->height;
		chnext = (pnext)? pnext->height: 0;

		//地付き/地上げ開始

		if(!bJiageStart && (p->flags & AO_LAYOUT_CHAR_F_JITSUKI))
			bJiageStart = 1;

		//折り返し/ぶら下げ判定
		/* wrap: [0]なし [1]折り返し [2]次をぶら下げ [3]位置セット後折り返し */

		wrap = 0;

		if(bwrapdown)
			//次がぶら下げで予約されている時
			wrap = 3;
		else if(y + ch > bottomY[bJiageStart])
			//現在の文字が下端を超えるので、折り返し
			wrap = 1;
		else if(y + ch + charsp + chnext > bottomY[bJiageStart])
		{
			//次の文字が下端を超える
		
			if(isNoBottomChar(dat, p))
				//現在の文字が行末になるので、行末禁則/分割禁止文字の場合、次行へ
				wrap = 1;
			else if(pnext && isMatchChar(dat->st->strNoHead, pnext->code))
			{
				//次の文字が行頭になるので、行頭禁則文字の場合、ぶら下げ or 折り返し

				if((dat->st->flags & AOSTYLE_F_HANGING)
						&& isMatchChar(dat->st->strHanging, pnext->code))
					wrap = 2;
				else
					wrap = 1;
			}
		}

		if(wrap == 2)
			//次の文字をぶら下げとして予約
			bwrapdown = TRUE;
		else
		{
			bwrapdown = FALSE;

			//折り返し

			if(wrap == 1)
			{
				x -= dat->lineNext;
				y = dat->jisageWrapY;
				wrapCnt++;
			}
		}

		//文字描画位置セット

		p->x = x - center_diffx;
		p->y = y;

		//折り返しの先頭文字にフラグを付加

		if(y == dat->jisageWrapY && p->i.prev)
			p->flags |= AO_LAYOUT_CHAR_F_WRAP;

		//次の文字のY位置

		y += ch + charsp;

		//ページにまたがる折り返し行数
		//最初に範囲外になった時に現在の折り返し数を記憶

		if(x < 0 && !nextWrapNum)
			nextWrapNum = wrapCnt;

		//位置セット後に折り返し (次文字がある場合)

		if(wrap == 3 && pnext)
		{
			x -= dat->lineNext;
			y = dat->jisageWrapY;
			wrapCnt++;
		}
	}

	//次行の位置

	ptcur->x = x - dat->lineNext;
	ptcur->y = 0;

	return nextWrapNum;
}

/** [描画時] 地付き/地上げ処理
 *
 * Y 位置を実際の位置に調整。 */

static void _procJitsuki(AO_LAYOUT *dat)
{
	AO_LAYOUT_CHAR *next,*top,*end,*start,*p;
	int charspace,texth,toph,upperH,n,jisageH;
	mBool findStart = FALSE;

	charspace = dat->st->charSpace;

	//字上げ文字数 (0 で地付き)

	n = (dat->blockstate.jitsuki)? dat->blockstate.jitsuki: dat->linestate.jitsuki;
	n--;
	if(n < 0) return;

	//上げる高さ

	if(n == 0)
		upperH = 0;
	else
		upperH = n * dat->fontH + (n - 1) * charspace;

	//各文字の Y 位置をずらす

	jisageH = dat->jisageY;

	for(top = (AO_LAYOUT_CHAR *)dat->listChar.top; top; top = (AO_LAYOUT_CHAR *)end->i.next)
	{
		//地上げの1行分の範囲 (start,end)、地付き開始位置、1行分の文字列高さ (texth)

		start = top;
		texth = toph = 0;

		for(end = (AO_LAYOUT_CHAR *)top; 1; end = next)
		{
			next = (AO_LAYOUT_CHAR *)end->i.next;

			if(!findStart && (end->flags & AO_LAYOUT_CHAR_F_JITSUKI))
			{
				//地上げ開始
				
				start = end;
				toph = texth;
				texth = 0;
				findStart = TRUE;
			}

			texth += end->height;

			if(!next) break;
			if(next->flags & AO_LAYOUT_CHAR_F_WRAP) break;

			texth += charspace;
		}

		//Y 位置をずらす

		if(findStart)
		{
			n = dat->areaH - toph - texth - upperH - jisageH;
			if(n < 0) n = 0;

			for(p = start; 1; p = (AO_LAYOUT_CHAR *)p->i.next)
			{
				p->y += n;
			
				if(p == end) break;
			}
		}

		//折り返し以降の字下げ

		jisageH = dat->jisageWrapY;
	}
}


//==============================
// ルビ文字位置セット
//==============================


/** ルビの描画位置セット */

static void _setRubyPos(AO_LAYOUT *dat)
{
	AO_LAYOUT_RUBY *pi,*next,*prev = NULL;
	int x,y,n,charh,prevBottom,wrap;

	for(pi = (AO_LAYOUT_RUBY *)dat->listRuby.top; pi; prev = pi, pi = next)
	{
		next = (AO_LAYOUT_RUBY *)pi->i.next;

		//親文字の情報
		/* charh : 親文字の高さ
		 * wrap  : [0bit] 先頭文字が折り返し [1bit] 先頭文字以降が折り返し */

		charh = getRubyLeftInfo(dat, pi, &wrap);

		//親文字の位置

		x = pi->charTop->x;
		y = pi->charTop->y;

		//前のルビの下端 Y 位置 (-1 でルビがない、または行が違う)

		prevBottom = (prev && prev->x == x)? prev->y + prev->height: -1;

		//

		if(prevBottom != -1 && prevBottom > y)
		{
			/* 前のルビが同じ行でかつ現在の親文字の位置にかぶさっている場合、
			 * 前回の終端位置から続ける */
			 
			y = prevBottom;
		}
		else if(pi->height >= charh)
		{
			/* ルビ文字全体が親文字より長い場合は、中央揃え。
			 * ただし、親文字が折り返している、または前後どちらかにルビが続く場合は除く。 */

			if(!wrap && !isRubyConnect(prev, pi) && !isRubyConnect(pi, next))
			{
				n = y - (pi->height - charh) / 2;
				if(prevBottom != -1 && n < prevBottom) n = prevBottom;

				y = n;
			}
		}
		else
		{
			/* 親文字内に収まる場合、ルビに余白を付ける。
			 * 親文字が折り返す場合、ルビ文字数と親文字数が同じなら１文字に１ルビとする。 */

			if(wrap & 2)
			{
				if(pi->rubylen == pi->charlen)
					pi->areaH = -2;
			}
			else
				pi->areaH = charh;
		}
		
		//セット

		pi->x = x;
		pi->y = y;
	}
}


//==============================
// sub
//==============================


/** ページ単位のコマンド処理
 *
 * @param pageno 現在のページ位置
 * @return 0:改ページ 1:次ページを空白に&改ページ 2:コマンドを無視 */

static int _procLayoutCommand(AO_LAYOUT *dat,int cmd,mPoint *ptcur,int pageno)
{
	int ret = -1;	//-1 で改ページ
	mBool topline;

	topline = (ptcur->x == dat->rightTextX);

	switch(cmd)
	{
		//改丁
		/* 現在右側のページなら改ページ。
		 * 左側のページなら次ページを空白にしてその次から開始 */
		case AO_CMD_KAITYO:
			if(dat->st->pages == 2)
				ret = (pageno & 1)? 1: 0;
			break;
		//改見開き
		/* 現在右側のページなら改ページ。
		 * 左側のページの場合は、次ページを空白にしてその次から。
		 * ただし、先頭ページでかつ先頭行の場合は無視する。 */
		case AO_CMD_KAIMIHIRAKI:
			if(dat->st->pages == 2)
			{
				if(pageno & 1)
					ret = 0;
				else
					//先頭ページの先頭行の場合、続行
					ret = (pageno == 0 && topline)? 2: 1;
			}
			break;
	}

	//

	if(ret == -1)
		//通常改ページ (ページの先頭行なら無視)
		return (topline)? 2: 0;
	else
		return ret;
}


//=============================
// ページレイアウト、描画
//=============================


/** 1ページのレイアウト
 *
 * lf->cur に現在のページ情報をセットし、
 * lf->next に次のページの情報をセット。
 *
 * @return FALSE でデータ終了 */

mBool layoutPage(AO_LAYOUT *dat,AO_LAYOUT_FIRST *lf)
{
	mPoint ptCur;
	uint8_t *curtop;
	int wrapNum,cmd,ret;
	AO_BLOCKSTATE blockstate;

	memset(&dat->pagestate, 0, sizeof(AO_PAGESTATE));

	dat->text = lf->cur.src;
	dat->blockstate = lf->cur.blockstate;
	dat->curline = lf->cur.lineno;

	wrapNum = lf->cur.wrapNum;

	lf->cur.diffx = 0;
	lf->next.flags = 0;

	//データの終端なら終了

	if(*(dat->text) == AO_TYPE_END) return FALSE;

	//空白ページの場合は、次ページの処理へ

	if(lf->cur.flags & AO_PAGEINFO_F_BLANK) goto NEXT;

	//----- 現在のページを、各行ごとに処理

	ptCur.x = dat->rightTextX;
	ptCur.y = 0;

	while(ptCur.x >= 0)
	{
		//次ページの折り返し用に保存
	
		curtop = dat->text;
		blockstate = dat->blockstate;
	
		//1行分の文字データと状態を取得
	
		if(!layoutGetLine(dat, &cmd)) break;

		//挿絵

		if(dat->pagestate.picture
			&& (dat->st->flags & AOSTYLE_F_ENABLE_PICTURE))
			break;

		//ページ単位のコマンド処理

		if(cmd != -1)
		{
			ret = _procLayoutCommand(dat, cmd, &ptCur, lf->pagenum);

			//次ページを空白に
			
			if(ret == 1)
				lf->next.flags |= AO_PAGEINFO_F_BLANK;

			if(ret == 2)
				//コマンドを無視
				continue;
			else
				break;
		}
		 
		//本文文字の描画位置セット

		wrapNum = _setTextPos(dat, &ptCur, wrapNum, 0);

		//文字データ解放

		mListDeleteAll(&dat->listChar);
		mListDeleteAll(&dat->listRuby);
	}

	//ページの左右中央位置 (次ページに続く折り返しがある場合は除く)

	if(dat->pagestate.center && !wrapNum)
		lf->cur.diffx = (ptCur.x + dat->lineNext) / 2;

	//------ 次のページ情報
	
NEXT:
	//次のページの行番号
	
	lf->next.lineno = dat->curline;

	//挿絵で先頭行でない場合、次ページは挿絵データの先頭から

	if((dat->st->flags & AOSTYLE_F_ENABLE_PICTURE)
		&& dat->pagestate.picture && ptCur.x != dat->rightTextX)
		dat->text = dat->pagestate.picture - 1;

	//次ページに折り返しが続く場合

	lf->next.wrapNum = wrapNum;

	if(wrapNum)
	{
		//最後の行を再処理
		lf->next.src = curtop;
		lf->next.blockstate = blockstate;
	}
	else
	{
		//折り返しがないなら、現在の状態をセット
		lf->next.src = dat->text;
		lf->next.blockstate = dat->blockstate;
		lf->next.lineno++;
	}

	return TRUE;
}

/** 1ページの描画
 *
 * @param page   描画するページ情報
 * @param pageno 単ページの場合 -1、見開きの場合はページ位置 (0:右 1:左) */

void layoutDrawPage(AO_LAYOUT *dat,AO_PAGEINFO *page,int pageno)
{
	mPoint ptCur;
	int wrapNum,cmd,ret;

	//ページ数描画

	if(dat->st->flags & AOSTYLE_F_DRAW_PAGENO)
		layoutDrawPageNo(dat, page->pageNo, pageno);

	//空白ページの場合、何も描画しない

	if(page->flags & AO_PAGEINFO_F_BLANK) return;

	//

	memset(&dat->pagestate, 0, sizeof(AO_PAGESTATE));

	dat->text = page->src;
	dat->blockstate = page->blockstate;

	ptCur.x = dat->rightTextX;
	ptCur.y = 0;

	wrapNum = page->wrapNum;

	//

	while(ptCur.x >= 0)
	{
		//1行分取得
	
		if(!layoutGetLine(dat, &cmd)) break;

		//挿絵

		if(dat->pagestate.picture
			&& (dat->st->flags & AOSTYLE_F_ENABLE_PICTURE))
		{
			//ページ先頭でない場合、次ページへ

			if(ptCur.x == dat->rightTextX)
				layoutDrawPicture(dat, pageno);

			break;
		}

		//ページ単位のコマンド

		if(cmd != -1)
		{
			ret = _procLayoutCommand(dat, cmd, &ptCur, page->pageNo);

			if(ret == 2)
				continue;
			else
				break;
		}
		 
		//描画位置セット

		_setTextPos(dat, &ptCur, wrapNum, page->diffx);

		_procJitsuki(dat);
		
		_setRubyPos(dat);

		wrapNum = 0;

		//描画

		layoutDrawLine(dat, pageno);
		
		//文字データ解放

		mListDeleteAll(&dat->listChar);
		mListDeleteAll(&dat->listRuby);
	}
}
