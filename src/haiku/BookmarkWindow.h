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

#ifndef BOOKMARK_WINDOW_H_
#define BOOKMARK_WINDOW_H_

#include <Window.h>

#include <memory>

class FileChooser;
class InputDialog;
class GlobalBookmarksView;
class LocalBookmarksView;

class BTabView;

class BookmarkDialog : public BWindow
{
public:
	BookmarkDialog(BWindow *owner);
	virtual ~BookmarkDialog();
	virtual void Quit();
	virtual void ShowDialog(BRect parent);
	virtual void MessageReceived(BMessage *msg);
	
	static const int32 GLOBAL_ADD_NEW = 'bkga';
	static const int32 LOCAL_ADD_NEW = 'bkla';
	
private:
	BWindow *fOwner;
	BTabView * fTabView;
	GlobalBookmarksView * fGlobalBkView;
	LocalBookmarksView * fLocalBkView;
	InputDialog * fCommentDialog;
	FileChooser * fFileLoadChooser;
	FileChooser * fFileStoreChooser;
	
	void _SetList();
	void _LoadLocal(entry_ref *ref);
	void _SaveLocal(entry_ref *ref, const char *name);
	
};

#endif
