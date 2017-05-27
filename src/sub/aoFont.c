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
 * aoFont : フォント
 *****************************************/

#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_BITMAP_H
#include FT_OUTLINE_H
#include FT_LCD_FILTER_H
#include FT_OPENTYPE_VALIDATE_H

#include <string.h>

#include "mDef.h"
#include "mFontConfig.h"
#include "mFreeType.h"
#include "mFTGSUB.h"
#include "mPixbuf.h"
#include "mList.h"

#include "aoFont.h"


//----------------------

static FT_Library g_ftlib = NULL;

//----------------------

typedef struct
{
	mListItem i;
	uint32_t code,flags;
	FT_BitmapGlyph glyph;
}GlyphCache;

struct _aoFont
{
	FT_Face face;
	mBool hasVert;
	int baseline,
		horzHeight,vertHeight, //横、縦書きそれぞれの高さ
		cacheMax;		//キャッシュ最大数

	void *vertTbl;		//縦書き文字テーブル
	mFreeTypeInfo info;

	mList listCache;	//グリフキャッシュ
};

enum
{
	GETGLYPH_F_VERT   = 1<<0,
	GETGLYPH_F_ROTATE = 1<<1
};

static FT_BitmapGlyph _getBitmapGlyph(
	FT_Library lib,aoFont *font,uint32_t code,uint32_t flags);

//----------------------


//==============================
// グリフキャッシュ
//==============================

/* 先頭から順に、新 -> 古 の順で並んでいる。 */


/** 破棄時 */

static void _destroy_cache(mListItem *p)
{
	FT_Done_Glyph((FT_Glyph)((GlyphCache *)p)->glyph);
}

/** キャッシュからグリフ取得 */

static FT_BitmapGlyph _getGlyphCache(aoFont *font,uint32_t code,uint32_t flags)
{
	GlyphCache *pi;

	for(pi = (GlyphCache *)font->listCache.top; pi; pi = (GlyphCache *)pi->i.next)
	{
		if(pi->code == code && pi->flags == flags)
		{
			//先頭へ

			mListMove(&font->listCache, M_LISTITEM(pi), font->listCache.top);
			
			return pi->glyph;
		}
	}

	return NULL;
}

/** キャッシュに追加 */

static void _appendCache(aoFont *font,uint32_t code,uint32_t flags,FT_BitmapGlyph glyph)
{
	mList *list = &font->listCache;
	GlyphCache *pi;

	//最後尾のデータを削除

	if(list->num == font->cacheMax)
		mListDelete(list, list->bottom);

	//先頭に挿入

	pi = (GlyphCache *)mListInsertNew(list, list->top, sizeof(GlyphCache), _destroy_cache);
	if(!pi) return;

	pi->code  = code;
	pi->flags = flags;
	pi->glyph = glyph;
}



//==============================
// サブ
//==============================


/** mFontInfo と fontconfig パターン取得 */

static mFcPattern *_create_getpattern(mFontInfo *info,const char *format,mBool bHalf)
{
	mFcPattern *pat;

	memset(info, 0, sizeof(mFontInfo));

	//フォント情報取得

	mFontFormatToInfo(info, format);

	if(bHalf) info->size /= 2;

	//マッチするパターン
	
	pat = mFontConfigMatch(info);
	if(!pat)
		mFontInfoFree(info);

	return pat;
}

/** フォントの各データセット */

static void _setFontData(aoFont *font,FT_Face face)
{
	void *gsub;
	FT_BitmapGlyph glyph;
	mFreeTypeMetricsInfo metrics;

	//キャッシュ (最低でも1必要)

	font->cacheMax = 10;

	//縦書き情報があるか

	font->hasVert = FT_HAS_VERTICAL(face);

	//縦書きグリフテーブル

	gsub = mFreeTypeGetGSUB(face);

	if(gsub)
	{
		font->vertTbl = mFTGSUB_getVertTable(gsub);
		
		FT_OpenType_Free(face, (FT_Bytes)gsub);
	}

	//ベースライン、横書き高さ

	mFreeTypeGetMetricsInfo(g_ftlib, face, &font->info, &metrics);

	font->baseline   = metrics.baseline;
	font->horzHeight = metrics.height;

	//縦書き文字高さ (グリフを取得して調べる)

	if(font->hasVert)
	{
		glyph = _getBitmapGlyph(g_ftlib, font, L'あ', GETGLYPH_F_VERT);

		if(glyph)
			font->vertHeight = glyph->root.advance.x >> 16;

		//キャッシュクリア
		mListDeleteAll(&font->listCache);
	}

	if(font->vertHeight == 0) font->vertHeight = font->horzHeight;
}


//==============================
// 初期化
//==============================


/** 初期化 */

mBool aoFontInit()
{
	return (FT_Init_FreeType(&g_ftlib) == 0);
}

