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
 * 環境設定 - キー設定
 *****************************************/


#include "mDef.h"

#include "mStr.h"
#include "mTrans.h"
#include "mEvent.h"
#include "mAccelerator.h"
#include "mWidget.h"
#include "mLabel.h"
#include "mListView.h"
#include "mInputAccelKey.h"
#include "mWidgetBuilder.h"

#include "trgroup.h"
#include "trid_menu.h"
#include "globaldata.h"
#include "envoptdlg_pv.h"

#define MENUDAT_EXTERN
#include "menudat.h"


//--------------------

enum
{
	WID_KEY_LIST = 100,
	WID_KEY_LABEL_CMD,
	WID_KEY_ACCELKEY,
	WID_KEY_CLEAR,
	WID_KEY_ALLCLEAR
};

//--------------------

static const char *g_wb_envkey =
"ct#v:lf=wh:sep=8;"
  "lv#R,vf:id=100:lf=wh;"
  "ct#g2:lf=w:sep=6,5;"
    "lb:tr=100:lf=rm;"
    "lb#B:id=101:lf=wm;"
    "lb:tr=101:lf=rm;"
    "ct#h:lf=w:sep=5;"
      "iak:id=102:lf=wm;"
      "bt:id=103:tr=102:lf=m;"
  "--;"
  "bt:id=104:tr=103:lf=rx;";

//--------------------

typedef struct
{
	uint32_t **ppbuf;

	mListView *lv;
	mLabel *labelCmd;
	mInputAccelKey *ackey;
}EnvOptKeyData;

//--------------------


//========================
// sub
//========================


/** リスト表示テキスト取得 */

static void _get_listtext(mStr *str,uint32_t key)
{
	char *keystr;

	//コマンド名

	mStrSetText(str, M_TR_T(key & 0xffff));

	//キーテキスト

	key >>= 16;

	if(key)
	{
		mStrAppendText(str, " = ");

		keystr = mAcceleratorGetKeyText(key);
		mStrAppendText(str, keystr);
		mFree(keystr);
	}
}

/** リストデータセット */

static void _set_list(EnvOptKeyData *p,uint32_t *keybuf)
{
	mStr str = MSTR_INIT;
	const uint16_t *pmenu;
	uint32_t *pkey,id,key;

	M_TR_G(TRGROUP_MENU);

	for(pmenu = g_menudat; *pmenu != 0xffff; pmenu++)
	{
		id = *pmenu;
		
		if(id < 1000 || id >= 0xfffd
			|| id == TRMENU_FILE_RECENTFILE || id == TRMENU_OPT_TOOL_SUB
			|| id == TRMENU_OPT_STYLE_SUB || id == TRMENU_HELP_ABOUT)
			continue;

		id &= 0x7fff;

		//IDからキー検索

		for(pkey = keybuf, key = 0; *pkey; pkey++)
		{
			if((*pkey & 0xffff) == id)
			{
				key = *pkey;
				break;
			}
		}

		if(key == 0) key = id;

		//テキスト

		_get_listtext(&str, key);

		//追加

		mListViewAddItem_textparam(p->lv, str.buf, key);
	}

	mStrFree(&str);
}


//========================
// イベント
//========================


/** リスト選択変更時 */

static void _change_sel(EnvOptKeyData *p,mListViewItem *pi)
{
	uint32_t key = pi->param;

	//コマンド名

	mLabelSetText(p->labelCmd, M_TR_T(key & 0xffff));

	//キー

	mInputAccelKey_setKey(p->ackey, key >> 16);

	//フォーカス移動

	mWidgetSetFocus(M_WIDGET(p->ackey));
}

/** キー変更時 */

static void _change_key(EnvOptKeyData *p,uint32_t key)
{
	mListViewItem *pi;
	mStr str = MSTR_INIT;

	pi = mListViewGetFocusItem(p->lv);
	if(!pi) return;

	pi->param &= 0xffff;
	pi->param |= key << 16;

	_get_listtext(&str, pi->param);

	mListViewSetItemText(p->lv, pi, str.buf);

	mStrFree(&str);
}

/** すべてクリア */

