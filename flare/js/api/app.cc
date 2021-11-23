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

#include "../_js.h"
#include "./_view.h"
#include "../../app.h"
#include "../../views2/root.h"
#include "../../display-port.h"
#include "../../draw.h"
#include "../../views2/limit.h"

/**
 * @ns flare::js
 */

JS_BEGIN

using namespace flare;

static cString load("Load");
static cString unload("Unload");
static cString background("Background");
static cString foreground("Foreground");
static cString pause("Pause");
static cString resume("Resume");
static cString memorywarning("MemoryWarning");

typedef Application NativeApplication;

/**
 * @class WrapNativeApplication
 */
class WrapNativeApplication: public WrapObject {
	public: 
	typedef Application Type;

	/**
	 * @func addEventListener
	 */
	virtual bool addEventListener(cString& name, cString& func, int id)
	{
		if ( name == load ) {
			self<Type>()->js_bind_common_native_event(Load);
		} else if ( name == unload ) {
			self<Type>()->js_bind_common_native_event(Unload);
		} else if ( name == background ) {
			self<Type>()->js_bind_common_native_event(Background);
		} else if ( name == foreground ) {
			self<Type>()->js_bind_common_native_event(Foreground);
		} else if ( name == pause ) {
			self<Type>()->js_bind_common_native_event(Pause);
		} else if ( name == resume ) {
			self<Type>()->js_bind_common_native_event(Resume);
		} else if ( name == memorywarning ) {
			self<Type>()->js_bind_common_native_event(Memorywarning);
		} else {
			return false;
		}
		return true;
	}
	
	/**
	 * @func removeEventListener
	 */
	virtual bool removeEventListener(cString& name, int id)
	{
		if ( name == load ) {
			self<Type>()->js_unbind_native_event(Load);
		} else if ( name == unload ) {
			self<Type>()->js_unbind_native_event(Unload);
		} else if ( name == background ) {
			self<Type>()->js_unbind_native_event(Background);
		} else if ( name == foreground ) {
			self<Type>()->js_unbind_native_event(Foreground);
		} else if ( name == pause ) {
			self<Type>()->js_unbind_native_event(Pause);
		} else if ( name == resume ) {
			self<Type>()->js_unbind_native_event(Resume);
		} else if ( name == memorywarning ) {
			self<Type>()->js_unbind_native_event(Memorywarning);
		} else {
			return false;
		}
		return true;
	}
	
	void memorywarning_handle(Event<>& evt) {
		worker()->garbageCollection(); // 清理内存
		#if F_MEMORY_TRACE_MARK
			uint32_t count = Object::mark_objects_count();
			F_LOG("App", "All unrelease heap objects count: %d", count);
		#endif
	}

	/**
	 * @constructor([options])
	 * @arg [options] {Object} { anisotropic {bool}, mipmap {bool}, multisample {0-4} }
	 */
	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		
		JSON options = JSON::object();
		if ( args.Length() > 0 && args[0]->IsObject(worker) ) {
			if (!args[0].To<JSObject>()->ToJSON(worker).To(options))
				return;
		}
		
		Wrap<NativeApplication>* wrap = nullptr;
		try {
			Handle<Application> h = new Application();
			h->initialize(options);
			auto app = h.collapse();
			app->F_On(Memorywarning,
								 &WrapNativeApplication::memorywarning_handle,
								 reinterpret_cast<WrapNativeApplication*>(wrap));
			app->run_loop_detach(); // run gui loop
			wrap = New<WrapNativeApplication>(args, app);
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
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
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
		JS_WORKER(args); UILock lock;
		if ( args.Length() == 0 ) {
			JS_THROW_ERR("@func openUrl(url)");
		}
		JS_SELF(Application);
		self->open_url( args[0]->ToStringValue(worker) );
	}
	
	/**
	 * @func send_email(recipient,title,body[,cc[,bcc]])
	 */
	static void send_email(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		if ( args.Length() < 2 ) {
			JS_THROW_ERR("@func sendEmail(recipient,title,body[,cc[,bcc])");
		}
		String recipient = args[0]->ToStringValue(worker);
		String title = args[1]->ToStringValue(worker);
		String body, cc, bcc;
		if ( args.Length() > 2 ) cc = args[2]->ToStringValue(worker);
		if ( args.Length() > 3 ) cc = args[3]->ToStringValue(worker);
		if ( args.Length() > 4 ) bcc = args[4]->ToStringValue(worker);
		
		JS_SELF(Application);
		self->send_email( recipient, title, cc, bcc, body );
	}
	
	static void max_texture_memory_limit(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN(self->max_texture_memory_limit());
	}

	static void set_max_texture_memory_limit(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		if ( args.Length() == 0 && !args[0]->IsNumber() ) {
			JS_THROW_ERR("@func setMaxTextureMemoryLimit(limit)");
		}
		JS_SELF(Application);
		self->set_max_texture_memory_limit( args[0]->ToNumberValue(worker) );
	}
	
	static void used_texture_memory(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN(self->used_texture_memory());
	}
	
