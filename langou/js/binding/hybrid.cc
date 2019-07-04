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

#include "langou/js/js-1.h"
#include "langou/js/langou.h"
#include "langou/hybrid.h"

/**
 * @ns langou::js
 */

JS_BEGIN

/**
 * @class WrapHybrid
 */
class WrapHybrid: public WrapViewBase {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapHybrid>(args, new Hybrid());
	}
	
	static void text_align(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Hybrid);
		JS_RETURN( worker->values()->New(self->text_align()) );
	}
	
	static void set_text_align(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(TextAlign, value, "Hybrid.textAlign = %s");
		JS_SELF(Hybrid);
		self->set_text_align(out);
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Hybrid, constructor, {
			JS_SET_CLASS_ACCESSOR(textAlign, text_align, set_text_align);
			WrapViewBase::inherit_text_layout(cls, worker);
		}, Box);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Hybrid), View::HYBRID);
	}
};

void binding_hybrid(Local<JSObject> exports, Worker* worker) {
	WrapHybrid::binding(exports, worker);
}

JS_END
