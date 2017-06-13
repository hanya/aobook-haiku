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

#include "BookmarkWindow.h"

#include "common.h"

#include "dialogs.h"
#include "FileChooser.h"

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <File.h>
#include <LayoutBuilder.h>
#include <LayoutUtils.h>
#include <ListView.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringItem.h>
#include <TabView.h>
#include <View.h>

#include "mDef.h"
#include "mGui.h"
#include "mStr.h"
#include "mList.h"
#include "mTextRead.h"

#include "globaldata.h"
#include "bookmarkdat.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"


class SmallButton : public BButton
{
public:
	SmallButton(const char *name, const char *label, BMessage *msg);
	virtual ~SmallButton();
	virtual void GetPreferredSize(float *width, float *height);
	virtual BSize MinSize();
	virtual BSize MaxSize();
	virtual BSize PreferredSize();
	virtual void LayoutInvalidated(bool descendants);
	
private:
	BSize fPreferredSize;
	
	BSize _ValidatePreferredSize();
	
};

SmallButton::SmallButton(const char *name, const char *label, 
							BMessage *msg)
	: BButton(name, label, msg),
	  fPreferredSize(-1, -1)
{
}

SmallButton::~SmallButton()
{
}

void SmallButton::GetPreferredSize(float *width, float *height)
{
	_ValidatePreferredSize();
	
	if (width != NULL) {
		*width = fPreferredSize.width;
	}
	if (height != NULL) {
		*height = fPreferredSize.height;
	}
}

BSize SmallButton::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(),
			_ValidatePreferredSize());
}

BSize SmallButton::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
			_ValidatePreferredSize());
}

BSize SmallButton::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(),
			_ValidatePreferredSize());
}

void SmallButton::LayoutInvalidated(bool descendants)
{
	fPreferredSize.Set(-1, -1);
}

BSize SmallButton::_ValidatePreferredSize()
{
	if (fPreferredSize.width < 0) {
		font_height fontHeight;
		GetFontHeight(&fontHeight);
		float height = fontHeight.ascent + fontHeight.descent;
		height += 12;
		float width = 10;
		const char *label = Label();
		if (label != NULL) {
			width = StringWidth(label);
		}
		width += 18;
		fPreferredSize.Set(width, height);
	}
	return fPreferredSize;
}


enum {
	CMD_GLOBAL_ADD = 'cmga',
	CMD_GLOBAL_DELETE = 'cmgd',
	CMD_GLOBAL_CALL = 'cmgc',
	
	CMD_LOCAL_ADD = 'cmla',
	CMD_LOCAL_LOAD = 'cmll',
	CMD_LOCAL_STORE = 'cmls',
	CMD_LOCAL_CLEAR = 'cmlc',
	CMD_LOCAL_DELETE = 'cmld',
	CMD_LOCAL_CALL = 'cmlg',
	CMD_LOCAL_EDIT = 'cmle',
};

enum {
	A_LOCAL_COMMENT_EDIT = 'alce',
	A_LOCAL_COMMENT_DIALOG_CLOSED = 'alcc',
};

class GlobalBookmarksView : public BView
{
public:
	GlobalBookmarksView();
	void MessageReceived(BMessage *msg);
	void InitList();
	int32 CurrentSelection();
	
private:
	BListView * fBookmarksList;
	BPopUpMenu * fContextMenu;
	
};

GlobalBookmarksView::GlobalBookmarksView()
	: BView(B_TRANSLATE("Global"), B_WILL_DRAW)
{
	fBookmarksList = new BListView("glbookmarks");
	fBookmarksList->SetInvocationMessage(new BMessage(CMD_GLOBAL_CALL));
	fContextMenu = new BPopUpMenu("contextmenug");
	fContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Call"), 
				new BMessage(CMD_GLOBAL_CALL)));
	fContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Delete"), 
				new BMessage(CMD_GLOBAL_DELETE)));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(6, 6, 6, 6)
		.AddGroup(B_HORIZONTAL, 3)
			.Add(new SmallButton("add", B_TRANSLATE("Add"),
					new BMessage(CMD_GLOBAL_ADD)))
			.Add(new SmallButton("delete", B_TRANSLATE("Delete"),
					new BMessage(CMD_GLOBAL_DELETE)))
			.Add(new SmallButton("call", B_TRANSLATE("Call"),
					new BMessage(CMD_GLOBAL_CALL)))
			.AddGlue()
		.End()
		.Add(new BScrollView("gbk", fBookmarksList, 0, false, true));
}

