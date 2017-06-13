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

#include "EnvOptDialog.h"

#include "common.h"
#include "mDef.h"
#include "mGui.h"
#include "globaldata.h"
#include "keys.h"
#include "mStr.h"

#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <LayoutUtils.h>
#include <ListView.h>
#include <MessageFilter.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <String.h>
#include <StringItem.h>
#include <StringView.h>
#include <TabView.h>
#include <View.h>

#include <string>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"

enum DialogResult
{
	DIALOG_OK = 'btok',
	DIALOG_CANCEL = 'btcl',
};


class SettingsView : public BView
{
public:
	SettingsView();
	void InitData();
	void StoreData();
	
	static const int32 A_OPEN_LAST = 'aool';
	static const int32 A_NO_COMMENT_LOCAL_BOOKMARK = 'aonl';
	
private:
	BCheckBox *fOpenLastCB;
	BCheckBox *fNoCommentLocalBookmark;
	
};

SettingsView::SettingsView()
	: BView(B_TRANSLATE("Settings"), B_WILL_DRAW)
{
	fOpenLastCB = new BCheckBox("openlastcb", 
			B_TRANSLATE("Open last file at starting."), 
			new BMessage(A_OPEN_LAST));
	fNoCommentLocalBookmark = new BCheckBox("nocommentlocalbookmarkcb", 
			B_TRANSLATE("No comments for local bookmarks."), 
			new BMessage(A_NO_COMMENT_LOCAL_BOOKMARK));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(6)
		// todo, no GUI font option
		.Add(fOpenLastCB)
		.Add(fNoCommentLocalBookmark)
		.AddGlue();
}

void SettingsView::InitData()
{
	fOpenLastCB->SetValue((GDAT->optflags & OPTFLAGS_OPEN_LASTFILE) > 0 ? 
							B_CONTROL_ON : B_CONTROL_OFF);
	fNoCommentLocalBookmark->SetValue(
			(GDAT->optflags & OPTFLAGS_LOCALBKM_NO_COMMENT) > 0 ?
			B_CONTROL_ON : B_CONTROL_OFF);
}

void SettingsView::StoreData()
{
	uint32 flags = 0;
	if (fOpenLastCB->Value() == B_CONTROL_ON) {
		flags |= OPTFLAGS_OPEN_LASTFILE;
	}
	if (fNoCommentLocalBookmark->Value() == B_CONTROL_ON) {
		flags |= OPTFLAGS_LOCALBKM_NO_COMMENT;
	}
	GDAT->optflags = flags;
}


class EnvOptMouseView : public BView
{
public:
	EnvOptMouseView();
	void InitData();
	void StoreData();
	virtual void MessageReceived(BMessage *msg);
	
	static const int32 A_MOUSE_BUTTON_CHOOSE = 'mobc';
	static const int32 A_MOUSE_FUNC_CHOOSE = 'mofc';
	
private:
	BListView * fButtonList;
	BListView * fFuncList;
	
	uint8 fMouseCtrl[MOUSECTRL_BTT_NUM];
	
};

EnvOptMouseView::EnvOptMouseView()
	: BView(B_TRANSLATE("Mouse"), B_WILL_DRAW)
{
	fButtonList = new BListView("buttonlist");
	fButtonList->SetSelectionMessage(new BMessage(A_MOUSE_BUTTON_CHOOSE));
	fButtonList->SetTarget(this);
	fButtonList->AddItem(new BStringItem(B_TRANSLATE("Left button")));
	fButtonList->AddItem(new BStringItem(B_TRANSLATE("Right button")));
	fButtonList->AddItem(new BStringItem(B_TRANSLATE("Wheel up")));
	fButtonList->AddItem(new BStringItem(B_TRANSLATE("Wheel down")));
	
	fFuncList = new BListView("funclist");
	fFuncList->SetSelectionMessage(new BMessage(A_MOUSE_FUNC_CHOOSE));
	fFuncList->SetTarget(this);
	fFuncList->AddItem(new BStringItem(B_TRANSLATE("None")));
	fFuncList->AddItem(new BStringItem(B_TRANSLATE("Next page")));
	fFuncList->AddItem(new BStringItem(B_TRANSLATE("Previous page")));
	
	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.SetInsets(6)
		.Add(new BScrollView("buttonscroll", fButtonList, 0, false, false))
		.Add(new BScrollView("funcscroll", fFuncList, 0, false, false));
}

