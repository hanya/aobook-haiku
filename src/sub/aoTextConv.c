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

/***********************************
 * テキストを内部データに変換
 ***********************************/

#include <string.h>

#include "_aotextconv.h"


//--------------------

enum
{
	//傍点
	AO_CMD_TMP_BOUTEN_GOMA = 200,
	AO_CMD_TMP_BOUTEN_SIROGOMA,
	AO_CMD_TMP_BOUTEN_JYANOME,
	AO_CMD_TMP_BOUTEN_MARU,
	AO_CMD_TMP_BOUTEN_SIROMARU,
	AO_CMD_TMP_BOUTEN_KUROSANKAKU,
	AO_CMD_TMP_BOUTEN_SIROSANKAKU,
	AO_CMD_TMP_BOUTEN_NIJUUMARU,
	AO_CMD_TMP_BOUTEN_BATU,

	//傍線
	AO_CMD_TMP_BOUSEN_NORMAL = 220,
	AO_CMD_TMP_BOUSEN_DOUBLE,
	AO_CMD_TMP_BOUSEN_KUSARI,
	AO_CMD_TMP_BOUSEN_HASEN,
	AO_CMD_TMP_BOUSEN_NAMISEN,

	//見出し
	AO_CMD_TMP_TITLE1 = 230,
	AO_CMD_TMP_TITLE2,
	AO_CMD_TMP_TITLE3
};

//--------------------

//改...

const static AO_MATCHCMD g_match_kai[] = {
	{L"丁", AO_CMD_KAITYO},
	{L"ページ", AO_CMD_KAIPAGE},
	{L"頁", AO_CMD_KAIPAGE},
	{L"見開き", AO_CMD_KAIMIHIRAKI},
	{0,0}
};

//「...」...

const static AO_MATCHCMD g_match_kakko[] = {
	{L"は縦中横", AO_CMD_TATETYUYOKO_START},
	{L"は横組み", AO_CMD_YOKOGUMI_START},
	{L"は太字", AO_CMD_BOLD_START},

	{L"は大見出し", AO_CMD_TMP_TITLE1},
	{L"は中見出し", AO_CMD_TMP_TITLE2},
	{L"は小見出し", AO_CMD_TMP_TITLE3},

	{L"に傍点", AO_CMD_TMP_BOUTEN_GOMA},
	{L"に白ゴマ傍点", AO_CMD_TMP_BOUTEN_SIROGOMA},
	{L"に丸傍点", AO_CMD_TMP_BOUTEN_MARU},
	{L"に白丸傍点", AO_CMD_TMP_BOUTEN_SIROMARU},
	{L"に黒三角傍点", AO_CMD_TMP_BOUTEN_KUROSANKAKU},
	{L"に白三角傍点", AO_CMD_TMP_BOUTEN_SIROSANKAKU},
	{L"に二重丸傍点", AO_CMD_TMP_BOUTEN_NIJUUMARU},
	{L"に蛇の目傍点", AO_CMD_TMP_BOUTEN_JYANOME},
	{L"にばつ傍点", AO_CMD_TMP_BOUTEN_BATU},

	{L"に傍線", AO_CMD_TMP_BOUSEN_NORMAL},
	{L"に二重傍線", AO_CMD_TMP_BOUSEN_DOUBLE},
	{L"に鎖線", AO_CMD_TMP_BOUSEN_KUSARI},
	{L"に破線", AO_CMD_TMP_BOUSEN_HASEN},
	{L"に波線", AO_CMD_TMP_BOUSEN_NAMISEN},
	{0,0}
};

//ここから...

