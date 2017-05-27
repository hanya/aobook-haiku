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
 * レイアウト
 *
 * 内部データから1行分のレイアウト用データ取得
 **********************************/

#include <string.h>

#include "_aolayout.h"
#include "_aobufdef.h"

#include "mList.h"


//-----------------

#define _BST_FLAG_ON(a)  bst->flags |= AO_BLOCKSTATE_F_ ## a
#define _BST_FLAG_OFF(a) bst->flags &= ~(AO_BLOCKSTATE_F_ ## a)

//-----------------


//=================================
// コマンド処理
//=================================


/** 値なしのコマンド処理
 *
 * @return レイアウト関連のコマンドならコマンド番号。-1 で次のデータを続ける */

static int _procCommand(AO_LAYOUT *dat,int cmd)
{
	AO_LINESTATE *lst = &dat->linestate;
	AO_BLOCKSTATE *bst = &dat->blockstate;

	switch(cmd)
	{
		//改〜
		case AO_CMD_KAIPAGE:
		case AO_CMD_KAITYO:
		case AO_CMD_KAIMIHIRAKI:
			return cmd;
		//字下げ終わり (ブロック)
		case AO_CMD_JISAGE_END:
			bst->jisage = bst->jisageWrap = 0;
			break;
		//縦中横
		case AO_CMD_TATETYUYOKO_START:
			lst->tatetyuyoko_num = 1;
			break;
		case AO_CMD_TATETYUYOKO_END:
			lst->tatetyuyoko_num = 0;
			break;
		//横組み
		case AO_CMD_YOKOGUMI_START:
			_BST_FLAG_ON(YOKOGUMI);
			break;
		case AO_CMD_YOKOGUMI_END:
			_BST_FLAG_OFF(YOKOGUMI);
			break;
		//太字
		case AO_CMD_BOLD_START:
			_BST_FLAG_ON(BOLD);
			break;
		case AO_CMD_BOLD_END:
			_BST_FLAG_OFF(BOLD);
			break;
		//傍点終わり
		case AO_CMD_BOUTEN_END:
			lst->bouten = 0;
			break;
		//傍線終わり
		case AO_CMD_BOUSEN_END:
			lst->bousen = 0;
			break;
		//地付き、地上げ終わり
		case AO_CMD_JITSUKI_END:
			bst->jitsuki = 0;
			break;
		//見出し終わり
		case AO_CMD_TITLE_END:
			bst->title = 0;

			if(dat->plistTitle)
				aolAppendTitleList(dat);
			break;
		//ページの左右中央
		case AO_CMD_PAGE_CENTER:
			dat->pagestate.center = TRUE;
			break;
	}

	return -1;
}

/** 値付きのコマンド処理 */

static void _procCommandVal(AO_LAYOUT *dat,uint8_t **pptop)
{
	uint8_t *p,cmd,val1,val2;
	AO_LINESTATE *lst = &dat->linestate;
	AO_BLOCKSTATE *bst = &dat->blockstate;

	p = *pptop;

	cmd  = p[0];
	val1 = p[1];
	val2 = p[2];

	*pptop = p + 3;

	switch(cmd)
	{
		//字下げ (1行)
		case AO_CMD_JISAGE_LINE:
			lst->jisage = val1;
			break;
		//字下げ (ブロック)
		case AO_CMD_JISAGE_START:
			bst->jisage = val1;
			bst->jisageWrap = val1;
			break;
		//字下げ、折り返し
		case AO_CMD_JISAGE_WRAP_START:
			bst->jisage = val1;
			bst->jisageWrap = val2;
			break;
		//字下げ、天付き・折り返し
		case AO_CMD_JISAGE_TEN_WRAP_START:
			bst->jisage = 0;
			bst->jisageWrap = val1;
			break;
		//傍点開始
		case AO_CMD_BOUTEN_START:
			lst->bouten = val1;
			break;
		//傍線開始
		case AO_CMD_BOUSEN_START:
			lst->bousen = val1;
			break;
		//地付き、地からｎ字上げ (1行)
		case AO_CMD_JITSUKI_LINE:
			lst->jitsuki = val1;
			break;
		//地付き、地からｎ字上げ (ブロック)
		case AO_CMD_JITSUKI_START:
			bst->jitsuki = val1;
			break;
		//見出し
		case AO_CMD_TITLE_START:
			bst->title = val1;
			break;
	}
}

/** 挿絵処理
 * 
 * [2byte] 文字列長さ
 * [文字数] ファイル名 (UTF-8。NULL 文字含む) */

