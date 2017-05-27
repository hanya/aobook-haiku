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
 * mLabel [ラベル]
 *****************************************/

#include <string.h>

#include "mDef.h"

#include "mLabel.h"
#include "mWidget.h"
#include "mFont.h"
#include "mPixbuf.h"
#include "mSysCol.h"
#include "mUtilStr.h"


//--------------------

#define SPACE_X  3
#define SPACE_Y  2

struct __mLabelRowInfo
{
	int text_top,len,tw;
};

//--------------------


/**
@defgroup label mLabel
@brief ラベル

'\\n' で改行して複数行可能。

@ingroup group_widget
 
@{

@file mLabel.h
@def M_LABEL(p)
@struct mLabelData
@struct mLabel
@enum MLABEL_STYLE
*/


//==========================


/** テキスト初期化処理 */

static int _init_text(mLabel *p)
{
	char *pc,*pc2,*pcnext;
	int len,row;
	
	if(!p->lb.text) return 0;
	
	//行数
	
	p->lb.rows = mGetStrLines(p->lb.text);
	
	p->lb.row_info = (struct __mLabelRowInfo *)mMalloc(
		sizeof(struct __mLabelRowInfo) * p->lb.rows, TRUE);
	
	if(!p->lb.row_info) return -1;
	
	//各行処理
	
	for(pc = p->lb.text, row = 0; *pc; pc = pcnext, row++)
	{
		pc2 = strchr(pc, '\n');
		
		if(pc2)
		{
			pcnext = pc2 + 1;
			len = pc2 - pc;
		}
		else
		{
			pcnext = p->lb.text + strlen(p->lb.text);
			len = pcnext - pc;
		}
		
		//改行で終わっている場合は除く
		
		if(len == 0 && !pc2) break;
		
		//
		
		p->lb.row_info[row].text_top = pc - p->lb.text;
		p->lb.row_info[row].len = len;
	}

	return 0;
}

/** テキストサイズ計算 */

static void _calc_text_size(mLabel *p)
{
	mFont *font;
	int i,w,maxw = 0;
	
	font = mWidgetGetFont(M_WIDGET(p));

	if(!p->lb.text || !p->lb.row_info)
		p->lb.textH = font->height;
	else
	{
		for(i = 0; i < p->lb.rows; i++)
		{
			w = mFontGetTextWidth(font,
				p->lb.text + p->lb.row_info[i].text_top, p->lb.row_info[i].len);
			
			p->lb.row_info[i].tw = w;
			
			if(w > maxw) maxw = w;
		}
		
		p->lb.textW = maxw;
		p->lb.textH = font->height + font->lineheight * (p->lb.rows - 1);
	}
}


//==========================


/** 解放処理 */

void mLabelDestroyHandle(mWidget *p)
{
	mLabel *label = M_LABEL(p);

	M_FREE_NULL(label->lb.text);
	M_FREE_NULL(label->lb.row_info);
}

/** サイズ計算 */

void mLabelCalcHintHandle(mWidget *p)
{
	_calc_text_size(M_LABEL(p));
	
	p->hintW = M_LABEL(p)->lb.textW;
	p->hintH = M_LABEL(p)->lb.textH;
	
	if(M_LABEL(p)->lb.style & MLABEL_S_BORDER)
	{
		p->hintW += SPACE_X * 2;
		p->hintH += SPACE_Y * 2;
	}
}


//==========================


/** ラベル作成 */

mLabel *mLabelCreate(mWidget *parent,uint32_t style,
	uint32_t fLayout,uint32_t marginB4,const char *text)
{
	mLabel *p;

	p = mLabelNew(0, parent, style);
	if(!p) return NULL;

	p->wg.fLayout = fLayout;

	mWidgetSetMargin_b4(M_WIDGET(p), marginB4);

	mLabelSetText(p, text);

	return p;
}


/** ラベル作成 */

mLabel *mLabelNew(int size,mWidget *parent,uint32_t style)
{
	mLabel *p;
	
	if(size < sizeof(mLabel)) size = sizeof(mLabel);
	
	p = (mLabel *)mWidgetNew(size, parent);
	if(!p) return NULL;
	
	p->wg.destroy = mLabelDestroyHandle;
	p->wg.calcHint = mLabelCalcHintHandle;
	p->wg.draw = mLabelDrawHandle;

	p->lb.style = style;
	
	return p;
}

/** テキストセット */

void mLabelSetText(mLabel *p,const char *text)
{
	mLabelDestroyHandle(M_WIDGET(p));

	if(text)
	{
		p->lb.text = mStrdup(text);
		
		_init_text(p);
	}

	if(p->wg.fUI & MWIDGET_UI_LAYOUTED)
		mLabelCalcHintHandle(M_WIDGET(p));
	
	mWidgetUpdate(M_WIDGET(p));
}

/** int 値からテキストセット */

void mLabelSetText_int(mLabel *p,int val)
{
	char m[32];

	mIntToStr(m, val);
	mLabelSetText(p, m);
}

/** int 値から浮動小数点数テキストセット */

void mLabelSetText_floatint(mLabel *p,int val,int dig)
{
	char m[32];

	mFloatIntToStr(m, val, dig);
	mLabelSetText(p, m);
}

/** 描画処理 */

void mLabelDrawHandle(mWidget *p,mPixbuf *pixbuf)
{
	mLabel *label = M_LABEL(p);
	mFont *font;
	int spx,spy,tx,ty,i;
	struct __mLabelRowInfo *info;

	font = mWidgetGetFont(p);

	//余白
	
	if(label->lb.style & MLABEL_S_BORDER)
		spx = SPACE_X, spy = SPACE_Y;
	else
		spx = spy = 0;
	
	//テキスト Y 位置
	
	if(label->lb.style & MLABEL_S_BOTTOM)
		ty = p->h - spy - label->lb.textH;
	else if(label->lb.style & MLABEL_S_MIDDLE)
		ty = spy + (p->h - spy * 2 - label->lb.textH) / 2;
	else
		ty = spy;
		
	//背景
	
	mWidgetDrawBkgnd(p, NULL);
	
	//テキスト
	
	info = label->lb.row_info;
	
	if(label->lb.text && info)
	{
		for(i = 0; i < label->lb.rows; i++, info++)
		{
			if(label->lb.style & MLABEL_S_RIGHT)
				tx = p->w - spx - info->tw;
			else if(label->lb.style & MLABEL_S_CENTER)
				tx = spx + (p->w - spx * 2 - info->tw) / 2;
			else
				tx = spx;
			
			mFontDrawText(font, pixbuf, tx, ty,
				label->lb.text + info->text_top, info->len, MSYSCOL_RGB(TEXT));
			
			ty += font->lineheight;
		}
	}
	
	//枠
	
	if(label->lb.style & MLABEL_S_BORDER)
		mPixbufBox(pixbuf, 0, 0, p->w, p->h, MSYSCOL(FRAME));
}


/** @} */
