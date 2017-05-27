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

/*******************************************
 * テキスト -> 内部データ変換 : サブ関数
 *******************************************/

#include <iconv.h>

#include "_aotextconv.h"

#include "mUtilCharCode.h"


//--------------

#define _CONVTMP_IS_CMD(p)    ((p)->dattype != AO_CONVTMP_DATTYPE_CHAR)
#define _CONVTMP_IS_CHAR(p)   ((p)->dattype == AO_CONVTMP_DATTYPE_CHAR)
#define _CONVTMP_ISNOT_CHAR(p) ((p)->dattype != AO_CONVTMP_DATTYPE_CHAR)

//--------------


//=============================
// UTF-16BE から１行取得
//=============================


/** UTF-16BE から１文字取得 */

static uint32_t _getSrcChar(AO_CONVERT *p,mBool forward)
{
	uint8_t *ps = p->src;
	uint32_t c,c2;

	if(p->remain == 0) return 0;

	//2byte 取得

	c = (ps[0] << 8) | ps[1];
	ps += 2;

	//4byte 文字

	if((c & 0xfc00) == 0xd800)
	{
		c2 = (ps[0] << 8) | ps[1];
		ps += 2;

		c = (((c & 0x03c0) + 0x40) << 10) | ((c & 0x3f) << 10) | (c2 & 0x3ff);
	}

	//ポインタを進める

	if(forward)
	{
		p->remain -= ps - p->src;
		p->src = ps;
	}

	return c;
}

/** UTF-16BE から UTF-32 文字列１行分取得
 *
 * - 改行文字は除外
 * - 末尾には 0 が入る
 *
 * @return TRUE で文字がある。FALSE で終端。 */

mBool aocGetSrcLine(AO_CONVERT *p)
{
	mMemAuto *mem = &p->memLine;
	mBool end = FALSE;
	uint32_t c;

	mMemAutoReset(mem);

	//1文字ずつ取得し、改行で終了

	while(1)
	{
		c = _getSrcChar(p, TRUE);

		if(c == 0)
		{
			end = TRUE;
			break;
		}
		else if(c == '\n')
			break;
		else if(c == '\r')
		{
			if(_getSrcChar(p, FALSE) == '\n')
				_getSrcChar(p, TRUE);

			break;
		}

		mMemAutoAppend(mem, &c, 4);
	}

	//

	if(end && mem->curpos == 0)
		//終端
		return FALSE;
	else
	{
		//NULL 追加

		c = 0;
		mMemAutoAppend(mem, &c, 4);

		return TRUE;
	}
}


//=============================
// UTF-32 文字列関連
//=============================


/** ISO-2022-JP-3 コードから Unicode 文字に変換 */

static uint32_t _convISOtoUTF32(uint8_t *iso,int size)
{
	uint8_t uni[4];
	char *inbuf,*dstbuf;
	size_t insize,dstsize;
	uint32_t c;

	iconv_t icv = iconv_open("UTF-32BE", "ISO-2022-JP-3");
	if(icv == (iconv_t)-1) return 0;

	inbuf   = (char *)iso;
	dstbuf  = (char *)uni;
	insize  = size;
	dstsize = 4;

	if(iconv(icv, &inbuf, &insize, &dstbuf, &dstsize) == -1)
		c = 0;
	else
		c = ((uint32_t)uni[0] << 24) | (uni[1] << 16) | (uni[2] << 8) | uni[3];

	iconv_close(icv);

	return c;
}


/** '］' を見つけて 0 に置換え、*pptop をその次の位置にセット */

mBool aocFindKagiClose(uint32_t **pptop)
{
	uint32_t *p;

	for(p = *pptop; *p && *p != L'］'; p++);

	if(*p == 0)
		return FALSE;
	else
	{
		*p = 0;
		*pptop = p + 1;

		return TRUE;
	}
}

/** 文字列から16進数の数値取得 */

uint32_t aocGetHexVal(uint32_t *p)
{
	uint32_t v = 0,c;

	while(*p)
	{
		c = *(p++);

		if(c >= '0' && c <= '9')
			c -= '0';
		else if(c >= 'a' && c <= 'f')
			c = c - 'a' + 10;
		else if(c >= 'A' && c <= 'F')
			c = c - 'A' + 10;
		else
			break;

		v <<= 4;
		v |= c;
	}

	return v;
}

/** JIS 面区点文字列から Unicode 文字取得 */