void EnvOptMouseView::InitData()
{
	// copy current data into work data
	memcpy(&fMouseCtrl, &GDAT->mousectrl, MOUSECTRL_BTT_NUM);
	
	// function is choosen in the event
	fButtonList->Select(0, false);
}

void EnvOptMouseView::StoreData()
{
	memcpy(&GDAT->mousectrl, &fMouseCtrl, MOUSECTRL_BTT_NUM);
}

void EnvOptMouseView::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case A_MOUSE_BUTTON_CHOOSE:
		{
			int32 n = fButtonList->CurrentSelection(0);
			if (0 <= n && n < MOUSECTRL_BTT_NUM) {
				fFuncList->Select(fMouseCtrl[n], false);
			}
			break;
		}
		case A_MOUSE_FUNC_CHOOSE:
		{
			int32 button = fButtonList->CurrentSelection(0);
			if (0 <= button && button < MOUSECTRL_BTT_NUM) {
				int32 n = fFuncList->CurrentSelection(0);
				if (0 <= n && n < MOUSECTRL_CMD_NUM) {
					if (fMouseCtrl[button] != n) {
						fMouseCtrl[button] = n;
					}
				}
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


class KeyTrackControl : public BControl
{
public:
	KeyTrackControl(const char *name, BHandler *handler);
	~KeyTrackControl();
	
	virtual void MessageReceived(BMessage *msg);
	virtual void MouseDown(BPoint where);
	virtual void Draw(BRect updateRect);
	virtual void GetPreferredSize(float *width, float *height);
	virtual BSize MinSize();
	virtual BSize MaxSize();
	virtual BSize PreferredSize();
	virtual void LayoutInvalidated(bool descendants);
	
	uint32 Key();
	void SetKey(uint32 key);
	
	static const int32 KEY_CHANGED = 'ktkc';
	
private:
	BString *fText;
	BSize fPreferredSize;
	uint32 fKey;
	BHandler * fOwner;
	
	void _UpdateLabel();
	BSize _ValidatePreferredSize();
	
};

KeyTrackControl::KeyTrackControl(const char *name, BHandler *handler)
	: BControl(name, "", new BMessage(), B_NAVIGABLE | B_WILL_DRAW),
	  fText(new BString()),
	  fPreferredSize(-1, -1),
	  fOwner(handler)
{
	AddFilter(new KeyFilter(this));
}

KeyTrackControl::~KeyTrackControl()
{
	delete fText;
}

void KeyTrackControl::MouseDown(BPoint where)
{
	MakeFocus(true);
}

void KeyTrackControl::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
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
			
			SetKey(k);
			
			if (fOwner != NULL) {
				BMessage mess(KEY_CHANGED);
				mess.AddInt32("key", fKey);
				fOwner->MessageReceived(&mess);
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

uint32 KeyTrackControl::Key()
{
	return fKey;
}

void KeyTrackControl::SetKey(uint32 key)
{
	fKey = key;
	_UpdateLabel();
	Invalidate();
}

void KeyTrackControl::_UpdateLabel()
{
	BString *str = new BString();
	AddModifierStr(str, fKey & MACCKEY_MODMASK);
	AddKeyStr(str, fKey & MACCKEY_KEYMASK);
	
	fText->SetTo(str->String());
	delete str;
}

void KeyTrackControl::Draw(BRect rect)
{
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	
	float width = StringWidth(fText->String(), fText->Length());
	BRect r = Bounds();
	float x = (r.Width() - width) / 2;
	DrawString(fText->String(), fText->Length(), 
			BPoint(x, r.Height() - fontHeight.descent - 4));
	
	uint32 flags = be_control_look->Flags(this);
	be_control_look->DrawTextControlBorder(this, r, rect, 
			LowColor(), flags);
}

void KeyTrackControl::GetPreferredSize(float *width, float *height)
{
	if (width != NULL) {
		*width = fPreferredSize.width;
	}
	if (height != NULL) {
		*height = fPreferredSize.height;
	}
}

BSize KeyTrackControl::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(),
			_ValidatePreferredSize());
}

BSize KeyTrackControl::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
			_ValidatePreferredSize());
}

BSize KeyTrackControl::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(),
			_ValidatePreferredSize());
}

void KeyTrackControl::LayoutInvalidated(bool descendants)
{
	fPreferredSize.Set(-1, -1);
}

