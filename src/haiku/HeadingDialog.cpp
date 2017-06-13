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

#include "HeadingDialog.h"

#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>
#include <String.h>
#include <StringItem.h>
#include <View.h>

#include "common.h"

#include "mDef.h"
#include "mGui.h"
#include "mStr.h"

#include "globaldata.h"
#include "aoLayout.h"

#include <string>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"

enum DialogResult
{
	DIALOG_OK = 'btok',
	DIALOG_CANCEL = 'btcl',
};

#define SELECT_ITEM 'seli'

class StringWithPageNoItem : public BStringItem
{
public:
	StringWithPageNoItem(const char *label, int32 pageno);
	
	int32 PageNo() { return fPageNo; };
	
private:
	int32 fPageNo;
};

StringWithPageNoItem::StringWithPageNoItem(const char* label, int32 pageno)
	: BStringItem(label),
	  fPageNo(pageno)
{
}


HeadingDialog::HeadingDialog(BWindow *owner)
	: BWindow(
		BRect(0, 0, 330, 400),
		B_TRANSLATE("Heading"),
		B_FLOATING_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE | 
		B_CLOSE_ON_ESCAPE),
	  fOwner(owner),
	  fInit(false)
{
	fList = new BListView("list");
	fList->SetInvocationMessage(new BMessage(DIALOG_OK));
	BButton *okButton = new BButton("ok", B_TRANSLATE("OK"),
								new BMessage(DIALOG_OK));
	okButton->MakeDefault(true);
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6)
		.Add(new BScrollView("scroll", fList, 0, false, true))
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(okButton)
			.Add(new BButton("cancel", B_TRANSLATE("Cancel"),
					new BMessage(DIALOG_CANCEL)))
		.End();
}

HeadingDialog::~HeadingDialog()
{
}

void HeadingDialog::Quit()
{
	fOwner->PostMessage(A_HEADING_DIALOG_CLOSED);
	BWindow::Quit();
}

void HeadingDialog::ShowDialog(BRect parent)
{
	if (!fInit) {
		_SetList();
	}
	CenterIn(parent);
	Show();
}

void HeadingDialog::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case DIALOG_OK:
		{
			int32 selected = fList->CurrentSelection(0);
			if (selected >= 0) {
				StringWithPageNoItem *item = 
						dynamic_cast<StringWithPageNoItem *>(
							fList->ItemAt(selected));
				if (item != NULL) {
					BMessage mess(A_GOTO_PAGE);
					mess.AddInt32("value", item->PageNo() + 1); // hack
					fOwner->PostMessage(&mess);
				}
			}
			Hide();
			Quit();
			break;
		}
		case DIALOG_CANCEL:
		{
			Hide();
			Quit();
			break;
		}
		case CLEAR_LIST:
		{
			fInit = false;
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

void HeadingDialog::WindowActivated(bool active)
{
	if (active) {
		fList->MakeFocus(true);
	}
}

void HeadingDialog::_SetList()
{
	// clear list
	fList->MakeEmpty();
	
	if (GDAT->layout->listTitle.num <= 0) {
		return;
	}
	
	int toplevel;
	
	for (toplevel = 0; toplevel < 3 && GDAT->layout->titleNum[toplevel] == 0; toplevel++);
	
	BString *str = new BString();
	AO_TITLEINFO *pi;
	for (pi = (AO_TITLEINFO *)GDAT->layout->listTitle.top; pi; pi = (AO_TITLEINFO *)pi->i.next)
	{
		str->SetTo("");
		for(int i = pi->type - toplevel; i > 0; i--) {
			str->Append("  ", 2);
		}
		str->Append(pi->text);
		
		fList->AddItem(new StringWithPageNoItem(str->String(), pi->pageno));
	}
	delete str;
	
	fList->Select(0, false);
}
