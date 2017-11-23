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
#include "ngui/js/str.h"
#include "ngui/action.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

/**
 * @class WrapAction
 */
class WrapAction: public WrapObject {
 public:

  static void constructor(FunctionCall args) {
    JS_WORKER(args);
    JS_THROW_ERR("Forbidden access abstract");
  }
  
  /**
   * @func play()
   */
  static void play(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    self->play();
  }
  
  /**
   * @func stop()
   */
  static void stop(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    self->stop();
  }
  
  /**
   * @func seek(ms)
   * @arg ms {int}
   */
  static void seek(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
      JS_THROW_ERR(
        "* @func seek(ms)\n"
        "* @arg ms {int}\n"
      );
    }
    JS_SELF(Action);
    self->seek( uint64(1000) * args[0]->ToInt32Value(worker) );
  }
  
  /**
   * @func seek_play(ms)
   * @arg ms {int}
   */
  static void seek_play(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
      JS_THROW_ERR(
        "* @func seekPlay(ms)\n"
        "* @arg ms {int}\n"
      );
    }
    JS_SELF(Action);
    self->seek_play( uint64(1000) * args[0]->ToInt32Value(worker) );
  }
  
  /**
   * @func seek_stop(ms)
   * @arg ms {int}
   */
  static void seek_stop(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
      JS_THROW_ERR(
        "* @func seekStop(ms)\n"
        "* @arg ms {int}\n"
      );
    }
    JS_SELF(Action);
    self->seek_stop( uint64(1000) * args[0]->ToInt32Value(worker) );
  }
  
  /**
   * @func clear()
   */
  static void clear(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    self->clear();
  }
  
  /**
   * @get loop {uint}
   */
  static void loop(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    JS_RETURN( self->loop() );
  }
  
  /**
   * @get loopd {uint}
   */
  static void loopd(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    JS_RETURN( self->loopd() );
  }
  
  /** 
   * @get delay {uint} ms
   */
  static void delay(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    JS_RETURN( self->delay() / 1000 );
  }

  /** 
   * @get delayd {int} ms
   */
  static void delayd(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    JS_RETURN( self->delayd() / 1000 );
  }

  /** 
   * @get speed {float} 0.1-10
   */
  static void speed(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    JS_RETURN( self->speed() );
  }

  /** 
   * @get speed {bool}
   */
  static void playing(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    JS_RETURN( self->playing() );
  }

  /** 
   * @get duration {uint} ms
   */
  static void duration(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    JS_RETURN( self->duration() / 1000 );
  }

  /** 
   * @get parent {Action}
   */
  static void parent(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    Action* action = self->parent();
    if ( action ) {
      Wrap<Action>* wrap = Wrap<Action>::pack(action);
      JS_RETURN( wrap->that() );
    } else {
      JS_RETURN_NULL();
    }
  }

  /**
   * @set playing {bool}
   */
  static void set_playing(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Action);
    self->playing( value->ToBooleanValue(worker) );
  }

  /**
   * @set loop {uint}
   */
  static void set_loop(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( !value->IsUint32(worker) ) {
      JS_THROW_ERR(
        
        "* @set loop {uint}\n"
        
      );
    }
    JS_SELF(Action);
    self->loop( value->ToUint32Value(worker) );
  }

  /**
   * @set delay {uint} ms
   */
  static void set_delay(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( !value->IsUint32(worker) ) {
      JS_THROW_ERR(
        "* @set delay {uint} ms\n"
      );
    }
    JS_SELF(Action);
    self->delay( uint64(1000) * value->ToUint32Value(worker) );
  }

  /**
   * @set speed {float} 0.1-10
   */
  static void set_speed(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( ! value->IsNumber(worker) ) {
      JS_THROW_ERR(
        "* @set speed {float} 0.1-10\n"
      );
    }
    JS_SELF(Action);
    self->speed( value->ToNumberValue(worker) );
  }

  static void null_set_accessor(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
  }

  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS(Action, constructor, {
      JS_SET_CLASS_METHOD(play, play);
      JS_SET_CLASS_METHOD(stop, stop);
      JS_SET_CLASS_METHOD(seek, seek);
      JS_SET_CLASS_METHOD(seekPlay, seek_play);
      JS_SET_CLASS_METHOD(seekStop, seek_stop);
      JS_SET_CLASS_METHOD(clear, clear);
      JS_SET_CLASS_ACCESSOR(duration, duration);
      JS_SET_CLASS_ACCESSOR(parent, parent);
      JS_SET_CLASS_ACCESSOR(playing, playing, set_playing);
      JS_SET_CLASS_ACCESSOR(loop, loop, set_loop);
      JS_SET_CLASS_ACCESSOR(loopd, loopd);
      JS_SET_CLASS_ACCESSOR(delay, delay, set_delay);
      JS_SET_CLASS_ACCESSOR(delayd, delayd);
      JS_SET_CLASS_ACCESSOR(speed, speed, set_speed);
      JS_SET_CLASS_ACCESSOR(seq, nullptr, null_set_accessor);
      JS_SET_CLASS_ACCESSOR(spawn, nullptr, null_set_accessor);
      JS_SET_CLASS_ACCESSOR(keyframe, nullptr, null_set_accessor);
    }, nullptr);
  }
};

