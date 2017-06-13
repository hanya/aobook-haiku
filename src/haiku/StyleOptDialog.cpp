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

#include "StyleOptDialog.h"

#include "common.h"

#include <Alert.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <GraphicsDefs.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <RadioButton.h>
#include <SeparatorView.h>
#include <StringView.h>
#include <TabView.h>
#include <TextView.h>
#include <View.h>

#include <Spinner.h>

#include "ColorButton.h"
#include "FileChooser.h"
#include "FontButton.h"
#include "dialogs.h"

#include "mDef.h"
#include "mGui.h"
#include "mStr.h"
#include "mDirEntry.h"
#include "mList.h"
#include "style.h"
#include "stylelist.h"
#include "aoStyle.h"
#include "mAppDef.h"
#include "mFont.h"

#include "globaldata.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "aobook"

enum DialogResult
{
	DIALOG_OK = 'btok',
	DIALOG_CANCEL = 'btcl',
};


class EditStyles
{
public:
	mList *styles;
	StyleListItem *current;
	
	EditStyles();
	~EditStyles();
};

EditStyles::EditStyles()
	: styles(new mList),
	  current(NULL)
{
	styles->top = NULL;
	mListDeleteAll(styles);
}

EditStyles::~EditStyles()
{
	current = NULL;
	// todo, crash
	//mListDeleteAll(styles);
	delete styles;
}


static rgb_color mRgbColTo_rgb_color(mRgbCol col)
{
	rgb_color color = {(uint8)(M_GET_R(col) >> MAPP->r_shift_right), 
					   (uint8)(M_GET_G(col) >> MAPP->g_shift_right),
					   (uint8)(M_GET_B(col) >> MAPP->b_shift_right), 0xff};
	return color;
}

static mRgbCol rgb_colorTomRgbCol(rgb_color col)
{
	return (col.red << MAPP->r_shift_left) | 
			(col.green << MAPP->g_shift_left) |
			(col.blue << MAPP->b_shift_left);
}

class StyleOptBasicView : public BView
{
public:
	StyleOptBasicView();
	void MessageReceived(BMessage *msg);
	void InitData(EditStyles *editStyles);
	void StoreData();
	void UpdateData();
	void SetBgPath(const char *path);
	
	static const int32 CMD_BG_IMAGE_CHOOSE = 'cmic';
	
private:
	static const int32 CMD_STYLE_ADD = 'cmsa';
	static const int32 CMD_STYLE_DELETE = 'cmsd';
	
	static const int32 CMD_SINGLE = 'cmsn';
	static const int32 CMD_SPREAD = 'cmsp';
	
	static const int32 CMD_CHARS = 'cmch';
	static const int32 CMD_CHAR_PITCH = 'cmcp';
	static const int32 CMD_LINES = 'cmln';
	static const int32 CMD_LINE_PITCH = 'cmlp';
	
	static const int32 CMD_MARGIN_LEFT = 'cmml';
	static const int32 CMD_MARGIN_TOP = 'cmmt';
	static const int32 CMD_MARGIN_RIGHT = 'cmmr';
	static const int32 CMD_MARGIN_BOTTOM = 'cmmb';
	static const int32 CMD_SEP_CENTER = 'cmcs';
	
	static const int32 CMD_BG_IMAGE_FIT = 'cmif';
	static const int32 CMD_BG_IMAGE_TILE = 'cmit';
	static const int32 CMD_BG_COLOR = 'cmbc';
	static const int32 CMD_BC_IMAGE_INPUT = 'cmii';
	
	static const int32 CMD_HANGING = 'cmhn';
	static const int32 CMD_ILLUSTRATION = 'cmil';
	static const int32 CMD_PAGINATION = 'cmpg';
	static const int32 CMD_AS_LINE = 'cmal';
	
	BRadioButton * fSinglePage;
	BRadioButton * fSpreadPages;
	
	BSpinner * fChars;
	BSpinner * fCharacterPitch;
	BSpinner * fLines;
	BSpinner * fLinePitch;
	
	BSpinner * fMarginLeft;
	BSpinner * fMarginTop;
	BSpinner * fMarginRight;
	BSpinner * fMarginBottom;
	BSpinner * fSepCenter;
	
	ColorButton * fBgColorButton;
	BTextControl * fBgImageText;
	BPopUpMenu * fBgImageFlagPM;
	
	BCheckBox * fHanging;
	BCheckBox * fShowIllustration;
	BCheckBox * fPagination;
	BCheckBox * fAsLine;
	
	EditStyles * fEditStyles;
};

