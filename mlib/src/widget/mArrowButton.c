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
 * mArrowButton
 *****************************************/

#include "mDef.h"

#include "mArrowButton.h"

#include "mWidget.h"
#include "mPixbuf.h"
#include "mSysCol.h"


/**
@defgroup arrowbutton mArrowButton
@brief 矢印ボタン
@ingroup group_widget

<h3>継承</h3>
mWidget @> mButton @> mArrowButton

@{
@file mArrowButton.h
@struct mArrowButtonData
@struct mArrowButton
@enum MARROWBUTTON_STYLE

@def M_ARROWBUTTON(p)
*/


/** 作成 */

mArrowButton *mArrowButtonNew(int size,mWidget *parent,uint32_t style)
{
	mArrowButton *p;
	
	if(size < sizeof(mArrowButton)) size = sizeof(mArrowButton);
	
	p = (mArrowButton *)mButtonNew(size, parent, 0);
	if(!p) return NULL;
	
	p->wg.draw = mArrowButtonDrawHandle;
	p->wg.calcHint = NULL;
	p->wg.hintW = p->wg.hintH = 17;
	
	p->abtt.style = style;
	
	return p;
}

/** 作成 */

mArrowButton *mArrowButtonCreate(mWidget *parent,int id,uint32_t style,
	uint32_t fLayout,uint32_t marginB4)
{
	mArrowButton *p;

	p = mArrowButtonNew(0, parent, style);
	if(!p) return NULL;

	p->wg.id = id;
	p->wg.fLayout = fLayout;

	mWidgetSetMargin_b4(M_WIDGET(p), marginB4);

	return p;
}

/** 描画 */

void mArrowButtonDrawHandle(mWidget *wg,mPixbuf *pixbuf)
{
	mArrowButton *p = M_ARROWBUTTON(wg);
	uint32_t type,col;
	int x,y;
	mBool press;

	press = mButtonIsPressed(M_BUTTON(p));

	//ボタン
	
	mButtonDrawBase(M_BUTTON(p), pixbuf, press);

	//矢印

	x = wg->w / 2 + press;
	y = wg->h / 2 + press;
	col = (wg->fState & MWIDGET_STATE_ENABLED)? MSYSCOL(TEXT): MSYSCOL(TEXT_DISABLE);

	type = (p->abtt.style & (MARROWBUTTON_S_LEFT | MARROWBUTTON_S_RIGHT | MARROWBUTTON_S_UP));

	switch(type)
	{
		case MARROWBUTTON_S_UP:
			mPixbufDrawArrowUp(pixbuf, x, y, col);
			break;
		case MARROWBUTTON_S_LEFT:
			mPixbufDrawArrowLeft(pixbuf, x, y, col);
			break;
		case MARROWBUTTON_S_RIGHT:
			mPixbufDrawArrowRight(pixbuf, x, y, col);
			break;
		default:
			mPixbufDrawArrowDown(pixbuf, x, y, col);
			break;
	}
}

/** @} */
