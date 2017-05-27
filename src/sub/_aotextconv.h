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

/********************************
 * UTF32 -> 内部データ変換定義
 ********************************/

#include <stddef.h>  //wchar_t

#include "mDef.h"
#include "mMemAuto.h"
#include "mList.h"

#include "_aobufdef.h"


/** 変換メインデータ */

typedef struct
{
	void *textbuf;    //変換元テキストのバッファ (UTF-16BE)
	uint8_t *src;     //現在の変換元テキスト位置
	uint32_t remain,  //変換元の残りバイト数
		curline;      //現在のテキスト行位置 (0〜)

	mBool match_no_enter;  //注記マッチ時、次の改行を無効にするか

	mMemAuto memDat,  //出力バッファ
			memLine;  //作業用:1行分のテキスト (UTF-32)
	mList listTmp;    //作業用:1行分の中間データ (AO_CONVTMP)
}AO_CONVERT;

/** 1行変換中の一文字データ (作業用) */

typedef struct
{
	mListItem i;

	uint32_t ch;
	uint8_t dattype,	//データタイプ
		cmdno,cmdval1,cmdval2;
	uint32_t *rubytop,	//ルビ文字列の先頭位置 (親文字すべてにセット)
		*picture;
	uint16_t rubylen;	//ルビ文字列の長さ
}AO_CONVTMP;

/** 注記マッチ用の定義データ */

typedef struct
{
	const wchar_t *str;
	uint8_t cmd;
}AO_MATCHCMD;

/** 注記マッチの結果データ */

typedef struct
{
	uint8_t cmd,val1,val2,flags;
	uint32_t *text;	//注記内の「」内の文字列
	int textlen;
	AO_CONVTMP *insert;	//注記の効果がかかる文字の先頭位置
}AO_CMDDAT;

//----------

#define AO_CONVTMP_DATTYPE_CHAR 255

#define AO_CMDDAT_F_NOENTER 1

//----------

/* aoTextConv_sub.c */

mBool aocGetSrcLine(AO_CONVERT *p);

mBool aocFindKagiClose(uint32_t **pptop);
uint32_t aocGetHexVal(uint32_t *p);
uint32_t aocGetCharJISText(uint32_t *p);
mBool aocIsKanji(uint32_t c);
mBool aocCompareString(uint32_t *text,const wchar_t *cmp);

mBool aocMatchCmd(uint32_t *ptext,const AO_MATCHCMD *src,AO_CMDDAT *dst);
mBool aocMatchCmdKakko(uint32_t *ptext,const AO_MATCHCMD *src,AO_CMDDAT *dst);
mBool aocMatchCmdPicture(uint32_t *pc,AO_CMDDAT *dst);

void aocSetRubyInfoAuto(AO_CONVERT *dat,uint32_t *rubytop,uint16_t rubylen);
void aocSetRubyInfo(AO_CONVERT *dat,AO_CONVTMP *p,uint32_t *rubytop,uint16_t rubylen);
mBool aocFindTmpTextBottom(AO_CONVERT *dat,AO_CMDDAT *cmd);

void aocAppendTmpNormalChar(AO_CONVERT *dat,uint32_t c);
void aocAppendTmpCommand(AO_CONVERT *dat,AO_CMDDAT *cmd,mBool bInsert);

void aocOutput(AO_CONVERT *dat);