/** 終了処理 */

void aoFontEnd()
{
	FT_Done_FreeType(g_ftlib);
}


//==============================
// aoFont
//==============================


/** フォント解放 */

void aoFontFree(aoFont *font)
{
	if(font)
	{
		mListDeleteAll(&font->listCache);
		mFree(font->vertTbl);

		FT_Done_Face(font->face);
		
		mFree(font);
	}
}

/** フォント作成 */

aoFont *aoFontCreate(const char *format,mBool bHalf)
{
	FT_Face face = NULL;
	aoFont *font;
	mFontInfo info;
	mFcPattern *pat;
	char *file;
	int index;

	//mFontInfo と、マッチするパターン

	pat = _create_getpattern(&info, format, bHalf);
	if(!pat) return NULL;
	
	//フォントファイル読み込み
	
	if(mFontConfigGetFileInfo(pat, &file, &index))
		goto ERR;
		
	if(FT_New_Face(g_ftlib, file, index, &face))
		goto ERR;
	
	//aoFont 確保
	
	font = (aoFont *)mMalloc(sizeof(aoFont), TRUE);
	if(!font) goto ERR;
	
	font->face = face;

	//FcPattern から情報取得

	mFreeTypeGetInfoByFontConfig(&font->info, pat, &info);

	//解放

	mFontConfigPatternFree(pat);
	mFontInfoFree(&info);

	//文字高さセット (FreeType)

	FT_Set_Char_Size(face, 0, (int)(font->info.size * 64 + 0.5), font->info.dpi, font->info.dpi);

	//他データセット

	_setFontData(font, face);

	return font;

ERR:
	if(face) FT_Done_Face(face);
	mFontConfigPatternFree(pat);
	mFontInfoFree(&info);
	
	return NULL;
}

/** 高さ取得 */

int aoFontGetHeight(aoFont *font)
{
	return (font)? font->vertHeight: 1;
}

/** キャッシュ最大数セット */

void aoFontSetCacheMax(aoFont *font,int max)
{
	if(font)
	{
		//現在数より少ない時は、超えている分を削除

		if(max < font->listCache.num)
			mListDeleteBottomNum(&font->listCache, font->listCache.num - max);

		font->cacheMax = max;
	}
}


//=================================
// 描画サブ
//=================================


/** 文字コードからビットマップグリフ取得
 *
 * @return NULL で失敗 */

FT_BitmapGlyph _getBitmapGlyph(
	FT_Library lib,aoFont *font,uint32_t code,uint32_t flags)
{
	FT_Face face = font->face;
	mFreeTypeInfo *info = &font->info;
	FT_UInt gindex;
	FT_Glyph glyph;
	FT_Matrix matrix;
	uint32_t loadflags;

	//キャッシュから取得

	glyph = (FT_Glyph)_getGlyphCache(font, code, flags);

	if(glyph) return (FT_BitmapGlyph)glyph;

	//-------- 
	
	//コードからグリフインデックス取得
	
	gindex = FT_Get_Char_Index(face, code);
	if(gindex == 0) return NULL;

	//縦書きグリフに置換

	if(flags & GETGLYPH_F_VERT)
		gindex = mFTGSUB_getVertGlyph(font->vertTbl, gindex);
	
	//グリフスロットにロード

	loadflags = info->fLoadGlyph;
	
	if((flags & GETGLYPH_F_VERT) && font->hasVert)
		loadflags |= FT_LOAD_VERTICAL_LAYOUT;
	
	if(FT_Load_Glyph(face, gindex, info->fLoadGlyph))
		return NULL;
	
	//グリフのコピー取得
	
	if(FT_Get_Glyph(face->glyph, &glyph))
		return NULL;
	
	//太字変換
	
	if(info->flags & MFTINFO_F_EMBOLDEN)
	{
		if(glyph->format == FT_GLYPH_FORMAT_OUTLINE)
			//アウトライン
			FT_Outline_Embolden(&((FT_OutlineGlyph)glyph)->outline, 1<<6);
		else if(glyph->format == FT_GLYPH_FORMAT_BITMAP)
			//ビットマップ
			FT_Bitmap_Embolden(lib, &((FT_BitmapGlyph)glyph)->bitmap, 1<<6, 0);
	}

	//matrix

	if(glyph->format == FT_GLYPH_FORMAT_OUTLINE)
	{
		//斜体
		
		if(info->flags & MFTINFO_F_MATRIX)
			FT_Outline_Transform(&((FT_OutlineGlyph)glyph)->outline, &info->matrix);

		//90度回転

		if(flags & GETGLYPH_F_ROTATE)
		{
			matrix.xx = matrix.yy = 0;
			matrix.xy = 1<<16, matrix.yx = -1<<16;

			FT_Outline_Transform(&((FT_OutlineGlyph)glyph)->outline, &matrix);
		}
	}
	
	//ビットマップに変換
	
	if(glyph->format != FT_GLYPH_FORMAT_BITMAP)
	{
		if(FT_Glyph_To_Bitmap(&glyph, info->nRenderMode, NULL, TRUE))
		{
			FT_Done_Glyph(glyph);
			return NULL;
		}
	}

	//キャッシュに追加

	_appendCache(font, code, flags, (FT_BitmapGlyph)glyph);
	
	return (FT_BitmapGlyph)glyph;
}