uint32_t aocGetCharJISText(uint32_t *p)
{
	int no[3] = {0,0,0},cur = 0;
	uint32_t c;
	uint8_t iso[6] = {0x1b,'$','(',0,0,0};

	//"面-区-点" 番号を no[0..2] に取得

	while(*p && cur < 3)
	{
		c = *(p++);

		if(c == '-')
			cur++;
		else if(c >= '0' && c <= '9')
		{
			no[cur] *= 10;
			no[cur] += c - '0';
		}
		else
			break;
	}

	if(cur < 2) return 0;

	//面は 1 or 2, 区点は 0-95
	
	if(no[0] != 1 && no[0] != 2) return 0;
	if(no[1] > 96 || no[2] > 96) return 0;

	//ISO-2022-JP-3 コードに

	iso[3] = (no[0] == 1)? 'O': 'P';
	iso[4] = 0x20 + no[1];
	iso[5] = 0x20 + no[2];

	return _convISOtoUTF32(iso, 6);
}

/** Unicode 文字が漢字かどうか */

mBool aocIsKanji(uint32_t c)
{
	return ((c >= 0x2E80 && c <= 0x2EF3)
		|| (c >= 0x2F00 && c <= 0x2FD5)
		|| (c >= 0x3400 && c <= 0x4DB5)
		|| (c >= 0x4E00 && c <= 0x9FFF)
		|| c == L'々' || c == L'〆' || c == L'〇' || c == L'ヶ' || c == L'〻'
		|| (c >= 0xF900 && c <= 0xFAFF)
		|| (c >= 0x20000 && c <= 0x2A6D6)
		|| (c >= 0x2F800 && c <= 0x2FA1D));

	/* 仝 = U+4EDD */
}

/** UTF-32 と wchar_t の文字列比較 */

mBool aocCompareString(uint32_t *text,const wchar_t *cmp)
{
	for(; *cmp && *text == *cmp; text++, cmp++);

	return (*cmp == 0);
}


//=============================
// ［＃...］ 検索
//=============================

/* [!] 終端の '］' は 0 に置換えられている */


/** 注記判定
 *
 * '*' は全角数字としてマッチさせる (最大２箇所。数値は 0..99 まで)。
 * dst->val1,val2 : 値がない場合は 255 */

mBool aocMatchCmd(uint32_t *ptext,const AO_MATCHCMD *src,AO_CMDDAT *dst)
{
	int val[2],valno;
	uint32_t *p;
	const wchar_t *pw;

	for(; src->str; src++)
	{
		//'*' の数値初期化
		
		valno = 0;
		val[0] = -1;
		val[1] = -1;

		//文字列比較
	
		for(p = ptext, pw = src->str; *p && *pw; p++)
		{
			//'*' は全角数字にマッチ
		
			if(*pw == '*')
			{
				if(*p < L'０' || *p > L'９')
				{
					//マッチ解除
					pw++;
					valno++;
				}
				else
				{
					//マッチ中
					
					if(val[valno] == -1)
						val[valno] = 0;
				
					val[valno] *= 10;
					val[valno] += *p - L'０';
					continue;
				}
			}

			//通常文字比較
			
			if(*p != *pw) break;
			pw++;
		}

		//すべてマッチした

		if(*pw == 0 && *p == 0)
		{
			if(val[0] > 99) val[0] = 99;
			if(val[1] > 99) val[1] = 99;
		
			dst->cmd = src->cmd;
			dst->val1 = val[0];
			dst->val2 = val[1];

			return TRUE;
		}
	}

	return FALSE;
}

/** 注記 '「...」...' 判定
 *
 * 「」内の文字列は dst->text,textlen にセット */

mBool aocMatchCmdKakko(uint32_t *ptext,const AO_MATCHCMD *src,AO_CMDDAT *dst)
{
	uint32_t *p,*ptop;
	const wchar_t *pw;

	//「」内の文字列取得

	for(p = ptext; *p && *p != L'」'; p++);
	if(!(*p)) return FALSE;

	dst->text = ptext;
	dst->textlen = p - ptext;

	//'」'以降を比較

	ptop = p + 1;

	for(; src->str; src++)
	{
		//文字列比較
		
		for(p = ptop, pw = src->str; *pw && *p == *pw; p++, pw++);

		if(*pw == 0 && *p == 0)
		{
			dst->cmd = src->cmd;
			return TRUE;
		}
	}

	return FALSE;
}

/** 挿絵注記 判定
 *
 * ［＃キャプション（ファイル名）入る］ */