static void _procPicture(AO_LAYOUT *dat,uint8_t **pptop)
{
	uint8_t *ps;
	int len;

	ps = *pptop;

	//コマンドの先頭位置をセット
	dat->pagestate.picture = ps;

	len = *((uint16_t *)ps);
	ps += 2;

	*pptop = ps + len;
}


//=================================
// ルビなし本文文字列追加
//=================================


/** [本文文字追加時] 現在の注記状態セット
 *
 * [!] 縦中横の場合、pi 自身がリストから削除される場合がある */

static void _text_setCharState(AO_LAYOUT *dat,AO_LAYOUT_CHAR *pi)
{
	AO_BLOCKSTATE *bst = &dat->blockstate;
	AO_LINESTATE *lst = &dat->linestate;
	int n;

	//地付き/地上げ

	if(lst->jitsuki || bst->jitsuki)
		pi->flags |= AO_LAYOUT_CHAR_F_JITSUKI;

	//太字

	if(bst->flags & AO_BLOCKSTATE_F_BOLD)
		pi->flags |= AO_LAYOUT_CHAR_F_BOLD;

	//傍点/傍線

	pi->bouten = lst->bouten;
	pi->bousen = lst->bousen;

	//見出し文字列
	/* 見出しが終わるまで、memTitle に文字を追加していく
	 * (最初のレイアウト時のみ) */

	if(bst->title && dat->plistTitle)
	{
		//最初の文字の場合、1byte目に見出しタイプ
		
		if(dat->memTitle.curpos == 0)
			mMemAutoAppendByte(&dat->memTitle, bst->title);

		//文字追加

		if(dat->memTitle.curpos <= 256 * 4)
			mMemAutoAppend(&dat->memTitle, &pi->code, 4);
	}

	//----- 文字の並べ方

	if(bst->flags & AO_BLOCKSTATE_F_YOKOGUMI)
		//横組み
		pi->chartype = AO_LAYOUT_CHAR_TYPE_VERT_ROTATE;
	else if(lst->tatetyuyoko_num)
	{
		//縦中横

		n = lst->tatetyuyoko_num;
	
		if(n < 4 && pi->code < 127)
		{
			if(n == 1)
			{
				//最初の文字
			
				pi->chartype = AO_LAYOUT_CHAR_TYPE_HORZ_IN_VERT;
				pi->horzchar[0] = (uint8_t)pi->code;

				lst->tatetyuyoko_top = pi;
			}
			else
			{
				//2文字目以降は先頭文字に結合し、データ削除
			
				(lst->tatetyuyoko_top)->horzchar[n - 1] = (uint8_t)pi->code;
			
				mListDelete(&dat->listChar, M_LISTITEM(pi));
			}

			lst->tatetyuyoko_num++;
		}
	}
}

/** [本文文字追加時] 文字列追加後の調整
 *
 * @param charlen 追加された文字数
 * @param replace 置き換え文字のリスト (２文字で１組)
 * @return 調整後の文字数 */

static int _text_adjust(mList *list,int charlen,uint32_t *replace)
{
	AO_LAYOUT_CHAR *pi,*next,*prev,*third;
	uint32_t c;

	for(pi = (AO_LAYOUT_CHAR *)list->top; pi; pi = next)
	{
		prev = (AO_LAYOUT_CHAR *)pi->i.prev;
		next = (AO_LAYOUT_CHAR *)pi->i.next;

		c = pi->code;

		if(replaceChar(replace, &c))
		{
			//文字置き換え

			pi->code = c;
		}
		else if(c == 0x309b || c == 0x309c)
		{
			//濁点・半濁点の場合、１文字にまとめる

			if(prev && !(prev->flags & (AO_LAYOUT_CHAR_F_DAKUTEN | AO_LAYOUT_CHAR_F_HANDAKUTEN)))
			{
				prev->flags |= (c == 0x309b)? AO_LAYOUT_CHAR_F_DAKUTEN: AO_LAYOUT_CHAR_F_HANDAKUTEN;

				mListDelete(list, M_LISTITEM(pi));
				charlen--;
			}
		}
		else if(c == L'／')
		{
			//くの字点を置換え

			if(next && next->code == L'＼')
			{
				pi->code = 0x3033;
				next->code = 0x3035;
			}
			else if(next && next->code == L'″'
				&& next->i.next && ((AO_LAYOUT_CHAR *)next->i.next)->code == L'＼')
			{
				//濁点付き
			
				third = (AO_LAYOUT_CHAR *)next->i.next;

				mListDelete(list, M_LISTITEM(next));
				charlen--;
			
				pi->code = 0x3034;
				third->code = 0x3035;

				next = third;
			}
		}
	}

	return charlen;
}


