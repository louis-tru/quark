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
#include "../../ui/view/view.h"
#include "../../render/font/families.h"
#include "../../../out/native-inl-js.h"

namespace qk { namespace js {

	TypesParser::TypesParser(Worker* worker, JSObject* exports)
		: worker(worker)
	{
		#define OneByte(s) worker->newStringOneByte(s)
		#define _Fun(Name) \
		_parse##Name.reset(worker, exports->get<JSFunction>(worker,OneByte("parse"#Name))); \
		_new##Name.reset(worker, exports->get<JSFunction>(worker,OneByte("new"#Name)));
		///
		_TypesBase.reset(worker, exports->get<JSFunction>(worker,OneByte("Base")));

		Js_Types_Each(_Fun)
		Qk_DLog("Init types %s ok", "TypesParser");

		#undef OneByte
		#undef _Fun
	}

	bool TypesParser::isTypesBase(JSObject* arg) {
		return _TypesBase->instanceOf(worker, arg);
	}

	static void throw_error(Worker* worker, JSValue* value, cChar* msg, cChar* help = nullptr) {
		// Bad argument. Input.type = `Bad`
		// reference value, "

		String msg2 = String::format(msg ? msg : "`%s`", *value->toString(worker)->value(worker) );
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
		auto rev = worker->newArray();
		if (ls.length()) {
			HandleScope scope(worker);
			for (int i = 0, e = ls.length(); i < e; i++) {
				rev->set(worker, i, jsvalue(ls[i]));
			}
		}
		return rev;
	}

	JSValue* TypesParser::jsvalue(const FileStat& stat) {
		auto func = worker->classes()->getFunction(Js_Typeid(FileStat));
		Qk_ASSERT( func );
		auto r = func->newInstance(worker);
		*MixObject::mix<FileStat>(r)->self() = stat;
		return r;
	}

	JSValue* TypesParser::jsvalue(cArray<FileStat>& ls) {
		auto rev = worker->newArray();
		if (ls.length()) {
			HandleScope scope(worker);
			for (int i = 0, e = ls.length(); i < e; i++) {
				rev->set(worker, i, jsvalue(ls[i]));
			}
		}
		return rev;
	}

	JSValue* TypesParser::jsvalue(const TouchPoint& val) {
		auto rv = worker->newObject();
		auto view = MixObject::mix(val.view);
		rv->set(worker,worker->strs()->id(), worker->newValue(val.id));
		rv->set(worker,worker->strs()->startX(), worker->newValue(val.start_x));
		rv->set(worker,worker->strs()->startY(), worker->newValue(val.start_y));
		rv->set(worker,worker->strs()->x(), worker->newValue(val.x));
		rv->set(worker,worker->strs()->y(), worker->newValue(val.y));
		rv->set(worker,worker->strs()->force(), worker->newValue(val.force));
		rv->set(worker,worker->strs()->clickIn(), worker->newBool(val.click_in));
		rv->set(worker,worker->strs()->view(), view->handle());
		return rv;
	}

	JSValue* TypesParser::jsvalue(const Array<TouchPoint>& val) {
		auto arr = worker->newArray();
		int j = 0;
		for ( auto& i : val ) {
			arr->set(worker, j++, jsvalue(i));
		}
		return arr;
	}

	// --------------------------------------------------------------------------------------------

	JSValue* TypesParser::jsvalue(const bool& value) {
		return worker->newBool(value);
	}

