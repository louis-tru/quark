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

// @private head

#ifndef __quark__js__api__view__
#define __quark__js__api__view__
#include "./types.h"
#include "../../ui/view/view.h"
#include "../../media/player.h"

namespace qk { namespace js {

	#define Js_MixObject_Accessor_Base(Type,T,Prop,Name,Ext) \
		Js_Class_Accessor(Name, {\
			Ext; \
			args.returnValue().set( worker->types()->jsvalue(self->Prop()) ); \
		}, { \
			Js_Parse_Type(T, val, "@prop "#Type"."#Name" = %s"); \
			Ext; \
			self->set_##Prop(out); \
		})

	#define Js_MixObject_Accessor(Type,T,Prop,Name) Js_MixObject_Accessor_Base(Type,T,Prop,Name,)

	#define Js_UISelf(Type) \
		static_assert_mix<MixUIObject>(); \
		auto self = static_cast<MixUIObject*>((MixObject*)mix)->as##Type()
	#define Js_UIObject_Accessor(Type,T,Prop,Name) \
		Js_MixObject_Accessor_Base(Type,T,Prop,Name,Js_UISelf(Type))

	#define Js_IsView(v) isView(worker,v)
	#define Js_IsWindow(v) isWindow(worker,v)
	#define Js_NewView(Type, ...) \
		auto win = checkNewView(args); \
		if (win) New<Mix##Type>(args, View::Make<Type>(win))

	template<class T = MixObject>
	static inline void static_assert_mix() {
		Js_Type_Check(MixObject, T);
	}

	extern uint64_t kView_Typeid;
	extern uint64_t kWindow_Typeid;

	class Qk_EXPORT MixUIObject: public MixObject {
	public:
		virtual TextOptions* asTextOptions();
		virtual ScrollView*  asScrollView();
		virtual MatrixView*  asMatrixView();
		virtual Player*      asPlayer();
		virtual NotificationBasic* asNotificationBasic();
		virtual bool addEventListener(cString& name_, cString& func, int id) override;
		virtual bool removeEventListener(cString& name, int id) override;
	};

	class Qk_EXPORT MixViewObject: public MixUIObject {
	public:
		virtual NotificationBasic* asNotificationBasic() override;
		static Window* checkNewView(FunctionArgs args);
	};

	bool checkApp(Worker *worker);
	bool isView(Worker *worker, JSValue* value);
	bool isWindow(Worker *worker, JSValue* value);
	void inheritScrollView(JSClass* cls, Worker* worker);
	void inheritMatrixView(JSClass* cls, Worker* worker);

} }
#endif