const static AO_MATCHCMD g_match_kokokara[] = {
	{L"横組み", AO_CMD_YOKOGUMI_START},
	{L"*字下げ", AO_CMD_JISAGE_START},
	{L"*字下げ、折り返して*字下げ", AO_CMD_JISAGE_WRAP_START},
	{L"改行天付き、折り返して*字下げ", AO_CMD_JISAGE_TEN_WRAP_START},
	{L"地付き", AO_CMD_JITSUKI_START},
	{L"地から*字上げ", AO_CMD_JITSUKI_START},
	{L"太字", AO_CMD_BOLD_START},
	{L"大見出し", AO_CMD_TMP_TITLE1},
	{L"中見出し", AO_CMD_TMP_TITLE2},
	{L"小見出し", AO_CMD_TMP_TITLE3},
	{0,0}
};

//ここで...

const static AO_MATCHCMD g_match_kokode[] = {
	{L"横組み終わり", AO_CMD_YOKOGUMI_END},
	{L"字下げ終わり", AO_CMD_JISAGE_END},
	{L"地付き終わり", AO_CMD_JITSUKI_END},
	{L"字上げ終わり", AO_CMD_JITSUKI_END},
	{L"太字終わり", AO_CMD_BOLD_END},
	{L"大見出し終わり", AO_CMD_TITLE_END},
	{L"中見出し終わり", AO_CMD_TITLE_END},
	{L"小見出し終わり", AO_CMD_TITLE_END},
	{0,0}
};

//ほか

const static AO_MATCHCMD g_match_etc[] = {
	{L"*字下げ", AO_CMD_JISAGE_LINE},
	{L"縦中横", AO_CMD_TATETYUYOKO_START},
	{L"縦中横終わり", AO_CMD_TATETYUYOKO_END},
	{L"横組み", AO_CMD_YOKOGUMI_START},
	{L"横組み終わり", AO_CMD_YOKOGUMI_END},
	{L"太字", AO_CMD_BOLD_START},
	{L"太字終わり", AO_CMD_BOLD_END},
	{L"地付き", AO_CMD_JITSUKI_LINE},
	{L"地から*字上げ", AO_CMD_JITSUKI_LINE},

	{L"大見出し", AO_CMD_TMP_TITLE1},
	{L"中見出し", AO_CMD_TMP_TITLE2},
	{L"小見出し", AO_CMD_TMP_TITLE3},
	{L"大見出し終わり", AO_CMD_TITLE_END},
	{L"中見出し終わり", AO_CMD_TITLE_END},
	{L"小見出し終わり", AO_CMD_TITLE_END},

	{L"傍点", AO_CMD_TMP_BOUTEN_GOMA},
	{L"白ゴマ傍点", AO_CMD_TMP_BOUTEN_SIROGOMA},
	{L"丸傍点", AO_CMD_TMP_BOUTEN_MARU},
	{L"白丸傍点", AO_CMD_TMP_BOUTEN_SIROMARU},
	{L"黒三角傍点", AO_CMD_TMP_BOUTEN_KUROSANKAKU},
	{L"白三角傍点", AO_CMD_TMP_BOUTEN_SIROSANKAKU},
	{L"二重丸傍点", AO_CMD_TMP_BOUTEN_NIJUUMARU},
	{L"蛇の目傍点", AO_CMD_TMP_BOUTEN_JYANOME},
	{L"ばつ傍点", AO_CMD_TMP_BOUTEN_BATU},

	{L"傍点終わり", AO_CMD_BOUTEN_END},
	{L"白ゴマ傍点終わり", AO_CMD_BOUTEN_END},
	{L"丸傍点終わり", AO_CMD_BOUTEN_END},
	{L"白丸傍点終わり", AO_CMD_BOUTEN_END},
	{L"黒三角傍点終わり", AO_CMD_BOUTEN_END},
	{L"白三角傍点終わり", AO_CMD_BOUTEN_END},
	{L"二重丸傍点終わり", AO_CMD_BOUTEN_END},
	{L"蛇の目傍点終わり", AO_CMD_BOUTEN_END},
	{L"ばつ傍点終わり", AO_CMD_BOUTEN_END},

	{L"傍線", AO_CMD_TMP_BOUSEN_NORMAL},
	{L"二重傍線", AO_CMD_TMP_BOUSEN_DOUBLE},
	{L"鎖線", AO_CMD_TMP_BOUSEN_KUSARI},
	{L"破線", AO_CMD_TMP_BOUSEN_HASEN},
	{L"波線", AO_CMD_TMP_BOUSEN_NAMISEN},

	{L"傍線終わり", AO_CMD_BOUSEN_END},
	{L"二重傍線終わり", AO_CMD_BOUSEN_END},
	{L"鎖線終わり", AO_CMD_BOUSEN_END},
	{L"破線終わり", AO_CMD_BOUSEN_END},
	{L"波線終わり", AO_CMD_BOUSEN_END},

	{L"ページの左右中央", AO_CMD_PAGE_CENTER},
	{0,0}
};