StyleOptBasicView::StyleOptBasicView()
	: BView(B_TRANSLATE("Basic"), B_WILL_DRAW),
	  fEditStyles(NULL)
{
	fSinglePage = new BRadioButton("single", B_TRANSLATE("Single"), 
						new BMessage(CMD_SINGLE));
	fSpreadPages = new BRadioButton("spread", B_TRANSLATE("Spread"),
						new BMessage(CMD_SPREAD));
	
	fChars = new BSpinner("chars", "", new BMessage(CMD_CHARS));
	fChars->SetRange(1, 1000);
	fCharacterPitch = new BSpinner("charpitch", "", new BMessage(CMD_CHAR_PITCH));
	fLines = new BSpinner("lines", "", new BMessage(CMD_LINES));
	fLines->SetRange(1, 1000);
	fLinePitch = new BSpinner("linepitch", "", new BMessage(CMD_LINE_PITCH));
	
	fMarginLeft = new BSpinner("left", "", new BMessage(CMD_MARGIN_LEFT));
	fMarginLeft->SetRange(0, 10000);
	fMarginTop = new BSpinner("top", "", new BMessage(CMD_MARGIN_TOP));
	fMarginTop->SetRange(0, 10000);
	fMarginRight = new BSpinner("right", "", new BMessage(CMD_MARGIN_RIGHT));
	fMarginRight->SetRange(0, 10000);
	fMarginBottom = new BSpinner("bottom", "", new BMessage(CMD_MARGIN_BOTTOM));
	fMarginBottom->SetRange(0, 10000);
	fSepCenter = new BSpinner("center", "", new BMessage(CMD_SEP_CENTER));
	fSepCenter->SetRange(0, 10000);
	
	BBox *marginBox = new BBox("margin");
	marginBox->SetLabel(B_TRANSLATE("Margin"));
	BGridLayout *marginLayout = BLayoutBuilder::Grid<>(
			B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
		.SetInsets(6, 6)
		.Add(new BStringView("leftl", B_TRANSLATE("Left")), 0, 0)
		.Add(fMarginLeft, 1, 0)
		.Add(new BStringView("topl", B_TRANSLATE("Top")), 0, 1)
		.Add(fMarginTop, 1, 1)
		.Add(new BStringView("centerl", B_TRANSLATE("Center")), 0, 2)
		.Add(fSepCenter, 1, 2)
		.Add(new BStringView("rightl", B_TRANSLATE("Right")), 2, 0)
		.Add(fMarginRight, 3, 0)
		.Add(new BStringView("bottoml", B_TRANSLATE("Bottom")), 2, 1)
		.Add(fMarginBottom, 3, 1);
	marginBox->AddChild(marginLayout->View());
	
	fBgImageFlagPM = new BPopUpMenu("bgflagimagePM");
	fBgImageFlagPM->SetRadioMode(true);
	fBgImageFlagPM->AddItem(new BMenuItem(B_TRANSLATE("Fit"), 
					new BMessage(CMD_BG_IMAGE_FIT)));
	fBgImageFlagPM->AddItem(new BMenuItem(B_TRANSLATE("Tile"),
					new BMessage(CMD_BG_IMAGE_TILE)));
	BMenuField * bgImageFlagMF = new BMenuField("bgimageflagmf", "", fBgImageFlagPM);
	
	fBgColorButton = new ColorButton("bgcolorcb", 
				new BMessage(CMD_BG_COLOR), make_color(0, 0, 0));
	fBgImageText = new BTextControl("imagetc", "", "", 
				new BMessage(CMD_BC_IMAGE_INPUT));
	
	BBox *bgBox = new BBox("bg");
	bgBox->SetLabel(B_TRANSLATE("Background"));
	BGridLayout *bgLayout = BLayoutBuilder::Grid<>(
			B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
		.SetInsets(6, 6)
		.Add(new BStringView("bgcolorl", B_TRANSLATE("Background color")), 0, 0)
		.Add(fBgColorButton, 1, 0)
		.Add(new BStringView("bgimagel", B_TRANSLATE("Background image")), 0, 1)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING, 1, 1)
			.Add(fBgImageText)
			.Add(new BButton("bgimageb", B_TRANSLATE("..."), 
						new BMessage(CMD_BG_IMAGE_CHOOSE)))
		.End()
		.Add(bgImageFlagMF, 1, 2);
	bgBox->AddChild(bgLayout->View());
	
	fHanging = new BCheckBox("hanging", B_TRANSLATE("Enable hanging"),
					new BMessage(CMD_HANGING));
	fShowIllustration = new BCheckBox("illustration", 
			B_TRANSLATE("Show illustration"), new BMessage(CMD_ILLUSTRATION));
	fPagination = new BCheckBox("pagination", B_TRANSLATE("Pagination"),
					new BMessage(CMD_PAGINATION));
	fAsLine = new BCheckBox("asline", B_TRANSLATE("'\xe3\x83\xbc' as line"),
					new BMessage(CMD_AS_LINE));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6)
		.AddGroup(B_HORIZONTAL)
			.AddGroup(B_VERTICAL)
				.AddGroup(B_HORIZONTAL)
					.Add(fSinglePage)
					.Add(fSpreadPages)
				.End()
				.AddGrid(B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
					.Add(new BStringView("charsl", B_TRANSLATE("Chars")), 0, 0)
					.Add(fChars, 1, 0)
					.Add(new BStringView("charpitchl", 
							B_TRANSLATE("Char pitch")), 0, 1)
					.Add(fCharacterPitch, 1, 1)
					.Add(new BStringView("linesl", B_TRANSLATE("Lines")), 2, 0)
					.Add(fLines, 3, 0)
					.Add(new BStringView("linepitchl", 
							B_TRANSLATE("Line pitch")), 2, 1)
					.Add(fLinePitch, 3, 1)
				.End()
				.AddGlue()
			.End()
			.Add(marginBox)
		.End()
		.Add(bgBox)
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(fHanging, 0, 0)
			.Add(fPagination, 0, 1)
			.Add(fShowIllustration, 1, 0)
			.Add(fAsLine, 1, 1)
		.End();
	
}

void StyleOptBasicView::InitData(EditStyles *editStyles)
{
	fEditStyles = editStyles;
}

void StyleOptBasicView::StoreData()
{
	if (fEditStyles == NULL || fEditStyles->current == NULL) {
		return;
	}
	StyleData data = fEditStyles->current->dat;
	
	data.b.pages = (fSinglePage->Value() == B_CONTROL_ON) ? 1 : 2;
	
	// chars
	data.b.chars = fChars->Value();
	data.b.lines = fLines->Value();
	data.b.charSpace = fCharacterPitch->Value();
	data.b.lineSpace = fLinePitch->Value();
	
	data.b.pageSpace = fSepCenter->Value();
	
	// margins
	data.b.margin.x1 = fMarginLeft->Value();
	data.b.margin.x2 = fMarginRight->Value();
	data.b.margin.y1 = fMarginTop->Value();
	data.b.margin.y2 = fMarginBottom->Value();
	
	// bg
	data.colBkgnd = rgb_colorTomRgbCol(fBgColorButton->Color());
	mStrSetText(&data.strBkgndFile, fBgImageText->Text());
	data.bkgnd_imgtype = (fBgImageFlagPM->FindMarkedIndex() == 1) ? 1 : 0;
	
	// options
	int flags = 0;
	if (fHanging->Value() == B_CONTROL_ON) {
		flags |= AOSTYLE_F_HANGING;
	}
	if (fShowIllustration->Value() == B_CONTROL_ON) {
		flags |= AOSTYLE_F_ENABLE_PICTURE;
	}
	if (fPagination->Value() == B_CONTROL_ON) {
		flags |= AOSTYLE_F_DRAW_PAGENO;
	}
	if (fAsLine->Value() == B_CONTROL_ON) {
		flags |= AOSTYLE_F_DASH_TO_LINE;
	}
	data.b.flags = flags;
	
	fEditStyles->current->dat = data;
}

void StyleOptBasicView::UpdateData()
{
	if (fEditStyles == NULL || fEditStyles->current == NULL) {
		return;
	}
	StyleData data = fEditStyles->current->dat;
	
	if (data.b.pages == 1) {
		fSinglePage->SetValue(B_CONTROL_ON);
	} else {
		fSpreadPages->SetValue(B_CONTROL_ON);
	}
	
	// chars
	fChars->SetValue(data.b.chars);
	fLines->SetValue(data.b.lines);
	fCharacterPitch->SetValue(data.b.charSpace);
	fLinePitch->SetValue(data.b.lineSpace);
	
	fSepCenter->SetValue(data.b.pageSpace);
	
	// margins
	fMarginLeft->SetValue(data.b.margin.x1);
	fMarginRight->SetValue(data.b.margin.x2);
	fMarginTop->SetValue(data.b.margin.y1);
	fMarginBottom->SetValue(data.b.margin.y2);
	
	// bg
	fBgColorButton->SetColor(mRgbColTo_rgb_color(data.colBkgnd));
	fBgImageText->SetText(data.strBkgndFile.buf);
	BMenuItem * item = fBgImageFlagPM->ItemAt(data.bkgnd_imgtype == 0 ? 0 : 1);
	if (item != NULL) {
		item->SetMarked(true);
	}
	
	// options
	fHanging->SetValue((data.b.flags & AOSTYLE_F_HANGING) > 0 ? 
						B_CONTROL_ON : B_CONTROL_OFF);
	fShowIllustration->SetValue((data.b.flags & AOSTYLE_F_ENABLE_PICTURE) > 0 ?
						B_CONTROL_ON : B_CONTROL_OFF);
	fPagination->SetValue((data.b.flags & AOSTYLE_F_DRAW_PAGENO) > 0 ?
						B_CONTROL_ON : B_CONTROL_OFF);
	fAsLine->SetValue((data.b.flags & AOSTYLE_F_DASH_TO_LINE) > 0 ?
						B_CONTROL_ON : B_CONTROL_OFF);
}

void StyleOptBasicView::SetBgPath(const char *path)
{
	fBgImageText->SetText(path);
}

void StyleOptBasicView::MessageReceived(BMessage *msg)
{
	BView::MessageReceived(msg);
}


class StyleOptFontView : public BView
{
public:
	StyleOptFontView();
	virtual void MessageReceived(BMessage *msg);
	void InitData(EditStyles *editStyles);
	void StoreData();
	void UpdateData();
	
private:
	static const int32 CMD_BODY_FONT = 'cmbf';
	static const int32 CMD_BODY_BOLD_FONT = 'cmbb';
	static const int32 CMD_RUBY_FONT = 'cmrf';
	static const int32 CMD_INFO_FONT = 'cmif';
	static const int32 CMD_BODY_COLOR = 'cmbc';
	static const int32 CMD_RUBY_COLOR = 'cmrc';
	static const int32 CMD_INFO_COLOR = 'cmic';
	static const int32 CMD_RENDERING = 'cmrn';
	
	FontButton * fBodyFontButton;
	FontButton * fBodyBoldFontButton;
	FontButton * fRubyFontButton;
	FontButton * fInfoFontButton;
	
	ColorButton * fBodyColorButton;
	ColorButton * fRubyColorButton;
	ColorButton * fInfoColorButton;
	
	BPopUpMenu * fRenderingPM;
	
	EditStyles * fEditStyles;
	
	void _UpdateFontData(FontButton *btn, const char *info, bool base=false);
	void _StoreFontData(FontButton *btn, mStr *str, bool base=false);
};

StyleOptFontView::StyleOptFontView()
	: BView(B_TRANSLATE("Font"), B_WILL_DRAW),
	  fEditStyles(NULL)
{
	fBodyFontButton = new FontButton("bodyb", "serif");
	fBodyBoldFontButton = new FontButton("bodyboldb", "serif");
	fRubyFontButton = new FontButton("rubyb", "serif");
	fInfoFontButton = new FontButton("infob", "serif");
	
	fBodyColorButton = new ColorButton("bodycb", 
			new BMessage(CMD_BODY_COLOR), make_color(0, 0, 0));
	fRubyColorButton = new ColorButton("rubycb", 
			new BMessage(CMD_RUBY_COLOR), make_color(0, 0, 0));
	fInfoColorButton = new ColorButton("infocb", 
			new BMessage(CMD_INFO_COLOR), make_color(0, 0, 0));
	
	fRenderingPM = new BPopUpMenu("renderingPM");
	fRenderingPM->SetRadioMode(true);
	BLayoutBuilder::Menu<>(fRenderingPM)
		.AddItem("Default", new BMessage(CMD_RENDERING))
		.AddItem("Mono", new BMessage(CMD_RENDERING))
		.AddItem("Gray", new BMessage(CMD_RENDERING))
		.AddItem("LCD (RGB)", new BMessage(CMD_RENDERING))
		.AddItem("LCD (BGR)", new BMessage(CMD_RENDERING))
		.AddItem("LCD (V RGB)", new BMessage(CMD_RENDERING))
		.AddItem("LCD (V BGR)", new BMessage(CMD_RENDERING));
	
	BMenuField * renderingMF = new BMenuField("renderingMF", "", fRenderingPM);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6)
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(new BStringView("body", B_TRANSLATE("Body")), 0, 0)
			.Add(fBodyFontButton, 1, 0)
			.Add(fBodyColorButton, 2, 0)
			.Add(new BStringView("bodybold", B_TRANSLATE("Body (bold)")), 0, 1)
			.Add(fBodyBoldFontButton, 1, 1)
			.Add(new BStringView("ruby", B_TRANSLATE("Ruby")), 0, 2)
			.Add(fRubyFontButton, 1, 2)
			.Add(fRubyColorButton, 2, 2)
			.Add(new BStringView("info", B_TRANSLATE("Information")), 0, 3)
			.Add(fInfoFontButton, 1, 3)
			.Add(fInfoColorButton, 2, 3)
			
			.Add(new BStringView("rendering", B_TRANSLATE("Rendering")), 0, 4)
			.Add(renderingMF, 1, 4)
		.End()
		.AddGlue();
}