	/**
	 * @func pending()
	 */
	static void pending(FunctionCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		self->pending();
	}

	/**
	 * @get is_loaded {bool}
	 */
	static void is_loaded(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( self->is_loaded() );
	}
	
	/**
	 * @get display_port {Display}
	 */
	static void display_port(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		auto wrap = pack(self->display_port());
		JS_RETURN( wrap->that() );
	}
	
	/**
	 * @get root {Root}
	 */
	static void root(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
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
		JS_SELF(Application);
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
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_background_color()) );
	}
	
	/**
	 * @get default_text_color {TextColor}
	 */
	static void default_text_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_color()) );
	}
	
	/**
	 * @get default_text_size {TextSize}
	 */
	static void default_text_size(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_size()) );
	}
	
	/**
	 * @get default_text_style {TextStyle}
	 */
	static void default_text_style(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_style()) );
	}
	
	/**
	 * @get default_text_family {TextFamily}
	 */
	static void default_text_family(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_family()) );
	}
	
	/**
	 * @get default_text_shadow {TextShadow}
	 */
	static void default_text_shadow(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_shadow()) );
	}
	
	/**
	 * @get default_text_line_height {TextLineHeight}
	 */
	static void default_text_line_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_line_height()) );
	}
	
	/**
	 * @get default_text_decoration {TextDecoration}
	 */
	static void default_text_decoration(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_decoration()) );
	}
	
	/**
	 * @get default_text_overflow {TextOverflow}
	 */
	static void default_text_overflow(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_overflow()) );
	}
	
	/**
	 * @get default_text_white_space {TextWhiteSpace}
	 */
	static void default_text_white_space(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Application);
		JS_RETURN( worker->values()->New(self->default_text_white_space()) );
	}
	
	/**
	 * @set default_text_background_color {TextColor}
	 */
	static void set_default_text_background_color(Local<JSString> name,
																								Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextColor, value, "Application.defaultTextBackgroundColor = %s");
		self->set_default_text_background_color(out);
	}
	
	/**
	 * @set default_text_color {TextColor}
	 */
	static void set_default_text_color(Local<JSString> name,
																		 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextColor, value, "Application.defaultTextColor = %s");
		self->set_default_text_color(out);
	}
	
	/**
	 * @set default_text_size {TextSize}
	 */
	static void set_default_text_size(Local<JSString> name,
																		Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextSize, value, "Application.defaultTextSize = %s");
		self->set_default_text_size(out);
	}
	
	/**
	 * @set default_text_style {TextStyle}
	 */
	static void set_default_text_style(Local<JSString> name,
																		 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextStyle, value, "Application.defaultTextStyle = %s");
		self->set_default_text_style(out);
	}
	
	/**
	 * @set default_text_family {TextFamily}
	 */
	static void set_default_text_family(Local<JSString> name,
																			Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextFamily, value, "Application.defaultTextFamily = %s");
		self->set_default_text_family(out);
	}
	
	/**
	 * @set default_text_shadow {TextShadow}
	 */
	static void set_default_text_shadow(Local<JSString> name,
																			Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextShadow, value, "Application.defaultTextShadow = %s");
		self->set_default_text_shadow(out);
	}
	
	/**
	 * @set default_text_line_height {TextLineHeight}
	 */
	static void set_default_text_line_height(Local<JSString> name,
																					 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextLineHeight, value, "Application.defaultTextLineHeight = %s");
		self->set_default_text_line_height(out);
	}
	
	/**
	 * @set default_text_decoration {TextDecoration}
	 */
	static void set_default_text_decoration(Local<JSString> name,
																					Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextDecoration, value, "Application.defaultTextDecoration = %s");
		self->set_default_text_decoration(out);
	}
	
	/**
	 * @set default_text_overflow {TextOverflow}
	 */
	static void set_default_text_overflow(Local<JSString> name,
																				Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextOverflow, value, "Application.defaultTextOverflow = %s");
		self->set_default_text_overflow(out);
	}
	
	/**
	 * @set default_text_white_space {TextWhiteSpace}
	 */
	static void set_default_text_white_space(Local<JSString> name,
																							Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Application);
		js_parse_value(TextWhiteSpace, value, "Application.defaultTextWhiteSpace = %s");
		self->set_default_text_white_space(out);
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(NativeApplication, constructor, {
			JS_SET_CLASS_METHOD(clear, clear);
			JS_SET_CLASS_METHOD(openUrl, open_url);
			JS_SET_CLASS_METHOD(sendEmail, send_email);
			JS_SET_CLASS_METHOD(maxTextureMemoryLimit, max_texture_memory_limit);
			JS_SET_CLASS_METHOD(setMaxTextureMemoryLimit, set_max_texture_memory_limit);
			JS_SET_CLASS_METHOD(usedMemory, used_texture_memory);
			JS_SET_CLASS_METHOD(pending, pending);
			JS_SET_CLASS_ACCESSOR(isLoaded, is_loaded);
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
	WrapNativeApplication::binding(exports, worker);
}

JS_END
