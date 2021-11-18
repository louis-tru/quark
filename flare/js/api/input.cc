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

#include "../_js.h"
#include "../_view.h"
#include "../../views2/input.h"
#include "../../views2/textarea.h"
#include "../../views2/scroll.h"

/**
 * @ns flare::js
 */

JS_BEGIN

class WrapInput: public WrapViewBase {
	public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapInput>(args, new Input());
	}
	
	static void type(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Input);
		JS_RETURN( worker->values()->New(self->type()) );
	}
	
	static void return_type(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Input);
		JS_RETURN( worker->values()->New(self->return_type()) );
	}
	
	static void placeholder(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Input);
		JS_RETURN( self->placeholder() );
	}
	
	static void placeholder_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Input);
		JS_RETURN( worker->values()->New(self->placeholder_color()) );
	}
	
	static void security(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Input);
		JS_RETURN( self->security() );
	}
	
	static void text_margin(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Input);
		JS_RETURN( self->text_margin() );
	}
	
	static void set_type(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		js_parse_value(KeyboardType, value, "Input.type = %s");
		JS_SELF(Input);
		self->set_type(out);
	}
	
	static void set_return_type(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		js_parse_value(KeyboardReturnType, value, "Input.returnType = %s");
		JS_SELF(Input);
		self->set_return_type(out);
	}
	
	static void set_placeholder(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Input);
		self->set_placeholder( value->ToString16Value(worker) );
	}
	
	static void set_placeholder_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		js_parse_value(Color, value, "Input.placeholderColor = %s");
		JS_SELF(Input);
		self->set_placeholder_color( out );
	}
	
	static void set_security(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Input);
		self->set_security( value->ToBooleanValue(worker) );
	}
	
	/**
	 * @set text_margin {float}
	 */
	static void set_text_margin(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("* @set textMargin {float}");
		}
		JS_SELF(Input);
		self->set_text_margin(value->ToNumberValue(worker));
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Input, constructor, {
			JS_SET_CLASS_ACCESSOR(type, type, set_type);
			JS_SET_CLASS_ACCESSOR(returnType, return_type, set_return_type);
			JS_SET_CLASS_ACCESSOR(placeholder, placeholder, set_placeholder);
			JS_SET_CLASS_ACCESSOR(placeholderColor, placeholder_color, set_placeholder_color);
			JS_SET_CLASS_ACCESSOR(security, security, set_security);
			JS_SET_CLASS_ACCESSOR(textMargin, text_margin, set_text_margin);
		}, Text);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Input), View::INPUT);
	}
};

class WrapTextarea: public WrapViewBase {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapTextarea>(args, new Textarea());
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Textarea, constructor, {
			WrapViewBase::inheritScroll(cls, worker);
		}, Input);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Textarea), View::TEXTAREA);
	}
};

void binding_input(Local<JSObject> exports, Worker* worker) {
	WrapInput::binding(exports, worker);
	WrapTextarea::binding(exports, worker);
}

JS_END
