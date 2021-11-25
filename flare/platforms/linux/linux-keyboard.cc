/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "flare/keyboard.h"
#include "flare/util/dict.h"

namespace flare {

	/**
	* @class UnixKeyboardAdapter
	*/
	class UnixKeyboardAdapter: public KeyboardAdapter {
		public:
			UnixKeyboardAdapter();
	};

	UnixKeyboardAdapter::UnixKeyboardAdapter() {
		_keycodes[22] = KEYCODE_BACK_SPACE;
		_keycodes[23] = KEYCODE_TAB;
		// _keycodes[AKEYCODE_CLEAR] = KEYCODE_CLEAR;
		_keycodes[36] = KEYCODE_ENTER;
		_keycodes[50] = KEYCODE_SHIFT;
		_keycodes[62] = KEYCODE_SHIFT;
		_keycodes[37] = KEYCODE_CTRL;
		_keycodes[105] = KEYCODE_CTRL;
		_keycodes[64] = KEYCODE_ALT;
		_keycodes[108] = KEYCODE_ALT;
		_keycodes[66] = KEYCODE_CAPS_LOCK;
		_keycodes[9] = KEYCODE_ESC;
		_keycodes[65] = KEYCODE_SPACE;
		_keycodes[133] = KEYCODE_COMMAND; // Pending query
		_keycodes[113] = KEYCODE_LEFT;
		_keycodes[111] = KEYCODE_UP;
		_keycodes[114] = KEYCODE_RIGHT;
		_keycodes[116] = KEYCODE_DOWN;
		_keycodes[118] = KEYCODE_INSERT;
		_keycodes[119] = KEYCODE_DELETE;
		_keycodes[112] = KEYCODE_PAGE_UP;
		_keycodes[117] = KEYCODE_PAGE_DOWN;
		_keycodes[115] = KEYCODE_MOVE_END;
		_keycodes[110] = KEYCODE_MOVE_HOME;
		_keycodes[78] = KEYCODE_SCROLL_LOCK;
		_keycodes[127] = KEYCODE_BREAK;
		_keycodes[107] = KEYCODE_SYSRQ;
		_keycodes[19] = KEYCODE_0;
		_keycodes[10] = KEYCODE_1;
		_keycodes[11] = KEYCODE_2;
		_keycodes[12] = KEYCODE_3;
		_keycodes[13] = KEYCODE_4;
		_keycodes[14] = KEYCODE_5;
		_keycodes[15] = KEYCODE_6;
		_keycodes[16] = KEYCODE_7;
		_keycodes[17] = KEYCODE_8;
		_keycodes[18] = KEYCODE_9;
		_keycodes[38] = KEYCODE_A;
		_keycodes[56] = KEYCODE_B;
		_keycodes[54] = KEYCODE_C;
		_keycodes[40] = KEYCODE_D;
		_keycodes[26] = KEYCODE_E;
		_keycodes[41] = KEYCODE_F;
		_keycodes[42] = KEYCODE_G;
		_keycodes[43] = KEYCODE_H;
		_keycodes[31] = KEYCODE_I;
		_keycodes[44] = KEYCODE_J;
		_keycodes[45] = KEYCODE_K;
		_keycodes[46] = KEYCODE_L;
		_keycodes[58] = KEYCODE_M;
		_keycodes[57] = KEYCODE_N;
		_keycodes[32] = KEYCODE_O;
		_keycodes[33] = KEYCODE_P;
		_keycodes[24] = KEYCODE_Q;
		_keycodes[27] = KEYCODE_R;
		_keycodes[39] = KEYCODE_S;
		_keycodes[28] = KEYCODE_T;
		_keycodes[30] = KEYCODE_U;
		_keycodes[55] = KEYCODE_V;
		_keycodes[25] = KEYCODE_W;
		_keycodes[53] = KEYCODE_X;
		_keycodes[29] = KEYCODE_Y;
		_keycodes[52] = KEYCODE_Z;
		_keycodes[77] = KEYCODE_NUM_LOCK;
		_keycodes[90] = KEYCODE_NUMPAD_0;
		_keycodes[87] = KEYCODE_NUMPAD_1;
		_keycodes[88] = KEYCODE_NUMPAD_2;
		_keycodes[89] = KEYCODE_NUMPAD_3;
		_keycodes[83] = KEYCODE_NUMPAD_4;
		_keycodes[84] = KEYCODE_NUMPAD_5;
		_keycodes[85] = KEYCODE_NUMPAD_6;
		_keycodes[79] = KEYCODE_NUMPAD_7;
		_keycodes[80] = KEYCODE_NUMPAD_8;
		_keycodes[81] = KEYCODE_NUMPAD_9;
		_keycodes[106] = KEYCODE_NUMPAD_DIVIDE;
		_keycodes[63] = KEYCODE_NUMPAD_MULTIPLY;
		_keycodes[82] = KEYCODE_NUMPAD_SUBTRACT;
		_keycodes[86] = KEYCODE_NUMPAD_ADD;
		_keycodes[91] = KEYCODE_NUMPAD_DOT;
		_keycodes[104] = KEYCODE_NUMPAD_ENTER;
		_keycodes[67] = KEYCODE_F1;
		_keycodes[68] = KEYCODE_F2;
		_keycodes[69] = KEYCODE_F3;
		_keycodes[70] = KEYCODE_F4;
		_keycodes[71] = KEYCODE_F5;
		_keycodes[72] = KEYCODE_F6;
		_keycodes[73] = KEYCODE_F7;
		_keycodes[74] = KEYCODE_F8;
		_keycodes[75] = KEYCODE_F9;
		_keycodes[76] = KEYCODE_F10;
		_keycodes[95] = KEYCODE_F11;
		_keycodes[96] = KEYCODE_F12;
		_keycodes[47] = KEYCODE_SEMICOLON; // :
		_keycodes[21] = KEYCODE_EQUALS; // =
		_keycodes[20] = KEYCODE_MINUS; // -
		_keycodes[59] = KEYCODE_COMMA; // ,
		_keycodes[60] = KEYCODE_PERIOD; // .
		_keycodes[61] = KEYCODE_SLASH;
		_keycodes[49] = KEYCODE_GRAVE;
		_keycodes[34] = KEYCODE_LEFT_BRACKET;
		_keycodes[51] = KEYCODE_BACK_SLASH;
		_keycodes[35] = KEYCODE_RIGHT_BRACKET;
		_keycodes[48] = KEYCODE_APOSTROPHE;
		_keycodes[180] = KEYCODE_HOME;
		// _keycodes[0] = KEYCODE_BACK;                 /* 返回键 */
		// _keycodes[0] = KEYCODE_CALL;                 /* 拨号键 */
		// _keycodes[0] = KEYCODE_ENDCALL;              /* 挂机键 */
		// _keycodes[0] = KEYCODE_STAR;                 /* * */
		// _keycodes[0] = KEYCODE_POUND;                /* # */
		// _keycodes[0] = KEYCODE_CENTER;               /* 导航键 确定键 */
		_keycodes[123] = KEYCODE_VOLUME_UP;
		_keycodes[122] = KEYCODE_VOLUME_DOWN;
		_keycodes[124] = KEYCODE_POWER;
		// _keycodes[0] = KEYCODE_CAMERA;               /* 拍照键 */
		// _keycodes[0] = KEYCODE_FOCUS;                /* 拍照对焦键 */
		_keycodes[135] = KEYCODE_MENU;
		// _keycodes[0] = KEYCODE_SEARCH;               /* 搜索键 */
		_keycodes[172] = KEYCODE_MEDIA_PLAY_PAUSE;
		_keycodes[174] = KEYCODE_MEDIA_STOP;
		_keycodes[171] = KEYCODE_MEDIA_NEXT;
		_keycodes[173] = KEYCODE_MEDIA_PREVIOUS;
		// _keycodes[0] = KEYCODE_MEDIA_REWIND;         /* 多媒体键 快退 */
		// _keycodes[0] = KEYCODE_MEDIA_FAST_FORWARD;   /* 多媒体键 快进 */
		// _keycodes[0] = KEYCODE_MUTE;                 /* 话筒静音键 */
		// _keycodes[0] = KEYCODE_CHANNEL_UP;           /* 按键Channel up */
		// _keycodes[0] = KEYCODE_CHANNEL_DOWN;         /* 按键Channel down */
		// _keycodes[0] = KEYCODE_MEDIA_PLAY;           /* 多媒体键 播放 */
		// _keycodes[0] = KEYCODE_MEDIA_PAUSE;          /* 多媒体键 暂停 */
		// _keycodes[0] = KEYCODE_MEDIA_CLOSE;          /* 多媒体键 关闭 */
		// _keycodes[0] = KEYCODE_MEDIA_EJECT;          /* 多媒体键 弹出 */
		// _keycodes[0] = KEYCODE_MEDIA_RECORD;         /* 多媒体键 录音 */
		_keycodes[121] = KEYCODE_VOLUME_MUTE;
		_keycodes[179] = KEYCODE_MUSIC;
		// _keycodes[0] = KEYCODE_EXPLORER;             /* 按键Explorer special function */
		// _keycodes[0] = KEYCODE_ENVELOPE;             /* 按键Envelope special function */ 
		_keycodes[164] = KEYCODE_BOOKMARK;              /* 按键Bookmark */
		// _keycodes[0] = KEYCODE_ZOOM_IN;              /* 放大键 */
		// _keycodes[0] = KEYCODE_ZOOM_OUT;             /* 缩小键 */
		// _keycodes[0] = KEYCODE_HELP;                 /* Help */
	}

	KeyboardAdapter* KeyboardAdapter::create() {
		return new UnixKeyboardAdapter();
	}

}