//--------------------


//================================
// ［＃...］ のサブ
//================================


/** ［＃...］ の判定
 *
 * @return TRUE でコマンドを追加 */

static mBool _matchCommand(AO_CONVERT *dat,uint32_t *p,AO_CMDDAT *dst)
{
	const AO_MATCHCMD *array = NULL;
	uint32_t c;
	mBool ret = FALSE,no_enter = FALSE;

	//結果を初期化

	memset(dst, 0, sizeof(AO_CMDDAT));

	dst->val1 = 255;
	dst->val2 = 255;

	//先頭文字から判断して各処理
	/* array が NULL 以外で、配列データから検索。
	 * array == NULL && ret != 0 で検索済み。 */

	c = *(p++);

	switch(c)
	{
		//改...
		case L'改':
			array = g_match_kai;
			no_enter = TRUE;
			break;

		//「*」...
		case L'「':
			if(aocMatchCmdKakko(p, g_match_kakko, dst))
			{
				/*「」の中の文字列はテキストの通常文字なので、
				 * 中間データの末尾が同じ文字列かどうか比較し、
				 * dst->insert に先頭位置をセット。 */
				
				if(aocFindTmpTextBottom(dat, dst))
					ret = TRUE;
			}
			break;

		//ここから.../ここで...
		case L'こ':
			if(aocCompareString(p, L"こから"))
			{
				array = g_match_kokokara;
				no_enter = TRUE;

				p += 3;
			}
			else if(aocCompareString(p, L"こで"))
			{
				array = g_match_kokode;
				no_enter = TRUE;

				p += 2;
			}
			break;

		//ほか
		default:
			p--;
			array = g_match_etc;
			break;
	}

	//次の改行を無視

	if(no_enter)
		dst->flags |= AO_CMDDAT_F_NOENTER;

	//

	if(!array && ret)
		//処理済み
		return TRUE;
	else if(array && aocMatchCmd(p, array, dst))
		//通常マッチ
		return TRUE;
	else
		//どれともマッチしない場合は、挿絵のマッチ
		return aocMatchCmdPicture(p, dst);
}

/** 中間データにコマンド追加 */

static void _appendCommand(AO_CONVERT *dat,AO_CMDDAT *p)
{
	//----- 各コマンドごとの調整
	/* レイアウト時は値 = 0 で状態なしを意味するので、+1 する */

	if(p->cmd == AO_CMD_JITSUKI_LINE || p->cmd == AO_CMD_JITSUKI_START)
	{
		//地付き or 地からｎ字上げ : 字上げ数は +1

		if(p->val1 == 255)
			p->val1 = 1;
		else
			(p->val1)++;
	}
	else if(p->cmd >= AO_CMD_TMP_BOUTEN_GOMA && p->cmd <= AO_CMD_TMP_BOUTEN_BATU)
	{
		//傍点
	
		p->val1 = p->cmd - AO_CMD_TMP_BOUTEN_GOMA + 1;
		p->cmd  = AO_CMD_BOUTEN_START;
	}
	else if(p->cmd >= AO_CMD_TMP_BOUSEN_NORMAL && p->cmd <= AO_CMD_TMP_BOUSEN_NAMISEN)
	{
		//傍線

		p->val1 = p->cmd - AO_CMD_TMP_BOUSEN_NORMAL + 1;
		p->cmd  = AO_CMD_BOUSEN_START;
	}
	else if(p->cmd >= AO_CMD_TMP_TITLE1 && p->cmd <= AO_CMD_TMP_TITLE3)
	{
		//見出し

		p->val1 = p->cmd - AO_CMD_TMP_TITLE1 + 1;
		p->cmd  = AO_CMD_TITLE_START;
	}

	//------ 中間データ追加

	if(!p->insert)
		//通常
		aocAppendTmpCommand(dat, p, FALSE);
	else
	{
		/* ［＃「...」...］ 用。
		 * cmd->insert のデータの前にコマンドを挿入し、cmd+1 のコマンドを末尾に追加 */
	
		aocAppendTmpCommand(dat, p, TRUE);

		p->cmd += 1;
		p->val1 = 255;
		aocAppendTmpCommand(dat, p, FALSE);
	}
}


