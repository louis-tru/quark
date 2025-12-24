/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "../../ui/keyboard.h"

namespace qk {

	class LinuxKeyboardAdapter: public KeyboardAdapter {
	public:
		LinuxKeyboardAdapter();
	};

	LinuxKeyboardAdapter::LinuxKeyboardAdapter() {
		_PlatformKeyCodeToKeyCode[0] = KEYCODE_UNKNOWN;
		_PlatformKeyCodeToKeyCode[22] = KEYCODE_BACK_SPACE;
		_PlatformKeyCodeToKeyCode[23] = KEYCODE_TAB;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_CLEAR] = KEYCODE_CLEAR;
		_PlatformKeyCodeToKeyCode[36] = KEYCODE_ENTER;
		_PlatformKeyCodeToKeyCode[50] = KEYCODE_SHIFT;
		_PlatformKeyCodeToKeyCode[62] = KEYCODE_SHIFT;
		_PlatformKeyCodeToKeyCode[37] = KEYCODE_CTRL;
		_PlatformKeyCodeToKeyCode[105] = KEYCODE_CTRL;
		_PlatformKeyCodeToKeyCode[64] = KEYCODE_ALT;
		_PlatformKeyCodeToKeyCode[108] = KEYCODE_ALT;
		_PlatformKeyCodeToKeyCode[127] = KEYCODE_BREAK;
		_PlatformKeyCodeToKeyCode[66] = KEYCODE_CAPS_LOCK;
		_PlatformKeyCodeToKeyCode[9] = KEYCODE_ESC;
		_PlatformKeyCodeToKeyCode[65] = KEYCODE_SPACE;
		_PlatformKeyCodeToKeyCode[112] = KEYCODE_PAGE_UP;
		_PlatformKeyCodeToKeyCode[117] = KEYCODE_PAGE_DOWN;
		_PlatformKeyCodeToKeyCode[115] = KEYCODE_MOVE_END;
		_PlatformKeyCodeToKeyCode[110] = KEYCODE_MOVE_HOME;
		_PlatformKeyCodeToKeyCode[113] = KEYCODE_LEFT;
		_PlatformKeyCodeToKeyCode[111] = KEYCODE_UP;
		_PlatformKeyCodeToKeyCode[114] = KEYCODE_RIGHT;
		_PlatformKeyCodeToKeyCode[116] = KEYCODE_DOWN;
		_PlatformKeyCodeToKeyCode[118] = KEYCODE_INSERT;
		_PlatformKeyCodeToKeyCode[119] = KEYCODE_DELETE;
		_PlatformKeyCodeToKeyCode[107] = KEYCODE_SYSRQ;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_HELP;
		_PlatformKeyCodeToKeyCode[19] = KEYCODE_0;
		_PlatformKeyCodeToKeyCode[10] = KEYCODE_1;
		_PlatformKeyCodeToKeyCode[11] = KEYCODE_2;
		_PlatformKeyCodeToKeyCode[12] = KEYCODE_3;
		_PlatformKeyCodeToKeyCode[13] = KEYCODE_4;
		_PlatformKeyCodeToKeyCode[14] = KEYCODE_5;
		_PlatformKeyCodeToKeyCode[15] = KEYCODE_6;
		_PlatformKeyCodeToKeyCode[16] = KEYCODE_7;
		_PlatformKeyCodeToKeyCode[17] = KEYCODE_8;
		_PlatformKeyCodeToKeyCode[18] = KEYCODE_9;
		_PlatformKeyCodeToKeyCode[38] = KEYCODE_A;
		_PlatformKeyCodeToKeyCode[56] = KEYCODE_B;
		_PlatformKeyCodeToKeyCode[54] = KEYCODE_C;
		_PlatformKeyCodeToKeyCode[40] = KEYCODE_D;
		_PlatformKeyCodeToKeyCode[26] = KEYCODE_E;
		_PlatformKeyCodeToKeyCode[41] = KEYCODE_F;
		_PlatformKeyCodeToKeyCode[42] = KEYCODE_G;
		_PlatformKeyCodeToKeyCode[43] = KEYCODE_H;
		_PlatformKeyCodeToKeyCode[31] = KEYCODE_I;
		_PlatformKeyCodeToKeyCode[44] = KEYCODE_J;
		_PlatformKeyCodeToKeyCode[45] = KEYCODE_K;
		_PlatformKeyCodeToKeyCode[46] = KEYCODE_L;
		_PlatformKeyCodeToKeyCode[58] = KEYCODE_M;
		_PlatformKeyCodeToKeyCode[57] = KEYCODE_N;
		_PlatformKeyCodeToKeyCode[32] = KEYCODE_O;
		_PlatformKeyCodeToKeyCode[33] = KEYCODE_P;
		_PlatformKeyCodeToKeyCode[24] = KEYCODE_Q;
		_PlatformKeyCodeToKeyCode[27] = KEYCODE_R;
		_PlatformKeyCodeToKeyCode[39] = KEYCODE_S;
		_PlatformKeyCodeToKeyCode[28] = KEYCODE_T;
		_PlatformKeyCodeToKeyCode[30] = KEYCODE_U;
		_PlatformKeyCodeToKeyCode[55] = KEYCODE_V;
		_PlatformKeyCodeToKeyCode[25] = KEYCODE_W;
		_PlatformKeyCodeToKeyCode[53] = KEYCODE_X;
		_PlatformKeyCodeToKeyCode[29] = KEYCODE_Y;
		_PlatformKeyCodeToKeyCode[52] = KEYCODE_Z;
		_PlatformKeyCodeToKeyCode[133] = KEYCODE_COMMAND;
		_PlatformKeyCodeToKeyCode[135] = KEYCODE_MENU;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_COMMAND_RIGHT;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_NUMPAD_EQUALS;
		_PlatformKeyCodeToKeyCode[90] = KEYCODE_NUMPAD_0;
		_PlatformKeyCodeToKeyCode[87] = KEYCODE_NUMPAD_1;
		_PlatformKeyCodeToKeyCode[88] = KEYCODE_NUMPAD_2;
		_PlatformKeyCodeToKeyCode[89] = KEYCODE_NUMPAD_3;
		_PlatformKeyCodeToKeyCode[83] = KEYCODE_NUMPAD_4;
		_PlatformKeyCodeToKeyCode[84] = KEYCODE_NUMPAD_5;
		_PlatformKeyCodeToKeyCode[85] = KEYCODE_NUMPAD_6;
		_PlatformKeyCodeToKeyCode[79] = KEYCODE_NUMPAD_7;
		_PlatformKeyCodeToKeyCode[80] = KEYCODE_NUMPAD_8;
		_PlatformKeyCodeToKeyCode[81] = KEYCODE_NUMPAD_9;
		_PlatformKeyCodeToKeyCode[63] = KEYCODE_NUMPAD_MULTIPLY;
		_PlatformKeyCodeToKeyCode[86] = KEYCODE_NUMPAD_ADD;
		_PlatformKeyCodeToKeyCode[104] = KEYCODE_NUMPAD_ENTER;
		_PlatformKeyCodeToKeyCode[82] = KEYCODE_NUMPAD_SUBTRACT;
		_PlatformKeyCodeToKeyCode[91] = KEYCODE_NUMPAD_DOT;
		_PlatformKeyCodeToKeyCode[106] = KEYCODE_NUMPAD_DIVIDE;
		_PlatformKeyCodeToKeyCode[67] = KEYCODE_F1;
		_PlatformKeyCodeToKeyCode[68] = KEYCODE_F2;
		_PlatformKeyCodeToKeyCode[69] = KEYCODE_F3;
		_PlatformKeyCodeToKeyCode[70] = KEYCODE_F4;
		_PlatformKeyCodeToKeyCode[71] = KEYCODE_F5;
		_PlatformKeyCodeToKeyCode[72] = KEYCODE_F6;
		_PlatformKeyCodeToKeyCode[73] = KEYCODE_F7;
		_PlatformKeyCodeToKeyCode[74] = KEYCODE_F8;
		_PlatformKeyCodeToKeyCode[75] = KEYCODE_F9;
		_PlatformKeyCodeToKeyCode[76] = KEYCODE_F10;
		_PlatformKeyCodeToKeyCode[95] = KEYCODE_F11;
		_PlatformKeyCodeToKeyCode[96] = KEYCODE_F12;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F13] = KEYCODE_F13;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F14] = KEYCODE_F14;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F15] = KEYCODE_F15;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F16] = KEYCODE_F16;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F17] = KEYCODE_F17;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F18] = KEYCODE_F18;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F19] = KEYCODE_F19;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F20] = KEYCODE_F20;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F21] = KEYCODE_F21;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F22] = KEYCODE_F22;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F23] = KEYCODE_F23;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_F24] = KEYCODE_F24;
		_PlatformKeyCodeToKeyCode[77] = KEYCODE_NUM_LOCK;
		_PlatformKeyCodeToKeyCode[78] = KEYCODE_SCROLL_LOCK;
		_PlatformKeyCodeToKeyCode[47] = KEYCODE_SEMICOLON;
		_PlatformKeyCodeToKeyCode[21] = KEYCODE_EQUALS;
		_PlatformKeyCodeToKeyCode[20] = KEYCODE_MINUS;
		_PlatformKeyCodeToKeyCode[59] = KEYCODE_COMMA;
		_PlatformKeyCodeToKeyCode[60] = KEYCODE_PERIOD;
		_PlatformKeyCodeToKeyCode[61] = KEYCODE_SLASH;
		_PlatformKeyCodeToKeyCode[49] = KEYCODE_GRAVE;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_FUN;
		_PlatformKeyCodeToKeyCode[34] = KEYCODE_LEFT_BRACKET;
		_PlatformKeyCodeToKeyCode[51] = KEYCODE_BACK_SLASH;
		_PlatformKeyCodeToKeyCode[35] = KEYCODE_RIGHT_BRACKET;
		_PlatformKeyCodeToKeyCode[48] = KEYCODE_APOSTROPHE;
		/* --------------------------------------------------- */
		_PlatformKeyCodeToKeyCode[180] = KEYCODE_HOME;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_BACK;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_CALL;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_ENDCALL;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_STAR;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_POUND;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_CENTER;
		_PlatformKeyCodeToKeyCode[123] = KEYCODE_VOLUME_UP;
		_PlatformKeyCodeToKeyCode[122] = KEYCODE_VOLUME_DOWN;
		_PlatformKeyCodeToKeyCode[124] = KEYCODE_POWER;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_CAMERA;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_FOCUS;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MENU_1;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_SEARCH;
		_PlatformKeyCodeToKeyCode[172] = KEYCODE_MEDIA_PLAY_PAUSE;
		_PlatformKeyCodeToKeyCode[174] = KEYCODE_MEDIA_STOP;
		_PlatformKeyCodeToKeyCode[171] = KEYCODE_MEDIA_NEXT;
		_PlatformKeyCodeToKeyCode[173] = KEYCODE_MEDIA_PREVIOUS;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MEDIA_REWIND;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MEDIA_FAST_FORWARD;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MUTE;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_CHANNEL_UP;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_CHANNEL_DOWN;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MEDIA_PLAY;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MEDIA_PAUSE;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MEDIA_CLOSE;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MEDIA_EJECT;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_MEDIA_RECORD;
		_PlatformKeyCodeToKeyCode[121] = KEYCODE_VOLUME_MUTE;
		_PlatformKeyCodeToKeyCode[179] = KEYCODE_MUSIC;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_EXPLORER;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_ENVELOPE;
		_PlatformKeyCodeToKeyCode[164] = KEYCODE_BOOKMARK;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_ZOOM_IN;
		// _PlatformKeyCodeToKeyCode[0] = KEYCODE_ZOOM_OUT;
	}

	KeyboardAdapter* KeyboardAdapter::create() {
		return new LinuxKeyboardAdapter();
	}

}
