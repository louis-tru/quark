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

#include "../keyboard.h"
#include "qgr/utils/map.h"

XX_NS(qgr)

/**
 * @class LinuxKeyboardAdapter
 */
class LinuxKeyboardAdapter: public KeyboardAdapter {
 public:
	LinuxKeyboardAdapter();
};

LinuxKeyboardAdapter::LinuxKeyboardAdapter() {
	m_keycodes[22] = KEYCODE_BACK_SPACE;
	m_keycodes[23] = KEYCODE_TAB;
	// m_keycodes[AKEYCODE_CLEAR] = KEYCODE_CLEAR;
	m_keycodes[36] = KEYCODE_ENTER;
	m_keycodes[50] = KEYCODE_SHIFT;
	m_keycodes[62] = KEYCODE_SHIFT;
	m_keycodes[37] = KEYCODE_CTRL;
	m_keycodes[105] = KEYCODE_CTRL;
	m_keycodes[64] = KEYCODE_ALT;
	m_keycodes[108] = KEYCODE_ALT;
	m_keycodes[66] = KEYCODE_CAPS_LOCK;
	m_keycodes[9] = KEYCODE_ESC;
	m_keycodes[65] = KEYCODE_SPACE;
	m_keycodes[133] = KEYCODE_COMMAND; // Pending query
	m_keycodes[113] = KEYCODE_LEFT;
	m_keycodes[111] = KEYCODE_UP;
	m_keycodes[114] = KEYCODE_RIGHT;
	m_keycodes[116] = KEYCODE_DOWN;
	m_keycodes[118] = KEYCODE_INSERT;
	m_keycodes[119] = KEYCODE_DELETE;
	m_keycodes[112] = KEYCODE_PAGE_UP;
	m_keycodes[117] = KEYCODE_PAGE_DOWN;
	m_keycodes[115] = KEYCODE_MOVE_END;
	m_keycodes[110] = KEYCODE_MOVE_HOME;
	m_keycodes[78] = KEYCODE_SCROLL_LOCK;
	m_keycodes[127] = KEYCODE_BREAK;
	m_keycodes[107] = KEYCODE_SYSRQ;
	m_keycodes[19] = KEYCODE_0;
	m_keycodes[10] = KEYCODE_1;
	m_keycodes[11] = KEYCODE_2;
	m_keycodes[12] = KEYCODE_3;
	m_keycodes[13] = KEYCODE_4;
	m_keycodes[14] = KEYCODE_5;
	m_keycodes[15] = KEYCODE_6;
	m_keycodes[16] = KEYCODE_7;
	m_keycodes[17] = KEYCODE_8;
	m_keycodes[18] = KEYCODE_9;
	m_keycodes[38] = KEYCODE_A;
	m_keycodes[56] = KEYCODE_B;
	m_keycodes[54] = KEYCODE_C;
	m_keycodes[40] = KEYCODE_D;
	m_keycodes[26] = KEYCODE_E;
	m_keycodes[41] = KEYCODE_F;
	m_keycodes[42] = KEYCODE_G;
	m_keycodes[43] = KEYCODE_H;
	m_keycodes[31] = KEYCODE_I;
	m_keycodes[44] = KEYCODE_J;
	m_keycodes[45] = KEYCODE_K;
	m_keycodes[46] = KEYCODE_L;
	m_keycodes[58] = KEYCODE_M;
	m_keycodes[57] = KEYCODE_N;
	m_keycodes[32] = KEYCODE_O;
	m_keycodes[33] = KEYCODE_P;
	m_keycodes[24] = KEYCODE_Q;
	m_keycodes[27] = KEYCODE_R;
	m_keycodes[39] = KEYCODE_S;
	m_keycodes[28] = KEYCODE_T;
	m_keycodes[30] = KEYCODE_U;
	m_keycodes[55] = KEYCODE_V;
	m_keycodes[25] = KEYCODE_W;
	m_keycodes[53] = KEYCODE_X;
	m_keycodes[29] = KEYCODE_Y;
	m_keycodes[52] = KEYCODE_Z;
	m_keycodes[77] = KEYCODE_NUM_LOCK;
	m_keycodes[90] = KEYCODE_NUMPAD_0;
	m_keycodes[87] = KEYCODE_NUMPAD_1;
	m_keycodes[88] = KEYCODE_NUMPAD_2;
	m_keycodes[89] = KEYCODE_NUMPAD_3;
	m_keycodes[83] = KEYCODE_NUMPAD_4;
	m_keycodes[84] = KEYCODE_NUMPAD_5;
	m_keycodes[85] = KEYCODE_NUMPAD_6;
	m_keycodes[79] = KEYCODE_NUMPAD_7;
	m_keycodes[80] = KEYCODE_NUMPAD_8;
	m_keycodes[81] = KEYCODE_NUMPAD_9;
	m_keycodes[106] = KEYCODE_NUMPAD_DIVIDE;
	m_keycodes[63] = KEYCODE_NUMPAD_MULTIPLY;
	m_keycodes[82] = KEYCODE_NUMPAD_SUBTRACT;
	m_keycodes[86] = KEYCODE_NUMPAD_ADD;
	m_keycodes[91] = KEYCODE_NUMPAD_DOT;
	m_keycodes[104] = KEYCODE_NUMPAD_ENTER;
	m_keycodes[67] = KEYCODE_F1;
	m_keycodes[68] = KEYCODE_F2;
	m_keycodes[69] = KEYCODE_F3;
	m_keycodes[70] = KEYCODE_F4;
	m_keycodes[71] = KEYCODE_F5;
	m_keycodes[72] = KEYCODE_F6;
	m_keycodes[73] = KEYCODE_F7;
	m_keycodes[74] = KEYCODE_F8;
	m_keycodes[75] = KEYCODE_F9;
	m_keycodes[76] = KEYCODE_F10;
	m_keycodes[95] = KEYCODE_F11;
	m_keycodes[96] = KEYCODE_F12;
	m_keycodes[47] = KEYCODE_SEMICOLON; // :
	m_keycodes[21] = KEYCODE_EQUALS; // =
	m_keycodes[20] = KEYCODE_MINUS; // -
	m_keycodes[59] = KEYCODE_COMMA; // ,
	m_keycodes[60] = KEYCODE_PERIOD; // .
	m_keycodes[61] = KEYCODE_SLASH;
	m_keycodes[49] = KEYCODE_GRAVE;
	m_keycodes[34] = KEYCODE_LEFT_BRACKET;
	m_keycodes[51] = KEYCODE_BACK_SLASH;
	m_keycodes[35] = KEYCODE_RIGHT_BRACKET;
	m_keycodes[48] = KEYCODE_APOSTROPHE;
	m_keycodes[180] = KEYCODE_HOME;
	// m_keycodes[0] = KEYCODE_BACK;                 /* 返回键 */
	// m_keycodes[0] = KEYCODE_CALL;                 /* 拨号键 */
	// m_keycodes[0] = KEYCODE_ENDCALL;              /* 挂机键 */
	// m_keycodes[0] = KEYCODE_STAR;                 /* * */
	// m_keycodes[0] = KEYCODE_POUND;                /* # */
	// m_keycodes[0] = KEYCODE_CENTER;               /* 导航键 确定键 */
	m_keycodes[123] = KEYCODE_VOLUME_UP;
	m_keycodes[122] = KEYCODE_VOLUME_DOWN;
	m_keycodes[124] = KEYCODE_POWER;
	// m_keycodes[0] = KEYCODE_CAMERA;               /* 拍照键 */
	// m_keycodes[0] = KEYCODE_FOCUS;                /* 拍照对焦键 */
	m_keycodes[135] = KEYCODE_MENU;
	// m_keycodes[0] = KEYCODE_SEARCH;               /* 搜索键 */
	m_keycodes[172] = KEYCODE_MEDIA_PLAY_PAUSE;
	m_keycodes[174] = KEYCODE_MEDIA_STOP;
	m_keycodes[171] = KEYCODE_MEDIA_NEXT;
	m_keycodes[173] = KEYCODE_MEDIA_PREVIOUS;
	// m_keycodes[0] = KEYCODE_MEDIA_REWIND;         /* 多媒体键 快退 */
	// m_keycodes[0] = KEYCODE_MEDIA_FAST_FORWARD;   /* 多媒体键 快进 */
	// m_keycodes[0] = KEYCODE_MUTE;                 /* 话筒静音键 */
	// m_keycodes[0] = KEYCODE_CHANNEL_UP;           /* 按键Channel up */
	// m_keycodes[0] = KEYCODE_CHANNEL_DOWN;         /* 按键Channel down */
	// m_keycodes[0] = KEYCODE_MEDIA_PLAY;           /* 多媒体键 播放 */
	// m_keycodes[0] = KEYCODE_MEDIA_PAUSE;          /* 多媒体键 暂停 */
	// m_keycodes[0] = KEYCODE_MEDIA_CLOSE;          /* 多媒体键 关闭 */
	// m_keycodes[0] = KEYCODE_MEDIA_EJECT;          /* 多媒体键 弹出 */
	// m_keycodes[0] = KEYCODE_MEDIA_RECORD;         /* 多媒体键 录音 */
	m_keycodes[121] = KEYCODE_VOLUME_MUTE;
	m_keycodes[179] = KEYCODE_MUSIC;
	// m_keycodes[0] = KEYCODE_EXPLORER;             /* 按键Explorer special function */
	// m_keycodes[0] = KEYCODE_ENVELOPE;             /* 按键Envelope special function */ 
	m_keycodes[164] = KEYCODE_BOOKMARK;              /* 按键Bookmark */
	// m_keycodes[0] = KEYCODE_ZOOM_IN;              /* 放大键 */
	// m_keycodes[0] = KEYCODE_ZOOM_OUT;             /* 缩小键 */
	// m_keycodes[0] = KEYCODE_HELP;                 /* Help */
}

KeyboardAdapter* KeyboardAdapter::create() {
	return new LinuxKeyboardAdapter();
}

XX_END
