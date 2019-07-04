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
#include "langou/image.h"

/**
 * @ns langou::js
 */

JS_BEGIN

/**
 * @class WrapImage
 */
class WrapImage: public WrapViewBase {
 public:

	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapImage>(args, new Image());
	}
	
	static void src(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Image);
		JS_RETURN( self->src() );
	}
	
	static void set_src(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Image);
		String src = value->ToStringValue(worker);
		self->set_src(src);
	}
	
	static void source_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Image);
		JS_RETURN( self->source_width() );
	}
	
	static void source_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Image);
		JS_RETURN( self->source_height() );
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Image, constructor, {
			JS_SET_CLASS_ACCESSOR(src, src, set_src);
			JS_SET_CLASS_ACCESSOR(sourceWidth, source_width);
			JS_SET_CLASS_ACCESSOR(sourceHeight, source_height);
		}, Div);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Image), View::IMAGE);
	}
};

void binding_image(Local<JSObject> exports, Worker* worker) {
	WrapImage::binding(exports, worker);
}

JS_END
