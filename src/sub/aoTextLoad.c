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
 * テキスト変換 (UTF-16BE)
 ********************************/

#include <iconv.h>
#include <errno.h>

#include "mDef.h"
#include "mMemAuto.h"

#include "aoText.h"


//文字コード判定のバイト数
#define _CHECK_MAXSIZE  1024


//===========================
// 文字コード判定
//===========================


/** UTF-8 のチェック */

static mBool _check_utf8(uint8_t *buf,uint32_t size)
{
	uint8_t c,*p = buf;
	int i,len;

	while(size && p - buf < _CHECK_MAXSIZE)
	{
		c = *(p++);
		size--;

		if(c < 0x80)
			//ASCII
			continue;
		else if(c <= 0xc1 || c >= 0xfe)
			//この範囲は UTF-8 では先頭バイトに来ない
			return FALSE;
		else if(c <= 0xfd)
		{
			//2バイト目以降のバイト数
			
			if(c >= 0xfc) len = 5;
			else if(c >= 0xf8) len = 4;
			else if(c >= 0xf0) len = 3;
			else if(c >= 0xe0) len = 2;
			else len = 1;

			//データが足りない

			if(size < len) return FALSE;

			//2バイト目以降が 0x80-0xBF の範囲内か

			for(i = len; i; i--, p++)
			{
				if(*p < 0x80 && *p > 0xbf) return FALSE;
			}

			size -= len;
		}
	}

	return TRUE;
}

/** Shift-JIS のチェック */

static int _check_sjis(uint8_t *buf,uint32_t size)
{
	uint8_t *p = buf,c,c2;

	while(size && p - buf < _CHECK_MAXSIZE)
	{
		c = *(p++);
		size--;

		if(c < 0x80 || (c >= 0xa1 && c <= 0xdf))
			//ASCII/半角カナ
			continue;
		else if((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc))
		{
			//2byte文字

			if(size < 1) return FALSE;

			c2 = *(p++);
			size--;

			if(!((c2 >= 0x40 && c2 <= 0x7e) || (c2 >= 0x80 && c2 <= 0xfc)))
				return FALSE;
		}
		else
			return FALSE;
	}

	return TRUE;
}

/** EUC-JP のチェック */

static int _check_eucjp(uint8_t *buf,uint32_t size)
{
	uint8_t *p = buf,c,c2;
	int ret = 1;

	while(size && p - buf < _CHECK_MAXSIZE)
	{
		c = *(p++);
		size--;

		if(c < 0x80)
			//ASCII
			continue;
		else if(c >= 0xa1 && c <= 0xfe)
		{
			//漢字など

			if(size < 1) return FALSE;

			c2 = *(p++);
			size--;

			if(c2 < 0xa1 || c2 > 0xfe) return FALSE;

			/* EUC-JP の 'ひらカナ' は Shift-JIS では半角カナになるので、
			 * この範囲なら EUC-JP を優先 */

		/*
			if((c == 0xa4 || c == 0xa5)
				&& ((c2 >= 0xa1 && c2 <= 0xf3) || (c2 >= 0xa1 && c2 <= 0xf6)))
				ret = 2;
		*/
		}
		else if(c == 0x8e)
		{
			//半角カナ

			if(size < 1) return FALSE;

			c2 = *(p++);
			size--;

			if(c2 < 0xa1 || c2 > 0xdf) return FALSE;
		}
		else if(c == 0x8f)
		{
			//補助漢字

			if(size < 2) return FALSE;

			if((*p >= 0xa1 && *p <= 0xfe) || (p[1] >= 0xa1 && p[1] <= 0xfe))
			{
				p += 2;
				size -= 2;
			}
			else
				return FALSE;
		}
		else
			return FALSE;
	}

	return ret;
}

/** 文字エンコード自動判定 */

