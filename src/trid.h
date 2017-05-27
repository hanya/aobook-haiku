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
 * 翻訳文字列 ID
 ********************************/

#ifndef TRID_H
#define TRID_H

/* メッセージ */
enum
{
	TRMES_ERR_LOADFILE,
	TRMES_ERR_CONVCODE,
	TRMES_ERR_BUF,
	TRMES_CONFIRM_ALLCLEAR,
	TRMES_CONFIRM_DELETE,
	TRMES_STYLE_SAMENAME
};

/* ダイアログ */
enum
{
	TRDLG_MOVEPAGENO_TITLE,
	TRDLG_MOVEPAGENO_MES,
	TRDLG_MOVELINENO_TITLE,
	TRDLG_MOVELINENO_MES,
	TRDLG_CAPTION_TITLE,
	TRDLG_BMCOMMENT_TITLE,
	TRDLG_BMCOMMENT_MES,
	TRDLG_STYLENEW_TITLE,
	TRDLG_STYLENEW_MES
};

#endif
