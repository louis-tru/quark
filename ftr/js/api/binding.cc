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

#include "./_view.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

void binding_app(Local<JSObject> exports, Worker* worker);
void binding_view(Local<JSObject> exports, Worker* worker);
void binding_sprite(Local<JSObject> exports, Worker* worker);
void binding_layout(Local<JSObject> exports, Worker* worker);
void binding_box(Local<JSObject> exports, Worker* worker);
void binding_div(Local<JSObject> exports, Worker* worker);
void binding_hybrid(Local<JSObject> exports, Worker* worker);
void binding_span(Local<JSObject> exports, Worker* worker);
void binding_text_node(Local<JSObject> exports, Worker* worker);
void binding_image(Local<JSObject> exports, Worker* worker);
void binding_indep_div(Local<JSObject> exports, Worker* worker);
void binding_root(Local<JSObject> exports, Worker* worker);
void binding_label(Local<JSObject> exports, Worker* worker);
void binding_limit(Local<JSObject> exports, Worker* worker);
void binding_action(Local<JSObject> exports, Worker* worker);
void binding_button(Local<JSObject> exports, Worker* worker);
void binding_panel(Local<JSObject> exports, Worker* worker);
void binding_scroll(Local<JSObject> exports, Worker* worker);
void binding_text(Local<JSObject> exports, Worker* worker);
void binding_input(Local<JSObject> exports, Worker* worker);

/**
 * @class NativeFtr
 */
class NativeFtr {
	public:

	static void lock(FunctionCall args) {
		JS_WORKER(args);
		if (args.Length() < 1 || ! args[0]->IsFunction(worker)) {
			JS_THROW_ERR("Bad argument");
		}
		GUILock lock;
		Local<JSValue> r = args[0].To<JSFunction>()->Call(worker);
		if (!r.IsEmpty()) {
			JS_RETURN(r);
		}
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->bindingModule("_value");	FX_DEBUG("binding ftr_value ok");
		worker->bindingModule("_event");	FX_DEBUG("binding ftr_event ok");
		binding_app(exports, worker); 			FX_DEBUG("binding app ok");
		binding_view(exports, worker); 			FX_DEBUG("binding view ok");
		binding_sprite(exports, worker); 		FX_DEBUG("binding sprite ok");
		binding_layout(exports, worker); 		FX_DEBUG("binding layout ok");
		binding_box(exports, worker); 			FX_DEBUG("binding box ok");
		binding_div(exports, worker); 			FX_DEBUG("binding div ok");
		binding_panel(exports, worker); 		FX_DEBUG("binding panel ok");
		binding_hybrid(exports, worker); 		FX_DEBUG("binding hybrid ok");
		binding_span(exports, worker); 			FX_DEBUG("binding span ok");
		binding_text_node(exports, worker); FX_DEBUG("binding text_node ok");
		binding_image(exports, worker); 		FX_DEBUG("binding image ok");
		binding_indep_div(exports, worker); FX_DEBUG("binding indep_div ok");
		binding_root(exports, worker); 			FX_DEBUG("binding root ok");
		binding_label(exports, worker); 		FX_DEBUG("binding label ok");
		binding_limit(exports, worker); 		FX_DEBUG("binding limit ok");
		binding_scroll(exports, worker); 		FX_DEBUG("binding scroll ok");
		binding_text(exports, worker); 			FX_DEBUG("binding text ok");
		binding_button(exports, worker); 		FX_DEBUG("binding button ok");
		binding_input(exports, worker); 		FX_DEBUG("binding input ok");
		JS_SET_METHOD(lock, lock); 					FX_DEBUG("binding lock ok");
	}
};

JS_REG_MODULE(_ftr, NativeFtr);
JS_END
