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

#ifndef COMMON_H_
#define COMMON_H_

#include "trid_menu.h"

enum {
	CMD_RECENT = 'aorc',
	CMD_EXIT = 'aoex',
	
	CMD_BOOKMARK_ADD_TO_GLOBAL = 'aobg',
	CMD_BOOKMARK_ADD_TO_LOCAL = 'aobl',
	
	CMD_TOOL = 'tl',
	CMD_STYLES = 'aosy',
	
	CMD_HELP_MANUAL = 'aohm',
};


enum Encoding {
	AUTO = 0,
	SHIFT_JIS,
	EUC_JP,
	UTF_8,
	UTF_16LE,
	UTF_16BE,
	
	ENCODING_END,
};


enum {
	A_WINDOW_CLOSE = 'aowc',
	A_SHOW_BOOKMARK_WIN = 'aosb',
	A_MESSAGE_ERROR = 'aoer',
	A_UPDATE_WINDOW = 'aoup',
	A_RESIZE_WINDOW = 'aorw',
	A_SET_TITLE = 'aosi',
	A_UPDATE_RECENT_FILE_MENU = 'aour',
	A_LOAD_FILE = 'aolf',
	
	A_GOTO_PAGE = 'agpg',
	A_GOTO_LINE = 'agln',
	A_RECENT_MENU_CHOOSE = 'agrm',
	A_STYLE_MENU_CHOOSE = 'asmc',
	A_STYLE_MENU_UPDAGE = 'asmu',
	A_STYLE_UPDATE = 'asup',
	A_UPDATE_TOOL_MENU = 'atmc',
	
	A_BOOKMARK_TAB_CHANGED = 'abkt',
	A_ACC_KEYS_CHANGED = 'abac',
	
	A_HEADING_DIALOG_CLOSED = 'ahdc',
	A_PAGE_DIALOG_CLOSED = 'apdc',
	A_LINE_DIALOG_CLOSED = 'aldc',
	A_STYLE_OPT_DIALOG_CLOSED = 'asdc',
	A_ENV_OPT_DIALOG_CLOSED = 'aedc',
	A_BOOKMARK_DIALOG_CLOSED = 'abdc',
	
};

#endif
