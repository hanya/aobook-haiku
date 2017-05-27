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
 * レイアウト - 描画
 **********************************/

#include <stdio.h>

#include "_aolayout.h"

#include "mPixbuf.h"
#include "mImageBuf.h"
#include "mLoadImage.h"
#include "mStr.h"
#include "mRectBox.h"
#include "mUtilStr.h"


//---------------

//傍点文字
static uint32_t g_bouten_char[] = {
	0xfe45, 0xfe46, 0x25c9, 0x25cf, 0x25cb, 0x25b2, 0x25b3, 0x25ce, L'×'
};

//---------------


//=============================
// 傍線描画
//=============================


/** mPixbuf 1xN パターンを描画 */

static void _drawPixbufPattern(mPixbuf *img,
	int x,int y,int h,uint32_t col,uint8_t pat,int patw)
{
	int iy,pcnt;
	uint8_t f;

	for(iy = 0, f = 0x80, pcnt = patw; iy < h; iy++)
	{
		if(pat & f)
			mPixbufSetPixel(img, x, y + iy, col);
	
		//

		pcnt--;
		if(pcnt)
			f >>= 1;
		else
		{
			f = 0x80;
			pcnt = patw;
		}
	}
}

/** mPixbuf 波線を描画 */

static void _drawPixbufWave(mPixbuf *img,int x,int y,int h,uint32_t col)
{
	int iy,py;
	uint8_t pat[] = { 1,1,2,2,2,1,1,0,0,0 };

	for(iy = 0, py = 0; iy < h; iy++)
	{
		mPixbufSetPixel(img, x + pat[py], y + iy, col);
	
		py++;
		if(py == 10) py = 0;
	}
}

/** 傍線描画 */

static void _drawPixbufBousen(mPixbuf *img,int x,int y,int h,uint32_t col,int type)
{
	switch(type)
	{
		//通常傍線
		case 1:
			mPixbufLineV(img, x, y, h, col);
			break;
		//二重傍線
		case 2:
			mPixbufLineV(img, x, y, h, col);
			mPixbufLineV(img, x + 2, y, h, col);
			break;
		//鎖線
		case 3:
			_drawPixbufPattern(img, x, y, h, col, 0xc0, 4);
			break;
		//破線
		case 4:
			_drawPixbufPattern(img, x, y, h, col, 0xf8, 8);
			break;
		//波線
		case 5:
			_drawPixbufWave(img, x, y, h, col);
			break;
	}
}


//=============================
//
//=============================


/** ルビ描画 */

static void _drawRuby(AO_LAYOUT *dat,int xtop,int ytop,int page)
{
	AO_LAYOUT_RUBY *pi;
	AO_LAYOUT_CHAR *pichar;
	int i,x,y,rubyh,bottom,addx;
	uint32_t col;

	xtop += dat->fontH;

	col = dat->st->colRuby;
	rubyh = dat->fontRubyH;

	for(pi = (AO_LAYOUT_RUBY *)dat->listRuby.top; pi; pi = (AO_LAYOUT_RUBY *)pi->i.next)
	{
		if(pi->x < 0) break;

		x = pi->x;
		y = pi->y;

		//傍線がある場合は x+2
		addx = (pi->charTop->bousen)? 2: 0;

		//親文字の先頭
		pichar = pi->charTop;

		//

		if(pi->areaH == -2)
		{
			//----- 親文字の横にルビを１文字つける

			for(i = 0; i < pi->rubylen; i++, pichar = (AO_LAYOUT_CHAR *)pichar->i.next)
			{
				x = pichar->x;
			
				if(x >= 0 && x < dat->areaW)
				{
					aoFontDrawText(dat->fontRuby, dat->img,
						xtop + x + addx, ytop + pichar->y,
						pi->ruby + i, 1, col, AOFONT_DRAWTEXT_F_VERT);
				}
			}
		}
		else if(pi->areaH == -1)
		{
			//----- 余白なし

			//折り返しの下端

			bottom = (pichar->flags & AO_LAYOUT_CHAR_F_JITSUKI)
						? dat->jiageBottom: dat->areaH;

			//各ルビ文字

			for(i = 0; i < pi->rubylen; i++, y += rubyh)
			{
				//折り返し
				
				if(y >= bottom)
				{
					x -= dat->lineNext;
					y = dat->jisageWrapY;

					//見開き右ページの終端行の場合、左ページ位置へ
					
					if(page == 0 && x < 0)
						x -= dat->st->pageSpace;
				}

				//1文字描画
				/* 前ページからの折り返しの文字がある場合があるため、
				 * x の範囲は1文字ずつ判定する */

				if(x >= 0 && x < dat->areaW)
				{
					aoFontDrawText(dat->fontRuby, dat->img,
						x + xtop + addx, y + ytop,
						pi->ruby + i, 1, col, AOFONT_DRAWTEXT_F_VERT);
				}
			}
		}
		else
		{
			//---- 余白あり (折り返しなし)

			if(x >= dat->areaW) continue;

			x += xtop + addx;
			y += ytop;

			for(i = 0; i < pi->rubylen; i++, y += rubyh)
			{
				aoFontDrawText(dat->fontRuby, dat->img,
					x,
					y + (i + 1) * (pi->areaH - pi->height) / (pi->rubylen + 1),
					pi->ruby + i, 1, col, AOFONT_DRAWTEXT_F_VERT);
			}
		}
	}
}

