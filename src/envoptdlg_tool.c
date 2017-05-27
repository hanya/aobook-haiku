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
 * 環境設定 - ツール
 *****************************************/

#include "mDef.h"
#include "mStr.h"
#include "mEvent.h"
#include "mTrans.h"
#include "mWidget.h"
#include "mLineEdit.h"
#include "mInputAccelKey.h"
#include "mListView.h"
#include "mWidgetBuilder.h"

#include "globaldata.h"
#include "envoptdlg_pv.h"


//--------------------

enum
{
	WID_TOOL_LIST = 100,
	WID_TOOL_ADD,
	WID_TOOL_DEL,
	WID_TOOL_EDIT_NAME,
	WID_TOOL_EDIT_CMD,
	WID_TOOL_ACCKEY,
	WID_TOOL_KEY_CLEAR,
	WID_TOOL_REPLACE
};

//--------------------

static const char *g_wb_envtool =
"ct#v:lf=wh:sep=5:pd=b4;"
  "ct#h:lf=wh:sep=3:mg=b2;"
    "lv#s,vf:id=100:lf=wh;"
    "ct#v:lf=h:sep=2;"
      "bt:id=101:tr=300;"
      "bt:id=102:tr=301;"
      "bt:lf=yb:id=107:tr=307;"
  "--;"
  "ct#g2:lf=w:sep=6,5;"
    "lb:lf=rm:tr=302;"
    "le:lf=wm:id=103;"
    "lb:lf=rm:tr=303;"
    "le:lf=wm:id=104;"
    "lb:lf=rm:tr=304;"
    "ct#h:lf=w:sep=5;"
      "iak:lf=wm:id=105;"
      "bt:lf=m:id=106:tr=305;"
  "--;"
  "lb#B:lf=w:tr=306;";

//--------------------

typedef struct
{
	mStr *strarray;
	int itemnum;
	mListViewItem *selitem;

	mListView *lv;
	mLineEdit *editName,*editCmd;
	mInputAccelKey *ackey;
}EnvOptToolData;

//--------------------


//========================
// sub
//========================


/** リストセット */

static void _set_list(EnvOptToolData *p)
{
	mStr str = MSTR_INIT;
	int i;

	mListViewDeleteAllItem(p->lv);

	for(i = 0; i < TOOLITEM_NUM; i++)
	{
		if(mStrIsEmpty(p->strarray + i)) break;

		mStrGetSplitText(&str, p->strarray[i].buf, '\t', 0);

		mListViewAddItem_textparam(p->lv, str.buf, i);
	}

	mStrFree(&str);
}

/** 選択変更時 */

static void _change_sel(EnvOptToolData *p)
{
	mStr str = MSTR_INIT;
	char *psrc;

	if(!p->selitem) return;

	psrc = p->strarray[p->selitem->param].buf;

	//表示名

	mStrGetSplitText(&str, psrc, '\t', 0);
	mLineEditSetText(p->editName, str.buf);

	//コマンド

	mStrGetSplitText(&str, psrc, '\t', 1);
	mLineEditSetText(p->editCmd, str.buf);

	//ショートカットキー

	mStrGetSplitText(&str, psrc, '\t', 2);
	mInputAccelKey_setKey(p->ackey, mStrToInt(&str));

	mStrFree(&str);
}


//========================
// イベント
//========================


/** 追加 */

static void _cmd_add(EnvOptToolData *p)
{
	if(p->itemnum < 10)
	{
		int no;

		no = p->itemnum;

		//データ追加
		
		mStrSetText(p->strarray + no, "New");
		p->itemnum++;

		//リスト追加

		p->selitem = mListViewAddItem_textparam(p->lv, "New", no);

		//選択

		mListViewSetFocusItem(p->lv, p->selitem);

		_change_sel(p);

		//フォーカス

		mWidgetSetFocus(M_WIDGET(p->editName));
	}
}

/** 削除 */

