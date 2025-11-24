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
#include "../../ui/views.h"
#include "../../ui/view/spine.h"
#include "../../render/bezier.h"
#include "../../util/fs.h"

namespace qk { 
	namespace js {

	#define Js_Parse_Type(Name, value, msg, ...) \
		Name out##__VA_ARGS__; \
		if ( !worker->types()->parse(value, out##__VA_ARGS__, msg)) return

	#define Js_Parse_Args(Name, argIdx, msg, ...) \
		Name arg##argIdx __VA_ARGS__; \
		if ( !worker->types()->parseArgs(args, argIdx, arg##argIdx, msg, sizeof(#__VA_ARGS__) > 1)) return

	typedef Window::Options WindowOptions;
	typedef FillImage::Init FillImageInit;
	typedef TouchEvent::TouchPoint TouchPoint;
	typedef Array<String> ArrayString;
	typedef Path* PathPtr;
	typedef Array<Vec2> ArrayVec2;
	typedef Array<Vec3> ArrayVec3;
	typedef Path::BorderRadius BorderRadius;
	typedef struct NativePtr_* NativePtr;

	#define Js_Types_Each(F) \
		F(bool) \
		F(float) \
		F(double) \
		F(int32_t) \
		F(uint32_t) \
		F(Color) \
		F(Vec2) \
		F(Vec3) \
		F(Vec4) \
		F(Rect) \
		F(Range) \
		F(Region) \
		F(Mat) \
		F(Mat4) \
		F(BorderRadius) \
		F(ArrayFloat) \
		F(ArrayString) \
		F(ArrayColor) \
		F(ArrayOrigin) \
		F(ArrayBorder) \
		F(ArrayVec2) \
		F(ArrayVec3) \
		F(String) \
		F(Curve) \
		F(Shadow) \
		F(Border) \
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
		F(CascadeColor) \
		F(SkeletonDataPtr) \
		F(TextStroke) \
		F(PathPtr) \
		F(Bounds) \

	class Qk_EXPORT TypesParser {
	public:
		TypesParser(Worker* worker, JSObject* exports);

		bool     isTypesBase(JSObject *arg);
		inline
		JSValue* jsvalue(int64_t val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(uint64_t val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(cString2& val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(cString4& val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(const Error& val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(const HttpError& val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(cDictSS& val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(Buffer& val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(Buffer&& val) { return worker->newValue(val);}
		template <class S>
		inline S* jsvalue(const Persistent<S>& value) { return *value; }
		inline
		JSValue* jsvalue(JSValue* val) { return val; }
		inline
		JSValue* jsvalue(Object *val) { return worker->newValue(val); }
		inline
		JSValue* jsvalue(const Bool& v) { return worker->newBool(v.value); }
		inline
		JSValue* jsvalue(const Float32& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Float64& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Int8& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Uint8& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Int16& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Uint16& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Int32& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Uint32& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Int64& v) { return worker->newValue(v.value); }
		inline
		JSValue* jsvalue(const Uint64& v) { return worker->newValue(v.value); }
		JSValue* jsvalue(const Dirent& val);
		JSValue* jsvalue(const FileStat& val);
		JSValue* jsvalue(const Array<Dirent>& val);
		JSValue* jsvalue(const Array<FileStat>& val);
		JSValue* jsvalue(const TouchPoint& val);
		JSValue* jsvalue(const Array<TouchPoint>& val);
		JSValue* jsvalue(const NativePtr& val); // jsvalue for pointer type

		bool parse(JSValue* in, WindowOptions& out, cChar* desc);
		bool parse(JSValue* in, FillImageInit& out, cChar* desc);
		bool parse(JSValue* in, NativePtr& out, cChar* msg); // parse pointer type
		bool parse(JSValue* in, WeakBuffer& out, cChar* msg);

		inline
		JSValue* jsvalue(const FFID& val) { return jsvalue((NativePtr)val); }
		inline
		bool parse(JSValue* in, FFID& out, cChar* msg) { return parse(in, (NativePtr&)out, msg); }

		template<typename T>
		bool parseArgs(FunctionArgs args, int argIdx, T& out, cChar* desc, bool optional = false) {
			if (argIdx >= args.length()) {
				if (optional) return true;
				Js_Throw(desc), false;
			}
			return parse(args[argIdx], out, desc);
		}

	#define _Def_Fun(Name) \
		JSValue* jsvalue(const Name& value); \
		bool parse(JSValue* in, Name& out, cChar* err_msg = 0);
		Js_Types_Each(_Def_Fun);
	#undef _Def_Fun
	private:
		template<typename T> T kind(JSObject* obj);
		Worker *worker;
	#define _Def_attr(Name) \
		Persistent<JSFunction> _parse##Name; \
		Persistent<JSFunction> _new##Name;
		Persistent<JSFunction> _TypesBase;
		Js_Types_Each(_Def_attr)
	#undef _Def_attr
		Strings* strs;
		template<class T> JSValue* jsValue(T val);
		template<class T> JSValue* jsvalueArray(const T& arr);
		template<class T> bool parseArray(JSValue* in, T& out, cChar* desc);
	};

	struct JsConverter { // convert data to js value
		template<class T>
		static inline JSValue* Cast(Worker* worker, const Object* obj) {
			return worker->types()->jsvalue( *static_cast<const T*>(obj) );
		}
		template<class T>
		static JsConverter* Instance() {
			static JsConverter value{&Cast<T>};
			return &value;
		}
		JSValue* (*cast)(Worker* worker, const Object* object);
	};

} }
#endif
