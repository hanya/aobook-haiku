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
 * スタイル設定 - 基本
 *****************************************/

#include "mDef.h"

#include "mWidget.h"
#include "mCheckButton.h"
#include "mLineEdit.h"
#include "mColorButton.h"
#include "mComboBox.h"
#include "mEvent.h"
#include "mWidgetBuilder.h"
#include "mTrans.h"
#include "mStr.h"
#include "mSysDialog.h"

#include "style.h"

#include "styleoptdlg_pv.h"


//--------------------

enum
{
	WID_PAGE_TOP = 100,
	WID_EDIT_LAYOUT_TOP = 102,
	WID_EDIT_SPACE_TOP = 106,

	WID_BKGND_COLOR = 200,
	WID_BKGND_IMG,
	WID_BKGND_BTT,
	WID_BKGND_TYPE,

	WID_FLAGS_TOP = 300
};

//--------------------

static const char *g_wb_style_basic =
"ct#v:sep=3:pd=6,0:lf=w;"
  "ct#h:sep=15;"
    "ct#v:sep=6;"
      /* ページ数 */
      "ct#h:sep=6;"
        "ck#r:id=100:tr=1000;"
        "ck#r:id=101:tr=1001;"
      "-;"
      /* 字数など */
      "ct#g4:sep=5,5:mg=t3;"
        "lb:tr=1002:lf=m;"
        "le#s:id=102:wlen=4;"
        "lb:tr=1003:lf=m:mg=l4;"
        "le#s:id=103:wlen=4;"
        "lb:tr=1004:lf=m;"
        "le#s:id=104:wlen=4;"
        "lb:tr=1005:lf=m:mg=l4;"
        "le#s:id=105:wlen=4;"
    "--;"
    /* 余白 */
    "gb:cts=g4:sep=5,5:tr=1006;"
      "lb:tr=1007:lf=mr;"
      "le#s:id=106:wlen=4;"
      "lb:tr=1008:lf=mr:mg=l4;"
      "le#s:id=107:wlen=4;"
      "lb:tr=1009:lf=mr;"
      "le#s:id=108:wlen=4;"
      "lb:tr=1010:lf=mr:mg=l4;"
      "le#s:id=109:wlen=4;"
      "lb:tr=1011:lf=mr;"
      "le#s:id=110:wlen=4;"
  "--;"
  /* 背景 */
  "gb:cts=g2:sep=5,5:tr=1012:lf=w:mg=0,7,0,5;"
    "lb:tr=1013:lf=mr;"
    "colbt#d:id=200:lf=m;"
    "lb:tr=1014:lf=r;"
    "ct#v:sep=5:lf=w;"
      "ct#h:sep=6:lf=w;"
        "le:id=201:lf=w;"
        "bt#wh:id=202:lf=m:t=...;"
      "-;"
      "cb:id=203;"
  "--;"
  /* フラグ */
  "ct#g2:sep=8,4;"
    "ck:id=300:tr=1015;"
    "ck:id=301:tr=1016;"
    "ck:id=302:tr=1017;"
    "ck:id=303:tr=1018;";

//--------------------

typedef struct
{
	mCheckButton *ckpage[2],
		*ckflags[4];
	mLineEdit *editLayout[4],
		*editSpace[5],
		*editBkgndFile;

	mColorButton *colbtBkgnd;
	mComboBox *cbBkgndType;
}StyleOptBasicDat;

//--------------------


//========================
// イベント
//========================


/** 背景画像選択 */

static void _sel_bkgnd_file(mWindow *owner,StyleOptBasicDat *p)
{
	mStr str = MSTR_INIT,dir = MSTR_INIT;

	mLineEditGetTextStr(p->editBkgndFile, &dir);
	mStrPathRemoveFileName(&dir);

	if(mSysDlgOpenFile(owner,
		"BMP/PNG/JPEG\t*.bmp;*.png;*.jpg;*.jpeg\tAll files\t*", 0,
		dir.buf, 0, &str))
	{
		mLineEditSetText(p->editBkgndFile, str.buf);
	}

	mStrFree(&str);
	mStrFree(&dir);
}

/** イベント */

static void _basic_event(void *data,mEvent *ev)
{
	if(ev->type == MEVENT_NOTIFY
		&& ev->notify.id == WID_BKGND_BTT)
	{
		_sel_bkgnd_file(M_WINDOW(ev->widget->toplevel), (StyleOptBasicDat *)data);
	}
}


//========================
// メイン
//========================


/** データ取得 */

