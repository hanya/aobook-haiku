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

#ifndef WINDOW_H_
#define WINDOW_H_


#include <Window.h>

#include <map>
#include <memory>

class BMenu;
class BookmarkDialog;
class EnvOptDialog;
class FileChooser;
class HeadingDialog;
class InputDialog;
class PageView;
class StyleOptDialog;
struct entry_ref;

class AoBookWindow : public BWindow
{
public:
	AoBookWindow(BPoint pos);
	virtual ~AoBookWindow();
	virtual void MessageReceived(BMessage *msg);
	virtual bool QuitRequested();
	
private:
	BMenuBar * _CreateMenu();
	void _Open();
	
	void _NextFile(bool nxt);
	void _NextPage(bool nxt);
	void _MovePage(bool first);
	void _MovePageNo(int32 page);
	void _MoveLineNo(int32 line);
	void _UpdateRecentFileMenu();
	void _UpdateStyleMenu();
	void _UpdateToolMenu();
	void _OpenFile(const entry_ref &ref, int32 code);
	void _ChooseRecentFile(int32 index);
	void _MouseFunc(int32 f);
	void _InitAccKeys();
	int32 _Key(uint8 byte, int32 key, uint32 raw_char, uint32 mod);
	
	std::unique_ptr<FileChooser> fFileChooser;
	InputDialog * fPageDialog;
	InputDialog * fLineDialog;
	HeadingDialog * fHeadingDialog;
	StyleOptDialog * fStyleOptDialog;
	EnvOptDialog * fEnvOptDialog;
	BookmarkDialog * fBookmarkDialog;
	InputDialog * fBookmarkCommentDialog;
	BMenu * fRecentMenu;
	BMenu * fStyleMenu;
	BMenu * fToolMenu;
	PageView * fPageView;
	std::map<uint16, uint16> fAccKeys;
	
};

#endif
