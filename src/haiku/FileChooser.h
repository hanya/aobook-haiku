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

#ifndef FILE_CHOOSER_H_
#define FILE_CHOOSER_H_

#include "common.h"

#include <Looper.h>
#include <FilePanel.h>

#include <memory>
#include <vector>

class BFilePanel;
class BMenu;
class BMessenger;


class FileFilter : public BRefFilter
{
public:
	FileFilter();
	~FileFilter();
	virtual bool Filter(const entry_ref* ref, BNode *node,
						struct stat_beos* stat, const char *mimeType);
	
	void AddFileFilter(const char *name, const char *ext);
	int32 FilterIndex();
	bool SetFilterIndex(int32 index);
	const char * FilterName(int32 index);
	int32 FilterCount();
	
	
private:
	typedef struct {
		std::string name;
		std::vector<std::string> filters;
	} FilterItem;
	std::vector<FilterItem> fFilters;
	int32 fFilterIndex;
	
};


class FileChooser : public BLooper
{
public:
	FileChooser(BLooper *looper, 
		file_panel_mode mode=B_OPEN_PANEL, FileFilter * filter=NULL,
		bool encoding=false, int32 kind=0);
	virtual ~FileChooser();
	virtual void MessageReceived(BMessage *msg);
	void Show();
	virtual bool QuitRequested();
	
private:
	BMessage * _CreateFilterMessage(int32 index);
	BMessage * _CreateEncodingMssage(Encoding encoding);
	void _SwitchMenuItem(BMenu *menu, const char *name, int32 v);
	
	//std::unique_ptr<FileFilter> fFileFilter;
	FileFilter * fFileFilter;
	std::unique_ptr<BFilePanel> fFilePanel;
	
	Encoding fEncoding;
	int32 fKind;
	
	BMenu * fEncodingMenu;
	BMenu * fFilterMenu;
	
	BLooper * fTarget;
	
	static const int32 SET_ENCODING = 'sten';
	static const int32 CHANGE_FILTER = 'chfl';
	
};

#endif