/** 本文描画 */

static void _drawText(AO_LAYOUT *dat,int xtop,int ytop)
{
	mPixbuf *img;
	AO_LAYOUT_CHAR *p,*next;
	uint32_t flags,col,c;
	int x,y,n1,fontno,fonth;
	mBool bDashToLine;
	aoFont *font[2];

	font[0] = dat->font;
	font[1] = dat->fontBold;

	col = dat->st->colText;
	img = dat->img;
	fonth = dat->fontH;

	//全角ダッシュは直線で描画
	
	bDashToLine = ((dat->st->flags & AOSTYLE_F_DASH_TO_LINE) != 0);

	//

	for(p = (AO_LAYOUT_CHAR *)dat->listChar.top; p; p = next)
	{
		next = (AO_LAYOUT_CHAR *)p->i.next;
	
		if(p->x < 0) break;
		if(p->x >= dat->areaW) continue; //前ページの折り返し部分

		x = p->x + xtop;
		y = p->y + ytop;

		fontno = ((p->flags & AO_LAYOUT_CHAR_F_BOLD) != 0);

		//------- 文字

		if(p->chartype == AO_LAYOUT_CHAR_TYPE_HORZ_IN_VERT)
		{
			//----- [縦中横]

			aoFontDrawText8(font[fontno], img,
				x + (fonth - p->width) / 2, y,
				p->horzchar, -1, col);
		}
		else
		{
			//----- [通常 or 横組み]

			//フラグ
		
			flags = 0;

			if(p->chartype == AO_LAYOUT_CHAR_TYPE_VERT_ROTATE)
				//横組み
				flags |= AOFONT_DRAWTEXT_F_VERT_ROTATE;
			else
				flags |= AOFONT_DRAWTEXT_F_VERT;

			//文字

			if(bDashToLine
				&& (p->code == 0x2015 || p->code == 0x2500)	//全角ダッシュと罫線
				&& p->chartype == AO_LAYOUT_CHAR_TYPE_NORMAL)
			{
				//横線文字を直線にして描画

				n1 = (next
						&& (next->code == 0x2015 || next->code == 0x2500)
						&& next->chartype == AO_LAYOUT_CHAR_TYPE_NORMAL
						&& !(next->flags & AO_LAYOUT_CHAR_F_WRAP))
						? dat->fontH + dat->st->charSpace: dat->fontH;

				mPixbufLineV(img, x + (fonth >> 1), y, n1, mRGBtoPix(col));
			}
			else
				aoFontDrawText(font[fontno], img, x, y, &p->code, 1, col, flags);

			//結合文字 (濁点、半濁点)

			if(p->flags & (AO_LAYOUT_CHAR_F_DAKUTEN | AO_LAYOUT_CHAR_F_HANDAKUTEN))
			{
				c = (p->flags & AO_LAYOUT_CHAR_F_DAKUTEN)? 0x3099: 0x309a;
			
				aoFontDrawText(font[fontno], img, x, y, &c, 1, col, flags);
			}
		}

		//----- 傍点
		/* ゴマと蛇の目は本文フォント、それ以外は半分のサイズのフォントで */

		if(p->bouten)
		{
			n1 = (p->bouten > 3);
		
			aoFontDrawText((n1)? dat->fontHalf: dat->font,
				img,
				x + fonth + n1,
				y + (n1? (p->height - dat->fontHalfH) / 2: 0),
				&g_bouten_char[p->bouten - 1], 1,
				col,
				AOFONT_DRAWTEXT_F_VERT | (n1? 0: AOFONT_DRAWTEXT_F_NO_LEFT));
		}
	}
}

