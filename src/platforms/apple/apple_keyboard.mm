/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#import "../../ui/keyboard.h"

namespace qk {

	class AppleKeyboardAdapter: public KeyboardAdapter {
	public:
		AppleKeyboardAdapter();
	};

	AppleKeyboardAdapter::AppleKeyboardAdapter() {
		_PlatformKeyCodeToKeyCode[0] = KEYCODE_A;
		_PlatformKeyCodeToKeyCode[1] = KEYCODE_S;
		_PlatformKeyCodeToKeyCode[2] = KEYCODE_D;
		_PlatformKeyCodeToKeyCode[3] = KEYCODE_F;
		_PlatformKeyCodeToKeyCode[4] = KEYCODE_H;
		_PlatformKeyCodeToKeyCode[5] = KEYCODE_G;
		_PlatformKeyCodeToKeyCode[6] = KEYCODE_Z;
		_PlatformKeyCodeToKeyCode[7] = KEYCODE_X;
		_PlatformKeyCodeToKeyCode[8] = KEYCODE_C;
		_PlatformKeyCodeToKeyCode[9] = KEYCODE_V;
		// _PlatformKeyCodeToKeyCode[10] = ISO_Section
		_PlatformKeyCodeToKeyCode[11] = KEYCODE_B;
		_PlatformKeyCodeToKeyCode[12] = KEYCODE_Q;
		_PlatformKeyCodeToKeyCode[13] = KEYCODE_W;
		_PlatformKeyCodeToKeyCode[14] = KEYCODE_E;
		_PlatformKeyCodeToKeyCode[15] = KEYCODE_R;
		_PlatformKeyCodeToKeyCode[16] = KEYCODE_Y;
		_PlatformKeyCodeToKeyCode[17] = KEYCODE_T;
		_PlatformKeyCodeToKeyCode[18] = KEYCODE_1;
		_PlatformKeyCodeToKeyCode[19] = KEYCODE_2;
		_PlatformKeyCodeToKeyCode[20] = KEYCODE_3;
		_PlatformKeyCodeToKeyCode[21] = KEYCODE_4;
		_PlatformKeyCodeToKeyCode[22] = KEYCODE_6;
		_PlatformKeyCodeToKeyCode[23] = KEYCODE_5;
		_PlatformKeyCodeToKeyCode[24] = KEYCODE_EQUALS;
		_PlatformKeyCodeToKeyCode[25] = KEYCODE_9;
		_PlatformKeyCodeToKeyCode[26] = KEYCODE_7;
		_PlatformKeyCodeToKeyCode[27] = KEYCODE_MINUS;
		_PlatformKeyCodeToKeyCode[28] = KEYCODE_8;
		_PlatformKeyCodeToKeyCode[29] = KEYCODE_0;
		_PlatformKeyCodeToKeyCode[30] = KEYCODE_RIGHT_BRACKET;
		_PlatformKeyCodeToKeyCode[31] = KEYCODE_O;
		_PlatformKeyCodeToKeyCode[32] = KEYCODE_U;
		_PlatformKeyCodeToKeyCode[33] = KEYCODE_LEFT_BRACKET;
		_PlatformKeyCodeToKeyCode[34] = KEYCODE_I;
		_PlatformKeyCodeToKeyCode[35] = KEYCODE_P;
		_PlatformKeyCodeToKeyCode[36] = KEYCODE_ENTER; // Return
		_PlatformKeyCodeToKeyCode[37] = KEYCODE_L;
		_PlatformKeyCodeToKeyCode[38] = KEYCODE_J;
		_PlatformKeyCodeToKeyCode[39] = KEYCODE_APOSTROPHE;
		_PlatformKeyCodeToKeyCode[40] = KEYCODE_K;
		_PlatformKeyCodeToKeyCode[41] = KEYCODE_SEMICOLON;
		_PlatformKeyCodeToKeyCode[42] = KEYCODE_BACK_SLASH;
		_PlatformKeyCodeToKeyCode[43] = KEYCODE_COMMA;
		_PlatformKeyCodeToKeyCode[44] = KEYCODE_SLASH;
		_PlatformKeyCodeToKeyCode[45] = KEYCODE_N;
		_PlatformKeyCodeToKeyCode[46] = KEYCODE_M;
		_PlatformKeyCodeToKeyCode[47] = KEYCODE_PERIOD;
		_PlatformKeyCodeToKeyCode[48] = KEYCODE_TAB;
		_PlatformKeyCodeToKeyCode[49] = KEYCODE_SPACE;
		_PlatformKeyCodeToKeyCode[50] = KEYCODE_GRAVE;
		_PlatformKeyCodeToKeyCode[51] = KEYCODE_BACK_SPACE;//DELETE
		_PlatformKeyCodeToKeyCode[53] = KEYCODE_ESC;
		_PlatformKeyCodeToKeyCode[54] = KEYCODE_COMMAND_RIGHT; // Command Right
		_PlatformKeyCodeToKeyCode[55] = KEYCODE_COMMAND; // Command
		_PlatformKeyCodeToKeyCode[56] = KEYCODE_SHIFT;//Shift
		_PlatformKeyCodeToKeyCode[57] = KEYCODE_CAPS_LOCK;
		_PlatformKeyCodeToKeyCode[58] = KEYCODE_ALT;//Option
		_PlatformKeyCodeToKeyCode[59] = KEYCODE_CTRL;//Control
		_PlatformKeyCodeToKeyCode[60] = KEYCODE_SHIFT;//RightShift
		_PlatformKeyCodeToKeyCode[61] = KEYCODE_ALT;//RightOption
		_PlatformKeyCodeToKeyCode[62] = KEYCODE_CTRL;//RightControl
		_PlatformKeyCodeToKeyCode[63] = KEYCODE_FUN; //Function
		_PlatformKeyCodeToKeyCode[64] = KEYCODE_F17;
		_PlatformKeyCodeToKeyCode[65] = KEYCODE_NUMPAD_DOT;//ANSI_KeypadDecimal
		_PlatformKeyCodeToKeyCode[67] = KEYCODE_NUMPAD_MULTIPLY; // *
		_PlatformKeyCodeToKeyCode[69] = KEYCODE_NUMPAD_ADD; // +
		_PlatformKeyCodeToKeyCode[71] = KEYCODE_NUM_LOCK;//ANSI_KeypadClear;
		_PlatformKeyCodeToKeyCode[72] = KEYCODE_VOLUME_UP;
		_PlatformKeyCodeToKeyCode[73] = KEYCODE_VOLUME_DOWN;
		_PlatformKeyCodeToKeyCode[74] = KEYCODE_MUTE;//Mute
		_PlatformKeyCodeToKeyCode[75] = KEYCODE_NUMPAD_DIVIDE;
		_PlatformKeyCodeToKeyCode[76] = KEYCODE_NUMPAD_ENTER; // NUMPAD Enter
		_PlatformKeyCodeToKeyCode[78] = KEYCODE_NUMPAD_SUBTRACT;
		_PlatformKeyCodeToKeyCode[79] = KEYCODE_F18;
		_PlatformKeyCodeToKeyCode[80] = KEYCODE_F19;
		_PlatformKeyCodeToKeyCode[81] = KEYCODE_NUMPAD_EQUALS;//ANSI_KeypadEquals
		_PlatformKeyCodeToKeyCode[82] = KEYCODE_NUMPAD_0;
		_PlatformKeyCodeToKeyCode[83] = KEYCODE_NUMPAD_1;
		_PlatformKeyCodeToKeyCode[84] = KEYCODE_NUMPAD_2;
		_PlatformKeyCodeToKeyCode[85] = KEYCODE_NUMPAD_3;
		_PlatformKeyCodeToKeyCode[86] = KEYCODE_NUMPAD_4;
		_PlatformKeyCodeToKeyCode[87] = KEYCODE_NUMPAD_5;
		_PlatformKeyCodeToKeyCode[88] = KEYCODE_NUMPAD_6;
		_PlatformKeyCodeToKeyCode[89] = KEYCODE_NUMPAD_7;
		_PlatformKeyCodeToKeyCode[90] = KEYCODE_F20;
		_PlatformKeyCodeToKeyCode[91] = KEYCODE_NUMPAD_8;
		_PlatformKeyCodeToKeyCode[92] = KEYCODE_NUMPAD_9;
		//_PlatformKeyCodeToKeyCode[93] = JIS_Yen
		//_PlatformKeyCodeToKeyCode[94] = JIS_Underscore
		//_PlatformKeyCodeToKeyCode[95] = JIS_KeypadComma
		_PlatformKeyCodeToKeyCode[96] = KEYCODE_F5;
		_PlatformKeyCodeToKeyCode[97] = KEYCODE_F6;
		_PlatformKeyCodeToKeyCode[98] = KEYCODE_F7;
		_PlatformKeyCodeToKeyCode[99] = KEYCODE_F3;
		_PlatformKeyCodeToKeyCode[100] = KEYCODE_F8;
		_PlatformKeyCodeToKeyCode[101] = KEYCODE_F9;
		//_PlatformKeyCodeToKeyCode[102] = JIS_Eisu
		_PlatformKeyCodeToKeyCode[103] = KEYCODE_F11;
		// _PlatformKeyCodeToKeyCode[104] = JIS_Kana
		_PlatformKeyCodeToKeyCode[105] = KEYCODE_F13;
		_PlatformKeyCodeToKeyCode[106] = KEYCODE_F16;
		_PlatformKeyCodeToKeyCode[107] = KEYCODE_F14;
		_PlatformKeyCodeToKeyCode[109] = KEYCODE_F10;
		_PlatformKeyCodeToKeyCode[110] = KEYCODE_MENU;
		_PlatformKeyCodeToKeyCode[111] = KEYCODE_F12;
		_PlatformKeyCodeToKeyCode[113] = KEYCODE_F15;
		_PlatformKeyCodeToKeyCode[114] = KEYCODE_HELP;
		_PlatformKeyCodeToKeyCode[115] = KEYCODE_MOVE_HOME;
		_PlatformKeyCodeToKeyCode[116] = KEYCODE_PAGE_UP;
		_PlatformKeyCodeToKeyCode[117] = KEYCODE_DELETE;//ForwardDelete
		_PlatformKeyCodeToKeyCode[118] = KEYCODE_F4;
		_PlatformKeyCodeToKeyCode[119] = KEYCODE_MOVE_END;
		_PlatformKeyCodeToKeyCode[120] = KEYCODE_F2;
		_PlatformKeyCodeToKeyCode[121] = KEYCODE_PAGE_DOWN;
		_PlatformKeyCodeToKeyCode[122] = KEYCODE_F1;
		_PlatformKeyCodeToKeyCode[123] = KEYCODE_LEFT;
		_PlatformKeyCodeToKeyCode[124] = KEYCODE_RIGHT;
		_PlatformKeyCodeToKeyCode[125] = KEYCODE_DOWN;
		_PlatformKeyCodeToKeyCode[126] = KEYCODE_UP;
	}

	KeyboardAdapter* KeyboardAdapter::create() {
		return new AppleKeyboardAdapter();
	}

}
