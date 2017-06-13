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

#ifndef COLOR_BUTTON_H_
#define COLOR_BUTTON_H_

#include <Control.h>
#include <Window.h>

class BColorControl;

class ColorPickerDialog : public BWindow
{
public:
	ColorPickerDialog(BHandler *owner, rgb_color currentColor);
	virtual ~ColorPickerDialog();
	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
	virtual void WindowActivated(bool active);
	
	virtual void ShowDialog(BRect parent);
	virtual void ShowDialogAt(BPoint where);
	
	virtual rgb_color CurrentColor();
	void SetCurrentColor(rgb_color color);
	
	virtual rgb_color Color();
	void SetColor(rgb_color color);
	
	static const uint32 COLOR_CHANGED = 'cpch';
	static const uint32 DIALOG_CLOSED = 'cpcl';
	
private:
	static const uint32 DIALOG_OK = 'btok';
	static const uint32 DIALOG_CANCEL = 'btcl';
	
	BHandler * fOwner;
	rgb_color fCurrentColor;
	rgb_color fNewColor;
	BColorControl * fColorControl;
	
};


class ColorButton : public BControl
{
public:
	ColorButton(const char *name, BMessage *message, rgb_color color);
	ColorButton(BMessage *data);
	virtual ~ColorButton();
	virtual void AttachedToWindow();
	virtual void MouseDown(BPoint where);
	virtual void MessageReceived(BMessage *msg);
	virtual void Draw(BRect updateRect);
	virtual status_t Archive(BMessage *data, bool deep) const;
	virtual void GetPreferredSize(float *width, float *height);
	virtual BSize MinSize();
	virtual BSize MaxSize();
	virtual BSize PreferredSize();
	virtual void LayoutInvalidated(bool descendants);
	virtual status_t Invoke(BMessage *msg = NULL);
	
	virtual rgb_color Color();
	virtual bool SetColor(rgb_color color);
	
private:
	BSize fPreferredSize;
	rgb_color fColor;
	ColorPickerDialog * fPicker;
	
	BSize _ValidatePreferredSize();
	void _ShowPicker();
	
};


#endif