BSize KeyTrackControl::_ValidatePreferredSize()
{
	if (fPreferredSize.width < 0) {
		font_height fontHeight;
		GetFontHeight(&fontHeight);
		float height = fontHeight.ascent + fontHeight.descent;
		height += 8;
		float width = 150.;
		fPreferredSize.Set(width, height);
		ResetLayoutInvalidation();
	}
	return fPreferredSize;
}


class EnvOptKeyView : public BView
{
public:
	EnvOptKeyView();
	virtual ~EnvOptKeyView();
	void InitData();
	void StoreData();
	virtual void MessageReceived(BMessage *msg);
	
	static const int32 A_KEY_CLEAR = 'aokc';
	static const int32 A_KEY_ALL_CLEAR = 'aoka';
	static const int32 A_KEY_FUNC_CHOOSE = 'aokf';
	
private:
	BListView * fCommandsList;
	BStringView * fCommandTC;
	KeyTrackControl * fKeyTC;
	
	uint32 * fShortcutkey;
	KeyFilter *fKeyFilter;
	
	void _Init(bool copy);
	
};

EnvOptKeyView::EnvOptKeyView()
	: BView(B_TRANSLATE("Key"), B_WILL_DRAW),
	  fShortcutkey(NULL),
	  fKeyFilter(new KeyFilter(this))
{
	fCommandsList = new BListView("commandslk");
	fCommandsList->SetSelectionMessage(new BMessage(A_KEY_FUNC_CHOOSE));
	
	fCommandTC = new BStringView("commandtck", "");
	//fCommandTC->SetAlignment(B_ALIGN_CENTER);
	
	fKeyTC = new KeyTrackControl("keytck", this);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6)
		.Add(new BScrollView("cmdlistsv", fCommandsList, 0, false, true))
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(new BStringView("commandlk", B_TRANSLATE("Command")), 0, 0)
			.Add(fCommandTC, 1, 0)
			.Add(new BStringView("keylk", B_TRANSLATE("Key")), 0, 1)
			.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING, 1, 1)
				.Add(fKeyTC)
				.Add(new BButton("clearbk", B_TRANSLATE("Clear"), 
						new BMessage(A_KEY_CLEAR)))
			.End()
			.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING, 1, 2)
			.AddGlue()
			.Add(new BButton("clearallbk", B_TRANSLATE("All clear"),
						new BMessage(A_KEY_ALL_CLEAR)))
		.End();
}

EnvOptKeyView::~EnvOptKeyView()
{
	delete[] fShortcutkey;
	//delete fKeyFilter;
}

struct Label
{
	int32 command;
	const char * label;
};

static const struct Label LABELS[] = {
	{TRMENU_FILE_OPEN, B_TRANSLATE("Open")},
	{TRMENU_FILE_RELOAD, B_TRANSLATE("Reload")},
	{TRMENU_FILE_NEXTFILE, B_TRANSLATE("Next file")},
	{TRMENU_FILE_PREVFILE, B_TRANSLATE("Previous file")},
	{TRMENU_MOVE_NEXT, B_TRANSLATE("Next page")},
	{TRMENU_MOVE_PREV, B_TRANSLATE("Previous page")},
	{TRMENU_MOVE_TOP, B_TRANSLATE("First page")},
	{TRMENU_MOVE_BOTTOM, B_TRANSLATE("Last page")},
	{TRMENU_MOVE_PAGENO, B_TRANSLATE("Go to page...")},
	{TRMENU_MOVE_LINENO, B_TRANSLATE("Go to line...")},
	{TRMENU_MOVE_CAPTION, B_TRANSLATE("Heading...")},
	{TRMENU_BM_LIST, B_TRANSLATE("Bookmarks...")},
	{TRMENU_OPT_STYLE, B_TRANSLATE("Style settings...")},
	{TRMENU_OPT_ENV, B_TRANSLATE("Settings...")},
	{0,0}
};

const char * _GetCommandLabel(uint32 cmd)
{
	for (int i = 0; ; ++i) {
		const struct Label v = LABELS[i];
		if (v.command == cmd) {
			return v.label;
			break;
		}
		if (v.label == NULL) {
			break;
		}
	}
	return NULL;
}

