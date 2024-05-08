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

#include "./types.h"
#include "../render/font/font.h"
#include "../../out/native-inl-js.h"

namespace qk { namespace js {

	Strings::Strings(Worker* worker) {
		#define _Fun(name) \
			__##name##__.reset(worker, worker->newStringOneByte(#name));
		Js_Strings_Each(_Fun);
		__Errno__.reset(worker, worker->newStringOneByte("errno"));
		#undef _Fun
	}

	TypesParser::TypesParser(Worker* worker, JSObject* exports)
		: worker(worker)
	{
		#define OneByte(s) worker->newStringOneByte(s)
		#define _Fun(Name) \
		_parse##Name.reset(worker, exports->get(worker,OneByte("parse"#Name))->cast<JSFunction>()); \
		_new##Name.reset(worker, exports->get(worker,OneByte("new"#Name))->cast<JSFunction>());
		_TypesBase.reset(worker, exports->get(worker,OneByte("Base"))->cast<JSFunction>());

		Js_Types_Each(_Fun)
		Qk_DEBUG("Init types %s ok", "TypesParser");

		#undef OneByte
		#undef _Fun
	}

	bool TypesParser::isTypesBase(JSObject* arg) {
		return _TypesBase->instanceOf(worker, arg);
	}

	static void throw_error(Worker* worker, JSValue* value, cChar* msg, cChar* help = nullptr) {
		// Bad argument. Input.type = `Bad`
		// reference value, "

		String msg2 = String::format(msg ? msg : "`%s`", *value->toStringValue(worker) );
		JSValue* err;

		if (help) {
			err = worker->newTypeError("Bad argument. %s. Examples: %s", *msg2, help);
		} else {
			err = worker->newTypeError("Bad argument. %s.", *msg2);
		}
		worker->throwError(err);
	}

	void TypesParser::throwError(JSValue* value, cChar* msg, cChar* help) {
		throw_error(worker, value, msg, help);
	}

	// --------------------------------------------------------------------------------------------

	JSValue* TypesParser::jsvalue(const Dirent& dir) {
		auto rev = worker->newObject();
		rev->set(worker, worker->strs()->name(), jsvalue(dir.name));
		rev->set(worker, worker->strs()->pathname(), jsvalue(dir.pathname));
		rev->set(worker, worker->strs()->type(), jsvalue(dir.type));
		return rev;
	}

	JSValue* TypesParser::jsvalue(cArray<Dirent>& ls) {
		auto rev = worker->newArray(ls.length());
		if (ls.length()) {
			HandleScope scope(worker);
			for (int i = 0, e = ls.length(); i < e; i++) {
				rev->set(worker, i, jsvalue(ls[i]));
			}
		}
		return rev;
	}

	JSValue* TypesParser::jsvalue(const FileStat& stat) {
		auto func = worker->classsinfo()->getFunction(Js_Typeid(FileStat));
		Qk_ASSERT( func );
		auto r = func->newInstance(worker);
		*WrapObject::wrap<FileStat>(r)->self() = stat;
		return r;
	}

	JSValue* TypesParser::jsvalue(cArray<FileStat>& ls) {
		auto rev = worker->newArray(ls.length());
		if (ls.length()) {
			HandleScope scope(worker);
			for (int i = 0, e = ls.length(); i < e; i++) {
				rev->set(worker, i, jsvalue(ls[i]));
			}
		}
		return rev;
	}

	// --------------------------------------------------------------------------------------------

