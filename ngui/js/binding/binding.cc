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

#include "ngui/js/ngui.h"

/**
 * @ns ngui::js
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
 * @class NativeNgui
 */
class NativeNgui {
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
		worker->binding_module("_value");	XX_DEBUG("binding ngui_value ok");
		worker->binding_module("_event");	XX_DEBUG("binding ngui_event ok");
		binding_app(exports, worker); 			XX_DEBUG("binding app ok");
		binding_view(exports, worker); 			XX_DEBUG("binding view ok");
		binding_sprite(exports, worker); 		XX_DEBUG("binding sprite ok");
		binding_layout(exports, worker); 		XX_DEBUG("binding layout ok");
		binding_box(exports, worker); 			XX_DEBUG("binding box ok");
		binding_div(exports, worker); 			XX_DEBUG("binding div ok");
		binding_panel(exports, worker); 		XX_DEBUG("binding panel ok");
		binding_hybrid(exports, worker); 		XX_DEBUG("binding hybrid ok");
		binding_span(exports, worker); 			XX_DEBUG("binding span ok");
		binding_text_node(exports, worker); XX_DEBUG("binding text_node ok");
		binding_image(exports, worker); 		XX_DEBUG("binding image ok");
		binding_indep_div(exports, worker); XX_DEBUG("binding indep_div ok");
		binding_root(exports, worker); 			XX_DEBUG("binding root ok");
		binding_label(exports, worker); 		XX_DEBUG("binding label ok");
		binding_limit(exports, worker); 		XX_DEBUG("binding limit ok");
		binding_scroll(exports, worker); 		XX_DEBUG("binding scroll ok");
		binding_text(exports, worker); 			XX_DEBUG("binding text ok");
		binding_button(exports, worker); 		XX_DEBUG("binding button ok");
		binding_input(exports, worker); 		XX_DEBUG("binding input ok");
		JS_SET_METHOD(lock, lock); 					XX_DEBUG("binding lock ok");
	}
};

JS_REG_MODULE(_ngui, NativeNgui);
JS_END