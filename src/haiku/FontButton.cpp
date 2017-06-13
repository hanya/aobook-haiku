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

#include "FontButton.h"

#include <Catalog.h>
#include <Font.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <String.h>
#include <StringView.h>
#include <TextControl.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"


enum DialogResult
{
	DIALOG_OK = 'btok',
	DIALOG_CANCEL = 'btcl',
};


FontDialog::FontDialog(BHandler *owner, int32 command, int32 quit_command)
	: BWindow(
		BRect(0, 0, 0, 0),
		B_TRANSLATE("Font"),
		B_MODAL_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | 
		B_AUTO_UPDATE_SIZE_LIMITS |
		B_CLOSE_ON_ESCAPE),
	  fOwner(owner),
	  fCommand(command),
	  fQuitCommand(quit_command)
{
	fFontPM = new BPopUpMenu("fontpm");
	fFontPM->SetRadioMode(true);
	fSizeTC = new BTextControl("sizetc", "", "", new BMessage());
	fSizeTC->SetText("12");
	
	fFontMF = new BMenuField("fontMF", "", fFontPM);
	
	fWeightPM = new BPopUpMenu("weightpm");
	fWeightPM->SetRadioMode(true);
	BLayoutBuilder::Menu<>(fWeightPM)
		.AddItem("----", new BMessage())
		.AddItem(B_TRANSLATE("Normal"), new BMessage())
		.AddItem(B_TRANSLATE("Bold"), new BMessage());
	fWeightPM->ItemAt(0)->SetMarked(true);
	BMenuField *weightMF = new BMenuField("weightFM", "", fWeightPM);
	
	fSlantPM = new BPopUpMenu("slantpm");
	fSlantPM->SetRadioMode(true);
	BLayoutBuilder::Menu<>(fSlantPM)
		.AddItem("----", new BMessage())
		.AddItem(B_TRANSLATE("Roman"), new BMessage())
		.AddItem(B_TRANSLATE("Italic"), new BMessage())
		.AddItem(B_TRANSLATE("Oblique"), new BMessage());
	fSlantPM->ItemAt(0)->SetMarked(true);
	BMenuField *slantMF = new BMenuField("slantFM", "", fSlantPM);
	
	BButton *okButton = new BButton("okbutton", B_TRANSLATE("OK"), 
						new BMessage(DIALOG_OK));
	okButton->MakeDefault(true);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6, 6, 6, 6)
		.AddGrid()
			.Add(new BStringView("fontl", B_TRANSLATE("Font")), 0, 0)
			.Add(fFontMF, 1, 0)
			.Add(new BStringView("sizel", B_TRANSLATE("Size")), 0, 1)
			.Add(fSizeTC, 1, 1)
			.Add(new BStringView("weightl", B_TRANSLATE("Weight")), 0, 2)
			.Add(weightMF, 1, 2)
			.Add(new BStringView("slantl", B_TRANSLATE("Slant")), 0, 3)
			.Add(slantMF, 1, 3)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(okButton)
			.Add(new BButton("cancell", B_TRANSLATE("Cancel"), 
					new BMessage(DIALOG_CANCEL)))
			.AddGlue()
		.End();
	
	_Init();
}

void FontDialog::ShowDialog(BRect rect)
{
	CenterIn(rect);
	Show();
}

void FontDialog::ShowDialogAt(BPoint where)
{
	MoveTo(where);
	Show();
}

void FontDialog::Quit()
{
	BMessage mess(FONT_DIALOG_CLOSED);
	fOwner->MessageReceived(&mess);
	BWindow::Quit();
}

void FontDialog::WindowActivated(bool active)
{
	if (active) {
		fFontMF->MakeFocus(true);
	} else {
		PostMessage(DIALOG_CANCEL);
	}
}

void FontDialog::SetFontFamily(const char *family)
{
	BMenuItem *item = fFontPM->FindItem(family);
	if (item != NULL) {
		BMenuItem *marked = fFontPM->FindMarked();
		if (marked != NULL) {
			marked->SetMarked(false);
		}
		item->SetMarked(true);
	}
}

