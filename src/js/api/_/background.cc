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
#include "../str.h"
#include "../../background.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapBackground
 */
class WrapBackground: public WrapObject {
	public:

	static void constructor(FunctionArgs args) {
		Js_Worker(args);
		Js_Throw("Forbidden access abstract");
	}
	
	/**
	 * @get next {Background}
	 */
	static void next(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Background);
		if (self->next()) {
			Js_Return( pack(self->next())->that() );
		} else {
			Js_Return_Null();
		}
	}
	
	/**
	 * @set next {Background}
	 */
	static void set_next(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value2(BackgroundPtr, Background, value, "Background.next = %s");
		Js_Self(Background);
		self->set_next(out);
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(Background, constructor, {
			Js_Set_Class_Accessor(next, next, set_next);
		}, nullptr);
	}
};

class WrapBackgroundImage: public WrapObject {
	public:
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		New<WrapBackgroundImage>(args, new BackgroundImage());
	}
	
	static void src(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(BackgroundImage);
		Js_Return(self->src());
	}
	static void has_base64(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(BackgroundImage);
		Js_Return(self->has_base64());
	}
	static void repeat(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(BackgroundImage);
		Js_Return( worker->values()->New(self->repeat()));
	}
	static void position_x(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(BackgroundImage);
		Js_Return( worker->values()->New(self->position_x()));
	}
	static void position_y(JSString* name, PropertyArgs args){
		Js_Worker(args);
		Js_Self(BackgroundImage);
		Js_Return( worker->values()->New(self->position_y()));
	}
	static void size_x(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(BackgroundImage);
		Js_Return( worker->values()->New(self->size_x()));
	}
	static void size_y(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(BackgroundImage);
		Js_Return( worker->values()->New(self->size_y()));
	}
	static void position(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Return_Null();
	}
	static void size(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Return_Null();
	}
	static void set_src(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		self->set_src( value->toStringValue(worker) );
	}
	// static void set_src_base64(JSString* name, PropertySetArgs args) {
	static void set_src_base64(FunctionArgs args) {
		// Js_Worker(args); UILock lock;
		// Js_Self(BackgroundImage);
		// self->set_src( value->toStringValue(worker) );
	}
	static void set_repeat(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		js_parse_value(Repeat, value, "BackgroundImage.repeat = %s");
		self->set_repeat(out);
	}
	static void set_position_x(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		js_parse_value(BackgroundPosition, value, "BackgroundImage.positionX = %s");
		self->set_position_x(out);
	}
	static void set_position_y(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		js_parse_value(BackgroundPosition, value, "BackgroundImage.positionY = %s");
		self->set_position_y(out);
	}
	static void set_size_x(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		js_parse_value(BackgroundSize, value, "BackgroundImage.sizeX = %s");
		self->set_size_x(out);
	}
	static void set_size_y(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		js_parse_value(BackgroundSize, value, "BackgroundImage.sizeY = %s");
		self->set_size_y(out);
	}
	static void set_position(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		js_parse_value2(BackgroundPositionCollection, BackgroundPositionCollection,
										value, "BackgroundImage.position = %s");
		self->set_position_x(out.x);
		self->set_position_y(out.y);
	}
	static void set_size(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(BackgroundImage);
		js_parse_value2(BackgroundSizeCollection, BackgroundSizeCollection,
										value, "BackgroundImage.size = %s");
		self->set_size_x(out.x);
		self->set_size_y(out.y);
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(BackgroundImage, constructor, {
			// Js_Set_Class_Accessor(setBase64Source, set_src_base64);
			Js_Set_Class_Accessor(src, src, set_src);
			// Js_Set_Class_Accessor(hasBase64, has_base64);
			Js_Set_Class_Accessor(repeat, repeat, set_repeat);
			Js_Set_Class_Accessor(position, position, set_position);
			Js_Set_Class_Accessor(positionX, position_x, set_position_x);
			Js_Set_Class_Accessor(positionY, position_y, set_position_y);
			Js_Set_Class_Accessor(size, size, set_size);
			Js_Set_Class_Accessor(sizeX, size_x, set_size_x);
			Js_Set_Class_Accessor(sizeY, size_y, set_size_y);
		}, Background);
	}
};

class WrapBackgroundGradient: public WrapObject {
	public:
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		New<WrapBackgroundGradient>(args, new BackgroundGradient());
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(BackgroundGradient, constructor, {
			
		}, Background);
	}
};

void binding_background(JSObject* exports, Worker* worker) {
	WrapBackground::binding(exports, worker);
	WrapBackgroundImage::binding(exports, worker);
	// WrapBackgroundGradient::binding(exports, worker);
}

Js_END