void _ConstructListLabel(BString *str, uint32 value)
{
	BString *keys = new BString();
	
	int32 cmd = value & 0xffff;
	const char *label = _GetCommandLabel(cmd);
	if (label != NULL) {
		str->SetTo(label);
		str->Append(" = ", 3);
		
		AddModifierStr(keys, (value >> 16) & MACCKEY_MODMASK);
		AddKeyStr(keys, (value >> 16) & MACCKEY_KEYMASK);
		
		str->Append(keys->String());
	}
	delete keys;
}

void EnvOptKeyView::InitData()
{
	_Init(true);
}

void EnvOptKeyView::_Init(bool copy)
{
	// clear list
	fCommandsList->MakeEmpty();
	
	if (copy) {
		int n = 0;
		for (uint32_t *p = GDAT->shortcutkey; *p; ++p, ++n) {
		}
		fShortcutkey = new uint32[n+1];
		uint32_t *p, *ps;
		for (p = GDAT->shortcutkey, ps = fShortcutkey; *p; ++p, ++ps) {
			*ps = *p;
		}
		fShortcutkey[n] = 0; // push zero at the end
	}
	
	BString *str = new BString();
	
	for (uint32_t *p = fShortcutkey; *p; ++p) {
		_ConstructListLabel(str, *p);
		fCommandsList->AddItem(new BStringItem(str->String()));
	}
	
	delete str;
	
	fCommandsList->Select(0);
}

void EnvOptKeyView::StoreData()
{
	uint32_t *p, *ps;
	for (p = GDAT->shortcutkey, ps = fShortcutkey; *p; ++p, ++ps) {
		*p = *ps;
	}
}

void EnvOptKeyView::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case KeyTrackControl::KEY_CHANGED:
		{
			int32 key;
			if (msg->FindInt32("key", &key) == B_OK) {
				int32 selected = fCommandsList->CurrentSelection(0);
				if (selected >= 0) {
					fShortcutkey[selected] &= 0xffff;
					fShortcutkey[selected] |= (key << 16);
					// udpate list entry
					BString *str = new BString();
					_ConstructListLabel(str, fShortcutkey[selected]);
					BStringItem * item = (BStringItem *)fCommandsList->ItemAt(selected);
					if (item != NULL) {
						item->SetText(str->String());
						fCommandsList->Invalidate();
					}
					delete str;
				}
			}
			break;
		}
		case A_KEY_FUNC_CHOOSE:
		{
			int32 selected = fCommandsList->CurrentSelection(0);
			if (selected >= 0) {
				uint32 p = fShortcutkey[selected];
				
				fCommandTC->SetText(LABELS[selected].label);
				fKeyTC->SetKey(p >> 16);
			}
			break;
		}
		case A_KEY_CLEAR:
		{
			fKeyTC->SetKey(0);
			
			BMessage mess(KeyTrackControl::KEY_CHANGED);
			mess.AddInt32("key", 0);
			MessageReceived(&mess);
			break;
		}
		case A_KEY_ALL_CLEAR:
		{
			for (uint32_t *p = fShortcutkey; *p; ++p) {
				*p &= 0xffff;
			}
			_Init(false);
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}


class EnvOptToolView : public BView
{
public:
	EnvOptToolView();
	virtual void MessageReceived(BMessage *msg);
	void InitData();
	bool StoreData();
	
	static const int32 A_TOOL_ADD = 'tlad';
	static const int32 A_TOOL_DELETE = 'tldl';
	static const int32 A_TOOL_REPLACE = 'tlrp';
	static const int32 A_TOOL_CLEAR = 'tlcr';
	static const int32 A_TOOL_LIST_CHOOSE = 'tllc';
	
private:
	BListView * fCommandsList;
	BTextControl * fNameTC;
	BTextControl * fCommandTC;
	KeyTrackControl * fShortcutKeyTC;
	
	mStr fTools[TOOLITEM_NUM];
	
};