	JSValue* TypesParser::jsvalue(const bool& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::jsvalue(const float& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::jsvalue(const int32_t& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::jsvalue(const uint32_t& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::jsvalue(const Color& value) {
		JSValue* args[] = {
			worker->newInstance(value.r()),
			worker->newInstance(value.g()),
			worker->newInstance(value.b()),
			worker->newInstance(value.a()),
		};
		return _newColor->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Vec2& value) {
		JSValue* args[] = {
			worker->newInstance(value.x()),
			worker->newInstance(value.y()),
		};
		return _newVec2->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const Vec3& value) {
		JSValue* args[] = {
			worker->newInstance(value.x()),
			worker->newInstance(value.y()),
			worker->newInstance(value.z()),
		};
		return _newVec3->call(worker, 3, args);
	}

	JSValue* TypesParser::jsvalue(const Vec4& value) {
		JSValue* args[] = {
			worker->newInstance(value.x()),
			worker->newInstance(value.y()),
			worker->newInstance(value.z()),
			worker->newInstance(value.w()),
		};
		return _newVec4->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Rect& value) {
		JSValue* args[] = {
			worker->newInstance(value.origin.x()),
			worker->newInstance(value.origin.y()),
			worker->newInstance(value.size.width()),
			worker->newInstance(value.size.height()),
		};
		return _newRect->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Mat& value) {
		JSValue* args[] = {
			worker->newInstance(value[0]),
			worker->newInstance(value[1]),
			worker->newInstance(value[2]),
			worker->newInstance(value[3]),
			worker->newInstance(value[4]),
			worker->newInstance(value[5]),
		};
		return _newMat->call(worker, 6, args);
	}

	JSValue* TypesParser::jsvalue(const Mat4& value) {
		JSValue* args[] = {
			worker->newInstance(value[0]),
			worker->newInstance(value[1]),
			worker->newInstance(value[2]),
			worker->newInstance(value[3]),
			worker->newInstance(value[4]),
			worker->newInstance(value[5]),
			worker->newInstance(value[6]),
			worker->newInstance(value[7]),
			worker->newInstance(value[8]),
			worker->newInstance(value[9]),
			worker->newInstance(value[10]),
			worker->newInstance(value[11]),
			worker->newInstance(value[12]),
			worker->newInstance(value[13]),
			worker->newInstance(value[14]),
			worker->newInstance(value[15]),
		};
		return _newMat4->call(worker, 16, args);
	}

	JSValue* TypesParser::jsvalue(const ArrayFloat& value) {
		auto arr = worker->newArray(value.length());
		for (int i = 0; i < value.length(); i ++) {
			arr->set(worker, i, worker->newInstance(value[i]));
		}
		return arr;
	}

	JSValue* TypesParser::jsvalue(const ArrayColor& value) {
		auto arr = worker->newArray(value.length());
		for (int i = 0; i < value.length(); i ++) {
			arr->set(worker, i, jsvalue(value[i]));
		}
		return arr;
	}

	JSValue* TypesParser::jsvalue(const String& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::jsvalue(cCurve& value) {
		JSValue* args[] = {
			worker->newInstance(value.p0().x()),
			worker->newInstance(value.p0().y()),
			worker->newInstance(value.p1().x()),
			worker->newInstance(value.p1().y()),
			worker->newInstance(value.p2().x()),
			worker->newInstance(value.p2().y()),
			worker->newInstance(value.p3().x()),
			worker->newInstance(value.p3().y()),
		};
		return _newCurve->call(worker, 8, args);
	}

	JSValue* TypesParser::jsvalue(const Shadow& value) {
		JSValue* args[] = {
			worker->newInstance(value.offset_x),
			worker->newInstance(value.offset_y),
			worker->newInstance(value.size),
			worker->newInstance(value.color.b()),
			worker->newInstance(value.color.g()),
			worker->newInstance(value.color.b()),
			worker->newInstance(value.color.a()),
		};
		return _newShadow->call(worker, 7, args);
	}

	JSValue* TypesParser::jsvalue(const FillPosition& val) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newFillPosition->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const FillSize& val) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newFillSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const Repeat& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const BoxFilterPtr& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::jsvalue(const BoxShadowPtr& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::jsvalue(const Direction& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const ItemsAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const CrossAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const Wrap& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const WrapAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const Align& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const BoxSize& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newBoxSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const BoxOrigin& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newBoxOrigin->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextDecoration& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextOverflow& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWhiteSpace& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWordBreak& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextColor& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value.r()),
			worker->newInstance(value.value.g()),
			worker->newInstance(value.value.b()),
			worker->newInstance(value.value.a()),
		};
		return _newTextColor->call(worker, 5, args);
	}

	JSValue* TypesParser::jsvalue(const TextSize& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newTextSize->call(worker, 1, args);
	}

	JSValue* TypesParser::jsvalue(const TextShadow& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value.offset_x),
			worker->newInstance(value.value.offset_y),
			worker->newInstance(value.value.size),
			worker->newInstance(value.value.color.r()),
			worker->newInstance(value.value.color.g()),
			worker->newInstance(value.value.color.b()),
			worker->newInstance(value.value.color.a()),
		};
		return _newTextShadow->call(worker, 8, args);
	}

	JSValue* TypesParser::jsvalue(const TextFamily& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			jsvalue(value.value),
		};
		return _newTextFamily->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextWeight& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWidth& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextSlant& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FontStyle& value) {
		return worker->newInstance(value.value());
	}

