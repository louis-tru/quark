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

#include "ftr-js/ftr.h"
#include "ftr/text-font.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

/**
 * @class WrapTextFont
 */
class WrapTextFont {
 public:

	static void text_background_color(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_background_color()) );
	}
	static void text_color(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_color()) );
	}
	static void text_size(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_size()) );
	}
	static void text_style(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_style()) );
	}
	static void text_family(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_family()) );
	}
	static void text_shadow(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_shadow()) );
	}
	static void text_line_height(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_line_height()) );
	}
	static void text_decoration(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		JS_RETURN( worker->values()->New(text->text_decoration()) );
	}
	static void set_text_background_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextColor, value, "TextFont.textBackgroundColor = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_background_color(out);
	}
	// set
	static void set_text_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextColor, value, "TextFont.textColor = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_color(out);
	}
	static void set_text_size(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextSize, value, "TextFont.textSize = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_size(out);
	}
	static void set_text_style(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextStyle, value, "TextFont.textStyle = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_style(out);
	}
	static void set_text_family(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextFamily, value, "TextFont.textFamily = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_family(out);
	}
	static void set_text_shadow(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextShadow, value, "TextFont.textShadow = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_shadow(out);
	}
	static void set_text_line_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextLineHeight, value, "TextFont.textLineHeight = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_line_height(out);
	}
	static void set_text_decoration(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextDecoration, value, "TextFont.textDecoration = %s");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		text->set_text_decoration(out);
	}
	
	static void simple_layout_width(FunctionCall args) {
//		JS_WORKER(args); GUILock lock;
//		if ( args.Length() < 1 ) JS_THROW_ERR("Bad argument.");
//		JS_SELF(View);
//		TextFont* text = self->as_text_font();
//		float width = text->simple_layout_width( args[0]->ToUcs2StringValue(worker) );
//		JS_RETURN ( width );
	}

	static void inherit(Local<JSClass> cls, Worker* worker) {
		JS_SET_CLASS_METHOD(simpleLayoutWidth, simple_layout_width);
		JS_SET_CLASS_ACCESSOR(textBackgroundColor, text_background_color, set_text_background_color);
		JS_SET_CLASS_ACCESSOR(textColor, text_color, set_text_color);
		JS_SET_CLASS_ACCESSOR(textSize, text_size, set_text_size);
		JS_SET_CLASS_ACCESSOR(textStyle, text_style, set_text_style);
		JS_SET_CLASS_ACCESSOR(textFamily, text_family, set_text_family);
		JS_SET_CLASS_ACCESSOR(textShadow, text_shadow, set_text_shadow);
		JS_SET_CLASS_ACCESSOR(textLineHeight, text_line_height, set_text_line_height);
		JS_SET_CLASS_ACCESSOR(textDecoration, text_decoration, set_text_decoration);
	}
};

class WrapTextLayout {
 public:

	static void text_overflow(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextLayout* text = self->as_text_layout();
//		JS_RETURN( worker->values()->New(text->text_overflow()) );
	}
	static void text_white_space(Local<JSString> name, PropertyCall args) {
//		JS_WORKER(args);
//		JS_SELF(View);
//		TextLayout* text = self->as_text_layout();
//		JS_RETURN( worker->values()->New(text->text_white_space()) );
	}
	static void set_text_overflow(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextOverflow, value, "TextLayout.textOverflow = %s");
//		JS_SELF(View);
//		TextLayout* text = self->as_text_layout();
//		text->set_text_overflow(out);
	}
	static void set_text_white_space(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
//		JS_WORKER(args); GUILock lock;
//		js_parse_value(TextWhiteSpace, value, "TextLayout.textWhiteSpace = %s");
//		JS_SELF(View);
//		TextLayout* text = self->as_text_layout();
//		text->set_text_white_space(out);
	}

	static void inherit(Local<JSClass> cls, Worker* worker) {
		WrapTextFont::inherit(cls, worker);
		JS_SET_CLASS_ACCESSOR(textOverflow, text_overflow, set_text_overflow);
		JS_SET_CLASS_ACCESSOR(textWhiteSpace, text_white_space, set_text_white_space);
	}
};

void WrapViewBase::inheritTextFont(Local<JSClass> cls, Worker* worker) {
	WrapTextFont::inherit(cls, worker);
}

void WrapViewBase::inheritTextLayout(Local<JSClass> cls, Worker* worker) {
	WrapTextLayout::inherit(cls, worker);
}

JS_END
