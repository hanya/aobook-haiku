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

#include "FileChooser.h"

#include "common.h"

#include <Catalog.h>
#include <FilePanel.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <Messenger.h>
#include <Path.h>

#include <compat/sys/stat.h>

#include <string.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"


class TrueFilter : public BRefFilter
{
public:
	TrueFilter();
	virtual	bool Filter(const entry_ref* ref, BNode* node,
						struct stat_beos* stat, const char* mimeType);
private:
	
};

TrueFilter::TrueFilter()
	: BRefFilter()
{
}

bool TrueFilter::Filter(const entry_ref* ref, BNode* node,
						struct stat_beos* stat, const char* mimeType)
{
	return true;
}


FileFilter::FileFilter()
	: BRefFilter(),
	  fFilterIndex(-1)
{
}

FileFilter::~FileFilter()
{
}

void FileFilter::AddFileFilter(const char *name, const char *ext)
{
	FilterItem f;
	f.name = name;
	// split with ;
	std::string filter(ext);
	int n;
	int prev = 0;
	while (true) {
		n = filter.find(';', prev);
		if (n != std::string::npos) {
			f.filters.push_back(filter.substr(prev, n-1));
			prev = n+1;
		} else {
			f.filters.push_back(filter.substr(prev, filter.size()));
			break;
		}
	}
	
	fFilters.push_back(f);
}

int32 FileFilter::FilterIndex()
{
	return fFilterIndex;
}

bool FileFilter::SetFilterIndex(int32 index)
{
	bool changed = fFilterIndex != index;
	fFilterIndex = index;
	return changed;
}

const char * FileFilter::FilterName(int32 index)
{
	if (0 <= index && index < fFilters.size()) {
		return fFilters[index].name.c_str();
	}
	return NULL;
}

int32 FileFilter::FilterCount()
{
	return fFilters.size();
}

bool FileFilter::Filter(const entry_ref* ref, BNode *node,
						struct stat_beos* stat, const char *mimeType)
{
	if (ref == NULL || ref->name == NULL) {
		return false;
	}
	if (S_ISDIR(stat->st_mode)) {
		return true;
	}
	if (!(0 <= fFilterIndex && fFilterIndex < fFilters.size())) {
		return true;
	}
	if (fFilters.size() == 0) {
		return true;
	}
	
	auto i = fFilters[fFilterIndex].filters.begin();
	auto e = fFilters[fFilterIndex].filters.end();
	for (; i != e; ++i) {
		const char *ext = (*i).c_str();
		int ext_length = (*i).size();
		if (ext_length == 0) {
			return true; // All *.*
		}
		
		int length = strlen(ref->name);
		
		if (length < ext_length) {
			continue;
		}
		char *found = NULL;
		char *prev = NULL;
		found = strstr(ref->name, ext);
		prev = found;
		while (found != NULL) {
			found = strstr(prev+1, ext);
			if (found != NULL) {
				prev = found;
			}
		}
		if (prev != NULL && (length - ext_length) == (prev - ref->name)) {
			return true;
		}
	}
	
	return false;
}


