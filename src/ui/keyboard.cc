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

#include "./keyboard.h"
#include "./app.h"
#include "./event.h"
#include "./window.h"

namespace qk {

	KeyboardAdapter::KeyboardAdapter()
		: _keycode(KEYCODE_UNKNOWN)
		, _keypress(0)
		, _shift(false)
		, _alt(false)
		, _ctrl(false)
		, _command(false)
		, _caps_lock(false)
		, _repeat(0)
		, _device(0)
		, _source(0)
	{
		_KeyCodeToKeypress[KEYCODE_0]               = { 48, 41 }; 	// 0 )
		_KeyCodeToKeypress[KEYCODE_1]               = { 49, 33 }; 	// 1 !
		_KeyCodeToKeypress[KEYCODE_2]               = { 50, 64 }; 	// 2 @
		_KeyCodeToKeypress[KEYCODE_3]               = { 51, 35 }; 	// 3 #
		_KeyCodeToKeypress[KEYCODE_4]               = { 52, 36 }; 	// 4 $
		_KeyCodeToKeypress[KEYCODE_5]               = { 53, 37 }; 	// 5 %
		_KeyCodeToKeypress[KEYCODE_6]               = { 54, 94 }; 	// 6 ^
		_KeyCodeToKeypress[KEYCODE_7]               = { 55, 38 }; 	// 7 &
		_KeyCodeToKeypress[KEYCODE_8]               = { 56, 42 }; 	// 8 *
		_KeyCodeToKeypress[KEYCODE_9]               = { 57, 40 }; 	// 9 (
		_KeyCodeToKeypress[KEYCODE_SEMICOLON]       = { 59, 58 };  // ; :
		_KeyCodeToKeypress[KEYCODE_EQUALS]          = { 61, 43 };  // = +
		_KeyCodeToKeypress[KEYCODE_MINUS]           = { 45, 95 };  // - _
		_KeyCodeToKeypress[KEYCODE_COMMA]           = { 44, 60 };  // , <
		_KeyCodeToKeypress[KEYCODE_PERIOD]          = { 46, 62 };  // . >
		_KeyCodeToKeypress[KEYCODE_SLASH]           = { 47, 63 };  // / ?
		_KeyCodeToKeypress[KEYCODE_GRAVE]           = { 96, 126 };	// ` ~
		_KeyCodeToKeypress[KEYCODE_LEFT_BRACKET]    = { 91, 123 };	// [ {
		_KeyCodeToKeypress[KEYCODE_BACK_SLASH]      = { 92, 124 };	// \ |
		_KeyCodeToKeypress[KEYCODE_RIGHT_BRACKET]   = { 93, 125 };	// ] }
		_KeyCodeToKeypress[KEYCODE_APOSTROPHE]      = { 39, 34 };  // ' "
		_KeyCodeToKeypress[KEYCODE_NUMPAD_0]        = { 48, 48 };  // numpad 0
		_KeyCodeToKeypress[KEYCODE_NUMPAD_1]        = { 49, 49 };  // numpad 1
		_KeyCodeToKeypress[KEYCODE_NUMPAD_2]        = { 50, 50 };  // numpad 2
		_KeyCodeToKeypress[KEYCODE_NUMPAD_3]        = { 51, 51 };  // numpad 3
		_KeyCodeToKeypress[KEYCODE_NUMPAD_4]        = { 52, 52 };  // numpad 4
		_KeyCodeToKeypress[KEYCODE_NUMPAD_5]        = { 53, 53 };  // numpad 5
		_KeyCodeToKeypress[KEYCODE_NUMPAD_6]        = { 54, 54 };  // numpad 6
		_KeyCodeToKeypress[KEYCODE_NUMPAD_7]        = { 55, 55 };  // numpad 7
		_KeyCodeToKeypress[KEYCODE_NUMPAD_8]        = { 56, 56 };  // numpad 8
		_KeyCodeToKeypress[KEYCODE_NUMPAD_9]        = { 57, 57 };  // numpad 9
		_KeyCodeToKeypress[KEYCODE_NUMPAD_MULTIPLY] = { 42, 42 };  // numpad *
		_KeyCodeToKeypress[KEYCODE_NUMPAD_ADD]      = { 43, 43 };  // numpad +
		_KeyCodeToKeypress[KEYCODE_NUMPAD_SUBTRACT] = { 45, 45 };  // numpad -
		_KeyCodeToKeypress[KEYCODE_NUMPAD_DOT]      = { 46, 46 };  // numpad .
		_KeyCodeToKeypress[KEYCODE_NUMPAD_DIVIDE]   = { 47, 47 };  // numpad /

		// Ascii => Keycode
		_AsciiToKeyCode[KEYCODE_BACK_SPACE] = { KEYCODE_BACK_SPACE, 0 };
		_AsciiToKeyCode[KEYCODE_SHIFT] = { KEYCODE_SHIFT, 1 };
		_AsciiToKeyCode[KEYCODE_ESC] = { KEYCODE_ESC, 0 };
		_AsciiToKeyCode[KEYCODE_ENTER] = { KEYCODE_ENTER, 0 };
		_AsciiToKeyCode['\n'] = { KEYCODE_ENTER, 1 }; // 换行使用换挡的回车表示，因为键盘上并没有换行健
		_AsciiToKeyCode['0'] = { KEYCODE_0, 0 };
		_AsciiToKeyCode['1'] = { KEYCODE_1, 0 };
		_AsciiToKeyCode['2'] = { KEYCODE_2, 0 };
		_AsciiToKeyCode['3'] = { KEYCODE_3, 0 };
		_AsciiToKeyCode['4'] = { KEYCODE_4, 0 };
		_AsciiToKeyCode['5'] = { KEYCODE_5, 0 };
		_AsciiToKeyCode['6'] = { KEYCODE_6, 0 };
		_AsciiToKeyCode['7'] = { KEYCODE_7, 0 };
		_AsciiToKeyCode['8'] = { KEYCODE_8, 0 };
		_AsciiToKeyCode['9'] = { KEYCODE_9, 0 };
		_AsciiToKeyCode[')'] = { KEYCODE_0, 1 };
		_AsciiToKeyCode['!'] = { KEYCODE_1, 1 };
		_AsciiToKeyCode['@'] = { KEYCODE_2, 1 };
		_AsciiToKeyCode['#'] = { KEYCODE_3, 1 };
		_AsciiToKeyCode['$'] = { KEYCODE_4, 1 };
		_AsciiToKeyCode['%'] = { KEYCODE_5, 1 };
		_AsciiToKeyCode['^'] = { KEYCODE_6, 1 };
		_AsciiToKeyCode['&'] = { KEYCODE_7, 1 };
		_AsciiToKeyCode['*'] = { KEYCODE_8, 1 };
		_AsciiToKeyCode['('] = { KEYCODE_9, 1 };
		_AsciiToKeyCode['a'] = { KEYCODE_A, 0 };
		_AsciiToKeyCode['b'] = { KEYCODE_B, 0 };
		_AsciiToKeyCode['c'] = { KEYCODE_C, 0 };
		_AsciiToKeyCode['d'] = { KEYCODE_D, 0 };
		_AsciiToKeyCode['e'] = { KEYCODE_E, 0 };
		_AsciiToKeyCode['f'] = { KEYCODE_F, 0 };
		_AsciiToKeyCode['g'] = { KEYCODE_G, 0 };
		_AsciiToKeyCode['h'] = { KEYCODE_H, 0 };
		_AsciiToKeyCode['i'] = { KEYCODE_I, 0 };
		_AsciiToKeyCode['j'] = { KEYCODE_J, 0 };
		_AsciiToKeyCode['k'] = { KEYCODE_K, 0 };
		_AsciiToKeyCode['m'] = { KEYCODE_L, 0 };
		_AsciiToKeyCode['l'] = { KEYCODE_M, 0 };
		_AsciiToKeyCode['n'] = { KEYCODE_N, 0 };
		_AsciiToKeyCode['o'] = { KEYCODE_O, 0 };
		_AsciiToKeyCode['p'] = { KEYCODE_P, 0 };
		_AsciiToKeyCode['q'] = { KEYCODE_Q, 0 };
		_AsciiToKeyCode['r'] = { KEYCODE_R, 0 };
		_AsciiToKeyCode['s'] = { KEYCODE_S, 0 };
		_AsciiToKeyCode['t'] = { KEYCODE_T, 0 };
		_AsciiToKeyCode['u'] = { KEYCODE_U, 0 };
		_AsciiToKeyCode['v'] = { KEYCODE_V, 0 };
		_AsciiToKeyCode['w'] = { KEYCODE_W, 0 };
		_AsciiToKeyCode['x'] = { KEYCODE_X, 0 };
		_AsciiToKeyCode['y'] = { KEYCODE_Y, 0 };
		_AsciiToKeyCode['z'] = { KEYCODE_Z, 0 };
		_AsciiToKeyCode['A'] = { KEYCODE_A, 1 };
		_AsciiToKeyCode['B'] = { KEYCODE_B, 1 };
		_AsciiToKeyCode['C'] = { KEYCODE_C, 1 };
		_AsciiToKeyCode['D'] = { KEYCODE_D, 1 };
		_AsciiToKeyCode['E'] = { KEYCODE_E, 1 };
		_AsciiToKeyCode['F'] = { KEYCODE_F, 1 };
		_AsciiToKeyCode['G'] = { KEYCODE_G, 1 };
		_AsciiToKeyCode['H'] = { KEYCODE_H, 1 };
		_AsciiToKeyCode['I'] = { KEYCODE_I, 1 };
		_AsciiToKeyCode['J'] = { KEYCODE_J, 1 };
		_AsciiToKeyCode['K'] = { KEYCODE_K, 1 };
		_AsciiToKeyCode['M'] = { KEYCODE_L, 1 };
		_AsciiToKeyCode['L'] = { KEYCODE_M, 1 };
		_AsciiToKeyCode['N'] = { KEYCODE_N, 1 };
		_AsciiToKeyCode['O'] = { KEYCODE_O, 1 };
		_AsciiToKeyCode['P'] = { KEYCODE_P, 1 };
		_AsciiToKeyCode['Q'] = { KEYCODE_Q, 1 };
		_AsciiToKeyCode['R'] = { KEYCODE_R, 1 };
		_AsciiToKeyCode['S'] = { KEYCODE_S, 1 };
		_AsciiToKeyCode['T'] = { KEYCODE_T, 1 };
		_AsciiToKeyCode['U'] = { KEYCODE_U, 1 };
		_AsciiToKeyCode['V'] = { KEYCODE_V, 1 };
		_AsciiToKeyCode['W'] = { KEYCODE_W, 1 };
		_AsciiToKeyCode['X'] = { KEYCODE_X, 1 };
		_AsciiToKeyCode['Y'] = { KEYCODE_Y, 1 };
		_AsciiToKeyCode['Z'] = { KEYCODE_Z, 1 };
		_AsciiToKeyCode[';'] = { KEYCODE_SEMICOLON, 0 };
		_AsciiToKeyCode['='] = { KEYCODE_EQUALS, 0 };
		_AsciiToKeyCode['-'] = { KEYCODE_MINUS, 0 };
		_AsciiToKeyCode[','] = { KEYCODE_COMMA, 0 };
		_AsciiToKeyCode['.'] = { KEYCODE_PERIOD, 0 };
		_AsciiToKeyCode['/'] = { KEYCODE_SLASH, 0 };
		_AsciiToKeyCode['`'] = { KEYCODE_GRAVE, 0 };
		_AsciiToKeyCode['['] = { KEYCODE_LEFT_BRACKET, 0 };
		_AsciiToKeyCode['\\'] = { KEYCODE_BACK_SLASH, 0 };
		_AsciiToKeyCode[']'] = { KEYCODE_RIGHT_BRACKET, 0 };
		_AsciiToKeyCode['\''] = { KEYCODE_APOSTROPHE, 0 };
		_AsciiToKeyCode[':'] = { KEYCODE_SEMICOLON, 1 };
		_AsciiToKeyCode['+'] = { KEYCODE_EQUALS, 1 };
		_AsciiToKeyCode['_'] = { KEYCODE_MINUS, 1 };
		_AsciiToKeyCode['<'] = { KEYCODE_COMMA, 1 };
		_AsciiToKeyCode['>'] = { KEYCODE_PERIOD, 1 };
		_AsciiToKeyCode['?'] = { KEYCODE_SLASH, 1 };
		_AsciiToKeyCode['~'] = { KEYCODE_GRAVE, 1 };
		_AsciiToKeyCode['{'] = { KEYCODE_LEFT_BRACKET, 1 };
		_AsciiToKeyCode['|'] = { KEYCODE_BACK_SLASH, 1 };
		_AsciiToKeyCode['}'] = { KEYCODE_RIGHT_BRACKET, 1 };
		_AsciiToKeyCode['"'] = { KEYCODE_APOSTROPHE, 1 };
		_AsciiToKeyCode[127] = { KEYCODE_DELETE, 0 };
	}

