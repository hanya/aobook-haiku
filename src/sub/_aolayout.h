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
 * レイアウトのデータと関数
 ********************************/

#include "mDef.h"
#include "mListDef.h"
#include "mMemAuto.h"

#include "aoFont.h"
#include "aoStyle.h"

//------------------

/** 本文の１文字データ */

typedef struct
{
	mListItem i;
	
	uint32_t code;	//文字コード
	short x,y,
		width,		//縦中横の文字幅 (px)
		height;		//文字高さ (px)
	uint8_t chartype,	//文字の配置タイプ
		flags,
		bouten,			//傍点タイプ
		bousen, 		//傍線タイプ
		horzchar[4];	//縦中横文字 (0で終了)
}AO_LAYOUT_CHAR;

/** ルビデータ */

typedef struct
{
	mListItem i;

	AO_LAYOUT_CHAR *charTop;	//親文字の先頭位置
	uint32_t *ruby;				//ルビ文字列先頭位置 (内部データのポインタ)
	int rubylen,
		charlen;
	short x,y,
		height,	//ルビ文字列全体の高さ (px)
		areaH;	//親文字の高さ
				//(-1 で余白なし、-2 で１文字に対してルビ１文字、それ以外で余白付き)
}AO_LAYOUT_RUBY;

/** 行の状態 (次の行まで影響が及ばない注記) */

typedef struct
{
	AO_LAYOUT_CHAR *tatetyuyoko_top; //縦中横の先頭文字
	uint8_t tatetyuyoko_num;		//縦中横の現在の文字数 (+1)
	uint8_t jisage,	//字下げ数
		jitsuki,	//地付き、字上げ数 (+1)
		bouten,		//傍点 (0 でなし)
		bousen;		//傍線
}AO_LINESTATE;

/** ブロック型注記の状態 (値はそれぞれ 0 でなし) */

typedef struct
{
	uint8_t flags,
		jisage,		//字下げ
		jisageWrap,	//折り返し以降の字下げ
		jitsuki,	//地付き、字上げ
		title;		//見出し (0:なし 1:大 2:中 3:小)
}AO_BLOCKSTATE;

/** ページの状態 */

typedef struct
{
	mBool center;      //ページの左右中央
	uint8_t *picture;  //挿絵コマンドのデータ位置 (コマンドタイプの次の位置)
}AO_PAGESTATE;

/** ページ情報 */

typedef struct
{
	uint8_t *src;	//内部データの先頭位置
	AO_BLOCKSTATE blockstate;	//現在のブロック型注記の状態
	int pageNo;			//ページ位置
	uint32_t lineno;	//ソーステキストの行番号
	short wrapNum,		//前ページの前行の折り返し行数 (次ページでずらす分)
		diffx;			//ページの左右中央時の先頭 X 位置
	uint8_t flags;
}AO_PAGEINFO;

typedef struct _AO_PAGEINFO_ITEM
{
	mListItem i;
	AO_PAGEINFO dat;
}AO_PAGEINFO_ITEM;

/** 最初のレイアウト時用の作業データ */

typedef struct
{
	AO_PAGEINFO cur,	//現在のページ情報
		next;			//次のページの情報
	int pagenum;		//全ページ数
}AO_LAYOUT_FIRST;

/** レイアウト作業用データ */

typedef struct
{
	uint8_t *text;  //内部データの現在位置
	AO_STYLE *st;	//スタイルデータ
	aoFont *font,
		*fontRuby,
		*fontBold,
		*fontHalf,
		*fontInfo;
	mPixbuf *img;	//描画先
	const char *filepath;	//テキストのファイルパス

	int fontH,
		fontRubyH,
		fontBoldH,
		fontHalfH,
		pagenum,		//全ページ数 (描画時)
		areaW,areaH,	//1ページ分のテキスト描画部分のサイズ (余白部分は除く)
		rightTextX,		//1ページの先頭行の X 位置
		lineNext;		//行幅 (次の行までの px 数)

	int jisageY,		//先頭行の字下げ Y位置 (px)
		jisageWrapY,	//折り返し時の字下げ Y位置
		jiageBottom,	//地からｎ字上げの下端位置
		curline;        //現在のソーステキスト行位置 (内部データで行情報が見つかるたびに更新)

	int titleNum[3];	//見出しの各個数

	AO_LAYOUT_FIRST *lf;	//最初のレイアウト用データ
	
	mList listChar,		//本文文字データ (1行分の作業用)
		listRuby,		//ルビデータ
		*plistTitle;	//見出しのデータ (レイアウト済みの場合)
	mMemAuto memTitle;	//見出しの文字列用

	AO_PAGESTATE pagestate;
	AO_BLOCKSTATE blockstate;
	AO_LINESTATE linestate;
}AO_LAYOUT;

//------------------

#define AO_PAGEINFO_F_BLANK 1	//空白ページ

enum
{
	AO_LAYOUT_CHAR_TYPE_NORMAL,
	AO_LAYOUT_CHAR_TYPE_VERT_ROTATE, //横組み
	AO_LAYOUT_CHAR_TYPE_HORZ_IN_VERT //縦中横
};

enum
{
	AO_LAYOUT_CHAR_F_WRAP       = 1<<0,	//折り返しの先頭文字である
	AO_LAYOUT_CHAR_F_DAKUTEN    = 1<<1, //濁点結合
	AO_LAYOUT_CHAR_F_HANDAKUTEN = 1<<2, //半濁点結合
	AO_LAYOUT_CHAR_F_JITSUKI    = 1<<3, //地付き
	AO_LAYOUT_CHAR_F_BOLD       = 1<<4  //太字
};

enum
{
	AO_BLOCKSTATE_F_YOKOGUMI = 1<<0, //横組み
	AO_BLOCKSTATE_F_BOLD     = 1<<1  //太字
};

//------------------

/* aoLayout_main.c */

mBool layoutPage(AO_LAYOUT *dat,AO_LAYOUT_FIRST *lf);
void layoutDrawPage(AO_LAYOUT *dat,AO_PAGEINFO *page,int pageno);

/* aoLayout.line.c */

mBool layoutGetLine(AO_LAYOUT *dat,int *cmd);

/* aoLayout_draw.c */

void layoutDrawLine(AO_LAYOUT *dat,int page);
void layoutDrawPageNo(AO_LAYOUT *dat,int pageno,int pos);
void layoutDrawPicture(AO_LAYOUT *dat,int page);

/* aoLayout_sub.c */

mBool isMatchChar(uint32_t *match,uint32_t c);
mBool isHorzInVertChar(uint32_t c);
mBool isNoBottomChar(AO_LAYOUT *dat,AO_LAYOUT_CHAR *pi);
mBool replaceChar(uint32_t *text,uint32_t *c);

int getRubyLeftInfo(AO_LAYOUT *dat,AO_LAYOUT_RUBY *pir,int *pwrap);
mBool isRubyConnect(AO_LAYOUT_RUBY *pi,AO_LAYOUT_RUBY *next);

void aolAppendTitleList(AO_LAYOUT *dat);
