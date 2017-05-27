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
 * 環境設定 - 内部処理
 ********************************/

#include "mStrDef.h"
#include "globaldata.h"

typedef struct
{
	uint8_t mousectrl[MOUSECTRL_BTT_NUM];
	uint32_t optflags,
		*keybuf;
	mStr strTool[TOOLITEM_NUM],
		strGUIFont;
}EnvOptDlgEditData;

typedef struct
{
	void *data;
	void (*destroy)(void *);
	void (*event)(void *,mEvent *);
}EnvOptDlgContents;


//opt

void EnvOptOpt_create(EnvOptDlgContents *dat,mWidget *parent,EnvOptDlgEditData *edit);
void EnvOptOpt_finish(EnvOptDlgEditData *edit,mBool bOK);

//mouse

void EnvOptMouse_create(EnvOptDlgContents *dat,mWidget *parent,uint8_t *buf);
mBool EnvOptMouse_finish(uint8_t *editbuf,mBool bOK);

//key

uint32_t *EnvOptKey_createEditData();
void EnvOptKey_create(EnvOptDlgContents *dat,mWidget *parent,uint32_t **ppbuf);
mBool EnvOptKey_finish(uint32_t *editbuf,uint32_t **ppnewbuf,mBool bOK);

//tool

void EnvOptTool_create(EnvOptDlgContents *dat,mWidget *parent,mStr *array);
mBool EnvOptTool_finish(mStr *array,mBool bOK);