/**
 * @class WrapGroupAction
 */
class WrapGroupAction: public WrapObject {
 public:

  static void constructor(FunctionCall args) {
    JS_WORKER(args);
    JS_THROW_ERR("Forbidden access abstract");
  }
  
  /**
   * @get length {uint}
   */
  static void length(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GroupAction);
    JS_RETURN( self->length() );
  }
  
  /**
   * @func append(child)
   * @arg child {Action}
   */
  static void append(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() < 1 || !worker->has_instance<Action>(args[0]) ) {
      JS_THROW_ERR(
        "* @func append(child)\n"
        "* @arg child {Action}\n"
      );
    }
    JS_SELF(GroupAction);
    Action* child = Wrap<Action>::unpack(args[0].To<JSObject>())->self();
    self->append( child );
  }
  
  /**
   * @func insert(index, child)
   * @arg index {uint}
   * @arg child {Action}
   */
  static void insert(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if (args.Length() < 2 || !args[0]->IsUint32(worker) || 
        !worker->has_instance<Action>(args[1]) ) {
      JS_THROW_ERR(
        "* @func insert(index, child)\n"
        "* @arg index {uint}\n"
        "* @arg child {Action}\n"
      );
    }
    JS_SELF(GroupAction);
    Action* child = Wrap<Action>::unpack(args[1].To<JSObject>())->self();
    self->insert( args[0]->ToUint32Value(worker), child );
  }
  
  /**
   * @func remove_child(index)
   * @arg index {uint}
   */
  static void remove_child(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() < 1 || !args[0]->IsUint32(worker) ) {
      JS_THROW_ERR(
        "* @func removeChild(index)\n"
        "* @arg index {uint}\n"
      );
    }
    JS_SELF(GroupAction);
    self->remove_child( args[0]->ToUint32Value(worker) );
  }
  
  static void children(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() < 1 || !args[0]->IsUint32(worker) ) {
      JS_THROW_ERR(
        "* @func children(index)\n"
        "* @arg index {uint}\n"
        "* @ret {Action} return child action\n"
      );
    }
    JS_SELF(GroupAction);
    
    uint index = args[0]->ToUint32Value(worker);
    if ( index < self->length() ) {
      Action* action = (*self)[ args[0]->ToUint32Value(worker) ];
      Wrap<Action>* wrap = Wrap<Action>::pack(action);
      JS_RETURN( wrap->that() );
    } else {
      JS_RETURN_NULL();
    }
  }
  
  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS_NO_EXPORTS(GroupAction, constructor, {
      JS_SET_CLASS_ACCESSOR(length, length);
      JS_SET_CLASS_METHOD(append, append);
      JS_SET_CLASS_METHOD(insert, insert);
      JS_SET_CLASS_METHOD(removeChild, remove_child);
      JS_SET_CLASS_METHOD(children, children);
    }, Action);
  }
};

