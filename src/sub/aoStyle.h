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
 * スタイルデータ
 ********************************/

#ifndef AOSTYLE_H
#define AOSTYLE_H

typedef struct _AO_STYLE
{
	int chars,		//文字数
		lines,		//行数
		pages,		//ページ数 (1or2)
		charSpace,	//字間
		lineSpace,	//行間
		pageSpace,	//ページ間の空白
		flags;

	mRect margin;	//余白

	mRgbCol colText, //本文文字色
		colRuby,	//ルビ文字色
		colInfo;

	uint32_t *strNoHead,	//行頭禁則
		*strNoBottom,		//行末禁則
		*strHanging,		//ぶら下げ対象文字
		*strNoSep,			//分割禁止
		*strReplace;		//置換
}AO_STYLE;


enum
{
	AOSTYLE_F_HANGING        = 1<<0, //ぶら下げ有効
	AOSTYLE_F_ENABLE_PICTURE = 1<<1, //挿絵表示
	AOSTYLE_F_DRAW_PAGENO    = 1<<2, //ページ数描画
	AOSTYLE_F_DASH_TO_LINE   = 1<<3  //全角ダッシュを直線に
};

#endif
