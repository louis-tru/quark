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

#include "../js_.h"
#include "../../ui/types.h"
#include "../../ui/filter.h"
#include "../../ui/window.h"
#include "../../ui/event.h"
#include "../../render/bezier.h"
#include "../../util/fs.h"

namespace qk { namespace js {

	#define Js_Parse_Type(Name, value, desc) \
		Name out; \
		if ( !worker->types()->parse(value, out, desc)) return

	#define Js_Throw_Types(value, msg, ...)\
		worker->types()->throwError(t, msg, ##__VA_ARGS__)

	typedef qk::Wrap Wrap;
	typedef Window::Options WindowOptions;
	typedef FillImage::Init FillImageInit;
	typedef TouchEvent::TouchPoint TouchPoint;
	typedef Array<String> ArrayString;

	#define Js_Types_Each(F) \
		F(bool) \
		F(float) \
		F(int32_t) \
		F(uint32_t) \
		F(Color) \
		F(Vec2) \
		F(Vec3) \
		F(Vec4) \
		F(Rect) \
		F(Mat) \
		F(Mat4) \
		F(ArrayFloat) \
		F(ArrayString) \
		F(ArrayColor) \
		F(ArrayOrigin) \
		F(ArrayBorder) \
		F(String) \
		F(Curve) \
		F(Shadow) \
		F(BoxBorder) \
		F(FillPosition) \
		F(FillSize) \
		F(Repeat) \
		F(BoxFilterPtr) \
		F(BoxShadowPtr) \
		F(Direction) \
		F(ItemsAlign) \
		F(CrossAlign) \
		F(Wrap) \
		F(WrapAlign) \
		F(Align) \
		F(BoxSize) \
		F(BoxOrigin) \
		F(TextAlign) \
		F(TextDecoration) \
		F(TextOverflow) \
		F(TextWhiteSpace) \
		F(TextWordBreak) \
		F(TextColor) \
		F(TextSize) /*TextLineHeight*/\
		F(TextShadow) \
		F(TextFamily) \
		F(TextWeight) \
		F(TextWidth) \
		F(TextSlant) \
		F(FontStyle) \
		F(KeyboardType) \
		F(KeyboardReturnType) \
		F(CursorStyle) \
		F(FindDirection) \
		F(FFID) \

	class Qk_EXPORT TypesParser {
	public:
		TypesParser(Worker* worker, JSObject* exports);

		bool     isTypesBase(JSObject *arg);
		void     throwError(JSValue* value, cChar* msg = 0, cChar* help = 0);
		inline
		JSValue* jsvalue(Object* val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(double val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(Char val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(uint8_t val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(int16_t val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(uint16_t val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(int64_t val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(uint64_t val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(cString2& val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(cString4& val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(const Error& val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(const HttpError& val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(cArray<String>& val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(cDictSS& val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(Buffer& val) { return worker->newInstance(val); }
		inline
		JSValue* jsvalue(Buffer&& val) { return worker->newInstance(val);}
		template <class S>
		inline S* jsvalue(const Persistent<S>& value) { return *value; }
		inline
		JSValue* jsvalue(JSValue* val) { return val; }
		inline
		JSValue* jsvalue(const Bool& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Float32& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Float64& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Int8& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Uint8& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Int16& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Uint16& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Int32& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Uint32& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Int64& v) { return worker->newInstance(v.value); }
		inline
		JSValue* jsvalue(const Uint64& v) { return worker->newInstance(v.value); }
		JSValue* jsvalue(const Dirent& val);
		JSValue* jsvalue(const FileStat& val);
		JSValue* jsvalue(const Array<Dirent>& val);
		JSValue* jsvalue(const Array<FileStat>& val);
		JSValue* jsvalue(const TouchPoint& val);
		JSValue* jsvalue(const Array<TouchPoint>& val);

		bool     parse(JSValue* in, WindowOptions& out, cChar* desc);
		bool     parse(JSValue* in, FillImageInit& out, cChar* desc);

	#define _Def_Fun(Name) \
		JSValue* jsvalue(const Name& value); \
		bool parse(JSValue* in, Name& out, cChar* err_msg = 0);
		Js_Types_Each(_Def_Fun);
	private:
		template<typedef T> T kind(JSObject* obj);
		Worker *worker;
	#define _Def_attr(Name) \
		Persistent<JSFunction> _parse##Name; \
		Persistent<JSFunction> _new##Name;
		Persistent<JSFunction> _TypesBase;
		Js_Types_Each(_Def_attr)
	#undef _Def_Fun
	#undef _Def_attr
	};

	struct JsConverter { // convert data to js value
		template<class T>
		static inline JSValue* Cast(Worker* worker, const Object& obj) {
			return worker->types()->newInstance( *static_cast<const T*>(&obj) );
		}
		template<class T>
		static JsConverter* Instance() {
			static JsConverter value{&Cast<T>};
			return &value;
		}
		JSValue* (*cast)(Worker* worker, const Object& object);
	};

} }
#endif