static int _auto_charcode(uint8_t *buf,uint32_t size)
{
	uint8_t *p = buf;
	int sjis,euc;

	//------ 先頭が BOM か

	//UTF-16

	if(size >= 2)
	{
		if(p[0] == 0xff && p[1] == 0xfe)
			return AOTEXT_CODE_UTF16LE;
		else if(p[0] == 0xfe && p[1] == 0xff)
			return AOTEXT_CODE_UTF16BE;
	}

	//UTF-8

	if(size >= 3 && p[0] == 0xef && p[1] == 0xbb && p[2] == 0xbf)
		return AOTEXT_CODE_UTF8;

	//------ UTF8/Shift-JIS/EUC/UTF16LE 判定

	if(_check_utf8(buf, size))
		//UTF-8
		return AOTEXT_CODE_UTF8;
	else
	{
		//UTF-8 ではない
	
		sjis = _check_sjis(buf, size);
		euc = _check_eucjp(buf, size);

		if(!sjis && !euc)
			return AOTEXT_CODE_UTF16LE;
		else if(sjis && !euc)
			return AOTEXT_CODE_SHIFTJIS;
		else
			return AOTEXT_CODE_EUCJP;
	}
}


//=======================
// メイン
//=======================


/** UTF-16BE に変換 */

static mBool _convert_to_utf16be(mMemAuto *memdst,void *srcbuf,uint32_t srcsize,int srccode)
{
	iconv_t icv = (iconv_t)-1;
	char *inbuf,*dstbuf,*workbuf;
	size_t insize,dstsize,ret;
	mBool result = FALSE,bLoop;

	//iconv 出力バッファ

	workbuf = (char *)mMalloc(4096, FALSE);
	if(!workbuf) return FALSE;

	//

	inbuf   = (char *)srcbuf;
	insize  = srcsize;
	dstbuf  = workbuf;
	dstsize = 4096;

	//変換

	icv = iconv_open("UTF-16BE", aoTextGetCodeName_conv(srccode));
	if(icv == (iconv_t)-1) goto END;

	do
	{
		ret = iconv(icv, &inbuf, &insize, &dstbuf, &dstsize);

		//出力先バッファが足りなければループ
		bLoop = (ret == -1 && errno == E2BIG);

		if(!mMemAutoAppend(memdst, workbuf, 4096 - dstsize))
			goto END;

		dstbuf  = workbuf;
		dstsize = 4096;

	} while(bLoop);

	result = TRUE;

	//終了

END:
	if(icv != (iconv_t)-1)
		iconv_close(icv);

	mFree(workbuf);

	return result;
}

/** テキストを UTF-16BE に変換
 *
 * @param code -1 で文字エンコード自動判定
 * @return 元テキストの文字コード (-1 でエラー) */

int aoTextConvUnicode(mBuf *srcbuf,mBuf *dstbuf,int code)
{
	mMemAuto mem;

	//文字エンコード自動判定

	if(code < 0)
		code = _auto_charcode((uint8_t *)srcbuf->buf, srcbuf->size);

	//出力バッファ確保

	mMemAutoInit(&mem);

	if(!mMemAutoAlloc(&mem, (srcbuf->size + 4095) & (~4095), 4096))
		return -1;

	//UTF-16BE に変換

	if(!_convert_to_utf16be(&mem, srcbuf->buf, srcbuf->size, code))
	{
		mMemAutoFree(&mem);
		return -1;
	}

	//

	mMemAutoCutCurrent(&mem);

	dstbuf->buf  = mem.buf;
	dstbuf->size = mem.curpos;

	return code;
}

/** 文字エンコード名取得 (表示用) */

const char *aoTextGetCodeName(int code)
{
	const char *codename[] = {
		"Shift-JIS", "EUC-JP", "UTF-8", "UTF-16LE", "UTF-16BE"
	};

	if(code < 0)
		return "auto";
	else
		return codename[code];
}

/** 文字エンコード名取得 (iconv 変換時用) */

const char *aoTextGetCodeName_conv(int code)
{
	const char *codename[] = {
		"CP932", "EUC-JP", "UTF-8", "UTF-16LE", "UTF-16BE"
	};

	return codename[code];
}
