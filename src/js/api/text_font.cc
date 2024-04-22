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

#include "./_view.h"
#include "../../views2/text-font.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapTextFont
 */
class WrapTextFont {
	public:

	static void text_background_color(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_background_color()) );
	}
	static void text_color(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_color()) );
	}
	static void text_size(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_size()) );
	}
	static void text_slant(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_slant()) );
	}
	static void text_family(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_family()) );
	}
	static void text_shadow(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_shadow()) );
	}
	static void text_line_height(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_line_height()) );
	}
	static void text_decoration(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextFont* text = self->as_text_font();
		Js_Return( worker->values()->New(text->text_decoration()) );
	}
	static void set_text_background_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextColor, value, "TextFont.textBackgroundColor = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_background_color(out);
	}
	// set
	static void set_text_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextColor, value, "TextFont.textColor = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_color(out);
	}
	static void set_text_size(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextSize, value, "TextFont.textSize = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_size(out);
	}
	static void set_text_slant(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextSlant, value, "TextFont.TextSlant = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_slant(out);
	}
	static void set_text_family(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextFamily, value, "TextFont.textFamily = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_family(out);
	}
	static void set_text_shadow(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextShadow, value, "TextFont.textShadow = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_shadow(out);
	}
	static void set_text_line_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextLineHeight, value, "TextFont.textLineHeight = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_line_height(out);
	}
	static void set_text_decoration(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextDecoration, value, "TextFont.textDecoration = %s");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		text->set_text_decoration(out);
	}
	
	static void simple_layout_width(FunctionCall args) {
		Js_Worker(args); UILock lock;
		if ( args.Length() < 1 ) Js_Throw("Bad argument.");
		Js_Self(View);
		TextFont* text = self->as_text_font();
		float width = text->simple_layout_width( args[0]->ToString2Value(worker) );
		Js_Return ( width );
	}

	static void inherit(Local<JSClass> cls, Worker* worker) {
		Js_Set_Class_Method(simpleLayoutWidth, simple_layout_width);
		Js_Set_Class_Accessor(textBackgroundColor, text_background_color, set_text_background_color);
		Js_Set_Class_Accessor(textColor, text_color, set_text_color);
		Js_Set_Class_Accessor(textSize, text_size, set_text_size);
		Js_Set_Class_Accessor(TextSlant, text_slant, set_text_slant);
		Js_Set_Class_Accessor(textFamily, text_family, set_text_family);
		Js_Set_Class_Accessor(textShadow, text_shadow, set_text_shadow);
		Js_Set_Class_Accessor(textLineHeight, text_line_height, set_text_line_height);
		Js_Set_Class_Accessor(textDecoration, text_decoration, set_text_decoration);
	}
};

class WrapTextLayout {
	public:

	static void text_overflow(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextLayout* text = self->as_text_layout();
		Js_Return( worker->values()->New(text->text_overflow()) );
	}
	static void text_white_space(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(View);
		TextLayout* text = self->as_text_layout();
		Js_Return( worker->values()->New(text->text_white_space()) );
	}
	static void set_text_overflow(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextOverflow, value, "TextLayout.textOverflow = %s");
		Js_Self(View);
		TextLayout* text = self->as_text_layout();
		text->set_text_overflow(out);
	}
	static void set_text_white_space(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextWhiteSpace, value, "TextLayout.textWhiteSpace = %s");
		Js_Self(View);
		TextLayout* text = self->as_text_layout();
		text->set_text_white_space(out);
	}

	static void inherit(Local<JSClass> cls, Worker* worker) {
		WrapTextFont::inherit(cls, worker);
		Js_Set_Class_Accessor(textOverflow, text_overflow, set_text_overflow);
		Js_Set_Class_Accessor(textWhiteSpace, text_white_space, set_text_white_space);
	}
};

void WrapViewBase::inheritTextFont(Local<JSClass> cls, Worker* worker) {
	WrapTextFont::inherit(cls, worker);
}

void WrapViewBase::inheritTextLayout(Local<JSClass> cls, Worker* worker) {
	WrapTextLayout::inherit(cls, worker);
}

Js_END
