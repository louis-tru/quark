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

#include "shark/js/js-1.h"
#include "shark/js/shark.h"
#include "shark/button.h"
#include "shark/panel.h"
#include "shark/hybrid.h"

/**
 * @ns shark::js
 */

JS_BEGIN

/**
 * @class WrapButton
 */
class WrapButton: public WrapViewBase {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		js_check_gui_app();
		New<WrapButton>(args, new Button());
	}
	
	/**
	 * @func find_next_button(direction)
	 * @arg direction {Direction}
	 * @ret {Button}
	 */
	static void find_next_button(FunctionCall args) {
		GUILock lock;
		
		cchar* argument =
		"* @func findNextButton(direction)\n"
		"* @arg direction {Direction}\n"
		"* @ret {Button}\n"
		;
		
		JS_WORKER(args);
		if ( args.Length() < 1 ) {
			JS_THROW_ERR(argument);
		}
		js_parse_value(Direction, args[0], "Button.findNextButton(%s)");
		
		JS_SELF(Button);
		Button* button = self->find_next_button(out);
		
		if ( button ) {
			JS_RETURN( Wrap<Button>::pack(button, View::BUTTON)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	static void panel(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Button);
		Panel* panel = self->panel();
		if ( panel ) {
			JS_RETURN( Wrap<Panel>::pack(panel, View::PANEL)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Button, constructor, {
			JS_SET_CLASS_METHOD(findNextButton, find_next_button);
			JS_SET_CLASS_METHOD(panel, panel);
		}, Hybrid);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Button), View::BUTTON);
	}
};

void binding_button(Local<JSObject> exports, Worker* worker) {
	WrapButton::binding(exports, worker);
}

JS_END