void FontDialog::SetFontStyle(const char *style)
{
	BMenuItem *item = fFontPM->FindMarked();
	if (item != NULL) {
		BMenu *menu = item->Submenu();
		if (menu != NULL) {
			BMenuItem *named = menu->FindItem(style);
			if (named != NULL) {
				BMenuItem *marked = menu->FindMarked();
				if (marked != NULL) {
					marked->SetMarked(false);
				}
				named->SetMarked(true);
			}
		}
	}
}

void FontDialog::SetFontSize(int32 size)
{
	char s[32];
	sprintf(s, "%d", size);
	fSizeTC->SetText((const char *)s);
}

void FontDialog::SetFontWeight(int32 weight)
{
	int32 index = 0;
	switch (weight)
	{
		case FONT_WEIGHT_NORMAL:
			index = 1;
			break;
		case FONT_WEIGHT_BOLD:
			index = 2;
			break;
		default:
			break;
	}
	BMenuItem *item = fWeightPM->FindMarked();
	if (item != NULL) {
		item->SetMarked(false);
	}
	item = fWeightPM->ItemAt(index);
	if (item != NULL) {
		item->SetMarked(true);
	}
}

void FontDialog::SetFontSlant(int32 slant)
{
	int32 index = 0;
	switch (slant)
	{
		case FONT_SLANT_ROMAN:
			index = 1;
			break;
		case FONT_SLANT_ITALIC:
			index = 2;
			break;
		case FONT_SLANT_OBLIQUE:
			index = 3;
			break;
		default:
			break;
	}
	BMenuItem *item = fSlantPM->FindMarked();
	if (item != NULL) {
		item->SetMarked(false);
	}
	item = fSlantPM->ItemAt(index);
	if (item != NULL) {
		item->SetMarked(true);
	}
}

#define FONT_DIALOG_STYLE_CHOOSE 'fdsc'

