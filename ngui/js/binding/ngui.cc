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

void WrapViewBase::destroy() {
  GUILock lock;
  delete this;
}

void binding_app(Local<JSObject> exports, Worker* worker);
void binding_display(Local<JSObject> exports, Worker* worker);
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
void binding_select_panel(Local<JSObject> exports, Worker* worker);
void binding_scroll(Local<JSObject> exports, Worker* worker);
void binding_text(Local<JSObject> exports, Worker* worker);
void binding_clip(Local<JSObject> exports, Worker* worker);
void binding_input(Local<JSObject> exports, Worker* worker);

/**
 * @class NativeNGUI
 */
class NativeNGUI {
  
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
  
 public:
  static void binding(Local<JSObject> exports, Worker* worker) {
    worker->binding_module("ngui_value");
    worker->binding_module("ngui_event");
    binding_app(exports, worker);
    binding_display(exports, worker);
    binding_view(exports, worker);
    binding_sprite(exports, worker);
    binding_layout(exports, worker);
    binding_box(exports, worker);
    binding_div(exports, worker);
    binding_select_panel(exports, worker);
    binding_hybrid(exports, worker);
    binding_span(exports, worker);
    binding_text_node(exports, worker);
    binding_image(exports, worker);
    binding_indep_div(exports, worker);
    binding_root(exports, worker);
    binding_label(exports, worker);
    binding_limit(exports, worker);
    binding_scroll(exports, worker);
    binding_text(exports, worker);
    binding_button(exports, worker);
    binding_clip(exports, worker);
    binding_input(exports, worker);
    JS_SET_METHOD(lock, lock);
  }
};

JS_REG_MODULE(ngui, NativeNGUI);
JS_END
