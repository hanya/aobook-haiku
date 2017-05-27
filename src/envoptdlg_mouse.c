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
 * 環境設定 - マウス操作
 *****************************************/

#include <string.h>

#include "mDef.h"

#include "mEvent.h"
#include "mTrans.h"
#include "mWidget.h"
#include "mListView.h"
#include "mWidgetBuilder.h"

#include "globaldata.h"
#include "envoptdlg_pv.h"


//--------------------

#define TRID_BTT_TOP  200
#define TRID_CMD_TOP  250

enum
{
	WID_MOUSE_LIST_LEFT = 100,
	WID_MOUSE_LIST_RIGHT
};

//--------------------

static const char *g_wb_envmouse =
"ct#h:lf=wh:sep=8;"
  "lv#,vf:id=100:lf=h;"
  "lv#,vf:id=101:lf=wh;";

//--------------------

typedef struct
{
	uint8_t *buf;
	int selbtt;

	mListView *lvleft,*lvright;
}EnvOptMouseData;

//--------------------


//========================
// sub
//========================


/** リストセット */

static void _set_list(EnvOptMouseData *p)
{
	int i;

	//左

	for(i = 0; i < MOUSECTRL_BTT_NUM; i++)
		mListViewAddItem_textparam(p->lvleft, M_TR_T(TRID_BTT_TOP + i), i);

	mListViewSetWidthAuto(p->lvleft, TRUE);

	//右

	for(i = 0; i < MOUSECTRL_CMD_NUM; i++)
		mListViewAddItem_textparam(p->lvright, M_TR_T(TRID_CMD_TOP + i), i);

	//最初の選択

	mListViewSetFocusItem_index(p->lvleft, 0);
	mListViewSetFocusItem_index(p->lvright, p->buf[0]);
}


/** イベント */

static void _mouse_event(void *data,mEvent *ev)
{
	EnvOptMouseData *p = (EnvOptMouseData *)data;
	int type;

	if(ev->type == MEVENT_NOTIFY)
	{
		type = ev->notify.type;

		switch(ev->notify.widgetFrom->id)
		{
			//ボタン
			case WID_MOUSE_LIST_LEFT:
				if(type == MLISTVIEW_N_CHANGE_FOCUS)
				{
					p->selbtt = ev->notify.param2;
					mListViewSetFocusItem_index(p->lvright, p->buf[p->selbtt]);
				}
				break;
			//コマンド
			case WID_MOUSE_LIST_RIGHT:
				if(type == MLISTVIEW_N_CHANGE_FOCUS)
					p->buf[p->selbtt] = ev->notify.param2;
				break;
		}
	}
}


//========================
// メイン
//========================


/** 作成 */

void EnvOptMouse_create(EnvOptDlgContents *dat,mWidget *parent,uint8_t *buf)
{
	EnvOptMouseData *p;

	//ウィジェット作成

	mWidgetBuilderCreateFromText(parent, g_wb_envmouse);

	//データ

	p = (EnvOptMouseData *)mMalloc(sizeof(EnvOptMouseData), TRUE);
	if(!p) return;

	p->buf = buf;
	p->selbtt = 0;

	p->lvleft = (mListView *)mWidgetFindByID(parent, WID_MOUSE_LIST_LEFT);
	p->lvright = (mListView *)mWidgetFindByID(parent, WID_MOUSE_LIST_RIGHT);

	//リストセット

	_set_list(p);

	//

	dat->data = p;
	dat->destroy = NULL;
	dat->event = _mouse_event;
}

/** 終了時 */

mBool EnvOptMouse_finish(uint8_t *editbuf,mBool bOK)
{
	if(bOK)
		memcpy(GDAT->mousectrl, editbuf, MOUSECTRL_BTT_NUM);

	return FALSE;
}
