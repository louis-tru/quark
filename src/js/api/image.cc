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
#include "../../views2/image.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapImage
 */
class WrapImage: public WrapViewBase {
	public:

	static void constructor(FunctionCall args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapImage>(args, new Image());
	}
	
	static void src(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Image);
		Js_Return( self->src() );
	}
	
	static void set_src(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Image);
		String src = value->ToStringValue(worker);
		self->set_src(src);
	}
	
	static void source_width(Local<JSString> name, PropertyCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Image);
		Js_Return( self->source_width() );
	}
	
	static void source_height(Local<JSString> name, PropertyCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Image);
		Js_Return( self->source_height() );
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Define_Class(Image, constructor, {
			Js_Set_Class_Accessor(src, src, set_src);
			Js_Set_Class_Accessor(sourceWidth, source_width);
			Js_Set_Class_Accessor(sourceHeight, source_height);
		}, Div);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(Image), View::IMAGE);
	}
};

void binding_image(Local<JSObject> exports, Worker* worker) {
	WrapImage::binding(exports, worker);
}

Js_END
