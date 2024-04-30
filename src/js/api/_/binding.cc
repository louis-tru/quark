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

/**
 * @ns qk::js
 */

Js_BEGIN

void binding_app(JSObject* exports, Worker* worker);
void binding_view(JSObject* exports, Worker* worker);
void binding_sprite(JSObject* exports, Worker* worker);
void binding_layout(JSObject* exports, Worker* worker);
void binding_box(JSObject* exports, Worker* worker);
void binding_div(JSObject* exports, Worker* worker);
void binding_hybrid(JSObject* exports, Worker* worker);
void binding_span(JSObject* exports, Worker* worker);
void binding_text_node(JSObject* exports, Worker* worker);
void binding_image(JSObject* exports, Worker* worker);
void binding_indep_div(JSObject* exports, Worker* worker);
void binding_root(JSObject* exports, Worker* worker);
void binding_label(JSObject* exports, Worker* worker);
void binding_limit(JSObject* exports, Worker* worker);
void binding_action(JSObject* exports, Worker* worker);
void binding_button(JSObject* exports, Worker* worker);
void binding_panel(JSObject* exports, Worker* worker);
void binding_scroll(JSObject* exports, Worker* worker);
void binding_text(JSObject* exports, Worker* worker);
void binding_input(JSObject* exports, Worker* worker);

/**
 * @class NativeQuark
 */
class NativeQuark {
	public:

	static void lock(FunctionArgs args) {
		Js_Worker(args);
		if (args.length() < 1 || ! args[0]->isFunction(worker)) {
			Js_Throw("Bad argument");
		}
		UILock lock;
		JSValue* r = args[0].To<JSFunction>()->Call(worker);
		if (!r.IsEmpty()) {
			Js_Return(r);
		}
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		worker->bindingModule("_value");	Qk_DEBUG("binding quark_value ok");
		worker->bindingModule("_event");	Qk_DEBUG("binding quark_event ok");
		binding_app(exports, worker); 			Qk_DEBUG("binding app ok");
		binding_view(exports, worker); 			Qk_DEBUG("binding view ok");
		binding_sprite(exports, worker); 		Qk_DEBUG("binding sprite ok");
		binding_layout(exports, worker); 		Qk_DEBUG("binding layout ok");
		binding_box(exports, worker); 			Qk_DEBUG("binding box ok");
		binding_div(exports, worker); 			Qk_DEBUG("binding div ok");
		binding_panel(exports, worker); 		Qk_DEBUG("binding panel ok");
		binding_hybrid(exports, worker); 		Qk_DEBUG("binding hybrid ok");
		binding_span(exports, worker); 			Qk_DEBUG("binding span ok");
		binding_text_node(exports, worker); Qk_DEBUG("binding text_node ok");
		binding_image(exports, worker); 		Qk_DEBUG("binding image ok");
		binding_indep_div(exports, worker); Qk_DEBUG("binding indep_div ok");
		binding_root(exports, worker); 			Qk_DEBUG("binding root ok");
		binding_label(exports, worker); 		Qk_DEBUG("binding label ok");
		binding_limit(exports, worker); 		Qk_DEBUG("binding limit ok");
		binding_scroll(exports, worker); 		Qk_DEBUG("binding scroll ok");
		binding_text(exports, worker); 			Qk_DEBUG("binding text ok");
		binding_button(exports, worker); 		Qk_DEBUG("binding button ok");
		binding_input(exports, worker); 		Qk_DEBUG("binding input ok");
		Js_Set_Method(lock, lock); 					Qk_DEBUG("binding lock ok");
	}
};

Js_REG_MODULE(_quark, NativeQuark);
Js_END