void StyleOptFontView::InitData(EditStyles *editStyles)
{
	fEditStyles = editStyles;
}

void StyleOptFontView::StoreData()
{
	if (fEditStyles == NULL || fEditStyles->current == NULL) {
		return;
	}
	StyleData data = fEditStyles->current->dat;
	
	_StoreFontData(fBodyFontButton, &data.strFontText, true);
	_StoreFontData(fBodyBoldFontButton, &data.strFontBold);
	_StoreFontData(fRubyFontButton, &data.strFontRuby);
	_StoreFontData(fInfoFontButton, &data.strFontInfo);
	
	data.b.colText = rgb_colorTomRgbCol(fBodyColorButton->Color());
	data.b.colRuby = rgb_colorTomRgbCol(fRubyColorButton->Color());
	data.b.colInfo = rgb_colorTomRgbCol(fInfoColorButton->Color());
	
	fEditStyles->current->dat = data;
}

void StyleOptFontView::_StoreFontData(FontButton *btn, mStr *str, bool base)
{
	mFontInfo info;
	info.mask = 0;
	info.strFamily = MSTR_INIT;
	info.strStyle = MSTR_INIT;
	
	//info.strFamily = btn->FontFamily();
	mStrSetText(&info.strFamily, btn->FontFamily());
	info.mask |= MFONTINFO_MASK_FAMILY;
	//info.strStyle = btn->FontStyle();
	mStrSetText(&info.strStyle, btn->FontStyle());
	info.mask |= MFONTINFO_MASK_STYLE;
	info.size = (double)btn->FontSize();
	if (info.size >= 0) {
		info.mask |= MFONTINFO_MASK_SIZE;
	}
	
	info.weight = btn->FontWeight();
	if (info.weight == FONT_WEIGHT_NORMAL) {
		info.weight = MFONTINFO_WEIGHT_NORMAL;
		info.mask |= MFONTINFO_MASK_WEIGHT;
	} else if (info.weight == FONT_WEIGHT_BOLD) {
		info.weight = MFONTINFO_WEIGHT_BOLD;
		info.mask |= MFONTINFO_MASK_WEIGHT;
	}
	
	info.slant = btn->FontSlant();
	if (info.slant == FONT_SLANT_ROMAN) {
		info.slant = MFONTINFO_SLANT_ROMAN;
		info.mask |= MFONTINFO_MASK_SLANT;
	} else if (info.slant == FONT_SLANT_ITALIC) {
		info.slant = MFONTINFO_SLANT_ITALIC;
		info.mask |= MFONTINFO_MASK_SLANT;
	} else if (info.slant == FONT_SLANT_OBLIQUE) {
		info.slant = MFONTINFO_SLANT_OBLIQUE;
		info.mask |= MFONTINFO_MASK_SLANT;
	}
	
	if (base) {
		int index = fRenderingPM->FindMarkedIndex();
		if (0 <= index && index <= MFONTINFO_RENDER_LCD_VBGR) {
			info.render = index;
			info.mask |= MFONTINFO_MASK_RENDER;
		}
	}
	
	mFontInfoToFormat(str, &info);
}

