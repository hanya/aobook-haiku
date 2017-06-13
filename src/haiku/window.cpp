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

#include "window.h"

#include "common.h"
#include "version.h"

#include "BookmarkWindow.h"
#include "EnvOptDialog.h"
#include "FileChooser.h"
#include "HeadingDialog.h"
#include "StyleOptDialog.h"
#include "dialogs.h"
#include "keys.h"

#include <Application.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <Entry.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MessageFilter.h>
#include <Path.h>
#include <View.h>

#include "mDef.h"
#include "mGui.h"
#include "mPixbuf.h"
#include "mStr.h"
#include "mDirEntry.h"
#include "mKeyDef.h"
#include "mUtilSys.h"

#include "globaldata.h"
#include "mainfunc.h"
#include "aoText.h"
#include "aoLayout.h"
#include "style.h"
#include "bookmarkdat.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"

#define A_MOUSE_CLICK 'amcl'
#define A_WHEEL_CHANGE 'amwc'

#define A_LOCAL_BOOKMARK_COMMENT 'albc'
#define A_LOCAL_BOOKMARK_COMMENT_DIALOG_CLOSED 'albd'

// see mlib/src/haiku/haiku_pixbuf.c
typedef struct
{
	uint8_t flags;
	int offsetX, offsetY;
	mRect clip, clipMaster;
}_mPixbufPrivate;

typedef struct
{
	mPixbuf b;
	_mPixbufPrivate p;
	
	BBitmap *bitmap;
} __mPixbufHaiku;


class PageView : public BView
{
public:
	PageView(AoBookWindow *owner, BSize size);
	virtual ~PageView();
	virtual void MouseDown(BPoint point);
	virtual void Draw(BRect rect);
	virtual void MessageReceived(BMessage *msg);
	virtual void WheelChanged(float x, float y);
	
private:
	mPixbuf * fPixbuf;
	AoBookWindow * fOwner;
	KeyFilter * fKeyFilter;
	
	void _InitBuffer(BSize size);
};

PageView::PageView(AoBookWindow *owner, BSize size)
	: BView(BRect(0, 0, size.width, size.height),
			"page",
			B_FOLLOW_ALL_SIDES,
			B_WILL_DRAW),
	  fPixbuf(NULL),
	  fOwner(owner),
	  fKeyFilter(new KeyFilter(this))
{
	_InitBuffer(size);
	AddFilter(fKeyFilter);
}

PageView::~PageView()
{
	if (fPixbuf != NULL) {
		mPixbufFree(fPixbuf);
	}
	//delete fKeyFilter;
}

void PageView::_InitBuffer(BSize size)
{
	if (fPixbuf != NULL) {
		mPixbufFree(fPixbuf);
	}
	fPixbuf = mPixbufCreate((int)size.width, (int)size.height);
}

void PageView::MouseDown(BPoint point)
{
	uint32 buttons;
	GetMouse(&point, &buttons);
	BMessage msg(A_MOUSE_CLICK);
	msg.AddInt32("buttons", buttons);
	fOwner->PostMessage(&msg);
}

void PageView::WheelChanged(float x, float y)
{
	int32 count = floorf(y);
	BMessage msg(A_WHEEL_CHANGE);
	msg.AddInt32("wheel", count);
	fOwner->PostMessage(&msg);
}

