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

#include "ngui/js/js-1.h"
#include "ngui/js/ngui.h"
#include "ngui/sprite.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

/**
 * @class WrapSprite
 */
class WrapSprite: public WrapViewBase {

  static void constructor(FunctionCall args) {
    JS_ATTACH(args);
    js_check_gui_app();
    New<WrapSprite>(args, new Sprite());
  }
  // get
  static void src(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( self->src() );
  }
  static void texture(Local<JSString> name, PropertyCall args) {
    // TODO ?
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( worker->NewNull() );
  }
  static void start_x(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( self->start_x() );
  }
  static void start_y(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( self->start_y() );
  }
  static void width(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( self->width() );
  }
  static void height(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( self->height() );
  }
  static void start(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( worker->value_program()->New(self->start()) );
  }
  // set
  static void set_src(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Sprite);
    self->set_src(value->ToStringValue(worker));
  }
  static void set_texture(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    // TODO ?
  }

  /**
   * @set start_x {float}
   */
  static void set_start_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( ! value->IsNumber(worker)) {
      JS_THROW_ERR("* @set startX {float}");
    }
    JS_SELF(Sprite);
    self->set_start_x( value->ToNumberValue(worker) );
  }

  /**
   * @set start_y {float}
   */
  static void set_start_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( ! value->IsNumber(worker)) {
      JS_THROW_ERR("* @set startY {float}");
    }
    JS_SELF(Sprite);
    self->set_start_y( value->ToNumberValue(worker) );
  }

  static void set_start(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    js_parse_value(Vec2, value, "Sprite.start = %s");
    JS_SELF(Sprite);
    self->start( out );
  }

  /**
   * @set width {float}
   */
  static void set_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( ! value->IsNumber(worker)) {
      JS_THROW_ERR("* @set width {float}");
    }
    JS_SELF(Sprite);
    self->set_width( value->ToNumberValue(worker) );
  }

  /**
   * @set height {float}
   */
  static void set_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( ! value->IsNumber(worker)) {
      JS_THROW_ERR("* @set height {float}");
    }
    JS_SELF(Sprite);
    self->set_height( value->ToNumberValue(worker) );
  }

  static void ratio_x(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( self->ratio_x() );
  }

  static void ratio_y(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( self->ratio_y() );
  }

  /**
   * @set ratio_x {float}
   */
  static void set_ratio_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( ! value->IsNumber(worker) ) {
      JS_THROW_ERR("* @set ratioX {float}");
    }
    JS_SELF(Sprite);
    self->set_ratio_x( value->ToNumberValue(worker) );
  }

  /**
   * @set ratio_y {float}
   */
  static void set_ratio_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( ! value->IsNumber(worker) ) {
      JS_THROW_ERR("* @set ratioY {float}");
    }
    JS_SELF(Sprite);
    self->set_ratio_y( value->ToNumberValue(worker) );
  }
  static void ratio(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( worker->value_program()->New(self->ratio()) );
  }
  static void set_ratio(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    js_parse_value(Vec2, value, "Sprite.ratio = %s");
    JS_SELF(Sprite);
    self->set_ratio( out );
  }
  static void repeat(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Sprite);
    JS_RETURN( worker->value_program()->New(self->repeat()) );
  }
  static void set_repeat(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    js_parse_value(Repeat, value, "Sprite.repeat = %s");
    JS_SELF(Sprite);
    self->set_repeat( out );
  }

 public:
  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS(Sprite, constructor, {
      JS_SET_CLASS_ACCESSOR(src, src, set_src);
      JS_SET_CLASS_ACCESSOR(texture, texture, set_texture);
      JS_SET_CLASS_ACCESSOR(startX, start_x, set_start_x);
      JS_SET_CLASS_ACCESSOR(startY, start_y, set_start_y);
      JS_SET_CLASS_ACCESSOR(width, width, set_width);
      JS_SET_CLASS_ACCESSOR(height, height, set_height);
      JS_SET_CLASS_ACCESSOR(start, start, set_start);
      JS_SET_CLASS_ACCESSOR(ratioX, ratio_x, set_ratio_x);
      JS_SET_CLASS_ACCESSOR(ratioY, ratio_y, set_ratio_y);
      JS_SET_CLASS_ACCESSOR(ratio, ratio, set_ratio);
      JS_SET_CLASS_ACCESSOR(repeat, repeat, set_repeat);
    }, View);
    IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Sprite), View::SPRITE);
  }
};

void binding_sprite(Local<JSObject> exports, Worker* worker) {
  WrapSprite::binding(exports, worker);
}

JS_END
