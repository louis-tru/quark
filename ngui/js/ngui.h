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

#ifndef __ngui__js__ngui__
#define __ngui__js__ngui__

#include "js.h"
#include "ngui/app.h"
#include "ngui/value.h"
#include "ngui/view.h"
#include "ngui/bezier.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

using namespace ngui;
using namespace ngui::value;

#define js_check_gui_app() if ( ! app() ) { \
  JS_WORKER(args); JS_THROW_ERR("Need to create a `new GUIApplication()`"); }

#define js_parse_value(Type, value, desc) js_parse_value2(Type, Type, value, desc)
#define js_parse_value2(Type, Name, value, desc) \
  Type out; \
  if ( !worker->value_program()->parse##Name(value, out, desc)) \
  { return; /*JS_THROW_ERR("Bad argument.");*/ }

#define js_throw_value_err(value, msg, ...)\
  worker->value_program()->throwError(t, msg, ##__VA_ARGS__)


// ------------- values -------------

#define js_value_table(F) \
F(TextAlign, TextAlign)   F(Align, Align)             F(ContentAlign, ContentAlign)  \
F(Border, Border)         F(Shadow, ShadowValue)      F(Color, Color) \
F(Vec2, Vec2)             F(Vec3, Vec3)               F(Vec4, Vec4) \
F(Rect, CGRect)           F(Mat, Mat)                 F(Mat4, Mat4) \
F(Value, Value)           F(TextColor, TextColor)     F(TextSize, TextSize)  \
F(TextFamily, TextFamily) F(TextStyle, TextStyle)     F(TextShadow, TextShadow)  \
F(TextLineHeight, TextLineHeight)                     F(TextDecoration, TextDecoration) \
F(Repeat, Repeat)         F(Curve, Curve)             F(Direction, Direction) \
F(String, String)         F(bool, bool)               F(TextOverflow, TextOverflow) \
F(TextWhiteSpace, TextWhiteSpace)                     F(KeyboardType, KeyboardType)  \
F(KeyboardReturnType, KeyboardReturnType)

/**
 * @class ValueProgram
 */
class XX_EXPORT ValueProgram: public Object {
 public:
#define def_attr_fn(Name, Type)           \
  Local<JSValue> New(const Type& value);  \
  bool parse##Name(Local<JSValue> in, Type& out, cchar* err_msg); \
  bool is##Name(Local<JSValue> value);
  
#define def_attr(Name, Type) \
  Persistent<JSFunction> _constructor##Name; \
  Persistent<JSFunction> _parse##Name; \
  Persistent<JSFunction> _parse##Name##Description; \
  Persistent<JSFunction> _##Name;

  ValueProgram(Worker* worker, Local<JSObject> exports, Local<JSObject> native);
  
  virtual ~ValueProgram();
  
  js_value_table(def_attr_fn);
  bool parseValues(Local<JSValue> in, Array<Value>& out, cchar* desc);
  bool parseFloatValues(Local<JSValue> in, Array<float>& out, cchar* desc);
  bool isBase(Local<JSValue> value);
  
  void throwError(Local<JSValue> value, cchar* msg,
                  Local<JSFunction> more_msg = Local<JSFunction>());
 private:
  js_value_table(def_attr)
  Worker* worker;
  Persistent<JSFunction> _BorderRgba;
  Persistent<JSFunction> _ShadowRgba;
  Persistent<JSFunction> _TextColorRgba;
  Persistent<JSFunction> _TextShadowRgba;
  Persistent<JSFunction> _parseValues;
  Persistent<JSFunction> _parseFloatValues;
  Persistent<JSFunction> _isBase;
  #undef def_attr_fn
  #undef def_attr
};

/**
 * @class ViewUtil
 */
class XX_EXPORT ViewUtil {
 public:
  
  /**
   * @func inherit_text_font
   */
  static void inherit_text_font(Local<JSClass> cls, Worker* worker);
  
  /**
   * @func inherit_text_layout
   */
  static void inherit_text_layout(Local<JSClass> cls, Worker* worker);
  
  /**
   * @func inherit_scroll
   */
  static void inherit_scroll(Local<JSClass> cls, Worker* worker);
  
  /**
   * @func add_event_listener
   */
  static bool add_event_listener(Wrap<View>* wrap, cString& name, cString& func, int id);
  
  /**
   * @func remove_event_listener
   */
  static bool remove_event_listener(Wrap<View>* wrap, cString& name, int id);
  
  /**
   * @func add_event_listener
   */
  static bool add_event_listener(Wrap<View>* wrap, const GUIEventName& name, cString& func, int id);

  /**
   * @func remove_event_listener
   */
  static bool remove_event_listener(Wrap<View>* wrap, const GUIEventName& name, int id);
  
  /**
   * @func panel_add_event_listener
   */
  static bool panel_add_event_listener(Wrap<View>* wrap, cString& name, cString& func, int id);
  
  /**
   * @func panel_remove_event_listener
   */
  static bool panel_remove_event_listener(Wrap<View>* wrap, cString& name, int id);
  
};

/**
 * @class WrapViewBase
 */
class XX_EXPORT WrapViewBase: public WrapObject {
 public:

  virtual void destroy();
  
  /**
   * @func overwrite
   */
  virtual bool add_event_listener(cString& name, cString& func, int id) {
    return ViewUtil::add_event_listener(reinterpret_cast<Wrap<View>*>(this), name, func, id);
  }
  
  /**
   * @func overwrite
   */
  virtual bool remove_event_listener(cString& name, int id) {
    return ViewUtil::remove_event_listener(reinterpret_cast<Wrap<View>*>(this), name, id);
  }
  
};

JS_END
#endif