void PageView::MessageReceived(BMessage *msg)
{
	switch(msg->what)
	{
		case B_MOUSE_WHEEL_CHANGED:
		{
			float x = 0;
			float y = 0;
			if (msg->FindFloat("be:wheel_delta_y", &y) == B_OK &&
			    msg->FindFloat("be:wheel_delta_x", &x) == B_OK) {
				WheelChanged(x, y);
			}
			break;
		}
		case A_RESIZE_WINDOW:
		{
			BRect rect;
			if (msg->FindRect("rect", &rect) == B_OK) {
				_InitBuffer(rect.Size());
				ResizeTo(rect.Width(), rect.Height());
			}
			break;
		}
		case B_KEY_DOWN:
		{
			fOwner->MessageReceived(msg);
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}

void PageView::Draw(BRect rect)
{
	if (fPixbuf == NULL) {
		return;
	}
	GlobalData *g = GDAT;
	if (g->bNowThread) {
		Invalidate();
		return;
	}
	
	__mPixbufHaiku *pHaiku = (__mPixbufHaiku *)fPixbuf;
	if (pHaiku != NULL && pHaiku->bitmap != NULL) {
		// resize bitmap for background
		BRect br = pHaiku->bitmap->Bounds();
		BRect wr = Bounds();
		int width = br.Width();
		int height = br.Height();
		if (width < wr.Width() || height < wr.Height()) {
			width = width < wr.Width() ? wr.Width() : width;
			height = height < wr.Height() ? wr.Height() : height;
			_InitBuffer(BSize(width, height));
		}
		
		if (!g->style->imgBkgnd) {
			mPixbufFillBox(fPixbuf, 0, 0, width, height, 
					mRGBtoPix(g->style->colBkgnd));
		} else {
			if (g->style->bkgnd_imgtype == 0) {
				mPixbufScaleImageBuf_oversamp(fPixbuf, 
						0, 0, width, height, g->style->imgBkgnd, 5);
			} else {
				mPixbufTileImageBuf(fPixbuf,
						0, 0, width, height, g->style->imgBkgnd);
			}
		}
		
		// page contents
		aoDrawPage(g->layout, fPixbuf, g->curpage);
		
		// draw bitmap on the view
		DrawBitmap(pHaiku->bitmap, BPoint(0, 0));
	}
}


AoBookWindow::AoBookWindow(BPoint pos)
	: BWindow(
		BRect(20, 20, 400, 300),
		APPNAME,
		B_TITLED_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL, 
		B_AUTO_UPDATE_SIZE_LIMITS),
	  fPageDialog(NULL),
	  fLineDialog(NULL),
	  fHeadingDialog(NULL),
	  fStyleOptDialog(NULL),
	  fEnvOptDialog(NULL),
	  fBookmarkDialog(NULL),
	  fBookmarkCommentDialog(NULL),
	  fRecentMenu(NULL),
	  fStyleMenu(NULL)
{
	// Create file panel here to avoid 
	// crash of localization catalog while deallocation.
	FileFilter *filter = new FileFilter();
	filter->AddFileFilter(B_TRANSLATE("All files (*.*)"), "");
	filter->AddFileFilter(B_TRANSLATE("Text files (*.txt)"), ".txt");
	filter->AddFileFilter(B_TRANSLATE("Zip files (*.zip)"), ".zip");
	filter->SetFilterIndex(0);
	fFileChooser = std::unique_ptr<FileChooser>(
						new FileChooser(this, B_OPEN_PANEL, filter, true));
	
	// todo, how to calculate height of the title bar?
	if (pos.x < 0) {
		pos.x = 5;
	}
	if (pos.y < 0) {
		pos.y = 30;
	}
	MoveTo(pos);
	
	fPageView = new PageView(this, BSize(400, 300));
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(_CreateMenu())
		.Add(fPageView);
	
	mfUpdateChangeStyle(TRUE);
	fPageView->MakeFocus(true);
	
	_InitAccKeys();
}

AoBookWindow::~AoBookWindow()
{
	if (fFileChooser && fFileChooser->Lock()) {
		fFileChooser->Quit();
	}
}

bool AoBookWindow::QuitRequested()
{
	BPoint point = Frame().LeftTop();
	GDAT->mainwin_point.x = ceil(point.x);
	GDAT->mainwin_point.y = ceil(point.y);
	
	BMessage msg(A_WINDOW_CLOSE);
	be_app->PostMessage(&msg);
}

void AoBookWindow::_InitAccKeys()
{
	for (uint32_t *p = GDAT->shortcutkey; *p; ++p) {
		fAccKeys.insert(std::make_pair(*p >> 16, *p & 0xffff));
	}
	// tool
	mStr str = MSTR_INIT;
	
	for (int i = 0; i < TOOLITEM_NUM; ++i) {
		if (mStrIsEmpty(GDAT->strTool + i)) {
			break;
		}
		mStrGetSplitText(&str, GDAT->strTool[i].buf, '\t', 2);
		
		int key = mStrToInt(&str);
		if (key != 0) {
			fAccKeys.insert(std::make_pair(key, CMD_TOOL));
		}
	}
	
	mStrFree(&str);
}

void AoBookWindow::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case TRMENU_FILE_OPEN:
		{
			_Open();
			break;
		}
		case TRMENU_FILE_RELOAD:
		{
			// todo, clear heading dialog, local bookmarks, and others
			mfReloadFile();
			break;
		}
		case TRMENU_FILE_NEXTFILE:
		{
			_NextFile(true);
			break;
		}
		case TRMENU_FILE_PREVFILE:
		{
			_NextFile(false);
			break;
		}
		case CMD_EXIT:
		{
			BMessage mess(B_QUIT_REQUESTED);
			PostMessage(&mess);
			break;
		}
		case TRMENU_MOVE_NEXT:
		{
			_NextPage(true);
			break;
		}
		case TRMENU_MOVE_PREV:
		{
			_NextPage(false);
			break;
		}
		case TRMENU_MOVE_TOP:
		{
			_MovePage(true);
			break;
		}
		case TRMENU_MOVE_BOTTOM:
		{
			_MovePage(false);
			break;
		}
		case TRMENU_MOVE_CAPTION:
		{
			if (fHeadingDialog == NULL) {
				fHeadingDialog = new HeadingDialog(this);
			}
			if (fHeadingDialog->IsHidden()) {
				fHeadingDialog->ShowDialog(Frame());
			} else {
				fHeadingDialog->Activate(true);
			}
			break;
		}
		case A_HEADING_DIALOG_CLOSED:
		{
			fHeadingDialog = NULL;
			break;
		}
		case TRMENU_BM_LIST:
		{
			if (fBookmarkDialog == NULL) {
				fBookmarkDialog = new BookmarkDialog(this);
			}
			if (fBookmarkDialog->IsHidden()) {
				fBookmarkDialog->ShowDialog(Frame());
				GDAT->bkmarkwin_show = 1;
			} else {
				fBookmarkDialog->Activate(true);
			}
			break;
		}
		case A_BOOKMARK_DIALOG_CLOSED:
		{
			fBookmarkDialog = NULL;
			GDAT->bkmarkwin_show = 0;
			break;
		}
		case CMD_BOOKMARK_ADD_TO_GLOBAL:
		{
			BmItemGlobal *pg = BookmarkGlobal_add();
			if (pg != NULL) {
				GDAT->fModify |= MODIFY_BKMARK_GLOBAL;
				
				if (fBookmarkDialog != NULL) {
					fBookmarkDialog->PostMessage(
										BookmarkDialog::GLOBAL_ADD_NEW);
				}
			}
			break;
		}
		case CMD_BOOKMARK_ADD_TO_LOCAL:
		{
			if (GLOBAL_ISNOT_HAVE_TEXT) {
				return;
			}
			if ((GDAT->optflags & OPTFLAGS_LOCALBKM_NO_COMMENT) == 0) {
				if (fBookmarkCommentDialog == NULL) {
					fBookmarkCommentDialog = new InputDialog(this,
							B_TRANSLATE("Comment"),
							B_TRANSLATE("Comment (leave empty with no comment)"),
							InputDialog::IT_STRING,
							A_LOCAL_BOOKMARK_COMMENT, 
							A_LOCAL_BOOKMARK_COMMENT_DIALOG_CLOSED);
				}
				if (fBookmarkCommentDialog->IsHidden()) {
					fBookmarkCommentDialog->ShowDialog(Frame());
				} else {
					fBookmarkCommentDialog->Activate();
				}
			} else {
				// add local comment without any comment
				BMessage mess(A_LOCAL_BOOKMARK_COMMENT);
				MessageReceived(&mess);
			}
			break;
		}
		case A_LOCAL_BOOKMARK_COMMENT_DIALOG_CLOSED:
		{
			fBookmarkCommentDialog = NULL;
			break;
		}
		case A_LOCAL_BOOKMARK_COMMENT:
		{
			if (GLOBAL_ISNOT_HAVE_TEXT) {
				return;
			}
			const char *comment = NULL;
			msg->FindString("value", &comment);
			mStr str = MSTR_INIT;
			mStrSetText(&str, comment);
			
			BmItemLocal *pi = BookmarkLocal_add(&str);
			mStrFree(&str);
			if (pi != NULL) {
				if (fBookmarkDialog != NULL) {
					fBookmarkDialog->PostMessage(
								BookmarkDialog::LOCAL_ADD_NEW);
				}
			}
			break;
		}
		case TRMENU_OPT_STYLE:
		{
			if (fStyleOptDialog == NULL) {
				fStyleOptDialog = new StyleOptDialog(this);
			}
			if (fStyleOptDialog->IsHidden()) {
				fStyleOptDialog->ShowDialog(Frame());
			} else {
				fStyleOptDialog->Activate(true);
			}
			break;
		}
		case A_STYLE_OPT_DIALOG_CLOSED:
		{
			fStyleOptDialog = NULL;
			break;
		}
		case TRMENU_OPT_ENV:
		{
			if (fEnvOptDialog == NULL) {
				fEnvOptDialog = new EnvOptDialog(this);
			}
			if (fEnvOptDialog->IsHidden()) {
				fEnvOptDialog->ShowDialog(Frame());
			} else {
				fEnvOptDialog->Activate(true);
			}
			break;
		}
		case A_ENV_OPT_DIALOG_CLOSED:
		{
			fEnvOptDialog = NULL;
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			int32 code = 0; // default AUTO
			if (msg->FindRef("refs", &ref) == B_OK &&
			    msg->FindInt32("encoding", &code) == B_OK) {
				_OpenFile(ref, code);
			}
			break;
		}
		case TRMENU_MOVE_PAGENO:
		{
			if (fPageDialog == NULL) {
				fPageDialog = new InputDialog(this, 
							B_TRANSLATE("Page number"),
							B_TRANSLATE("Page number (%d-%d)"),
							InputDialog::IT_INTEGER, 
							A_GOTO_PAGE, A_PAGE_DIALOG_CLOSED);
			}
			mStr str = MSTR_INIT;
			mStrSetFormat(&str, B_TRANSLATE("Page number (%d-%d)"),
							1, GDAT->layout->pagenum);
			BMessage mess(InputDialog::SET_LABEL);
			mess.AddString("label", str.buf);
			fPageDialog->PostMessage(&mess);
			mStrFree(&str);
			
			if (fPageDialog->IsHidden()) {
				fPageDialog->ShowDialog(Frame());
			} else {
				fPageDialog->Activate(true);
			}
			break;
		}
		case A_PAGE_DIALOG_CLOSED:
		{
			fPageDialog = NULL;
			break;
		}
		case TRMENU_MOVE_LINENO:
		{
			if (fLineDialog == NULL) {
				fLineDialog = new InputDialog(this,
							B_TRANSLATE("Line number"),
							B_TRANSLATE("Line number (1-)"), 
							InputDialog::IT_INTEGER, 
							A_GOTO_LINE, A_LINE_DIALOG_CLOSED);
			}
			if (fLineDialog->IsHidden()) {
				fLineDialog->ShowDialog(Frame());
			} else {
				fLineDialog->Activate(true);
			}
			break;
		}
		case A_LINE_DIALOG_CLOSED:
		{
			fLineDialog = NULL;
			break;
		}
		case A_GOTO_PAGE:
		{
			int32 value;
			if (msg->FindInt32("value", &value) == B_OK) {
				_MovePageNo(value -1);
			}
			break;
		}
		case A_GOTO_LINE:
		{
			int32 value;
			if (msg->FindInt32("value", &value) == B_OK) {
				_MoveLineNo(value -1);
			}
			break;
		}
		case A_LOAD_FILE:
		{
			const char *path;
			int32 lineno;
			if (msg->FindString("path", &path) == B_OK &&
				msg->FindInt32("lineno", &lineno) == B_OK) {
				
				mfLoadTextFile(path, -1, lineno, FALSE);
			}
			break;
		}
		case B_ABOUT_REQUESTED:
		{
			be_app->MessageReceived(msg);
			break;
		}
		case A_SHOW_BOOKMARK_WIN:
		{
			PostMessage(TRMENU_BM_LIST);
			break;
		}
		case A_UPDATE_WINDOW:
		{
			fPageView->Invalidate();
			break;
		}
		case A_RESIZE_WINDOW:
		{
			BRect rect;
			if (msg->FindRect("rect", &rect) == B_OK) {
				BView * menuView = FindView("MenuBar");
				if (menuView != NULL) {
					// add menubar height
					BSize windowSize = rect.Size();
					windowSize.height += menuView->Bounds().Height();
					ResizeTo(windowSize.width, windowSize.height);
				} else {
					ResizeTo(rect.Width(), rect.Height());
				}
				fPageView->MessageReceived(msg);
			}
			break;
		}
		case A_SET_TITLE:
		{
			const char *title;
			if (msg->FindString("title", &title) == B_OK) {
				SetTitle(title);
			}
			break;
		}
		case A_UPDATE_RECENT_FILE_MENU:
		{
			_UpdateRecentFileMenu();
			break;
		}
		case A_RECENT_MENU_CHOOSE:
		{
			int32 index;
			if (msg->FindInt32("index", &index) == B_OK) {
				_ChooseRecentFile(index);
			}
			break;
		}
		case A_MOUSE_CLICK:
		{
			int32 buttons;
			if (msg->FindInt32("buttons", &buttons) == B_OK) {
				if ((buttons & B_PRIMARY_MOUSE_BUTTON) > 0) {
					_MouseFunc(MOUSECTRL_BTT_LEFT);
				} else if ((buttons & B_SECONDARY_MOUSE_BUTTON) > 0) {
					_MouseFunc(MOUSECTRL_BTT_RIGHT);
				}
			}
			break;
		}
		case A_WHEEL_CHANGE:
		{
			int32 wheel;
			if (msg->FindInt32("wheel", &wheel) == B_OK) {
				if (wheel > 0) {
					_MouseFunc(MOUSECTRL_BTT_SCROLL_DOWN);
				} else if (wheel < 0) {
					_MouseFunc(MOUSECTRL_BTT_SCROLL_UP);
				}
			}
			break;
		}
		case A_STYLE_MENU_CHOOSE:
		{
			const char *name;
			if (msg->FindString("name", &name) == B_OK) {
				mBool relayout = StyleChangeByName(name);
				mfUpdateChangeStyle(relayout);
			}
			break;
		}
		case A_STYLE_MENU_UPDAGE:
		{
			_UpdateStyleMenu();
			break;
		}
		case A_STYLE_UPDATE:
		{
			mfUpdateChangeStyle(TRUE);
			break;
		}
		case A_UPDATE_TOOL_MENU:
		{
			_UpdateToolMenu();
			break;
		}
		case B_KEY_DOWN:
		{
			int32 key = msg->GetInt32("key", 0);
			uint8 byte = (uint8)msg->GetInt8("byte", 0);
			uint32 mod = (uint32)msg->GetInt32("modifiers", 0);
			uint32 c = (uint32)msg->GetInt32("raw_char", 0);
			int32 k = ParseHaikuKey(byte, key, c, mod);
			if (mod & B_COMMAND_KEY) {
				k |= MACCKEY_CTRL;
			}
			if (mod & B_SHIFT_KEY) {
				k |= MACCKEY_SHIFT;
			}
			
			auto it = fAccKeys.find((uint16)k);
			if (it != fAccKeys.end()) {
				if (it->second != CMD_TOOL) {
					PostMessage(it->second);
				} else {
					mStr str = MSTR_INIT;
					for (int i = 0; i < TOOLITEM_NUM; ++i) {
						if (mStrIsEmpty(GDAT->strTool + i)) {
							break;
						}
						mStrGetSplitText(&str, GDAT->strTool[i].buf, '\t', 2);
						int toolKey = mStrToInt(&str);
						if (toolKey == k && toolKey != 0) {
							mfExecTool(i);
							break;
						}
					}
					mStrFree(&str);
				}
			}
			break;
		}
		case A_ACC_KEYS_CHANGED:
		{
			_InitAccKeys();
			break;
		}
		case CMD_TOOL:
		{
			int32 index;
			if (msg->FindInt32("index", &index) == B_OK) {
				mfExecTool(index);
			}
			break;
		}
		case CMD_HELP_MANUAL:
		{
			mStr str = MSTR_INIT;
			mStrSetText(&str, "/boot/system/apps/WebPositive /boot/system/data/aobook/doc/manual.html");
			mExec(str.buf);
			mStrFree(&str);
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

void AoBookWindow::_Open()
{
	fFileChooser->Show();
}

void AoBookWindow::_NextFile(bool nxt)
{
	if (nxt) {
		mfLoadNextPrevFile(FALSE);
	} else {
		mfLoadNextPrevFile(TRUE);
	}
}

void AoBookWindow::_NextPage(bool nxt)
{
	if (nxt) {
		mfMovePage(PAGENO_NEXT);
	} else {
		mfMovePage(PAGENO_PREV);
	}
}

void AoBookWindow::_MovePage(bool first)
{
	if (first) {
		mfMovePage(PAGENO_HOME);
	} else {
		mfMovePage(PAGENO_END);
	}
}

void AoBookWindow::_MovePageNo(int32 page)
{
	if (0 <= page && page < GDAT->layout->pagenum) {
		mfMovePage(page);
	}
}

void AoBookWindow::_MoveLineNo(int32 line)
{
	if (0 <= line) {
		mfMovePageToLine(line);
	}
}

void AoBookWindow::_ChooseRecentFile(int32 index)
{
	if (0 <= index && index < RECENTFILE_NUM &&
	    !mStrIsEmpty(GDAT->strRecentFile + index)) {
		mfLoadTextFileFromRecent(index);
	}
}

void AoBookWindow::_OpenFile(const entry_ref &ref, int32 code)
{
	BEntry entry(&ref);
	BPath path;
	entry.GetPath(&path);
	mStrPathGetDir(&GDAT->strOpenPath, path.Path());
	// code specifies encoding to load
	mfLoadTextFile(path.Path(), code, 0, FALSE);
}

void AoBookWindow::_MouseFunc(int32 f)
{
	if (MOUSECTRL_BTT_LEFT <= f && f < MOUSECTRL_BTT_NUM) {
		uint8_t c = GDAT->mousectrl[f];
		switch (c)
		{
			case MOUSECTRL_CMD_NEXTPAGE:
			{
				_NextPage(true);
				break;
			}
			case MOUSECTRL_CMD_PREVPAGE:
			{
				_NextPage(false);
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

void AoBookWindow::_UpdateRecentFileMenu()
{
	fRecentMenu->RemoveItems(0, fRecentMenu->CountItems());
	
	// add recent files
	mStr str = MSTR_INIT;
	mStr fname = MSTR_INIT;
	
	for (int i = 0; i < RECENTFILE_NUM; ++i) {
		if (mStrIsEmpty(GDAT->strRecentFile + i)) {
			break;
		}
		
		int code, line;
		GlobalGetRecentFileInfo(i, &fname, &code, &line);
		mStrSetFormat(&str, "%s [%s] %d",
				fname.buf, aoTextGetCodeName(code), line + 1);
		
		BMessage *msg = new BMessage(A_RECENT_MENU_CHOOSE);
		msg->AddInt32("index", i);
		fRecentMenu->AddItem(new BMenuItem(str.buf, msg));
	}
	
	mStrFree(&str);
	mStrFree(&fname);
}

void AoBookWindow::_UpdateToolMenu()
{
	fToolMenu->RemoveItems(0, fToolMenu->CountItems());
	
	mStr str = MSTR_INIT;
	
	for (int i = 0; i < TOOLITEM_NUM; ++i) {
		mStr tool = GDAT->strTool[i];
		if (mStrIsEmpty(&tool)) {
			break;
		}
		
		mStrEmpty(&str);
		mStrGetSplitText(&str, tool.buf, '\t', 0);
		BMessage *msg = new BMessage(CMD_TOOL);
		msg->AddInt32("index", i);
		fToolMenu->AddItem(new BMenuItem(str.buf, msg));
	}
	
	mStrFree(&str);
}

void AoBookWindow::_UpdateStyleMenu()
{
	fStyleMenu->RemoveItems(0, fStyleMenu->CountItems());
	
	// add styles
	mStr str = MSTR_INIT;
	mAppGetConfigPath(&str, "style");
	mDirEntry *dir = mDirEntryOpen(str.buf);
	if (dir != NULL) {
		int cnt = 0;
		while (mDirEntryRead(dir) && cnt < STYLE_MAXNUM) {
			if (mDirEntryIsDirectory(dir)) {
				continue; 
			}
			if (mDirEntryIsEqExt(dir, "conf")) {
				mStrPathGetFileNameNoExt(&str, mDirEntryGetFileName(dir));
				
				BMessage *msg = new BMessage(A_STYLE_MENU_CHOOSE);
				msg->AddString("name", str.buf); // can we use label as its name?
				
				fStyleMenu->AddItem(new BMenuItem(str.buf, msg));
			}
			
			cnt++;
		}
		mDirEntryClose(dir);
	}
	mStrFree(&str);
	
	if (fStyleMenu->CountItems() == 0) {
		BMenuItem *item = new BMenuItem(GDAT->style->strName.buf, 
								new BMessage());
		item->SetMarked(true);
		fStyleMenu->AddItem(item);
	} else {
		BMenuItem * item = fStyleMenu->FindItem(GDAT->style->strName.buf);
		if (item != NULL) {
			item->SetMarked(true);
		}
	}
}

BMenuBar * AoBookWindow::_CreateMenu()
{
	BMenuBar *bar = new BMenuBar("MenuBar");
	
	fRecentMenu = new BMenu(B_TRANSLATE("Open"));
	fStyleMenu = new BMenu(B_TRANSLATE("Style"));
	fStyleMenu->SetRadioMode(true);
	fToolMenu = new BMenu(B_TRANSLATE("Tool"));
	
	BMenu *menu = new BMenu(B_TRANSLATE("File"));
	BLayoutBuilder::Menu<>(menu)
		.AddItem(new BMenuItem(fRecentMenu, new BMessage(TRMENU_FILE_OPEN)))
		// todo, add recent sub menu to open
		.AddItem(B_TRANSLATE("Reload"), TRMENU_FILE_RELOAD)
		.AddSeparator()
		.AddItem(B_TRANSLATE("Next file"), TRMENU_FILE_NEXTFILE)
		.AddItem(B_TRANSLATE("Previous file"), TRMENU_FILE_PREVFILE)
		.AddSeparator()
		.AddItem(B_TRANSLATE("Exit"), CMD_EXIT, 'Q', B_COMMAND_KEY);
	bar->AddItem(menu);
	
	menu = new BMenu(B_TRANSLATE("Move"));
	BLayoutBuilder::Menu<>(menu)
		.AddItem(B_TRANSLATE("Next page"), TRMENU_MOVE_NEXT)
		.AddItem(B_TRANSLATE("Previous page"), TRMENU_MOVE_PREV)
		.AddSeparator()
		.AddItem(B_TRANSLATE("First page"), TRMENU_MOVE_TOP)
		.AddItem(B_TRANSLATE("Last page"), TRMENU_MOVE_BOTTOM)
		.AddSeparator()
		.AddItem(B_TRANSLATE("Go to page..."), TRMENU_MOVE_PAGENO)
		.AddItem(B_TRANSLATE("Go to line..."), TRMENU_MOVE_LINENO)
		.AddSeparator()
		.AddItem(B_TRANSLATE("Heading..."), TRMENU_MOVE_CAPTION);
	bar->AddItem(menu);
	
	menu = new BMenu(B_TRANSLATE("Bookmark"));
	BLayoutBuilder::Menu<>(menu)
		.AddItem(B_TRANSLATE("Bookmarks..."), TRMENU_BM_LIST)
		.AddSeparator()
		.AddItem(B_TRANSLATE("Add to global"), CMD_BOOKMARK_ADD_TO_GLOBAL)
		.AddItem(B_TRANSLATE("Add to local..."), CMD_BOOKMARK_ADD_TO_LOCAL);
	bar->AddItem(menu);
	
	menu = new BMenu(B_TRANSLATE("Option"));
	BLayoutBuilder::Menu<>(menu)
		.AddItem(fToolMenu)
		.AddItem(fStyleMenu)
		.AddSeparator()
		.AddItem(B_TRANSLATE("Style settings..."), TRMENU_OPT_STYLE)
		.AddItem(B_TRANSLATE("Settings..."), TRMENU_OPT_ENV);
	bar->AddItem(menu);
	
	menu = new BMenu(B_TRANSLATE("Help"));
	BLayoutBuilder::Menu<>(menu)
		.AddItem(B_TRANSLATE("Manual"), CMD_HELP_MANUAL)
		.AddSeparator()
		.AddItem(B_TRANSLATE("Version information..."), B_ABOUT_REQUESTED);
	bar->AddItem(menu);
	
	_UpdateRecentFileMenu();
	_UpdateStyleMenu();
	_UpdateToolMenu();
	return bar;
}