/**
 * @class WrapSpawnAction
 */
class WrapSpawnAction: public WrapObject {
 public:
  
  static void constructor(FunctionCall args) {
    JS_ATTACH(args);
    js_check_gui_app();
    New<WrapSpawnAction>(args, new SpawnAction());
  }
  
  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS(SpawnAction, constructor, {
    }, GroupAction);
  }
};

/**
 * @class WrapSequenceAction
 */
class WrapSequenceAction: public WrapObject {
 public:
  
  static void constructor(FunctionCall args) {
    JS_ATTACH(args);
    js_check_gui_app();
    New<WrapSequenceAction>(args, new SequenceAction());
  }

  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS(SequenceAction, constructor, {
    }, GroupAction);
  }
};

typedef KeyframeAction::Frame Frame;

// ----------------------------------------------------------------

#define def_get_property(Name) \
static void Name(Local<JSString> name, PropertyCall args) { \
  JS_WORKER(args); \
  JS_SELF(Frame); \
  if (self->host()) JS_RETURN( self->Name() ); \
}
#define def_set_property(Name)\
static void set_##Name(Local<JSString> name, Local<JSValue> value, PropertySetCall args) { \
  JS_WORKER(args); GUILock lock; \
  JS_SELF(Frame); \
  if (self->host()) { \
    if ( ! value->IsNumber(worker) ) { JS_THROW_ERR("Bad argument."); } \
    self->set_##Name( value->ToNumberValue(worker) ); \
  }\
}
#define def_property(Name) \
def_get_property(Name) \
def_set_property(Name)

// ----------------------------------------------------------------

#define def_get_property_from_type(Name)\
static void Name(Local<JSString> name, PropertyCall args) {\
  JS_WORKER(args);\
  JS_SELF(Frame);\
  if (self->host()) JS_RETURN( worker->value_program()->New(self->Name()) );\
}
#define def_set_property_from_type(Name, Type) def_set_property_from_type2(Name, Type, Type)
#define def_set_property_from_type2(Name, Type, parse_func)\
static void set_##Name(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {\
  JS_WORKER(args); GUILock lock;\
  JS_SELF(Frame);\
  if (self->host()) {\
    js_parse_value2(Type, parse_func, value, "Action."#Name" = %s");\
    self->set_##Name(out);\
  }\
}
#define def_property_from_type(Name, Type) def_property_from_type2(Name, Type, Type)
#define def_property_from_type2(Name, Type, parse_func) \
def_get_property_from_type(Name)\
def_set_property_from_type2(Name, Type, parse_func)

/**
 * @class WrapFrame
 */
class WrapFrame: public WrapObject {
 public: 
  typedef Frame Type;

  static void constructor(FunctionCall args) {
    JS_ATTACH(args);
    JS_WORKER(args);
    JS_THROW_ERR("Forbidden access abstract");
  }
  
  /**
   * @func fetch([view]) fetch style attribute by view
   * @arg [view] {View}
   */
  static void fetch(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    View* view = nullptr;
    if ( args.Length() > 0 && worker->has_instance(args[0], View::VIEW) ) {
      view = Wrap<View>::unpack(args[0].To<JSObject>())->self();
    }
    JS_SELF(Frame);
    self->fetch(view);
  }

