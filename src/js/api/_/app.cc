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

#include "../js_.h"
#include "./_view.h"
#include "../../app.h"
#include "../../views2/root.h"
#include "../../display-port.h"
#include "../../draw.h"
#include "../../views2/limit.h"

/**
 * @ns qk::js
 */

Js_BEGIN

using namespace qk;

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

	virtual bool addEventListener(cString& name, cString& func, int id)
	{
		if ( name == load ) {
			self<Type>()->Js_Native_On(Load, func, id);
		} else if ( name == unload ) {
			self<Type>()->Js_Native_On(Unload, func, id);
		} else if ( name == background ) {
			self<Type>()->Js_Native_On(Background, func, id);
		} else if ( name == foreground ) {
			self<Type>()->Js_Native_On(Foreground, func, id);
		} else if ( name == pause ) {
			self<Type>()->Js_Native_On(Pause, func, id);
		} else if ( name == resume ) {
			self<Type>()->Js_Native_On(Resume, func, id);
		} else if ( name == memorywarning ) {
			self<Type>()->Js_Native_On(Memorywarning, func, id);
		} else {
			return false;
		}
		return true;
	}

	virtual bool removeEventListener(cString& name, int id)
	{
		if ( name == load ) {
			self<Type>()->Qk_Off(Load, id);
		} else if ( name == unload ) {
			self<Type>()->Qk_Off(Unload, id);
		} else if ( name == background ) {
			self<Type>()->Qk_Off(Background, id);
		} else if ( name == foreground ) {
			self<Type>()->Qk_Off(Foreground, id);
		} else if ( name == pause ) {
			self<Type>()->Qk_Off(Pause, id);
		} else if ( name == resume ) {
			self<Type>()->Qk_Off(Resume, id);
		} else if ( name == memorywarning ) {
			self<Type>()->Qk_Off(Memorywarning, id);
		} else {
			return false;
		}
		return true;
	}
	
	void memorywarning_handle(Event<>& evt) {
		worker()->garbageCollection(); // 清理内存
#if Qk_MEMORY_TRACE_MARK
			uint32_t count = Object::mark_objects_count();
			Qk_LOG("All unrelease heap objects count: %d", count);
#endif
	}

	/**
	 * @constructor([options])
	 * @param [options] {Object} { anisotropic {bool}, mipmap {bool}, multisample {0-4} }
	 */
	static void constructor(FunctionArgs args) {
		Js_Worker(args);
		
		JSON options = JSON::object();
		if ( args.length() > 0 && args[0]->IsObject(worker) ) {
			if (!args[0].To<JSObject>()->ToJSON(worker).To(options))
				return;
		}
		
		Wrap<NativeApplication>* wrap = nullptr;
		try {
			Handle<Application> h = new Application();
			h->initialize(options);
			auto app = h.collapse();
			app->Qk_On(Memorywarning,
								 &WrapNativeApplication::memorywarning_handle,
								 reinterpret_cast<WrapNativeApplication*>(wrap));
			app->run_loop_detach(); // run gui loop
			wrap = New<WrapNativeApplication>(args, app);
		} catch(cError& err) {
			if ( wrap )
				delete wrap;
			Js_Throw(err);
		}
	}
	
	/**
	 * @method clear() clear gui application resources
	 */
	static void clear(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		if ( args.length() > 1 ) {
			self->clear( args[0]->ToBooleanValue(worker) );
		} else {
			self->clear();
		}
	}
	
	/**
	 * @method open_url(url)
	 */
	static void open_url(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() == 0 ) {
			Js_Throw("@func openUrl(url)");
		}
		Js_Self(Application);
		self->open_url( args[0]->toStringValue(worker) );
	}
	
	/**
	 * @method send_email(recipient,title,body[,cc[,bcc]])
	 */
	static void send_email(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 2 ) {
			Js_Throw("@func sendEmail(recipient,title,body[,cc[,bcc])");
		}
		String recipient = args[0]->toStringValue(worker);
		String title = args[1]->toStringValue(worker);
		String body, cc, bcc;
		if ( args.length() > 2 ) cc = args[2]->toStringValue(worker);
		if ( args.length() > 3 ) cc = args[3]->toStringValue(worker);
		if ( args.length() > 4 ) bcc = args[4]->toStringValue(worker);
		
		Js_Self(Application);
		self->send_email( recipient, title, cc, bcc, body );
	}
	
	static void max_texture_memory_limit(FunctionArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return(self->max_texture_memory_limit());
	}

	static void set_max_texture_memory_limit(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() == 0 && !args[0]->IsNumber() ) {
			Js_Throw("@func setMaxTextureMemoryLimit(limit)");
		}
		Js_Self(Application);
		self->set_max_texture_memory_limit( args[0]->ToNumberValue(worker) );
	}
	
	static void used_texture_memory(FunctionArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return(self->used_texture_memory());
	}
	
	/**
	 * @method pending()
	 */
	static void pending(FunctionArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		self->pending();
	}

	/**
	 * @get is_loaded {bool}
	 */
	static void is_loaded(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( self->is_loaded() );
	}
	
	/**
	 * @get display_port {Display}
	 */
	static void display_port(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		auto wrap = pack(self->display_port());
		Js_Return( wrap->that() );
	}
	
	/**
	 * @get root {Root}
	 */
	static void root(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Root* root = self->root();
		if (! root) { // null
			Js_Return( worker->NewNull() );
		}
		Wrap<Root>* wrap = pack(root);
		Js_Return( wrap->that() );
	}
	
	/**
	 * @get focus_view {View}
	 */
	static void focus_view(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		View* view = self->focus_view();
		if (! view) { // null
			Js_Return_Null();
		}
		Wrap<View>* wrap = Wrap<View>::pack(view);
		Js_Return( wrap->that() );
	}
	
	/**
	 * @get default_text_background_color {TextColor}
	 */
	static void default_text_background_color(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_background_color()) );
	}
	
	/**
	 * @get default_text_color {TextColor}
	 */
	static void default_text_color(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_color()) );
	}
	
	/**
	 * @get default_text_size {TextSize}
	 */
	static void default_text_size(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_size()) );
	}
	
	/**
	 * @get default_text_slant {TextSlant}
	 */
	static void default_text_slant(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_slant()) );
	}
	
	/**
	 * @get default_text_family {TextFamily}
	 */
	static void default_text_family(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_family()) );
	}
	
	/**
	 * @get default_text_shadow {TextShadow}
	 */
	static void default_text_shadow(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_shadow()) );
	}
	
	/**
	 * @get default_text_line_height {TextLineHeight}
	 */
	static void default_text_line_height(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_line_height()) );
	}
	
	/**
	 * @get default_text_decoration {TextDecoration}
	 */
	static void default_text_decoration(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_decoration()) );
	}
	
	/**
	 * @get default_text_overflow {TextOverflow}
	 */
	static void default_text_overflow(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_overflow()) );
	}
	
	/**
	 * @get default_text_white_space {TextWhiteSpace}
	 */
	static void default_text_white_space(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Application);
		Js_Return( worker->values()->New(self->default_text_white_space()) );
	}
	
	/**
	 * @set default_text_background_color {TextColor}
	 */
	static void set_default_text_background_color(JSString* name,
																								JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextColor, value, "Application.defaultTextBackgroundColor = %s");
		self->set_default_text_background_color(out);
	}
	
	/**
	 * @set default_text_color {TextColor}
	 */
	static void set_default_text_color(JSString* name,
																		 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextColor, value, "Application.defaultTextColor = %s");
		self->set_default_text_color(out);
	}
	
	/**
	 * @set default_text_size {TextSize}
	 */
	static void set_default_text_size(JSString* name,
																		JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextSize, value, "Application.defaultTextSize = %s");
		self->set_default_text_size(out);
	}
	
	/**
	 * @set default_text_slant {TextSlant}
	 */
	static void set_default_text_slant(JSString* name,
																		 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextSlant, value, "Application.defaultTextSlant = %s");
		self->set_default_text_slant(out);
	}
	
	/**
	 * @set default_text_family {TextFamily}
	 */
	static void set_default_text_family(JSString* name,
																			JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextFamily, value, "Application.defaultTextFamily = %s");
		self->set_default_text_family(out);
	}
	
	/**
	 * @set default_text_shadow {TextShadow}
	 */
	static void set_default_text_shadow(JSString* name,
																			JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextShadow, value, "Application.defaultTextShadow = %s");
		self->set_default_text_shadow(out);
	}
	
	/**
	 * @set default_text_line_height {TextLineHeight}
	 */
	static void set_default_text_line_height(JSString* name,
																					 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextLineHeight, value, "Application.defaultTextLineHeight = %s");
		self->set_default_text_line_height(out);
	}
	
	/**
	 * @set default_text_decoration {TextDecoration}
	 */
	static void set_default_text_decoration(JSString* name,
																					JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextDecoration, value, "Application.defaultTextDecoration = %s");
		self->set_default_text_decoration(out);
	}
	
	/**
	 * @set default_text_overflow {TextOverflow}
	 */
	static void set_default_text_overflow(JSString* name,
																				JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextOverflow, value, "Application.defaultTextOverflow = %s");
		self->set_default_text_overflow(out);
	}
	
	/**
	 * @set default_text_white_space {TextWhiteSpace}
	 */
	static void set_default_text_white_space(JSString* name,
																							JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Application);
		js_parse_value(TextWhiteSpace, value, "Application.defaultTextWhiteSpace = %s");
		self->set_default_text_white_space(out);
	}
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(NativeApplication, constructor, {
			Js_Set_Class_Method(clear, clear);
			Js_Set_Class_Method(openUrl, open_url);
			Js_Set_Class_Method(sendEmail, send_email);
			Js_Set_Class_Method(maxTextureMemoryLimit, max_texture_memory_limit);
			Js_Set_Class_Method(setMaxTextureMemoryLimit, set_max_texture_memory_limit);
			Js_Set_Class_Method(usedMemory, used_texture_memory);
			Js_Set_Class_Method(pending, pending);
			Js_Set_Class_Accessor(isLoaded, is_loaded);
			Js_Set_Class_Accessor(displayPort, display_port);
			Js_Set_Class_Accessor(root, root);
			Js_Set_Class_Accessor(focusView, focus_view);
			Js_Set_Class_Accessor(defaultTextBackgroundColor,
														default_text_background_color, set_default_text_background_color);
			Js_Set_Class_Accessor(defaultTextColor, default_text_color, set_default_text_color);
			Js_Set_Class_Accessor(defaultTextSize, default_text_size, set_default_text_size);
			Js_Set_Class_Accessor(defaultTextSlant, default_text_slant, set_default_text_slant);
			Js_Set_Class_Accessor(defaultTextFamily, default_text_family, set_default_text_family);
			Js_Set_Class_Accessor(defaultTextShadow, default_text_shadow, set_default_text_shadow);
			Js_Set_Class_Accessor(defaultTextLineHeight, default_text_line_height, set_default_text_line_height);
			Js_Set_Class_Accessor(defaultTextDecoration, default_text_decoration, set_default_text_decoration);
			Js_Set_Class_Accessor(defaultTextOverflow, default_text_overflow, set_default_text_overflow);
			Js_Set_Class_Accessor(defaultTextWhiteSpace, default_text_white_space, set_default_text_white_space);
		}, NULL);
	}
};

void binding_app(JSObject* exports, Worker* worker) {
	WrapNativeApplication::binding(exports, worker);
}

Js_END
