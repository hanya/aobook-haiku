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
 * メインの処理関数
 ********************************/

#ifndef MAINFUNC_H
#define MAINFUNC_H

enum
{
	PAGENO_NEXT = -1,
	PAGENO_PREV = -2,
	PAGENO_HOME = -3,
	PAGENO_END  = -4
};

#ifdef __cplusplus
extern "C" {
#endif

void mfReloadFile();
void mfLoadTextFile(const char *filename,int code,int linepage,mBool reload);
void mfLoadTextFileFromRecent(int no);
void mfLoadNextPrevFile(mBool prev);
void mfReLayout();

void mfUpdateScreen();
void mfUpdateChangeStyle(mBool relayout);
void mfSetWindowTitle();

void mfMovePage(int page);
void mfMovePageToLine(int no);

void mfExecTool(int no);

#ifdef __cplusplus
}
#endif

#endif