//=================================
// １行分取得
//=================================


/** ルビなしの本文文字列を追加
 *
 * [2byte] 文字数
 * [4byte x 文字数] 文字列
 *
 * @param ppCharTop NULL 以外の場合、追加された先頭の文字データを返す
 * @return 追加された文字数 */

static int _appendText(AO_LAYOUT *dat,uint8_t **pptop,AO_LAYOUT_CHAR **ppCharTop)
{
	uint8_t *p;
	int clen,i;
	AO_LAYOUT_CHAR *pi;
	mList *list = &dat->listChar;
	mListItem *piBottom;

	p = *pptop;

	//文字数

	clen = *((uint16_t *)p);
	p += 2;

	//次のデータ位置

	*pptop = p + (clen << 2);

	//現在の終端位置

	piBottom = list->bottom;

	//リストに1文字ずつ追加 (縦中横の場合は一つのデータにまとめられる)

	for(i = clen; i > 0; i--, p += 4)
	{
		pi = (AO_LAYOUT_CHAR *)mListAppendNew(list, sizeof(AO_LAYOUT_CHAR), NULL);
		if(!pi) continue;

		pi->code = *((uint32_t *)p);

		//現在の注記状態をセット

		_text_setCharState(dat, pi);
	}

	//調整

	clen = _text_adjust(list, clen, dat->st->strReplace);

	//追加された先頭の文字の位置

	if(ppCharTop)
		*ppCharTop = (piBottom)? (AO_LAYOUT_CHAR *)piBottom->next: (AO_LAYOUT_CHAR *)list->top;

	return clen;
}

/** ルビ付き文字列を追加
 *
 * [2byte] 本文文字数
 * [4byte x 文字数] 本文文字列
 * [2byte] ルビ文字数
 * [4byte x 文字数] ルビ文字列 */

static void _appendRuby(AO_LAYOUT *dat,uint8_t **pptop)
{
	uint8_t *p;
	int clen,rlen;
	AO_LAYOUT_CHAR *piCharTop;
	AO_LAYOUT_RUBY *piRuby;

	//本文文字列を追加

	clen = _appendText(dat, pptop, &piCharTop);

	//ルビデータをリストに追加

	p = *pptop;

	rlen = *((uint16_t *)p);
	p += 2;

	piRuby = (AO_LAYOUT_RUBY *)mListAppendNew(&dat->listRuby, sizeof(AO_LAYOUT_RUBY), NULL);

	if(piRuby)
	{
		piRuby->charTop = piCharTop;
		piRuby->ruby = (uint32_t *)p;
		piRuby->rubylen = rlen;
		piRuby->charlen = clen;
		piRuby->height = dat->fontRubyH * rlen;
		piRuby->areaH = -1;
	}

	//次のデータ位置

	*pptop = p + (rlen << 2);
}

/** 内部データから１行分のデータを取得
 *
 * dat->listChar, listRuby に文字データ、
 * 注記の情報は行/ブロック状態にセット。 */

static void _getLine(AO_LAYOUT *dat,int *cmdret)
{
	uint8_t *p = dat->text,type;
	int loop = TRUE,cmd;

	*cmdret = -1;

	while(loop)
	{
		//データタイプ (1byte)
		type = *(p++);

		switch(type)
		{
			//行番号の情報
			case AO_TYPE_LINEINFO:
				dat->curline = *((uint32_t *)p);
				p += 4;
				break;
			//通常文字列
			case AO_TYPE_NORMAL_STR:
				_appendText(dat, &p, NULL);
				break;
			//ルビ付き文字列
			case AO_TYPE_RUBY_STR:
				_appendRuby(dat, &p);
				break;
			//改行
			case AO_TYPE_RETURN:
				loop = FALSE;
				break;
			//値なしコマンド
			case AO_TYPE_COMMAND:
				cmd = _procCommand(dat, *(p++));
				if(cmd != -1)
				{
					//ページが変わるコマンドの場合
					
					loop = FALSE;

					if(dat->listChar.top)
						/* 本文文字が残っている場合は先に処理させるので、
						 * 処理位置を戻す */
						p -= 2;
					else
						*cmdret = cmd;
				}
				break;
			//値付きコマンド
			case AO_TYPE_COMMAND_VAL:
				_procCommandVal(dat, &p);
				break;
			//挿絵
			case AO_TYPE_PICTURE:
				loop = FALSE;

				if(dat->listChar.top)
					//本文文字が残っている場合、先に処理
					p--;
				else
					_procPicture(dat, &p);
				break;
			//終了
			case AO_TYPE_END:
				p--;
				loop = FALSE;
				break;
		}
	}

	dat->text = p;
}


