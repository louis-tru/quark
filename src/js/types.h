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

#ifndef __quark__js__types__
#define __quark__js__types__

#include "./js.h"
#include "../ui/types.h"
#include "../render/bezier.h"
#include "../ui/filter.h"

namespace qk { namespace js {

	#define Js_Parse_Type(Type, value, desc) Js_Parse_Type2(Type, Type, value, desc)
	#define Js_Parse_Type2(Type, Name, value, desc) \
		Type out; \
		if ( !worker->values()->parse##Name(value, out, desc)) \
		{ return; /*Js_Throw("Bad argument.");*/ }

	#define Js_Throw_Types(value, msg, ...)\
		worker->types()->throwError(t, msg, ##__VA_ARGS__)

	#define Js_Common_Strings_Each(F)  \
		F(global)         F(exports)        F(constructor) \
		F(console)        F(__proto__)      F(__native_external_data) \
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
		F(responseHeaders) F(Throw)\

	class Qk_EXPORT CommonStrings {
	public:
		Qk_DEFINE_PROP_GET(Worker*, worker);
		CommonStrings(Worker* worker);
		#define _Fun(name) \
		public: inline Local<JSValue> name() { return __##name##__.toLocal(); } \
		private: Persistent<JSValue> __##name##__;
		Js_Common_Strings_Each(_Fun);
		#undef _Fun
	};

	#define Js_Types_Each(F) \
		F(bool, bool) \
		F(float, float) \
		F(int32_t, int32_t) \
		F(uint32_t, uint32_t) \
		F(Color, Color) \
		F(Vec2, Vec2) \
		F(Vec3, Vec3) \
		F(Vec4, Vec4) \
		F(Rect, Rect) \
		F(Mat, Mat) \
		F(Mat4, Mat4) \
		F(String, String) \
		F(Curve, Curve) \
		F(Shadow, Shadow) \
		F(Repeat, Repeat) \
		F(BoxFilter, BoxFilter*) \
		F(FillGradient, FillGradient*) \
		F(BoxShadow, BoxShadow*) \
		/********************************/\
		F(Direction, Direction) \
		F(ItemsAlign, ItemsAlign) \
		F(CrossAlign, CrossAlign) \
		F(Wrap, Wrap) \
		F(WrapAlign, WrapAlign) \
		F(Align, Align) \
		F(BoxSizeKind, BoxSizeKind) \
		F(BoxOriginKind, BoxOriginKind) \
		F(BoxSize, BoxSize) \
		F(BoxOrigin, BoxOrigin) \
		F(TextAlign, TextAlign) \
		F(TextDecoration, TextDecoration) \
		F(TextOverflow, TextOverflow) \
		F(TextWhiteSpace, TextWhiteSpace) \
		F(TextWordBreak, TextWordBreak) \
		F(TextValueKind, TextValueKind) \
		F(TextColor, TextColor) \
		F(TextSize, TextSize) \
		F(TextLineHeight, TextLineHeight) \
		F(TextShadow, TextShadow) \
		F(TextFamily, TextFamily) \
		F(TextWeight, TextWeight) \
		F(TextWidth, TextWidth) \
		F(TextSlant, TextSlant) \
		F(KeyboardType, KeyboardType) \
		F(KeyboardReturnType, KeyboardReturnType) \
		F(CursorStyle, CursorStyle) \
		F(FindDirection, FindDirection) \

	class Qk_EXPORT TypesParser {
	public:
		Qk_DEFINE_PROP_GET(Worker*, worker);
		TypesParser(Worker* worker, Local<JSObject> exports, Local<JSObject> native);
		void throwError(Local<JSValue> value, cChar* msg = nullptr, cChar* help = nullptr);
	#define _Fun(Name, Type) \
		Local<JSValue> newInstance(const Type& value); \
		bool parse##Name(Local<JSValue> in, Type& out, cChar* err_msg = nullptr);
		Js_Types_Each(_Fun);
	private:
	#define _Def_attr(Name, Type) \
		Persistent<JSFunction> _parse##Name; \
		Persistent<JSFunction> _new##Name;
		Js_Types_Each(_Def_attr)
	#undef _Fun
	#undef _Def_attr
	};

} }
#endif