/** 1文字描画 */

static void _drawChar(aoFont *font,mPixbuf *img,
	FT_BitmapGlyph glyph,int x,int y,mRgbCol col)
{
	FT_Bitmap *bm;
	uint8_t *pbuf,*pb,r,g,b,fBGR;
	int ix,iy,w,h,pitch,pitch2,f;
	uint32_t dcol;

	if(!font) return;
	
	bm = &glyph->bitmap;
	
	w = bm->width;
	h = bm->rows;
	pbuf  = bm->buffer;
	pitch = bm->pitch;
	
	if(pitch < 0) pbuf += -pitch * (h - 1);
	
	fBGR = ((font->info.flags & MFTINFO_F_SUBPIXEL_BGR) != 0);
	
	//
	
	if(bm->pixel_mode == FT_PIXEL_MODE_MONO)
	{
		//1bit モノクロ

		dcol = mRGBtoPix(col);
		
		for(iy = 0; iy < h; iy++, pbuf += pitch)
		{
			for(ix = 0, f = 0x80, pb = pbuf; ix < w; ix++)
			{
				if(*pb & f)
					mPixbufSetPixel(img, x + ix, y + iy, dcol);
				
				f >>= 1;
				if(!f) { f = 0x80; pb++; }
			}
		}
	}
	else if(bm->pixel_mode == FT_PIXEL_MODE_GRAY)
	{
		//8bit グレイスケール
		
		pitch -= w;

		for(iy = 0; iy < h; iy++, pbuf += pitch)
		{
			for(ix = 0; ix < w; ix++, pbuf++)
			{
				if(*pbuf)
				{
					dcol = mFreeTypeBlendColorGray(
						mPixbufGetPixelRGB(img, x + ix, y + iy), col, *pbuf);

					mPixbufSetPixel(img, x + ix, y + iy, mRGBtoPix(dcol));
				}
			}
		}
	}
	else if(bm->pixel_mode == FT_PIXEL_MODE_LCD)
	{
		//LCD
		
		pitch -= w;
		w /= 3;
		
		for(iy = 0; iy < h; iy++, pbuf += pitch)
		{
			for(ix = 0; ix < w; ix++, pbuf += 3)
			{
				if(fBGR)
					r = pbuf[2], g = pbuf[1], b = pbuf[0];
				else
					r = pbuf[0], g = pbuf[1], b = pbuf[2];
				
				if(r || g || b)
				{
					dcol = mFreeTypeBlendColorLCD(
						mPixbufGetPixelRGB(img, x + ix, y + iy), col, r, g, b);

					mPixbufSetPixel(img, x + ix, y + iy, mRGBtoPix(dcol));
				}
			}
		}
	}
	else if(bm->pixel_mode == FT_PIXEL_MODE_LCD_V)
	{
		//LCD Vertical
		
		pitch2 = pitch * 3 - w;
		h /= 3;
		
		for(iy = 0; iy < h; iy++, pbuf += pitch2)
		{
			for(ix = 0; ix < w; ix++, pbuf++)
			{
				if(fBGR)
					r = pbuf[pitch << 1], g = pbuf[pitch], b = pbuf[0];
				else
					r = pbuf[0], g = pbuf[pitch], b = pbuf[pitch << 1];
				
				if(r || g || b)
				{
					dcol = mFreeTypeBlendColorLCD(
						mPixbufGetPixelRGB(img, x + ix, y + iy), col, r, g, b);

					mPixbufSetPixel(img, x + ix, y + iy, mRGBtoPix(dcol));
				}
			}
		}
	}
}

/** １文字描画 (1bit で横組み文字の場合) */

static void _drawChar1bitRot(aoFont *font,mPixbuf *img,
	FT_BitmapGlyph glyph,int x,int y,mRgbCol col)
{
	FT_Bitmap *bm;
	uint8_t *pbuf,*pb;
	int ix,iy,w,h,pitch,f;
	
	if(!font) return;

	bm = &glyph->bitmap;
	
	w = bm->width;
	h = bm->rows;
	pbuf  = bm->buffer;
	pitch = bm->pitch;
	
	if(pitch < 0) pbuf += -pitch * (h - 1);
		
	col = mRGBtoPix(col);

	x -= h;

	for(ix = h - 1; ix >= 0; ix--, pbuf += pitch)
	{
		for(iy = 0, f = 0x80, pb = pbuf; iy < w; iy++)
		{
			if(*pb & f)
				mPixbufSetPixel(img, x + ix, y + iy, col);
			
			f >>= 1;
			if(!f) { f = 0x80; pb++; }
		}
	}
}