FileChooser::FileChooser(BLooper *looper, 
		file_panel_mode mode, FileFilter *filter, bool encoding, int32 kind)
	: BLooper("chooser"),
	  fEncoding(AUTO),
	  fTarget(looper),
	  fFileFilter(filter),
	  fKind(kind)
{
	BMessenger messenger(NULL, this);
	fFilePanel = std::unique_ptr<BFilePanel>(
		new BFilePanel(mode,
			&messenger, NULL,
			0, false,
			NULL, filter, false, true));
	
	BMenuBar *bar = dynamic_cast<BMenuBar *>(
					fFilePanel->Window()->FindView("MenuBar"));
	if (bar != NULL) {
		// add encoding
		if (encoding) {
			fEncodingMenu = new BMenu(B_TRANSLATE("Encoding"));
			
			bar->AddItem(fEncodingMenu);
			BLayoutBuilder::Menu<>(fEncodingMenu)
				.AddItem(B_TRANSLATE("Auto"), _CreateEncodingMssage(AUTO))
				.AddItem(B_TRANSLATE("Shift-JIS"), 
						_CreateEncodingMssage(SHIFT_JIS))
				.AddItem(B_TRANSLATE("EUC-JP"), _CreateEncodingMssage(EUC_JP))
				.AddItem(B_TRANSLATE("UTF-8"), _CreateEncodingMssage(UTF_8))
				.AddItem(B_TRANSLATE("UTF-16LE"), _CreateEncodingMssage(UTF_16LE))
				.AddItem(B_TRANSLATE("UTF-16BE"), _CreateEncodingMssage(UTF_16BE));
			fEncodingMenu->ItemAt(0)->SetMarked(true);
			fEncodingMenu->SetTargetForItems(messenger);
		}
		
		// add filter
		if (fFileFilter != NULL) {
			fFilterMenu = new BMenu(B_TRANSLATE("Filter"));
			bar->AddItem(fFilterMenu);
			for (int i = 0; i < fFileFilter->FilterCount(); ++i) {
				fFilterMenu->AddItem(new BMenuItem(
						fFileFilter->FilterName(i), 
						_CreateFilterMessage(i)));
			}
			if (fFileFilter->FilterIndex() >= 0) {
				if (fFilterMenu->CountItems() > fFileFilter->FilterIndex()) {
					fFilterMenu->ItemAt(fFileFilter->FilterIndex())->SetMarked(true);
				}
			} else if (fFilterMenu->CountItems() > 0) {
				fFileFilter->SetFilterIndex(0);
				fFilterMenu->ItemAt(0)->SetMarked(true);
			}
			
			fFilterMenu->SetTargetForItems(messenger);
		}
	}
	Run();
}

FileChooser::~FileChooser()
{
	//delete fFileFilter;
}

bool FileChooser::QuitRequested()
{
	return true;
}

BMessage * FileChooser::_CreateEncodingMssage(Encoding encoding)
{
	BMessage *msg = new BMessage(SET_ENCODING);
	msg->AddInt32("encoding", encoding);
	return msg;
}

BMessage * FileChooser::_CreateFilterMessage(int32 index)
{
	BMessage *msg = new BMessage(CHANGE_FILTER);
	msg->AddInt32("index", index);
	return msg;
}

void FileChooser::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) == B_OK) {
				BMessage mess(B_REFS_RECEIVED);
				mess.AddRef("refs", &ref);
				mess.AddInt32("encoding", (int32)fEncoding);
				mess.AddInt32("kind", fKind);
				fTarget->PostMessage(&mess);
			}
			break;
		}
		case B_SAVE_REQUESTED:
		{
			entry_ref ref;
			const char * name;
			if (msg->FindRef("directory", &ref) == B_OK &&
				msg->FindString("name", &name) == B_OK) {
				BMessage mess(B_SAVE_REQUESTED);
				mess.AddRef("directory", &ref);
				mess.AddString("name", name);
				mess.AddInt32("encoding", (int32)fEncoding);
				mess.AddInt32("kind", fKind);
				fTarget->PostMessage(&mess);
			}
			break;
		}
		case SET_ENCODING:
		{
			int32 encoding;
			if (msg->FindInt32("encoding", &encoding) == B_OK) {
				if (AUTO <= encoding && encoding < ENCODING_END) {
					fEncoding = (Encoding)encoding;
					_SwitchMenuItem(fEncodingMenu, "encoding", encoding);
				}
			}
			break;
		}
		case CHANGE_FILTER:
		{
			int32 index;
			if (msg->FindInt32("index", &index) == B_OK) {
				if (fFileFilter->SetFilterIndex(index)) {
					_SwitchMenuItem(fFilterMenu, "index", index);
					fFilePanel->Refresh();
				}
			}
			break;
		}
		default:
		{
			BHandler::MessageReceived(msg);
			break;
		}
	}
}

void FileChooser::_SwitchMenuItem(BMenu *menu, const char *name, int32 v)
{
	BMenuItem *item = menu->FindMarked();
	if (item != NULL) {
		item->SetMarked(false);
	}
	for (int i = 0; i < menu->CountItems(); ++i) {
		BMenuItem *item = menu->ItemAt(i);
		if (item != NULL) {
			BMessage *mess = item->Message();
			if (mess != NULL) {
				int32 t;
				if (mess->FindInt32(name, &t) == B_OK && t == v) {
					item->SetMarked(true);
				}
			}
		}
	}
}

void FileChooser::Show()
{
	fFilePanel->Show();
}
