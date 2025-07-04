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

/**
 * @enum KeyboardKeyCode
*/
export enum KeyboardKeyCode {
	UNKNOWN            = 0, //!<
	MOUSE_LEFT         = 1, //!<
	MOUSE_CENTER       = 2, //!<
	MOUSE_RIGHT        = 3, //!<
	MOUSE_WHEEL        = 4, //!<
	BACK_SPACE         = 8,  //!< back space
	TAB                = 9,  //!< tab
	CLEAR              = 12, //!< clear
	ENTER              = 13, //!< enter
	SHIFT              = 16, //!< shift
	CTRL               = 17, //!< ctrl
	ALT                = 18, //!< alt
	BREAK              = 19, //!< break
	CAPS_LOCK          = 20, //!< caps lock
	ESC                = 27, //!< esc
	SPACE              = 32, //!< space
	PAGE_UP            = 33, //!< page up
	PAGE_DOWN          = 34, //!< page down
	MOVE_END           = 35, //!< end
	MOVE_HOME          = 36, //!< home
	LEFT               = 37, //!< left
	UP                 = 38, //!< up
	RIGHT              = 39, //!< right
	DOWN               = 40, //!< down
	INSERT             = 45, //!< insert
	DELETE             = 46, //!< delete
	SYSRQ              = 42, //!< sysrq
	HELP               = 47, //!< help
	NUM_0              = 48, //!< 0 )
	NUM_1              = 49, //!< 1 !
	NUM_2              = 50, //!< 2 @
	NUM_3              = 51, //!< 3 #
	NUM_4              = 52, //!< 4 $
	NUM_5              = 53, //!< 5 %
	NUM_6              = 54, //!< 6 ^
	NUM_7              = 55, //!< 7 &
	NUM_8              = 56, //!< 8 *
	NUM_9              = 57, //!< 9 (
	A                  = 65, //!< a-z
	B                  = 66, //!<
	C                  = 67, //!<
	D                  = 68, //!<
	E                  = 69, //!<
	F                  = 70, //!<
	G                  = 71, //!<
	H                  = 72, //!<
	I                  = 73, //!<
	J                  = 74, //!<
	K                  = 75, //!<
	L                  = 76, //!<
	M                  = 77, //!<
	N                  = 78, //!<
	O                  = 79, //!<
	P                  = 80, //!<
	Q                  = 81, //!<
	R                  = 82, //!<
	S                  = 83, //!<
	T                  = 84, //!<
	U                  = 85, //!<
	V                  = 86, //!<
	W                  = 87, //!<
	X                  = 88, //!<
	Y                  = 89, //!<
	Z                  = 90, //!<
	COMMAND            = 91, //!< command
	MENU               = 92, //!< menu
	COMMAND_RIGHT      = 93, //!< command right
	NUMPAD_EQUALS      = 94, //!< numpad =
	NUMPAD_0           = 96, //!< numpad 0-9
	NUMPAD_1           = 97, //!<
	NUMPAD_2           = 98, //!<
	NUMPAD_3           = 99, //!<
	NUMPAD_4           = 100, //!<
	NUMPAD_5           = 101, //!<
	NUMPAD_6           = 102, //!<
	NUMPAD_7           = 103, //!<
	NUMPAD_8           = 104, //!<
	NUMPAD_9           = 105, //!<
	NUMPAD_MULTIPLY    = 106, //!< *
	NUMPAD_ADD         = 107, //!< +
	NUMPAD_ENTER       = 108, //!< Numpad Enter
	NUMPAD_SUBTRACT    = 109, //!< -
	NUMPAD_DOT         = 110, //!< .
	NUMPAD_DIVIDE      = 111, //!< /
	F1                 = 112, //!< f1 - f24
	F2                 = 113, //!<
	F3                 = 114, //!<
	F4                 = 115, //!<
	F5                 = 116, //!<
	F6                 = 117, //!<
	F7                 = 118, //!<
	F8                 = 119, //!<
	F9                 = 120, //!<
	F10                = 121, //!<
	F11                = 122, //!<
	F12                = 123, //!<
	F13                = 124, //!<
	F14                = 125, //!<
	F15                = 126, //!<
	F16                = 127, //!<
	F17                = 128, //!<
	F18                = 129, //!<
	F19                = 130, //!<
	F20                = 131, //!<
	F21                = 132, //!<
	F22                = 133, //!<
	F23                = 134, //!<
	F24                = 135, //!<
	NUM_LOCK           = 144, //!< num lock
	SCROLL_LOCK        = 145, //!< SCROLL_LOCK
	SEMICOLON          = 186, //!< ; :
	EQUALS             = 187, //!< = +
	MINUS              = 189, //!< - _
	COMMA              = 188, //!< , <
	PERIOD             = 190, //!< . >
	SLASH              = 191, //!< / ?
	GRAVE              = 192, //!< ` ~
	LEFT_BRACKET       = 219, //!< [ {
	BACK_SLASH         = 220, //!< \ |
	RIGHT_BRACKET      = 221, //!< ] }
	APOSTROPHE         = 222, //!< ' "
	/* --------------------------------------------------- */
	FUN                = 209, //!< Function
	HOME               = 300, //!< 按键Home
	BACK               = 301, //!< 返回键
	CALL               = 302, //!< 拨号键
	ENDCALL            = 303, //!< 挂机键
	STAR               = 304, //!< *
	POUND              = 305, //!< #
	CENTER             = 306, //!< 导航键 确定键
	VOLUME_UP          = 307, //!< 音量增加键
	VOLUME_DOWN        = 308, //!< 音量减小键
	POWER              = 309, //!< 电源键
	CAMERA             = 310, //!< 拍照键
	FOCUS              = 311, //!< 拍照对焦键
	MENU_1             = 312, //!< 菜单键
	SEARCH             = 313, //!< 搜索键
	MEDIA_PLAY_PAUSE   = 314, //!< 多媒体键 播放/暂停
	MEDIA_STOP         = 315, //!< 多媒体键 停止
	MEDIA_NEXT         = 316, //!< 多媒体键 下一首
	MEDIA_PREVIOUS     = 317, //!< 多媒体键 上一首
	MEDIA_REWIND       = 318, //!< 多媒体键 快退
	MEDIA_FAST_FORWARD = 319, //!< 多媒体键 快进
	MUTE               = 320, //!< 话筒静音键
	CHANNEL_UP         = 321, //!< 按键Channel up
	CHANNEL_DOWN       = 322, //!< 按键Channel down
	MEDIA_PLAY         = 323, //!< 多媒体键 播放
	MEDIA_PAUSE        = 324, //!< 多媒体键 暂停
	MEDIA_CLOSE        = 325, //!< 多媒体键 关闭
	MEDIA_EJECT        = 326, //!< 多媒体键 弹出
	MEDIA_RECORD       = 327, //!< 多媒体键 录音
	VOLUME_MUTE        = 328, //!< 扬声器静音键
	MUSIC              = 329, //!< music
	EXPLORER           = 330, //!< 按键Explorer special function
	ENVELOPE           = 331, //!< 按键Envelope special function
	BOOKMARK           = 332, //!< 按键Bookmark
	ZOOM_IN            = 333, //!< 放大键
	ZOOM_OUT           = 334, //!< 缩小键
}