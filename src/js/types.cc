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
		#define _Fun(Name, Type) \
		_parse##Name.reset(worker, exports->get(worker,OneByte("parse"#Name))->cast<JSFunction>()); \
		_new##Name.reset(worker, exports->get(worker,OneByte("_new"#Name))->cast<JSFunction>());

		Js_Types_Each(_Fun)
		Qk_DEBUG("Init types %s ok", "TypesParser");

		#undef OneByte
		#undef _Fun
	}

	bool TypesParser::isBase(JSValue* arg) {
		// TODO ...
		return false;
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

	JSValue* TypesParser::newInstance(const bool& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::newInstance(const float& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::newInstance(const int& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::newInstance(const uint32_t& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::newInstance(const Color& value) {
		JSValue* args[] = {
			worker->newInstance(value.r()),
			worker->newInstance(value.g()),
			worker->newInstance(value.b()),
			worker->newInstance(value.a()),
		};
		return _newColor->call(worker, 4, args);
	}

	JSValue* TypesParser::newInstance(const Vec2& value) {
		JSValue* args[] = {
			worker->newInstance(value.x()),
			worker->newInstance(value.y()),
		};
		return _newVec2->call(worker, 2, args);
	}

	JSValue* TypesParser::newInstance(const Vec3& value) {
		JSValue* args[] = {
			worker->newInstance(value.x()),
			worker->newInstance(value.y()),
			worker->newInstance(value.z()),
		};
		return _newVec3->call(worker, 3, args);
	}

	JSValue* TypesParser::newInstance(const Vec4& value) {
		JSValue* args[] = {
			worker->newInstance(value.x()),
			worker->newInstance(value.y()),
			worker->newInstance(value.z()),
			worker->newInstance(value.w()),
		};
		return _newVec4->call(worker, 4, args);
	}

	JSValue* TypesParser::newInstance(const Rect& value) {
		JSValue* args[] = {
			worker->newInstance(value.origin.x()),
			worker->newInstance(value.origin.y()),
			worker->newInstance(value.size.width()),
			worker->newInstance(value.size.height()),
		};
		return _newRect->call(worker, 4, args);
	}

	JSValue* TypesParser::newInstance(const Mat& value) {
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

	JSValue* TypesParser::newInstance(const Mat4& value) {
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

	JSValue* TypesParser::newInstance(cCurve& value) {
		JSValue* args[] = {
			worker->newInstance(value.p1().x()),
			worker->newInstance(value.p1().y()),
			worker->newInstance(value.p2().x()),
			worker->newInstance(value.p2().y()),
		};
		return _newCurve->call(worker, 4, args);
	}

	JSValue* TypesParser::newInstance(const Shadow& value) {
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

	JSValue* TypesParser::newInstance(const Repeat& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const BoxFilter*& value) {
		//JSValue* arg = worker->newInstance((uint32_t)value);
		//return _newRepeat->call(worker, 1, &arg);
	}


	JSValue* TypesParser::newInstance(const FillGradient*& value) {
		// JSValue* arg = worker->newInstance((uint32_t)value);
		// return _newRepeat->call(worker, 1, &arg);
	}

	JSValue* TypesParser::newInstance(const BoxShadow*& value) {
		// JSValue* arg = worker->newInstance((uint32_t)value);
		// return _newRepeat->call(worker, 1, &arg);
	}

	JSValue* TypesParser::newInstance(const Direction& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const ItemsAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const CrossAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const qk::Wrap& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const WrapAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const Align& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const BoxSizeKind& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const BoxOriginKind& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const BoxSize& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newBoxSize->call(worker, 2, args);
	}

	JSValue* TypesParser::newInstance(const BoxOrigin& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newBoxOrigin->call(worker, 2, args);
	}

	JSValue* TypesParser::newInstance(const TextAlign& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextDecoration& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextOverflow& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextWhiteSpace& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextWordBreak& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextValueKind& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextColor& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value.r()),
			worker->newInstance(value.value.g()),
			worker->newInstance(value.value.b()),
			worker->newInstance(value.value.a()),
		};
		return _newTextColor->call(worker, 5, args);
	}

	JSValue* TypesParser::newInstance(const TextSize& value) {
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(value.value),
		};
		return _newTextSize->call(worker, 1, args);
	}

	JSValue* TypesParser::newInstance(const TextShadow& value) {
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

	JSValue* TypesParser::newInstance(const TextFamily& value) {
		auto ffid = uint64_t(value.value);
		JSValue* args[] = {
			worker->newInstance((uint32_t)value.kind),
			worker->newInstance(uint32_t(ffid >> 32)),
			worker->newInstance(uint32_t(ffid & 0xffffffff)),
		};
		return _newTextFamily->call(worker, 3, args);
	}

	JSValue* TypesParser::newInstance(const TextWeight& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextWidth& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const TextSlant& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const FontStyle& value) {
		return worker->newInstance(value.value());
	}

	JSValue* TypesParser::newInstance(const KeyboardType& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const KeyboardReturnType& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const CursorStyle& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const FindDirection& value) {
		return worker->newInstance((uint32_t)value);
	}

	JSValue* TypesParser::newInstance(const String& value) {
		return worker->newInstance(value);
	}

	JSValue* TypesParser::newInstance(const Dirent& dir) {
		auto rev = worker->newObject();
		rev->set(worker, worker->strs()->name(), newInstance(dir.name));
		rev->set(worker, worker->strs()->pathname(), newInstance(dir.pathname));
		rev->set(worker, worker->strs()->type(), newInstance(dir.type));
		return rev;
	}

	JSValue* TypesParser::newInstance(cArray<Dirent>& ls) {
		auto rev = worker->newArray(ls.length());
		if (ls.length()) {
			HandleScope scope(worker);
			for (int i = 0, e = ls.length(); i < e; i++) {
				rev->set(worker, i, newInstance(ls[i]));
			}
		}
		return rev;
	}

	JSValue* TypesParser::newInstance(const FileStat& stat) {
		auto func = worker->classsinfo()->getFunction(Js_Typeid(FileStat));
		Qk_ASSERT( func );
		auto r = func->newInstance(worker);
		*WrapObject::wrap<FileStat>(r)->self() = stat;
		return r;
	}

	JSValue* TypesParser::newInstance(cArray<FileStat>& ls) {
		auto rev = worker->newArray(ls.length());
		if (ls.length()) {
			HandleScope scope(worker);
			for (int i = 0, e = ls.length(); i < e; i++) {
				rev->set(worker, i, newInstance(ls[i]));
			}
		}
		return rev;
	}

	// --------------------------------------------------------------------------------------------

	static const Dict<String, cCurve*> CURCE({
		{"linear", &LINEAR },
		{"ease", &EASE },
		{"ease_in", &EASE_IN },
		{"ease_out", &EASE_OUT },
		{"ease_in_out", &EASE_IN_OUT },
	});

	#define js_parse(Type, ok) { \
		JSObject* obj;\
		JSValue* val;\
		if (desc) {\
			JSValue* args[] = { in, worker->newInstance(desc)->cast() };\
			val = _parse##Type->call(worker, 2, args);\
		} else {\
			val = _parse##Type->call(worker, 1, &in);\
		}\
		if ( val ) {\
			obj = val->cast<JSObject>();\
		} else {\
			return false;\
		} \
		ok \
		return true;\
	}

	bool TypesParser::parseWindowOptions(JSValue* in, WindowOptions& _out, cChar* desc) {
		if (in->isObject()) {
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
		}
		return true;
	}

	bool TypesParser::parseTextAlign(JSValue* in, TextAlign& out, cChar* desc) {
		js_parse(TextAlign, {
			out = (TextAlign)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parseAlign(JSValue* in, Align& out, cChar* desc) {
		js_parse(Align, {
			out = (Align)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parseRepeat(JSValue* in, Repeat& out, cChar* desc) {
		js_parse(Repeat, {
			out = (Repeat)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parseDirection(JSValue* in, Direction& out, cChar* desc) {
		js_parse(Direction, {
			out = (Direction)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parseKeyboardType(JSValue* in, KeyboardType& out, cChar* desc) {
		js_parse(KeyboardType, {
			out = (KeyboardType)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parseKeyboardReturnType(JSValue* in, KeyboardReturnType& out, cChar* desc) {
		js_parse(KeyboardReturnType, {
			out = (KeyboardReturnType)obj->toUint32Value(worker);
		});
	}

	bool TypesParser::parseShadow(JSValue* in, Shadow& out, cChar* desc) {
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

	bool TypesParser::parseColor(JSValue* in, Color& out, cChar* desc) {
		js_parse(Color, {
			out.set_r(obj->get(worker, worker->strs()->r())->toUint32Value(worker));
			out.set_g(obj->get(worker, worker->strs()->g())->toUint32Value(worker));
			out.set_b(obj->get(worker, worker->strs()->b())->toUint32Value(worker));
			out.set_a(obj->get(worker, worker->strs()->a())->toUint32Value(worker));
		});
	}

	bool TypesParser::parseVec2(JSValue* in, Vec2& out, cChar* desc) {
		js_parse(Vec2, {
			out.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
		});
	}

	bool TypesParser::parseVec3(JSValue* in, Vec3& out, cChar* desc) {
		js_parse(Vec3, {
			out.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
			out.set_z(obj->get(worker, worker->strs()->z())->toNumberValue(worker));
		});
	}

	bool TypesParser::parseVec4(JSValue* in, Vec4& out, cChar* desc) {
		js_parse(Vec4, {
			out.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
			out.set_z(obj->get(worker, worker->strs()->z())->toNumberValue(worker));
			out.set_w(obj->get(worker, worker->strs()->w())->toNumberValue(worker));
		});
	}

	bool TypesParser::parseCurve(JSValue* in, Curve& out, cChar* desc) {
		if ( in->isString() ) {
			cCurve *out1;
			if (CURCE.get(in->toStringValue(worker,true), out1)) {
				out = *out1;
				return true;
			}
		}
		js_parse(Curve, {
			out = Curve(obj->get(worker, worker->strs()->point1X())->toNumberValue(worker),
									obj->get(worker, worker->strs()->point1Y())->toNumberValue(worker),
									obj->get(worker, worker->strs()->point2X())->toNumberValue(worker),
									obj->get(worker, worker->strs()->point2Y())->toNumberValue(worker));
		});
	}

	bool TypesParser::parseRect(JSValue* in, Rect& out, cChar* desc) {
		js_parse(Rect, {
			out.origin.set_x(obj->get(worker, worker->strs()->x())->toNumberValue(worker));
			out.origin.set_y(obj->get(worker, worker->strs()->y())->toNumberValue(worker));
			out.size.set_width(obj->get(worker, worker->strs()->width())->toNumberValue(worker));
			out.size.set_height(obj->get(worker, worker->strs()->height())->toNumberValue(worker));
		});
	}

	bool TypesParser::parseMat(JSValue* in, Mat& out, cChar* desc) {
		js_parse(Mat, {
			auto mat = obj->get(worker, worker->strs()->_value())->cast<JSArray>();
			out[0] = mat->get(worker, uint32_t(0))->toNumberValue(worker);
			out[1] = mat->get(worker, 1)->toNumberValue(worker);
			out[2] = mat->get(worker, 2)->toNumberValue(worker);
			out[3] = mat->get(worker, 3)->toNumberValue(worker);
			out[4] = mat->get(worker, 4)->toNumberValue(worker);
			out[5] = mat->get(worker, 5)->toNumberValue(worker);
		});
	}

	bool TypesParser::parseMat4(JSValue* in, Mat4& out, cChar* desc) {
		js_parse(Mat4, {
			auto mat = obj->get(worker, worker->strs()->_value())->cast<JSArray>();
			out[0] = mat->get(worker, uint32_t(0))->toNumberValue(worker);
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

	bool TypesParser::parseTextColor(JSValue* in, TextColor& out, cChar* desc) {
		js_parse(TextColor, {
			out.kind = (TextValueKind)obj->get(worker, worker->strs()->kind())->toUint32Value(worker);
			// out.value.r(obj->get(worker, worker->strs()->r())->toUint32Value(worker));
			// out.value.g(obj->get(worker, worker->strs()->g())->toUint32Value(worker));
			// out.value.b(obj->get(worker, worker->strs()->b())->toUint32Value(worker));
			// out.value.a(obj->get(worker, worker->strs()->a())->toUint32Value(worker));
		});
	}

	bool TypesParser::parseTextSize(JSValue* in, TextSize& out, cChar* desc) {
		// if (in->isNumber(worker)) {
		// 	out.type = TextValueType::VALUE;
		// 	out.value = in-toNumberValue(_worker);
		// 	return true;
		// }
		// js_parse(TextSize, {
		// 	out.type = (TextValueType)obj->get(worker, worker->strs()->type())->toUint32Value(worker);
		// 	out.value = obj->get(worker, worker->strs()->value())->toUint32Value(worker);
		// });
	}
	bool TypesParser::parseTextFamily(JSValue* in, TextFamily& out, cChar* desc) {
		// js_parse(TextFamily, {
		// 	out.type = (TextValueType)obj->get(worker, worker->strs()->type())->toUint32Value(worker);
		// 	String fonts = obj->get(worker, worker->strs()->value())->toStringValue(worker);
		// 	out.value = FontPool::get_font_familys_id(fonts);
		// });
	}
	bool TypesParser::parseTextSlant(JSValue* in, TextSlant& out, cChar* desc) {
		// js_parse(TextSlant, {
		// 	out.type = (TextValueType)obj->get(worker, worker->strs()->type())->toUint32Value(worker);
		// 	out.value = (TextSlantEnum)obj->get(worker, worker->strs()->value())->toUint32Value(worker);
		// });
	}
	bool TypesParser::parseTextShadow(JSValue* in, TextShadow& out, cChar* desc) {
		// js_parse(TextShadow, {
		// 	out.type = (TextValueType)obj->get(worker, worker->strs()->type())->toUint32Value(worker);
		// 	out.value.offset_x = obj->get(worker, worker->strs()->offsetX())->toNumberValue(worker);
		// 	out.value.offset_y = obj->get(worker, worker->strs()->offsetY())->toNumberValue(worker);
		// 	out.value.size = obj->get(worker, worker->strs()->size())->toNumberValue(worker);
		// 	out.value.color.r(obj->get(worker, worker->strs()->r())->toUint32Value(worker));
		// 	out.value.color.g(obj->get(worker, worker->strs()->g())->toUint32Value(worker));
		// 	out.value.color.b(obj->get(worker, worker->strs()->b())->toUint32Value(worker));
		// 	out.value.color.a(obj->get(worker, worker->strs()->a())->toUint32Value(worker));
		// });
	}
	bool TypesParser::parseTextDecoration(JSValue* in,
																				TextDecoration& out, cChar* desc) {
		// js_parse(TextDecoration, {
		// 	out.type = (TextValueType)obj->get(worker, worker->strs()->type())->toUint32Value(worker);
		// 	out.value = (TextDecorationEnum)obj->get(worker, worker->strs()->value())->toUint32Value(worker);
		// });
	}
	bool TypesParser::parseString(JSValue* in, String& out, cChar* desc) {
		out = in->toStringValue(worker);
		return true;
	}
	bool TypesParser::parsebool(JSValue* in, bool& out, cChar* desc) {
		out = in->toBooleanValue(worker);
		return true;
	}
	bool TypesParser::parsefloat(JSValue* in, float& out, cChar* desc) {
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
	bool TypesParser::parseint(JSValue* in, int& out, cChar* desc) {
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
	bool TypesParser::parseuint32_t(JSValue* in, uint32_t& out, cChar* desc) {
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
	bool TypesParser::parseTextOverflow(JSValue* in, TextOverflow& out, cChar* desc) {
		// js_parse(TextOverflow, {
		// 	out.type = (TextValueType)obj->get(worker, worker->strs()->type())->toUint32Value(worker);
		// 	out.value = (TextOverflowEnum)obj->get(worker, worker->strs()->value())->toUint32Value(worker);
		// });
	}
	bool TypesParser::parseTextWhiteSpace(JSValue* in, TextWhiteSpace& out, cChar* desc) {
		// js_parse(TextWhiteSpace, {
		// 	out.type = (TextValueType)obj->get(worker, worker->strs()->type())->toUint32Value(worker);
		// 	out.value = (TextWhiteSpaceEnum)obj->get(worker, worker->strs()->value())->toUint32Value(worker);
		// });
	}

	class NativeTypes: public Worker {
	public:
		void setTypesParser(TypesParser *types) {
			_types = types;
		}
		static void binding(JSObject* exports, Worker* worker) {
			// binding Background / BackgroundImage / BackgroundGradient
			// binding_background(exports, worker);
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