  /**
   * @func flush() flush frame restore default values
   */
  static void flush(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Frame);
    self->flush();
  }

  /**
   * @get index {uint} frame index in action
   */
  static void index(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Frame);
    JS_RETURN( self->index() );
  }

  /**
   * @get time {uint} ms
   */
  static void time(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Frame);
    JS_RETURN( self->time() / 1000 );
  }

  /**
   * @set time {uint} ms
   */
  static void set_time(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    if ( !value->IsNumber(worker) ) {
      JS_THROW_ERR(
        "* @set time {uint} ms\n"
      );
    }
    JS_SELF(Frame);
    self->set_time(uint64(1000) * value->ToNumberValue(worker));
  }

  /**
   * @get host {KeyframeAction}
   */
  static void host(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Frame);
    KeyframeAction* host = self->host();
    if ( host ) {
      JS_RETURN( Wrap<KeyframeAction>::pack(host)->that() );
    } else {
      JS_RETURN_NULL();
    }
  }
  
  def_property_from_type(curve, Curve)
  
  // -------------------- get/set property --------------------
  
  def_property_from_type(translate, Vec2);
  def_property_from_type(scale, Vec2);
  def_property_from_type(skew, Vec2);
  def_property_from_type(origin, Vec2);
  def_set_property_from_type(margin, Value);
  def_set_property_from_type(border, Border);
  def_set_property(border_width);
  def_set_property_from_type(border_color, Color);
  def_set_property(border_radius);
  def_property_from_type(min_width, Value);
  def_property_from_type(min_height, Value);
  def_property_from_type(start, Vec2);
  def_property_from_type(ratio, Vec2);
  // def_set_property_from_type(align, Align);
  
  // --------------------
  
  static void width(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Frame);
    KeyframeAction* host = self->host();
    if ( host ) {
      if ( host->is_bind_view() ) {
        if ( host->match_property(PROPERTY_WIDTH) ) {
          JS_RETURN( worker->value_program()->New(self->width()) );
        } else {
          JS_RETURN( self->width2() );
        }
      } else {
        if ( host->has_property(PROPERTY_WIDTH) ) {
          JS_RETURN( worker->value_program()->New(self->width()) );
        } else {
          JS_RETURN( self->width2() );
        }
      }
    }
  }
  static void height(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(Frame);
    KeyframeAction* host = self->host();
    if ( host ) {
      if ( host->is_bind_view() ) {
        if ( host->match_property(PROPERTY_HEIGHT) ) {
          JS_RETURN( worker->value_program()->New(self->height()) );
        } else {
          JS_RETURN( self->height2() );
        }
      } else {
        if ( host->has_property(PROPERTY_HEIGHT) ) {
          JS_RETURN( worker->value_program()->New(self->height()) );
        } else {
          JS_RETURN( self->height2() );
        }
      }
    }
  }
  static void set_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Frame);
    KeyframeAction* host = self->host();
    if ( host ) {
      if ( host->is_bind_view() ) {
        if ( host->match_property(PROPERTY_WIDTH) ) {
          js_parse_value(Value, value, "Action.width");
          self->set_width(out);
        } else {
          if ( ! value->IsNumber(worker) ) {
            JS_THROW_ERR("* @set width {float}");
          }
          self->set_width2( value->ToNumberValue(worker) );
        }
      } else {
        js_parse_value(Value, value, "Action.width");
        self->set_width(out);
        if ( out.type == ValueType::PIXEL ) {
          self->set_width2(out.value);
        }
      }
    }
  }
  static void set_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(Frame);
    KeyframeAction* host = self->host();
    if ( host ) {
      if ( host->is_bind_view() ) {
        if ( host->match_property(PROPERTY_HEIGHT) ) {
          js_parse_value(Value, value, "Action.height = %s");
          self->set_height(out);
        } else {
          if ( ! value->IsNumber(worker) ) {
            JS_THROW_ERR("* @set height {float}");
          }
          self->set_height2( value->ToNumberValue(worker) );
        }
      } else {
        js_parse_value(Value, value, "Action.height = %s");
        self->set_height(out);
        if ( out.type == ValueType::PIXEL ) {
          self->set_height2(out.value);
        }
      }
    }
  }
  
  def_property(x);
  def_property(y);
  def_property(scale_x);
  def_property(scale_y);
  def_property(skew_x);
  def_property(skew_y);
  def_property(origin_x);
  def_property(origin_y);
  def_property(rotate_z);
  def_property(opacity);
  def_property_from_type(visible, bool);
  def_property_from_type(margin_left, Value);
  def_property_from_type(margin_top, Value);
  def_property_from_type(margin_right, Value);
  def_property_from_type(margin_bottom, Value);
  def_property_from_type(border_left, Border);
  def_property_from_type(border_top, Border);
  def_property_from_type(border_right, Border);
  def_property_from_type(border_bottom, Border);
  def_property(border_left_width);
  def_property(border_top_width);
  def_property(border_right_width);
  def_property(border_bottom_width);
  def_property_from_type(border_left_color, Color);
  def_property_from_type(border_top_color, Color);
  def_property_from_type(border_right_color, Color);
  def_property_from_type(border_bottom_color, Color);
  def_property(border_radius_left_top);
  def_property(border_radius_right_top);
  def_property(border_radius_right_bottom);
  def_property(border_radius_left_bottom);
  def_property_from_type(background_color, Color);
  def_property_from_type(newline, bool);
  def_property_from_type(content_align, ContentAlign);
  def_property_from_type(text_align, TextAlign);
  def_property_from_type(max_width, Value);
  def_property_from_type(max_height, Value);
  def_property(start_x);
  def_property(start_y);
  def_property(ratio_x);
  def_property(ratio_y);
  def_property_from_type(repeat, Repeat);
  def_property_from_type(text_background_color, TextColor);
  def_property_from_type(text_color, TextColor);
  def_property_from_type(text_size, TextSize);
  def_property_from_type(text_style, TextStyle);
  def_property_from_type(text_family, TextFamily);
  def_property_from_type(text_line_height, TextLineHeight);
  def_property_from_type(text_shadow, TextShadow);
  def_property_from_type(text_decoration, TextDecoration);
  def_property_from_type(text_overflow, TextOverflow);
  def_property_from_type(text_white_space, TextWhiteSpace);
  def_property_from_type(align_x, Align);
  def_property_from_type(align_y, Align);
  def_property_from_type2(shadow, ShadowValue, Shadow);
  def_property_from_type(src, String);
  def_property_from_type(background_image, String);

 public:
  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS(Frame, constructor, {
      JS_SET_CLASS_METHOD(fetch, fetch);
      JS_SET_CLASS_METHOD(flush, flush);
      JS_SET_CLASS_ACCESSOR(index, index);
      JS_SET_CLASS_ACCESSOR(host, host);
      //
      JS_SET_CLASS_ACCESSOR(time, time, set_time);
      JS_SET_CLASS_ACCESSOR(curve, curve, set_curve);
      //
      JS_SET_CLASS_ACCESSOR(translate, translate, set_translate);
      JS_SET_CLASS_ACCESSOR(scale, scale, set_scale);
      JS_SET_CLASS_ACCESSOR(skew, skew, set_skew);
      JS_SET_CLASS_ACCESSOR(origin, origin, set_origin);
      JS_SET_CLASS_ACCESSOR(margin, nullptr, set_margin);
      JS_SET_CLASS_ACCESSOR(border, nullptr, set_border);
      JS_SET_CLASS_ACCESSOR(borderWidth, nullptr, set_border_width);
      JS_SET_CLASS_ACCESSOR(borderColor, nullptr, set_border_color);
      JS_SET_CLASS_ACCESSOR(borderRadius, nullptr, set_border_radius);
      JS_SET_CLASS_ACCESSOR(minWidth, min_width, set_min_width);
      JS_SET_CLASS_ACCESSOR(minHeight, min_height, set_min_height);
      JS_SET_CLASS_ACCESSOR(start, start, set_start);
      JS_SET_CLASS_ACCESSOR(ratio, ratio, set_ratio);
      // JS_SET_CLASS_ACCESSOR(align, nullptr, set_align);
      // style property
      JS_SET_CLASS_ACCESSOR(x, x, set_x);
      JS_SET_CLASS_ACCESSOR(y, y, set_y);
      JS_SET_CLASS_ACCESSOR(scaleX, scale_x, set_scale_x);
      JS_SET_CLASS_ACCESSOR(scaleY, scale_y, set_scale_y);
      JS_SET_CLASS_ACCESSOR(skewX, skew_x, set_skew_x);
      JS_SET_CLASS_ACCESSOR(skewY, skew_y, set_skew_y);
      JS_SET_CLASS_ACCESSOR(originX, origin_x, set_origin_x);
      JS_SET_CLASS_ACCESSOR(originY, origin_y, set_origin_y);
      JS_SET_CLASS_ACCESSOR(rotateZ, rotate_z, set_rotate_z);
      JS_SET_CLASS_ACCESSOR(opacity, opacity, set_opacity);
      JS_SET_CLASS_ACCESSOR(visible, visible, set_visible);
      JS_SET_CLASS_ACCESSOR(width, width, set_width);
      JS_SET_CLASS_ACCESSOR(height, height, set_height);
      JS_SET_CLASS_ACCESSOR(marginLeft, margin_left, set_margin_left);
      JS_SET_CLASS_ACCESSOR(marginTop, margin_top, set_margin_top);
      JS_SET_CLASS_ACCESSOR(marginRight, margin_right, set_margin_right);
      JS_SET_CLASS_ACCESSOR(marginBottom, margin_bottom, set_margin_bottom);
      JS_SET_CLASS_ACCESSOR(borderLeft, border_left, set_border_left);
      JS_SET_CLASS_ACCESSOR(borderTop, border_top, set_border_top);
      JS_SET_CLASS_ACCESSOR(borderRight, border_right, set_border_right);
      JS_SET_CLASS_ACCESSOR(borderBottom, border_bottom, set_border_bottom);
      JS_SET_CLASS_ACCESSOR(borderLeftWidth, border_left_width, set_border_left_width);
      JS_SET_CLASS_ACCESSOR(borderTopWidth, border_top_width, set_border_top_width);
      JS_SET_CLASS_ACCESSOR(borderRightWidth, border_right_width, set_border_right_width);
      JS_SET_CLASS_ACCESSOR(borderBottomWidth, border_bottom_width, set_border_bottom_width);
      JS_SET_CLASS_ACCESSOR(borderLeftColor, border_left_color, set_border_left_color);
      JS_SET_CLASS_ACCESSOR(borderTopColor, border_top_color, set_border_top_color);
      JS_SET_CLASS_ACCESSOR(borderRightColor, border_right_color, set_border_right_color);
      JS_SET_CLASS_ACCESSOR(borderBottomColor, border_bottom_color, set_border_bottom_color);
      JS_SET_CLASS_ACCESSOR(borderRadiusLeftTop, border_radius_left_top, set_border_radius_left_top);
      JS_SET_CLASS_ACCESSOR(borderRadiusRightTop, border_radius_right_top, set_border_radius_right_top);
      JS_SET_CLASS_ACCESSOR(borderRadiusRightBottom, border_radius_right_bottom, set_border_radius_right_bottom);
      JS_SET_CLASS_ACCESSOR(borderRadiusLeftBottom, border_radius_left_bottom, set_border_radius_left_bottom);
      JS_SET_CLASS_ACCESSOR(backgroundColor, background_color, set_background_color);
      JS_SET_CLASS_ACCESSOR(newline, newline, set_newline);
      JS_SET_CLASS_ACCESSOR(contentAlign, content_align, set_content_align);
      JS_SET_CLASS_ACCESSOR(textAlign, text_align, set_text_align);
      JS_SET_CLASS_ACCESSOR(maxWidth, max_width, set_max_width);
      JS_SET_CLASS_ACCESSOR(maxHeight, max_height, set_max_height);
      JS_SET_CLASS_ACCESSOR(startX, start_x, set_start_x);
      JS_SET_CLASS_ACCESSOR(startY, start_y, set_start_y);
      JS_SET_CLASS_ACCESSOR(ratioX, ratio_x, set_ratio_x);
      JS_SET_CLASS_ACCESSOR(ratioY, ratio_y, set_ratio_y);
      JS_SET_CLASS_ACCESSOR(repeat, repeat, set_repeat);
      JS_SET_CLASS_ACCESSOR(textBackgroundColor, text_background_color, set_text_background_color);
      JS_SET_CLASS_ACCESSOR(textColor, text_color, set_text_color);
      JS_SET_CLASS_ACCESSOR(textSize, text_size, set_text_size);
      JS_SET_CLASS_ACCESSOR(textStyle, text_style, set_text_style);
      JS_SET_CLASS_ACCESSOR(textFamily, text_family, set_text_family);
      JS_SET_CLASS_ACCESSOR(textLineHeight, text_line_height, set_text_line_height);
      JS_SET_CLASS_ACCESSOR(textShadow, text_shadow, set_text_shadow);
      JS_SET_CLASS_ACCESSOR(textDecoration, text_decoration, set_text_decoration);
      JS_SET_CLASS_ACCESSOR(textOverflow, text_overflow, set_text_overflow);
      JS_SET_CLASS_ACCESSOR(textWhiteSpace, text_white_space, set_text_white_space);
      JS_SET_CLASS_ACCESSOR(alignX, align_x, set_align_x);
      JS_SET_CLASS_ACCESSOR(alignY, align_y, set_align_y);
      JS_SET_CLASS_ACCESSOR(shadow, shadow, set_shadow);
      JS_SET_CLASS_ACCESSOR(src, src, set_src);
      JS_SET_CLASS_ACCESSOR(backgroundImage, background_image, set_background_image);
      // style property end
    }, nullptr);
  }
};

