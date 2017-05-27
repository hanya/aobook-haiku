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

#ifndef MLIB_INPUTACCELKEY_H
#define MLIB_INPUTACCELKEY_H

#include "mWidgetDef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_INPUTACCELKEY(p)  ((mInputAccelKey *)(p))

typedef struct
{
	char *text;
	uint32_t key,keyprev;
	mBool bKeyDown;
}mInputAccelKeyData;

typedef struct _mInputAccelKey
{
	mWidget wg;
	mInputAccelKeyData ak;
}mInputAccelKey;


enum MINPUTACCELKEY_NOTIFY
{
	MINPUTACCELKEY_N_CHANGEKEY
};


void mInputAccelKeyDestroyHandle(mWidget *p);
void mInputAccelKeyCalcHintHandle(mWidget *p);
void mInputAccelKeyDrawHandle(mWidget *p,mPixbuf *pixbuf);
int mInputAccelKeyEventHandle(mWidget *wg,mEvent *ev);

mInputAccelKey *mInputAccelKeyNew(int size,mWidget *parent,uint32_t style);

uint32_t mInputAccelKey_getKey(mInputAccelKey *p);
void mInputAccelKey_setKey(mInputAccelKey *p,uint32_t key);

#ifdef __cplusplus
}
#endif

#endif
