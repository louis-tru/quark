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

#include "./ui.h"
#include "../../render/font/pool.h"
#include "../../ui/app.h"
#include "../../ui/view/root.h"
#include "../../ui/screen.h"
#include "../../ui/text/text_opts.h"

namespace qk { namespace js {
	typedef Application NativeApplication;

	uint64_t kView_Typeid(Js_Typeid(View));
	uint64_t kWindow_Typeid(Js_Typeid(Window));

	bool isView(Worker *worker, JSValue* value) {
		return worker->instanceOf(value, kView_Typeid);
	}

	bool isWindow(Worker *worker, JSValue* value) {
		return worker->instanceOf(value, kWindow_Typeid);
	}

	bool checkApp(Worker *worker) {
		auto app = shared_app();
		if ( !app ) {
			Js_Throw("Unable to find shared application, need to create application first"), false;
		}
		return true;
	}

	TextOptions* WrapUIObject::asTextOptions() {
		return nullptr;
	}

	ScrollBase* WrapUIObject::asScrollBase() {
		return nullptr;
	}

	Player* WrapUIObject::asPlayer() {
		return nullptr;
	}

	NotificationBasic* WrapUIObject::asNotificationBasic() {
		return nullptr;
	}

	bool WrapUIObject::addEventListener(cString& name, cString& func, int id) {
		auto notific = asNotificationBasic();
		const UIEventName *key;
		if (!notific || !UIEventNames.get(name, key)) {
			return false;
		}
		JsConverter* cData = nullptr;

		switch ( kTypesMask_UIEventFlags & key->flag() ) {
			case kError_UIEventFlags: cData = JsConverter::Instance<Error>(); break;
			case kFloat32_UIEventFlags: cData = JsConverter::Instance<Float32>(); break;
			case kUint64_UIEventFlags: cData = JsConverter::Instance<Uint64>(); break;
		}

		typedef EventNoticerBasic Basic;
		auto f = [this, func, cData](Event<>& e) {
			auto worker = this->worker();
			Js_Handle_Scope(); // Callback Scope
			// arg event
			auto ev = WrapObject::wrap(&e);
			Qk_Assert_Ne(ev, nullptr);
			if (cData) {
				ev->setProp(worker->strs()->_data(), cData->cast(worker, e.data()));
			}
			JSValue* args[2] = { ev->that(), worker->newValue(true) };

			// Qk_DLog("addEventListenerFrom, %s, EventType: %s", *func, *e.name());
			JSValue* r = call(func, 2, args); // call js trigger func
		};

		notific->add_event_listener(
			key->hashCode(), Basic::MakeLambdaListener(*(Basic::OnLambdaListenerFunc*)(&f), id, false)
		);

		return true;
	}

	bool WrapUIObject::removeEventListener(cString& name, int id) {
		auto notific = asNotificationBasic();
		const UIEventName *key;
		if (!notific || !UIEventNames.get(name, key)) {
			return false;
		}
		Qk_DLog("removeEventListener, name:%s, id:%d", *name, id);

		notific->remove_event_listener_for_id(key->hashCode(), id); // off event listener
		return true;
	}

	struct WrapNativeApplication: WrapObject {
		typedef Application Type;

		virtual bool addEventListener(cString& name, cString& func, int id) {
			if ( name == "Load" ) {
				self<Type>()->Js_Native_On(Load, func, id);
			} else if ( name == "Unload" ) {
				self<Type>()->Js_Native_On(Unload, func, id);
			} else if ( name == "Pause" ) {
				self<Type>()->Js_Native_On(Pause, func, id);
			} else if ( name == "Resume" ) {
				self<Type>()->Js_Native_On(Resume, func, id);
			} else if ( name == "Memorywarning" ) {
				self<Type>()->Js_Native_On(Memorywarning, func, id);
			} else {
				return false;
			}
			return true;
		}

		virtual bool removeEventListener(cString& name, int id) {
			if ( name == "Load" ) {
				self<Type>()->Qk_Off(Load, id);
			} else if ( name == "Unload" ) {
				self<Type>()->Qk_Off(Unload, id);
			} else if ( name == "Pause" ) {
				self<Type>()->Qk_Off(Pause, id);
			} else if ( name == "Resume" ) {
				self<Type>()->Qk_Off(Resume, id);
			} else if ( name == "Memorywarning" ) {
				self<Type>()->Qk_Off(Memorywarning, id);
			} else {
				return false;
			}
			return true;
		}

		void memorywarning_handle(Event<>& evt) {
			worker()->garbageCollection(); // js gc
		}
		