//================================
// 各注記処理
//================================
/* [戻り値]
 * 文字列が正しくない場合、
 * FALSE で返すと通常文字として追加、
 * TRUE で返すと通常文字には追加せず無視する。 */


/** ［＃...］ の処理 */

static mBool _procCommand(AO_CONVERT *dat,uint32_t **pptop,mBool *no_enter)
{
	uint32_t *p;
	AO_CMDDAT cmddat;
	mBool f;

	p = *pptop + 1;

	//'］' 検索

	if(!aocFindKagiClose(pptop)) return FALSE;

	//［］中の文字列判定
	//[!] マッチするものがなかった場合は全体を無視

	if(!_matchCommand(dat, p, &cmddat)) return TRUE;

	//コマンド追加

	_appendCommand(dat, &cmddat);

	//次の改行を無視
	/* ページの左右中央は強制無視 */

	f = ((cmddat.flags & AO_CMDDAT_F_NOENTER) != 0);

	if(cmddat.cmd == AO_CMD_PAGE_CENTER) f = TRUE;

	*no_enter = f;

	return TRUE;
}

/** ルビ文字列の処理 ('《' が来た時)
 *
 * @param rubyTextTop '｜'による先頭指定があった位置。1 で指定なし。 */

static mBool _procRuby(AO_CONVERT *dat,uint32_t **pptop,AO_CONVTMP *rubyTextTop)
{
	uint32_t *st,*end;

	//st = '《' の次

	st = *pptop;

	//end = '》'

	for(end = st; *end && *end != L'》'; end++);

	if(!(*end)) return FALSE;

	//ルビ文字列が空 => 通常文字とする

	if(st == end) return FALSE;

	//

	*pptop = end + 1;

	//ルビ親文字列に情報セット

	if(rubyTextTop == (AO_CONVTMP *)1)
		//範囲は自動で判定
		aocSetRubyInfoAuto(dat, st, end - st);
	else
		//'｜' から終端まで
		aocSetRubyInfo(dat, rubyTextTop, st, end - st);

	return TRUE;
}

/** ※［＃...］ の外字注記を処理
 *
 * <例>
 * ※［＃「てへん＋劣」、第3水準1-84-77］
 * ※［＃「口＋世」、U+546D、ページ数-行数］
 * ※［＃「土へん＋竒」、ページ数-行数］
 * ※［＃二の字点、1-2-22］
 */