void StyleOptFontView::UpdateData()
{
	if (fEditStyles == NULL || fEditStyles->current == NULL) {
		return;
	}
	StyleData data = fEditStyles->current->dat;
	
	_UpdateFontData(fBodyFontButton, data.strFontText.buf, true);
	_UpdateFontData(fBodyBoldFontButton, data.strFontBold.buf);
	_UpdateFontData(fRubyFontButton, data.strFontRuby.buf);
	_UpdateFontData(fInfoFontButton, data.strFontInfo.buf);
	
	fBodyColorButton->SetColor(mRgbColTo_rgb_color(data.b.colText));
	fRubyColorButton->SetColor(mRgbColTo_rgb_color(data.b.colRuby));
	fInfoColorButton->SetColor(mRgbColTo_rgb_color(data.b.colInfo));
}

void StyleOptFontView::_UpdateFontData(FontButton *btn, 
							const char *fontInfo, bool base)
{
	mFontInfo info;
	info.strFamily = MSTR_INIT;
	mStrSetText(&info.strFamily, "serif");
	info.strStyle = MSTR_INIT;
	mStrSetText(&info.strStyle, "Regular");
	info.size = 12;
	info.weight = MFONTINFO_WEIGHT_NORMAL;
	info.slant = MFONTINFO_SLANT_ROMAN;
	
	mFontFormatToInfo(&info, fontInfo);
	if ((info.mask & MFONTINFO_MASK_FAMILY) > 0) {
		btn->SetFontFamily(info.strFamily.buf);
	}
	if ((info.mask & MFONTINFO_MASK_STYLE) > 0) {
		btn->SetFontStyle(info.strStyle.buf);
	}
	if ((info.mask & MFONTINFO_MASK_SIZE) > 0) {
		btn->SetFontSize((int32)info.size);
	}
	if ((info.mask & MFONTINFO_MASK_WEIGHT) > 0) {
		btn->SetFontWeight(info.weight == MFONTINFO_WEIGHT_NORMAL ? 
				FONT_WEIGHT_NORMAL : FONT_WEIGHT_BOLD);
	}
	if ((info.mask & MFONTINFO_MASK_SLANT) > 0) {
		int32 slant = FONT_SLANT_NONE;
		if (info.slant == MFONTINFO_SLANT_ROMAN) {
			slant = FONT_SLANT_ROMAN;
		} else if (info.slant == MFONTINFO_SLANT_ITALIC) {
			slant = FONT_SLANT_ITALIC;
		} else if (info.slant == MFONTINFO_SLANT_OBLIQUE) {
			slant = FONT_SLANT_OBLIQUE;
		}
		btn->SetFontSlant(slant);
	}
	if (base) {
		BMenuItem *item = fRenderingPM->FindMarked();
		if (item != NULL) {
			item->SetMarked(false);
		}
		int render = ((info.mask & MFONTINFO_MASK_RENDER) > 0) &&
				0 <= info.render && info.render <= MFONTINFO_RENDER_LCD_VBGR ?
				info.render : MFONTINFO_RENDER_DEFAULT;
		item = fRenderingPM->ItemAt(info.render);
		if (item != NULL) {
			item->SetMarked(true);
		}
	}
	
	mFontInfoFree(&info);
}

