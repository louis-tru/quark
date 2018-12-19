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
#include "ngui/base/map.h"

XX_NS(ngui)

/**
 * @class AppleKeyboardAdapter
 */
class AppleKeyboardAdapter: public KeyboardAdapter {
 public:
	AppleKeyboardAdapter();
};

AppleKeyboardAdapter::AppleKeyboardAdapter() {
	// m_keycodes[AKEYCODE_DEL] = KEYCODE_BACK_SPACE;
	// m_keycodes[AKEYCODE_TAB] = KEYCODE_TAB;
	// m_keycodes[AKEYCODE_CLEAR] = KEYCODE_CLEAR;
	// m_keycodes[AKEYCODE_ENTER] = KEYCODE_ENTER;
	// m_keycodes[AKEYCODE_SHIFT_LEFT] = KEYCODE_SHIFT;
	// m_keycodes[AKEYCODE_SHIFT_RIGHT] = KEYCODE_SHIFT;
	// m_keycodes[AKEYCODE_CTRL_LEFT] = KEYCODE_CTRL;
	// m_keycodes[AKEYCODE_CTRL_RIGHT] = KEYCODE_CTRL;
	// m_keycodes[AKEYCODE_ALT_LEFT] = KEYCODE_ALT;
	// m_keycodes[AKEYCODE_ALT_RIGHT] = KEYCODE_ALT;
	// m_keycodes[AKEYCODE_CAPS_LOCK] = KEYCODE_CAPS_LOCK;
	// m_keycodes[AKEYCODE_ESCAPE] = KEYCODE_ESC;
	// m_keycodes[AKEYCODE_SPACE] = KEYCODE_SPACE;
	// // m_keycodes[AKEYCODE_COMMAND] = KEYCODE_COMMAND; // Pending query
	// m_keycodes[AKEYCODE_DPAD_LEFT] = KEYCODE_LEFT;
	// m_keycodes[AKEYCODE_DPAD_UP] = KEYCODE_UP;
	// m_keycodes[AKEYCODE_DPAD_RIGHT] = KEYCODE_RIGHT;
	// m_keycodes[AKEYCODE_DPAD_DOWN] = KEYCODE_DOWN;
	// m_keycodes[AKEYCODE_INSERT] = KEYCODE_INSERT;
	// m_keycodes[AKEYCODE_FORWARD_DEL] = KEYCODE_DELETE;
	// m_keycodes[AKEYCODE_PAGE_UP] = KEYCODE_PAGE_UP;
	// m_keycodes[AKEYCODE_PAGE_DOWN] = KEYCODE_PAGE_DOWN;
	// m_keycodes[AKEYCODE_MOVE_END] = KEYCODE_MOVE_END;
	// m_keycodes[AKEYCODE_MOVE_HOME] = KEYCODE_MOVE_HOME;
	// m_keycodes[AKEYCODE_SCROLL_LOCK] = KEYCODE_SCROLL_LOCK;
	// m_keycodes[AKEYCODE_BREAK] = KEYCODE_BREAK;
	// m_keycodes[AKEYCODE_SYSRQ] = KEYCODE_SYSRQ;
	// m_keycodes[AKEYCODE_0] = KEYCODE_0;
	// m_keycodes[AKEYCODE_1] = KEYCODE_1;
	// m_keycodes[AKEYCODE_2] = KEYCODE_2;
	// m_keycodes[AKEYCODE_3] = KEYCODE_3;
	// m_keycodes[AKEYCODE_4] = KEYCODE_4;
	// m_keycodes[AKEYCODE_5] = KEYCODE_5;
	// m_keycodes[AKEYCODE_6] = KEYCODE_6;
	// m_keycodes[AKEYCODE_7] = KEYCODE_7;
	// m_keycodes[AKEYCODE_8] = KEYCODE_8;
	// m_keycodes[AKEYCODE_9] = KEYCODE_9;
	// m_keycodes[AKEYCODE_A] = KEYCODE_A;
	// m_keycodes[AKEYCODE_B] = KEYCODE_B;
	// m_keycodes[AKEYCODE_C] = KEYCODE_C;
	// m_keycodes[AKEYCODE_D] = KEYCODE_D;
	// m_keycodes[AKEYCODE_E] = KEYCODE_E;
	// m_keycodes[AKEYCODE_F] = KEYCODE_F;
	// m_keycodes[AKEYCODE_G] = KEYCODE_G;
	// m_keycodes[AKEYCODE_H] = KEYCODE_H;
	// m_keycodes[AKEYCODE_I] = KEYCODE_I;
	// m_keycodes[AKEYCODE_J] = KEYCODE_J;
	// m_keycodes[AKEYCODE_K] = KEYCODE_K;
	// m_keycodes[AKEYCODE_L] = KEYCODE_L;
	// m_keycodes[AKEYCODE_M] = KEYCODE_M;
	// m_keycodes[AKEYCODE_N] = KEYCODE_N;
	// m_keycodes[AKEYCODE_O] = KEYCODE_O;
	// m_keycodes[AKEYCODE_P] = KEYCODE_P;
	// m_keycodes[AKEYCODE_Q] = KEYCODE_Q;
	// m_keycodes[AKEYCODE_R] = KEYCODE_R;
	// m_keycodes[AKEYCODE_S] = KEYCODE_S;
	// m_keycodes[AKEYCODE_T] = KEYCODE_T;
	// m_keycodes[AKEYCODE_U] = KEYCODE_U;
	// m_keycodes[AKEYCODE_V] = KEYCODE_V;
	// m_keycodes[AKEYCODE_W] = KEYCODE_W;
	// m_keycodes[AKEYCODE_X] = KEYCODE_X;
	// m_keycodes[AKEYCODE_Y] = KEYCODE_Y;
	// m_keycodes[AKEYCODE_Z] = KEYCODE_Z;
	// m_keycodes[AKEYCODE_NUM_LOCK] = KEYCODE_NUM_LOCK;
	// m_keycodes[AKEYCODE_NUMPAD_0] = KEYCODE_NUMPAD_0;
	// m_keycodes[AKEYCODE_NUMPAD_1] = KEYCODE_NUMPAD_1;
	// m_keycodes[AKEYCODE_NUMPAD_2] = KEYCODE_NUMPAD_2;
	// m_keycodes[AKEYCODE_NUMPAD_3] = KEYCODE_NUMPAD_3;
	// m_keycodes[AKEYCODE_NUMPAD_4] = KEYCODE_NUMPAD_4;
	// m_keycodes[AKEYCODE_NUMPAD_5] = KEYCODE_NUMPAD_5;
	// m_keycodes[AKEYCODE_NUMPAD_6] = KEYCODE_NUMPAD_6;
	// m_keycodes[AKEYCODE_NUMPAD_7] = KEYCODE_NUMPAD_7;
	// m_keycodes[AKEYCODE_NUMPAD_8] = KEYCODE_NUMPAD_8;
	// m_keycodes[AKEYCODE_NUMPAD_9] = KEYCODE_NUMPAD_9;
	// m_keycodes[AKEYCODE_NUMPAD_DIVIDE] = KEYCODE_NUMPAD_DIVIDE;
	// m_keycodes[AKEYCODE_NUMPAD_MULTIPLY] = KEYCODE_NUMPAD_MULTIPLY;
	// m_keycodes[AKEYCODE_NUMPAD_SUBTRACT] = KEYCODE_NUMPAD_SUBTRACT;
	// m_keycodes[AKEYCODE_NUMPAD_ADD] = KEYCODE_NUMPAD_ADD;
	// m_keycodes[AKEYCODE_NUMPAD_DOT] = KEYCODE_NUMPAD_DOT;
	// m_keycodes[AKEYCODE_NUMPAD_ENTER] = KEYCODE_NUMPAD_ENTER;
	// m_keycodes[AKEYCODE_F1] = KEYCODE_F1;
	// m_keycodes[AKEYCODE_F2] = KEYCODE_F2;
	// m_keycodes[AKEYCODE_F3] = KEYCODE_F3;
	// m_keycodes[AKEYCODE_F4] = KEYCODE_F4;
	// m_keycodes[AKEYCODE_F5] = KEYCODE_F5;
	// m_keycodes[AKEYCODE_F6] = KEYCODE_F6;
	// m_keycodes[AKEYCODE_F7] = KEYCODE_F7;
	// m_keycodes[AKEYCODE_F8] = KEYCODE_F8;
	// m_keycodes[AKEYCODE_F9] = KEYCODE_F9;
	// m_keycodes[AKEYCODE_F10] = KEYCODE_F10;
	// m_keycodes[AKEYCODE_F11] = KEYCODE_F11;
	// m_keycodes[AKEYCODE_F12] = KEYCODE_F12;
	// m_keycodes[AKEYCODE_SEMICOLON] = KEYCODE_SEMICOLON;
	// m_keycodes[AKEYCODE_EQUALS] = KEYCODE_EQUALS;
	// m_keycodes[AKEYCODE_MINUS] = KEYCODE_MINUS;
	// m_keycodes[AKEYCODE_COMMA] = KEYCODE_COMMA;
	// m_keycodes[AKEYCODE_PERIOD] = KEYCODE_PERIOD;
	// m_keycodes[AKEYCODE_SLASH] = KEYCODE_SLASH;
	// m_keycodes[AKEYCODE_GRAVE] = KEYCODE_GRAVE;
	// m_keycodes[AKEYCODE_LEFT_BRACKET] = KEYCODE_LEFT_BRACKET;
	// m_keycodes[AKEYCODE_BACKSLASH] = KEYCODE_BACK_SLASH;
	// m_keycodes[AKEYCODE_RIGHT_BRACKET] = KEYCODE_RIGHT_BRACKET;
	// m_keycodes[AKEYCODE_APOSTROPHE] = KEYCODE_APOSTROPHE;
	// m_keycodes[AKEYCODE_HOME] = KEYCODE_HOME;
	// m_keycodes[AKEYCODE_BACK] = KEYCODE_BACK;
	// m_keycodes[AKEYCODE_CALL] = KEYCODE_CALL;
	// m_keycodes[AKEYCODE_ENDCALL] = KEYCODE_ENDCALL;
	// m_keycodes[AKEYCODE_STAR] = KEYCODE_STAR;
	// m_keycodes[AKEYCODE_POUND] = KEYCODE_POUND;
	// m_keycodes[AKEYCODE_DPAD_CENTER] = KEYCODE_CENTER;
	// m_keycodes[AKEYCODE_VOLUME_UP] = KEYCODE_VOLUME_UP;
	// m_keycodes[AKEYCODE_VOLUME_DOWN] = KEYCODE_VOLUME_DOWN;
	// m_keycodes[AKEYCODE_POWER] = KEYCODE_POWER;
	// m_keycodes[AKEYCODE_CAMERA] = KEYCODE_CAMERA;
	// m_keycodes[AKEYCODE_FOCUS] = KEYCODE_FOCUS;
	// m_keycodes[AKEYCODE_MENU] = KEYCODE_MENU;
	// m_keycodes[AKEYCODE_SEARCH] = KEYCODE_SEARCH;
	// m_keycodes[AKEYCODE_MEDIA_PLAY_PAUSE] = KEYCODE_MEDIA_PLAY_PAUSE;
	// m_keycodes[AKEYCODE_MEDIA_STOP] = KEYCODE_MEDIA_STOP;
	// m_keycodes[AKEYCODE_MEDIA_NEXT] = KEYCODE_MEDIA_NEXT;
	// m_keycodes[AKEYCODE_MEDIA_PREVIOUS] = KEYCODE_MEDIA_PREVIOUS;
	// m_keycodes[AKEYCODE_MEDIA_REWIND] = KEYCODE_MEDIA_REWIND;
	// m_keycodes[AKEYCODE_MEDIA_FAST_FORWARD] = KEYCODE_MEDIA_FAST_FORWARD;
	// m_keycodes[AKEYCODE_MUTE] = KEYCODE_MUTE;
	// m_keycodes[AKEYCODE_CHANNEL_UP] = KEYCODE_CHANNEL_UP;
	// m_keycodes[AKEYCODE_CHANNEL_DOWN] = KEYCODE_CHANNEL_DOWN;
	// m_keycodes[AKEYCODE_MEDIA_PLAY] = KEYCODE_MEDIA_PLAY;
	// m_keycodes[AKEYCODE_MEDIA_PAUSE] = KEYCODE_MEDIA_PAUSE;
	// m_keycodes[AKEYCODE_MEDIA_CLOSE] = KEYCODE_MEDIA_CLOSE;
	// m_keycodes[AKEYCODE_MEDIA_EJECT] = KEYCODE_MEDIA_EJECT;
	// m_keycodes[AKEYCODE_MEDIA_RECORD] = KEYCODE_MEDIA_RECORD;
	// m_keycodes[AKEYCODE_VOLUME_MUTE] = KEYCODE_VOLUME_MUTE;
	// m_keycodes[AKEYCODE_MUSIC] = KEYCODE_MUSIC;
	// m_keycodes[AKEYCODE_EXPLORER] = KEYCODE_EXPLORER;
	// m_keycodes[AKEYCODE_ENVELOPE] = KEYCODE_ENVELOPE;
	// m_keycodes[AKEYCODE_BOOKMARK] = KEYCODE_BOOKMARK;
	// m_keycodes[AKEYCODE_ZOOM_IN] = KEYCODE_ZOOM_IN;
	// m_keycodes[AKEYCODE_ZOOM_OUT] = KEYCODE_ZOOM_OUT;
	// m_keycodes[AKEYCODE_HELP] = KEYCODE_HELP;
}

KeyboardAdapter* KeyboardAdapter::create() {
	return new AppleKeyboardAdapter();
}

XX_END
