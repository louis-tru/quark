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

#include "qgr/js/js-1.h"
#include "qgr/js/qgr.h"
#include "qgr/limit.h"
#include "qgr/limit-indep.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

template<class T> class WrapLimit: public WrapViewBase {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		js_check_gui_app();
		New<WrapLimit<T>>(args, new T());
	}
	static void min_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(T);
		JS_RETURN( worker->value_program()->New(self->min_width()) );
	}
	static void min_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(T);
		JS_RETURN( worker->value_program()->New(self->min_height()) );
	}
	static void max_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(T);
		JS_RETURN( worker->value_program()->New(self->max_width()) );
	}
	static void max_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(T);
		JS_RETURN( worker->value_program()->New(self->max_height()) );
	}
	static void set_min_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Limit.minWidth = %s");
		JS_SELF(T);
		self->set_min_width(out);
	}
	static void set_min_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Limit.minHeight = %s");
		JS_SELF(T);
		self->set_min_height(out);
	}
	static void set_max_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Limit.maxWidth = %s");
		JS_SELF(T);
		self->set_max_width(out);
	}
	static void set_max_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Limit.maxHeight = %s");
		JS_SELF(T);
		self->set_max_height(out);
	}
};

/**
 * @class WrapLimitDiv
 */
class WrapLimitDiv: public WrapLimit<Limit> {
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Limit, constructor, {
			JS_SET_CLASS_ACCESSOR(minWidth, min_width, set_min_width);
			JS_SET_CLASS_ACCESSOR(minHeight, min_height, set_min_height);
			JS_SET_CLASS_ACCESSOR(maxWidth, max_width, set_max_width);
			JS_SET_CLASS_ACCESSOR(maxHeight, max_height, set_max_height);
		}, Div);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Limit), View::LIMIT);
	}
};

/**
 * @class WrapLimitIndep
 */
class WrapLimitIndep: public WrapLimit<LimitIndep> {
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(LimitIndep, constructor, {
			JS_SET_CLASS_ACCESSOR(minWidth, min_width, set_min_width);
			JS_SET_CLASS_ACCESSOR(minHeight, min_height, set_min_height);
			JS_SET_CLASS_ACCESSOR(maxWidth, max_width, set_max_width);
			JS_SET_CLASS_ACCESSOR(maxHeight, max_height, set_max_height);
		}, Indep);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(LimitIndep), View::LIMIT_INDEP);
	}
};

void binding_limit(Local<JSObject> exports, Worker* worker) {
	WrapLimitDiv::binding(exports, worker);
	WrapLimitIndep::binding(exports, worker);
}

JS_END
