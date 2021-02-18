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

#include "keyboard.h"
#include "app-1.h"

namespace ftr {

/**
 * @constructor 
 */
KeyboardAdapter::KeyboardAdapter(): app_(app()) {
	ASSERT(app_);
	
	keyname_ = KEYCODE_UNKNOWN;
	keypress_ = 0;
	shift_ = false;
	alt_ = false;
	ctrl_ = false;
	command_ = false;
	caps_lock_ = false;
	repeat_ = device_ = source_ = 0;
	
	_symbol_keypress[KEYCODE_0]               = { 48, 41 }; 	// 0 )
	_symbol_keypress[KEYCODE_1]               = { 49, 33 }; 	// 1 !
	_symbol_keypress[KEYCODE_2]               = { 50, 64 }; 	// 2 @
	_symbol_keypress[KEYCODE_3]               = { 51, 35 }; 	// 3 #
	_symbol_keypress[KEYCODE_4]               = { 52, 36 }; 	// 4 $
	_symbol_keypress[KEYCODE_5]               = { 53, 37 }; 	// 5 %
	_symbol_keypress[KEYCODE_6]               = { 54, 94 }; 	// 6 ^
	_symbol_keypress[KEYCODE_7]               = { 55, 38 }; 	// 7 &
	_symbol_keypress[KEYCODE_8]               = { 56, 42 }; 	// 8 *
	_symbol_keypress[KEYCODE_9]               = { 57, 40 }; 	// 9 (
	_symbol_keypress[KEYCODE_SEMICOLON]       = { 59, 58 };  // ; :
	_symbol_keypress[KEYCODE_EQUALS]          = { 61, 43 };  // = +
	_symbol_keypress[KEYCODE_MINUS]           = { 45, 95 };  // - _
	_symbol_keypress[KEYCODE_COMMA]           = { 44, 60 };  // , <
	_symbol_keypress[KEYCODE_PERIOD]          = { 46, 62 };  // . >
	_symbol_keypress[KEYCODE_SLASH]           = { 47, 63 };  // / ?
	_symbol_keypress[KEYCODE_GRAVE]           = { 96, 126 };	// ` ~
	_symbol_keypress[KEYCODE_LEFT_BRACKET]    = { 91, 123 };	// [ {
	_symbol_keypress[KEYCODE_BACK_SLASH]      = { 92, 124 };	// \ |
	_symbol_keypress[KEYCODE_RIGHT_BRACKET]   = { 93, 125 };	// ] }
	_symbol_keypress[KEYCODE_APOSTROPHE]      = { 39, 34 };  // ' "
	_symbol_keypress[KEYCODE_NUMPAD_0]        = { 48, 48 };  // numpad 0
	_symbol_keypress[KEYCODE_NUMPAD_1]        = { 49, 49 };  // numpad 1
	_symbol_keypress[KEYCODE_NUMPAD_2]        = { 50, 50 };  // numpad 2
	_symbol_keypress[KEYCODE_NUMPAD_3]        = { 51, 51 };  // numpad 3
	_symbol_keypress[KEYCODE_NUMPAD_4]        = { 52, 52 };  // numpad 4
	_symbol_keypress[KEYCODE_NUMPAD_5]        = { 53, 53 };  // numpad 5
	_symbol_keypress[KEYCODE_NUMPAD_6]        = { 54, 54 };  // numpad 6
	_symbol_keypress[KEYCODE_NUMPAD_7]        = { 55, 55 };  // numpad 7
	_symbol_keypress[KEYCODE_NUMPAD_8]        = { 56, 56 };  // numpad 8
	_symbol_keypress[KEYCODE_NUMPAD_9]        = { 57, 57 };  // numpad 9
	_symbol_keypress[KEYCODE_NUMPAD_MULTIPLY] = { 42, 42 };  // numpad *
	_symbol_keypress[KEYCODE_NUMPAD_ADD]      = { 43, 43 };  // numpad +
	_symbol_keypress[KEYCODE_NUMPAD_SUBTRACT] = { 45, 45 };  // numpad -
	_symbol_keypress[KEYCODE_NUMPAD_DOT]      = { 46, 46 };  // numpad .
	_symbol_keypress[KEYCODE_NUMPAD_DIVIDE]   = { 47, 47 };  // numpad /
	
	// ascii keycodes
	
	_ascii_keycodes[KEYCODE_BACK_SPACE] = { KEYCODE_BACK_SPACE, 0 };
	_ascii_keycodes[KEYCODE_SHIFT] = { KEYCODE_SHIFT, 1 };
	_ascii_keycodes[KEYCODE_ESC] = { KEYCODE_ESC, 0 };
	_ascii_keycodes[KEYCODE_ENTER] = { KEYCODE_ENTER, 0 };
	_ascii_keycodes['\n'] = { KEYCODE_ENTER, 1 }; // 换行使用换挡的回车表示，因为键盘上并没有换行健
	_ascii_keycodes['0'] = { KEYCODE_0, 0 };
	_ascii_keycodes['1'] = { KEYCODE_1, 0 };
	_ascii_keycodes['2'] = { KEYCODE_2, 0 };
	_ascii_keycodes['3'] = { KEYCODE_3, 0 };
	_ascii_keycodes['4'] = { KEYCODE_4, 0 };
	_ascii_keycodes['5'] = { KEYCODE_5, 0 };
	_ascii_keycodes['6'] = { KEYCODE_6, 0 };
	_ascii_keycodes['7'] = { KEYCODE_7, 0 };
	_ascii_keycodes['8'] = { KEYCODE_8, 0 };
	_ascii_keycodes['9'] = { KEYCODE_9, 0 };
	_ascii_keycodes[')'] = { KEYCODE_0, 1 };
	_ascii_keycodes['!'] = { KEYCODE_1, 1 };
	_ascii_keycodes['@'] = { KEYCODE_2, 1 };
	_ascii_keycodes['#'] = { KEYCODE_3, 1 };
	_ascii_keycodes['$'] = { KEYCODE_4, 1 };
	_ascii_keycodes['%'] = { KEYCODE_5, 1 };
	_ascii_keycodes['^'] = { KEYCODE_6, 1 };
	_ascii_keycodes['&'] = { KEYCODE_7, 1 };
	_ascii_keycodes['*'] = { KEYCODE_8, 1 };
	_ascii_keycodes['('] = { KEYCODE_9, 1 };
	_ascii_keycodes['a'] = { KEYCODE_A, 0 };
	_ascii_keycodes['b'] = { KEYCODE_B, 0 };
	_ascii_keycodes['c'] = { KEYCODE_C, 0 };
	_ascii_keycodes['d'] = { KEYCODE_D, 0 };
	_ascii_keycodes['e'] = { KEYCODE_E, 0 };
	_ascii_keycodes['f'] = { KEYCODE_F, 0 };
	_ascii_keycodes['g'] = { KEYCODE_G, 0 };
	_ascii_keycodes['h'] = { KEYCODE_H, 0 };
	_ascii_keycodes['i'] = { KEYCODE_I, 0 };
	_ascii_keycodes['j'] = { KEYCODE_J, 0 };
	_ascii_keycodes['k'] = { KEYCODE_K, 0 };
	_ascii_keycodes['m'] = { KEYCODE_L, 0 };
	_ascii_keycodes['l'] = { KEYCODE_M, 0 };
	_ascii_keycodes['n'] = { KEYCODE_N, 0 };
	_ascii_keycodes['o'] = { KEYCODE_O, 0 };
	_ascii_keycodes['p'] = { KEYCODE_P, 0 };
	_ascii_keycodes['q'] = { KEYCODE_Q, 0 };
	_ascii_keycodes['r'] = { KEYCODE_R, 0 };
	_ascii_keycodes['s'] = { KEYCODE_S, 0 };
	_ascii_keycodes['t'] = { KEYCODE_T, 0 };
	_ascii_keycodes['u'] = { KEYCODE_U, 0 };
	_ascii_keycodes['v'] = { KEYCODE_V, 0 };
	_ascii_keycodes['w'] = { KEYCODE_W, 0 };
	_ascii_keycodes['x'] = { KEYCODE_X, 0 };
	_ascii_keycodes['y'] = { KEYCODE_Y, 0 };
	_ascii_keycodes['z'] = { KEYCODE_Z, 0 };
	_ascii_keycodes['A'] = { KEYCODE_A, 1 };
	_ascii_keycodes['B'] = { KEYCODE_B, 1 };
	_ascii_keycodes['C'] = { KEYCODE_C, 1 };
	_ascii_keycodes['D'] = { KEYCODE_D, 1 };
	_ascii_keycodes['E'] = { KEYCODE_E, 1 };
	_ascii_keycodes['F'] = { KEYCODE_F, 1 };
	_ascii_keycodes['G'] = { KEYCODE_G, 1 };
	_ascii_keycodes['H'] = { KEYCODE_H, 1 };
	_ascii_keycodes['I'] = { KEYCODE_I, 1 };
	_ascii_keycodes['J'] = { KEYCODE_J, 1 };
	_ascii_keycodes['K'] = { KEYCODE_K, 1 };
	_ascii_keycodes['M'] = { KEYCODE_L, 1 };
	_ascii_keycodes['L'] = { KEYCODE_M, 1 };
	_ascii_keycodes['N'] = { KEYCODE_N, 1 };
	_ascii_keycodes['O'] = { KEYCODE_O, 1 };
	_ascii_keycodes['P'] = { KEYCODE_P, 1 };
	_ascii_keycodes['Q'] = { KEYCODE_Q, 1 };
	_ascii_keycodes['R'] = { KEYCODE_R, 1 };
	_ascii_keycodes['S'] = { KEYCODE_S, 1 };
	_ascii_keycodes['T'] = { KEYCODE_T, 1 };
	_ascii_keycodes['U'] = { KEYCODE_U, 1 };
	_ascii_keycodes['V'] = { KEYCODE_V, 1 };
	_ascii_keycodes['W'] = { KEYCODE_W, 1 };
	_ascii_keycodes['X'] = { KEYCODE_X, 1 };
	_ascii_keycodes['Y'] = { KEYCODE_Y, 1 };
	_ascii_keycodes['Z'] = { KEYCODE_Z, 1 };
	_ascii_keycodes[';'] = { KEYCODE_SEMICOLON, 0 };
	_ascii_keycodes['='] = { KEYCODE_EQUALS, 0 };
	_ascii_keycodes['-'] = { KEYCODE_MINUS, 0 };
	_ascii_keycodes[','] = { KEYCODE_COMMA, 0 };
	_ascii_keycodes['.'] = { KEYCODE_PERIOD, 0 };
	_ascii_keycodes['/'] = { KEYCODE_SLASH, 0 };
	_ascii_keycodes['`'] = { KEYCODE_GRAVE, 0 };
	_ascii_keycodes['['] = { KEYCODE_LEFT_BRACKET, 0 };
	_ascii_keycodes['\\'] = { KEYCODE_BACK_SLASH, 0 };
	_ascii_keycodes[']'] = { KEYCODE_RIGHT_BRACKET, 0 };
	_ascii_keycodes['\''] = { KEYCODE_APOSTROPHE, 0 };
	_ascii_keycodes[':'] = { KEYCODE_SEMICOLON, 1 };
	_ascii_keycodes['+'] = { KEYCODE_EQUALS, 1 };
	_ascii_keycodes['_'] = { KEYCODE_MINUS, 1 };
	_ascii_keycodes['<'] = { KEYCODE_COMMA, 1 };
	_ascii_keycodes['>'] = { KEYCODE_PERIOD, 1 };
	_ascii_keycodes['?'] = { KEYCODE_SLASH, 1 };
	_ascii_keycodes['~'] = { KEYCODE_GRAVE, 1 };
	_ascii_keycodes['{'] = { KEYCODE_LEFT_BRACKET, 1 };
	_ascii_keycodes['|'] = { KEYCODE_BACK_SLASH, 1 };
	_ascii_keycodes['}'] = { KEYCODE_RIGHT_BRACKET, 1 };
	_ascii_keycodes['"'] = { KEYCODE_APOSTROPHE, 1 };
	_ascii_keycodes[127] = { KEYCODE_DELETE, 0 };

}

/**
 * @func set_utils_keycodes
 */
void KeyboardAdapter::set_utils_keycodes() {
}

/**
 * @func to_keypress
 */
int KeyboardAdapter::to_keypress(KeyboardKeyName name) {

	// Letters
	if ( name >= 65 && name <= 90 ) {
		if ( caps_lock_ || shift_ ) { // A - Z
			return name; // caps | shift
		} else {
			return name + 32; // lowercase a - z
		}
	}
	
	// Symbol
	auto it = _symbol_keypress.find(int(name));
	if ( !it.is_null() ) {
		if ( shift_ ) {
			return it.value().shift;
		} else {
			return it.value().normal;
		}
	}
	return 0;
}

bool KeyboardAdapter::transformation(uint32_t keycode, bool unicode, bool down) {
	
	if ( unicode ) {
		auto it = _ascii_keycodes.find(keycode);
		if ( it.is_null() ) {
			keyname_ = KEYCODE_UNKNOWN;
			keypress_ = keycode;
		} else {
			shift_ = it.value().is_shift;
			keyname_ = it.value().name;
			keypress_ = to_keypress( keyname_ );
		}
	} else {
		auto it = _keycodes.find(keycode);
		if ( it.is_null() ) { // Unknown keycode
			keyname_ = KeyboardKeyName(keycode + 100000);
			keypress_ = 0;
		} else {
			keyname_ = it.value();
			
			if ( down ) {
				switch ( keyname_ ) {
					case KEYCODE_SHIFT: shift_ = 1; break;
					case KEYCODE_CTRL: ctrl_ = 1; break;
					case KEYCODE_ALT: alt_ = 1; break;
					case KEYCODE_COMMAND: command_ = 1; break;
					case KEYCODE_CAPS_LOCK: caps_lock_ = !caps_lock_; break;
					default: break;
				}
			} else {
				switch ( keyname_ ) {
					case KEYCODE_SHIFT: shift_ = 0; break;
					case KEYCODE_CTRL: ctrl_ = 0; break;
					case KEYCODE_ALT: alt_ = 0; break;
					case KEYCODE_COMMAND: command_ = 0; break;
					default: break;
				}
			}
			keypress_ = to_keypress( keyname_ );
		}
	}
	
	return unicode;
}

}