void GlobalBookmarksView::InitList()
{
	fBookmarksList->MakeEmpty();
	
	mStr str = MSTR_INIT;
	BmItemGlobal *pg;
	
	for (pg = BMITEM_G(GDAT->listBkmarkGlobal.top); pg; pg = BMITEM_G(pg->i.next)) {
		BookmarkGlobal_getListStr(&str, pg);
		
		fBookmarksList->AddItem(new BStringItem(str.buf));
	}
	
	mStrFree(&str);
}

int32 GlobalBookmarksView::CurrentSelection()
{
	return fBookmarksList->CurrentSelection(0);
}

void GlobalBookmarksView::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case BookmarkDialog::GLOBAL_ADD_NEW:
		{
			BmItemGlobal *pg = BMITEM_G(GDAT->listBkmarkGlobal.bottom);
			if (pg != NULL) {
				mStr str = MSTR_INIT;
				BookmarkGlobal_getListStr(&str, pg);
				
				fBookmarksList->AddItem(new BStringItem(str.buf));
				mStrFree(&str);
			}
			break;
		}
		case CMD_GLOBAL_DELETE:
		{
			int32 selected = fBookmarksList->CurrentSelection(0);
			if (selected >= 0) {
				mListDeleteByIndex(&GDAT->listBkmarkGlobal, selected);
				
				fBookmarksList->RemoveItem(selected);
			}
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}

class LocalBookmarksView : public BView
{
public:
	LocalBookmarksView();
	void MessageReceived(BMessage *msg);
	void InitList();
	int32 CurrentSelection();
	void ReloadItem(int32 index);
	virtual void MouseDown(BPoint where);
	
private:
	BListView * fBookmarksList;
	BPopUpMenu * fContextMenu;
	
};

LocalBookmarksView::LocalBookmarksView()
	: BView(B_TRANSLATE("Local"), B_WILL_DRAW)
{
	fBookmarksList = new BListView("locbookmarks");
	fBookmarksList->SetInvocationMessage(new BMessage(CMD_LOCAL_CALL));
	fContextMenu = new BPopUpMenu("contextmenu");
	fContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Call"), 
					new BMessage(CMD_LOCAL_CALL)));
	fContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Edit"), 
					new BMessage(CMD_LOCAL_EDIT)));
	fContextMenu->AddItem(new BMenuItem(B_TRANSLATE("Delete"), 
					new BMessage(CMD_LOCAL_DELETE)));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(6, 6, 6, 6)
		.AddGroup(B_HORIZONTAL, 3)
			.Add(new SmallButton("add", B_TRANSLATE("Add"),
					new BMessage(CMD_LOCAL_ADD)))
			.Add(new SmallButton("load", B_TRANSLATE("Load"),
					new BMessage(CMD_LOCAL_LOAD)))
			.Add(new SmallButton("store", B_TRANSLATE("Store"),
					new BMessage(CMD_LOCAL_STORE)))
			.Add(new SmallButton("clear", B_TRANSLATE("Clear"),
					new BMessage(CMD_LOCAL_CLEAR)))
			.AddGlue()
		.End()
		.Add(new BScrollView("lbk", fBookmarksList, 0, false, true));
}

void LocalBookmarksView::InitList()
{
	fBookmarksList->MakeEmpty();
	
	mStr str = MSTR_INIT;
	BmItemLocal *pl;
	
	for (pl = BMITEM_L(GDAT->listBkmarkLocal.top); pl; pl = BMITEM_L(pl->i.next)) {
		BookmarkLocal_getListStr(&str, pl);
		
		fBookmarksList->AddItem(new BStringItem(str.buf));
	}
	
	mStrFree(&str);
}