static void _basic_getdata(void *data,StyleData *s)
{
	StyleOptBasicDat *p = (StyleOptBasicDat *)data;
	int i;

	//レイアウト

	s->b.pages = (mCheckButtonIsChecked(p->ckpage[1]))? 2: 1;

	s->b.chars = mLineEditGetNum(p->editLayout[0]);
	s->b.lines = mLineEditGetNum(p->editLayout[1]);
	s->b.charSpace = mLineEditGetNum(p->editLayout[2]);
	s->b.lineSpace = mLineEditGetNum(p->editLayout[3]);

	//余白

	s->b.margin.x1 = mLineEditGetNum(p->editSpace[0]);
	s->b.margin.x2 = mLineEditGetNum(p->editSpace[1]);
	s->b.margin.y1 = mLineEditGetNum(p->editSpace[2]);
	s->b.margin.y2 = mLineEditGetNum(p->editSpace[3]);
	s->b.pageSpace = mLineEditGetNum(p->editSpace[4]);

	//背景

	s->colBkgnd = mColorButtonGetColor(p->colbtBkgnd);
	mLineEditGetTextStr(p->editBkgndFile, &s->strBkgndFile);
	s->bkgnd_imgtype = mComboBoxGetSelItemIndex(p->cbBkgndType);

	//フラグ

	s->b.flags = 0;

	for(i = 0; i < 4; i++)
	{
		if(mCheckButtonIsChecked(p->ckflags[i]))
			s->b.flags |= 1 << i;
	}
}

/** データセット */

static void _basic_setdata(void *data,StyleData *s)
{
	StyleOptBasicDat *p = (StyleOptBasicDat *)data;
	int i;

	//レイアウト

	mCheckButtonSetState(p->ckpage[(s->b.pages == 2)], 1);

	mLineEditSetNum(p->editLayout[0], s->b.chars);
	mLineEditSetNum(p->editLayout[1], s->b.lines);
	mLineEditSetNum(p->editLayout[2], s->b.charSpace);
	mLineEditSetNum(p->editLayout[3], s->b.lineSpace);

	//余白

	mLineEditSetNum(p->editSpace[0], s->b.margin.x1);
	mLineEditSetNum(p->editSpace[1], s->b.margin.x2);
	mLineEditSetNum(p->editSpace[2], s->b.margin.y1);
	mLineEditSetNum(p->editSpace[3], s->b.margin.y2);
	mLineEditSetNum(p->editSpace[4], s->b.pageSpace);

	//背景

	mColorButtonSetColor(p->colbtBkgnd, s->colBkgnd);
	mLineEditSetText(p->editBkgndFile, s->strBkgndFile.buf);
	mComboBoxSetSel_index(p->cbBkgndType, s->bkgnd_imgtype);

	//フラグ

	for(i = 0; i < 4; i++)
		mCheckButtonSetState(p->ckflags[i], ((s->b.flags & (1 << i)) != 0));
}

/** 作成 */

void StyleOptBasic_create(StyleOptDlgContents *cdat,mWidget *parent)
{
	StyleOptBasicDat *p;
	int i;

	//ウィジェット作成

	mWidgetBuilderCreateFromText(parent, g_wb_style_basic);

	//確保

	p = (StyleOptBasicDat *)mMalloc(sizeof(StyleOptBasicDat), TRUE);
	if(!p) return;

	cdat->data = p;
	cdat->setdata = _basic_setdata;
	cdat->getdata = _basic_getdata;
	cdat->event = _basic_event;

	//------ ウィジェット取得・初期化

	//レイアウト

	for(i = 0; i < 2; i++)
		p->ckpage[i] = (mCheckButton *)mWidgetFindByID(parent, WID_PAGE_TOP + i);

	for(i = 0; i < 4; i++)
	{
		p->editLayout[i] = (mLineEdit *)mWidgetFindByID(parent, WID_EDIT_LAYOUT_TOP + i);

		mLineEditSetNumStatus(p->editLayout[i], (i < 2)? 1: -100, 100, 0);
	}

	//余白

	for(i = 0; i < 5; i++)
	{
		p->editSpace[i] = (mLineEdit *)mWidgetFindByID(parent, WID_EDIT_SPACE_TOP + i);

		mLineEditSetNumStatus(p->editSpace[i], 0, 1000, 0);
	}

	//背景

	p->colbtBkgnd = (mColorButton *)mWidgetFindByID(parent, WID_BKGND_COLOR);
	p->editBkgndFile = (mLineEdit *)mWidgetFindByID(parent, WID_BKGND_IMG);
	p->cbBkgndType = (mComboBox *)mWidgetFindByID(parent, WID_BKGND_TYPE);

	for(i = 0; i < 2; i++)
		mComboBoxAddItem(p->cbBkgndType, M_TR_T(2000 + i), 0);

	mComboBoxSetWidthAuto(p->cbBkgndType);

	//フラグ

	for(i = 0; i < 4; i++)
		p->ckflags[i] = (mCheckButton *)mWidgetFindByID(parent, WID_FLAGS_TOP + i);
}