		static void NewApp(FunctionArgs args) {
			Js_Worker(args);
			auto app = new Application();
			auto wrap = New<WrapNativeApplication>(args, app);
			app->Qk_On(Memorywarning,
								&WrapNativeApplication::memorywarning_handle,
								reinterpret_cast<WrapNativeApplication*>(wrap));
			wrap->handle().clearWeak(); // clear weak, persistent object
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(NativeApplication, 0, { NewApp(args); });

			Js_Set_Class_Accessor_Get(isLoaded, {
				Js_Self(Type);
				Js_ReturnBool( self->isLoaded() );
			});

			Js_Set_Class_Accessor_Get(defaultTextOptions, {
				Js_Self(Type);
				Js_Return( self->defaultTextOptions() );
			});

			Js_Set_Class_Accessor_Get(screen, {
				Js_Self(Type);
				Js_Return( self->screen() );
			});

			// Qk_DEFINE_PGET(RunLoop*, loop); //! main run loop

			Js_Set_Class_Accessor_Get(fontPool, {
				Js_Self(Type);
				auto val = WrapObject::wrap(self->fontPool(), Js_Typeid(FontPool))->that();
				Js_Return( val );
			});

			// Qk_DEFINE_PGET(ImageSourcePool*, imgPool); //! image loader and image cache

			Js_Set_Class_Accessor_Get(activeWindow, {
				Js_Self(Type);
				Js_Return( self->activeWindow() );
			});

			// Qk_DEFINE_PGET(RootStyleSheets*, styleSheets); //! root style sheets

			Js_Set_Class_Accessor(maxResourceMemoryLimit, {
				Js_Self(Type);
				Js_Return( self->maxResourceMemoryLimit() );
			}, {
				if (!val->isUint32())
					Js_Throw("@prop Application.maxResourceMemoryLimit = {uint32_t}");
				Js_Self(Type);
				self->set_maxResourceMemoryLimit(val->toUint32Value(worker).unsafe());
			});

			Js_Set_Class_Accessor_Get(windows, {
				Js_Self(Type);
				uint32_t i = 0;
				auto arr = worker->newArray();
				for (auto it: self->windows()) {
					arr->set(worker, i++, wrap<Window>(it)->that());
				}
				Js_Return(arr);
			});

			Js_Set_Class_Method(clear, {
				auto all = args.length() && args[0]->toBooleanValue(worker);
				Js_Self(Type);
				self->clear(all);
			});

			Js_Set_Class_Method(usedResourceMemory, {
				Js_Self(Type);
				Js_Return(self->usedResourceMemory());
			});

			Js_Set_Class_Method(openURL, {
				if (!args.length() || !args[0]->isString()) {
					Js_Throw(
						"@methos Application.openURL(url)\n"
						"@param url {String}\n"
					);
				}
				Js_Self(Type);
				self->openURL(args[0]->toStringValue(worker));
			});

			Js_Set_Class_Method(sendEmail, {
				if (args.length() < 3 || !args[0]->isString() || !args[1]->isString() || !args[2]->isString()) {
					Js_Throw(
						"@methos Application.sendEmail(recipient,subject,body[,cc[,bcc]])\n"
						"@param recipient {String}\n"
						"@param subject {String}\n"
						"@param body {String}\n"
						"@param [cc] {String}\n"
						"@param [bcc] {String}\n"
					);
				}
				Js_Self(Type);
				self->sendEmail(
					args[0]->toStringValue(worker), // recipient
					args[1]->toStringValue(worker), // subject
					args[2]->toStringValue(worker), // body
					args.length() > 3 ? args[3]->toStringValue(worker): String(), // cc
					args.length() > 4 ? args[4]->toStringValue(worker): String() // bcc
				);
			});

			cls->exports("NativeApplication", exports);
		}
	};

	void binding_screen(JSObject* exports, Worker* worker);
	void binding_window(JSObject* exports, Worker* worker);
	void binding_view(JSObject* exports, Worker* worker);
	void binding_box(JSObject* exports, Worker* worker);
	void binding_text(JSObject* exports, Worker* worker);
	void binding_scroll(JSObject* exports, Worker* worker);
	void binding_transform(JSObject* exports, Worker* worker);
	void binding_player(JSObject* exports, Worker* worker);

	struct NativeUI {
		static void binding(JSObject* exports, Worker* worker) {
			worker->bindingModule("_font");
			worker->bindingModule("_types");
			worker->bindingModule("_event");
			WrapNativeApplication::binding(exports, worker);
			binding_screen(exports, worker);
			binding_window(exports, worker);
			binding_view(exports, worker);
			binding_box(exports, worker);
			binding_text(exports, worker);
			binding_scroll(exports, worker);
			binding_transform(exports, worker);
			binding_player(exports, worker);
		}
	};

	Js_Set_Module(_ui, NativeUI);
} }
