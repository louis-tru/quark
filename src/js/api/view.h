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
#include "../js_.h"
#include "../../ui/view/view.h"

#define Js_IsView(v) isView(worker,v)
#define Js_IsWindow(v) isWindow(worker,v)
#define Js_NewView(Type, ...) \
	auto win = NewCheck(args); \
	if (win) New<Wrap##Type>(args, View::Make<Type>(win))

#define Js_TextOptions() \
	auto self = qk::js::WrapObject::wrapObject<WrapUIObject>(args.This())->asTextOptions()
#define Js_ScrollBase() \
	auto self = qk::js::WrapObject::wrapObject<WrapUIObject>(args.This())->asScrollBase()

namespace qk { namespace js {

	extern uint64_t kView_Typeid;
	extern uint64_t kWindow_Typeid;

	class Qk_EXPORT WrapUIObject: public WrapObject {
	public:
		virtual TextOptions* asTextOptions();
		virtual ScrollBase*  asScrollBase();
	};

	class Qk_EXPORT WrapViewObject: public WrapUIObject {
	public:
		virtual bool addEventListener(cString& name, cString& func, int id);
		virtual bool removeEventListener(cString& name, int id);
		static  Window* NewCheck(FunctionArgs args);
	};

	bool isView(Worker *worker, JSValue* value);
	bool isWindow(Worker *worker, JSValue* value);
	void inheritScrollBase(JSClass* cls, Worker* worker);

} }
#endif
