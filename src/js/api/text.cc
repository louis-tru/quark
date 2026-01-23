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

#include "./ui.h"
#include "../../ui/view/text.h"
#include "../../ui/view/button.h"
#include "../../ui/view/label.h"
#include "../../ui/view/input.h"
#include "../../ui/view/textarea.h"

namespace qk { namespace js {

	void inheritTextOptions(JSClass* cls, Worker* worker) {
		typedef Object Type;
		Js_UIObject_Acce_Get(TextOptions, FontStyle, font_style, fontStyle);
		Js_UIObject_Accessor(TextOptions, TextAlign, text_align, textAlign);
		Js_UIObject_Accessor(TextOptions, FontWeight, font_weight, fontWeight);
		Js_UIObject_Accessor(TextOptions, FontSlant, font_slant, fontSlant);
		Js_UIObject_Accessor(TextOptions, TextDecoration, text_decoration, textDecoration);
		Js_UIObject_Accessor(TextOptions, TextOverflow, text_overflow, textOverflow);
		Js_UIObject_Accessor(TextOptions, WhiteSpace, white_space, whiteSpace);
		Js_UIObject_Accessor(TextOptions, WordBreak, word_break, wordBreak);
		Js_UIObject_Accessor(TextOptions, FontSize, font_size, fontSize);
		Js_UIObject_Accessor(TextOptions, TextColor, text_background_color, textBackgroundColor);
		Js_UIObject_Accessor(TextOptions, TextStroke, text_stroke, textStroke);
		Js_UIObject_Accessor(TextOptions, TextColor, text_color, textColor);
		Js_UIObject_Accessor(TextOptions, FontSize, line_height, lineHeight);
		Js_UIObject_Accessor(TextOptions, TextShadow, text_shadow, textShadow);
		Js_UIObject_Accessor(TextOptions, FontFamily, font_family, fontFamily);
		Js_Class_Method(computeLayoutSize, {
			if (!args.length())
				Js_Throw("@method TextOptions.compute_layout_size(cString& value, Vec2 limit)\n");
			auto str = args[0]->toString(worker)->value(worker);
			Js_Parse_Args(Vec2, 1, "limit = %s", (std::numeric_limits<float>::max()));
			Js_Self(Text);
			Js_Return( worker->types()->jsvalue(self->compute_layout_size(str, arg1)) );
		});
	}

	class MixText: public MixViewObject {
	public:
		typedef Text Type;
		virtual TextOptions* asTextOptions() { return self<Text>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Text, Box, { Js_NewView(Text); });
			inheritTextOptions(cls, worker);
			Js_MixObject_Accessor(Text, String, value, value);
			cls->exports("Text", exports);
		}
	};

