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

#include "ColorButton.h"

#include <Button.h>
#include <Catalog.h>
#include <ColorControl.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <LayoutUtils.h>
#include <Window.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"


ColorPickerDialog::ColorPickerDialog(BHandler *owner, rgb_color currentColor)
	: BWindow(
		BRect(0, 0, 0, 0),
		B_TRANSLATE("Color picker"), // todo, way to change title
		B_MODAL_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS |
		B_CLOSE_ON_ESCAPE),
	  fOwner(owner),
	  fCurrentColor(currentColor)
{
	fColorControl = new BColorControl(BPoint(0, 0), B_CELLS_32x8, 7.0,
					"ColorControl", new BMessage('clch'));
	fColorControl->SetValue(fCurrentColor);
	
	// todo, preview?
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fColorControl)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(new BButton("ok", B_TRANSLATE("OK"),
				new BMessage(DIALOG_OK)))
			.Add(new BButton("cancel", B_TRANSLATE("Cancel"),
				new BMessage(DIALOG_CANCEL)))
		.End();
}

ColorPickerDialog::~ColorPickerDialog()
{
}

void ColorPickerDialog::WindowActivated(bool active)
{
	if (!active) {
		PostMessage(DIALOG_CANCEL);
	}
}

void ColorPickerDialog::ShowDialog(BRect parent)
{
	CenterIn(parent);
	Show();
}

void ColorPickerDialog::ShowDialogAt(BPoint where)
{
	MoveTo(where);
	Show();
}

rgb_color ColorPickerDialog::Color()
{
	return fNewColor;
}

void ColorPickerDialog::SetColor(rgb_color color)
{
	fNewColor = color;
}

rgb_color ColorPickerDialog::CurrentColor()
{
	return fCurrentColor;
}

void ColorPickerDialog::SetCurrentColor(rgb_color color)
{
	fCurrentColor = color;
}

void ColorPickerDialog::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case DIALOG_OK:
		{
			Hide();
			rgb_color color = fColorControl->ValueAsColor();
			BMessage mess(COLOR_CHANGED);
			mess.AddColor("color", color);
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
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

bool ColorPickerDialog::QuitRequested()
{
	BMessage mess(DIALOG_CLOSED);
	fOwner->MessageReceived(&mess);
	return true;
}


ColorButton::ColorButton(const char *name, BMessage *message, rgb_color color)
	: BControl(name, "", message, B_WILL_DRAW | B_NAVIGABLE),
	  fPreferredSize(-1, -1),
	  fColor(color),
	  fPicker(NULL)
{
}

ColorButton::ColorButton(BMessage *data)
	: BControl(data),
	  fPreferredSize(-1, -1),
	  fColor({0, 0, 0, 0xff})
{
	rgb_color color;
	if (data->FindColor("color", &color) == B_OK) {
		fColor = color;
	}
}

ColorButton::~ColorButton()
{
}

rgb_color ColorButton::Color()
{
	return fColor;
}

bool ColorButton::SetColor(rgb_color color)
{
	bool changed = fColor != color;
	fColor = color;
	if (changed) {
		if (Window() != NULL) {
			Window()->Lock();
			Invalidate(Bounds());
			Window()->Unlock();
		}
	}
	return changed;
}

void ColorButton::AttachedToWindow()
{
	BControl::AttachedToWindow();
	
	SetLowUIColor(B_CONTROL_BACKGROUND_COLOR, 1.115);
}

void ColorButton::GetPreferredSize(float *width, float *height)
{
	if (width != NULL) {
		*width = fPreferredSize.width;
	}
	if (height != NULL) {
		*height = fPreferredSize.height;
	}
}

BSize ColorButton::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(),
			_ValidatePreferredSize());
}

BSize ColorButton::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
			_ValidatePreferredSize());
}

BSize ColorButton::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(),
			_ValidatePreferredSize());
}

void ColorButton::LayoutInvalidated(bool descendants)
{
	fPreferredSize.Set(-1, -1);
}

status_t ColorButton::Invoke(BMessage *msg)
{
	Sync();
	_ShowPicker();
	return B_OK;
}

void ColorButton::MouseDown(BPoint where)
{
	uint32 buttons;
	GetMouse(&where, &buttons);
	if ((buttons & B_PRIMARY_MOUSE_BUTTON) > 0) {
		Invoke();
	}
}

void ColorButton::_ShowPicker()
{
	if (fPicker == NULL) {
		fPicker = new ColorPickerDialog(this, fColor);
		
		BPoint where = ConvertToScreen(Bounds()).LeftBottom();
		where.x += 5;
		where.y += 5;
		fPicker->ShowDialogAt(where);
	} else {
		fPicker->Activate();
	}
}

void ColorButton::Draw(BRect updateRect)
{
	uint32 flags = be_control_look->Flags(this);
	BRect rect(Bounds());
	
	be_control_look->DrawButtonFrame(this, rect, updateRect,
						LowColor(), ViewColor(), flags);
	// draw colored rectangle
	rect.InsetBy(1., 1.);
	SetHighColor(fColor);
	FillRect(rect);
}

status_t ColorButton::Archive(BMessage *data, bool deep) const
{
	status_t err = BControl::Archive(data, deep);
	if (err != B_OK) {
		return err;
	}
	err = data->AddColor("color", fColor);
	return err;
}

void ColorButton::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case ColorPickerDialog::COLOR_CHANGED:
		{
			rgb_color color;
			if (msg->FindColor("color", &color) == B_OK) {
				if (SetColor(color)) {
					// todo, send message to 
				}
			}
			break;
		}
		case ColorPickerDialog::DIALOG_CLOSED:
		{
			fPicker = NULL;
			break;
		}
		default:
		{
			BControl::MessageReceived(msg);
			break;
		}
	}
}

BSize ColorButton::_ValidatePreferredSize()
{
	if (fPreferredSize.width < 0) {
		font_height fontHeight;
		GetFontHeight(&fontHeight);
		float height = fontHeight.ascent + fontHeight.descent;
		height += 4;
		float width = 40.; // todo
		fPreferredSize.Set(width, height);
		ResetLayoutInvalidation();
	}
	return fPreferredSize;
}
