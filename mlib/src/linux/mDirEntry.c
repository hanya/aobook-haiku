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
 * <Linux> mDirEntry
 *****************************************/

#include <sys/types.h>
#include <dirent.h>

#include "mDef.h"
#include "mDirEntry.h"

#include "mFileStat.h"
#include "mStr.h"
#include "mUtilFile.h"
#include "mUtilCharCode.h"


//-----------------

struct _mDirEntry
{
	DIR *dir;
	struct dirent *curent;
	mStr strPath,
		strCurName;
};

//-----------------


/**
@defgroup direntry mDirEntry
@brief ディレクトリ内ファイル操作

@ingroup group_system
@{

@file mDirEntry.h
*/


/** 閉じる */

void mDirEntryClose(mDirEntry *p)
{
	if(p)
	{
		mStrFree(&p->strPath);
		mStrFree(&p->strCurName);
	
		closedir(p->dir);

		mFree(p);
	}
}

/** ディレクトリ開く */

mDirEntry *mDirEntryOpen(const char *path)
{
	mDirEntry *p;
	char *fname;
	DIR *dir;

	//開く

	fname = mUTF8ToLocal_alloc(path, -1, NULL);
	if(!fname) return NULL;

	dir = opendir(fname);

	mFree(fname);

	if(!dir) return NULL;

	//確保

	p = (mDirEntry *)mMalloc(sizeof(mDirEntry), TRUE);
	if(!p)
	{
		closedir(dir);
		return NULL;
	}

	p->dir = dir;

	mStrSetText(&p->strPath, path);

	return p;
}

/** 次のエントリを取得 */

mBool mDirEntryRead(mDirEntry *p)
{
	struct dirent *ent;

	if(p)
	{
		ent = readdir(p->dir);
		if(ent)
		{
			p->curent = ent;
		
			mStrSetTextLocal(&p->strCurName, ent->d_name, -1);
			return TRUE;
		}
	}
	
	return FALSE;
}

/** 現在のファイル名取得 */

const char *mDirEntryGetFileName(mDirEntry *p)
{
	return p->strCurName.buf;
}

/** 現在のファイル名取得 (mStr) */

void mDirEntryGetFileName_str(mDirEntry *p,mStr *str,mBool bFullPath)
{
	if(!p)
		mStrEmpty(str);
	else if(bFullPath)
	{
		mStrCopy(str, &p->strPath);
		mStrPathAdd(str, p->strCurName.buf);
	}
	else
		mStrCopy(str, &p->strCurName);
}

/** ファイル情報取得 */

mBool mDirEntryGetStat(mDirEntry *p,mFileStat *dst)
{
	mStr str = MSTR_INIT;
	mBool ret;

	mDirEntryGetFileName_str(p, &str, TRUE);

	ret = mGetFileStat(str.buf, dst);

	mStrFree(&str);

	return ret;
}

/** 現在のファイルがディレクトリか */

mBool mDirEntryIsDirectory(mDirEntry *p)
{
	if(p && p->curent)
	{
		if(p->curent->d_type == DT_UNKNOWN)
		{
			mFileStat st;

			if(mDirEntryGetStat(p, &st))
				return ((st.flags & MFILESTAT_F_DIRECTORY) != 0);
		}
		else
			return (p->curent->d_type == DT_DIR);
	}

	return FALSE;
}

/** 現在のファイル名が特殊な名前 ("." or "..") か */

mBool mDirEntryIsSpecName(mDirEntry *p)
{
	char *pc;

	if(p && p->strCurName.buf)
	{
		pc = p->strCurName.buf;
	
		if(*pc == '.')
			return (pc[1] == 0 || (pc[1] == '.' && pc[2] == 0));
	}

	return FALSE;
}

/** 拡張子が指定文字列と同じか (大/小文字区別しない) */

mBool mDirEntryIsEqExt(mDirEntry *p,const char *ext)
{
	return (p && mStrPathCompareExtEq(&p->strCurName, ext));
}

/** @} */
