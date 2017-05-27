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
 * ZIP 解凍
 ********************************/

#include <zlib.h>

#include "mDef.h"
#include "mFile.h"
#include "mUtil.h"
#include "mZlib.h"


//--------------------

typedef struct
{
	uint16_t method;
	uint32_t compsize,uncompsize;
}ZipFileInfo;

//--------------------


/** ローカルファイルヘッダ取得
 *
 * @return 0:終了 -1:次のファイルへ 1:成功 */

static int _getLocalFileHader(mFile file,ZipFileInfo *info)
{
	uint8_t buf[26];
	uint16_t flags;
	
	//識別子

	if(mFileGet32LE(file) != 0x04034B50) return 0;

	//読み込み

	if(!mFileReadSize(file, buf, 26)) return 0;

	//LE -> システムのエンディアン

	mConvertEndianBuf(buf, -1, "2222244422");

	//-------

	//フラグ

	flags = M_BUF_UINT16(buf + 2);

	//圧縮メソッド

	info->method = M_BUF_UINT16(buf + 4);

	//圧縮/非圧縮サイズ

	info->compsize   = M_BUF_UINT32(buf + 14);
	info->uncompsize = M_BUF_UINT32(buf + 18);

	//名前・拡張フィールドをスキップ

	mFileSeekCur(file, M_BUF_UINT16(buf + 22));
	mFileSeekCur(file, M_BUF_UINT16(buf + 24));

	//有効なファイルか

	if(!(flags & 1)
		&& (info->method == 0 || info->method == 8)
		&& info->compsize && info->uncompsize)
		return 1;
	else
	{
		//次のファイルへ
		mFileSeekCur(file, info->compsize);
		return -1;
	}
}

/** データ展開 */

static mBool _decode(mFile file,ZipFileInfo *info,mBuf *buf)
{
	mBool ret;

	//メモリ確保

	buf->buf = mMalloc(info->uncompsize, FALSE);
	if(!buf->buf) return FALSE;

	buf->size = info->uncompsize;

	//展開

	if(info->method == 0)
		//無圧縮
		ret = mFileReadSize(file, buf->buf, buf->size);
	else
		//deflate
		ret = mZlibDecodeFILE(file, buf->buf, buf->size, info->compsize, 0, -15);

	//
	
	if(ret)
		return TRUE;
	else
	{
		mFree(buf->buf);
		return FALSE;
	}
}

/** ZIP の先頭ファイルを読み込み */

mBool extractZipFile(const char *filename,mBuf *dstbuf)
{
	mFile file;
	ZipFileInfo info;
	int ret;
	mBool result = FALSE;

	if(!mFileOpenRead(&file, filename, 0))
		return FALSE;

	//有効なファイルが来るまで検索

	while(1)
	{
		ret = _getLocalFileHader(file, &info);
		if(ret >= 0) break;
	}

	//ファイルが見つかれば解凍

	if(ret == 1)
		result = _decode(file, &info, dstbuf);

	mFileClose(file);

	return result;
}