mBool aocMatchCmdPicture(uint32_t *pc,AO_CMDDAT *dst)
{
	uint32_t c,*pfile = NULL;
	int filelen = -1;

	for(; *pc; pc++)
	{
		c = *pc;
	
		if(c == L'（')
			//ファイル名開始
			pfile = pc + 1;
		else if(c == L'、' || c == L'）')
		{
			//ファイル名終了
			
			if(pfile && filelen < 0)
				filelen = pc - pfile;
		}
		else if(c == L'入' && pc[1] == L'る' && pc[2] == 0)
		{
			//最後が '入る'

			if(!pfile || filelen <= 0) return FALSE;

			dst->cmd = AO_CMD_PICTURE;
			dst->text = pfile;
			dst->textlen = filelen;
			dst->flags |= AO_CMDDAT_F_NOENTER;
			return TRUE;
		}
	}

	return FALSE;
}


//=============================
// 中間データ追加
//=============================


/** 中間データに通常１文字追加 */

void aocAppendTmpNormalChar(AO_CONVERT *dat,uint32_t c)
{
	AO_CONVTMP *p;

	p = (AO_CONVTMP *)mListAppendNew(&dat->listTmp, sizeof(AO_CONVTMP), NULL);

	if(p)
	{
		p->ch = c;
		p->dattype = AO_CONVTMP_DATTYPE_CHAR;
	}
}

/** 中間データにコマンドを追加 */

void aocAppendTmpCommand(AO_CONVERT *dat,AO_CMDDAT *cmd,mBool bInsert)
{
	AO_CONVTMP *p;

	//追加/挿入

	if(bInsert)
	{
		p = (AO_CONVTMP *)mListInsertNew(&dat->listTmp,
				M_LISTITEM(cmd->insert), sizeof(AO_CONVTMP), NULL);
	}
	else
		p = (AO_CONVTMP *)mListAppendNew(&dat->listTmp, sizeof(AO_CONVTMP), NULL);

	if(!p) return;

	//

	if(cmd->cmd == AO_CMD_PICTURE)
	{
		//挿絵
		p->dattype = AO_TYPE_PICTURE;
		p->picture = cmd->text;
		p->rubylen = cmd->textlen;
	}
	else
	{
		p->dattype = (cmd->val1 == 255)? AO_TYPE_COMMAND: AO_TYPE_COMMAND_VAL;
		p->cmdno   = cmd->cmd;
		p->cmdval1 = cmd->val1;
		p->cmdval2 = cmd->val2;
	}
}


//=============================
// 中間データサブ処理
//=============================


/** 中間データの一番最後の通常文字位置取得 */

static AO_CONVTMP *_getTmpBottomChar(AO_CONVERT *dat)
{
	AO_CONVTMP *p;

	for(p = (AO_CONVTMP *)dat->listTmp.bottom;
			p && _CONVTMP_ISNOT_CHAR(p); p = (AO_CONVTMP *)p->i.prev);

	return p;
}

/** ルビの親文字列を自動判定し、ルビ情報セット */

void aocSetRubyInfoAuto(AO_CONVERT *dat,uint32_t *rubytop,uint16_t rubylen)
{
	AO_CONVTMP *p;
	uint32_t c;
	mBool bAlphabet;

	//中間データの一番最後の通常文字位置

	p = _getTmpBottomChar(dat);
	if(!p) return;

	//最後の文字が半角英字か

	c = p->ch;

	bAlphabet = ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));

	//最後の文字から前方向に判定しつつルビ情報セット

	for(; p; p = (AO_CONVTMP *)p->i.prev)
	{
		if(_CONVTMP_ISNOT_CHAR(p)) continue;

		//すでにルビがある文字なら終了
		
		if(p->rubytop) break;

		//文字判定
	
		c = p->ch;

		if(bAlphabet)
		{
			//半角英字は、スペースまたは半角英字以外の文字で終了
			
			if(c == ' ') break;
			if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
				break;
		}
		else
		{
			//漢字以外の文字で終了
			
			if(!aocIsKanji(c)) break;
		}

		p->rubytop = rubytop;
		p->rubylen = rubylen;
	}
}

/** ルビの親文字列にルビの情報セット */

void aocSetRubyInfo(AO_CONVERT *dat,AO_CONVTMP *p,uint32_t *rubytop,uint16_t rubylen)
{
	//先頭位置 (p の次)

	if(p)
		p = (AO_CONVTMP *)p->i.next;
	else
		p = (AO_CONVTMP *)dat->listTmp.top;

	//ルビ文字列情報セット

	for(; p; p = (AO_CONVTMP *)p->i.next)
	{
		if(_CONVTMP_ISNOT_CHAR(p)) continue;
	
		p->rubytop = rubytop;
		p->rubylen = rubylen;
	}
}