/**
 * @class WrapKeyframeAction
 */
class WrapKeyframeAction: public WrapObject {
 public:
  
  static void constructor(FunctionCall args) {
    JS_ATTACH(args);
    js_check_gui_app();
    New<WrapKeyframeAction>(args, new KeyframeAction());
  }
  
  /**
   * @func hasProperty(name)
   * @arg name {emun PropertyName} 
   * @ret {bool}
   */
  static void hasProperty(FunctionCall args) {
    JS_WORKER(args);
    if ( args.Length() < 1 || !args[0]->IsInt32(worker) ) {
      JS_THROW_ERR(
        "* @func hasProperty(name)\n"
        "* @arg name {emun PropertyName}\n"
        "* @ret {bool}\n"
      );
    }
    JS_SELF(KeyframeAction);
    JS_RETURN( self->has_property( static_cast<PropertyName>(args[0]->ToInt32Value(worker)) ));
  }
  
  /**
   * @func matchProperty(name)
   * @arg name {emun PropertyName} 
   * @ret {bool}
   */
  static void matchProperty(FunctionCall args) {
    JS_WORKER(args);
    if ( args.Length() < 1 || ! args[0]->IsInt32(worker) ) {
      JS_THROW_ERR(
        "* @func matchProperty(name)\n"
        "* @arg name {emun PropertyName}\n"
        "* @ret {bool}\n"
      );
    }
    JS_SELF(KeyframeAction);
    JS_RETURN( self->match_property( static_cast<PropertyName>(args[0]->ToInt32Value(worker)) ));
  }
  
