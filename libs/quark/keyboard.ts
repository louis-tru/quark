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

/**
 * @enum KeyboardCode
 *
 * Platform-independent keyboard, mouse, media, and device key codes.
*/
export enum KeyboardCode {
	NONE =             0,   //!< {0} No key.
	BACK_SPACE =       8,   //!< {8} Backspace key.
	TAB =              9,   //!< {9} Tab key.
	CLEAR =            12,  //!< {12} Clear key.
	ENTER =            13,  //!< {13} Enter or Return key.
	SHIFT =            16,  //!< {16} Shift or left Shift key.
	CTRL =             17,  //!< {17} Control or left Control key.
	ALT =              18,  //!< {18} Alt or left Alt key.
	BREAK =            19,  //!< {19} Break or Pause key.
	CAPS_LOCK =        20,  //!< {20} Caps Lock key.
	ESC =              27,  //!< {27} Escape key.
	SPACE =            32,  //!< {32} Space key.
	PAGE_UP =          33,  //!< {33} Page Up key.
	PAGE_DOWN =        34,  //!< {34} Page Down key.
	MOVE_END =         35,  //!< {35} End key.
	MOVE_HOME =        36,  //!< {36} Home navigation key.
	LEFT =             37,  //!< {37} Left Arrow key.
	UP =               38,  //!< {38} Up Arrow key.
	RIGHT =            39,  //!< {39} Right Arrow key.
	DOWN =             40,  //!< {40} Down Arrow key.
	SYSRQ =            42,  //!< {42} System Request or Print Screen key.
	INSERT =           45,  //!< {45} Insert key.
	DELETE =           46,  //!< {46} Delete key.
	HELP =             47,  //!< {47} Help key.
	NUM_0 =            48,  //!< {48} Main keyboard 0 key.
	NUM_1 =            49,  //!< {49} Main keyboard 1 key.
	NUM_2 =            50,  //!< {50} Main keyboard 2 key.
	NUM_3 =            51,  //!< {51} Main keyboard 3 key.
	NUM_4 =            52,  //!< {52} Main keyboard 4 key.
	NUM_5 =            53,  //!< {53} Main keyboard 5 key.
	NUM_6 =            54,  //!< {54} Main keyboard 6 key.
	NUM_7 =            55,  //!< {55} Main keyboard 7 key.
	NUM_8 =            56,  //!< {56} Main keyboard 8 key.
	NUM_9 =            57,  //!< {57} Main keyboard 9 key.
	A =                65,  //!< {65} A key.
	B =                66,  //!< {66} B key.
	C =                67,  //!< {67} C key.
	D =                68,  //!< {68} D key.
	E =                69,  //!< {69} E key.
	F =                70,  //!< {70} F key.
	G =                71,  //!< {71} G key.
	H =                72,  //!< {72} H key.
	I =                73,  //!< {73} I key.
	J =                74,  //!< {74} J key.
	K =                75,  //!< {75} K key.
	L =                76,  //!< {76} L key.
	M =                77,  //!< {77} M key.
	N =                78,  //!< {78} N key.
	O =                79,  //!< {79} O key.
	P =                80,  //!< {80} P key.
	Q =                81,  //!< {81} Q key.
	R =                82,  //!< {82} R key.
	S =                83,  //!< {83} S key.
	T =                84,  //!< {84} T key.
	U =                85,  //!< {85} U key.
	V =                86,  //!< {86} V key.
	W =                87,  //!< {87} W key.
	X =                88,  //!< {88} X key.
	Y =                89,  //!< {89} Y key.
	Z =                90,  //!< {90} Z key.
	COMMAND =          91,  //!< {91} Command or left Command key.
	MENU =             92,  //!< {92} Menu key.
	COMMAND_RIGHT =    93,  //!< {93} Right Command key.
	NUMPAD_EQUALS =    94,  //!< {94} Numeric keypad Equals key.
	NUMPAD_0 =         96,  //!< {96} Numeric keypad 0 key.
	NUMPAD_1 =         97,  //!< {97} Numeric keypad 1 key.
	NUMPAD_2 =         98,  //!< {98} Numeric keypad 2 key.
	NUMPAD_3 =         99,  //!< {99} Numeric keypad 3 key.
	NUMPAD_4 =         100, //!< {100} Numeric keypad 4 key.
	NUMPAD_5 =         101, //!< {101} Numeric keypad 5 key.
	NUMPAD_6 =         102, //!< {102} Numeric keypad 6 key.
	NUMPAD_7 =         103, //!< {103} Numeric keypad 7 key.
	NUMPAD_8 =         104, //!< {104} Numeric keypad 8 key.
	NUMPAD_9 =         105, //!< {105} Numeric keypad 9 key.
	NUMPAD_MULTIPLY =  106, //!< {106} Numeric keypad Multiply key.
	NUMPAD_ADD =       107, //!< {107} Numeric keypad Add key.
	NUMPAD_ENTER =     108, //!< {108} Numeric keypad Enter key.
	NUMPAD_SUBTRACT =  109, //!< {109} Numeric keypad Subtract key.
	NUMPAD_DOT =       110, //!< {110} Numeric keypad Decimal key.
	NUMPAD_DIVIDE =    111, //!< {111} Numeric keypad Divide key.
	F1 =               112, //!< {112} F1 function key.
	F2 =               113, //!< {113} F2 function key.
	F3 =               114, //!< {114} F3 function key.
	F4 =               115, //!< {115} F4 function key.
	F5 =               116, //!< {116} F5 function key.
	F6 =               117, //!< {117} F6 function key.
	F7 =               118, //!< {118} F7 function key.
	F8 =               119, //!< {119} F8 function key.
	F9 =               120, //!< {120} F9 function key.
	F10 =              121, //!< {121} F10 function key.
	F11 =              122, //!< {122} F11 function key.
	F12 =              123, //!< {123} F12 function key.
	F13 =              124, //!< {124} F13 function key.
	F14 =              125, //!< {125} F14 function key.
	F15 =              126, //!< {126} F15 function key.
	F16 =              127, //!< {127} F16 function key.
	F17 =              128, //!< {128} F17 function key.
	F18 =              129, //!< {129} F18 function key.
	F19 =              130, //!< {130} F19 function key.
	F20 =              131, //!< {131} F20 function key.
	F21 =              132, //!< {132} F21 function key.
	F22 =              133, //!< {133} F22 function key.
	F23 =              134, //!< {134} F23 function key.
	F24 =              135, //!< {135} F24 function key.
	NUM_LOCK =         144, //!< {144} Num Lock key.
	SCROLL_LOCK =      145, //!< {145} Scroll Lock key.
	SHIFT_RIGHT =      160, //!< {160} Right Shift key.
	CTRL_RIGHT =       161, //!< {161} Right Control key.
	ALT_RIGHT =        162, //!< {162} Right Alt key.
	SEMICOLON =        186, //!< {186} Semicolon key.
	EQUALS =           187, //!< {187} Equals key.
	MINUS =            189, //!< {189} Minus key.
	COMMA =            188, //!< {188} Comma key.
	PERIOD =           190, //!< {190} Period key.
	SLASH =            191, //!< {191} Slash key.
	GRAVE =            192, //!< {192} Grave Accent key.
	FUN =              209, //!< {209} Function key; support depends on the platform.
	LEFT_BRACKET =     219, //!< {219} Left Bracket key.
	BACK_SLASH =       220, //!< {220} Backslash key.
	RIGHT_BRACKET =    221, //!< {221} Right Bracket key.
	APOSTROPHE =       222, //!< {222} Apostrophe key.
	MOUSE_LEFT =       256, //!< {256} Left mouse button.
	MOUSE_CENTER =     257, //!< {257} Middle mouse button.
	MOUSE_RIGHT =      258, //!< {258} Right mouse button.
	MOUSE_WHEEL_UP =   259, //!< {259} Mouse wheel up.
	MOUSE_WHEEL_DOWN = 260, //!< {260} Mouse wheel down.
	MOUSE_WHEEL_LEFT = 261, //!< {261} Mouse wheel left.
	MOUSE_WHEEL_RIGHT= 262, //!< {262} Mouse wheel right.
	HOME =             300, //!< {300} Device Home key.
	BACK =             301, //!< {301} Device Back key.
	CALL =             302, //!< {302} Call key.
	ENDCALL =          303, //!< {303} End Call key.
	STAR =             304, //!< {304} Star key.
	POUND =            305, //!< {305} Pound key.
	CENTER =           306, //!< {306} Center navigation or Confirm key.
	VOLUME_UP =        307, //!< {307} Volume Up key.
	VOLUME_DOWN =      308, //!< {308} Volume Down key.
	POWER =            309, //!< {309} Power key.
	CAMERA =           310, //!< {310} Camera shutter key.
	FOCUS =            311, //!< {311} Camera focus key.
	MENU_1 =           312, //!< {312} Device Menu key.
	SEARCH =           313, //!< {313} Search key.
	MEDIA_PLAY_PAUSE = 314, //!< {314} Media Play/Pause key.
	MEDIA_STOP =       315, //!< {315} Media Stop key.
	MEDIA_NEXT =       316, //!< {316} Media Next key.
	MEDIA_PREVIOUS =   317, //!< {317} Media Previous key.
	MEDIA_REWIND =     318, //!< {318} Media Rewind key.
	MEDIA_FAST_FORWARD =319, //!< {319} Media Fast Forward key.
	MUTE =             320, //!< {320} Microphone Mute key.
	CHANNEL_UP =       321, //!< {321} Channel Up key.
	CHANNEL_DOWN =     322, //!< {322} Channel Down key.
	MEDIA_PLAY =       323, //!< {323} Media Play key.
	MEDIA_PAUSE =      324, //!< {324} Media Pause key.
	MEDIA_CLOSE =      325, //!< {325} Media Close key.
	MEDIA_EJECT =      326, //!< {326} Media Eject key.
	MEDIA_RECORD =     327, //!< {327} Media Record key.
	VOLUME_MUTE =      328, //!< {328} Speaker Mute key.
	MUSIC =            329, //!< {329} Music key.
	EXPLORER =         330, //!< {330} Explorer key.
	ENVELOPE =         331, //!< {331} Mail key.
	BOOKMARK =         332, //!< {332} Bookmark key.
	ZOOM_IN =          333, //!< {333} Zoom In key.
	ZOOM_OUT =         334, //!< {334} Zoom Out key.
}
