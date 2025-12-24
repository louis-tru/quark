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
#include <android/keycodes.h>

namespace qk {

	class AndroidKeyboardAdapter: public KeyboardAdapter {
	public:
		AndroidKeyboardAdapter();
	};

	AndroidKeyboardAdapter::AndroidKeyboardAdapter() {
		_PlatformKeyCodeToKeyCode[AKEYCODE_UNKNOWN] = KEYCODE_UNKNOWN;
		_PlatformKeyCodeToKeyCode[AKEYCODE_DEL] = KEYCODE_BACK_SPACE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_TAB] = KEYCODE_TAB;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CLEAR] = KEYCODE_CLEAR;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ENTER] = KEYCODE_ENTER;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SHIFT_LEFT] = KEYCODE_SHIFT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SHIFT_RIGHT] = KEYCODE_SHIFT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CTRL_LEFT] = KEYCODE_CTRL;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CTRL_RIGHT] = KEYCODE_CTRL;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ALT_LEFT] = KEYCODE_ALT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ALT_RIGHT] = KEYCODE_ALT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_BREAK] = KEYCODE_BREAK;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CAPS_LOCK] = KEYCODE_CAPS_LOCK;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ESCAPE] = KEYCODE_ESC;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SPACE] = KEYCODE_SPACE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_PAGE_UP] = KEYCODE_PAGE_UP;
		_PlatformKeyCodeToKeyCode[AKEYCODE_PAGE_DOWN] = KEYCODE_PAGE_DOWN;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MOVE_END] = KEYCODE_MOVE_END;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MOVE_HOME] = KEYCODE_MOVE_HOME;
		_PlatformKeyCodeToKeyCode[AKEYCODE_DPAD_LEFT] = KEYCODE_LEFT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_DPAD_UP] = KEYCODE_UP;
		_PlatformKeyCodeToKeyCode[AKEYCODE_DPAD_RIGHT] = KEYCODE_RIGHT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_DPAD_DOWN] = KEYCODE_DOWN;
		_PlatformKeyCodeToKeyCode[AKEYCODE_INSERT] = KEYCODE_INSERT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_FORWARD_DEL] = KEYCODE_DELETE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SYSRQ] = KEYCODE_SYSRQ;
		_PlatformKeyCodeToKeyCode[AKEYCODE_HELP] = KEYCODE_HELP;
		_PlatformKeyCodeToKeyCode[AKEYCODE_0] = KEYCODE_0;
		_PlatformKeyCodeToKeyCode[AKEYCODE_1] = KEYCODE_1;
		_PlatformKeyCodeToKeyCode[AKEYCODE_2] = KEYCODE_2;
		_PlatformKeyCodeToKeyCode[AKEYCODE_3] = KEYCODE_3;
		_PlatformKeyCodeToKeyCode[AKEYCODE_4] = KEYCODE_4;
		_PlatformKeyCodeToKeyCode[AKEYCODE_5] = KEYCODE_5;
		_PlatformKeyCodeToKeyCode[AKEYCODE_6] = KEYCODE_6;
		_PlatformKeyCodeToKeyCode[AKEYCODE_7] = KEYCODE_7;
		_PlatformKeyCodeToKeyCode[AKEYCODE_8] = KEYCODE_8;
		_PlatformKeyCodeToKeyCode[AKEYCODE_9] = KEYCODE_9;
		_PlatformKeyCodeToKeyCode[AKEYCODE_A] = KEYCODE_A;
		_PlatformKeyCodeToKeyCode[AKEYCODE_B] = KEYCODE_B;
		_PlatformKeyCodeToKeyCode[AKEYCODE_C] = KEYCODE_C;
		_PlatformKeyCodeToKeyCode[AKEYCODE_D] = KEYCODE_D;
		_PlatformKeyCodeToKeyCode[AKEYCODE_E] = KEYCODE_E;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F] = KEYCODE_F;
		_PlatformKeyCodeToKeyCode[AKEYCODE_G] = KEYCODE_G;
		_PlatformKeyCodeToKeyCode[AKEYCODE_H] = KEYCODE_H;
		_PlatformKeyCodeToKeyCode[AKEYCODE_I] = KEYCODE_I;
		_PlatformKeyCodeToKeyCode[AKEYCODE_J] = KEYCODE_J;
		_PlatformKeyCodeToKeyCode[AKEYCODE_K] = KEYCODE_K;
		_PlatformKeyCodeToKeyCode[AKEYCODE_L] = KEYCODE_L;
		_PlatformKeyCodeToKeyCode[AKEYCODE_M] = KEYCODE_M;
		_PlatformKeyCodeToKeyCode[AKEYCODE_N] = KEYCODE_N;
		_PlatformKeyCodeToKeyCode[AKEYCODE_O] = KEYCODE_O;
		_PlatformKeyCodeToKeyCode[AKEYCODE_P] = KEYCODE_P;
		_PlatformKeyCodeToKeyCode[AKEYCODE_Q] = KEYCODE_Q;
		_PlatformKeyCodeToKeyCode[AKEYCODE_R] = KEYCODE_R;
		_PlatformKeyCodeToKeyCode[AKEYCODE_S] = KEYCODE_S;
		_PlatformKeyCodeToKeyCode[AKEYCODE_T] = KEYCODE_T;
		_PlatformKeyCodeToKeyCode[AKEYCODE_U] = KEYCODE_U;
		_PlatformKeyCodeToKeyCode[AKEYCODE_V] = KEYCODE_V;
		_PlatformKeyCodeToKeyCode[AKEYCODE_W] = KEYCODE_W;
		_PlatformKeyCodeToKeyCode[AKEYCODE_X] = KEYCODE_X;
		_PlatformKeyCodeToKeyCode[AKEYCODE_Y] = KEYCODE_Y;
		_PlatformKeyCodeToKeyCode[AKEYCODE_Z] = KEYCODE_Z;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_COMMAND] = KEYCODE_COMMAND; // Pending query
		_PlatformKeyCodeToKeyCode[AKEYCODE_MENU] = KEYCODE_MENU;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_COMMAND] = KEYCODE_COMMAND_RIGHT; // Pending query
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_EQUALS] = KEYCODE_NUMPAD_EQUALS;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_0] = KEYCODE_NUMPAD_0;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_1] = KEYCODE_NUMPAD_1;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_2] = KEYCODE_NUMPAD_2;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_3] = KEYCODE_NUMPAD_3;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_4] = KEYCODE_NUMPAD_4;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_5] = KEYCODE_NUMPAD_5;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_6] = KEYCODE_NUMPAD_6;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_7] = KEYCODE_NUMPAD_7;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_8] = KEYCODE_NUMPAD_8;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_9] = KEYCODE_NUMPAD_9;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_MULTIPLY] = KEYCODE_NUMPAD_MULTIPLY;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_ADD] = KEYCODE_NUMPAD_ADD;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_ENTER] = KEYCODE_NUMPAD_ENTER;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_SUBTRACT] = KEYCODE_NUMPAD_SUBTRACT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_DOT] = KEYCODE_NUMPAD_DOT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUMPAD_DIVIDE] = KEYCODE_NUMPAD_DIVIDE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F1] = KEYCODE_F1;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F2] = KEYCODE_F2;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F3] = KEYCODE_F3;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F4] = KEYCODE_F4;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F5] = KEYCODE_F5;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F6] = KEYCODE_F6;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F7] = KEYCODE_F7;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F8] = KEYCODE_F8;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F9] = KEYCODE_F9;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F10] = KEYCODE_F10;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F11] = KEYCODE_F11;
		_PlatformKeyCodeToKeyCode[AKEYCODE_F12] = KEYCODE_F12;
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
		_PlatformKeyCodeToKeyCode[AKEYCODE_NUM_LOCK] = KEYCODE_NUM_LOCK;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SCROLL_LOCK] = KEYCODE_SCROLL_LOCK;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SEMICOLON] = KEYCODE_SEMICOLON;
		_PlatformKeyCodeToKeyCode[AKEYCODE_EQUALS] = KEYCODE_EQUALS;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MINUS] = KEYCODE_MINUS;
		_PlatformKeyCodeToKeyCode[AKEYCODE_COMMA] = KEYCODE_COMMA;
		_PlatformKeyCodeToKeyCode[AKEYCODE_PERIOD] = KEYCODE_PERIOD;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SLASH] = KEYCODE_SLASH;
		_PlatformKeyCodeToKeyCode[AKEYCODE_GRAVE] = KEYCODE_GRAVE;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_FUN] = KEYCODE_FUN;
		_PlatformKeyCodeToKeyCode[AKEYCODE_LEFT_BRACKET] = KEYCODE_LEFT_BRACKET;
		_PlatformKeyCodeToKeyCode[AKEYCODE_BACKSLASH] = KEYCODE_BACK_SLASH;
		_PlatformKeyCodeToKeyCode[AKEYCODE_RIGHT_BRACKET] = KEYCODE_RIGHT_BRACKET;
		_PlatformKeyCodeToKeyCode[AKEYCODE_APOSTROPHE] = KEYCODE_APOSTROPHE;
		/* --------------------------------------------------- */
		_PlatformKeyCodeToKeyCode[AKEYCODE_HOME] = KEYCODE_HOME;
		_PlatformKeyCodeToKeyCode[AKEYCODE_BACK] = KEYCODE_BACK;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CALL] = KEYCODE_CALL;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ENDCALL] = KEYCODE_ENDCALL;
		_PlatformKeyCodeToKeyCode[AKEYCODE_STAR] = KEYCODE_STAR;
		_PlatformKeyCodeToKeyCode[AKEYCODE_POUND] = KEYCODE_POUND;
		_PlatformKeyCodeToKeyCode[AKEYCODE_DPAD_CENTER] = KEYCODE_CENTER;
		_PlatformKeyCodeToKeyCode[AKEYCODE_VOLUME_UP] = KEYCODE_VOLUME_UP;
		_PlatformKeyCodeToKeyCode[AKEYCODE_VOLUME_DOWN] = KEYCODE_VOLUME_DOWN;
		_PlatformKeyCodeToKeyCode[AKEYCODE_POWER] = KEYCODE_POWER;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CAMERA] = KEYCODE_CAMERA;
		_PlatformKeyCodeToKeyCode[AKEYCODE_FOCUS] = KEYCODE_FOCUS;
		// _PlatformKeyCodeToKeyCode[AKEYCODE_MENU] = KEYCODE_MENU_1;
		_PlatformKeyCodeToKeyCode[AKEYCODE_SEARCH] = KEYCODE_SEARCH;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_PLAY_PAUSE] = KEYCODE_MEDIA_PLAY_PAUSE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_STOP] = KEYCODE_MEDIA_STOP;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_NEXT] = KEYCODE_MEDIA_NEXT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_PREVIOUS] = KEYCODE_MEDIA_PREVIOUS;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_REWIND] = KEYCODE_MEDIA_REWIND;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_FAST_FORWARD] = KEYCODE_MEDIA_FAST_FORWARD;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MUTE] = KEYCODE_MUTE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CHANNEL_UP] = KEYCODE_CHANNEL_UP;
		_PlatformKeyCodeToKeyCode[AKEYCODE_CHANNEL_DOWN] = KEYCODE_CHANNEL_DOWN;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_PLAY] = KEYCODE_MEDIA_PLAY;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_PAUSE] = KEYCODE_MEDIA_PAUSE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_CLOSE] = KEYCODE_MEDIA_CLOSE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_EJECT] = KEYCODE_MEDIA_EJECT;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MEDIA_RECORD] = KEYCODE_MEDIA_RECORD;
		_PlatformKeyCodeToKeyCode[AKEYCODE_VOLUME_MUTE] = KEYCODE_VOLUME_MUTE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_MUSIC] = KEYCODE_MUSIC;
		_PlatformKeyCodeToKeyCode[AKEYCODE_EXPLORER] = KEYCODE_EXPLORER;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ENVELOPE] = KEYCODE_ENVELOPE;
		_PlatformKeyCodeToKeyCode[AKEYCODE_BOOKMARK] = KEYCODE_BOOKMARK;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ZOOM_IN] = KEYCODE_ZOOM_IN;
		_PlatformKeyCodeToKeyCode[AKEYCODE_ZOOM_OUT] = KEYCODE_ZOOM_OUT;
	}

	KeyboardAdapter* KeyboardAdapter::create() {
		return new AndroidKeyboardAdapter();
	}
}