	void KeyboardAdapter::dispatch(
		int code, bool isAscii, bool isDown, bool isCapsLock,
		int repeat, int device, int source
	) {
		UILock lock(_host->window());
		_repeat = repeat;
		_device = device;
		_source = source;
		_caps_lock = isCapsLock;
		onDispatch(code, isAscii, isDown);
	}

	int KeyboardAdapter::toKeypress(KeyboardKeyCode code) {
		// Letters
		if ( code >= 65 && code <= 90 ) {
			if ( _caps_lock || _shift ) { // A - Z
				return code; // caps | shift
			} else {
				return code + 32; // lowercase a - z
			}
		}
		KeyCodeToKeypress keypress;
		if (_KeyCodeToKeypress.get(code, keypress)) {
			return _shift ? keypress.shift: keypress.normal;
		}
		return 0;
	}

	void KeyboardAdapter::onDispatch(int code, bool isAscii, bool isDown) {
		if ( isAscii ) {
			AsciiToKeyCode keycode;
			if (_AsciiToKeyCode.get(code, keycode)) {
				_shift = keycode.shift;
				_keycode = keycode.code;
				_keypress = toKeypress(_keycode);
			} else {
				_keycode = KEYCODE_UNKNOWN;
				_keypress = code;
			}
		} else {
			if (_PlatformKeyCodeToKeyCode.get(code, _keycode)) {
				switch ( _keycode ) {
					case KEYCODE_SHIFT: _shift = isDown; break;
					case KEYCODE_CTRL: _ctrl = isDown; break;
					case KEYCODE_ALT: _alt = isDown; break;
					case KEYCODE_COMMAND_RIGHT:
					case KEYCODE_COMMAND: _command = isDown; break;
					default: break;
				}
				_keypress = toKeypress(_keycode);
			} else { // Unknown keycode
				_keycode = KEYCODE_UNKNOWN;
				_keypress = 0;
			}
		}
		//Qk_DEBUG("keycode: %d, %d, isDown: %i", code, _keycode, isDown);

		if ( isDown ) {
			_host->onKeyboardDown();
		} else {
			_host->onKeyboardUp();
		}
		if ( isAscii ) {
			_shift = false;
		}
	}

}
