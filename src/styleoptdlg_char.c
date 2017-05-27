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
 * スタイル設定 - 文字
 *****************************************/

#include "mDef.h"

#include "mContainerDef.h"
#include "mWidget.h"
#include "mContainer.h"
#include "mLabel.h"
#include "mLineEdit.h"
#include "mTrans.h"
#include "mStr.h"

#include "style.h"

#include "styleoptdlg_pv.h"


//--------------------

typedef struct
{
	mLineEdit *edit[5];
}StyleOptCharDat;

//--------------------


/** UCS-4 文字列取得 */

static void _set_str(uint32_t **buf,mLineEdit *le)
{
	mStr str = MSTR_INIT;

	mLineEditGetTextStr(le, &str);

	StyleSetUTF32Text(buf, str.buf);

	mStrFree(&str);
}

/** データ取得 */

static void _char_getdata(void *data,StyleData *s)
{
	StyleOptCharDat *p = (StyleOptCharDat *)data;

	_set_str(&s->b.strNoHead, p->edit[0]);
	_set_str(&s->b.strNoBottom, p->edit[1]);
	_set_str(&s->b.strHanging, p->edit[2]);
	_set_str(&s->b.strNoSep, p->edit[3]);
	_set_str(&s->b.strReplace, p->edit[4]);
}

/** データセット */

static void _char_setdata(void *data,StyleData *s)
{
	StyleOptCharDat *p = (StyleOptCharDat *)data;

	mLineEditSetText_ucs4(p->edit[0], s->b.strNoHead);
	mLineEditSetText_ucs4(p->edit[1], s->b.strNoBottom);
	mLineEditSetText_ucs4(p->edit[2], s->b.strHanging);
	mLineEditSetText_ucs4(p->edit[3], s->b.strNoSep);
	mLineEditSetText_ucs4(p->edit[4], s->b.strReplace);
}

/** 作成 */

void StyleOptChar_create(StyleOptDlgContents *cdat,mWidget *parent)
{
	StyleOptCharDat *p;
	int i;
	mWidget *ct;

	//確保

	p = (StyleOptCharDat *)mMalloc(sizeof(StyleOptCharDat), TRUE);
	if(!p) return;

	cdat->data = p;
	cdat->setdata = _char_setdata;
	cdat->getdata = _char_getdata;

	//ウィジェット作成

	ct = mContainerCreate(parent, MCONTAINER_TYPE_GRID, 2, 0, MLF_EXPAND_W);

	M_CONTAINER(ct)->ct.gridSepCol = 6;
	M_CONTAINER(ct)->ct.gridSepRow = 8;

	for(i = 0; i < 5; i++)
	{
		//ラベル

		mLabelCreate(ct, 0, MLF_RIGHT | MLF_MIDDLE, 0, M_TR_T(1200 + i));

		//エディット

		p->edit[i] = mLineEditCreate(ct, 0, 0, MLF_EXPAND_W, 0);
	}

	mWidgetNew(0, ct);
	mLabelCreate(ct, MLABEL_S_BORDER, MLF_EXPAND_W, 0, M_TR_T(1205));
}