static void _clear_all(EnvOptKeyData *p)
{
	mListViewItem *pi;

	for(pi = mListViewGetTopItem(p->lv); pi; pi = M_LISTVIEWITEM(pi->i.next))
	{
		pi->param &= 0xffff;

		mListViewSetItemText(p->lv, pi, M_TR_T(pi->param));
	}

	mInputAccelKey_setKey(p->ackey, 0);
}

/** イベントハンドラ */

static void _key_event(void *data,mEvent *ev)
{
	EnvOptKeyData *p = (EnvOptKeyData *)data;
	int type;

	if(ev->type == MEVENT_NOTIFY)
	{
		type = ev->notify.type;
	
		switch(ev->notify.widgetFrom->id)
		{
			//リスト
			case WID_KEY_LIST:
				if(type == MLISTVIEW_N_CHANGE_FOCUS)
					_change_sel(p, (mListViewItem *)ev->notify.param1);
				break;
			//キー
			case WID_KEY_ACCELKEY:
				if(type == MINPUTACCELKEY_N_CHANGEKEY)
					_change_key(p, ev->notify.param1);
				break;
			//クリア
			case WID_KEY_CLEAR:
				mInputAccelKey_setKey(p->ackey, 0);
				_change_key(p, 0);
				break;
			//全てクリア
			case WID_KEY_ALLCLEAR:
				_clear_all(p);
				break;
		}
	}
}


//========================
// メイン
//========================


/** 編集中用データ作成 */

uint32_t *EnvOptKey_createEditData()
{
	uint32_t *ps;
	int num;

	//個数

	for(ps = GDAT->shortcutkey, num = 0; *ps; ps++, num++);

	//コピー

	return mMemdup(GDAT->shortcutkey, sizeof(uint32_t) * (num + 1));
}

/** 破棄時 (編集用データに退避) */

static void _key_destroy(void *data)
{
	EnvOptKeyData *p = (EnvOptKeyData *)data;
	mListViewItem *pi;
	int num;
	uint32_t *pd;

	//データ個数取得

	for(pi = mListViewGetTopItem(p->lv), num = 0; pi; pi = M_LISTVIEWITEM(pi->i.next))
	{
		if(pi->param >> 16) num++;
	}

	//確保

	mFree(*(p->ppbuf));

	*(p->ppbuf) = mMalloc(sizeof(uint32_t) * (num + 1), FALSE);

	//リストからキーデータ作成

	pd = *(p->ppbuf);

	for(pi = mListViewGetTopItem(p->lv); pi; pi = M_LISTVIEWITEM(pi->i.next))
	{
		if(pi->param >> 16)
			*(pd++) = pi->param;
	}

	*pd = 0;
}

/** 作成 */

void EnvOptKey_create(EnvOptDlgContents *dat,mWidget *parent,uint32_t **ppbuf)
{
	EnvOptKeyData *p;

	//ウィジェット作成

	mWidgetBuilderCreateFromText(parent, g_wb_envkey);

	//データ

	p = (EnvOptKeyData *)mMalloc(sizeof(EnvOptKeyData), TRUE);
	if(!p) return;

	p->ppbuf = ppbuf;

	p->lv = (mListView *)mWidgetFindByID(parent, WID_KEY_LIST);
	p->labelCmd = (mLabel *)mWidgetFindByID(parent, WID_KEY_LABEL_CMD);
	p->ackey = (mInputAccelKey *)mWidgetFindByID(parent, WID_KEY_ACCELKEY);

	//リストデータセット

	_set_list(p, *ppbuf);

	//

	dat->data = p;
	dat->destroy = _key_destroy;
	dat->event = _key_event;
}

/** 終了時
 *
 * データ変更時、新しいデータを ppnewbuf にセット */

mBool EnvOptKey_finish(uint32_t *editbuf,uint32_t **ppnewbuf,mBool bOK)
{
	uint32_t *p1,*p2;

	//キャンセル時

	if(!bOK)
	{
		mFree(editbuf);
		return FALSE;
	}

	//データを比較

	p1 = GDAT->shortcutkey;
	p2 = editbuf;

	for(; *p1 && *p2 && *p1 == *p2; p1++, p2++);

	//

	if(*p1 == 0 && *p2 == 0)
	{
		//データが同じなので変更なし

		mFree(editbuf);
		return FALSE;
	}
	else
	{
		//変更あり

		*ppnewbuf = editbuf;
		return TRUE;
	}
}

