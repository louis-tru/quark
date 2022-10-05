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

#ifndef __quark__js__value__
#define __quark__js__value__

#include "./js.h"
#include "../value.h"
#include "../math/bezier.h"
#include "../background.h"

/**
 * @ns quark::js
 */

JS_BEGIN

using namespace quark::value;

#define js_parse_value(Type, value, desc) js_parse_value2(Type, Type, value, desc)
#define js_parse_value2(Type, Name, value, desc) \
	Type out; \
	if ( !worker->values()->parse##Name(value, out, desc)) \
	{ return; /*JS_THROW_ERR("Bad argument.");*/ }

#define js_throw_value_err(value, msg, ...)\
	worker->values()->throwError(t, msg, ##__VA_ARGS__)

// ------------- values -------------

#define js_value_table(F) \
F(String, String)                       F(bool, bool) \
F(float, float)                         F(int, int) \
F(uint, uint)                           F(TextAlign, TextAlign) \
F(Align, Align)                         F(ContentAlign, ContentAlign) \
F(Border, Border)                       F(Shadow, Shadow) \
F(Color, Color)                         F(Vec2, Vec2) \
F(Vec3, Vec3)                           F(Vec4, Vec4) \
F(Rect, Rect)                         F(Mat, Mat) \
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

/**
 * @class ValueProgram
 */
class Qk_EXPORT ValueProgram: public Object {
	public:
	#define def_attr_fn(Name, Type)           \
		Local<JSValue> New(const Type& value);  \
		bool parse##Name(Local<JSValue> in, Type& out, cChar* err_msg = nullptr);
	#define def_attr(Name, Type) \
		Persistent<JSFunction> _parse##Name; \
		Persistent<JSFunction> _##Name;
	ValueProgram(Worker* worker, Local<JSObject> exports, Local<JSObject> native);
	virtual ~ValueProgram();
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


JS_END
#endif