  /**
   * @func frame(index)
   * @arg index {uint}
   * @ret {Frame}
   */
  static void frame(FunctionCall args) {
    JS_WORKER(args);
    if ( args.Length() < 1 || ! args[0]->IsUint32(worker) ) {
      JS_THROW_ERR("Bad argument.");
    }
    JS_SELF(KeyframeAction);
    uint index = args[0]->ToUint32Value(worker);
    if ( index < self->length() ) {
      Frame* frame = self->frame(index);
      JS_RETURN( Wrap<Frame>::pack(frame)->that() );
    } else {
      JS_RETURN_NULL();
    }
  }
  
  /**
   * @func add([time[,curve]][style])
   * arg [time=0] {uint}
   * arg [curve] {Curve}
   * arg [style] {Object}
   * @ret {Frame}
   */
  static void add(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    uint64 time = 0;
    
    if ( args.Length() > 0 ) {
      if ( args[0]->IsObject(worker) && ! args[0]->IsNull(worker) ) {
        JS_HANDLE_SCOPE();
        
        Local<JSObject> arg = args[0].To<JSObject>();
        Local<JSArray> names = arg->GetPropertyNames(worker);
        Local<JSValue> t = arg->Get(worker, worker->strs()->time());
        if ( !t.IsEmpty() ) {
          if ( t->IsNumber(worker) ) {
            time = uint64(1000) * t->ToNumberValue(worker);
          } else {
            js_throw_value_err(t, "KeyframeAction.add([time = %s])");
          }
        } else { // js error
          return;
        }
        JS_SELF(KeyframeAction);
        Frame* frame = self->add(time);
        Local<JSObject> handle = Wrap<Frame>::pack(frame, JS_TYPEID(Frame))->that();
        
        for ( uint i = 0, len = names->Length(worker); i < len; i++ ) {
          Local<JSValue> key = names->Get(worker, i);
          Local<JSValue> val = arg->Get(worker, key );
          if ( ! handle->Set(worker, key, val) ) { // js error
            return;
          }
        }
        
        JS_RETURN( handle );
      } else if ( args[0]->IsNumber(worker) ) {
        time = uint64(1000) * args[0]->ToNumberValue(worker);
      }
    }
    
    JS_SELF(KeyframeAction);
    
    Frame* frame = nullptr;
    if ( args.Length() > 1 && !args[1]->IsUndefined(worker) ) {
      js_parse_value(Curve, args[1], "KeyframeAction.add([time[,curve = %s]])");
      frame = self->add(time, out);
    } else {
      frame = self->add(time);
    }
    
    Wrap<Frame>* wrap = Wrap<Frame>::pack(frame, JS_TYPEID(Frame));
    JS_RETURN( wrap->that() );
  }
  