void StyleOptFontView::MessageReceived(BMessage *msg)
{
	BView::MessageReceived(msg);
}

class StyleOptCharView : public BView
{
public:
	StyleOptCharView();
	virtual void MessageReceived(BMessage *msg);
	void InitData(EditStyles *editStyles);
	void StoreData();
	void UpdateData();
	
private:
	
	BTextControl * fNoStartText;
	BTextControl * fNoEndText;
	BTextControl * fHangingText;
	BTextControl * fNoSepText;
	BTextControl * fReplaceText;
	
	EditStyles * fEditStyles;
	
};

StyleOptCharView::StyleOptCharView()
	: BView(B_TRANSLATE("Character"), B_WILL_DRAW),
	  fEditStyles(NULL)
{
	fNoStartText = new BTextControl("nostart", "", "", new BMessage());
	fNoEndText = new BTextControl("noend", "", "", new BMessage());
	fHangingText = new BTextControl("hanging", "", "", new BMessage());
	fNoSepText = new BTextControl("nosep", "", "", new BMessage());
	fReplaceText = new BTextControl("replace", "", "", new BMessage());
	BTextView * desc = new BTextView("desc");
	desc->MakeEditable(false);
	desc->SetWordWrap(true);
	desc->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	desc->SetText(B_TRANSLATE("Set of target character and replacement characters.\n"
			"Halfwidth space can be used as separator.\n"
			"ex. \xe2\x89\xaa\xe3\x80\x8a \xe2\x89\xab\xe3\x80\x8b"));
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(6)
		.AddGrid(B_USE_DEFAULT_SPACING, B_USE_HALF_ITEM_SPACING)
			.Add(new BStringView("nostartl", B_TRANSLATE("Not at start")), 0, 0)
			.Add(fNoStartText, 1, 0)
			.Add(new BStringView("noendl", B_TRANSLATE("Not at end")), 0, 1)
			.Add(fNoEndText, 1, 1)
			.Add(new BStringView("hangingl", B_TRANSLATE("Hanging")), 0, 2)
			.Add(fHangingText, 1, 2)
			.Add(new BStringView("nosepl", B_TRANSLATE("No separation")), 0, 3)
			.Add(fNoSepText, 1, 3)
			.Add(new BStringView("replacel", B_TRANSLATE("Replacement")), 0, 4)
			.Add(fReplaceText, 1, 4)
			.Add(desc, 1, 5)
		.End()
		.AddGlue();
}
// todo, we do not need rendering option for each type
void StyleOptCharView::InitData(EditStyles *editStyles)
{
	fEditStyles = editStyles;
}

