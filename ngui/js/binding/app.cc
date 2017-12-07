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
#include "ngui/app.h"
#include "ngui/root.h"
#include "ngui/display-port.h"
#include "ngui/draw.h"
#include "ngui/limit.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

using namespace ngui;

static cString load("Load");
static cString unload("Unload");
static cString background("Background");
static cString foreground("Foreground");
static cString pause("Pause");
static cString resume("Resume");
static cString memorywarning("Memorywarning");

typedef GUIApplication NativeGUIApplication;

/**
 * @class WrapNativeGUIApplication
 */
class WrapNativeGUIApplication: public WrapObject {
 public: 
  typedef GUIApplication Type;

  /**
   * @func add_event_listener
   */
  virtual bool add_event_listener(cString& name, cString& func, int id)
  {
    if ( name == load ) {
      self<Type>()->js_bind_common_native_event(load);
    } else if ( name == unload ) {
      self<Type>()->js_bind_common_native_event(unload);
    } else if ( name == background ) {
      self<Type>()->js_bind_common_native_event(background);
    } else if ( name == foreground ) {
      self<Type>()->js_bind_common_native_event(foreground);
    } else if ( name == pause ) {
      self<Type>()->js_bind_common_native_event(pause);
    } else if ( name == resume ) {
      self<Type>()->js_bind_common_native_event(resume);
    } else if ( name == memorywarning ) {
      self<Type>()->js_bind_common_native_event(memorywarning);
    } else {
      return false;
    }
    return true;
  }
  
  /**
   * @func remove_event_listener
   */
  virtual bool remove_event_listener(cString& name, int id)
  {
    if ( name == load ) {
      self<Type>()->js_unbind_native_event(load);
    } else if ( name == unload ) {
      self<Type>()->js_unbind_native_event(unload);
    } else if ( name == background ) {
      self<Type>()->js_unbind_native_event(background);
    } else if ( name == foreground ) {
      self<Type>()->js_unbind_native_event(foreground);
    } else if ( name == pause ) {
      self<Type>()->js_unbind_native_event(pause);
    } else if ( name == resume ) {
      self<Type>()->js_unbind_native_event(resume);
    } else if ( name == memorywarning ) {
      self<Type>()->js_unbind_native_event(memorywarning);
    } else {
      return false;
    }
    return true;
  }
  
  void memorywarning_handle(Event<>& evt) {
    worker()->garbage_collection(); // 清理内存
#if XX_MEMORY_TRACE_MARK
    uint count = Object::mark_objects_count();
    LOG("All unrelease heap objects count: %d", count);
#endif
  }

  /**
   * @constructor([options])
   * @arg [options] {Object} { anisotropic {bool}, mipmap {bool}, multisample {0-4} }
   */
  static void constructor(FunctionCall args) {
    JS_WORKER(args);
    
    Map<String, int> option;
    
    if ( args.Length() > 0 && args[0]->IsObject(worker) ) {
      if (!args[0].To<JSObject>()->ToIntegerMapMaybe(worker).To(option)) 
        return;
    }
    
    Wrap<NativeGUIApplication>* wrap = nullptr;
    try {
      Handle<GUIApplication> h = new GUIApplication();
      h->initialize(option);
      auto app = h.collapse();
      wrap = New<WrapNativeGUIApplication>(args, app);
      app->XX_ON(memorywarning,
                 &WrapNativeGUIApplication::memorywarning_handle,
                 reinterpret_cast<WrapNativeGUIApplication*>(wrap));
      SimpleThread::detach([app](SimpleThread& t) {
        app->run(); // run gui main thread loop
      }, "render_loop");
      
    } catch(cError& err) {
      if ( wrap )
        delete wrap;
      JS_THROW_ERR(err);
    }
  }
  
  /**
   * @func clear() clear gui application resources
   */
  static void clear(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    if ( args.Length() > 1 ) {
      self->clear( args[0]->ToBooleanValue(worker) );
    } else {
      self->clear();
    }
  }
  
