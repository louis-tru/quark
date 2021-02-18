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
#include "../str.h"
#include "../../background.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

/**
 * @class WrapBackground
 */
class WrapBackground: public WrapObject {
 public:

	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		JS_THROW_ERR("Forbidden access abstract");
	}
	
	/**
	 * @get next {Background}
	 */
	static void next(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Background);
		if (self->next()) {
			JS_RETURN( pack(self->next())->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	/**
	 * @set next {Background}
	 */
	static void set_next(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value2(BackgroundPtr, Background, value, "Background.next = %s");
		JS_SELF(Background);
		self->set_next(out);
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Background, constructor, {
			JS_SET_CLASS_ACCESSOR(next, next, set_next);
		}, nullptr);
	}
};

class WrapBackgroundImage: public WrapObject {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		New<WrapBackgroundImage>(args, new BackgroundImage());
	}
	
	static void src(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(BackgroundImage);
		JS_RETURN(self->src());
	}
	static void has_base64(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(BackgroundImage);
		JS_RETURN(self->has_base64());
	}
	static void repeat(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(BackgroundImage);
		JS_RETURN( worker->values()->New(self->repeat()));
	}
	static void position_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(BackgroundImage);
		JS_RETURN( worker->values()->New(self->position_x()));
	}
	static void position_y(Local<JSString> name, PropertyCall args){
		JS_WORKER(args);
		JS_SELF(BackgroundImage);
		JS_RETURN( worker->values()->New(self->position_y()));
	}
	static void size_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(BackgroundImage);
		JS_RETURN( worker->values()->New(self->size_x()));
	}
	static void size_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(BackgroundImage);
		JS_RETURN( worker->values()->New(self->size_y()));
	}
	static void position(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_RETURN_NULL();
	}
	static void size(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_RETURN_NULL();
	}
	static void set_src(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		self->set_src( value->ToStringValue(worker) );
	}
	// static void set_src_base64(Local<JSString> name, PropertySetCall args) {
	static void set_src_base64(FunctionCall args) {
		// JS_WORKER(args); GUILock lock;
		// JS_SELF(BackgroundImage);
		// self->set_src( value->ToStringValue(worker) );
	}
	static void set_repeat(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		js_parse_value(Repeat, value, "BackgroundImage.repeat = %s");
		self->set_repeat(out);
	}
	static void set_position_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		js_parse_value(BackgroundPosition, value, "BackgroundImage.positionX = %s");
		self->set_position_x(out);
	}
	static void set_position_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		js_parse_value(BackgroundPosition, value, "BackgroundImage.positionY = %s");
		self->set_position_y(out);
	}
	static void set_size_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		js_parse_value(BackgroundSize, value, "BackgroundImage.sizeX = %s");
		self->set_size_x(out);
	}
	static void set_size_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		js_parse_value(BackgroundSize, value, "BackgroundImage.sizeY = %s");
		self->set_size_y(out);
	}
	static void set_position(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		js_parse_value2(BackgroundPositionCollection, BackgroundPositionCollection,
										value, "BackgroundImage.position = %s");
		self->set_position_x(out.x);
		self->set_position_y(out.y);
	}
	static void set_size(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(BackgroundImage);
		js_parse_value2(BackgroundSizeCollection, BackgroundSizeCollection,
										value, "BackgroundImage.size = %s");
		self->set_size_x(out.x);
		self->set_size_y(out.y);
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(BackgroundImage, constructor, {
			// JS_SET_CLASS_ACCESSOR(setBase64Source, set_src_base64);
			JS_SET_CLASS_ACCESSOR(src, src, set_src);
			// JS_SET_CLASS_ACCESSOR(hasBase64, has_base64);
			JS_SET_CLASS_ACCESSOR(repeat, repeat, set_repeat);
			JS_SET_CLASS_ACCESSOR(position, position, set_position);
			JS_SET_CLASS_ACCESSOR(positionX, position_x, set_position_x);
			JS_SET_CLASS_ACCESSOR(positionY, position_y, set_position_y);
			JS_SET_CLASS_ACCESSOR(size, size, set_size);
			JS_SET_CLASS_ACCESSOR(sizeX, size_x, set_size_x);
			JS_SET_CLASS_ACCESSOR(sizeY, size_y, set_size_y);
		}, Background);
	}
};

class WrapBackgroundGradient: public WrapObject {
 public:
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		New<WrapBackgroundGradient>(args, new BackgroundGradient());
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(BackgroundGradient, constructor, {
			
		}, Background);
	}
};

void binding_background(Local<JSObject> exports, Worker* worker) {
	WrapBackground::binding(exports, worker);
	WrapBackgroundImage::binding(exports, worker);
	// WrapBackgroundGradient::binding(exports, worker);
}

JS_END