//=================================
// テキスト描画
//=================================


/** テキスト描画
 *
 * x,y は文字の左上位置 */

void aoFontDrawText(aoFont *font,mPixbuf *img,int x,int y,
	const uint32_t *text,int len,mRgbCol col,uint32_t flags)
{
	FT_BitmapGlyph glyph;
	
	if(!font || !img) return;
	
#ifdef FT_CONFIG_OPTION_SUBPIXEL_RENDERING
	FT_Library_SetLcdFilter(g_ftlib, font->info.nLCDFilter);
#endif

	if(flags & AOFONT_DRAWTEXT_F_VERT_ROTATE)
	{
		//横組み (回転して縦書き)

		x += font->vertHeight - font->baseline;

		for(; len > 0; text++, len--)
		{
			glyph = _getBitmapGlyph(g_ftlib, font, *text, GETGLYPH_F_ROTATE);
			if(!glyph) continue;

			if(glyph->bitmap.pixel_mode == FT_PIXEL_MODE_MONO)
				_drawChar1bitRot(font, img, glyph, x + glyph->top, y + glyph->left, col);
			else
				_drawChar(font, img, glyph, x + glyph->left, y - glyph->top, col);

			y += glyph->root.advance.x >> 16;
		}
	}
	else if(flags & AOFONT_DRAWTEXT_F_VERT)
	{
		//縦書き

		y += font->baseline;

		for(; len > 0; text++, len--)
		{
			glyph = _getBitmapGlyph(g_ftlib, font, *text, GETGLYPH_F_VERT);
			if(!glyph) continue;
			
			_drawChar(font, img, glyph,
				x + ((flags & AOFONT_DRAWTEXT_F_NO_LEFT)? 0: glyph->left),
				y - glyph->top, col);
			
			y += font->vertHeight;
		}
	}
	else
	{
		//横書き

		y += font->baseline;

		for(; len > 0; text++, len--)
		{
			glyph = _getBitmapGlyph(g_ftlib, font, *text, 0);
			if(!glyph) continue;
			
			_drawChar(font, img, glyph, x + glyph->left, y - glyph->top, col);
			
			x += glyph->root.advance.x >> 16;
		}
	}
}

/** テキスト描画 (ASCII 文字、横書き) */

void aoFontDrawText8(aoFont *font,mPixbuf *img,int x,int y,
	const uint8_t *text,int len,mRgbCol col)
{
	FT_BitmapGlyph glyph;
	int i;
	
	if(!font || !img) return;
		
#ifdef FT_CONFIG_OPTION_SUBPIXEL_RENDERING
	FT_Library_SetLcdFilter(g_ftlib, font->info.nLCDFilter);
#endif
	
	y += font->baseline;

	for(i = 0; *text; text++, i++)
	{
		if(len >= 0 && i >= len) break;
	
		glyph = _getBitmapGlyph(g_ftlib, font, *text, 0);
		if(!glyph) continue;
		
		_drawChar(font, img, glyph, x + glyph->left, y - glyph->top, col);
		
		x += glyph->root.advance.x >> 16;
	}
}


//=================================
// テキスト幅描画
//=================================


/** テキスト幅取得 (横書き/横組み) */

int aoFontGetTextWidth(aoFont *font,const uint32_t *text,int len)
{
	FT_BitmapGlyph glyph;
	int w = 0;
	
	if(!font) return 0;

#ifdef FT_CONFIG_OPTION_SUBPIXEL_RENDERING
	FT_Library_SetLcdFilter(g_ftlib, font->info.nLCDFilter);
#endif

	for(; len > 0; text++, len--)
	{
		glyph = _getBitmapGlyph(g_ftlib, font, *text, 0);

		if(glyph)
			w += glyph->root.advance.x >> 16;
	}

	return w;
}

/** テキスト幅取得 (ASCII 文字列) */

int aoFontGetTextWidth8(aoFont *font,const uint8_t *text,int len)
{
	FT_BitmapGlyph glyph;
	int w = 0,i;
	
	if(!font) return 0;
	
#ifdef FT_CONFIG_OPTION_SUBPIXEL_RENDERING
	FT_Library_SetLcdFilter(g_ftlib, font->info.nLCDFilter);
#endif
	
	for(i = 0; *text; text++, i++)
	{
		if(len >= 0 && i >= len) break;
	
		glyph = _getBitmapGlyph(g_ftlib, font, *text, 0);

		if(glyph)
			w += glyph->root.advance.x >> 16;
	}

	return w;
}

