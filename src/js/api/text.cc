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

#include "./ui.h"
#include "../../ui/view/text.h"
#include "../../ui/view/label.h"
#include "../../ui/view/input.h"
#include "../../ui/view/textarea.h"

namespace qk { namespace js {

	void inheritTextOptions(JSClass* cls, Worker* worker) {
		Js_Set_Class_Accessor_Get(fontStyle, {
			Js_TextOptions();
			Js_Return( worker->types()->jsvalue(self->font_style()) );
		});
		Js_Set_UIObject_Accessor(TextOptions, TextAlign, text_align, textAlign);
		Js_Set_UIObject_Accessor(TextOptions, TextWeight, text_weight, textWeight);
		Js_Set_UIObject_Accessor(TextOptions, TextSlant, text_slant, textSlant);
		Js_Set_UIObject_Accessor(TextOptions, TextDecoration, text_decoration, textDecoration);
		Js_Set_UIObject_Accessor(TextOptions, TextOverflow, text_overflow, textOverflow);
		Js_Set_UIObject_Accessor(TextOptions, TextWhiteSpace, text_white_space, textWhiteSpace);
		Js_Set_UIObject_Accessor(TextOptions, TextWordBreak, text_word_break, textWordBreak);
		Js_Set_UIObject_Accessor(TextOptions, TextSize, text_size, textSize);
		Js_Set_UIObject_Accessor(TextOptions, TextColor, text_background_color, textBackgroundColor);
		Js_Set_UIObject_Accessor(TextOptions, TextColor, text_color, textColor);
		Js_Set_UIObject_Accessor(TextOptions, TextSize, text_line_height, textLineHeight);
		Js_Set_UIObject_Accessor(TextOptions, TextShadow, text_shadow, textShadow);
		Js_Set_UIObject_Accessor(TextOptions, TextFamily, text_family, textFamily);

		Js_Set_Class_Method(computeLayoutSize, {
			if (!args.length()) {
				Js_Throw("@method TextOptions.compute_layout_size(cString& value)\n");
			}
			auto str = args[0]->toStringValue2(worker);
			Js_Self(Text);
			Js_Return( worker->types()->jsvalue(self->compute_layout_size(str)) );
		});
	}

	class WrapText: public WrapViewObject {
	public:
		virtual TextOptions* asTextOptions() {
			return self<Text>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Text, Box, {
				Js_NewView(Text);
			});
			inheritTextOptions(cls, worker);
			Js_Set_WrapObject_Accessor(Text, String, value, value);
			cls->exports("Text", exports);
		}
	};

	class WrapButton: public WrapViewObject {
	public:
		virtual TextOptions* asTextOptions() {
			return self<Button>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Button, Text, {
				Js_NewView(Button);
			});
			Js_Set_Class_Method(nextButton, {
				if (!args.length()) {
					Js_Throw("@method next_button(FindDirection dir)\n");
				}
				Js_Parse_Type(FindDirection, args[0], "@method next_button(dir = %s)");
				Js_Self(Button);
				Js_Return( self->next_button(out) );
			});
			cls->exports("Button", exports);
		}
	};

	class WrapLabel: public WrapViewObject {
	public:
		virtual TextOptions* asTextOptions() {
			return self<Label>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Label, View, {
				Js_NewView(Label);
			});
			inheritTextOptions(cls, worker);
			Js_Set_WrapObject_Accessor(Label, String, value, value);
			cls->exports("Label", exports);
		}
	};

	class WrapInput: public WrapViewObject {
	public:
		virtual TextOptions* asTextOptions() {
			return self<Input>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Input, Box, {
				Js_NewView(Input);
			});
			inheritTextOptions(cls, worker);

			Js_Set_WrapObject_Accessor(Input, bool, security, security);
			Js_Set_WrapObject_Accessor(Input, bool, readonly, readonly);
			Js_Set_WrapObject_Accessor(Input, KeyboardType, type, type);
			Js_Set_WrapObject_Accessor(Input, KeyboardReturnType, return_type, returnType);
			Js_Set_WrapObject_Accessor(Input, Color, placeholder_color, placeholderColor);
			Js_Set_WrapObject_Accessor(Input, Color, cursor_color, cursorColor);
			Js_Set_WrapObject_Accessor(Input, uint32_t, max_length, maxLength);

			Js_Set_Class_Accessor(value, {
				Js_Self(Input);
				Js_Return( self->value_u4() );
			}, {
				Js_Self(Input);
				self->set_value_u4(val->toStringValue4(worker));
			});

			Js_Set_Class_Accessor(placeholder, {
				Js_Self(Input);
				Js_Return( self->placeholder_u4() );
			}, {
				Js_Self(Input);
				self->set_placeholder_u4(val->toStringValue4(worker));
			});

			Js_Set_Class_Accessor_Get(textLength, {
				Js_Self(Input);
				Js_Return( worker->types()->jsvalue(self->text_length()) );
			});

			cls->exports("Input", exports);
		}
	};

	class WrapTextarea: public WrapViewObject {
	public:
		virtual TextOptions* asTextOptions() {
			return self<Textarea>();
		}
		virtual ScrollBase* asScrollBase() {
			return self<Textarea>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Textarea, Input, {
				Js_NewView(Textarea);
			});
			inheritTextOptions(cls, worker);
			inheritScrollBase(cls, worker);
			cls->exports("Textarea", exports);
		}
	};

	class WrapDefaultTextOptions: public WrapUIObject {
	public:
		virtual TextOptions* asTextOptions() {
			return self<DefaultTextOptions>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(DefaultTextOptions, 0, {
				Js_Throw("Access forbidden.");
			});
			inheritTextOptions(cls, worker);
		}
	};

	void binding_text(JSObject* exports, Worker* worker) {
		WrapText::binding(exports, worker);
		WrapButton::binding(exports, worker);
		WrapLabel::binding(exports, worker);
		WrapInput::binding(exports, worker);
		WrapTextarea::binding(exports, worker);
		WrapDefaultTextOptions::binding(exports, worker);
	}
} }
