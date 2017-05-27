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
 * 内部データの値 定義
 ********************************/

enum
{
	AO_TYPE_END,
	AO_TYPE_LINEINFO,
	AO_TYPE_NORMAL_STR,
	AO_TYPE_RUBY_STR,
	AO_TYPE_RETURN,
	AO_TYPE_COMMAND,
	AO_TYPE_COMMAND_VAL,
	AO_TYPE_PICTURE
};

enum
{
	AO_CMD_NONE,
	AO_CMD_PICTURE,	//挿絵

	AO_CMD_KAITYO,		//改丁
	AO_CMD_KAIPAGE,		//改ページ
	AO_CMD_KAIMIHIRAKI,	//改見開き
	AO_CMD_PAGE_CENTER,	//左右中央

	AO_CMD_TITLE_START,	//見出し
	AO_CMD_TITLE_END,

	AO_CMD_JISAGE_LINE, //字下げ
	AO_CMD_JISAGE_START,
	AO_CMD_JISAGE_END,
	AO_CMD_JISAGE_WRAP_START,
	AO_CMD_JISAGE_TEN_WRAP_START,

	AO_CMD_JITSUKI_LINE, //地付き or 地からｎ字上げ
	AO_CMD_JITSUKI_START,
	AO_CMD_JITSUKI_END,

	AO_CMD_BOLD_START,	//太字
	AO_CMD_BOLD_END,
	AO_CMD_TATETYUYOKO_START,	//縦中横
	AO_CMD_TATETYUYOKO_END,
	AO_CMD_YOKOGUMI_START,		//横組み
	AO_CMD_YOKOGUMI_END,

	AO_CMD_BOUTEN_START,	//傍点
	AO_CMD_BOUTEN_END,
	AO_CMD_BOUSEN_START,	//傍線
	AO_CMD_BOUSEN_END
};
