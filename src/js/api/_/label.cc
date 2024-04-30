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

#include "../js_.h"
#include "../_view.h"
#include "../../views2/label.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapLabel
 */
class WrapLabel: public WrapViewBase {
	public:

	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapLabel>(args, new Label());
	}
	
	static void length(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Label);
		Js_Return( self->length() );
	}

	static void value(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Label);
		Js_Return( self->value() );
	}
	
	static void text_hori_bearing(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Label);
		Js_Return( self->text_hori_bearing() );
	}
	
	static void text_height(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Label);
		Js_Return( self->text_height() );
	}
	
	static void set_value(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Label);
		String2 str = value->ToString2Value(worker);
		self->set_value(str);
	}
	
	static void text_align(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Label);
		Js_Return( worker->values()->New(self->text_align()) );
	}
	
	static void set_text_align(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(TextAlign, value, "Label.textAlign = %s");
		Js_Self(Label);
		self->set_text_align(out);
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(Label, constructor, {
			Js_Set_Class_Accessor(length, length);
			Js_Set_Class_Accessor(value, value, set_value);
			Js_Set_Class_Accessor(textHoriBearing, text_hori_bearing);
			Js_Set_Class_Accessor(textHeight, text_height);
			Js_Set_Class_Accessor(textAlign, text_align, set_text_align);
			WrapViewBase::inheritTextFont(cls, worker);
		}, View);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(Label), View::LABEL);
	}
};

void binding_label(JSObject* exports, Worker* worker) {
	WrapLabel::binding(exports, worker);
}

Js_END