	class MixButton: public MixViewObject {
	public:
		typedef Button Type;
		virtual TextOptions* asTextOptions() { return self<Button>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Button, Text, { Js_NewView(Button); });
			Js_Class_Method(nextButton, {
				if (!args.length()) {
					Js_Throw("@method next_button(FindDirection dir)\n");
				}
				Js_Parse_Type(Direction, args[0], "@method next_button(dir = %s)");
				Js_Return( self->next_button(out) );
			});
			cls->exports("Button", exports);
		}
	};

	class MixLabel: public MixViewObject {
	public:
		typedef Label Type;
		virtual TextOptions* asTextOptions() { return self<Label>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Label, View, { Js_NewView(Label); });
			inheritTextOptions(cls, worker);
			Js_MixObject_Accessor(Label, String, value, value);
			Js_MixObject_Accessor(Label, Align, align, align);
			cls->exports("Label", exports);
		}
	};

	class MixInput: public MixViewObject {
	public:
		typedef Input Type;
		virtual TextOptions* asTextOptions() { return self<Input>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Input, Box, { Js_NewView(Input); });
			inheritTextOptions(cls, worker);
			Js_MixObject_Accessor(Input, bool, security, security);
			Js_MixObject_Accessor(Input, bool, readonly, readonly);
			Js_MixObject_Acce_Get(Input, bool, is_marked_text, isMarkedText);
			Js_MixObject_Accessor(Input, KeyboardType, keyboard_type, keyboardType);
			Js_MixObject_Accessor(Input, KeyboardReturnType, return_type, returnType);
			Js_MixObject_Accessor(Input, Color, placeholder_color, placeholderColor);
			Js_MixObject_Accessor(Input, Color, cursor_color, cursorColor);
			Js_MixObject_Accessor(Input, uint32_t, max_length, maxLength);
			Js_MixObject_Acce_Get(Input, uint32_t, text_length, textLength);
			Js_MixObject_Acce_Get(Input, uint32_t, cursor_index, cursorIndex);
			Js_MixObject_Acce_Get(Input, uint32_t, cursor_line, cursorLine);
			Js_MixObject_Acce_Get(Input, String, marked_text, markedText);
			Js_MixObject_Acce_Get(Input, uint32_t, marked_text_index, markedTextIndex);
			Js_MixObject_Acce_Get(Input, uint32_t, marked_text_length, markedTextLength);

			Js_Class_Accessor(value, {
				Js_Return( self->value_u4() );
			}, {
				self->set_value_u4(val->toString(worker)->value4(worker));
			});

			Js_Class_Accessor(placeholder, {
				Js_Return( self->placeholder_u4() );
			}, {
				self->set_placeholder_u4(val->toString(worker)->value4(worker));
			});

			Js_Class_Method(cancelMarkedText, { self->cancel_marked_text(); });

			cls->exports("Input", exports);
		}
	};

	class MixTextarea: public MixViewObject {
	public:
		virtual TextOptions* asTextOptions() { return self<Textarea>(); }
		virtual ScrollView* asScrollView() { return self<Textarea>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Textarea, Input, { Js_NewView(Textarea); });
			inheritTextOptions(cls, worker);
			inheritScrollView(cls, worker);
			cls->exports("Textarea", exports);
		}
	};

	class MixInputSink: public MixViewObject {
	public:
		typedef InputSink Type;
		virtual TextOptions* asTextOptions() { return self<Input>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(InputSink, View, { Js_NewView(InputSink); });
			Js_MixObject_Acce_Get(InputSink, bool, is_marked_text, isMarkedText);
			Js_MixObject_Acce_Get(InputSink, String, marked_text, markedText);
			Js_MixObject_Acce_Get(InputSink, uint32_t, marked_text_length, markedTextLength);
			Js_MixObject_Acce_Get(InputSink, uint32_t, cursor_index, cursorIndex);
			Js_MixObject_Accessor(InputSink, Rect, spot_rect, spotRect);
			Js_MixObject_Accessor(InputSink, bool, can_delete, canDelete);
			Js_MixObject_Accessor(InputSink, bool, can_backspace, canBackspace);
			Js_MixObject_Accessor(InputSink, bool, readonly, readonly);
			Js_MixObject_Accessor(InputSink, KeyboardType, keyboard_type, keyboardType);
			Js_MixObject_Accessor(InputSink, KeyboardReturnType, return_type, returnType);
			Js_Class_Method(cancelMarkedText, { self->cancel_marked_text(); });
			cls->exports("InputSink", exports);
		}
	};

	class MixDefaultTextOptions: public MixUIObject {
	public:
		virtual TextOptions* asTextOptions() { return self<DefaultTextOptions>(); }
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(DefaultTextOptions, 0, { Js_Throw("Access forbidden."); });
			inheritTextOptions(cls, worker);
		}
	};

	void binding_text(JSObject* exports, Worker* worker) {
		MixText::binding(exports, worker);
		MixButton::binding(exports, worker);
		MixLabel::binding(exports, worker);
		MixInput::binding(exports, worker);
		MixTextarea::binding(exports, worker);
		MixInputSink::binding(exports, worker);
		MixDefaultTextOptions::binding(exports, worker);
} }}
