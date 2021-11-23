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
#include "../../app.h"
#include "../../views2/view.h"
#include "./_view.h"
#include "../../util/http.h"
#include "./_event.h"
#include "../../../android/android.h"
#include <native-inl-js.h>
#include <deps/node/src/flare.h>
#include <uv.h>

/**
 * @ns flare::js
 */

JS_BEGIN

void WrapViewBase::destroy() {
	UILock lock;
	delete this;
}

template<class T, class Self>
static void addEventListener_1(
	Wrap<Self>* wrap, const UIEventName& type, cString& func, int id, Cast* cast = nullptr) 
{
	auto f = [wrap, func, cast](typename Self::EventType& evt) {
		auto worker = wrap->worker();
		JS_HANDLE_SCOPE();
		JS_CALLBACK_SCOPE();

		// arg event
		Wrap<T>* ev = Wrap<T>::pack(static_cast<T*>(&evt), JS_TYPEID(T));
		if (cast) 
			ev->setPrivateData(cast); // set data cast func
		Local<JSValue> args[2] = { ev->that(), worker->New(true) };
		
		DLOG("JsView", "addEventListener_1, %s, EventType: %s", *func, *evt.name());

		// call js trigger func
		Local<JSValue> r = wrap->call( worker->New(func,1), 2, args );
	};
	
	Self* self = wrap->self();
	self->add_event_listener(type, f, id);
}

bool WrapViewBase::addEventListener(cString& name_s, cString& func, int id)
{
	auto i = UIEventNames.find(name_s);
	if ( i.is_null() ) {
		return false;
	}

	UIEventName name = i.value();
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
	Cast* cast = nullptr;

	switch ( UI_EVENT_FLAG_CAST & name.flag() ) {
		case UI_EVENT_FLAG_ERROR: cast = Cast::Entity<Error>(); break;
		case UI_EVENT_FLAG_FLOAT: cast = Cast::Entity<Float>(); break;
		case UI_EVENT_FLAG_UINT64: cast = Cast::Entity<Uint64>(); break;
	}

	switch ( name.category() ) {
		case UI_EVENT_CATEGORY_CLICK:
			addEventListener_1<ClickEvent>(wrap, name, func, id, cast); break;
		case UI_EVENT_CATEGORY_KEYBOARD:
			addEventListener_1<KeyEvent>(wrap, name, func, id, cast); break;
		case UI_EVENT_CATEGORY_MOUSE:
		 addEventListener_1<MouseEvent>(wrap, name, func, id, cast); break;
		case UI_EVENT_CATEGORY_TOUCH:
			addEventListener_1<TouchEvent>(wrap, name, func, id, cast); break;
		case UI_EVENT_CATEGORY_HIGHLIGHTED:
			addEventListener_1<HighlightedEvent>(wrap, name, func, id, cast); break;
		case UI_EVENT_CATEGORY_ACTION:
			addEventListener_1<ActionEvent>(wrap, name, func, id, cast); break;
		case UI_EVENT_CATEGORY_FOCUS_MOVE:
			addEventListener_1<FocusMoveEvent>(wrap, name, func, id, cast); break;
		default: // DEFAULT
			addEventListener_1<UIEvent>(wrap, name, func, id, cast); break;
	}
	return true;
}

bool WrapViewBase::removeEventListener(cString& name, int id) {
	auto i = UI_EVENT_TABLE.find(name);
	if ( i.is_null() ) {
		return false;
	}
	
	DLOG("JsView", "removeEventListener, name:%s, id:%d", *name, id);
	
	auto wrap = reinterpret_cast<Wrap<View>*>(this);
	wrap->self()->remove_event_listener(i.value(), id); // off event listener
	return true;
}

JS_END