void LocalBookmarksView::MouseDown(BPoint where)
{
	uint32 buttons;
	GetMouse(&where, &buttons);
	if ((buttons & B_SECONDARY_MOUSE_BUTTON) > 0) {
		int32 selected = fBookmarksList->CurrentSelection(0);
		if (selected >= 0) {
			fContextMenu->Go(where, true);
			return;
		}
	}
	BView::MouseDown(where);
}

int32 LocalBookmarksView::CurrentSelection()
{
	return fBookmarksList->CurrentSelection(0);
}

void LocalBookmarksView::ReloadItem(int32 index)
{
	BListItem *item = fBookmarksList->ItemAt(index);
	if (item != NULL) {
		BmItemLocal *pl = BMITEM_L(mListGetItemByIndex(
						&GDAT->listBkmarkLocal, index));
		if (pl != NULL) {
			mStr str = MSTR_INIT;
			BookmarkLocal_getListStr(&str, pl);
			
			((BStringItem *)item)->SetText(str.buf);
			
			mStrFree(&str);
		}
	}
}

void LocalBookmarksView::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case BookmarkDialog::LOCAL_ADD_NEW:
		{
			InitList();
			break;
		}
		case CMD_LOCAL_DELETE:
		{
			int32 selected = CurrentSelection();
			if (selected >= 0) {
				mListDeleteByIndex(&GDAT->listBkmarkLocal, selected);
				
				fBookmarksList->RemoveItem(selected);
			}
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}


BookmarkDialog::BookmarkDialog(BWindow *owner)
	: BWindow(
		BRect(50, 50, 330, 300),
		B_TRANSLATE("Bookmarks"),
		B_FLOATING_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS |
		B_CLOSE_ON_ESCAPE),
	  fOwner(owner),
	  fFileLoadChooser(NULL),
	  fFileStoreChooser(NULL)
{
	fGlobalBkView = new GlobalBookmarksView();
	fLocalBkView = new LocalBookmarksView();
	
	fTabView = new BTabView("tabview");
	fTabView->AddTab(fGlobalBkView);
	fTabView->AddTab(fLocalBkView);
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(0, 6, 0, 0)
		.Add(fTabView);
	
	Layout(true);
}

BookmarkDialog::~BookmarkDialog()
{
}

void BookmarkDialog::Quit()
{
	BRect r = Frame();
	GDAT->bkmarkwin_box.x = r.left;
	GDAT->bkmarkwin_box.y = r.top;
	GDAT->bkmarkwin_box.w = r.Width();
	GDAT->bkmarkwin_box.h = r.Height();
	
	GDAT->bkmarkwin_tabno = fTabView->FocusTab();
	
	if (fFileLoadChooser && fFileLoadChooser->Lock()) {
		fFileLoadChooser->Quit();
		//delete fFileLoadChooser;
	}
	if (fFileStoreChooser && fFileStoreChooser->Lock()) {
		fFileStoreChooser->Quit();
		//delete fFileStoreChooser;
	}
	
	fOwner->PostMessage(A_BOOKMARK_DIALOG_CLOSED);
	BWindow::Quit();
}

void BookmarkDialog::ShowDialog(BRect parent)
{
	_SetList();
	
	float x = 100;
	float y = 100;
	if (GDAT->bkmarkwin_box.x > 0) {
		x = GDAT->bkmarkwin_box.x;
	}
	if (GDAT->bkmarkwin_box.y > 0) {
		y = GDAT->bkmarkwin_box.y;
	}
	MoveTo(x, y);
	float width = 300;
	float height = 300;
	if (GDAT->bkmarkwin_box.w > 0) {
		width = GDAT->bkmarkwin_box.w;
	}
	if (GDAT->bkmarkwin_box.h > 0) {
		height = GDAT->bkmarkwin_box.h;
	}
	ResizeTo(width, height);
	
	int tabno = GDAT->bkmarkwin_tabno;
	if (0 <= tabno && tabno <= 1) {
		fTabView->Select(tabno);
	}
	
	Show();
}