void StyleOptCharView::StoreData()
{
	if (fEditStyles == NULL || fEditStyles->current == NULL) {
		return;
	}
	StyleData data = fEditStyles->current->dat;
	
	StyleSetUTF32Text(&data.b.strNoHead, fNoStartText->Text());
	StyleSetUTF32Text(&data.b.strNoBottom, fNoEndText->Text());
	StyleSetUTF32Text(&data.b.strHanging, fHangingText->Text());
	StyleSetUTF32Text(&data.b.strReplace, fReplaceText->Text());
	
	fEditStyles->current->dat = data;
}

void StyleOptCharView::UpdateData()
{
	if (fEditStyles == NULL || fEditStyles->current == NULL) {
		return;
	}
	StyleData data = fEditStyles->current->dat;
	
	mStr str = MSTR_INIT;
	
	mStrSetTextUCS4(&str, data.b.strNoHead, -1);
	fNoStartText->SetText(str.buf);
	
	mStrSetTextUCS4(&str, data.b.strNoBottom, -1);
	fNoEndText->SetText(str.buf);
	
	mStrSetTextUCS4(&str, data.b.strHanging, -1);
	fHangingText->SetText(str.buf);
	
	mStrSetTextUCS4(&str, data.b.strNoSep, -1);
	fNoSepText->SetText(str.buf);
	
	mStrSetTextUCS4(&str, data.b.strReplace, -1);
	fReplaceText->SetText(str.buf);
	
	mStrFree(&str);
}

void StyleOptCharView::MessageReceived(BMessage *msg)
{
	BView::MessageReceived(msg);
}


