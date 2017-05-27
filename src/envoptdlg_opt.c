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
 * 環境設定 - 設定
 *****************************************/

#include "mDef.h"

#include "mStr.h"
#include "mWidget.h"
#include "mFontButton.h"
#include "mCheckButton.h"
#include "mWidgetBuilder.h"

#include "envoptdlg_pv.h"


//--------------------

enum
{
	WID_GUIFONT = 100,
	WID_CK_FLAGS = 200
};

//--------------------

static const char *g_wb_envopt =
"ct#v:lf=wh:sep=3;"
  "ct#h:lf=w:sep=4:mg=b6;"
    "lb:tr=400:lf=m;"
    "fontbt:id=100:lf=w;"
  "-;"
  "ck:id=200:tr=401;"
  "ck:id=201:tr=402;";

//--------------------

typedef struct
{
	EnvOptDlgEditData *edit;

	mFontButton *fontbtt;
	mCheckButton *ckflags[2];
}EnvOptOptData;

//--------------------


//========================
// メイン
//========================


/** 破棄時 */

static void _opt_destroy(void *data)
{
	EnvOptOptData *p = (EnvOptOptData *)data;
	int i;

	mFontButtonGetInfoFormat_str(p->fontbtt, &p->edit->strGUIFont);

	//フラグ

	p->edit->optflags = 0;

	for(i = 0; i < 2; i++)
	{
		if(mCheckButtonIsChecked(p->ckflags[i]))
			p->edit->optflags |= 1 << i;
	}
}

/** 作成 */

void EnvOptOpt_create(EnvOptDlgContents *dat,mWidget *parent,EnvOptDlgEditData *edit)
{
	EnvOptOptData *p;
	int i;

	//ウィジェット作成

	mWidgetBuilderCreateFromText(parent, g_wb_envopt);

	//データ

	p = (EnvOptOptData *)mMalloc(sizeof(EnvOptOptData), TRUE);
	if(!p) return;

	p->edit = edit;

	//ウィジェット

	p->fontbtt = (mFontButton *)mWidgetFindByID(parent, WID_GUIFONT);

	mFontButtonSetInfoFormat(p->fontbtt, edit->strGUIFont.buf);

	for(i = 0; i < 2; i++)
	{
		p->ckflags[i] = (mCheckButton *)mWidgetFindByID(parent, WID_CK_FLAGS + i);

		mCheckButtonSetState(p->ckflags[i], edit->optflags & (1 << i));
	}

	//

	dat->data = p;
	dat->destroy = _opt_destroy;
	dat->event = NULL;
}

/** 終了時 */

void EnvOptOpt_finish(EnvOptDlgEditData *edit,mBool bOK)
{
	if(bOK)
	{
		mStrCopy(&GDAT->strGUIFont, &edit->strGUIFont);

		GDAT->optflags = edit->optflags;
	}

	mStrFree(&edit->strGUIFont);
}
