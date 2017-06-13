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

#include "keys.h"

#include "mDef.h"
#include "mKeyDef.h"
#include "mDefGui.h"


static const uint8 NUMPAD[] = {
		MKEY_NUM_DIV,MKEY_NUM_MUL,MKEY_NUM_SUB,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,
	MKEY_NUM7,MKEY_NUM8,MKEY_NUM9,MKEY_NUM_ADD,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,
	MKEY_NUM4,MKEY_NUM5,MKEY_NUM6,
	0,0,0,0,0, 0,0,0,0,0, 0,0,0,
	MKEY_NUM1,MKEY_NUM2,MKEY_NUM3,MKEY_ENTER,
	0,0,0,0,0, 0,0,0,
	MKEY_NUM0,MKEY_NUM_DECIMAL,
};

static const uint8 SPECIAL_KEYS[] = {
	0,MKEY_HOME,0,0,MKEY_END,
		MKEY_INSERT,0,0,MKEY_BACKSPACE,MKEY_TAB,
		MKEY_ENTER,MKEY_PAGEUP,MKEY_PAGEDOWN,0,0, 0,
	MKEY_F1,0,0,0,0, 0,0,0,0,0,
	0,MKEY_ESCAPE,MKEY_LEFT,MKEY_RIGHT,MKEY_UP,
		MKEY_DOWN,
	MKEY_SPACE,
};


int32 ParseHaikuKey(uint8 byte, int32 key, uint32 raw_char, uint32 mod)
{
	int32 k = 0;
	if (0x23 <= key && key <= 0x65) {
		k = NUMPAD[key - 0x23];
		if (k != 0) {
			return k;
		}
	}
	if ('!' <= raw_char && raw_char <= '~') {
		if ('a' <= raw_char && raw_char <= 'z') {
			k = raw_char - 'a' + 'A';
		} else if ('0' <= raw_char && raw_char <= '9') {
			k = raw_char;
		} else {
			k = raw_char;
		}
	} else if (byte == B_FUNCTION_KEY) {
		k = key - B_F1_KEY + MKEY_F1;
	} else {
		k = SPECIAL_KEYS[byte];
	}
	return k;
}


static const char * NUMBERS[] = {
	"0","1","2","3","4","5","6","7","8","9","10","11","12",
};

static const char * SPKEY_NAMES1[] = {
	"","BREAK","PAUSE","CLEAR","PRINTSCREEN","ESCAPE",
	"NUM_LOCK","SCROLL_LOCK","CAPS_LOCK",
	"ENTER","BACKSPACE","TAB","DELETE","INSERT",
	"SHIFT","CONTROL","ALT","SUPER","MENU",
};

static const char * SPKEY_NAMES2[] = {
	"SPACE","PAGEUP","PAGEDOWN","END","HOME","LEFT","UP","RIGHT","DOWN",
};

void AddKeyStr(BString *str, uint32 key)
{
	if (!str->IsEmpty()) {
		str->Append('+', 1);
	}
	if (MKEY_0 <= key && key <= MKEY_9 || 
		MKEY_A <= key && key <= MKEY_Z) {
		str->Append(key, 1);
	} else if (MKEY_F1 <= key && key <= MKEY_F12) {
		str->Append('F', 1);
		str->Append(NUMBERS[key - MKEY_F1]);
	} else if (MKEY_UNKNOWN <= key && key <= MKEY_MENU) {
		str->Append(SPKEY_NAMES1[key]);
	} else if (MKEY_SPACE <= key && key <= MKEY_DOWN) {
		str->Append(SPKEY_NAMES2[key - MKEY_SPACE]);
	}
}

void AddModifierStr(BString *str, uint32 mod)
{
	if (mod & MACCKEY_CTRL) {
		str->Append("Command", 7); // Ctrl
	}
	if (mod & MACCKEY_SHIFT) {
		if (!str->IsEmpty()) {
			str->Append('+', 1);
		}
		str->Append("Shift", 5);
	}
}


KeyFilter::KeyFilter(BHandler *handler)
	: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE),
	  fHandler(handler)
{
}

filter_result KeyFilter::Filter(BMessage *msg, BHandler **target)
{
	if (msg->what == B_KEY_DOWN) {
		fHandler->MessageReceived(msg);
	}
	return B_DISPATCH_MESSAGE;
}