StyleOptDialog::StyleOptDialog(BWindow *owner)
	: BWindow(
		BRect(50, 50, 450, 450),
		"Style settings",
		B_FLOATING_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL,
		B_NOT_MINIMIZABLE | B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS |
		B_CLOSE_ON_ESCAPE),
	  fOwner(owner),
	  fInputDialog(NULL),
	  fFileChooser(NULL),
	  fEditStyles(new EditStyles()),
	  fStyleListChanged(false)
{
	fStylesMenu = new BPopUpMenu("styles");
	fStylesMenu->SetRadioMode(true);
	fStylesMF = new BMenuField("stylesMF", "", fStylesMenu);
	
	fBasicView = new StyleOptBasicView();
	fFontView = new StyleOptFontView();
	fCharView = new StyleOptCharView();
	
	BTabView *tabview = new BTabView("tabview", B_WIDTH_FROM_LABEL);
	tabview->SetBorder(B_NO_BORDER);
	tabview->AddTab(fBasicView);
	tabview->AddTab(fFontView);
	tabview->AddTab(fCharView);
	
	BButton *okButton = new BButton("ok", B_TRANSLATE("OK"), 
					new BMessage(DIALOG_OK));
	okButton->MakeDefault(true);
	
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.SetInsets(6, 6, 6, 0)
			.Add(fStylesMF)
			.Add(new BButton("new", B_TRANSLATE("New"),
					new BMessage(CMD_NEW)))
			.Add(new BButton("delete", B_TRANSLATE("Delete"),
					new BMessage(CMD_DELETE)))
		.End()
		.Add(tabview)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
			.SetInsets(6)
			.AddGlue()
			.Add(okButton)
			.Add(new BButton("cancel", B_TRANSLATE("Cancel"),
					new BMessage(DIALOG_CANCEL)))
		.End();
	
	Layout(true);
}

StyleOptDialog::~StyleOptDialog()
{
}

void StyleOptDialog::Quit()
{
	fEditStyles->current = NULL;
	fBasicView->InitData(NULL);
	fFontView->InitData(NULL);
	fCharView->InitData(NULL);
	
	if (fFileChooser && fFileChooser->Lock()) {
		fFileChooser->Quit();
	}
	
	delete fEditStyles;
	fOwner->PostMessage(A_STYLE_OPT_DIALOG_CLOSED);
	BWindow::Quit();
}

void StyleOptDialog::ShowDialog(BRect parent)
{
	_InitData();
	
	CenterIn(parent);
	Show();
}

void StyleOptDialog::WindowActivated(bool active)
{
	if (active) {
		fStylesMF->MakeFocus(true);
	}
}

#define A_STYLE_CHOOSE 'asch'
#define A_STYLE_NEW 'asns'
#define A_STYLE_INPUT_CLOSED 'assc'

void StyleOptDialog::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case CMD_NEW:
		{
			if (fInputDialog == NULL) {
				fInputDialog = new InputDialog(this, B_TRANSLATE("New style"), 
					B_TRANSLATE("Input style name"), InputDialog::IT_STRING,
					A_STYLE_NEW, A_STYLE_INPUT_CLOSED);
			}
			fInputDialog->ShowDialog(Frame());
			break;
		}
		case A_STYLE_INPUT_CLOSED:
		{
			fInputDialog = NULL;
			break;
		}
		case CMD_DELETE:
		{
			if (fStylesMenu->CountItems() == 1) {
				return;
			}
			
			BAlert *alert = new BAlert("Delete style", 
					B_TRANSLATE("Do you want to delete?"),
					B_TRANSLATE("YES"), B_TRANSLATE("NO"), NULL,
					B_WIDTH_AS_USUAL, B_IDEA_ALERT);
			alert->SetShortcut(1, B_ESCAPE);
			int32 button_index = alert->Go();
			if (button_index == 0) {
				_DeleteStyle();
			}
			break;
		}
		case DIALOG_OK:
		{
			// store data
			_StoreData();
			
			Hide();
			
			fStyleListChanged = !mStrPathCompareEq(&GDAT->style->strName, 
									fEditStyles->current->dat.strName.buf);
			
			StyleListSaveAndDelete(fEditStyles->styles);
			
			if (StyleChange(GDAT->style, &fEditStyles->current->dat)) {
				fOwner->PostMessage(A_STYLE_UPDATE);
			}
			
			if (fStyleListChanged) {
				fOwner->PostMessage(A_STYLE_MENU_UPDAGE);
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
		case A_STYLE_CHOOSE:
		{
			_ChangeStyle(true);
			break;
		}
		case A_STYLE_NEW:
		{
			const char *name;
			if (msg->FindString("value", &name) == B_OK) {
				BMenuItem *item = fStylesMenu->FindItem(name);
				if (item != NULL) {
					// there is a style having the same name
					BAlert *alert = new BAlert("Style name conflicts",
						B_TRANSLATE("Style name conflicts with the existing one."),
						B_TRANSLATE("OK"), NULL, NULL,
						B_WIDTH_AS_USUAL, B_STOP_ALERT);
					alert->SetShortcut(0, B_ESCAPE);
					alert->Go();
					return;
				}
				_NewStyle(name);
			}
			break;
		}
		case StyleOptBasicView::CMD_BG_IMAGE_CHOOSE:
		{
			if (!fFileChooser) {
				FileFilter * filter = new FileFilter();
				filter->AddFileFilter(B_TRANSLATE("All files (*.*)"), "");
				filter->AddFileFilter(B_TRANSLATE("PNG files (*.png)"), ".png");
				filter->AddFileFilter(B_TRANSLATE("JPEG files (*.jpg)"), ".jpg;.jpeg");
				fFileChooser = new FileChooser(this, B_OPEN_PANEL, filter, false);
			}
			fFileChooser->Show();
			break;
		}
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) == B_OK) {
				BPath path(&ref);
				fBasicView->SetBgPath(path.Path());
			}
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}

