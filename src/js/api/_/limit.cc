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
#include "../../views2/limit.h"
#include "../../views2/limit-indep.h"

/**
 * @ns qk::js
 */

Js_BEGIN

template<class T> class WrapLimit: public WrapViewBase {
	public:
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapLimit<T>>(args, new T());
	}
	static void min_width(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(T);
		Js_Return( worker->values()->New(self->min_width()) );
	}
	static void min_height(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(T);
		Js_Return( worker->values()->New(self->min_height()) );
	}
	static void max_width(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(T);
		Js_Return( worker->values()->New(self->max_width()) );
	}
	static void max_height(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(T);
		Js_Return( worker->values()->New(self->max_height()) );
	}
	static void set_min_width(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Value, value, "Limit.minWidth = %s");
		Js_Self(T);
		self->set_min_width(out);
	}
	static void set_min_height(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Value, value, "Limit.minHeight = %s");
		Js_Self(T);
		self->set_min_height(out);
	}
	static void set_max_width(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Value, value, "Limit.maxWidth = %s");
		Js_Self(T);
		self->set_max_width(out);
	}
	static void set_max_height(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Value, value, "Limit.maxHeight = %s");
		Js_Self(T);
		self->set_max_height(out);
	}
};

/**
 * @class WrapLimitDiv
 */
class WrapLimitDiv: public WrapLimit<Limit> {
	public:
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(Limit, constructor, {
			Js_Set_Class_Accessor(minWidth, min_width, set_min_width);
			Js_Set_Class_Accessor(minHeight, min_height, set_min_height);
			Js_Set_Class_Accessor(maxWidth, max_width, set_max_width);
			Js_Set_Class_Accessor(maxHeight, max_height, set_max_height);
		}, Div);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(Limit), View::LIMIT);
	}
};

/**
 * @class WrapLimitIndep
 */
class WrapLimitIndep: public WrapLimit<LimitIndep> {
	public:
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(LimitIndep, constructor, {
			Js_Set_Class_Accessor(minWidth, min_width, set_min_width);
			Js_Set_Class_Accessor(minHeight, min_height, set_min_height);
			Js_Set_Class_Accessor(maxWidth, max_width, set_max_width);
			Js_Set_Class_Accessor(maxHeight, max_height, set_max_height);
		}, Indep);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(LimitIndep), View::LIMIT_INDEP);
	}
};

void binding_limit(JSObject* exports, Worker* worker) {
	WrapLimitDiv::binding(exports, worker);
	WrapLimitIndep::binding(exports, worker);
}

Js_END
