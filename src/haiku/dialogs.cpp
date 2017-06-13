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

#include "dialogs.h"

#include <Button.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <TextControl.h>

#include <string>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"


enum DialogResult
{
	DIALOG_OK = 'btok',
	DIALOG_CANCEL = 'btcl',
};


InputDialog::InputDialog(BWindow *owner, 
						const char *title, const char *label, 
						InputType type, int32 command, int32 quit_command)
	: BWindow(
		BRect(100, 100, 200, 150),
		title,
		B_FLOATING_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | 
		B_AUTO_UPDATE_SIZE_LIMITS |
		B_CLOSE_ON_ESCAPE),
	  fOwner(owner),
	  fCommand(command),
	  fQuitCommand(quit_command),
	  fInputType(type)
{
	fLabel = new BStringView("label", label);
	fTextControl = new BTextControl("input", "", "", new BMessage(DATA_INPUT));
	BButton *okButton = new BButton("ok", B_TRANSLATE("OK"), 
					new BMessage(DIALOG_OK));
	okButton->MakeDefault(true);
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6, 6)
		.Add(fLabel)
		.Add(fTextControl)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(okButton)
			.Add(new BButton("cancel", B_TRANSLATE("Cancel"), 
					new BMessage(DIALOG_CANCEL)))
		.End();
	
	if (fInputType == IT_INTEGER) {
		AddCommonFilter(new BMessageFilter(B_KEY_DOWN, IntegerKeyDownFilter));
	} else if (fInputType == IT_FLOAT) {
		AddCommonFilter(new BMessageFilter(B_KEY_DOWN, FloatKeyDownFilter));
	}
}

InputDialog::~InputDialog()
{
}

void InputDialog::Quit()
{
	fOwner->PostMessage(fQuitCommand);
	BWindow::Quit();
}

void InputDialog::SetText(const char *text)
{
	fTextControl->TextView()->SetText(text);
}

void InputDialog::ShowDialog(BRect parent)
{
	CenterIn(parent);
	fTextControl->TextView()->SelectAll();
	Show();
}

void InputDialog::WindowActivated(bool active)
{
	if (active) {
		fTextControl->MakeFocus(true);
	}
}

filter_result InputDialog::IntegerKeyDownFilter(BMessage *msg, 
						BHandler **target, BMessageFilter *filter)
{
	if (msg->what == B_KEY_DOWN) {
		uint8 byte = (uint8)msg->GetInt8("byte", 0);
		if (!(('0' <= byte && byte <= '9') || byte == '.' || 
			byte == B_ENTER || byte == B_TAB || byte == B_ESCAPE)) {
			return B_SKIP_MESSAGE;
		}
	}
	return B_DISPATCH_MESSAGE;
}

filter_result InputDialog::FloatKeyDownFilter(BMessage *msg, 
						BHandler **target, BMessageFilter *filter)
{
	if (msg->what == B_KEY_DOWN) {
		uint8 byte = (uint8)msg->GetInt8("byte", 0);
		if (!(('0' <= byte && byte <= '9') || 
			byte == B_ENTER || byte == B_TAB || byte == B_ESCAPE)) {
			return B_SKIP_MESSAGE;
		}
	}
	return B_DISPATCH_MESSAGE;
}


void InputDialog::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case DIALOG_OK:
		{
			Hide();
			BMessage mess(fCommand);
			if (fInputType == InputDialog::IT_STRING) {
				mess.AddString("value", fTextControl->Text());
			} else if (fInputType == InputDialog::IT_INTEGER) {
				try {
					int32 value = std::stoi(fTextControl->Text());
					mess.AddInt32("value", value);
				} catch (...) {
				}
			} else if (fInputType == InputDialog::IT_FLOAT) {
				try {
					float value = std::stof(fTextControl->Text());
					mess.AddFloat("value", value);
				} catch (...) {
				}
			}
			fOwner->PostMessage(&mess);
			Quit();
			break;
		}
		case DIALOG_CANCEL:
		{
			Hide();
			Quit();
			break;
		}
		case SET_TITLE:
		{
			const char *title;
			if (msg->FindString("title", &title) == B_OK) {
				SetTitle(title);
			}
			break;
		}
		case SET_LABEL:
		{
			const char *label;
			if (msg->FindString("label", &label) == B_OK) {
				fLabel->SetText(label);
			}
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