void StyleOptDialog::_InitData()
{
	// add styles
	mStr str = MSTR_INIT;
	mAppGetConfigPath(&str, "style");
	mDirEntry *dir = mDirEntryOpen(str.buf);
	if (dir != NULL) {
		int cnt = 0;
		while (mDirEntryRead(dir) && cnt < STYLE_MAXNUM) {
			if (mDirEntryIsDirectory(dir) || !mDirEntryIsEqExt(dir, "conf")) {
				continue;
			}
			mStrPathGetFileNameNoExt(&str, mDirEntryGetFileName(dir));
			fStylesMenu->AddItem(new BMenuItem(str.buf, 
							new BMessage(A_STYLE_CHOOSE)));
			cnt++;
		}
		mDirEntryClose(dir);
	}
	mStrFree(&str);
	
	if (fStylesMenu->CountItems() == 0) {
		// add default
		fStylesMenu->AddItem(new BMenuItem(GDAT->style->strName.buf,
								new BMessage(A_STYLE_CHOOSE)));
	}
	
	fEditStyles->current = StyleListCreate(fEditStyles->styles);
	// 
	fBasicView->InitData(fEditStyles);
	fFontView->InitData(fEditStyles);
	fCharView->InitData(fEditStyles);
	
	_UpdateData();
	
	// choose current
	BMenuItem *item = fStylesMenu->FindItem(GDAT->style->strName.buf);
	if (item != NULL) {
		item->SetMarked(true);
		
		_ChangeStyle(false);
	}
}

void StyleOptDialog::_SetStyleChanged()
{
	BMessage mess(A_STYLE_CHOOSE);
	MessageReceived(&mess);
}

void StyleOptDialog::_StoreData()
{
	fBasicView->StoreData();
	fFontView->StoreData();
	fCharView->StoreData();
}

void StyleOptDialog::_UpdateData()
{
	fBasicView->UpdateData();
	fFontView->UpdateData();
	fCharView->UpdateData();
}

void StyleOptDialog::_ChangeStyle(bool store)
{
	// store current data
	if (store) {
		_StoreData();
	}
	
	// find choosen style and set it to current
	BMenuItem *item = fStylesMenu->FindMarked();
	if (item != NULL) {
		const char *label = item->Label();
		if (label != NULL) {
			StyleListItem *p;
			for (p = (StyleListItem *)fEditStyles->styles->top; 
				p != NULL; p = (StyleListItem *)p->i.next) {
				if (!p->bDelete && mStrPathCompareEq(&p->dat.strName, label)) {
					fEditStyles->current = p;
					break;
				}
			}
			if (p == NULL) {
				// not loaded into the list, read it from the file
				fEditStyles->current = StyleListLoad(fEditStyles->styles, label);
			}
		}
	}
	// update data to the current style
	_UpdateData();
}

void StyleOptDialog::_NewStyle(const char *name)
{
	mStr str = MSTR_INIT;
	
	mStrSetText(&str, name);
	StyleListItem *pi = StyleListAppend(fEditStyles->styles, &str);
	if (pi != NULL) {
		BMenuItem *newItem = new BMenuItem(name,
								new BMessage(A_STYLE_CHOOSE));
		fStylesMenu->AddItem(newItem);
		// change to new style
		BMenuItem * item = fStylesMenu->FindMarked();
		if (item != NULL) {
			item->SetMarked(false);
		}
		newItem->SetMarked(true);
		
		_SetStyleChanged();
	}
	
	mStrFree(&str);
}

void StyleOptDialog::_DeleteStyle()
{
	// set deleted flag
	fEditStyles->current->bDelete = TRUE;
	
	// remove current from the popup
	int32 marked = fStylesMenu->FindMarkedIndex();
	if (marked >= 0) {
		fStylesMenu->RemoveItem(marked);
	} else {
		marked = 0;
	}
	// choose one
	if (fStylesMenu->CountItems() > marked) {
		BMenuItem *item = fStylesMenu->ItemAt(marked);
		if (item != NULL) {
			item->SetMarked(true);
		}
	}
	
	_SetStyleChanged();
}
