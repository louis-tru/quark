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
#include "../../views2/indep.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapIndep
 */
class WrapIndep: public WrapViewBase {
	public:

	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapIndep>(args, new Indep());
	}
	
	static void align_x(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Indep);
		Js_Return( worker->values()->New(self->align_x()) );
	}
	static void align_y(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Indep);
		Js_Return( worker->values()->New(self->align_y()) );
	}
	static void set_align_x(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Align, value, "Indep.alignX = %s");
		Js_Self(Indep);
		self->set_align_x(out);
	}
	static void set_align_y(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Align, value, "Indep.alignY = %s");
		Js_Self(Indep);
		self->set_align_y(out);
	}
	
	static void align(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Indep);
		Local<JSArray> ret = worker->NewArray();
		ret->Set(worker, 0, worker->values()->New(self->align_x()) );
		ret->Set(worker, 1, worker->values()->New(self->align_y()) );
		Js_Return( ret );
	}
	
	static void set_align(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value2(Array<Align>, Aligns, value, "Indep.align = %s");
		Js_Self(Indep);
		self->set_align_x(out[0]);
		self->set_align_y(out.length() > 1 ? out[1]: out[0]);
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(Indep, constructor, {
			Js_Set_Class_Accessor(alignX, align_x, set_align_x);
			Js_Set_Class_Accessor(alignY, align_y, set_align_y);
			Js_Set_Class_Accessor(align, align, set_align);
		}, Div);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(Indep), View::INDEP);
	}
};

void binding_indep_div(JSObject* exports, Worker* worker) {
	WrapIndep::binding(exports, worker);
}

Js_END