EnvOptToolView::EnvOptToolView()
	: BView(B_TRANSLATE("Tools"), B_WILL_DRAW)
{
	fCommandsList = new BListView("list");
	fCommandsList->SetSelectionMessage(new BMessage(A_TOOL_LIST_CHOOSE));
	
	fNameTC = new BTextControl("nametc", "", "", new BMessage());
	fCommandTC = new BTextControl("commandtc", "", "", new BMessage());
	fShortcutKeyTC = new KeyTrackControl("shortcutkeytc", this);
	
	BTextView *desc = new BTextView("desc");
	desc->MakeEditable(false);
	desc->MakeSelectable(false);
	desc->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	desc->SetText(
		B_TRANSLATE("%f: file name, %l: line number, %c: character encoding"));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6)
		.AddGroup(B_HORIZONTAL)
			.Add(new BScrollView("scroll", fCommandsList, 0, false, true))
			.AddGroup(B_VERTICAL)
				.Add(new BButton("add", B_TRANSLATE("Add"), 
						new BMessage(A_TOOL_ADD)))
				.Add(new BButton("delete", B_TRANSLATE("Delete"), 
						new BMessage(A_TOOL_DELETE)))
				.AddGlue()
				.Add(new BButton("replace", B_TRANSLATE("Replace"), 
						new BMessage(A_TOOL_REPLACE)))
			.End()
		.End()
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(new BStringView("namel", B_TRANSLATE("Name")), 0, 0)
			.Add(fNameTC, 1, 0)
			.Add(new BStringView("commandl", B_TRANSLATE("Command")), 0, 1)
			.Add(fCommandTC, 1, 1)
			.Add(new BStringView("shortcutkeyl", 
					B_TRANSLATE("Shortcut key")), 0, 2)
			.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING, 1, 2)
				.Add(fShortcutKeyTC)
				.Add(new BButton("clearb", B_TRANSLATE("Clear"), 
					new BMessage(A_TOOL_CLEAR)))
			.End()
		.End()
		.Add(desc)
		.AddGlue();
}

void EnvOptToolView::InitData()
{
	mStrArrayInit(fTools, TOOLITEM_NUM);
	mStrArrayCopy(fTools, GDAT->strTool, TOOLITEM_NUM);
	
	mStr str = MSTR_INIT;
	
	for (int i = 0; i < TOOLITEM_NUM; ++i) {
		if (mStrIsEmpty(fTools + i)) {
			break;
		}
		mStrGetSplitText(&str, fTools[i].buf, '\t', 0);
		
		fCommandsList->AddItem(new BStringItem(str.buf));
	}
	
	mStrFree(&str);
}

bool EnvOptToolView::StoreData()
{
	bool ret = false;
	int i = 0;
	
	for (i = 0; i < TOOLITEM_NUM; ++i) {
		if (!mStrCompareEq(fTools + i, GDAT->strTool[i].buf)) {
			break;
		}
	}
	
	ret = i != TOOLITEM_NUM;
	
	for (i = 0; i < TOOLITEM_NUM; ++i) {
		if (mStrIsEmpty(fTools + i)) {
			mStrEmpty(GDAT->strTool + i);
		} else {
			mStrCopy(GDAT->strTool + i, fTools + i);
		}
	}
	
	mStrArrayFree(fTools, TOOLITEM_NUM);
	
	return ret;
}