/** 傍線を描画 */

static void _drawBousen(AO_LAYOUT *dat,int xtop,int ytop)
{
	AO_LAYOUT_CHAR *p,*top,*next;
	uint32_t col;
	int h,x,y,type,charspace;

	col = mRGBtoPix(dat->st->colText);

	charspace = dat->st->charSpace;

	for(top = (AO_LAYOUT_CHAR *)dat->listChar.top; top; top = p)
	{
		//傍線開始位置

		for(p = top; p && !p->bousen; p = (AO_LAYOUT_CHAR *)p->i.next);

		if(!p) break;

		//終了位置まで描画

		x = p->x;
		y = p->y;
		h = 0;
		type = p->bousen;

		for(; p && p->bousen == type; p = next)
		{
			next = (AO_LAYOUT_CHAR *)p->i.next;

			h += p->height;

			if(!next
				|| (next->bousen != type || (next->flags & AO_LAYOUT_CHAR_F_WRAP)))
			{
				if(x >= 0 && x < dat->areaW)
				{
					_drawPixbufBousen(dat->img,
						x + dat->fontH + xtop, y + ytop, h, col, type);
				}

				if(next)
				{
					x = next->x;
					y = next->y;
				}
				
				h = 0;
			}
			else
				h += charspace;
		}
	}
}

/** １行分を描画 */

void layoutDrawLine(AO_LAYOUT *dat,int page)
{
	int xtop,ytop;

	xtop = dat->st->margin.x1;
	ytop = dat->st->margin.y1;

	//見開きで右ページの場合

	if(page == 0)
		xtop += dat->st->pageSpace + dat->areaW;

	//本文

	_drawText(dat, xtop, ytop);

	//傍線

	_drawBousen(dat, xtop, ytop);

	//ルビ

	_drawRuby(dat, xtop, ytop, page);
}

/** ページ番号を描画
 *
 * @param pos  -1:単ページ 0:右側 1:左側 */

void layoutDrawPageNo(AO_LAYOUT *dat,int pageno,int pos)
{
	uint8_t m[64];
	int x;

	if(pos == -1 || pos == 0)
		snprintf((char *)m, 64, "%d/%d", pageno + 1, dat->pagenum);
	else
		mIntToStr((char *)m, pageno + 1);

	if(pos == -1 || pos == 1)
		//単ページまたは左側
		x = 4;
	else
	{
		//見開きの右側

		x = dat->st->margin.x1 + dat->st->margin.x2 + dat->areaW * 2 + dat->st->pageSpace
				- 4 - aoFontGetTextWidth8(dat->fontInfo, m, -1);
	}

	aoFontDrawText8(dat->fontInfo, dat->img,
		x, 4, m, -1, dat->st->colInfo);
}

/** 画像読み込み */

static mImageBuf *_load_image(const char *fname)
{
	mLoadImageSource src;
	mLoadImageFunc func;

	src.type = MLOADIMAGE_SOURCE_TYPE_PATH;
	src.filename = fname;

	if(mLoadImage_checkFormat(&src, &func,
		MLOADIMAGE_CHECKFORMAT_F_BMP | MLOADIMAGE_CHECKFORMAT_F_PNG | MLOADIMAGE_CHECKFORMAT_F_JPEG))
	{
		return mImageBufLoadImage(&src, (mDefEmptyFunc)func, 3, NULL);
	}

	return NULL;
}

/** 挿絵描画 */

void layoutDrawPicture(AO_LAYOUT *dat,int page)
{
	mStr str = MSTR_INIT;
	mImageBuf *img;
	int x,y;
	mBox box;

	//ファイル名 (テキストファイルのパスを基点とする)

	mStrSetText(&str, dat->filepath);
	mStrPathAdd(&str, (const char *)(dat->pagestate.picture + 2));
	
	//読み込み

	img = _load_image(str.buf);

	mStrFree(&str);
	
	if(!img) return;

	//位置・サイズ

	box.x = box.y = 0;
	box.w = img->w, box.h = img->h;

	mBoxScaleKeepAspect(&box, dat->areaW, dat->areaH, TRUE);

	x = dat->st->margin.x1 + box.x;
	y = dat->st->margin.y1 + box.y;

	if(page == 0)
		x += dat->st->pageSpace + dat->areaW;

	//描画

	mPixbufScaleImageBuf_oversamp(dat->img,
		x, y, box.w, box.h, img, 5);

	mImageBufFree(img);
}
