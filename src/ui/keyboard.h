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

#ifndef __quark__keyboard__
#define __quark__keyboard__

#include "../util/util.h"
#include "../util/dict.h"

namespace qk {

	class EventDispatch;

// Keep these codes
// 0x01 SOH(start of headline) 标题开始
// 0x02 STX (start of text) 正文开始
// 0x03 ETX (end of text) 正文结束
// 0x04 EOT (end of transmission) 传输结束
// 0x05 ENQ (enquiry) 请求
// 0x06 ACK (acknowledge) 收到通知
// 0x07 BEL (bell) 响铃
#define Qk_Keyboard_Keys(F) \
	F(KEYCODE_UNKNOWN,          0)  \
	F(KEYCODE_BACK_SPACE,       8)  /* back space */ \
	F(KEYCODE_TAB,              9)  /* tab */ \
	F(KEYCODE_CLEAR,            12) /* clear */ \
	F(KEYCODE_ENTER,            13) /* enter */ \
	F(KEYCODE_SHIFT,            16) /* shift */ \
	F(KEYCODE_CTRL,             17) /* ctrl */ \
	F(KEYCODE_ALT,              18) /* alt */ \
	F(KEYCODE_BREAK,            19) /* break */\
	F(KEYCODE_CAPS_LOCK,        20) /* caps lock */ \
	F(KEYCODE_ESC,              27) /* esc */ \
	F(KEYCODE_SPACE,            32) /* space */ \
	F(KEYCODE_PAGE_UP,          33) /* page up */ \
	F(KEYCODE_PAGE_DOWN,        34) /* page down */ \
	F(KEYCODE_MOVE_END,         35) /* end */ \
	F(KEYCODE_MOVE_HOME,        36) /* home */ \
	F(KEYCODE_LEFT,             37) /* left */ \
	F(KEYCODE_UP,               38) /* up */ \
	F(KEYCODE_RIGHT,            39) /* right */ \
	F(KEYCODE_DOWN,             40) /* down */ \
	F(KEYCODE_INSERT,           45) /* insert */ \
	F(KEYCODE_DELETE,           46) /* delete */ \
	F(KEYCODE_SYSRQ,            42) /* sysrq */\
	F(KEYCODE_HELP,             47) /* help */ \
	F(KEYCODE_0,                48) /* 0 ) */ \
	F(KEYCODE_1,                49) /* 1 ! */ \
	F(KEYCODE_2,                50) /* 2 @ */ \
	F(KEYCODE_3,                51) /* 3 # */ \
	F(KEYCODE_4,                52) /* 4 $ */ \
	F(KEYCODE_5,                53) /* 5 % */ \
	F(KEYCODE_6,                54) /* 6 ^ */ \
	F(KEYCODE_7,                55) /* 7 & */ \
	F(KEYCODE_8,                56) /* 8 * */ \
	F(KEYCODE_9,                57) /* 9 ( */ \
	F(KEYCODE_A,                65) /* a-z */ \
	F(KEYCODE_B,                66)\
	F(KEYCODE_C,                67)\
	F(KEYCODE_D,                68)\
	F(KEYCODE_E,                69)\
	F(KEYCODE_F,                70)\
	F(KEYCODE_G,                71)\
	F(KEYCODE_H,                72)\
	F(KEYCODE_I,                73)\
	F(KEYCODE_J,                74)\
	F(KEYCODE_K,                75)\
	F(KEYCODE_L,                76)\
	F(KEYCODE_M,                77)\
	F(KEYCODE_N,                78)\
	F(KEYCODE_O,                79)\
	F(KEYCODE_P,                80)\
	F(KEYCODE_Q,                81)\
	F(KEYCODE_R,                82)\
	F(KEYCODE_S,                83)\
	F(KEYCODE_T,                84)\
	F(KEYCODE_U,                85)\
	F(KEYCODE_V,                86)\
	F(KEYCODE_W,                87)\
	F(KEYCODE_X,                88)\
	F(KEYCODE_Y,                89)\
	F(KEYCODE_Z,                90)\
	F(KEYCODE_COMMAND,          91) /* command */ \
	F(KEYCODE_MENU,             92) /* menu */\
	F(KEYCODE_COMMAND_RIGHT,    93) /* command right ************** */ \
	F(KEYCODE_NUMPAD_EQUALS,    94) /* numpad = */ \
	F(KEYCODE_NUMPAD_0,         96) /* numpad 0-9 */\
	F(KEYCODE_NUMPAD_1,         97)\
	F(KEYCODE_NUMPAD_2,         98)\
	F(KEYCODE_NUMPAD_3,         99)\
	F(KEYCODE_NUMPAD_4,         100)\
	F(KEYCODE_NUMPAD_5,         101)\
	F(KEYCODE_NUMPAD_6,         102)\
	F(KEYCODE_NUMPAD_7,         103)\
	F(KEYCODE_NUMPAD_8,         104)\
	F(KEYCODE_NUMPAD_9,         105)\
	F(KEYCODE_NUMPAD_MULTIPLY,  106) /* * */ \
	F(KEYCODE_NUMPAD_ADD,       107) /* + */ \
	F(KEYCODE_NUMPAD_ENTER,     108) /* Numpad Enter ************** */ \
	F(KEYCODE_NUMPAD_SUBTRACT,  109)  /* - */ \
	F(KEYCODE_NUMPAD_DOT,       110) /* . */ \
	F(KEYCODE_NUMPAD_DIVIDE,    111) /* / */ \
	F(KEYCODE_F1,               112) /* f1 - f24 */ \
	F(KEYCODE_F2,               113) \
	F(KEYCODE_F3,               114) \
	F(KEYCODE_F4,               115) \
	F(KEYCODE_F5,               116) \
	F(KEYCODE_F6,               117) \
	F(KEYCODE_F7,               118) \
	F(KEYCODE_F8,               119) \
	F(KEYCODE_F9,               120) \
	F(KEYCODE_F10,              121) \
	F(KEYCODE_F11,              122) \
	F(KEYCODE_F12,              123) \
	F(KEYCODE_F13,              124) \
	F(KEYCODE_F14,              125) \
	F(KEYCODE_F15,              126) \
	F(KEYCODE_F16,              127) \
	F(KEYCODE_F17,              128) \
	F(KEYCODE_F18,              129) \
	F(KEYCODE_F19,              130) \
	F(KEYCODE_F20,              131) \
	F(KEYCODE_F21,              132) \
	F(KEYCODE_F22,              133) \
	F(KEYCODE_F23,              134) \
	F(KEYCODE_F24,              135) \
	F(KEYCODE_NUM_LOCK,         144) /* num lock */ \
	F(KEYCODE_SCROLL_LOCK,      145) /* SCROLL_LOCK */\
	F(KEYCODE_SEMICOLON,        186)  /* ; : */ \
	F(KEYCODE_EQUALS,           187)  /* = + */ \
	F(KEYCODE_MINUS,            189)  /* - _ */ \
	F(KEYCODE_COMMA,            188)  /* , < */ \
	F(KEYCODE_PERIOD,           190)  /* . > */ \
	F(KEYCODE_SLASH,            191)  /* / ? */ \
	F(KEYCODE_GRAVE,            192)  /* ` ~ */ \
	F(KEYCODE_FUN,              209)  /* Function */ /*Maybe the platform is not implemented*/ \
	F(KEYCODE_LEFT_BRACKET,     219)  /* [ { */ \
	F(KEYCODE_BACK_SLASH,       220)  /* \ | */ \
	F(KEYCODE_RIGHT_BRACKET,    221)  /* ] } */ \
	F(KEYCODE_APOSTROPHE,       222)  /* ' " */ \
	/* --------------------------------------------------- */ \
	F(KEYCODE_MOUSE_LEFT,       296)  \
	F(KEYCODE_MOUSE_CENTER,     297)  \
	F(KEYCODE_MOUSE_RIGHT,      298)  \
	F(KEYCODE_MOUSE_WHEEL,      299)  \
	/* --------------------------------------------------- */ \
	F(KEYCODE_HOME,             300)     /* 按键Home */ \
	F(KEYCODE_BACK,             301)     /* 返回键 */ \
	F(KEYCODE_CALL,             302)     /* 拨号键 */ \
	F(KEYCODE_ENDCALL,          303)     /* 挂机键 */ \
	F(KEYCODE_STAR,             304)     /* * */ \
	F(KEYCODE_POUND,            305)     /* # */ \
	F(KEYCODE_CENTER,           306)     /* 导航键 确定键 */ \
	F(KEYCODE_VOLUME_UP,        307)     /* 音量增加键 */ \
	F(KEYCODE_VOLUME_DOWN,      308)     /* 音量减小键 */ \
	F(KEYCODE_POWER,            309)     /* 电源键 */ \
	F(KEYCODE_CAMERA,           310)     /* 拍照键 */ \
	F(KEYCODE_FOCUS,            311)     /* 拍照对焦键 */ \
	F(KEYCODE_MENU_1,           312)     /* 菜单键 */ \
	F(KEYCODE_SEARCH,           313)     /* 搜索键 */ \
	F(KEYCODE_MEDIA_PLAY_PAUSE, 314)     /* 多媒体键 播放/暂停 */ \
	F(KEYCODE_MEDIA_STOP,       315)     /* 多媒体键 停止 */ \
	F(KEYCODE_MEDIA_NEXT,       316)     /* 多媒体键 下一首 */ \
	F(KEYCODE_MEDIA_PREVIOUS,   317)     /* 多媒体键 上一首 */ \
	F(KEYCODE_MEDIA_REWIND,     318)     /* 多媒体键 快退 */ \
	F(KEYCODE_MEDIA_FAST_FORWARD,319)    /* 多媒体键 快进 */ \
	F(KEYCODE_MUTE,             320)     /* 话筒静音键 */ \
	F(KEYCODE_CHANNEL_UP,       321)     /* 按键Channel up */ \
	F(KEYCODE_CHANNEL_DOWN,     322)     /* 按键Channel down */ \
	F(KEYCODE_MEDIA_PLAY,       323)     /* 多媒体键 播放 */ \
	F(KEYCODE_MEDIA_PAUSE,      324)     /* 多媒体键 暂停 */ \
	F(KEYCODE_MEDIA_CLOSE,      325)     /* 多媒体键 关闭 */ \
	F(KEYCODE_MEDIA_EJECT,      326)     /* 多媒体键 弹出 */ \
	F(KEYCODE_MEDIA_RECORD,     327)     /* 多媒体键 录音 */ \
	F(KEYCODE_VOLUME_MUTE,      328)     /* 扬声器静音键 */ \
	F(KEYCODE_MUSIC,            329)     /* music */ \
	F(KEYCODE_EXPLORER,         330)     /* 按键Explorer special function */ \
	F(KEYCODE_ENVELOPE,         331)     /* 按键Envelope special function */ \
	F(KEYCODE_BOOKMARK,         332)     /* 按键Bookmark */ \
	F(KEYCODE_ZOOM_IN,          333)     /* 放大键 */ \
	F(KEYCODE_ZOOM_OUT,         334)     /* 缩小键 */ \