/** 中間データの終端の通常文字が cmd->text の文字列と一致する場合、
 * その先頭位置を cmd->insert にセット。 */

mBool aocFindTmpTextBottom(AO_CONVERT *dat,AO_CMDDAT *cmd)
{
	AO_CONVTMP *p;
	uint32_t *pc;
	int len;

	len = cmd->textlen;
	if(len == 0) return FALSE;

	//一番最後の通常文字位置

	p = _getTmpBottomChar(dat);
	if(!p) return FALSE;

	//後ろから比較

	pc = cmd->text + len - 1;

	for(; p; p = (AO_CONVTMP *)p->i.prev)
	{
		if(_CONVTMP_ISNOT_CHAR(p)) continue;

		if(p->ch != *pc) break;

		pc--;
		len--;

		if(len == 0) break;
	}

	//len == 0 で一致

	if(len)
		return FALSE;
	else
	{
		cmd->insert = p;
		return TRUE;
	}
}


//===============================
// 中間データ => 出力データ変換
//===============================


/** 文字列追加 */

static void _appendString(mMemAuto *mem,AO_CONVTMP *p,uint16_t len)
{
	//長さ

	mMemAutoAppend(mem, &len, 2);

	//UTF-32 文字列

	for(; len > 0; len--, p =  (AO_CONVTMP *)p->i.next)
		mMemAutoAppend(mem, &p->ch, 4);
}

/** 挿絵コマンド追加 */

static void _appendCommand_picture(mMemAuto *mem,AO_CONVTMP *p)
{
	char *utf8;
	int len;
	uint16_t lenw;

	utf8 = mUCS4ToUTF8_alloc(p->picture, p->rubylen, &len);
	if(!utf8) return;

	if(len > 0)
	{
		lenw = len + 1;

		mMemAutoAppendByte(mem, p->dattype);
		mMemAutoAppend(mem, &lenw, 2);
		mMemAutoAppend(mem, utf8, lenw);
	}

	mFree(utf8);
}

/** コマンドデータ追加 */

static void _appendCommand(mMemAuto *mem,AO_CONVTMP *p)
{
	if(p->dattype == AO_TYPE_PICTURE)
		//挿絵
		_appendCommand_picture(mem, p);
	else
	{
		//他コマンド

		mMemAutoAppendByte(mem, p->dattype);
		mMemAutoAppendByte(mem, p->cmdno);

		if(p->dattype == AO_TYPE_COMMAND_VAL)
		{
			mMemAutoAppendByte(mem, p->cmdval1);
			mMemAutoAppendByte(mem, p->cmdval2);
		}
	}
}

/** 中間データから出力データに変換 */

void aocOutput(AO_CONVERT *dat)
{
	AO_CONVTMP *p,*pend,*pnext;
	mMemAuto *mem = &dat->memDat;
	int cnt;

	//行情報

	mMemAutoAppendByte(mem, AO_TYPE_LINEINFO);
	mMemAutoAppend(mem, &dat->curline, 4);

	//

	for(p = (AO_CONVTMP *)dat->listTmp.top; p; p = pnext)
	{
		pnext = (AO_CONVTMP *)p->i.next;
	
		//------- コマンド

		if(_CONVTMP_IS_CMD(p))
		{
			_appendCommand(mem, p);
			continue;
		}

		//------ 通常文字列 or ルビ付き

		//通常文字またはルビ付き文字の連続範囲

		for(pend = pnext, cnt = 1; pend; cnt++, pend = (AO_CONVTMP *)pend->i.next)
		{
			if(_CONVTMP_ISNOT_CHAR(pend)) break;
			
			if(p->rubytop != pend->rubytop) break;
		}

		//追加

		if(p->rubytop)
		{
			//ルビ付き文字列

			mMemAutoAppendByte(mem, AO_TYPE_RUBY_STR);
			_appendString(mem, p, cnt);

			mMemAutoAppend(mem, &p->rubylen, 2);
			mMemAutoAppend(mem, p->rubytop, p->rubylen << 2);
		}
		else
		{
			//通常文字列
		
			mMemAutoAppendByte(mem, AO_TYPE_NORMAL_STR);
			_appendString(mem, p, cnt);
		}

		//

		pnext = pend;
	}

	//改行追加

	if(!dat->match_no_enter)
		mMemAutoAppendByte(mem, AO_TYPE_RETURN);

	//中間データ削除

	mListDeleteAll(&dat->listTmp);
}

