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

#include "mDef.h"

#include "app.h"
#include "common.h"

#include "globaldata.h"


extern "C" {

void mfShowBookmarkWin()
{
	be_app->PostMessage(A_SHOW_BOOKMARK_WIN);
}

void mfMessageBoxErrTr(uint16_t groupid, uint16_t strid)
{
	BMessage msg(A_MESSAGE_ERROR);
	msg.AddInt16("groupid", (int16)groupid);
	msg.AddInt16("strid", (int16)strid);
	be_app->PostMessage(&msg);
}

void mfUpdate()
{
	be_app->PostMessage(A_UPDATE_WINDOW);
}

void mfWindowResize(int w, int h)
{
	BMessage msg(A_RESIZE_WINDOW);
	msg.AddRect("rect", BRect(0, 0, w - 1, h - 1));
	be_app->PostMessage(&msg);
}

void mfWindowSetTitle(const char *title)
{
	BMessage msg(A_SET_TITLE);
	msg.AddString("title", title);
	be_app->PostMessage(&msg);
}

void mfUpdateRecentFileMenu()
{
	be_app->PostMessage(A_UPDATE_RECENT_FILE_MENU);
}

void mfAppRun(mPoint *pt, mBool show_bkmark, const char *name, int recent)
{
	AoBook *app = new AoBook(BPoint(pt->x, pt->y));
	if (name != NULL) {
		app->OpenFile(name);
	} else if (recent >= 0) {
		app->OpenRecentFile(recent);
	}
	if (show_bkmark) {
		app->ShowBookmark();
	}
	app->Run();
	// todo, way to call save config before deleting app, callback func?
	delete app;
}

} // extern "C"