  /**
   * @func open_url(url)
   */
  static void open_url(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() == 0 ) {
      JS_THROW_ERR("@func openUrl(url)");
    }
    JS_SELF(GUIApplication);
    self->open_url( args[0]->ToStringValue(worker) );
  }
  
  /**
   * @func send_email(recipient,title[,cc[,bcc[,body]]])
   */
  static void send_email(FunctionCall args) {
    JS_WORKER(args); GUILock lock;
    if ( args.Length() < 2 ) {
      JS_THROW_ERR("@func sendEmail(recipient,title[,cc[,bcc[,body]]])");
    }
    String recipient = args[0]->ToStringValue(worker);
    String title = args[1]->ToStringValue(worker);
    String cc, bcc, body;
    if ( args.Length() > 2 ) cc = args[2]->ToStringValue(worker);
    if ( args.Length() > 3 ) bcc = args[3]->ToStringValue(worker);
    if ( args.Length() > 4 ) body = args[4]->ToStringValue(worker);
    
    JS_SELF(GUIApplication);
    self->send_email( recipient, title, cc, bcc, body );
  }
  
  /**
   * @get is_load {bool}
   */
  static void is_load(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( self->is_load() );
  }
  
  /**
   * @get display_port {DisplayPort}
   */
  static void display_port(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    auto wrap = pack(self->display_port());
    JS_RETURN( wrap->that() );
  }
  
  /**
   * @get root {Root}
   */
  static void root(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    Root* root = self->root();
    if (! root) { // null
      JS_RETURN( worker->NewNull() );
    }
    Wrap<Root>* wrap = pack(root);
    JS_RETURN( wrap->that() );
  }
  
  /**
   * @get focus_view {View}
   */
  static void focus_view(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    View* view = self->focus_view();
    if (! view) { // null
      JS_RETURN_NULL();
    }
    Wrap<View>* wrap = Wrap<View>::pack(view);
    JS_RETURN( wrap->that() );
  }
  
  /**
   * @get default_text_background_color {TextColor}
   */
  static void default_text_background_color(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_background_color()) );
  }
  
  /**
   * @get default_text_color {TextColor}
   */
  static void default_text_color(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_color()) );
  }
  
  /**
   * @get default_text_size {TextSize}
   */
  static void default_text_size(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_size()) );
  }
  
  /**
   * @get default_text_style {TextStyle}
   */
  static void default_text_style(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_style()) );
  }
  
  /**
   * @get default_text_family {TextFamily}
   */
  static void default_text_family(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_family()) );
  }
  
  /**
   * @get default_text_shadow {TextShadow}
   */
  static void default_text_shadow(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_shadow()) );
  }
  
  /**
   * @get default_text_line_height {TextLineHeight}
   */
  static void default_text_line_height(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_line_height()) );
  }
  
  /**
   * @get default_text_decoration {TextDecoration}
   */
  static void default_text_decoration(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_decoration()) );
  }
  
  /**
   * @get default_text_overflow {TextOverflow}
   */
  static void default_text_overflow(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_overflow()) );
  }
  
  /**
   * @get default_text_white_space {TextWhiteSpace}
   */
  static void default_text_white_space(Local<JSString> name, PropertyCall args) {
    JS_WORKER(args);
    JS_SELF(GUIApplication);
    JS_RETURN( worker->value_program()->New(self->default_text_white_space()) );
  }
  
  /**
   * @set default_text_background_color {TextColor}
   */
  static void set_default_text_background_color(Local<JSString> name,
                                                Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextColor, value, "GUIApplication.defaultTextBackgroundColor = %s");
    self->set_default_text_background_color(out);
  }
  
  /**
   * @set default_text_color {TextColor}
   */
  static void set_default_text_color(Local<JSString> name,
                                     Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextColor, value, "GUIApplication.defaultTextColor = %s");
    self->set_default_text_color(out);
  }
  
  /**
   * @set default_text_size {TextSize}
   */
  static void set_default_text_size(Local<JSString> name,
                                    Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextSize, value, "GUIApplication.defaultTextSize = %s");
    self->set_default_text_size(out);
  }
  
  /**
   * @set default_text_style {TextStyle}
   */
  static void set_default_text_style(Local<JSString> name,
                                     Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextStyle, value, "GUIApplication.defaultTextStyle = %s");
    self->set_default_text_style(out);
  }
  
  /**
   * @set default_text_family {TextFamily}
   */
  static void set_default_text_family(Local<JSString> name,
                                      Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextFamily, value, "GUIApplication.defaultTextFamily = %s");
    self->set_default_text_family(out);
  }
  
  /**
   * @set default_text_shadow {TextShadow}
   */
  static void set_default_text_shadow(Local<JSString> name,
                                      Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextShadow, value, "GUIApplication.defaultTextShadow = %s");
    self->set_default_text_shadow(out);
  }
  
  /**
   * @set default_text_line_height {TextLineHeight}
   */
  static void set_default_text_line_height(Local<JSString> name,
                                           Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextLineHeight, value, "GUIApplication.defaultTextLineHeight = %s");
    self->set_default_text_line_height(out);
  }
  
  /**
   * @set default_text_decoration {TextDecoration}
   */
  static void set_default_text_decoration(Local<JSString> name,
                                          Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextDecoration, value, "GUIApplication.defaultTextDecoration = %s");
    self->set_default_text_decoration(out);
  }
  
  /**
   * @set default_text_overflow {TextOverflow}
   */
  static void set_default_text_overflow(Local<JSString> name,
                                        Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextOverflow, value, "GUIApplication.defaultTextOverflow = %s");
    self->set_default_text_overflow(out);
  }
  
  /**
   * @set default_text_white_space {TextWhiteSpace}
   */
  static void set_default_text_white_space(Local<JSString> name,
                                              Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args); GUILock lock;
    JS_SELF(GUIApplication);
    js_parse_value(TextWhiteSpace, value, "GUIApplication.defaultTextWhiteSpace = %s");
    self->set_default_text_white_space(out);
  }
  
  static void binding(Local<JSObject> exports, Worker* worker) {
    JS_DEFINE_CLASS(NativeGUIApplication, constructor, {
      JS_SET_CLASS_METHOD(clear, clear);
      JS_SET_CLASS_METHOD(openUrl, open_url);
      JS_SET_CLASS_METHOD(sendEmail, send_email);
      JS_SET_CLASS_ACCESSOR(isLoad, is_load);
      JS_SET_CLASS_ACCESSOR(displayPort, display_port);
      JS_SET_CLASS_ACCESSOR(root, root);
      JS_SET_CLASS_ACCESSOR(focusView, focus_view);
      JS_SET_CLASS_ACCESSOR(defaultTextBackgroundColor,
                          default_text_background_color, set_default_text_background_color);
      JS_SET_CLASS_ACCESSOR(defaultTextColor, default_text_color, set_default_text_color);
      JS_SET_CLASS_ACCESSOR(defaultTextSize, default_text_size, set_default_text_size);
      JS_SET_CLASS_ACCESSOR(defaultTextStyle, default_text_style, set_default_text_style);
      JS_SET_CLASS_ACCESSOR(defaultTextFamily, default_text_family, set_default_text_family);
      JS_SET_CLASS_ACCESSOR(defaultTextShadow, default_text_shadow, set_default_text_shadow);
      JS_SET_CLASS_ACCESSOR(defaultTextLineHeight, default_text_line_height, set_default_text_line_height);
      JS_SET_CLASS_ACCESSOR(defaultTextDecoration, default_text_decoration, set_default_text_decoration);
      JS_SET_CLASS_ACCESSOR(defaultTextOverflow, default_text_overflow, set_default_text_overflow);
      JS_SET_CLASS_ACCESSOR(defaultTextWhiteSpace, default_text_white_space, set_default_text_white_space);
    }, NULL);
  }
};

void binding_app(Local<JSObject> exports, Worker* worker) {
  WrapNativeGUIApplication::binding(exports, worker);
}

JS_END