static void _cmd_del(EnvOptToolData *p)
{
	int no;

	if(!p->selitem) return;

	no = p->selitem->param;

	//データ削除 (詰める)

	mStrArrayShiftUp(p->strarray, no, TOOLITEM_NUM - 1);

	p->itemnum--;

	//リスト再セット

	_set_list(p);

	//選択

	if(no >= p->itemnum) no = p->itemnum - 1;

	p->selitem = mListViewSetFocusItem_index(p->lv, no);

	_change_sel(p);
}

/** 置換 */

static void _cmd_replace(EnvOptToolData *p)
{
	mStr *pdst,str = MSTR_INIT;

	if(!p->selitem) return;

	pdst = p->strarray + p->selitem->param;

	//表示名 [!]空文字列にならないようにする

	mLineEditGetTextStr(p->editName, pdst);

	if(mStrIsEmpty(pdst))
		mStrSetText(pdst, "empty");

	//リスト名変更

	mListViewSetItemText(p->lv, p->selitem, pdst->buf);

	//コマンド

	mLineEditGetTextStr(p->editCmd, &str);

	mStrAppendChar(pdst, '\t');
	mStrAppendText(pdst, str.buf);

	//ショートカットキー

	mStrAppendChar(pdst, '\t');
	mStrAppendInt(pdst, mInputAccelKey_getKey(p->ackey));

	mStrFree(&str);
}

/** イベント */

static void _tool_event(void *data,mEvent *ev)
{
	EnvOptToolData *p = (EnvOptToolData *)data;
	int type;

	if(ev->type == MEVENT_NOTIFY)
	{
		type = ev->notify.type;

		switch(ev->notify.widgetFrom->id)
		{
			//リスト
			case WID_TOOL_LIST:
				if(type == MLISTVIEW_N_CHANGE_FOCUS)
				{
					p->selitem = (mListViewItem *)ev->notify.param1;

					_change_sel(p);
				}
				break;
			//追加
			case WID_TOOL_ADD:
				_cmd_add(p);
				break;
			//削除
			case WID_TOOL_DEL:
				_cmd_del(p);
				break;
			//置換
			case WID_TOOL_REPLACE:
				_cmd_replace(p);
				break;
			//クリア
			case WID_TOOL_KEY_CLEAR:
				mInputAccelKey_setKey(p->ackey, 0);
				break;
		}
	}
}


//========================
// メイン
//========================


/** 作成 */

void EnvOptTool_create(EnvOptDlgContents *dat,mWidget *parent,mStr *array)
{
	EnvOptToolData *p;

	//ウィジェット作成

	mWidgetBuilderCreateFromText(parent, g_wb_envtool);

	//データ

	p = (EnvOptToolData *)mMalloc(sizeof(EnvOptToolData), TRUE);
	if(!p) return;

	p->strarray = array;

	p->lv = (mListView *)mWidgetFindByID(parent, WID_TOOL_LIST);
	p->editName = (mLineEdit *)mWidgetFindByID(parent, WID_TOOL_EDIT_NAME);
	p->editCmd = (mLineEdit *)mWidgetFindByID(parent, WID_TOOL_EDIT_CMD);
	p->ackey = (mInputAccelKey *)mWidgetFindByID(parent, WID_TOOL_ACCKEY);

	//リストセット

	_set_list(p);

	//アイテム数

	p->itemnum = mListViewGetItemNum(p->lv);

	//

	dat->data = p;
	dat->destroy = NULL;
	dat->event = _tool_event;
}

/** 終了時 */

mBool EnvOptTool_finish(mStr *array,mBool bOK)
{
	int i;
	mBool ret;

	//更新されたか

	for(i = 0; i < TOOLITEM_NUM; i++)
	{
		if(!mStrCompareEq(array + i, GDAT->strTool[i].buf))
			break;
	}

	ret = (bOK && i != TOOLITEM_NUM);

	//入れ替え

	if(ret)
	{
		for(i = 0; i < TOOLITEM_NUM; i++)
		{
			if(mStrIsEmpty(array + i))
				mStrEmpty(GDAT->strTool + i);
			else
				mStrCopy(GDAT->strTool + i, array + i);
		}
	}

	//

	mStrArrayFree(array, TOOLITEM_NUM);

	return ret;
}
