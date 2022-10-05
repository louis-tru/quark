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

#import "../../keyboard.h"

namespace quark {

	/**
	* @class AppleKeyboardAdapter
	*/
	class AppleKeyboardAdapter: public KeyboardAdapter {
	public:
		AppleKeyboardAdapter();
	};

	AppleKeyboardAdapter::AppleKeyboardAdapter() {
		// _keycodes[AKEYCODE_DEL] = KEYCODE_BACK_SPACE;
		// _keycodes[AKEYCODE_TAB] = KEYCODE_TAB;
		// _keycodes[AKEYCODE_CLEAR] = KEYCODE_CLEAR;
		// _keycodes[AKEYCODE_ENTER] = KEYCODE_ENTER;
		// _keycodes[AKEYCODE_SHIFT_LEFT] = KEYCODE_SHIFT;
		// _keycodes[AKEYCODE_SHIFT_RIGHT] = KEYCODE_SHIFT;
		// _keycodes[AKEYCODE_CTRL_LEFT] = KEYCODE_CTRL;
		// _keycodes[AKEYCODE_CTRL_RIGHT] = KEYCODE_CTRL;
		// _keycodes[AKEYCODE_ALT_LEFT] = KEYCODE_ALT;
		// _keycodes[AKEYCODE_ALT_RIGHT] = KEYCODE_ALT;
		// _keycodes[AKEYCODE_CAPS_LOCK] = KEYCODE_CAPS_LOCK;
		// _keycodes[AKEYCODE_ESCAPE] = KEYCODE_ESC;
		// _keycodes[AKEYCODE_SPACE] = KEYCODE_SPACE;
		// // _keycodes[AKEYCODE_COMMAND] = KEYCODE_COMMAND; // Pending query
		// _keycodes[AKEYCODE_DPAD_LEFT] = KEYCODE_LEFT;
		// _keycodes[AKEYCODE_DPAD_UP] = KEYCODE_UP;
		// _keycodes[AKEYCODE_DPAD_RIGHT] = KEYCODE_RIGHT;
		// _keycodes[AKEYCODE_DPAD_DOWN] = KEYCODE_DOWN;
		// _keycodes[AKEYCODE_INSERT] = KEYCODE_INSERT;
		// _keycodes[AKEYCODE_FORWARD_DEL] = KEYCODE_DELETE;
		// _keycodes[AKEYCODE_PAGE_UP] = KEYCODE_PAGE_UP;
		// _keycodes[AKEYCODE_PAGE_DOWN] = KEYCODE_PAGE_DOWN;
		// _keycodes[AKEYCODE_MOVE_END] = KEYCODE_MOVE_END;
		// _keycodes[AKEYCODE_MOVE_HOME] = KEYCODE_MOVE_HOME;
		// _keycodes[AKEYCODE_SCROLL_LOCK] = KEYCODE_SCROLL_LOCK;
		// _keycodes[AKEYCODE_BREAK] = KEYCODE_BREAK;
		// _keycodes[AKEYCODE_SYSRQ] = KEYCODE_SYSRQ;
		// _keycodes[AKEYCODE_0] = KEYCODE_0;
		// _keycodes[AKEYCODE_1] = KEYCODE_1;
		// _keycodes[AKEYCODE_2] = KEYCODE_2;
		// _keycodes[AKEYCODE_3] = KEYCODE_3;
		// _keycodes[AKEYCODE_4] = KEYCODE_4;
		// _keycodes[AKEYCODE_5] = KEYCODE_5;
		// _keycodes[AKEYCODE_6] = KEYCODE_6;
		// _keycodes[AKEYCODE_7] = KEYCODE_7;
		// _keycodes[AKEYCODE_8] = KEYCODE_8;
		// _keycodes[AKEYCODE_9] = KEYCODE_9;
		// _keycodes[AKEYCODE_A] = KEYCODE_A;
		// _keycodes[AKEYCODE_B] = KEYCODE_B;
		// _keycodes[AKEYCODE_C] = KEYCODE_C;
		// _keycodes[AKEYCODE_D] = KEYCODE_D;
		// _keycodes[AKEYCODE_E] = KEYCODE_E;
		// _keycodes[AKEYCODE_F] = KEYCODE_F;
		// _keycodes[AKEYCODE_G] = KEYCODE_G;
		// _keycodes[AKEYCODE_H] = KEYCODE_H;
		// _keycodes[AKEYCODE_I] = KEYCODE_I;
		// _keycodes[AKEYCODE_J] = KEYCODE_J;
		// _keycodes[AKEYCODE_K] = KEYCODE_K;
		// _keycodes[AKEYCODE_L] = KEYCODE_L;
		// _keycodes[AKEYCODE_M] = KEYCODE_M;
		// _keycodes[AKEYCODE_N] = KEYCODE_N;
		// _keycodes[AKEYCODE_O] = KEYCODE_O;
		// _keycodes[AKEYCODE_P] = KEYCODE_P;
		// _keycodes[AKEYCODE_Q] = KEYCODE_Q;
		// _keycodes[AKEYCODE_R] = KEYCODE_R;
		// _keycodes[AKEYCODE_S] = KEYCODE_S;
		// _keycodes[AKEYCODE_T] = KEYCODE_T;
		// _keycodes[AKEYCODE_U] = KEYCODE_U;
		// _keycodes[AKEYCODE_V] = KEYCODE_V;
		// _keycodes[AKEYCODE_W] = KEYCODE_W;
		// _keycodes[AKEYCODE_X] = KEYCODE_X;
		// _keycodes[AKEYCODE_Y] = KEYCODE_Y;
		// _keycodes[AKEYCODE_Z] = KEYCODE_Z;
		// _keycodes[AKEYCODE_NUM_LOCK] = KEYCODE_NUM_LOCK;
		// _keycodes[AKEYCODE_NUMPAD_0] = KEYCODE_NUMPAD_0;
		// _keycodes[AKEYCODE_NUMPAD_1] = KEYCODE_NUMPAD_1;
		// _keycodes[AKEYCODE_NUMPAD_2] = KEYCODE_NUMPAD_2;
		// _keycodes[AKEYCODE_NUMPAD_3] = KEYCODE_NUMPAD_3;
		// _keycodes[AKEYCODE_NUMPAD_4] = KEYCODE_NUMPAD_4;
		// _keycodes[AKEYCODE_NUMPAD_5] = KEYCODE_NUMPAD_5;
		// _keycodes[AKEYCODE_NUMPAD_6] = KEYCODE_NUMPAD_6;
		// _keycodes[AKEYCODE_NUMPAD_7] = KEYCODE_NUMPAD_7;
		// _keycodes[AKEYCODE_NUMPAD_8] = KEYCODE_NUMPAD_8;
		// _keycodes[AKEYCODE_NUMPAD_9] = KEYCODE_NUMPAD_9;
		// _keycodes[AKEYCODE_NUMPAD_DIVIDE] = KEYCODE_NUMPAD_DIVIDE;
		// _keycodes[AKEYCODE_NUMPAD_MULTIPLY] = KEYCODE_NUMPAD_MULTIPLY;
		// _keycodes[AKEYCODE_NUMPAD_SUBTRACT] = KEYCODE_NUMPAD_SUBTRACT;
		// _keycodes[AKEYCODE_NUMPAD_ADD] = KEYCODE_NUMPAD_ADD;
		// _keycodes[AKEYCODE_NUMPAD_DOT] = KEYCODE_NUMPAD_DOT;
		// _keycodes[AKEYCODE_NUMPAD_ENTER] = KEYCODE_NUMPAD_ENTER;
		// _keycodes[AKEYCODE_F1] = KEYCODE_F1;
		// _keycodes[AKEYCODE_F2] = KEYCODE_F2;
		// _keycodes[AKEYCODE_F3] = KEYCODE_F3;
		// _keycodes[AKEYCODE_F4] = KEYCODE_F4;
		// _keycodes[AKEYCODE_F5] = KEYCODE_F5;
		// _keycodes[AKEYCODE_F6] = KEYCODE_F6;
		// _keycodes[AKEYCODE_F7] = KEYCODE_F7;
		// _keycodes[AKEYCODE_F8] = KEYCODE_F8;
		// _keycodes[AKEYCODE_F9] = KEYCODE_F9;
		// _keycodes[AKEYCODE_F10] = KEYCODE_F10;
		// _keycodes[AKEYCODE_F11] = KEYCODE_F11;
		// _keycodes[AKEYCODE_F12] = KEYCODE_F12;
		// _keycodes[AKEYCODE_SEMICOLON] = KEYCODE_SEMICOLON;
		// _keycodes[AKEYCODE_EQUALS] = KEYCODE_EQUALS;
		// _keycodes[AKEYCODE_MINUS] = KEYCODE_MINUS;
		// _keycodes[AKEYCODE_COMMA] = KEYCODE_COMMA;
		// _keycodes[AKEYCODE_PERIOD] = KEYCODE_PERIOD;
		// _keycodes[AKEYCODE_SLASH] = KEYCODE_SLASH;
		// _keycodes[AKEYCODE_GRAVE] = KEYCODE_GRAVE;
		// _keycodes[AKEYCODE_LEFT_BRACKET] = KEYCODE_LEFT_BRACKET;
		// _keycodes[AKEYCODE_BACKSLASH] = KEYCODE_BACK_SLASH;
		// _keycodes[AKEYCODE_RIGHT_BRACKET] = KEYCODE_RIGHT_BRACKET;
		// _keycodes[AKEYCODE_APOSTROPHE] = KEYCODE_APOSTROPHE;
		// _keycodes[AKEYCODE_HOME] = KEYCODE_HOME;
		// _keycodes[AKEYCODE_BACK] = KEYCODE_BACK;
		// _keycodes[AKEYCODE_CALL] = KEYCODE_CALL;
		// _keycodes[AKEYCODE_ENDCALL] = KEYCODE_ENDCALL;
		// _keycodes[AKEYCODE_STAR] = KEYCODE_STAR;
		// _keycodes[AKEYCODE_POUND] = KEYCODE_POUND;
		// _keycodes[AKEYCODE_DPAD_CENTER] = KEYCODE_CENTER;
		// _keycodes[AKEYCODE_VOLUME_UP] = KEYCODE_VOLUME_UP;
		// _keycodes[AKEYCODE_VOLUME_DOWN] = KEYCODE_VOLUME_DOWN;
		// _keycodes[AKEYCODE_POWER] = KEYCODE_POWER;
		// _keycodes[AKEYCODE_CAMERA] = KEYCODE_CAMERA;
		// _keycodes[AKEYCODE_FOCUS] = KEYCODE_FOCUS;
		// _keycodes[AKEYCODE_MENU] = KEYCODE_MENU;
		// _keycodes[AKEYCODE_SEARCH] = KEYCODE_SEARCH;
		// _keycodes[AKEYCODE_MEDIA_PLAY_PAUSE] = KEYCODE_MEDIA_PLAY_PAUSE;
		// _keycodes[AKEYCODE_MEDIA_STOP] = KEYCODE_MEDIA_STOP;
		// _keycodes[AKEYCODE_MEDIA_NEXT] = KEYCODE_MEDIA_NEXT;
		// _keycodes[AKEYCODE_MEDIA_PREVIOUS] = KEYCODE_MEDIA_PREVIOUS;
		// _keycodes[AKEYCODE_MEDIA_REWIND] = KEYCODE_MEDIA_REWIND;
		// _keycodes[AKEYCODE_MEDIA_FAST_FORWARD] = KEYCODE_MEDIA_FAST_FORWARD;
		// _keycodes[AKEYCODE_MUTE] = KEYCODE_MUTE;
		// _keycodes[AKEYCODE_CHANNEL_UP] = KEYCODE_CHANNEL_UP;
		// _keycodes[AKEYCODE_CHANNEL_DOWN] = KEYCODE_CHANNEL_DOWN;
		// _keycodes[AKEYCODE_MEDIA_PLAY] = KEYCODE_MEDIA_PLAY;
		// _keycodes[AKEYCODE_MEDIA_PAUSE] = KEYCODE_MEDIA_PAUSE;
		// _keycodes[AKEYCODE_MEDIA_CLOSE] = KEYCODE_MEDIA_CLOSE;
		// _keycodes[AKEYCODE_MEDIA_EJECT] = KEYCODE_MEDIA_EJECT;
		// _keycodes[AKEYCODE_MEDIA_RECORD] = KEYCODE_MEDIA_RECORD;
		// _keycodes[AKEYCODE_VOLUME_MUTE] = KEYCODE_VOLUME_MUTE;
		// _keycodes[AKEYCODE_MUSIC] = KEYCODE_MUSIC;
		// _keycodes[AKEYCODE_EXPLORER] = KEYCODE_EXPLORER;
		// _keycodes[AKEYCODE_ENVELOPE] = KEYCODE_ENVELOPE;
		// _keycodes[AKEYCODE_BOOKMARK] = KEYCODE_BOOKMARK;
		// _keycodes[AKEYCODE_ZOOM_IN] = KEYCODE_ZOOM_IN;
		// _keycodes[AKEYCODE_ZOOM_OUT] = KEYCODE_ZOOM_OUT;
		// _keycodes[AKEYCODE_HELP] = KEYCODE_HELP;
	}

	KeyboardAdapter* KeyboardAdapter::create() {
		return new AppleKeyboardAdapter();
	}

}