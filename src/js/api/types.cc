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

		#undef OneByte
		#undef _Fun

		strs = worker->strs();
	}

	bool TypesParser::isTypesBase(JSObject* arg) {
		return _TypesBase->instanceOf(worker, arg);
	}

	static void throw_error(Worker* worker, JSValue* value, cChar* msg, cChar* help = nullptr) {
		// Bad argument. Input.type = `Bad`
		// reference value, "
		String msg0 = String::format(msg ? msg : "`%s`", *value->toString(worker)->value(worker) );
		JSValue* err;

		if (help) {
			err = worker->newTypeError("Bad argument %s. Examples: %s", *msg0, help);
		} else {
			err = worker->newTypeError("Bad argument %s.", *msg0);
		}
		worker->throwError(err);
	}

	// void TypesParser::throwError(JSValue* value, cChar* msg, cChar* help) {
	// 	throw_error(worker, value, msg, help);
	// }

	// --------------------------------------------------------------------------------------------

	JSValue* TypesParser::jsvalue(const Dirent& dir) {
		auto rev = worker->newObject();
		rev->set(worker, strs->name(), jsvalue(dir.name));
		rev->set(worker, strs->pathname(), jsvalue(dir.pathname));
		rev->set(worker, strs->type(), jsvalue(dir.type));
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
		rv->set(worker,strs->id(), worker->newValue(val.id));
		rv->set(worker,strs->startPosition(), jsvalue(val.start_position));
		rv->set(worker,strs->position(), jsvalue(val.position));
		rv->set(worker,strs->force(), worker->newValue(val.force));
		// rv->set(worker,strs->clickIn(), worker->newBool(val.click_in));
		rv->set(worker,strs->view(), MixObject::mix(val.view)->handle());
		return rv;
	}

	JSValue* TypesParser::jsvalue(cArray<TouchPoint>& val) {
		auto arr = worker->newArray();
		int j = 0;
		for ( auto& i : val ) {
			arr->set(worker, j++, jsvalue(i));
		}
		return arr;
	}

	// --------------------------------------------------------------------------------------------

	template<class T>
	JSValue* TypesParser::jsValue(T val) {
		return worker->newValue(val);
	}

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
			jsValue(value.r()),
			jsValue(value.g()),
			jsValue(value.b()),
			jsValue(value.a()),
		};
		return _newColor->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Vec2& value) {
		JSValue* args[] = {
			jsValue(value.x()),
			jsValue(value.y()),
		};
		return _newVec2->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const Vec3& value) {
		JSValue* args[] = {
			jsValue(value.x()),
			jsValue(value.y()),
			jsValue(value.z()),
		};
		return _newVec3->call(worker, 3, args);
	}

	JSValue* TypesParser::jsvalue(const Vec4& value) {
		JSValue* args[] = {
			jsValue(value.x()),
			jsValue(value.y()),
			jsValue(value.z()),
			jsValue(value.w()),
		};
		return _newVec4->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Rect& value) {
		JSValue* args[] = {
			jsValue(value.begin.x()),
			jsValue(value.begin.y()),
			jsValue(value.size.width()),
			jsValue(value.size.height()),
		};
		return _newRect->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Range& value) {
		JSValue* args[] = {
			jsValue(value.begin.x()),
			jsValue(value.begin.y()),
			jsValue(value.end.x()),
			jsValue(value.end.y()),
		};
		return _newRange->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Region& value) {
		JSValue* args[] = {
			jsValue(value.begin.x()),
			jsValue(value.begin.y()),
			jsValue(value.end.x()),
			jsValue(value.end.y()),
			jsValue(value.origin.x()),
			jsValue(value.origin.y()),
		};
		return _newRegion->call(worker, 6, args);
	}

	JSValue* TypesParser::jsvalue(const Mat& value) {
		JSValue* args[] = {
			jsValue(value[0]),
			jsValue(value[1]),
			jsValue(value[2]),
			jsValue(value[3]),
			jsValue(value[4]),
			jsValue(value[5]),
		};
		return _newMat->call(worker, 6, args);
	}

	JSValue* TypesParser::jsvalue(const Mat4& value) {
		JSValue* args[] = {
			jsValue(value[0]),
			jsValue(value[1]),
			jsValue(value[2]),
			jsValue(value[3]),
			jsValue(value[4]),
			jsValue(value[5]),
			jsValue(value[6]),
			jsValue(value[7]),
			jsValue(value[8]),
			jsValue(value[9]),
			jsValue(value[10]),
			jsValue(value[11]),
			jsValue(value[12]),
			jsValue(value[13]),
			jsValue(value[14]),
			jsValue(value[15]),
		};
		return _newMat4->call(worker, 16, args);
	}

	JSValue* TypesParser::jsvalue(const BorderRadius& value) {
		JSValue* args[] = {
			jsValue(value.leftTop[0]),
			jsValue(value.leftTop[1]),
			jsValue(value.rightTop[0]),
			jsValue(value.rightTop[1]),
			jsValue(value.rightBottom[0]),
			jsValue(value.rightBottom[1]),
			jsValue(value.leftBottom[0]),
			jsValue(value.leftBottom[1]),
		};
		return _newBorderRadius->call(worker, 8, args);
	}

	template<class T> JSValue* TypesParser::jsvalueArray(const T& val) {
		auto arr = worker->newArray();
		for (uint32_t i = 0; i < val.length(); i ++) {
			arr->set(worker, i, jsvalue(val[i]));
		}
		return arr;
	}

	JSValue* TypesParser::jsvalue(const ArrayFloat& value) {
		return jsvalueArray(value);
	}

	JSValue* TypesParser::jsvalue(const ArrayString& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(const ArrayColor& value) {
		return jsvalueArray(value);
	}

	JSValue* TypesParser::jsvalue(const ArrayOrigin& value) {
		return jsvalueArray(value);
	}

	JSValue* TypesParser::jsvalue(const ArrayBorder& value) {
		return jsvalueArray(value);
	}

	JSValue* TypesParser::jsvalue(const ArrayVec2& value) {
		return jsvalueArray(value);
	}

	JSValue* TypesParser::jsvalue(const ArrayVec3& value) {
		return jsvalueArray(value);
	}

	JSValue* TypesParser::jsvalue(const String& value) {
		return worker->newValue(value);
	}

	JSValue* TypesParser::jsvalue(cCurve& value) {
		JSValue* args[] = {
			jsValue(value.p1().x()),
			jsValue(value.p1().y()),
			jsValue(value.p2().x()),
			jsValue(value.p2().y()),
		};
		return _newCurve->call(worker, 4, args);
	}

	JSValue* TypesParser::jsvalue(const Shadow& value) {
		JSValue* args[] = {
			jsValue(value.x),
			jsValue(value.y),
			jsValue(value.size),
			jsValue(value.color.r()),
			jsValue(value.color.g()),
			jsValue(value.color.b()),
			jsValue(value.color.a()),
		};
		return _newShadow->call(worker, 7, args);
	}

	JSValue* TypesParser::jsvalue(const Border& value) {
		JSValue* args[] = {
			jsValue(value.width),
			jsValue(value.color.r()),
			jsValue(value.color.g()),
			jsValue(value.color.b()),
			jsValue(value.color.a()),
		};
		return _newBorder->call(worker, 5, args);
	}

	JSValue* TypesParser::jsvalue(const FillPosition& val) {
		JSValue* args[] = {
			jsValue((uint32_t)val.kind),
			jsValue(val.value),
		};
		return _newFillPosition->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const FillSize& val) {
		JSValue* args[] = {
			jsValue((uint32_t)val.kind),
			jsValue(val.value),
		};
		return _newFillSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const Repeat& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const BoxFilterPtr& value) {
		return jsValue(value);
	}

	JSValue* TypesParser::jsvalue(const BoxShadowPtr& value) {
		return jsValue(value);
	}

	JSValue* TypesParser::jsvalue(const SkeletonDataPtr& value) {
		return jsValue(value);
	}

	JSValue* TypesParser::jsvalue(const Direction& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const ItemsAlign& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const CrossAlign& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const Wrap& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const WrapAlign& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const Align& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const BoxSize& value) {
		JSValue* args[] = {
			jsValue((uint32_t)value.kind),
			jsValue(value.value),
		};
		return _newBoxSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const BoxOrigin& value) {
		JSValue* args[] = {
			jsValue((uint32_t)value.kind),
			jsValue(value.value),
		};
		return _newBoxOrigin->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextAlign& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextDecoration& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextOverflow& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWhiteSpace& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWordBreak& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextColor& value) {
		JSValue* args[] = {
			jsValue((uint32_t)value.kind),
			jsValue(value.value.r()),
			jsValue(value.value.g()),
			jsValue(value.value.b()),
			jsValue(value.value.a()),
		};
		return _newTextColor->call(worker, 5, args);
	}

	JSValue* TypesParser::jsvalue(const TextSize& value) {
		JSValue* args[] = {
			jsValue((uint32_t)value.kind),
			jsValue(value.value),
		};
		return _newTextSize->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextShadow& value) {
		JSValue* args[] = {
			jsValue((uint32_t)value.kind),
			jsValue(value.value.x),
			jsValue(value.value.y),
			jsValue(value.value.size),
			jsValue(value.value.color.r()),
			jsValue(value.value.color.g()),
			jsValue(value.value.color.b()),
			jsValue(value.value.color.a()),
		};
		return _newTextShadow->call(worker, 8, args);
	}

	JSValue* TypesParser::jsvalue(const TextStroke& value) {
		JSValue* args[] = {
			jsValue((uint32_t)value.kind),
			jsValue(value.value.width),
			jsValue(value.value.color.r()),
			jsValue(value.value.color.g()),
			jsValue(value.value.color.b()),
			jsValue(value.value.color.a()),
		};
		return _newTextStroke->call(worker, 6, args);
	}

	JSValue* TypesParser::jsvalue(const TextFamily& value) {
		JSValue* args[] = {
			jsValue((uint32_t)value.kind),
			jsvalue(value.value),
		};
		return _newTextFamily->call(worker, 2, args);
	}

	JSValue* TypesParser::jsvalue(const TextWeight& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextWidth& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const TextSlant& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FontStyle& value) {
		return jsValue(value.value());
	}

	JSValue* TypesParser::jsvalue(const KeyboardType& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const KeyboardReturnType& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const CursorStyle& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const CascadeColor& value) {
		return jsValue((uint32_t)value);
	}

	JSValue* TypesParser::jsvalue(const FFID& val) {
		cChar* addr = reinterpret_cast<cChar*>(&val);
		Buffer buffer(sizeof(FFID));
		buffer.write(addr, sizeof(FFID), 0);
		return worker->newValue(buffer);
	}

	JSValue* TypesParser::jsvalue(const PathPtr& val) {
		return MixObject::mix(val)->handle();
	}

	JSValue* TypesParser::jsvalue(const Bounds& val) {
		JSValue* args[] = {
			jsValue((uint32_t)val.type),
			jsValue(val.radius),
			jsValue( MixObject::mix(val.pts.load())->handle() ),
		};
		return _newBounds->call(worker, 3, args);
	}

	// --------------------------------------------------------------------------------------------

	#define js_parse(Type, block) { \
		JSObject* obj;\
		JSValue* val;\
		if (msg) {\
			JSValue* args[] = { in, worker->newValue(String(msg))->cast() };\
			val = _parse##Type->call(worker, 2, args);\
		} else {\
			val = _parse##Type->call(worker, 1, &in);\
		}\
		if ( !val ) return /*throw_error(worker, in, msg),*/ false;\
		obj = val->cast<JSObject>();\
		block \
		return true;\
	}

	template<typename T>
	T TypesParser::kind(JSObject* obj) {
		auto kind = obj->get(worker, strs->kind());
		return (T)kind->toUint32(worker)->value();
	}

	bool TypesParser::parse(JSValue* in, WindowOptions& _out, cChar* msg) {
		if (!in->isObject()) {
			return throw_error(worker, in, msg), false;
		}
		_out = {.backgroundColor=Color(255,255,255)};
		auto obj = in->cast<JSObject>();
		auto colorType = obj->get(worker, worker->newStringOneByte("colorType"));
		auto msaa = obj->get(worker, worker->newStringOneByte("msaa"));
		auto fps = obj->get(worker, worker->newStringOneByte("fps"));
		auto frame = obj->get(worker, worker->newStringOneByte("frame"));
		auto title = obj->get(worker, worker->newStringOneByte("title"));
		auto backgroundColor = obj->get(worker, worker->newStringOneByte("backgroundColor"));
		auto navigationColor = obj->get(worker, worker->newStringOneByte("navigationColor"));

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
		if (!navigationColor->isUndefined()) {
			Js_Parse_Type(Color, backgroundColor, "Window::Options{.navigationColor=%s}") false;
			_out.navigationColor = out;
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, FillImageInit& out_, cChar* msg) {
		if (!in->isObject()) {
			return throw_error(worker, in, msg), false;
		}
		auto obj = in->cast<JSObject>();
		auto width = obj->get(worker, worker->newStringOneByte("width"));
		auto height = obj->get(worker, worker->newStringOneByte("height"));
		auto x = obj->get(worker, worker->newStringOneByte("x"));
		auto y = obj->get(worker, worker->newStringOneByte("y"));
		auto repeat = obj->get(worker, worker->newStringOneByte("repeat"));

		if (!width->isUndefined()) {
			Js_Parse_Type(FillSize, width, msg) false;
			out_.width = out;
		}
		if (!height->isUndefined()) {
			Js_Parse_Type(FillSize, height, msg) false;
			out_.height = out;
		}
		if (!x->isUndefined()) {
			Js_Parse_Type(FillPosition, x, msg) false;
			out_.x = out;
		}
		if (!y->isUndefined()) {
			Js_Parse_Type(FillPosition, y, msg) false;
			out_.y = out;
		}
		if (!repeat->isUndefined()) {
			Js_Parse_Type(Repeat, repeat, msg) false;
			out_.repeat = out;
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, bool& out, cChar* msg) {
		out = in->toBoolean(worker);
		return true;
	}

	bool TypesParser::parse(JSValue* in, float& out, cChar* msg) {
		auto num = in->toNumber(worker);
		if (!num)
			return throw_error(worker, in, msg), false;
		out = num->float32();
		return true;
	}

	bool TypesParser::parse(JSValue* in, int32_t& out, cChar* msg) {
		auto num = in->toInt32(worker);
		if (!num)
			return throw_error(worker, in, msg), false;
		out = num->value();
		return true;
	}

	bool TypesParser::parse(JSValue* in, uint32_t& out, cChar* msg) {
		auto num = in->toUint32(worker);
		if (!num)
			return throw_error(worker, in, msg), false;
		out = num->value();
		return true;
	}

	bool TypesParser::parse(JSValue* in, Color& out, cChar* msg) {
		js_parse(Color, {
			// TODO: need to check type
			out[0] = obj->get<JSInt32>(worker, strs->r())->value();
			out[1] = obj->get<JSInt32>(worker, strs->g())->value();
			out[2] = obj->get<JSInt32>(worker, strs->b())->value();
			out[3] = obj->get<JSInt32>(worker, strs->a())->value();
		});
	}

	bool TypesParser::parse(JSValue* in, Vec2& out, cChar* msg) {
		js_parse(Vec2, {
			// TODO: need to check type
			out[0] = obj->get<JSNumber>(worker, strs->x())->float32();
			out[1] = obj->get<JSNumber>(worker, strs->y())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Vec3& out, cChar* msg) {
		js_parse(Vec3, {
			// TODO: need to check type
			out[0] = obj->get<JSNumber>(worker, strs->x())->float32();
			out[1] = obj->get<JSNumber>(worker, strs->y())->float32();
			out[2] = obj->get<JSNumber>(worker, strs->z())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Vec4& out, cChar* msg) {
		js_parse(Vec4, {
			// TODO: need to check type
			out[0] = obj->get<JSNumber>(worker, strs->x())->float32();
			out[1] = obj->get<JSNumber>(worker, strs->y())->float32();
			out[2] = obj->get<JSNumber>(worker, strs->z())->float32();
			out[3] = obj->get<JSNumber>(worker, strs->w())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Rect& out, cChar* msg) {
		js_parse(Rect, {
			// TODO: need to check type
			out.begin[0] = obj->get<JSNumber>(worker, strs->x())->float32();
			out.begin[1] = obj->get<JSNumber>(worker, strs->y())->float32();
			out.size[0] = obj->get<JSNumber>(worker, strs->width())->float32();
			out.size[1] = obj->get<JSNumber>(worker, strs->height())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Range& out, cChar* msg) {
		js_parse(Range, {
			// TODO: need to check type
			out.begin[0] = obj->get<JSNumber>(worker, strs->p0())->float32();
			out.begin[1] = obj->get<JSNumber>(worker, strs->p1())->float32();
			out.end[0] = obj->get<JSNumber>(worker, strs->p2())->float32();
			out.end[1] = obj->get<JSNumber>(worker, strs->p3())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Region& out, cChar* msg) {
		js_parse(Region, {
			// TODO: need to check type
			out.begin[0] = obj->get<JSNumber>(worker, strs->p0())->float32();
			out.begin[1] = obj->get<JSNumber>(worker, strs->p1())->float32();
			out.end[0] = obj->get<JSNumber>(worker, strs->p2())->float32();
			out.end[1] = obj->get<JSNumber>(worker, strs->p3())->float32();
			out.origin[0] = obj->get<JSNumber>(worker, strs->p4())->float32();
			out.origin[1] = obj->get<JSNumber>(worker, strs->p5())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Mat& out, cChar* msg) {
		js_parse(Mat, {
			// TODO: need to check type
			auto mat = obj->get<JSArray>(worker, strs->value());
			out[0] = mat->get<JSNumber>(worker, 0u)->float32();
			out[1] = mat->get<JSNumber>(worker, 1)->float32();
			out[2] = mat->get<JSNumber>(worker, 2)->float32();
			out[3] = mat->get<JSNumber>(worker, 3)->float32();
			out[4] = mat->get<JSNumber>(worker, 4)->float32();
			out[5] = mat->get<JSNumber>(worker, 5)->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Mat4& out, cChar* msg) {
		js_parse(Mat4, {
			// TODO: need to check type
			auto mat = obj->get<JSArray>(worker, strs->value());
			out[0] = mat->get<JSNumber>(worker, 0u)->float32();
			out[1] = mat->get<JSNumber>(worker, 1)->float32();
			out[2] = mat->get<JSNumber>(worker, 2)->float32();
			out[3] = mat->get<JSNumber>(worker, 3)->float32();
			out[4] = mat->get<JSNumber>(worker, 4)->float32();
			out[5] = mat->get<JSNumber>(worker, 5)->float32();
			out[6] = mat->get<JSNumber>(worker, 6)->float32();
			out[7] = mat->get<JSNumber>(worker, 7)->float32();
			out[8] = mat->get<JSNumber>(worker, 8)->float32();
			out[9] = mat->get<JSNumber>(worker, 9)->float32();
			out[10] = mat->get<JSNumber>(worker, 10)->float32();
			out[11] = mat->get<JSNumber>(worker, 11)->float32();
			out[12] = mat->get<JSNumber>(worker, 12)->float32();
			out[13] = mat->get<JSNumber>(worker, 13)->float32();
			out[14] = mat->get<JSNumber>(worker, 14)->float32();
			out[15] = mat->get<JSNumber>(worker, 15)->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, BorderRadius& out, cChar* msg) {
		js_parse(BorderRadius, {
			// TODO: need to check type
			out.leftTop[0] = obj->get<JSNumber>(worker, strs->p0())->float32();
			out.leftTop[1] = obj->get<JSNumber>(worker, strs->p1())->float32();
			out.rightTop[0] = obj->get<JSNumber>(worker, strs->p2())->float32();
			out.rightTop[1] = obj->get<JSNumber>(worker, strs->p3())->float32();
			out.rightBottom[0] = obj->get<JSNumber>(worker, strs->p4())->float32();
			out.rightBottom[1] = obj->get<JSNumber>(worker, strs->p5())->float32();
			out.leftBottom[0] = obj->get<JSNumber>(worker, strs->p6())->float32();
			out.leftBottom[1] = obj->get<JSNumber>(worker, strs->p7())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, ArrayFloat& out, cChar* msg) {
		if (!in->isArray()) {
			out.extend(1);
			auto num = in->toNumber(worker);
			if (!num) return throw_error(worker, in, msg), false;
			float val = num->float32();
			out[0] = val;
		} else {
			auto arr = in->cast<JSArray>();
			out.extend(arr->length());
			for (uint32_t i = 0; i < out.length(); i++) {
				auto num = arr->get(worker, i)->toNumber(worker);
				if (!num) return throw_error(worker, in, msg), false;
				out[i] = num->float32();
			}
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, ArrayString& out, cChar* msg) {
		if (!in->isArray()) {
			out.extend(1);
			out[0] = in->toString(worker)->value(worker);
		} else {
			if (!in->cast<JSArray>()->toStringArray(worker).to(out)) {
				return throw_error(worker, in, msg), false;
			}
		}
		return true;
	}

	template<class T> bool TypesParser::parseArray(JSValue* in, T& out, cChar* msg) {
		if (!in->isArray()) {
			out.extend(1);
			return parse(in, out[0], msg);
		}
		auto arr = in->cast<JSArray>();
		out.extend(arr->length());
		for (uint32_t i = 0; i < out.length(); i++) {
			if (!parse(arr->get(worker, i), out[i], msg))
				return false;
		}
		return true;
	}

	bool TypesParser::parse(JSValue* in, ArrayColor& out, cChar* msg) {
		return parseArray(in, out, msg);
	}

	bool TypesParser::parse(JSValue* in, ArrayOrigin& out, cChar* msg) {
		return parseArray(in, out, msg);
	}

	bool TypesParser::parse(JSValue* in, ArrayBorder& out, cChar* msg) {
		return parseArray(in, out, msg);
	}

	bool TypesParser::parse(JSValue* in, ArrayVec2& out, cChar* msg) {
		return parseArray(in, out, msg);
	}

	bool TypesParser::parse(JSValue* in, ArrayVec3& out, cChar* msg) {
		return parseArray(in, out, msg);
	}

	bool TypesParser::parse(JSValue* in, String& out, cChar* msg) {
		out = in->toString(worker)->value(worker);
		return true;
	}

	bool TypesParser::parse(JSValue* in, Curve& out, cChar* msg) {
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
			// TODO: need to check type
			out = Curve(
				Vec2{
					obj->get(worker, strs->p1x())->cast<JSNumber>()->float32(),
					obj->get(worker, strs->p1y())->cast<JSNumber>()->float32()
				},
				Vec2{
					obj->get(worker, strs->p2x())->cast<JSNumber>()->float32(),
					obj->get(worker, strs->p2y())->cast<JSNumber>()->float32()
				}
			);
		});
	}

	bool TypesParser::parse(JSValue* in, Shadow& out, cChar* msg) {
		js_parse(Shadow, {
			// TODO: need to check type
			out.x = obj->get<JSNumber>(worker, strs->x())->float32();
			out.y = obj->get<JSNumber>(worker, strs->y())->float32();
			out.size = obj->get<JSNumber>(worker, strs->size())->float32();
			out.color[0] = obj->get<JSInt32>(worker, strs->r())->value();
			out.color[1] = obj->get<JSInt32>(worker, strs->g())->value();
			out.color[2] = obj->get<JSInt32>(worker, strs->b())->value();
			out.color[3] = obj->get<JSInt32>(worker, strs->a())->value();
		});
	}

	bool TypesParser::parse(JSValue* in, Border& out, cChar* msg) {
		js_parse(Border, {
			// TODO: need to check type
			out.width = obj->get<JSNumber>(worker, strs->width())->float32();
			out.color[0] = obj->get<JSInt32>(worker, strs->r())->value();
			out.color[1] = obj->get<JSInt32>(worker, strs->g())->value();
			out.color[2] = obj->get<JSInt32>(worker, strs->b())->value();
			out.color[3] = obj->get<JSInt32>(worker, strs->a())->value();
		});
	}

	bool TypesParser::parse(JSValue* in, FillPosition& out, cChar* msg) {
		js_parse(FillPosition, {
			// TODO: need to check type
			out.kind = kind<FillPositionKind>(obj);
			out.value = obj->get<JSNumber>(worker, strs->value())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, FillSize& out, cChar* msg) {
		js_parse(FillSize, {
			// TODO: need to check type
			out.kind = kind<FillSizeKind>(obj);
			out.value = obj->get<JSNumber>(worker, strs->value())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, Repeat& out, cChar* msg) {
		js_parse(Repeat, {
			// TODO: need to check type
			out = (Repeat)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxFilterPtr& out, cChar* msg) {
		if (in->isNull()) return out = nullptr, true;
		js_parse(BoxFilterPtr, {
			// TODO: need to check type
		 	out = MixObject::mix<BoxFilter>(obj)->self();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxShadowPtr& out, cChar* msg) {
		js_parse(BoxShadowPtr, {
			// TODO: need to check type
			out = MixObject::mix<BoxShadow>(obj)->self();
		});
	}

	bool TypesParser::parse(JSValue* in, SkeletonDataPtr& out, cChar* msg) {
		if (in->isNull()) return out = nullptr, true;
		js_parse(SkeletonDataPtr, {
			// TODO: need to check type
			out = MixObject::mix<SkeletonData>(obj)->self();
		});
	}

	bool TypesParser::parse(JSValue* in, Direction& out, cChar* msg) {
		js_parse(Direction, {
			// TODO: need to check type
			out = (Direction)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, ItemsAlign& out, cChar* msg) {
		js_parse(ItemsAlign, {
			// TODO: need to check type
			out = (ItemsAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, CrossAlign& out, cChar* msg) {
		js_parse(CrossAlign, {
			// TODO: need to check type
			out = (CrossAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, Wrap& out, cChar* msg) {
		js_parse(Wrap, {
			// TODO: need to check type
			out = (Wrap)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, WrapAlign& out, cChar* msg) {
		js_parse(WrapAlign, {
			// TODO: need to check type
			out = (WrapAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, Align& out, cChar* msg) {
		js_parse(Align, {
			// TODO: need to check type
			out = (Align)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxSize& out, cChar* msg) {
		js_parse(BoxSize, {
			// TODO: need to check type
			out.kind = kind<BoxSizeKind>(obj);
			out.value = obj->get<JSNumber>(worker, strs->value())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, BoxOrigin& out, cChar* msg) {
		js_parse(BoxOrigin, {
			// TODO: need to check type
			out.kind = kind<BoxOriginKind>(obj);
			out.value = obj->get<JSNumber>(worker, strs->value())->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, TextAlign& out, cChar* msg) {
		js_parse(TextAlign, {
			// TODO: need to check type
			out = (TextAlign)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextDecoration& out, cChar* msg) {
		js_parse(TextDecoration, {
			// TODO: need to check type
			out = (TextDecoration)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextOverflow& out, cChar* msg) {
		js_parse(TextOverflow, {
			// TODO: need to check type
			out = (TextOverflow)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextWhiteSpace& out, cChar* msg) {
		js_parse(TextWhiteSpace, {
			// TODO: need to check type
			out = (TextWhiteSpace)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextWordBreak& out, cChar* msg) {
		js_parse(TextWordBreak, {
			// TODO: need to check type
			out = (TextWordBreak)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextColor& out, cChar* msg) {
		js_parse(TextColor, {
			// TODO: need to check type
			out.kind = kind<TextValueKind>(obj);
			out.value[0] = obj->get<JSInt32>(worker, strs->r())->value();
			out.value[1] = obj->get<JSInt32>(worker, strs->g())->value();
			out.value[2] = obj->get<JSInt32>(worker, strs->b())->value();
			out.value[3] = obj->get<JSInt32>(worker, strs->a())->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextSize& out, cChar* msg) {
		if (in->isNumber()) {
			out.kind = TextValueKind::Value;
			out.value = in->cast<JSNumber>()->float32();
			return true;
		}
		js_parse(TextSize, {
			// TODO: need to check type
			out.kind = kind<TextValueKind>(obj);
			out.value = obj->get(worker, strs->value())->cast<JSNumber>()->float32();
		});
	}

	bool TypesParser::parse(JSValue* in, TextShadow& out, cChar* msg) {
		js_parse(TextShadow, {
			// TODO: need to check type
			out.kind = kind<TextValueKind>(obj);
			out.value.x = obj->get<JSNumber>(worker, strs->x())->float32();
			out.value.y = obj->get<JSNumber>(worker, strs->y())->float32();
			out.value.size = obj->get<JSNumber>(worker, strs->size())->float32();
			out.value.color[0] = obj->get<JSInt32>(worker, strs->r())->value();
			out.value.color[1] = obj->get<JSInt32>(worker, strs->g())->value();
			out.value.color[2] = obj->get<JSInt32>(worker, strs->b())->value();
			out.value.color[3] = obj->get<JSInt32>(worker, strs->a())->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextStroke& out, cChar* msg) {
		js_parse(TextStroke, {
			// TODO: need to check type
			out.kind = kind<TextValueKind>(obj);
			out.value.width = obj->get<JSNumber>(worker, strs->width())->float32();
			out.value.color[0] = obj->get<JSInt32>(worker, strs->r())->value();
			out.value.color[1] = obj->get<JSInt32>(worker, strs->g())->value();
			out.value.color[2] = obj->get<JSInt32>(worker, strs->b())->value();
			out.value.color[3] = obj->get<JSInt32>(worker, strs->a())->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextFamily& out, cChar* msg) {
		js_parse(TextFamily, {
			// TODO: need to check type
			out.kind = kind<TextValueKind>(obj);
			return parse(obj->get(worker, strs->value()), out.value, msg);
		});
	}

	bool TypesParser::parse(JSValue* in, TextWeight& out, cChar* msg) {
		js_parse(TextWeight, {
			// TODO: need to check type
			out = (TextWeight)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextWidth& out, cChar* msg) {
		js_parse(TextWidth, {
			// TODO: need to check type
			out = (TextWidth)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, TextSlant& out, cChar* msg) {
		js_parse(TextSlant, {
			// TODO: need to check type
			out = (TextSlant)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, FontStyle& out, cChar* msg) {
		int value;
		if (!in->asInt32(worker).to(value)) {
			return throw_error(worker, in, msg), false;
		}
		struct Out { int value; } out_ = { value };
		out = *reinterpret_cast<FontStyle*>(&out_);
		return true;
	}

	bool TypesParser::parse(JSValue* in, KeyboardType& out, cChar* msg) {
		js_parse(KeyboardType, {
			// TODO: need to check type
			out = (KeyboardType)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, KeyboardReturnType& out, cChar* msg) {
		js_parse(KeyboardReturnType, {
			// TODO: need to check type
			out = (KeyboardReturnType)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, CursorStyle& out, cChar* msg) {
		js_parse(CursorStyle, {
			// TODO: need to check type
			out = (CursorStyle)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, CascadeColor& out, cChar* msg) {
		js_parse(CascadeColor, {
			// TODO: need to check type
			out = (CascadeColor)obj->toUint32(worker)->value();
		});
	}

	bool TypesParser::parse(JSValue* in, FFID& out, cChar* msg) {
		WeakBuffer buff;
		if (!in->asBuffer(worker).to(buff) || buff.length() < sizeof(FFID)) {
			return throw_error(worker, in, msg), false;
		}
		out = *reinterpret_cast<const FFID*>( buff.val() );
		return true;
	}

	bool TypesParser::parse(JSValue* in, PathPtr& out, cChar* msg) {
		out = MixObject::mix<Path>(in)->self();
		return true;
	}

	bool TypesParser::parse(JSValue* in, Bounds& out, cChar* msg) {
		js_parse(Bounds, {
			// TODO: need to check type
			out.type = (Entity::BoundsType)obj->get<JSInt32>(worker, strs->p0())->value();
			out.radius = obj->get<JSNumber>(worker, strs->p1())->float32();
			auto p2 = obj->get(worker, strs->p2());
			out.pts = p2->isObject() ? MixObject::mix<Path>(p2->cast<JSObject>())->self(): nullptr;
		});
		return true;
	}

	void binding_filter(JSObject* exports, Worker* worker);
	void binding_skeletonData(JSObject* exports, Worker* worker);

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
			binding_skeletonData(exports, worker);
		}
	};

	Js_Module(_types, NativeTypes);
} }
