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
 * メインメニューデータ
 ********************************/

#ifdef MENUDAT_EXTERN

extern const uint16_t g_menudat[];

#else

const uint16_t g_menudat[] = {
	//file
	TRMENU_TOP_FILE,
	TRMENU_FILE_OPEN, TRMENU_FILE_RELOAD, 0xfffe,
	TRMENU_FILE_NEXTFILE, TRMENU_FILE_PREVFILE, 0xfffe,
	TRMENU_FILE_RECENTFILE, 0xfffe, TRMENU_FILE_EXIT,

	//move
	TRMENU_TOP_MOVE,
	TRMENU_MOVE_NEXT, TRMENU_MOVE_PREV, 0xfffe,
	TRMENU_MOVE_TOP, TRMENU_MOVE_BOTTOM, 0xfffe,
	TRMENU_MOVE_PAGENO, TRMENU_MOVE_LINENO, 0xfffe,
	TRMENU_MOVE_CAPTION,

	//bookmark
	TRMENU_TOP_BOOKMARK,
	TRMENU_BM_LIST, 0xfffe,
	TRMENU_BM_ADD_GLOBAL, TRMENU_BM_ADD_LOCAL,

	//option
	TRMENU_TOP_OPTION,
	TRMENU_OPT_TOOL_SUB, 0xfffd,0xfffd,
	TRMENU_OPT_STYLE_SUB, 0xfffd,0xfffd,
	0xfffe,
	TRMENU_OPT_STYLE, TRMENU_OPT_ENV,

	//help
	TRMENU_TOP_HELP,
	TRMENU_HELP_ABOUT,

	0xffff
};

#endif
