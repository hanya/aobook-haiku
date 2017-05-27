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

#ifndef MLIB_SYSCOL_H
#define MLIB_SYSCOL_H

enum MSYSCOL_INDEX
{
	MSYSCOL_WHITE,

	MSYSCOL_FACE,
	MSYSCOL_FACE_FOCUS,
	MSYSCOL_FACE_SELECT,
	MSYSCOL_FACE_SELECT_LIGHT,
	MSYSCOL_FACE_DARK,
	MSYSCOL_FACE_DARKER,
	MSYSCOL_FACE_LIGHTEST,

	MSYSCOL_FRAME,
	MSYSCOL_FRAME_FOCUS,
	MSYSCOL_FRAME_DARK,
	MSYSCOL_FRAME_LIGHT,

	MSYSCOL_TEXT,
	MSYSCOL_TEXT_REVERSE,
	MSYSCOL_TEXT_DISABLE,
	MSYSCOL_TEXT_SELECT,

	MSYSCOL_MENU_FACE,
	MSYSCOL_MENU_FRAME,
	MSYSCOL_MENU_SEP,
	MSYSCOL_MENU_SELECT,
	MSYSCOL_MENU_TEXT,
	MSYSCOL_MENU_TEXT_DISABLE,
	MSYSCOL_MENU_TEXT_SHORTCUT,

	MSYSCOL_NUM
};

extern uint32_t g_mSysCol[MSYSCOL_NUM * 2];

#define MSYSCOL(name)     g_mSysCol[MSYSCOL_ ## name]
#define MSYSCOL_RGB(name) g_mSysCol[MSYSCOL_NUM + MSYSCOL_ ## name]

#endif
