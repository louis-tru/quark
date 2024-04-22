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
#include "../../views2/sprite.h"

/**
 * @ns qk::js
 */

Js_BEGIN

/**
 * @class WrapSprite
 */
class WrapSprite: public WrapViewBase {
	public:

	static void constructor(FunctionCall args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapSprite>(args, new Sprite());
	}
	// get
	static void src(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( self->src() );
	}
	static void texture(Local<JSString> name, PropertyCall args) {
		// TODO ?
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( worker->NewNull() );
	}
	static void start_x(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( self->start_x() );
	}
	static void start_y(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( self->start_y() );
	}
	static void width(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( self->width() );
	}
	static void height(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( self->height() );
	}
	static void start(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( worker->values()->New(self->start()) );
	}
	// set
	static void set_src(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		Js_Self(Sprite);
		self->set_src(value->ToStringValue(worker));
	}
	static void set_texture(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		// TODO ?
	}

	/**
	 * @set start_x {float}
	 */
	static void set_start_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker)) {
			Js_Throw("* @set startX {float}");
		}
		Js_Self(Sprite);
		self->set_start_x( value->ToNumberValue(worker) );
	}

	/**
	 * @set start_y {float}
	 */
	static void set_start_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker)) {
			Js_Throw("* @set startY {float}");
		}
		Js_Self(Sprite);
		self->set_start_y( value->ToNumberValue(worker) );
	}

	static void set_start(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Vec2, value, "Sprite.start = %s");
		Js_Self(Sprite);
		self->start( out );
	}

	/**
	 * @set width {float}
	 */
	static void set_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker)) {
			Js_Throw("* @set width {float}");
		}
		Js_Self(Sprite);
		self->set_width( value->ToNumberValue(worker) );
	}

	/**
	 * @set height {float}
	 */
	static void set_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker)) {
			Js_Throw("* @set height {float}");
		}
		Js_Self(Sprite);
		self->set_height( value->ToNumberValue(worker) );
	}

	static void ratio_x(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( self->ratio_x() );
	}

	static void ratio_y(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( self->ratio_y() );
	}

	/**
	 * @set ratio_x {float}
	 */
	static void set_ratio_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) {
			Js_Throw("* @set ratioX {float}");
		}
		Js_Self(Sprite);
		self->set_ratio_x( value->ToNumberValue(worker) );
	}

	/**
	 * @set ratio_y {float}
	 */
	static void set_ratio_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) {
			Js_Throw("* @set ratioY {float}");
		}
		Js_Self(Sprite);
		self->set_ratio_y( value->ToNumberValue(worker) );
	}
	static void ratio(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( worker->values()->New(self->ratio()) );
	}
	static void set_ratio(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Vec2, value, "Sprite.ratio = %s");
		Js_Self(Sprite);
		self->set_ratio( out );
	}
	static void repeat(Local<JSString> name, PropertyCall args) {
		Js_Worker(args);
		Js_Self(Sprite);
		Js_Return( worker->values()->New(self->repeat()) );
	}
	static void set_repeat(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Repeat, value, "Sprite.repeat = %s");
		Js_Self(Sprite);
		self->set_repeat( out );
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Define_Class(Sprite, constructor, {
			Js_Set_Class_Accessor(src, src, set_src);
			Js_Set_Class_Accessor(texture, texture, set_texture);
			Js_Set_Class_Accessor(startX, start_x, set_start_x);
			Js_Set_Class_Accessor(startY, start_y, set_start_y);
			Js_Set_Class_Accessor(width, width, set_width);
			Js_Set_Class_Accessor(height, height, set_height);
			Js_Set_Class_Accessor(start, start, set_start);
			Js_Set_Class_Accessor(ratioX, ratio_x, set_ratio_x);
			Js_Set_Class_Accessor(ratioY, ratio_y, set_ratio_y);
			Js_Set_Class_Accessor(ratio, ratio, set_ratio);
			Js_Set_Class_Accessor(repeat, repeat, set_repeat);
		}, View);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(Sprite), View::SPRITE);
	}
};

void binding_sprite(Local<JSObject> exports, Worker* worker) {
	WrapSprite::binding(exports, worker);
}

Js_END
