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

#ifndef STYLE_OPT_DIALOG_H_
#define STYLE_OPT_DIALOG_H_

#include <Window.h>

class BMenuField;
class BPopUpMenu;

class EditStyles;
class FileChooser;
class InputDialog;
class StyleOptBasicView;
class StyleOptCharView;
class StyleOptFontView;

class StyleOptDialog : public BWindow
{
public:
	StyleOptDialog(BWindow *owner);
	virtual ~StyleOptDialog();
	void ShowDialog(BRect parent);
	virtual void Quit();
	virtual void WindowActivated(bool active);
	virtual void MessageReceived(BMessage *msg);
	
private:
	static const int32 CMD_NEW = 'cmne';
	static const int32 CMD_DELETE = 'cmdl';
	
	BWindow * fOwner;
	BPopUpMenu * fStylesMenu;
	BMenuField * fStylesMF;
	InputDialog * fInputDialog;
	
	StyleOptBasicView * fBasicView;
	StyleOptCharView * fCharView;
	StyleOptFontView * fFontView;
	
	FileChooser * fFileChooser;
	
	EditStyles *fEditStyles;
	bool fStyleListChanged;
	
	void _InitData();
	void _StoreData();
	void _UpdateData();
	void _ChangeStyle(bool store=true);
	void _SetStyleChanged();
	void _NewStyle(const char *name);
	void _DeleteStyle();
	
};

#endif
