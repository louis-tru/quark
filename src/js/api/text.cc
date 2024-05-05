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

#include "./view.h"
#include "../../ui/view/text.h"
#include "../../ui/view/label.h"
#include "../../ui/view/input.h"
#include "../../ui/view/textarea.h"

namespace qk { namespace js {

	void inheritTextOptions(JSClass* cls, Worker* worker) {
		Js_Set_Class_Accessor_Get(fontStyle, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->font_style()) );
		});

		Js_Set_Class_Accessor(textAlign, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_align()) );
		}, {
			Js_Parse_Type(TextAlign, val, "TextOptions.textAlign = %s");
			Js_TextOptions();
			self->set_text_align(out);
		});

		Js_Set_Class_Accessor(textWeight, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_weight()) );
		}, {
			Js_Parse_Type(TextWeight, val, "TextOptions.textWeight = %s");
			Js_TextOptions();
			self->set_text_weight(out);
		});

		Js_Set_Class_Accessor(textSlant, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_weight()) );
		}, {
			Js_Parse_Type(TextSlant, val, "TextOptions.textSlant = %s");
			Js_TextOptions();
			self->set_text_slant(out);
		});

		Js_Set_Class_Accessor(textDecoration, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_decoration()) );
		}, {
			Js_Parse_Type(TextDecoration, val, "TextOptions.textDecoration = %s");
			Js_TextOptions();
			self->set_text_decoration(out);
		});

		Js_Set_Class_Accessor(textOverflow, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_overflow()) );
		}, {
			Js_Parse_Type(TextOverflow, val, "TextOptions.textOverflow = %s");
			Js_TextOptions();
			self->set_text_overflow(out);
		});

		Js_Set_Class_Accessor(textWhiteSpace, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_white_space()) );
		}, {
			Js_Parse_Type(TextWhiteSpace, val, "TextOptions.textWhiteSpace = %s");
			Js_TextOptions();
			self->set_text_white_space(out);
		});

		Js_Set_Class_Accessor(textWordBreak, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_word_break()) );
		}, {
			Js_Parse_Type(TextWordBreak, val, "TextOptions.textWordBreak = %s");
			Js_TextOptions();
			self->set_text_word_break(out);
		});

		Js_Set_Class_Accessor(textSize, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_size()) );
		}, {
			Js_Parse_Type(TextSize, val, "TextOptions.textSize = %s");
			Js_TextOptions();
			self->set_text_size(out);
		});

		Js_Set_Class_Accessor(textBackgroundColor, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_background_color()) );
		}, {
			Js_Parse_Type(TextColor, val, "TextOptions.textBackgroundColor = %s");
			Js_TextOptions();
			self->set_text_background_color(out);
		});

		Js_Set_Class_Accessor(textColor, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_color()) );
		}, {
			Js_Parse_Type(TextColor, val, "TextOptions.textColor = %s");
			Js_TextOptions();
			self->set_text_color(out);
		});

		Js_Set_Class_Accessor(textLineHeight, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_line_height()) );
		}, {
			Js_Parse_Type(TextSize, val, "TextOptions.textLineHeight = %s");
			Js_TextOptions();
			self->set_text_line_height(out);
		});

		Js_Set_Class_Accessor(textShadow, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_shadow()) );
		}, {
			Js_Parse_Type(TextShadow, val, "TextOptions.textShadow = %s");
			Js_TextOptions();
			self->set_text_shadow(out);
		});

		Js_Set_Class_Accessor(textFamily, {
			Js_TextOptions();
			Js_Return( worker->types()->newInstance(self->text_family()) );
		}, {
			Js_Parse_Type(TextFamily, val, "TextOptions.textFamily = %s");
			Js_TextOptions();
			self->set_text_family(out);
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
			cls->exports("Text", exports);
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

			Js_Set_Class_Accessor(value, {
				Js_Self(Label);
				Js_Return( self->value() );
			}, {
				Js_Parse_Type(String, val, "Label.value = %s");
				Js_Self(Label);
				self->set_value(out);
			});

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

			Js_Set_Class_Accessor(security, {
				Js_Self(Input);
				Js_Return( self->security() );
			}, {
				Js_Parse_Type(bool, val, "Input.security = %s");
				Js_Self(Input);
				self->set_security(out);
			});

			Js_Set_Class_Accessor(readonly, {
				Js_Self(Input);
				Js_Return( self->readonly() );
			}, {
				Js_Parse_Type(bool, val, "Input.readonly = %s");
				Js_Self(Input);
				self->set_readonly(out);
			});

			Js_Set_Class_Accessor(type, {
				Js_Self(Input);
				Js_Return( worker->types()->newInstance(self->type()) );
			}, {
				Js_Parse_Type(KeyboardType, val, "Input.type = %s");
				Js_Self(Input);
				self->set_type(out);
			});

			Js_Set_Class_Accessor(returnType, {
				Js_Self(Input);
				Js_Return( worker->types()->newInstance(self->return_type()) );
			}, {
				Js_Parse_Type(KeyboardReturnType, val, "Input.returnType = %s");
				Js_Self(Input);
				self->set_return_type(out);
			});

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

			Js_Set_Class_Accessor(placeholderColor, {
				Js_Self(Input);
				Js_Return( worker->types()->newInstance(self->placeholder_color()) );
			}, {
				Js_Parse_Type(Color, val, "Input.placeholderColor = %s");
				Js_Self(Input);
				self->set_placeholder_color(out);
			});

			Js_Set_Class_Accessor(cursorColor, {
				Js_Self(Input);
				Js_Return( worker->types()->newInstance(self->cursor_color()) );
			}, {
				Js_Parse_Type(Color, val, "Input.cursorColor = %s");
				Js_Self(Input);
				self->set_cursor_color(out);
			});

			Js_Set_Class_Accessor(maxLength, {
				Js_Self(Input);
				Js_Return( worker->types()->newInstance(self->max_length()) );
			}, {
				Js_Parse_Type(float, val, "Input.maxLength = %s");
				Js_Self(Input);
				self->set_max_length(out);
			});

			Js_Set_Class_Accessor_Get(textLength, {
				Js_Self(Input);
				Js_Return( worker->types()->newInstance(self->text_length()) );
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
		WrapLabel::binding(exports, worker);
		WrapInput::binding(exports, worker);
		WrapTextarea::binding(exports, worker);
		WrapDefaultTextOptions::binding(exports, worker);
	}
} }