	enum KeyboardKeyCode {
		#define _Fun(Name, Code) Name = Code,
		Qk_Keyboard_Keys(_Fun)
		#undef _Fun
	};

	/**
	* @class KeyboardAdapter
	*/
	class Qk_Export KeyboardAdapter: public Object {
	public:
		typedef int Ascii;
		Qk_DEFINE_P_GET(EventDispatch*, host);
		Qk_DEFINE_P_GET(KeyboardKeyCode, keycode, Const);
		Qk_DEFINE_P_GET(Ascii, keypress, Const);
		Qk_DEFINE_P_GET(bool, shift, Const);
		Qk_DEFINE_P_GET(bool, alt, Const);
		Qk_DEFINE_P_GET(bool, ctrl, Const);
		Qk_DEFINE_P_GET(bool, command, Const);
		Qk_DEFINE_P_GET(bool, caps_lock, Const);
		Qk_DEFINE_P_GET(bool, repeat, Const);
		Qk_DEFINE_P_GET(bool, device, Const);
		Qk_DEFINE_P_GET(bool, source, Const);
		KeyboardAdapter();
		void dispatch(
			int code, bool isAscii, bool isDown, bool isCapsLock,
			int repeat, int device, int source
		);
		Ascii toKeypress(KeyboardKeyCode code);
		static KeyboardAdapter* create();
	protected:
		virtual void onDispatch(int code, bool isAscii, bool isDown);

		struct AsciiToKeyCode {
			KeyboardKeyCode code;
			bool shift;
		};
		struct KeyCodeToKeypress {
			Ascii normal, shift; // Keypress ascii code
		};
		Dict<int, KeyboardKeyCode>   _PlatformKeyCodeToKeyCode;
		Dict<int, AsciiToKeyCode>    _AsciiToKeyCode;
		Dict<int, KeyCodeToKeypress> _KeyCodeToKeypress;

		friend class EventDispatch;
	};

}
#endif