  /**
   * @get first {Frame}
   */
  static void first(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(KeyframeAction);
    if ( self->length() ) {
      Frame* frame = self->first();
      JS_RETURN( Wrap<Frame>::pack(frame)->that() );
    } else {
      JS_RETURN_NULL();
    }
  }
  
  /**
   * @get last {Frame}
   */
  static void last(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(KeyframeAction);
    if ( self->length() ) {
      Frame* frame = self->last();
      JS_RETURN( Wrap<Frame>::pack(frame)->that() );
    } else {
      JS_RETURN_NULL();
    }
  }
  
  /**
   * @get length {uint}
   */
  static void length(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(KeyframeAction);
    JS_RETURN( self->length() );
  }
  
  /**
   * @get position {uint} get play frame position
   */
  static void position(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(KeyframeAction);
    JS_RETURN( self->position() );
  }
  
  /**
   * @get time {uint} ms get play time position
   */
  static void time(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(KeyframeAction);
    JS_RETURN( self->time() / 1000 );
  }
  
 public:
  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS(KeyframeAction, constructor, {
      JS_SET_CLASS_METHOD(hasProperty, hasProperty);
      JS_SET_CLASS_METHOD(matchProperty, matchProperty);
      JS_SET_CLASS_METHOD(frame, frame);
      JS_SET_CLASS_METHOD(add, add);
      JS_SET_CLASS_ACCESSOR(first, first);
      JS_SET_CLASS_ACCESSOR(last, last);
      JS_SET_CLASS_ACCESSOR(length, length);
      JS_SET_CLASS_ACCESSOR(position, position);
      JS_SET_CLASS_ACCESSOR(time, time);
    }, Action);
  }
};

/**
 * @class NativeAction
 */
class BindingAction {
 public:
  static void binding(Local<JSObject> exports, Worker* worker) {
    worker->binding_module("ngui_value");
    
    // CubicBezier const
    JS_SET_PROPERTY(LINEAR, 0);
    JS_SET_PROPERTY(EASE, 1);
    JS_SET_PROPERTY(EASE_IN, 2);
    JS_SET_PROPERTY(EASE_OUT, 3);
    JS_SET_PROPERTY(EASE_IN_OUT, 4);
    
    WrapAction::binding(exports, worker);
    WrapGroupAction::binding(exports, worker);
    WrapSpawnAction::binding(exports, worker);
    WrapSequenceAction::binding(exports, worker);
    WrapKeyframeAction::binding(exports, worker);
    WrapFrame::binding(exports, worker);
  }
};

JS_REG_MODULE(ngui_action, BindingAction);
JS_END