	JSValue* TypesParser::jsvalue(const float& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(const int32_t& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(const uint32_t& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(const Color& value) {
		JSValue* args[] = {
			worker->newValue(value.r()),
			worker->newValue(value.g()),
			worker->newValue(value.b()),
			worker->newValue(value.a()),
		};
		return _newColor->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Vec2& value) {
		JSValue* args[] = {
			worker->newValue(value.x()),
			worker->newValue(value.y()),
		};
		return _newVec2->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const Vec3& value) {
		JSValue* args[] = {
			worker->newValue(value.x()),
			worker->newValue(value.y()),
			worker->newValue(value.z()),
		};
		return _newVec3->call(worker, 3, args);
	}

	JSValue* TypesParser::jsvalue(const Vec4& value) {
		JSValue* args[] = {
			worker->newValue(value.x()),
			worker->newValue(value.y()),
			worker->newValue(value.z()),
			worker->newValue(value.w()),
		};
		return _newVec4->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Rect& value) {
		JSValue* args[] = {
			worker->newValue(value.origin.x()),
			worker->newValue(value.origin.y()),
			worker->newValue(value.size.width()),
			worker->newValue(value.size.height()),
		};
		return _newRect->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Mat& value) {
		JSValue* args[] = {
			worker->newValue(value[0]),
			worker->newValue(value[1]),
			worker->newValue(value[2]),
			worker->newValue(value[3]),
			worker->newValue(value[4]),
			worker->newValue(value[5]),
		};
		return _newMat->call(worker, 6, args);
	}

	JSValue* TypesParser::jsvalue(const Mat4& value) {
		JSValue* args[] = {
			worker->newValue(value[0]),
			worker->newValue(value[1]),
			worker->newValue(value[2]),
			worker->newValue(value[3]),
			worker->newValue(value[4]),
			worker->newValue(value[5]),
			worker->newValue(value[6]),
			worker->newValue(value[7]),
			worker->newValue(value[8]),
			worker->newValue(value[9]),
			worker->newValue(value[10]),
			worker->newValue(value[11]),
			worker->newValue(value[12]),
			worker->newValue(value[13]),
			worker->newValue(value[14]),
			worker->newValue(value[15]),
		};
		return _newMat4->call(worker, 16, args);
	}

	JSValue* TypesParser::jsvalue(const ArrayFloat& value) {
		auto arr = worker->newArray(/*value.length()*/);
		for (int i = 0; i < value.length(); i ++) {
			arr->set(worker, i, worker->newValue(value[i]));
		}
		return arr;
	}

	JSValue* TypesParser::jsvalue(const ArrayString& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(const ArrayColor& value) {
		auto arr = worker->newArray(/*value.length()*/);
		for (int i = 0; i < value.length(); i ++) {
			arr->set(worker, i, jsvalue(value[i]));
		}
		return arr;
	}

	JSValue* TypesParser::jsvalue(const ArrayOrigin& value) {
		auto arr = worker->newArray(/*value.length()*/);
		for (int i = 0; i < value.length(); i ++) {
			arr->set(worker, i, jsvalue(value[i]));
		}
		return arr;
	}

	JSValue* TypesParser::jsvalue(const ArrayBorder& value) {
		auto arr = worker->newArray(/*value.length()*/);
		for (int i = 0; i < value.length(); i ++) {
			arr->set(worker, i, jsvalue(value[i]));
		}
		return arr;
	}

	JSValue* TypesParser::jsvalue(const String& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(cCurve& value) {
		JSValue* args[] = {
			worker->newValue(value.p1().x()),
			worker->newValue(value.p1().y()),
			worker->newValue(value.p2().x()),
			worker->newValue(value.p2().y()),
		};
		return _newCurve->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Shadow& value) {
		JSValue* args[] = {
			worker->newValue(value.x),
			worker->newValue(value.y),
			worker->newValue(value.size),
			worker->newValue(value.color.r()),
			worker->newValue(value.color.g()),
			worker->newValue(value.color.b()),
			worker->newValue(value.color.a()),
		};
		return _newShadow->call(worker, 7, args);
	}

	JSValue* TypesParser::jsvalue(const BoxBorder& value) {
		JSValue* args[] = {
			worker->newValue(value.width),
			worker->newValue(value.color.r()),
			worker->newValue(value.color.g()),
			worker->newValue(value.color.b()),
			worker->newValue(value.color.a()),
		};
		return _newBoxBorder->call(worker, 5, args);
	}

	JSValue* TypesParser::jsvalue(const FillPosition& val) {
		JSValue* args[] = {
			worker->newValue((uint32_t)val.kind),
			worker->newValue(val.value),
		};
		return _newFillPosition->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const FillSize& val) {
		JSValue* args[] = {
			worker->newValue((uint32_t)val.kind),
			worker->newValue(val.value),
		};
		return _newFillSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const Repeat& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const BoxFilterPtr& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(const BoxShadowPtr& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(const Direction& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const ItemsAlign& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const CrossAlign& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const Wrap& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const WrapAlign& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const Align& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const BoxSize& value) {
		JSValue* args[] = {
			worker->newValue((uint32_t)value.kind),
			worker->newValue(value.value),
		};
		return _newBoxSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const BoxOrigin& value) {
		JSValue* args[] = {
			worker->newValue((uint32_t)value.kind),
			worker->newValue(value.value),
		};
		return _newBoxOrigin->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextAlign& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextDecoration& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextOverflow& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWhiteSpace& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWordBreak& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextColor& value) {
		JSValue* args[] = {
			worker->newValue((uint32_t)value.kind),
			worker->newValue(value.value.r()),
			worker->newValue(value.value.g()),
			worker->newValue(value.value.b()),
			worker->newValue(value.value.a()),
		};
		return _newTextColor->call(worker, 5, args);
	}

	JSValue* TypesParser::jsvalue(const TextSize& value) {
		JSValue* args[] = {
			worker->newValue((uint32_t)value.kind),
			worker->newValue(value.value),
		};
		return _newTextSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextShadow& value) {
		JSValue* args[] = {
			worker->newValue((uint32_t)value.kind),
			worker->newValue(value.value.x),
			worker->newValue(value.value.y),
			worker->newValue(value.value.size),
			worker->newValue(value.value.color.r()),
			worker->newValue(value.value.color.g()),
			worker->newValue(value.value.color.b()),
			worker->newValue(value.value.color.a()),
		};
		return _newTextShadow->call(worker, 8, args);
	}

	JSValue* TypesParser::jsvalue(const TextFamily& value) {
		JSValue* args[] = {
			worker->newValue((uint32_t)value.kind),
			jsvalue(value.value),
		};
		return _newTextFamily->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextWeight& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWidth& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextSlant& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FontStyle& value) {
		return worker->newValue(value.value());
	}

	JSValue* TypesParser::jsvalue(const KeyboardType& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const KeyboardReturnType& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const CursorStyle& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FindDirection& value) {
		return worker->newValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FFID& val) {
		cChar* addr = reinterpret_cast<cChar*>(&val);
		Buffer buffer(sizeof(FFID));
		buffer.write(addr, sizeof(FFID), 0);
		return worker->newValue(buffer);
	}

	// --------------------------------------------------------------------------------------------

	#define js_parse(Type, block) { \
		JSObject* obj;\
		JSValue* val;\
		if (desc) {\
			JSValue* args[] = { in, worker->newValue(String(desc))->cast() };\
			val = _parse##Type->call(worker, 2, args);\
		} else {\
			val = _parse##Type->call(worker, 1, &in);\
		}\
		if ( !val ) return throw_error(worker, in, desc), false;\
		obj = val->cast<JSObject>();\
		block \
		return true;\
	}

	template<typename T>
	T TypesParser::kind(JSObject* obj) {
		return (T)obj->get(worker, worker->strs()->kind())->toUint32(worker)->value();
	}

	bool TypesParser::parse(JSValue* in, WindowOptions& _out, cChar* desc) {
		if (!in->isObject()) {
			return throw_error(worker, in, desc), false;
		}
		_out = {.backgroundColor=Color(255,255,255)};
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

	bool TypesParser::parse(JSValue* in, FillImageInit& out_, cChar* desc) {
		if (!in->isObject()) {
			return throw_error(worker, in, desc), false;
		}
		auto obj = in->cast<JSObject>();
		auto width = obj->get(worker, worker->newStringOneByte("width"));
		auto height = obj->get(worker, worker->newStringOneByte("height"));
		auto x = obj->get(worker, worker->newStringOneByte("x"));
		auto y = obj->get(worker, worker->newStringOneByte("y"));
		auto repeat = obj->get(worker, worker->newStringOneByte("repeat"));

		if (!width->isUndefined()) {
			Js_Parse_Type(FillSize, width, desc) false;
			out_.width = out;
		}
		if (!height->isUndefined()) {
			Js_Parse_Type(FillSize, height, desc) false;
			out_.height = out;
		}
		if (!x->isUndefined()) {
			Js_Parse_Type(FillPosition, x, desc) false;
			out_.x = out;
		}
		if (!y->isUndefined()) {
			Js_Parse_Type(FillPosition, y, desc) false;
			out_.y = out;
		}
		if (!repeat->isUndefined()) {
			Js_Parse_Type(Repeat, repeat, desc) false;
			out_.repeat = out;
		}
		return true;
	}

	// --------------------------------------------------------------------------------------------

	bool TypesParser::parse(JSValue* in, bool& out, cChar* desc) {
		out = in->toBoolean(worker);
		return true;
	}

	bool TypesParser::parse(JSValue* in, float& out, cChar* desc) {
		auto num = in->toNumber(worker);
		if (!num)
			return throw_error(worker, in, desc), false;
		out = num->float32();
		return true;
	}

	bool TypesParser::parse(JSValue* in, int32_t& out, cChar* desc) {
		auto num = in->toInt32(worker);
		if (!num)
			return throw_error(worker, in, desc), false;
		out = num->value();
		return true;
	}

	bool TypesParser::parse(JSValue* in, uint32_t& out, cChar* desc) {
		auto num = in->toUint32(worker);
		if (!num)
			return throw_error(worker, in, desc), false;
		out = num->value();
		return true;
	}

	bool TypesParser::parse(JSValue* in, Color& out, cChar* desc) {
		js_parse(Color, {
			out.set_r(obj->get(worker, worker->strs()->r())->toUint32(worker)->value());
			out.set_g(obj->get(worker, worker->strs()->g())->toUint32(worker)->value());
			out.set_b(obj->get(worker, worker->strs()->b())->toUint32(worker)->value());
			out.set_a(obj->get(worker, worker->strs()->a())->toUint32(worker)->value());
		});
	}

	bool TypesParser::parse(JSValue* in, Vec2& out, cChar* desc) {
		js_parse(Vec2, {
			out.set_x(obj->get(worker, worker->strs()->x())->cast<JSNumber>()->float32());
			out.set_y(obj->get(worker, worker->strs()->y())->cast<JSNumber>()->float32());
		});
	}

	bool TypesParser::parse(JSValue* in, Vec3& out, cChar* desc) {
		js_parse(Vec3, {
			out.set_x(obj->get(worker, worker->strs()->x())->cast<JSNumber>()->float32());
			out.set_y(obj->get(worker, worker->strs()->y())->cast<JSNumber>()->float32());
			out.set_z(obj->get(worker, worker->strs()->z())->cast<JSNumber>()->float32());
		});
	}

	bool TypesParser::parse(JSValue* in, Vec4& out, cChar* desc) {
		js_parse(Vec4, {
			out.set_x(obj->get(worker, worker->strs()->x())->cast<JSNumber>()->float32());
			out.set_y(obj->get(worker, worker->strs()->y())->cast<JSNumber>()->float32());
			out.set_z(obj->get(worker, worker->strs()->z())->cast<JSNumber>()->float32());
			out.set_w(obj->get(worker, worker->strs()->w())->cast<JSNumber>()->float32());
		});
	}

	bool TypesParser::parse(JSValue* in, Rect& out, cChar* desc) {
		js_parse(Rect, {
			out.origin.set_x(obj->get(worker, worker->strs()->x())->cast<JSNumber>()->float32());
			out.origin.set_y(obj->get(worker, worker->strs()->y())->cast<JSNumber>()->float32());
			out.size.set_width(obj->get(worker, worker->strs()->width())->cast<JSNumber>()->float32());
			out.size.set_height(obj->get(worker, worker->strs()->height())->cast<JSNumber>()->float32());
		});
	}

	bool TypesParser::parse(JSValue* in, Mat& out, cChar* desc) {
		js_parse(Mat, {
			auto mat = obj->get(worker, worker->strs()->value())->cast<JSArray>();
			out[0] = mat->get(worker, 0u)->cast<JSNumber>()->float32();
			out[1] = mat->get(worker, 1)->cast<JSNumber>()->float32();
			out[2] = mat->get(worker, 2)->cast<JSNumber>()->float32();
			out[3] = mat->get(worker, 3)->cast<JSNumber>()->float32();
			out[4] = mat->get(worker, 4)->cast<JSNumber>()->float32();
			out[5] = mat->get(worker, 5)->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Mat4& out, cChar* desc) {
		js_parse(Mat4, {
			auto mat = obj->get(worker, worker->strs()->value())->cast<JSArray>();
			out[0] = mat->get(worker, 0u)->cast<JSNumber>()->float32();
			out[1] = mat->get(worker, 1)->cast<JSNumber>()->float32();
			out[2] = mat->get(worker, 2)->cast<JSNumber>()->float32();
			out[3] = mat->get(worker, 3)->cast<JSNumber>()->float32();
			out[4] = mat->get(worker, 4)->cast<JSNumber>()->float32();
			out[5] = mat->get(worker, 5)->cast<JSNumber>()->float32();
			out[6] = mat->get(worker, 6)->cast<JSNumber>()->float32();
			out[7] = mat->get(worker, 7)->cast<JSNumber>()->float32();
			out[8] = mat->get(worker, 8)->cast<JSNumber>()->float32();
			out[9] = mat->get(worker, 9)->cast<JSNumber>()->float32();
			out[10] = mat->get(worker, 10)->cast<JSNumber>()->float32();
			out[11] = mat->get(worker, 11)->cast<JSNumber>()->float32();
			out[12] = mat->get(worker, 12)->cast<JSNumber>()->float32();
			out[13] = mat->get(worker, 13)->cast<JSNumber>()->float32();
			out[14] = mat->get(worker, 14)->cast<JSNumber>()->float32();
			out[15] = mat->get(worker, 15)->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, ArrayFloat& out, cChar* desc) {
		if (!in->isArray()) {
			out.extend(1);
			auto num = in->toNumber(worker);
			if (!num)
				return throw_error(worker, in, desc), false;
			out[0] = num->float32();
		} else {
			auto arr = in->cast<JSArray>();
			out.extend(arr->length());
			for (uint32_t i = 0; i < out.length(); i++) {
				auto num = arr->get(worker, i)->toNumber(worker);
				if (!num)
					return throw_error(worker, in, desc), false;
				out[i] = num->float32();
			}
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, ArrayString& out, cChar* desc) {
		if (!in->isArray()) {
			out.extend(1);
			out[0] = in->toString(worker)->value(worker);
		} else {
			if (!in->cast<JSArray>()->toStringArray(worker).to(out)) {
				return throw_error(worker, in, desc), false;
			}
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, ArrayColor& out, cChar* desc) {
		if (!in->isArray()) {
			out.extend(1);
			return parse(in, out[0], desc);
		}
		auto arr = in->cast<JSArray>();
		out.extend(arr->length());
		for (uint32_t i = 0; i < out.length(); i++) {
			if (!parse(arr->get(worker, i), out[i], desc))
				return false;
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, ArrayOrigin& out, cChar* desc) {
		if (!in->isArray()) {
			out.extend(1);
			return parse(in, out[0], desc);
		}
		auto arr = in->cast<JSArray>();
		out.extend(arr->length());
		for (uint32_t i = 0; i < out.length(); i++) {
			if (!parse(arr->get(worker, i), out[i], desc))
				return false;
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, ArrayBorder& out, cChar* desc) {
		if (!in->isArray()) {
			out.extend(1);
			return parse(in, out[0], desc);
		}
		auto arr = in->cast<JSArray>();
		out.extend(arr->length());
		for (uint32_t i = 0; i < out.length(); i++) {
			if (!parse(arr->get(worker, i), out[i], desc))
				return false;
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, String& out, cChar* desc) {
		out = in->toString(worker)->value(worker);
		return true;
	}

	bool TypesParser::parse(JSValue* in, Curve& out, cChar* desc) {
		static const Dict<String, cCurve*> CURCEs({
			{"linear", &LINEAR },
			{"ease", &EASE },
			{"easeIn", &EASE_IN },
			{"easeOut", &EASE_OUT },
			{"easeInOut", &EASE_IN_OUT },
		});
		const Curve *out_;
		if ( in->isString() && CURCEs.get(in->toString(worker)->value(worker), out_)) {
			out = *out_;
			return true;
		}
		js_parse(Curve, {
			out = Curve(
				Vec2{
					obj->get(worker, worker->strs()->p1x())->cast<JSNumber>()->float32(),
					obj->get(worker, worker->strs()->p1y())->cast<JSNumber>()->float32()
				},
				Vec2{
					obj->get(worker, worker->strs()->p2x())->cast<JSNumber>()->float32(),
					obj->get(worker, worker->strs()->p2y())->cast<JSNumber>()->float32()
				}
			);
		});
	}

	bool TypesParser::parse(JSValue* in, Shadow& out, cChar* desc) {
		js_parse(Shadow, {
			out.x = obj->get(worker, worker->strs()->x())->cast<JSNumber>()->float32();
			out.y = obj->get(worker, worker->strs()->y())->cast<JSNumber>()->float32();
			out.size = obj->get(worker, worker->strs()->size())->cast<JSNumber>()->float32();
			out.color.set_r(obj->get(worker, worker->strs()->r())->toUint32(worker)->value());
			out.color.set_g(obj->get(worker, worker->strs()->g())->toUint32(worker)->value());
			out.color.set_b(obj->get(worker, worker->strs()->b())->toUint32(worker)->value());
			out.color.set_a(obj->get(worker, worker->strs()->a())->toUint32(worker)->value());
		});
	}

	bool TypesParser::parse(JSValue* in, BoxBorder& out, cChar* desc) {
		js_parse(BoxBorder, {
			out.width = obj->get(worker, worker->strs()->width())->cast<JSNumber>()->float32();
			out.color.set_r(obj->get(worker, worker->strs()->r())->toUint32(worker)->value());
			out.color.set_g(obj->get(worker, worker->strs()->g())->toUint32(worker)->value());
			out.color.set_b(obj->get(worker, worker->strs()->b())->toUint32(worker)->value());
			out.color.set_a(obj->get(worker, worker->strs()->a())->toUint32(worker)->value());
		});
	}

	bool TypesParser::parse(JSValue* in, FillPosition& out, cChar* desc) {
		js_parse(FillPosition, {
			out.kind = kind<FillPositionKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, FillSize& out, cChar* desc) {
		js_parse(FillSize, {
			out.kind = kind<FillSizeKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Repeat& out, cChar* desc) {
		js_parse(Repeat, {
			out = (Repeat)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxFilterPtr& out, cChar* desc) {
		js_parse(BoxFilterPtr, {
			out = MixObject::mix<BoxFilter>(obj)->self();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxShadowPtr& out, cChar* desc) {
		js_parse(BoxShadowPtr, {
			out = MixObject::mix<BoxShadow>(obj)->self();
		});
	}

	bool TypesParser::parse(JSValue* in, Direction& out, cChar* desc) {
		js_parse(Direction, {
			out = (Direction)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, ItemsAlign& out, cChar* desc) {
		js_parse(ItemsAlign, {
			out = (ItemsAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, CrossAlign& out, cChar* desc) {
		js_parse(CrossAlign, {
			out = (CrossAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, Wrap& out, cChar* desc) {
		js_parse(Wrap, {
			out = (Wrap)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, WrapAlign& out, cChar* desc) {
		js_parse(WrapAlign, {
			out = (WrapAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, Align& out, cChar* desc) {
		js_parse(Align, {
			out = (Align)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxSize& out, cChar* desc) {
		js_parse(BoxSize, {
			out.kind = kind<BoxSizeKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxOrigin& out, cChar* desc) {
		js_parse(BoxOrigin, {
			out.kind = kind<BoxOriginKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, TextAlign& out, cChar* desc) {
		js_parse(TextAlign, {
			out = (TextAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextDecoration& out, cChar* desc) {
		js_parse(TextDecoration, {
			out = (TextDecoration)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextOverflow& out, cChar* desc) {
		js_parse(TextOverflow, {
			out = (TextOverflow)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextWhiteSpace& out, cChar* desc) {
		js_parse(TextWhiteSpace, {
			out = (TextWhiteSpace)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextWordBreak& out, cChar* desc) {
		js_parse(TextWordBreak, {
			out = (TextWordBreak)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextColor& out, cChar* desc) {
		js_parse(TextColor, {
			out.kind = kind<TextValueKind>(obj);
			out.value.set_r(obj->get(worker, worker->strs()->r())->toUint32(worker)->value());
			out.value.set_g(obj->get(worker, worker->strs()->g())->toUint32(worker)->value());
			out.value.set_b(obj->get(worker, worker->strs()->b())->toUint32(worker)->value());
			out.value.set_a(obj->get(worker, worker->strs()->a())->toUint32(worker)->value());
		});
	}

	bool TypesParser::parse(JSValue* in, TextSize& out, cChar* desc) {
		if (in->isNumber()) {
			out.kind = TextValueKind::Value;
			out.value = in->cast<JSNumber>()->float32();
			return true;
		}
		js_parse(TextSize, {
			out.kind = kind<TextValueKind>(obj);
			out.value = obj->get(worker, worker->strs()->value())->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, TextShadow& out, cChar* desc) {
		js_parse(TextShadow, {
			out.kind = kind<TextValueKind>(obj);
			out.value.x = obj->get(worker, worker->strs()->x())->cast<JSNumber>()->float32();
			out.value.y = obj->get(worker, worker->strs()->y())->cast<JSNumber>()->float32();
			out.value.size = obj->get(worker, worker->strs()->size())->cast<JSNumber>()->float32();
			out.value.color.set_r(obj->get(worker, worker->strs()->r())->toUint32(worker)->value());
			out.value.color.set_g(obj->get(worker, worker->strs()->g())->toUint32(worker)->value());
			out.value.color.set_b(obj->get(worker, worker->strs()->b())->toUint32(worker)->value());
			out.value.color.set_a(obj->get(worker, worker->strs()->a())->toUint32(worker)->value());
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
			out = (TextWeight)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextWidth& out, cChar* desc) {
		js_parse(TextWidth, {
			out = (TextWidth)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextSlant& out, cChar* desc) {
		js_parse(TextSlant, {
			out = (TextSlant)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, FontStyle& out, cChar* desc) {
		if (!in->isUint32()) {
			return throw_error(worker, in, desc), false;
		}
		struct Out {
			uint32_t value;
		} out_ = { in->toUint32(worker)->value() };
		out = *reinterpret_cast<FontStyle*>(&out_);
		return true;
	}

	bool TypesParser::parse(JSValue* in, KeyboardType& out, cChar* desc) {
		js_parse(KeyboardType, {
			out = (KeyboardType)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, KeyboardReturnType& out, cChar* desc) {
		js_parse(KeyboardReturnType, {
			out = (KeyboardReturnType)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, CursorStyle& out, cChar* desc) {
		js_parse(CursorStyle, {
			out = (CursorStyle)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, FindDirection& out, cChar* desc) {
		js_parse(FindDirection, {
			out = (FindDirection)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, FFID& out, cChar* desc) {
		WeakBuffer buff;
		if (!in->asBuffer(worker).to(buff) || buff.length() < sizeof(FFID)) {
			return throw_error(worker, in, desc), false;
		}
		out = *reinterpret_cast<const FFID*>( buff.val() );
		return true;
	}

	void binding_filter(JSObject* exports, Worker* worker);

	class NativeTypes: public Worker {
	public:
		void setTypesParser(TypesParser *types) {
			_types = types;
		}
		static void binding(JSObject* exports, Worker* worker) {
			{
				TryCatch try_catch(worker);
				if (!worker->runNativeScript((Char*)
							native_js::INL_native_js_code__types_,
							native_js::INL_native_js_code__types_count_, "_types.js", exports)) {
					if ( try_catch.hasCaught() ) {
						try_catch.print();
					}
					Qk_Fatal("Could not initialize native _types.js");
				}
			}
			static_cast<NativeTypes*>(worker)->setTypesParser(new TypesParser(worker, exports));

			binding_filter(exports, worker);
		}
	};

	Js_Module(_types, NativeTypes);
} }