void BookmarkDialog::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case GLOBAL_ADD_NEW:
		{
			fGlobalBkView->MessageReceived(msg);
			break;
		}
		case LOCAL_ADD_NEW:
		{
			fLocalBkView->MessageReceived(msg);
			break;
		}
		case CMD_GLOBAL_ADD:
		{
			// GLOBAL_ADD_NEW will be sent from the owner.
			fOwner->PostMessage(CMD_BOOKMARK_ADD_TO_GLOBAL);
			break;
		}
		case CMD_GLOBAL_DELETE:
		{
			fGlobalBkView->MessageReceived(msg);
			break;
		}
		case CMD_GLOBAL_CALL:
		{
			int32 selected = fGlobalBkView->CurrentSelection();
			if (selected >= 0) {
				BmItemGlobal *pg = BMITEM_G(mListGetItemByIndex(
						&GDAT->listBkmarkGlobal, selected));
				if (pg != NULL) {
					if (mStrPathCompareEq(&GDAT->strFileName, pg->fname)) {
						BMessage mess(A_GOTO_LINE);
						mess.AddInt32("value", pg->lineno);
						fOwner->PostMessage(&mess);
					} else {
						BMessage mess(A_LOAD_FILE);
						mess.AddString("path", pg->fname);
						mess.AddInt32("lineno", pg->lineno);
						fOwner->PostMessage(&mess);
					}
				}
			}
			break;
		}
		case CMD_LOCAL_ADD:
		{
			// LOCAL_ADD_NEW will be sent from the owner.
			fOwner->PostMessage(CMD_BOOKMARK_ADD_TO_LOCAL);
			break;
		}
		case CMD_LOCAL_LOAD:
		{
			if (fFileLoadChooser == NULL) {
				FileFilter *loadFilter = new FileFilter();
				loadFilter->AddFileFilter(B_TRANSLATE("All files (*.*)"), "");
				loadFilter->AddFileFilter(B_TRANSLATE("Text files (*.txt)"), ".txt");
				fFileLoadChooser = new FileChooser(this, B_OPEN_PANEL, 
						loadFilter, false, 0);
			}
			fFileLoadChooser->Show();
			break;
		}
		case CMD_LOCAL_STORE:
		{
			if (fFileStoreChooser == NULL) {
				FileFilter *storeFilter = new FileFilter();
				storeFilter->AddFileFilter(B_TRANSLATE("All files (*.*)"), "");
				storeFilter->AddFileFilter(B_TRANSLATE("Text files (*.txt)"), ".txt");
				fFileStoreChooser = new FileChooser(this, B_SAVE_PANEL, 
						storeFilter, false, 1);
			}
			fFileStoreChooser->Show();
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) == B_OK) {
				_LoadLocal(&ref);
			}
			break;
		}
		case B_SAVE_REQUESTED:
		{
			entry_ref ref;
			const char *name;
			if (msg->FindRef("directory", &ref) == B_OK &&
				msg->FindString("name", &name) == B_OK) {
				_SaveLocal(&ref, name);
			}
			break;
		}
		case CMD_LOCAL_CLEAR:
		{
			if (GDAT->listBkmarkLocal.num == 0) {
				return;
			}
			BAlert *alert = new BAlert("Clear local bookmarks",
						B_TRANSLATE("Do you want to clear local bookmarks?"),
						B_TRANSLATE("YES"), B_TRANSLATE("NO"), NULL,
						B_WIDTH_AS_USUAL, B_IDEA_ALERT);
			alert->SetShortcut(1, B_ESCAPE);
			int32 button_index = alert->Go();
			if (button_index == 0) {
				mListDeleteAll(&GDAT->listBkmarkLocal);
				
				fLocalBkView->InitList();
			}
			break;
		}
		case CMD_LOCAL_CALL:
		{
			int32 selected = fLocalBkView->CurrentSelection();
			if (selected >= 0) {
				BmItemLocal *pl = BMITEM_L(mListGetItemByIndex(
								&GDAT->listBkmarkLocal, selected));
				if (pl != NULL) {
					BMessage mess(A_GOTO_LINE);
					mess.AddInt32("value", pl->lineno + 1);
					fOwner->PostMessage(&mess);
				}
			}
			break;
		}
		case CMD_LOCAL_DELETE:
		{
			fLocalBkView->MessageReceived(msg);
			break;
		}
		case CMD_LOCAL_EDIT:
		{
			// todo, allow to set base message
			if (fCommentDialog == NULL) {
				fCommentDialog = new InputDialog(this, 
						B_TRANSLATE("Comment"),
						B_TRANSLATE("Comment (leave empty with no comment)"),
						InputDialog::IT_STRING,
						A_LOCAL_COMMENT_EDIT, 
						A_LOCAL_COMMENT_DIALOG_CLOSED);
			}
			// todo, set current comment
			int32 selected = fLocalBkView->CurrentSelection();
			if (selected < 0) {
				return;
			}
			BmItemLocal *pl = BMITEM_L(mListGetItemByIndex(
							&GDAT->listBkmarkLocal, selected));
			if (pl != NULL) {
				fCommentDialog->SetText(pl->comment);
				
				// todo, lock the list view to avoid change of the current selection
				fCommentDialog->ShowDialog(Frame());
			}
			break;
		}
		case A_LOCAL_COMMENT_DIALOG_CLOSED:
		{
			fCommentDialog = NULL;
			break;
		}
		case A_LOCAL_COMMENT_EDIT:
		{
			const char *value;
			if (msg->FindString("value", &value) == B_OK) {
				int32 selected = fLocalBkView->CurrentSelection();
				if (selected < 0) {
					return;
				}
				BmItemLocal *pl = BMITEM_L(mListGetItemByIndex(
							&GDAT->listBkmarkLocal, selected));
				if (pl != NULL) {
					M_FREE_NULL(pl->comment);
					if (value != NULL) {
						pl->comment = mStrdup(value);
					}
					
					fLocalBkView->ReloadItem(selected);
				}
			}
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

void BookmarkDialog::_SetList()
{
	fGlobalBkView->InitList();
	fLocalBkView->InitList();
}


static void _local_destroy(mListItem *p)
{
	mFree(BMITEM_L(p)->comment);
}

void BookmarkDialog::_LoadLocal(entry_ref *ref)
{
	BPath path(ref);
	mTextRead *p = mTextRead_readFile(path.Path());
	if (p != NULL) {
		mListDeleteAll(&GDAT->listBkmarkLocal);
		
		BmItemLocal *item;
		char *pc, *split;
		while (1) {
			pc = mTextReadGetLine_skipEmpty(p);
			if (pc == NULL) {
				break;
			}
			split = strchr(pc, ' ');
			if (split) {
				*(split++) = 0;
			}
			item = (BmItemLocal *)mListAppendNew(&GDAT->listBkmarkLocal,
					sizeof(BmItemLocal), _local_destroy);
			if (item != NULL) {
				item->lineno = atoi(pc) -1;
				item->comment = (split && *split) ? mStrdup(split) : NULL;
			}
		}
	}
	mTextReadEnd(p);
	
	fLocalBkView->InitList();
}

void BookmarkDialog::_SaveLocal(entry_ref *ref, const char *name)
{
	BPath path(ref);
	path.Append(name);
	BEntry entry(path.Path());
	entry_ref file;
	entry.GetRef(&file);
	BFile f(&file, B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
	if (f.InitCheck() == B_OK) {
		BmItemLocal *pg = NULL;
		for (pg = BMITEM_L(GDAT->listBkmarkLocal.top); pg; pg = BMITEM_L(pg->i.next)) {
			std::string lineno = std::to_string(pg->lineno + 1);
			f.Write(lineno.c_str(), lineno.size());
			if (pg->comment != NULL) {
				f.Write(" ", 1);
				f.Write(pg->comment, strlen(pg->comment));
			}
			f.Write("\n", 1);
		}
	}
}