void EnvOptToolView::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case KeyTrackControl::KEY_CHANGED:
		{
			break;
		}
		case A_TOOL_ADD:
		{
			if (fCommandsList->CountItems() < 10) {
				int no = fCommandsList->CountItems();
				mStrSetText(fTools + no, "New");
				fCommandsList->AddItem(new BStringItem("New"));
				fCommandsList->Select(fCommandsList->CountItems()-1);
				
				fNameTC->MakeFocus(true);
			}
			break;
		}
		case A_TOOL_DELETE:
		{
			int32 selected = fCommandsList->CurrentSelection(0);
			if (selected >= 0) {
				mStrArrayShiftUp(fTools, selected, TOOLITEM_NUM -1);
				fCommandsList->RemoveItem(selected);
				fCommandsList->Select(selected -1);
			}
			break;
		}
		case A_TOOL_REPLACE:
		{
			int32 selected = fCommandsList->CurrentSelection(0);
			if (selected >= 0) {
				mStr str = MSTR_INIT;
				mStr *pdst = &fTools[selected];
				
				mStrSetText(pdst, fNameTC->Text());
				if (mStrIsEmpty(pdst)) {
					mStrSetText(pdst, "empty");
				}
				
				mStrAppendChar(pdst, '\t');
				mStrAppendText(pdst, fCommandTC->Text());
				
				mStrAppendChar(pdst, '\t');
				mStrAppendInt(pdst, fShortcutKeyTC->Key());
				
				mStrFree(&str);
				
				BStringItem * item = (BStringItem *)fCommandsList->ItemAt(selected);
				if (item != NULL) {
					item->SetText(fNameTC->Text());
					fCommandsList->Invalidate();
				}
			}
			break;
		}
		case A_TOOL_CLEAR:
		{
			fShortcutKeyTC->SetKey(0);
			break;
		}
		case A_TOOL_LIST_CHOOSE:
		{
			int32 selected = fCommandsList->CurrentSelection(0);
			if (selected >= 0) {
				mStr str = MSTR_INIT;
				char *psrc = fTools[selected].buf;
				
				mStrGetSplitText(&str, psrc, '\t', 0);
				fNameTC->SetText(str.buf);
				
				mStrGetSplitText(&str, psrc, '\t', 1);
				fCommandTC->SetText(str.buf);
				
				mStrGetSplitText(&str, psrc, '\t', 2);
				fShortcutKeyTC->SetKey(mStrToInt(&str));
				
				mStrFree(&str);
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


EnvOptDialog::EnvOptDialog(BWindow *owner)
	: BWindow(
		BRect(50, 50, 400, 450),
		B_TRANSLATE("Environmental settings"),
		B_FLOATING_WINDOW_LOOK,
		B_MODAL_APP_WINDOW_FEEL,
		B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS |
		B_CLOSE_ON_ESCAPE),
	  fOwner(owner)
{
	fKeyView = new EnvOptKeyView();
	fMouseView = new EnvOptMouseView();
	fToolView = new EnvOptToolView();
	fSettingsView = new SettingsView();
	
	BTabView *tabview = new BTabView("tabview", B_WIDTH_FROM_LABEL);
	tabview->SetBorder(B_NO_BORDER);
	tabview->AddTab(fSettingsView);
	tabview->AddTab(fMouseView);
	tabview->AddTab(fKeyView);
	tabview->AddTab(fToolView);
	
	BButton *okButton = new BButton("ok", B_TRANSLATE("OK"),
				new BMessage(DIALOG_OK));
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_VERTICAL)
			.SetInsets(0, 6, 0, 0)
			.Add(tabview)
			.Add(new BSeparatorView(B_HORIZONTAL))
			.AddGroup(B_HORIZONTAL)
				.SetInsets(6)
				.AddGlue()
				.Add(okButton)
				.Add(new BButton("cancel", B_TRANSLATE("Cancel"),
					new BMessage(DIALOG_CANCEL)))
			.End()
		.End();
}

void EnvOptDialog::Quit()
{
	fOwner->PostMessage(A_ENV_OPT_DIALOG_CLOSED);
	BWindow::Quit();
}

void EnvOptDialog::ShowDialog(BRect rect)
{
	fSettingsView->InitData();
	fMouseView->InitData();
	fKeyView->InitData();
	fToolView->InitData();
	
	CenterIn(rect);
	Show();
}

void EnvOptDialog::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case DIALOG_OK:
		{
			Hide();
			
			fSettingsView->StoreData();
			fMouseView->StoreData();
			fKeyView->StoreData();
			bool updateTool = fToolView->StoreData();
			
			fOwner->PostMessage(A_ACC_KEYS_CHANGED);
			if (updateTool) {
				fOwner->PostMessage(A_UPDATE_TOOL_MENU);
			}
			Quit();
			break;
		}
		case DIALOG_CANCEL:
		{
			Hide();
			Quit();
			break;
		}
		case EnvOptMouseView::A_MOUSE_BUTTON_CHOOSE:
		{
			fMouseView->MessageReceived(msg);
			break;
		}
		case EnvOptMouseView::A_MOUSE_FUNC_CHOOSE:
		{
			fMouseView->MessageReceived(msg);
			break;
		}
		case EnvOptKeyView::A_KEY_FUNC_CHOOSE:
		case EnvOptKeyView::A_KEY_CLEAR:
		case EnvOptKeyView::A_KEY_ALL_CLEAR:
		{
			fKeyView->MessageReceived(msg);
			break;
		}
		case EnvOptToolView::A_TOOL_ADD:
		case EnvOptToolView::A_TOOL_DELETE:
		case EnvOptToolView::A_TOOL_REPLACE:
		case EnvOptToolView::A_TOOL_CLEAR:
		case EnvOptToolView::A_TOOL_LIST_CHOOSE:
		{
			fToolView->MessageReceived(msg);
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}
