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

#ifndef __flare__value__
#define __flare__value__

#include "./util/util.h"
#include "./math/math.h"

namespace flare {

	namespace value {

		#define FX_Enum_Value(F) \
			\
			F(NONE,               none) \
			F(DEFAULT,            default) \
			F(INHERIT,            inherit) \
			F(AUTO,               auto) \
			F(NORMAL,             normal) \
			\
			F(WRAP,               wrap) \
			F(MATCH,              match) \
			F(PIXEL,              pixel) \
			F(RATIO,              ratio) \
			F(MINUS,              minus) \
			\
			F(ROW,                row) \
			F(ROW_REVERSE,        row_reverse) \
			F(COLUMN,             column) \
			F(COLUMN_REVERSE,     column_reverse) \
			\
			F(LEFT,               left) \
			F(RIGHT,              right) \
			F(TOP,                top) \
			F(BOTTOM,             bottom) \
			\
			F(LEFT_TOP,           left_top) \
			F(CENTER_TOP,         center_top) \
			F(RIGHT_TOP,          right_top) \
			F(LEFT_CENTER,        left_center) \
			F(CENTER_CENTER,      center_center) \
			F(RIGHT_CENTER,       right_center) \
			F(LEFT_BOTTOM,        left_bottom) \
			F(CENTER_BOTTOM,      center_bottom) \
			F(RIGHT_BOTTOM,       right_bottom) \
			\
			F(LEFT_REVERSE,       left_reverse) \
			F(CENTER_REVERSE,     center_reverse) \
			F(RIGHT_REVERSE,      right_reverse) \
			\
			F(START,              start) \
			F(CENTER,             center) \
			F(END,                end) \
			F(BASELINE,           baseline) \
			F(STRETCH,            stretch) \
			F(SPACE_BETWEEN,      space_between) \
			F(SPACE_AROUND,       space_around) \
			F(SPACE_EVENLY,       space_evenly) \
			\
			F(MIDDLE,             middle) \
			F(REPEAT,             repeat) \
			F(REPEAT_X,           repeat_x) \
			F(REPEAT_Y,           repeat_y) \
			F(MIRRORED_REPEAT,    mirrored_repeat) \
			F(MIRRORED_REPEAT_X,  mirrored_repeat_x) \
			F(MIRRORED_REPEAT_Y,  mirrored_repeat_y) \
			\
			F(CLIP,               clip) \
			F(ELLIPSIS,           ellipsis) \
			F(CENTER_ELLIPSIS,    center_ellipsis) \
			\
			/* text white space */ \
			/*F(NORMAL,           normal)*/ \
			F(NO_WRAP,            no_wrap) \
			F(NO_SPACE,           no_space) \
			F(PRE,                pre) \
			F(PRE_LINE,           pre_line) \
			/*F(WRAP,               wrap)*/ \
			F(WRAP_REVERSE,       wrap_reverse) \
			\
			/* text weight */ \
			F(THIN,               thin)              /*100*/ \
			F(ULTRALIGHT,         ultralight)        /*200*/ \
			F(LIGHT,              light)             /*300*/ \
			F(REGULAR,            regular)           /*400*/ \
			F(MEDIUM,             medium)            /*500*/ \
			F(SEMIBOLD,           semibold)          /*600*/ \
			F(BOLD,               bold)              /*700*/ \
			F(HEAVY,              heavy)             /*800*/ \
			F(BLACK,              black)             /*900*/ \
			\
			/* text style */ \
			/*F(NORMAL,       normal)*/ \
			F(ITALIC,         italic) \
			F(OBLIQUE,        oblique) \
			\
			/* text decoration */ \
			F(OVERLINE,           overline) \
			F(LINE_THROUGH,       line_through) \
			F(UNDERLINE,          underline) \
			\
			/* soft keyboard style */\
			F(ASCII,              ascii) \
			F(NUMBER,             number) \
			F(URL,                url) \
			F(NUMBER_PAD,         number_pad) \
			F(PHONE,              phone) \
			F(NAME_PHONE,         name_phone) \
			F(EMAIL,              email) \
			F(DECIMAL,            decimal) \
			F(TWITTER,            twitter) \
			F(SEARCH,             search) \
			F(ASCII_NUMBER,       ascii_numner) \
			\
			/* enter key style in soft keyboard */ \
			F(GO,                 go) \
			F(JOIN,               join) \
			F(NEXT,               next) \
			F(ROUTE,              route) \
			F(SEND,               send) \
			F(DONE,               done) \
			F(EMERGENCY,          emergency) \
			F(CONTINUE,           continue) \

		enum Enum {
			# define DEF_Enum_Value(NAME, NAME2) NAME,
				FX_Enum_Value(DEF_Enum_Value)
			# undef DEF_Enum_Value
		};

		struct Shadow {
			float offset_x, offset_y, size;
			Color color;
			inline bool operator==(const Shadow& val) const {
				return (
					val.offset_x == offset_x && val.offset_y == offset_y && 
					val.size == size && val.color == color);
			}
			inline bool operator!=(const Shadow& val) const {
				return ! operator==(val);
			}
		};

		template<typename Type, Type TypeInit, typename Value = float>
		struct TemplateValue {
			Value value;
			Type type;
			inline bool operator==(const TemplateValue& val) const {
				return val.type == type && val.value == value;
			}
			inline bool operator!=(const TemplateValue& val) const {
				return !operator==(val);
			}
			inline TemplateValue(Value v, Type t = TypeInit): value(v), type(t) {}
		};
		
	}
}

#endif
