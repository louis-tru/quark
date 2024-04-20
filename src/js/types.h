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

#ifndef __quark__js__value__
#define __quark__js__value__

#include "./js.h"
#include "../ui/types.h"
#include "../render/bezier.h"
#include "../ui/filter.h"

namespace qk { namespace js {
	// using namespace qk::value;

	#define js_parse_value(Type, value, desc) js_parse_value2(Type, Type, value, desc)
	#define js_parse_value2(Type, Name, value, desc) \
		Type out; \
		if ( !worker->values()->parse##Name(value, out, desc)) \
		{ return; /*JS_THROW_ERR("Bad argument.");*/ }

	#define js_throw_value_err(value, msg, ...)\
		worker->values()->throwError(t, msg, ##__VA_ARGS__)

	#define js_common_string(F)  \
		F(global)         F(exports)        F(constructor) \
		F(console)        F(__proto__)      F(__native_private_data) \
		F(prototype)      F(type)           F(value) \
		F(isAuto)         F(width)          F(height) \
		F(offset)         F(offsetX)        F(offsetY) \
		F(_value)         F(r)              F(g) \
		F(b)              F(a)              F(x) \
		F(y)              F(z)              F(start) \
		F(point)          F(end)            F(w) \
		F(size)           F(color)          F(toJSON) \
		F(stack)          F(get_path)       F(_exit) \
		F(code)           F(message)        F(status) \
		F(url)            F(id)             F(startX) \
		F(startY)         F(force)          F(clickIn) \
		F(view)           F(_noticer)       F(point1X) \
		F(point1Y)        F(point2X)        F(point2Y) \
		F(time)           F(_change_touches)F(name) \
		F(pathname)       F(styles)         F(sender) \
		F(Buffer)         F(data)           F(total) \
		F(complete)       F(httpVersion)    F(statusCode) \
		F(responseHeaders) \

	#define js_value_table(F) \
		F(String, String)                       F(bool, bool) \
		F(float, float)                         F(int, int) \
		F(uint, uint)                           F(TextAlign, TextAlign) \
		F(Align, Align)                         F(ContentAlign, ContentAlign) \
		F(Border, Border)                       F(Shadow, Shadow) \
		F(Color, Color)                         F(Vec2, Vec2) \
		F(Vec3, Vec3)                           F(Vec4, Vec4) \
		F(Rect, Rect)                           F(Mat, Mat) \
		F(Mat4, Mat4)                           F(Value, Value) \
		F(TextColor, TextColor)                 F(TextSize, TextSize)  \
		F(TextFamily, TextFamily)               F(TextSlant, TextSlant) \
		F(TextShadow, TextShadow)               F(TextLineHeight, TextLineHeight) \
		F(TextDecoration, TextDecoration)       F(Repeat, Repeat) \
		F(Curve, Curve)                         F(Direction, Direction) \
		F(TextOverflow, TextOverflow)           F(TextWhiteSpace, TextWhiteSpace) \
		F(KeyboardType, KeyboardType)           F(KeyboardReturnType, KeyboardReturnType) \
		F(Background, BackgroundPtr)            F(BackgroundPosition, BackgroundPosition) \
		F(BackgroundSize, BackgroundSize) \
		/* Append, no actual type */\
		F(Values, Array<Value>)                 F(BackgroundSizeCollection, BackgroundSizeCollection) \
		F(Aligns, Array<Align>)                 F(BackgroundPositionCollection, BackgroundPositionCollection) \
		F(Floats, Array<float>) \

	class Qk_EXPORT CommonStrings: public Object {
	public:
		CommonStrings(Worker* worker);
		#define js_def_persistent_string(name) \
			public: Local<JSValue> name() { \
			auto r = reinterpret_cast<Local<JSValue>*>(&__##name##_$_); return *r; } \
			private: Persistent<JSValue> __##name##_$_;
	private:
		Worker* _worker;
		js_def_persistent_string(Throw)
		js_common_string(js_def_persistent_string);
	};

	class Qk_EXPORT TypesProgram: public Object {
	public:
		#define def_attr_fn(Name, Type)           \
			Local<JSValue> New(const Type& value);  \
			bool parse##Name(Local<JSValue> in, Type& out, cChar* err_msg = nullptr);
		#define def_attr(Name, Type) \
			Persistent<JSFunction> _parse##Name; \
			Persistent<JSFunction> _##Name;
		TypesProgram(Worker* worker, Local<JSObject> exports, Local<JSObject> native);
		virtual ~TypesProgram();
		void throwError(Local<JSValue> value, cChar* msg = nullptr, cChar* help = nullptr);
		js_value_table(def_attr_fn);
		bool isBase(Local<JSValue> arg);
	private:
		js_value_table(def_attr)
		Persistent<JSFunction> _Base;
		Worker* worker;
		#undef def_attr_fn
		#undef def_attr
	};

} }
#endif
