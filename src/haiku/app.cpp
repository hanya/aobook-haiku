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

#include "app.h"
#include "common.h"
#include "version.h"

#include <Alert.h>
#include <Catalog.h>
#include <Entry.h>

#include "trid.h"
#include "trgroup.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"


AoBook::AoBook(BPoint pos)
	: BApplication(APP_SIG)
{
	fWindow = new AoBookWindow(pos);
	fWindow->Show();
}

AoBook::~AoBook()
{
}

void AoBook::ShowBookmark()
{
	fWindow->PostMessage(A_SHOW_BOOKMARK_WIN);
}

void AoBook::OpenFile(const char *name)
{
	BEntry entry(name);
	entry_ref ref;
	entry.GetRef(&ref);
	BMessage msg(B_REFS_RECEIVED);
	msg.AddRef("refs", &ref);
	msg.AddInt32("encoding", AUTO);
	fWindow->PostMessage(&msg);
}

void AoBook::OpenRecentFile(int index)
{
	BMessage msg(A_RECENT_MENU_CHOOSE);
	msg.AddInt32("index", index);
	fWindow->PostMessage(&msg);
}

void AoBook::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case B_ABOUT_REQUESTED:
		{
			BAboutWindow *about = new BAboutWindow(APPNAME, APP_SIG);
			about->AddCopyright(2014, "Azel");
			about->Show();
			break;
		}
		case A_WINDOW_CLOSE:
		{
			Quit();
			break;
		}
		case A_SHOW_BOOKMARK_WIN:
		{
			fWindow->PostMessage(msg);
			break;
		}
		case A_MESSAGE_ERROR:
		{
			int16 groupid, strid;
			if (msg->FindInt16("groupid", &groupid) == B_OK &&
				msg->FindInt16("strid", &strid) == B_OK) {
				if (groupid == TRGROUP_MESSAGE) {
					const char *message = NULL;
					switch (strid) {
						case TRMES_ERR_LOADFILE:
							message = B_TRANSLATE("Failed to load the file.");
							break;
						case TRMES_ERR_CONVCODE:
							message = B_TRANSLATE(
								"Failed to convert the string into Unicode.");
							break;
						case TRMES_ERR_BUF:
							message = B_TRANSLATE("No memory.");
							break;
						default:
							break;
					}
					BAlert *alert = new BAlert("Error",
						message,
						B_TRANSLATE("OK"), NULL, NULL,
						B_WIDTH_AS_USUAL, B_STOP_ALERT);
					alert->SetShortcut(0, B_ESCAPE);
					alert->Go();
				}
			}
			break;
		}
		case A_UPDATE_WINDOW:
		{
			fWindow->PostMessage(msg);
			break;
		}
		case A_RESIZE_WINDOW:
		{
			fWindow->PostMessage(msg);
			break;
		}
		case A_SET_TITLE:
		{
			fWindow->PostMessage(msg);
			break;
		}
		case A_UPDATE_RECENT_FILE_MENU:
		{
			fWindow->PostMessage(msg);
			break;
		}
		default:
		{
			BApplication::MessageReceived(msg);
			break;
		}
	}
}