void FontDialog::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case DIALOG_OK:
		{
			Hide();
			BMessage mess(FONT_DIALOG_CHOOSE);
			
			BMenuItem *item = fFontPM->FindMarked();
			if (item != NULL) {
				mess.AddString("family", item->Label());
				BMenu *menu = item->Submenu();
				if (menu != NULL) {
					item = menu->FindMarked();
					if (item != NULL) {
						mess.AddString("style", item->Label());
					}
				}
			}
			try {
				int32 value = std::stoi(fSizeTC->Text());
				mess.AddInt32("size", value);
			} catch (...) {
			}
			int32 selected = fWeightPM->FindMarkedIndex();
			if (selected == 1) {
				mess.AddInt32("weight", FONT_WEIGHT_NORMAL);
			} else if (selected == 2) {
				mess.AddInt32("weight", FONT_WEIGHT_BOLD);
			}
			selected = fSlantPM->FindMarkedIndex();
			if (selected == 1) {
				mess.AddInt32("slant", FONT_SLANT_ROMAN);
			} else if (selected == 2) {
				mess.AddInt32("slant", FONT_SLANT_ITALIC);
			} else if (selected == 3) {
				mess.AddInt32("slant", FONT_SLANT_OBLIQUE);
			}
			
			fOwner->MessageReceived(&mess);
			
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case DIALOG_CANCEL:
		{
			Hide();
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case FONT_DIALOG_STYLE_CHOOSE:
		{
			// If style name is choosen, mark the family.
			int32 family;
			if (msg->FindInt32("family", &family) == B_OK) {
				_ChooseStyle(family);
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

void FontDialog::_ChooseStyle(int32 family)
{
	BMenuItem *item = fFontPM->FindMarked();
	if (item != NULL) {
		item->SetMarked(false);
	}
	
	if (0 <= family && family < fFontPM->CountItems()) {
		item = fFontPM->ItemAt(family);
		if (item != NULL) {
			item->SetMarked(true);
		}
	}
}

void FontDialog::_Init()
{
	int32 families = count_font_families();
	font_family name;
	font_style style;
	
	for (int32 i = 0; i < families; ++i) {
		if (get_font_family(i, &name) == B_OK) {
			
			int32 styles = count_font_styles(name);
			if (styles == 0) {
				continue;
			}
			BMenu *stylesPM = new BPopUpMenu(name);
			stylesPM->SetRadioMode(true);
			stylesPM->SetLabelFromMarked(false);
			fFontPM->AddItem(stylesPM);
			
			for (int32 j = 0; j < styles; ++j) {
				if (get_font_style(name, j, &style) == B_OK) {
					BMessage * mess = new BMessage(FONT_DIALOG_STYLE_CHOOSE);
					mess->AddInt32("family", i);
					
					BMenuItem *item = new BMenuItem(style, mess);
					stylesPM->AddItem(item);
					if (j == 0) {
						item->SetMarked(true);
					}
				}
			}
		}
	}
}


FontButton::FontButton(const char *name, const char *label)
	: BButton(name, label, new BMessage()),
	  fFontSize(12),
	  fFontWeight(FONT_WEIGHT_NONE),
	  fFontSlant(FONT_SLANT_NONE),
	  fFontDialog(NULL)
{
	fFontFamily = "";
	fFontStyle = "";
}

FontButton::~FontButton()
{
}

const char * FontButton::FontFamily()
{
	return fFontFamily.c_str();
}

void FontButton::SetFontFamily(const char *family)
{
	bool changed = strcmp(fFontFamily.c_str(), family) == 0;
	fFontFamily = family;
	if (changed) {
		_UpdateLabel();
	}
}

const char * FontButton::FontStyle()
{
	return fFontStyle.c_str();
}

void FontButton::SetFontStyle(const char *style)
{
	fFontStyle = style;
}

int32 FontButton::FontSize()
{
	return fFontSize;
}

void FontButton::SetFontSize(int32 size)
{
	bool changed = fFontSize != size;
	fFontSize = size;
	if (changed) {
		_UpdateLabel();
	}
}

int32 FontButton::FontWeight()
{
	return fFontWeight;
}

void FontButton::SetFontWeight(int32 weight)
{
	fFontWeight = weight;
}

int32 FontButton::FontSlant()
{
	return fFontSlant;
}

void FontButton::SetFontSlant(int32 slant)
{
	fFontSlant = slant;
}

void FontButton::_UpdateLabel()
{
	std::string str;
	str.append(fFontFamily.c_str());
	str.append(" | ");
	str.append(std::to_string(fFontSize));
	
	LockLooper();
	BButton::SetLabel(str.c_str());
	UnlockLooper();
}

status_t FontButton::Invoke(BMessage *msg)
{
	Sync();
	_ShowPicker();
	SetValue(B_CONTROL_OFF);
	return B_OK;
}

void FontButton::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case FONT_DIALOG_CHOOSE:
		{
			const char *family;
			if (msg->FindString("family", &family) == B_OK) {
				SetFontFamily(family);
			}
			const char *style;
			if (msg->FindString("style", &style) == B_OK) {
				SetFontStyle(style);
			}
			int32 n;
			if (msg->FindInt32("size", &n) == B_OK) {
				SetFontSize(n);
			}
			if (msg->FindInt32("weight", &n) == B_OK) {
				SetFontWeight(n);
			}
			if (msg->FindInt32("slant", &n) == B_OK) {
				SetFontSlant(n);
			}
			break;
		}
		case FONT_DIALOG_CLOSED:
		{
			fFontDialog = NULL;
			break;
		}
		default:
		{
			BButton::MessageReceived(msg);
			break;
		}
	}
}

void FontButton::_ShowPicker()
{
	if (fFontDialog == NULL) {
		fFontDialog = new FontDialog(this);
		fFontDialog->SetFontFamily(fFontFamily.c_str());
		fFontDialog->SetFontStyle(fFontStyle.c_str());
		fFontDialog->SetFontSize(fFontSize);
		fFontDialog->SetFontWeight(fFontWeight);
		fFontDialog->SetFontSlant(fFontSlant);
		
		BPoint where = ConvertToScreen(Bounds()).LeftBottom();
		where.x += 5;
		where.y += 5;
		fFontDialog->ShowDialogAt(where);
	} else {
		fFontDialog->Activate();
	}
}
