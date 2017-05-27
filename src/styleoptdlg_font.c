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
 * スタイル設定 - フォント
 *****************************************/

#include "mDef.h"
#include "mContainerDef.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mLabel.h"
#include "mColorButton.h"
#include "mFontButton.h"
#include "mTrans.h"

#include "style.h"

#include "styleoptdlg_pv.h"


//--------------------

typedef struct
{
	mFontButton *fbtt[4];
	mColorButton *colbt[3];
}StyleOptFontDat;

//--------------------


/** データ取得 */

static void _font_getdata(void *data,StyleData *s)
{
	StyleOptFontDat *p = (StyleOptFontDat *)data;

	mFontButtonGetInfoFormat_str(p->fbtt[0], &s->strFontText);
	mFontButtonGetInfoFormat_str(p->fbtt[1], &s->strFontBold);
	mFontButtonGetInfoFormat_str(p->fbtt[2], &s->strFontRuby);
	mFontButtonGetInfoFormat_str(p->fbtt[3], &s->strFontInfo);

	s->b.colText = mColorButtonGetColor(p->colbt[0]);
	s->b.colRuby = mColorButtonGetColor(p->colbt[1]);
	s->b.colInfo = mColorButtonGetColor(p->colbt[2]);
}

/** データセット */

static void _font_setdata(void *data,StyleData *s)
{
	StyleOptFontDat *p = (StyleOptFontDat *)data;
	
	mFontButtonSetInfoFormat(p->fbtt[0], s->strFontText.buf);
	mFontButtonSetInfoFormat(p->fbtt[1], s->strFontBold.buf);
	mFontButtonSetInfoFormat(p->fbtt[2], s->strFontRuby.buf);
	mFontButtonSetInfoFormat(p->fbtt[3], s->strFontInfo.buf);

	mColorButtonSetColor(p->colbt[0], s->b.colText);
	mColorButtonSetColor(p->colbt[1], s->b.colRuby);
	mColorButtonSetColor(p->colbt[2], s->b.colInfo);
}

/** 作成 */

void StyleOptFont_create(StyleOptDlgContents *cdat,mWidget *parent)
{
	StyleOptFontDat *p;
	int i;
	mWidget *ct;

	//確保

	p = (StyleOptFontDat *)mMalloc(sizeof(StyleOptFontDat), TRUE);
	if(!p) return;

	cdat->data = p;
	cdat->setdata = _font_setdata;
	cdat->getdata = _font_getdata;

	//ウィジェット作成

	ct = mContainerCreate(parent, MCONTAINER_TYPE_GRID, 3, 0, MLF_EXPAND_W);

	M_CONTAINER(ct)->ct.gridSepCol = 6;
	M_CONTAINER(ct)->ct.gridSepRow = 7;

	for(i = 0; i < 4; i++)
	{
		//ラベル

		mLabelCreate(ct, 0, MLF_RIGHT | MLF_MIDDLE, 0, M_TR_T(1100 + i));

		//フォントボタン

		p->fbtt[i] = mFontButtonCreate(ct, 0, 0, MLF_EXPAND_W | MLF_MIDDLE, 0);

		//色ボタン

		if(i == 1)
			mWidgetNew(0, ct);
		else
			p->colbt[i - (i > 1)] = mColorButtonNew(0, ct, MCOLORBUTTON_S_DIALOG);
	}
}
