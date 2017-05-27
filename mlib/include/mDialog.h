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

#ifndef MLIB_DIALOG_H
#define MLIB_DIALOG_H

#include "mWindowDef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define M_DIALOG(p)  ((mDialog *)(p))

typedef struct
{
	intptr_t retval;
}mDialogData;

typedef struct _mDialog
{
	mWidget wg;
	mContainerData ct;
	mWindowData win;
	mDialogData dlg;
}mDialog;


int mDialogEventHandle(mWidget *wg,mEvent *ev);

mWidget *mDialogNew(int size,mWindow *owner,uint32_t style);
intptr_t mDialogRun(mDialog *p,mBool destroy);
void mDialogEnd(mDialog *p,intptr_t ret);

int mDialogEventHandle_okcancel(mWidget *wg,mEvent *ev);

#ifdef __cplusplus
}
#endif

#endif
