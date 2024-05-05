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

#include "./js_.h"
#include "../ui/types.h"
#include "../ui/filter.h"
#include "../ui/window.h"
#include "../render/bezier.h"
#include "../util/fs.h"

namespace qk { namespace js {

	#define Js_Parse_Type(Type, value, desc) \
		Type out; \
		if ( !worker->types()->parse##Type(value, out, desc)) return

	#define Js_Throw_Types(value, msg, ...)\
		worker->types()->throwError(t, msg, ##__VA_ARGS__)

	#define Js_Strings_Each(F)  \
		F(global)         F(exports)        F(constructor) \
		F(console)        F(__proto__)      F(_wrap_external_data) \
		F(prototype)      F(type)           F(value) \
		F(isAuto)         F(width)          F(height) \
		F(offset)         F(offsetX)        F(offsetY) \
		F(_value)         F(r)              F(g) \
		F(b)              F(a)              F(x) \
		F(y)              F(z)              F(start) \
		F(point)          F(end)            F(w) \
		F(size)           F(color)          F(toJSON) \
		F(stack)          F(get_path)       F(_cb) \
		F(message)        F(status)         F(Errno) \
		F(url)            F(id)             F(startX) \
		F(startY)         F(force)          F(clickIn) \
		F(view)           F(_data)          F(point1X) \
		F(point1Y)        F(point2X)        F(point2Y) \
		F(time)           F(_change_touches)F(name) \
		F(pathname)       F(styles)         F(sender) \
		F(Buffer)         F(data)           F(total) \
		F(complete)       F(httpVersion)    F(statusCode) \
		F(responseHeaders) F(Throw)         F(kind) \

	class Qk_EXPORT Strings {
	public:
		Strings(Worker* worker);
		#define _Fun(name) \
		public: inline JSValue* name() { return *__##name##__; } \
		private: Persistent<JSValue> __##name##__;
		Js_Strings_Each(_Fun);
		#undef _Fun
	};

	typedef BoxFilter* BoxFilterPtr;
	typedef FillGradient* FillGradientPtr;
	typedef BoxShadow* BoxShadowPtr;

	#define Js_Types_Each(F) \
		F(bool, bool) \
		F(float, float) \
		F(int, int) \
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
		F(BoxFilterPtr, BoxFilterPtr) \
		F(FillGradientPtr, FillGradientPtr) \
		F(BoxShadowPtr, BoxShadowPtr) \
		F(Direction, Direction) \
		F(ItemsAlign, ItemsAlign) \
		F(CrossAlign, CrossAlign) \
		F(Wrap, qk::Wrap) \
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
		F(TextSize, TextSize) /*TextLineHeight*/\
		F(TextShadow, TextShadow) \
		F(TextFamily, TextFamily) \
		F(TextWeight, TextWeight) \
		F(TextWidth, TextWidth) \
		F(TextSlant, TextSlant) \
		F(FontStyle, FontStyle) \
		F(KeyboardType, KeyboardType) \
		F(KeyboardReturnType, KeyboardReturnType) \
		F(CursorStyle, CursorStyle) \
		F(FindDirection, FindDirection) \

	typedef Window::Options WindowOptions;

	class Qk_EXPORT TypesParser {
	public:
		TypesParser(Worker* worker, JSObject* exports);
		inline
		JSValue* newInstance(Object* val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(double val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(Char val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(uint8_t val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(int16_t val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(uint16_t val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(int64_t val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(uint64_t val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(cString2& val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(cString4& val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(const Error& val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(const HttpError& val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(cArray<String>& val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(cDictSS& val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(Buffer& val) { return worker->newInstance(val); }
		inline
		JSValue* newInstance(Buffer&& val) { return worker->newInstance(val);}
		template <class S>
		inline S* newInstance(const Persistent<S>& value) { return *value; }
		inline
		JSValue* newInstance(JSValue* val) { return val; }
		inline
		JSValue* newInstance(const Bool& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Float32& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Float64& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Int8& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Uint8& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Int16& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Uint16& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Int32& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Uint32& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Int64& v) { return worker->newInstance(v.value); }
		inline
		JSValue* newInstance(const Uint64& v) { return worker->newInstance(v.value); }
		JSValue* newInstance(const Dirent& val);
		JSValue* newInstance(const FileStat& val);
		JSValue* newInstance(const Array<Dirent>& val);
		JSValue* newInstance(const Array<FileStat>& val);
		bool     isBase(JSValue *arg);
		void     throwError(JSValue* value, cChar* msg = 0, cChar* help = 0);
		bool     parseWindowOptions(JSValue* in, WindowOptions& out, cChar* desc);
	#define _Def_Fun(Name, Type) \
		JSValue* newInstance(const Type& value); \
		bool parse##Name(JSValue* in, Type& out, cChar* err_msg = 0);
		Js_Types_Each(_Def_Fun);
	private:
		Worker *worker;
	#define _Def_attr(Name, Type) \
		Persistent<JSFunction> _parse##Name; \
		Persistent<JSFunction> _new##Name;
		Js_Types_Each(_Def_attr)
	#undef _Def_Fun
	#undef _Def_attr
	};

} }
#endif
