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

#ifndef DIALOG_H_
#define DIALOG_H_

#include <MessageFilter.h>
#include <Window.h>

class BMenuField;
class BPopUpMenu;
class BStringView;
class BTextControl;


class InputDialog : public BWindow
{
public:
	enum InputType {
		IT_STRING,
		IT_INTEGER,
		IT_FLOAT,
	};
	
	InputDialog(BWindow *owner, const char *title, const char *label, 
				InputType type, int32 command, int32 quit_command);
	virtual ~InputDialog();
	virtual void ShowDialog(BRect parent);
	virtual void MessageReceived(BMessage *msg);
	virtual void WindowActivated(bool active);
	virtual void Quit();
	void SetText(const char *text);
	static filter_result IntegerKeyDownFilter(BMessage *msg, BHandler **target, 
						BMessageFilter *filter);
	static filter_result FloatKeyDownFilter(BMessage *msg, BHandler **target, 
						BMessageFilter *filter);
	
	static const int32 SET_TITLE = 'sttl';
	static const int32 SET_LABEL = 'stle';
	
private:
	static const int32 DATA_INPUT = 'inpt';
	BTextControl * fTextControl;
	BStringView * fLabel;
	BWindow * fOwner;
	int32 fCommand;
	int32 fQuitCommand;
	InputType fInputType;
	
};


#endif