//=================================
// 1行分取得後の調整
//=================================


/** 文字の並び方セット
 *
 * [通常縦書き/横組み/縦中横] */

static void _setCharType(AO_LAYOUT *dat)
{
	AO_LAYOUT_CHAR *p,*pnext,*ptop,*p2;
	int i,cnt;

	for(p = (AO_LAYOUT_CHAR *)dat->listChar.top; p; p = pnext)
	{
		pnext = (AO_LAYOUT_CHAR *)p->i.next;
	
		//すでに注記で文字種が指定されているか、半角文字以外ならそのまま
		
		if(p->chartype != AO_LAYOUT_CHAR_TYPE_NORMAL || p->code >= 128)
			continue;
	
		//------- 半角文字の自動判定
	
		//縦中横の対象文字が連続している数

		for(cnt = 0, p2 = p; cnt < 4 && p2 && isHorzInVertChar(p2->code);
				cnt++, p2 = (AO_LAYOUT_CHAR *)p2->i.next);

		//

		if(cnt >= 1 && cnt <= 3)
		{
			//[縦中横]
			/* 半角文字が3文字以内で連続している場合は自動で縦中横。
			 * 最初の文字だけ残して1文字にまとめる */
		
			ptop = p;

			ptop->chartype = AO_LAYOUT_CHAR_TYPE_HORZ_IN_VERT;
			ptop->horzchar[0] = (uint8_t)ptop->code;

			for(i = 1, p = pnext; p && i < cnt; p = p2, i++)
			{
				p2 = (AO_LAYOUT_CHAR *)p->i.next;

				ptop->horzchar[i] = (uint8_t)p->code;

				mListDelete(&dat->listChar, M_LISTITEM(p));
			}
		}
		else
		{
			//[横組み]
			/* 半角文字が4文字以上連続しているなら横組み。
			 * 非半角文字が出るまで続ける。 */

			for(; p && p->code < 127; p = (AO_LAYOUT_CHAR *)p->i.next)
				p->chartype = AO_LAYOUT_CHAR_TYPE_VERT_ROTATE;
		}

		pnext = p;
	}
}

/** 本文文字列の文字幅/高さセット */

static void _setCharHeight(AO_LAYOUT *dat)
{
	AO_LAYOUT_CHAR *p;
	aoFont *font[2];
	int fontH[2],n;

	font[0] = dat->font;
	font[1] = dat->fontBold;
	fontH[0] = dat->fontH;
	fontH[1] = dat->fontBoldH;

	for(p = (AO_LAYOUT_CHAR *)dat->listChar.top; p; p = (AO_LAYOUT_CHAR *)p->i.next)
	{
		//太字か
		n = ((p->flags & AO_LAYOUT_CHAR_F_BOLD) != 0);

		switch(p->chartype)
		{
			//通常
			case AO_LAYOUT_CHAR_TYPE_NORMAL:
				p->height = fontH[n];
				break;
			//横組み
			case AO_LAYOUT_CHAR_TYPE_VERT_ROTATE:
				p->height = aoFontGetTextWidth(font[n], &p->code, 1);
				break;
			//縦中横
			case AO_LAYOUT_CHAR_TYPE_HORZ_IN_VERT:
				p->width  = aoFontGetTextWidth8(font[n], p->horzchar, -1);
				p->height = fontH[n];
				break;
		}
	}
}


//=============================
// main
//=============================


/** １行分の文字データ取得
 *
 * @param cmd -1 で通常。それ以外で改ページなどページ単位のコマンドの番号
 * @return FALSE でデータの終端 */

mBool layoutGetLine(AO_LAYOUT *dat,int *cmd)
{
	if(*(dat->text) == AO_TYPE_END) return FALSE;

	//行状態クリア

	memset(&dat->linestate, 0, sizeof(AO_LINESTATE));

	//データ取得

	_getLine(dat, cmd);

	//各情報セット

	_setCharType(dat);
	_setCharHeight(dat);

	return TRUE;
}

