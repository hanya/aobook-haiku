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

#ifndef FONT_BUTTON_H_
#define FONT_BUTTON_H_

#include <MessageFilter.h>
#include <Button.h>
#include <Window.h>

#include <string>

class BWindow;
class BMenuField;
class BPopUpMenu;
class BTextControl;

enum {
	FONT_WEIGHT_NONE,
	FONT_WEIGHT_NORMAL,
	FONT_WEIGHT_BOLD,
};

enum {
	FONT_SLANT_NONE,
	FONT_SLANT_ROMAN,
	FONT_SLANT_ITALIC,
	FONT_SLANT_OBLIQUE,
};

#define FONT_DIALOG_CHOOSE 'fdch'
#define FONT_DIALOG_CLOSED 'fdcl'

class FontDialog : public BWindow
{
public:
	FontDialog(BHandler *owner, int32 command=FONT_DIALOG_CHOOSE, 
				int32 quit_command=FONT_DIALOG_CLOSED);
	virtual void MessageReceived(BMessage *msg);
	void ShowDialog(BRect parent);
	void ShowDialogAt(BPoint where);
	virtual void Quit();
	virtual void WindowActivated(bool active);
	
	void SetFontFamily(const char *family);
	void SetFontStyle(const char *style);
	void SetFontSize(int32 size);
	void SetFontWeight(int32 weight);
	void SetFontSlant(int32 slant);
	
private:
	BHandler * fOwner;
	BPopUpMenu * fFontPM;
	BTextControl * fSizeTC;
	BMenuField * fFontMF;
	BPopUpMenu * fWeightPM;
	BPopUpMenu * fSlantPM;
	int32 fCommand;
	int32 fQuitCommand;
	
	void _Init();
	void _ChooseStyle(int32 family);
	
};


class FontButton : public BButton
{
public:
	FontButton(const char *name, const char *label);
	virtual ~FontButton();
	virtual void MessageReceived(BMessage *msg);
	virtual status_t Invoke(BMessage *msg);
	
	const char * FontFamily();
	void SetFontFamily(const char *family);
	const char * FontStyle();
	void SetFontStyle(const char *style);
	int32 FontSize();
	void SetFontSize(int32 size);
	int32 FontWeight();
	void SetFontWeight(int32 weight);
	int32 FontSlant();
	void SetFontSlant(int32 slant);
	
private:
	std::string fFontFamily;
	std::string fFontStyle;
	int32 fFontSize;
	int32 fFontWeight;
	int32 fFontSlant;
	
	FontDialog * fFontDialog;
	
	void _ShowPicker();
	void _UpdateLabel();
	
};


#endif