static mBool _procGaiji(AO_CONVERT *dat,uint32_t **pptop)
{
	uint32_t *p,c;
	int lv;

	p = *pptop + 2;

	//'］' 検索

	if(!aocFindKagiClose(pptop)) return FALSE;

	//最初の '、' までスキップ (「」内は除く)

	for(lv = 0; *p; )
	{
		c = *(p++);

		if(c == L'、')
		{
			if(lv == 0) break; 
		}
		else if(c == L'「')
			lv++;
		else if(c == L'」')
		{
			if(lv) lv--;
		}
	}

	//'、' がない場合は注記全体を無効

	if(!(*p)) return TRUE;

	//----- 外字文字を番号から取得

	c = 0;

	if(*p == 'U' && p[1] == '+')
		//'U+XXXX' => Unicode 番号
		c = aocGetHexVal(p + 2);
	else if(*p >= '0' && *p <= '9')
		//'n-n-n' 面区点番号 or 'ページ数-行数'
		c = aocGetCharJISText(p);
	else if(*p == L'第')
	{
		//第3/第4水準 [例:'第3水準1-84-77']

		for(; *p && *p != L'準'; p++);

		if(*p)
			c = aocGetCharJISText(p + 1);
	}

	//通常1文字として追加

	if(c && c <= 0x10ffff)
		aocAppendTmpNormalChar(dat, c);

	return TRUE;
}


//================================
// main
//================================


/** 1行分を処理 (UTF-32 -> 中間データ) */

static void _procLine(AO_CONVERT *dat)
{
	uint32_t *p,c;
	mListItem *rubyTop = (mListItem *)1;  //1 でなし。行頭で'｜'が指定された場合は 0 となる
	mBool ret,no_enter = FALSE;

	p = (uint32_t *)dat->memLine.buf;

	while(1)
	{
		c = *(p++);
		
		if(c == 0) break;
		else if(c == 0xfeff) continue;	//BOM はスキップ

		no_enter = FALSE;

		//先頭文字から判定

		switch(c)
		{
			//注記
			case L'［':
				ret = (*p == L'＃' && _procCommand(dat, &p, &no_enter));
				break;
			//ルビ親文字列の先頭
			case L'｜':
				rubyTop = dat->listTmp.bottom;
				ret = TRUE;
				break;
			//ルビ文字列
			case L'《':
				ret = _procRuby(dat, &p, (AO_CONVTMP *)rubyTop);
				rubyTop = (mListItem *)1;
				break;
			//外字注記
			case L'※':
				ret = (*p == L'［' && p[1] == L'＃' && _procGaiji(dat, &p));
				break;
			default:
				ret = FALSE;
				break;
		}

		//通常文字として追加

		if(!ret)
			aocAppendTmpNormalChar(dat, c);
	}

	dat->match_no_enter = no_enter;
}

/** AO_CONVERT 解放 */

static void _freeCONVERT(AO_CONVERT *p,mBool failed)
{
	mFree(p->textbuf);

	if(failed) mMemAutoFree(&p->memDat);

	mMemAutoFree(&p->memLine);
}

/** UTF-16BE から内部用データに変換
 *
 * [!] textbuf のバッファは解放される */

mBool aoTextConvert(mBuf *textbuf,mBuf *dstbuf)
{
	AO_CONVERT dat;
	uint32_t size;

	//変換用データ

	memset(&dat, 0, sizeof(AO_CONVERT));

	dat.textbuf = textbuf->buf;
	dat.src    = (uint8_t *)textbuf->buf;
	dat.remain = textbuf->size;

	//各メモリ確保

	size = textbuf->size << 1;
	size = (size + 4095) & (~4095);

	if(!mMemAutoAlloc(&dat.memDat, size, 16 * 1024)
		|| !mMemAutoAlloc(&dat.memLine, 4 * 1024, 4096))
	{
		_freeCONVERT(&dat, TRUE);
		return FALSE;
	}

	//1行ごとに処理

	while(aocGetSrcLine(&dat))
	{
		_procLine(&dat);

		aocOutput(&dat);
		
		//行数

		(dat.curline)++;
	}

	mMemAutoAppendByte(&dat.memDat, AO_TYPE_END);

	//[!] memDat は解放しない

	_freeCONVERT(&dat, FALSE);

	//

	mMemAutoCutCurrent(&dat.memDat);

	dstbuf->buf  = dat.memDat.buf;
	dstbuf->size = dat.memDat.curpos;

	return TRUE;
}
