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

#include "../_js.h"
#include "../_view.h"
#include "../../views2/indep.h"

/**
 * @ns qk::js
 */

JS_BEGIN

/**
 * @class WrapIndep
 */
class WrapIndep: public WrapViewBase {
	public:

	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapIndep>(args, new Indep());
	}
	
	static void align_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Indep);
		JS_RETURN( worker->values()->New(self->align_x()) );
	}
	static void align_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Indep);
		JS_RETURN( worker->values()->New(self->align_y()) );
	}
	static void set_align_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		js_parse_value(Align, value, "Indep.alignX = %s");
		JS_SELF(Indep);
		self->set_align_x(out);
	}
	static void set_align_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		js_parse_value(Align, value, "Indep.alignY = %s");
		JS_SELF(Indep);
		self->set_align_y(out);
	}
	
	static void align(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Indep);
		Local<JSArray> ret = worker->NewArray();
		ret->Set(worker, 0, worker->values()->New(self->align_x()) );
		ret->Set(worker, 1, worker->values()->New(self->align_y()) );
		JS_RETURN( ret );
	}
	
	static void set_align(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		js_parse_value2(Array<Align>, Aligns, value, "Indep.align = %s");
		JS_SELF(Indep);
		self->set_align_x(out[0]);
		self->set_align_y(out.length() > 1 ? out[1]: out[0]);
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Indep, constructor, {
			JS_SET_CLASS_ACCESSOR(alignX, align_x, set_align_x);
			JS_SET_CLASS_ACCESSOR(alignY, align_y, set_align_y);
			JS_SET_CLASS_ACCESSOR(align, align, set_align);
		}, Div);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Indep), View::INDEP);
	}
};

void binding_indep_div(Local<JSObject> exports, Worker* worker) {
	WrapIndep::binding(exports, worker);
}

JS_END