	JSValue* TypesParser::jsvalue(const KeyboardType& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const KeyboardReturnType& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const CursorStyle& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FindDirection& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FFID& val) {
		cChar* addr = reinterpret_cast<cChar*>(&val);
		Buffer buffer(sizeof(FFID));
		buffer.write(addr, sizeof(FFID), 0);
		return worker->newInstance(buffer);
	}

	// --------------------------------------------------------------------------------------------

	#define js_parse(Type, ok) { \
		JSObject* obj;\
		JSValue* val;\
		if (desc) {\
			JSValue* args[] = { in, worker->newInstance(desc)->cast() };\
			val = _parse##Type->call(worker, 2, args);\
		} else {\
			val = _parse##Type->call(worker, 1, &in);\
		}\
		if ( !val ) return false;\
		obj = val->cast<JSObject>();\
		ok \
		return true;\
	}

	template<typedef T>
	T TypesParser::kind(JSObject* obj) {
		return (T)obj->get(worker, worker->strs()->kind())->toUint32Value(worker);
	}

	bool TypesParser::parse(JSValue* in, WindowOptions& _out, cChar* desc) {
		if (!in->isObject()) {
			return throw_error(worker, in, desc), false;
		}
		auto obj = in->cast<JSObject>();
		auto colorType = obj->get(worker, worker->newStringOneByte("colorType"));
		auto msaa = obj->get(worker, worker->newStringOneByte("msaa"));
		auto fps = obj->get(worker, worker->newStringOneByte("fps"));
		auto frame = obj->get(worker, worker->newStringOneByte("frame"));
		auto title = obj->get(worker, worker->newStringOneByte("title"));
		auto backgroundColor = obj->get(worker, worker->newStringOneByte("backgroundColor"));

		if (!colorType->isUndefined()) {
			Js_Parse_Type(uint32_t, colorType, "Window::Options{.colorType=%s}") false;
			_out.colorType = ColorType(out);
		}
		if (!msaa->isUndefined()) {
			Js_Parse_Type(uint32_t, msaa, "Window::Options{.msaa=%s}") false;
			_out.msaa = out;
		}
		if (!fps->isUndefined()) {
			Js_Parse_Type(uint32_t, fps, "Window::Options{.fps=%s}") false;
			_out.fps = out;
		}
		if (!frame->isUndefined()) {
			Js_Parse_Type(Rect, frame, "Window::Options{.frame=%s}") false;
			_out.frame = out;
		}
		if (!title->isUndefined()) {
			Js_Parse_Type(String, title, "Window::Options{.title=%s}") false;
			_out.title = out;
		}
		if (!backgroundColor->isUndefined()) {
			Js_Parse_Type(Color, backgroundColor, "Window::Options{.backgroundColor=%s}") false;
			_out.backgroundColor = out;
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, FillImageInit& out, cChar* desc) {
		if (!in->isObject()) {
			return throw_error(worker, in, desc), false;
		}
		auto size_x = obj->get(worker, worker->newStringOneByte("size_x"));
		auto size_y = obj->get(worker, worker->newStringOneByte("size_y"));
		auto position_x = obj->get(worker, worker->newStringOneByte("position_x"));
		auto position_y = obj->get(worker, worker->newStringOneByte("position_y"));
		auto repeat = obj->get(worker, worker->newStringOneByte("repeat"));

		if (!size_x->isUndefined()) {
			Js_Parse_Type(FillSize, size_x, desc) false;
			out.size_x = out;
		}
		if (!size_y->isUndefined()) {
			Js_Parse_Type(FillSize, size_y, desc) false;
			out.size_y = out;
		}
		if (!position_x->isUndefined()) {
			Js_Parse_Type(FillPosition, position_x, desc) false;
			out.position_x = out;
		}
		if (!position_y->isUndefined()) {
			Js_Parse_Type(FillPosition, position_y, desc) false;
			out.position_y = out;
		}
		if (!repeat->isUndefined()) {
			Js_Parse_Type(Repeat, repeat, desc) false;
			out.repeat = out;
		}
		return true;
	}

	// --------------------------------------------------------------------------------------------

	bool TypesParser::parse(JSValue* in, bool& out, cChar* desc) {
		out = in->toBooleanValue(worker);
		return true;
	}

	bool TypesParser::parse(JSValue* in, float& out, cChar* desc) {
		if (in->isNumber()) {
			out = in->toNumberValue(worker);
			return true;
		}
		if (in->isString()) {
			if (in->toStringValue(worker).toNumber<float>(&out)) {
				return true;
			}
		}
		throw_error(worker, in, desc);
		return false;
	}

	bool TypesParser::parse(JSValue* in, int32_t& out, cChar* desc) {
		if (in->isNumber()) {
			out = in->toInt32Value(worker);
			return true;
		}
		if (in->isString()) {
			if (in->toStringValue(worker).toNumber<int>(&out)) {
				return true;
			}
		}
		throw_error(worker, in, desc);
		return false;
	}

	bool TypesParser::parse(JSValue* in, uint32_t& out, cChar* desc) {
		if (in->isNumber()) {
			out = in->toUint32Value(worker);
			return true;
		}
		if (in->isString()) {
			if (in->toStringValue(worker).toNumber<uint32_t>(&out)) {
				return true;
			}
		}
		throw_error(worker, in, desc);
		return false;
	}

	bool TypesParser::parse(JSValue* in, Color& out, cChar* desc) {
		js_parse(Color, {
			out.set_r(obj->get(worker, worker->strs()->r())->toUint32Value(worker));
			out.set_g(obj->get(worker, worker->strs()->g())->toUint32Value(worker));
			out.set_b(obj->get(worker, worker->strs()->b())->toUint32Value(worker));
			out.set_a(obj->get(worker, worker->strs()->a())->toUint32Value(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, Vec2& out, cChar* desc) {
		js_parse(Vec2, {
			out.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, Vec3& out, cChar* desc) {
		js_parse(Vec3, {
			out.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
			out.set_z(obj->get(worker, worker->strs()->z())->toNumberValue(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, Vec4& out, cChar* desc) {
		js_parse(Vec4, {
			out.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
			out.set_z(obj->get(worker, worker->strs()->z())->toNumberValue(worker));
			out.set_w(obj->get(worker, worker->strs()->w())->toNumberValue(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, Rect& out, cChar* desc) {
		js_parse(Rect, {
			out.origin.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.origin.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
			out.size.set_width(obj->get(worker, worker->strs()->width())->toNumberValue(worker));
			out.size.set_height(obj->get(worker, worker->strs()->height())->toNumberValue(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, Mat& out, cChar* desc) {
		js_parse(Mat, {
			auto mat = obj->get(worker, worker->strs()->_value())->cast<JSArray>();
			out[0] = mat->get(worker, 0u)->toNumberValue(worker);
			out[1] = mat->get(worker, 1)->toNumberValue(worker);
			out[2] = mat->get(worker, 2)->toNumberValue(worker);
			out[3] = mat->get(worker, 3)->toNumberValue(worker);
			out[4] = mat->get(worker, 4)->toNumberValue(worker);
			out[5] = mat->get(worker, 5)->toNumberValue(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, Mat4& out, cChar* desc) {
		js_parse(Mat4, {
			auto mat = obj->get(worker, worker->strs()->_value())->cast<JSArray>();
			out[0] = mat->get(worker, 0u)->toNumberValue(worker);
			out[1] = mat->get(worker, 1)->toNumberValue(worker);
			out[2] = mat->get(worker, 2)->toNumberValue(worker);
			out[3] = mat->get(worker, 3)->toNumberValue(worker);
			out[4] = mat->get(worker, 4)->toNumberValue(worker);
			out[5] = mat->get(worker, 5)->toNumberValue(worker);
			out[6] = mat->get(worker, 6)->toNumberValue(worker);
			out[7] = mat->get(worker, 7)->toNumberValue(worker);
			out[8] = mat->get(worker, 8)->toNumberValue(worker);
			out[9] = mat->get(worker, 9)->toNumberValue(worker);
			out[10] = mat->get(worker, 10)->toNumberValue(worker);
			out[11] = mat->get(worker, 11)->toNumberValue(worker);
			out[12] = mat->get(worker, 12)->toNumberValue(worker);
			out[13] = mat->get(worker, 13)->toNumberValue(worker);
			out[14] = mat->get(worker, 14)->toNumberValue(worker);
			out[15] = mat->get(worker, 15)->toNumberValue(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, ArrayFloat& out, cChar* desc) {
		auto arr = in->cast<JSArray>();
		if (!arr->isArray()) {
			return throw_error(worker, in, desc), false;
		}
		out.reset(arr->length());
		for (uint32_t i = 0; i < out.length(); i++) {
			auto it = arr->get(worker, i);
			if (!it->isNumber()) {
				return throw_error(worker, in, desc), false;
			}
			out[i] = it->toNumberValue(worker);
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, ArrayColor& out, cChar* desc) {
		auto arr = in->cast<JSArray>();
		if (!arr->isArray()) {
			return throw_error(worker, in, desc), false;
		}
		out.reset(arr->length());
		for (uint32_t i = 0; i < out.length(); i++) {
			if (!parse(arr->get(worker, i), out[i], desc)) {
				return false;
			}
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, String& out, cChar* desc) {
		out = in->toStringValue(worker);
		return true;
	}

	bool TypesParser::parse(JSValue* in, Curve& out, cChar* desc) {
		static const Dict<String, cCurve*> CURCEs({
			{"linear", &LINEAR },
			{"ease", &EASE },
			{"ease_in", &EASE_IN },
			{"ease_out", &EASE_OUT },
			{"ease_in_out", &EASE_IN_OUT },
		});
		if ( in->isString() && CURCEs.get(in->toStringValue(worker,true), out)) {
			return true;
		}
		js_parse(Curve, {
			out = Curve(
				obj->get(worker, worker->strs()->point1X())->toNumberValue(worker),
				obj->get(worker, worker->strs()->point1Y())->toNumberValue(worker),
				obj->get(worker, worker->strs()->point2X())->toNumberValue(worker),
				obj->get(worker, worker->strs()->point2Y())->toNumberValue(worker)
			);
		});
	}

	bool TypesParser::parse(JSValue* in, Shadow& out, cChar* desc) {
		js_parse(Shadow, {
			out.offset_x = obj->get(worker, worker->strs()->offsetX())->toNumberValue(worker);
			out.offset_y = obj->get(worker, worker->strs()->offsetY())->toNumberValue(worker);
			out.size = obj->get(worker, worker->strs()->size())->toNumberValue(worker);
			out.color.set_r(obj->get(worker, worker->strs()->r())->toUint32Value(worker));
			out.color.set_g(obj->get(worker, worker->strs()->g())->toUint32Value(worker));
			out.color.set_b(obj->get(worker, worker->strs()->b())->toUint32Value(worker));
			out.color.set_a(obj->get(worker, worker->strs()->a())->toUint32Value(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, FillPosition& out, cChar* desc) {
		js_parse(FillPosition, {
			out.kind = kind<FillPositionKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->toNumberValue(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, FillSize& out, cChar* desc) {
		js_parse(FillSize, {
			out.kind = kind<FillSizeKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->toNumberValue(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, Repeat& out, cChar* desc) {
		js_parse(Repeat, {
			out = (Repeat)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, BoxFilterPtr& out, cChar* desc) {
		js_parse(BoxFilterPtr, {
			out = WrapObject::wrap<BoxFilter>(obj)->self();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxShadowPtr& out, cChar* desc) {
		js_parse(BoxFilterPtr, {
			out = WrapObject::wrap<BoxShadow>(obj)->self();
		});
	}

	bool TypesParser::parse(JSValue* in, Direction& out, cChar* desc) {
		js_parse(Direction, {
			out = (Direction)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, ItemsAlign& out, cChar* desc) {
		js_parse(ItemsAlign, {
			out = (ItemsAlign)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, CrossAlign& out, cChar* desc) {
		js_parse(CrossAlign, {
			out = (CrossAlign)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, Wrap& out, cChar* desc) {
		js_parse(Wrap, {
			out = (Wrap)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, WrapAlign& out, cChar* desc) {
		js_parse(WrapAlign, {
			out = (WrapAlign)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, Align& out, cChar* desc) {
		js_parse(Align, {
			out = (Align)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, BoxSize& out, cChar* desc) {
		js_parse(BoxSize, {
			out.kind = kind<BoxSizeKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->toNumberValue(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, BoxOrigin& out, cChar* desc) {
		js_parse(BoxOrigin, {
			out.kind = kind<BoxOriginKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->toNumberValue(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextAlign& out, cChar* desc) {
		js_parse(TextAlign, {
			out = (TextAlign)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextDecoration& out, cChar* desc) {
		js_parse(TextDecoration, {
			out = (TextDecoration)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextOverflow& out, cChar* desc) {
		js_parse(TextOverflow, {
			out = (TextOverflow)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextWhiteSpace& out, cChar* desc) {
		js_parse(TextWhiteSpace, {
			out = (TextWhiteSpace)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextWordBreak& out, cChar* desc) {
		js_parse(TextWordBreak, {
			out = (TextWordBreak)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextColor& out, cChar* desc) {
		js_parse(TextColor, {
			out.kind = kind<TextValueKind>(obj);
			out.value.set_r(obj->get(worker, worker->strs()->r())->toUint32Value(worker));
			out.value.set_g(obj->get(worker, worker->strs()->g())->toUint32Value(worker));
			out.value.set_b(obj->get(worker, worker->strs()->b())->toUint32Value(worker));
			out.value.set_a(obj->get(worker, worker->strs()->a())->toUint32Value(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, TextSize& out, cChar* desc) {
		if (in->isNumber()) {
			out.kind = TextValueKind::kValue;
			out.value = in->toNumberValue(worker);
			return true;
		}
		js_parse(TextSize, {
			out.kind = kind<TextValueKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextShadow& out, cChar* desc) {
		js_parse(TextShadow, {
			out.kind = kind<TextValueKind>(obj);
			out.value.offset_x = obj->get(worker, worker->strs()->offsetX())->toNumberValue(worker);
			out.value.offset_y = obj->get(worker, worker->strs()->offsetY())->toNumberValue(worker);
			out.value.size = obj->get(worker, worker->strs()->size())->toNumberValue(worker);
			out.value.color.set_r(obj->get(worker, worker->strs()->r())->toUint32Value(worker));
			out.value.color.set_g(obj->get(worker, worker->strs()->g())->toUint32Value(worker));
			out.value.color.set_b(obj->get(worker, worker->strs()->b())->toUint32Value(worker));
			out.value.color.set_a(obj->get(worker, worker->strs()->a())->toUint32Value(worker));
		});
	}

	bool TypesParser::parse(JSValue* in, TextFamily& out, cChar* desc) {
		js_parse(TextFamily, {
			out.kind = kind<TextValueKind>(obj);
			return parse(obj->get(worker, worker->strs()->value()), out.value, desc);
		});
	}

	bool TypesParser::parse(JSValue* in, TextWeight& out, cChar* desc) {
		js_parse(TextWeight, {
			out = (TextWeight)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextWidth& out, cChar* desc) {
		js_parse(TextWidth, {
			out = (TextWidth)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, TextSlant& out, cChar* desc) {
		js_parse(TextSlant, {
			out = (TextSlant)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, FontStyle& out, cChar* desc) {
		if (!in->isUint32()) {
			return throw_error(worker, in, desc), false;
		}
		struct Out {
			uint32_t value;
		} out = { in->toUint32Value(worker) };
		out = *reinterpret_cast<Out*>(&out);
		return true;
	}

	bool TypesParser::parse(JSValue* in, KeyboardType& out, cChar* desc) {
		js_parse(KeyboardType, {
			out = (KeyboardType)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, KeyboardReturnType& out, cChar* desc) {
		js_parse(KeyboardReturnType, {
			out = (KeyboardReturnType)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, CursorStyle& out, cChar* desc) {
		js_parse(CursorStyle, {
			out = (CursorStyle)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, FindDirection& out, cChar* desc) {
		js_parse(FindDirection, {
			out = (FindDirection)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parse(JSValue* in, FFID& out, cChar* desc) {
		auto ffid = in->asBuffer(worker);
		if (ffid.length() < sizeof(FFID)) {
			return throw_error(worker, in, desc), false;
		}
		out = reinterpret_cast<const FFID>(ffid.val());
		return true;
	}

	class NativeTypes: public Worker {
	public:
		void setTypesParser(TypesParser *types) {
			_types = types;
		}
		static void binding(JSObject* exports, Worker* worker) {
			{
				TryCatch try_catch(worker);
				if (!worker->runNativeScript(WeakBuffer((Char*)
							native_js::INL_native_js_code__types_,
							native_js::INL_native_js_code__types_count_).buffer(), "_types.js", exports)) {
					if ( try_catch.hasCaught() ) {
						worker->reportException(&try_catch);
					}
					Qk_FATAL("Could not initialize native _types.js");
				}
			}
			static_cast<NativeTypes*>(worker)->setTypesParser(new TypesParser(worker, exports));
		}
	};

	Js_Set_Module(_types, NativeTypes);
} }
