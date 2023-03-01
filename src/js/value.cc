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

#include "./str.h"
#include "./_js.h"
#include "./api/_view.h"
#include "./types.h"
#include "../font/font.h"
#include "../draw.h"
#include <native-inl-js.h>

/**
 * @ns qk::js
 */

JS_BEGIN

ValueProgram::ValueProgram(Worker* worker,
												 Local<JSObject> exports,
												 Local<JSObject> priv): worker(worker) {
#define Ascii(s) worker->NewAscii(s)

#define js_init_func(Name, Type) \
	Qk_DEBUG("init value %s", #Name);\
	_parse##Name       .Reset(worker, priv->Get(worker,Ascii("parse"#Name)).To<JSFunction>()); \
	_##Name.Reset(worker, priv->Get(worker,Ascii("_"#Name)).To<JSFunction>());

	// reset
	js_value_table(js_init_func)

	_Base.Reset(worker, priv->Get(worker,Ascii("_Base")).To<JSFunction>());

#undef Ascii
#undef js_init_func
}

ValueProgram::~ValueProgram() {}

Local<JSValue> ValueProgram::New(const TextAlign& value) {
	Local<JSValue> arg = worker->New((uint)value);
	return _TextAlign.local()->Call(worker, 1, &arg);
}

Local<JSValue> ValueProgram::New(const Align& value) {
	Local<JSValue> arg = worker->New((uint)value);
	return _Align.local()->Call(worker, 1, &arg);
}

Local<JSValue> ValueProgram::New(const ContentAlign& value) {
	Local<JSValue> arg = worker->New((uint)value);
	return _ContentAlign.local()->Call(worker, 1, &arg);
}

Local<JSValue> ValueProgram::New(const Repeat& value) {
	Local<JSValue> arg = worker->New((uint)value);
	return _Repeat.local()->Call(worker, 1, &arg);
}

Local<JSValue> ValueProgram::New(const Direction& value) {
	Local<JSValue> arg = worker->New((uint)value);
	return _Direction.local()->Call(worker, 1, &arg);
}

Local<JSValue> ValueProgram::New(const KeyboardType& value) {
	Local<JSValue> arg = worker->New((uint)value);
	return _KeyboardType.local()->Call(worker, 1, &arg);
}

Local<JSValue> ValueProgram::New(const KeyboardReturnType& value) {
	Local<JSValue> arg = worker->New((uint)value);
	return _KeyboardReturnType.local()->Call(worker, 1, &arg);
}

Local<JSValue> ValueProgram::New(const Border& value) {
	Local<JSValue> args[] = {
		worker->New(value.width),
		worker->New(value.color.r()),
		worker->New(value.color.g()),
		worker->New(value.color.b()),
		worker->New(value.color.a()),
	};
	return _Border.local()->Call(worker, 5, args);
}

Local<JSValue> ValueProgram::New(const Shadow& value) {
	Local<JSValue> args[] = {
		worker->New(value.offset_x),
		worker->New(value.offset_y),
		worker->New(value.size),
		worker->New(value.color.b()),
		worker->New(value.color.g()),
		worker->New(value.color.b()),
		worker->New(value.color.a()),
	};
	return _Shadow.local()->Call(worker, 7, args);
}

Local<JSValue> ValueProgram::New(const Color& value) {
	Local<JSValue> args[] = {
		worker->New(value.r()),
		worker->New(value.g()),
		worker->New(value.b()),
		worker->New(value.a()),
	};
	return _Color.local()->Call(worker, 4, args);
}

Local<JSValue> ValueProgram::New(const Vec2& value) {
	Local<JSValue> args[] = {
		worker->New(value.x()),
		worker->New(value.y()),
	};
	return _Vec2.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const Vec3& value) {
	Local<JSValue> args[] = {
		worker->New(value.x()),
		worker->New(value.y()),
		worker->New(value.z()),
	};
	return _Vec3.local()->Call(worker, 3, args);
}

Local<JSValue> ValueProgram::New(const Vec4& value) {
	Local<JSValue> args[] = {
		worker->New(value.x()),
		worker->New(value.y()),
		worker->New(value.z()),
		worker->New(value.w()),
	};
	return _Vec4.local()->Call(worker, 4, args);
}

Local<JSValue> ValueProgram::New(cCurve& value) {
	Local<JSValue> args[] = {
		worker->New(value.point1().x()),
		worker->New(value.point1().y()),
		worker->New(value.point2().x()),
		worker->New(value.point2().y()),
	};
	return _Curve.local()->Call(worker, 4, args);
}

Local<JSValue> ValueProgram::New(const Rect& value) {
	Local<JSValue> args[] = {
		worker->New(value.origin.x()),
		worker->New(value.origin.y()),
		worker->New(value.size.width()),
		worker->New(value.size.height()),
	};
	return _Rect.local()->Call(worker, 4, args);
}

Local<JSValue> ValueProgram::New(const Mat& value) {
	Local<JSValue> args[] = {
		worker->New(value[0]),
		worker->New(value[1]),
		worker->New(value[2]),
		worker->New(value[3]),
		worker->New(value[4]),
		worker->New(value[5]),
	};
	return _Mat.local()->Call(worker, 6, args);
}

Local<JSValue> ValueProgram::New(const Mat4& value) {
	Local<JSValue> args[] = {
		worker->New(value[0]),
		worker->New(value[1]),
		worker->New(value[2]),
		worker->New(value[3]),
		worker->New(value[4]),
		worker->New(value[5]),
		worker->New(value[6]),
		worker->New(value[7]),
		worker->New(value[8]),
		worker->New(value[9]),
		worker->New(value[10]),
		worker->New(value[11]),
		worker->New(value[12]),
		worker->New(value[13]),
		worker->New(value[14]),
		worker->New(value[15]),
	};
	return _Mat4.local()->Call(worker, 16, args);
}

Local<JSValue> ValueProgram::New(const Value& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.value),
	};
	return _Value.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const BackgroundPosition& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.value),
	};
	return _BackgroundPosition.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const BackgroundSize& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.value),
	};
	return _BackgroundSize.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const BackgroundPtr& value) {
	Qk_UNIMPLEMENTED();
	return Local<JSValue>();
}

Local<JSValue> ValueProgram::New(const TextColor& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.value.r()),
		worker->New(value.value.g()),
		worker->New(value.value.b()),
		worker->New(value.value.a()),
	};
	return _TextColor.local()->Call(worker, 5, args);
}

Local<JSValue> ValueProgram::New(const TextSize& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.value),
	};
	return _TextSize.local()->Call(worker, 1, args);
}

Local<JSValue> ValueProgram::New(const TextFamily& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.name()),
	};
	return _TextFamily.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const TextSlant& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New((uint)value.value),
	};
	return _TextSlant.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const TextShadow& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.value.offset_x),
		worker->New(value.value.offset_y),
		worker->New(value.value.size),
		worker->New(value.value.color.r()),
		worker->New(value.value.color.g()),
		worker->New(value.value.color.b()),
		worker->New(value.value.color.a()),
	};
	return _TextShadow.local()->Call(worker, 8, args);
}

Local<JSValue> ValueProgram::New(const TextLineHeight& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New(value.value.height),
	};
	return _TextLineHeight.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const TextDecoration& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New((uint)value.value),
	};
	return _TextDecoration.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(cString& value) {
	return worker->New(value);
}

Local<JSValue> ValueProgram::New(const bool& value) {
	return worker->New(value);
}

Local<JSValue> ValueProgram::New(const float& value) {
	return worker->New(value);
}

Local<JSValue> ValueProgram::New(const int& value) {
	return worker->New(value);
}

Local<JSValue> ValueProgram::New(const uint& value) {
	return worker->New(value);
}

Local<JSValue> ValueProgram::New(const TextOverflow& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New((uint)value.value),
	};
	return _TextOverflow.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const TextWhiteSpace& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New((uint)value.value),
	};
	return _TextWhiteSpace.local()->Call(worker, 2, args);
}

Local<JSValue> ValueProgram::New(const BackgroundPositionCollection& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.x.type),
		worker->New((uint)value.x.value),
		worker->New((uint)value.y.type),
		worker->New((uint)value.y.value),
	};
	return _BackgroundPositionCollection.local()->Call(worker, 4, args);
}

Local<JSValue> ValueProgram::New(const BackgroundSizeCollection& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.x.type),
		worker->New((uint)value.x.value),
		worker->New((uint)value.y.type),
		worker->New((uint)value.y.value),
	};
	return _BackgroundSizeCollection.local()->Call(worker, 4, args);
}

static void throw_error(Worker* worker, Local<JSValue> value, cChar* msg, cChar* help = nullptr) {
	// Bad argument. Input.type = `Bad`
	// reference value, "

	String msg2 = String::format(msg ? msg : "`%s`", *value->ToStringValue(worker) );
	Local<JSValue> err;

	if (help) {
		err = worker->NewTypeError("Bad argument. %s. Examples: %s", *msg2, help);
	} else {
		err = worker->NewTypeError("Bad argument. %s.", *msg2);
	}
	worker->throwError(err);
}

void ValueProgram::throwError(Local<JSValue> value, cChar* msg, cChar* help) {
	throw_error(worker, value, msg, help);
}

static const Map<String, cCurve*> CURCE({
	{"linear", &LINEAR },
	{"ease", &EASE },
	{"ease_in", &EASE_IN },
	{"ease_out", &EASE_OUT },
	{"ease_in_out", &EASE_IN_OUT },
});

#define js_parse(Type, ok) { \
Local<JSObject> obj;\
Local<JSValue> val;\
if (desc) {\
	Local<JSValue> args[] = { in, worker->New(desc) };\
	val = _parse##Type.local()->Call(worker, 2, args);\
} else {\
	val = _parse##Type.local()->Call(worker, 1, &in);\
}\
if ( val.IsEmpty() ) {\
	return false;\
} else {\
	obj = val.To<JSObject>();\
} \
ok \
return true;\
}

// parse
bool ValueProgram::parseTextAlign(Local<JSValue> in, TextAlign& out, cChar* desc) {
	js_parse(TextAlign, {
		out = (TextAlign)obj->ToUint32Value(worker);
	});
}

bool ValueProgram::parseAlign(Local<JSValue> in, Align& out, cChar* desc) {
	js_parse(Align, {
		out = (Align)obj->ToUint32Value(worker);
	});
}

bool ValueProgram::parseContentAlign(Local<JSValue> in, ContentAlign& out, cChar* desc) {
	js_parse(ContentAlign, {
		out = (ContentAlign)obj->ToUint32Value(worker);
	});
}

bool ValueProgram::parseRepeat(Local<JSValue> in, Repeat& out, cChar* desc) {
	js_parse(Repeat, {
		out = (Repeat)obj->ToUint32Value(worker);
	});
}

bool ValueProgram::parseDirection(Local<JSValue> in, Direction& out, cChar* desc) {
	js_parse(Direction, {
		out = (Direction)obj->ToUint32Value(worker);
	});
}

bool ValueProgram::parseKeyboardType(Local<JSValue> in, KeyboardType& out, cChar* desc) {
	js_parse(KeyboardType, {
		out = (KeyboardType)obj->ToUint32Value(worker);
	});
}

bool ValueProgram::parseKeyboardReturnType(Local<JSValue> in, KeyboardReturnType& out, cChar* desc) {
	js_parse(KeyboardReturnType, {
		out = (KeyboardReturnType)obj->ToUint32Value(worker);
	});
}

bool ValueProgram::parseBorder(Local<JSValue> in, Border& out, cChar* desc) {
	js_parse(Border, {
		out.width = obj->Get(worker, worker->strs()->width())->ToNumberValue(worker);
		out.color.r(obj->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.color.g(obj->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.color.b(obj->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.color.a(obj->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}

bool ValueProgram::parseShadow(Local<JSValue> in, Shadow& out, cChar* desc) {
	js_parse(Shadow, {
		out.offset_x = obj->Get(worker, worker->strs()->offsetX())->ToNumberValue(worker);
		out.offset_y = obj->Get(worker, worker->strs()->offsetY())->ToNumberValue(worker);
		out.size = obj->Get(worker, worker->strs()->size())->ToNumberValue(worker);
		out.color.r(obj->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.color.g(obj->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.color.b(obj->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.color.a(obj->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}

bool ValueProgram::parseColor(Local<JSValue> in, Color& out, cChar* desc) {
	js_parse(Color, {
		out.r(obj->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.g(obj->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.b(obj->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.a(obj->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}

bool ValueProgram::parseVec2(Local<JSValue> in, Vec2& out, cChar* desc) {
	js_parse(Vec2, {
		out.x(obj->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.y(obj->Get(worker, worker->strs()->y())->ToNumberValue(worker));
	});
}

bool ValueProgram::parseVec3(Local<JSValue> in, Vec3& out, cChar* desc) {
	js_parse(Vec3, {
		out.x(obj->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.y(obj->Get(worker, worker->strs()->y())->ToNumberValue(worker));
		out.z(obj->Get(worker, worker->strs()->z())->ToNumberValue(worker));
	});
}

bool ValueProgram::parseVec4(Local<JSValue> in, Vec4& out, cChar* desc) {
	js_parse(Vec4, {
		out.x(obj->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.y(obj->Get(worker, worker->strs()->y())->ToNumberValue(worker));
		out.z(obj->Get(worker, worker->strs()->z())->ToNumberValue(worker));
		out.w(obj->Get(worker, worker->strs()->w())->ToNumberValue(worker));
	});
}

bool ValueProgram::parseCurve(Local<JSValue> in, Curve& out, cChar* desc) {
	if ( in->IsString(worker) ) {
		JS_WORKER();
		auto it = CURCE.find( in->ToStringValue(worker,1) );
		if ( !it.is_null() ) {
			out = *it.value();
			return true;
		}
	}
	js_parse(Curve, {
		out = Curve(obj->Get(worker, worker->strs()->point1X())->ToNumberValue(worker),
								obj->Get(worker, worker->strs()->point1Y())->ToNumberValue(worker),
								obj->Get(worker, worker->strs()->point2X())->ToNumberValue(worker),
								obj->Get(worker, worker->strs()->point2Y())->ToNumberValue(worker));
	});
}

bool ValueProgram::parseRect(Local<JSValue> in, Rect& out, cChar* desc) {
	js_parse(Rect, {
		out.origin.x(obj->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.origin.y(obj->Get(worker, worker->strs()->y())->ToNumberValue(worker));
		out.size.width(obj->Get(worker, worker->strs()->width())->ToNumberValue(worker));
		out.size.height(obj->Get(worker, worker->strs()->height())->ToNumberValue(worker));
	});
}

bool ValueProgram::parseMat(Local<JSValue> in, Mat& out, cChar* desc) {
	js_parse(Mat, {
		Local<JSArray> mat = obj->Get(worker, worker->strs()->_value()).To<JSArray>();
		out.m0(mat->Get(worker, 0)->ToNumberValue(worker));
		out.m1(mat->Get(worker, 1)->ToNumberValue(worker));
		out.m2(mat->Get(worker, 2)->ToNumberValue(worker));
		out.m3(mat->Get(worker, 3)->ToNumberValue(worker));
	});
}

bool ValueProgram::parseMat4(Local<JSValue> in, Mat4& out, cChar* desc) {
	js_parse(Mat4, {
		Local<JSArray> mat = obj->Get(worker, worker->strs()->_value()).To<JSArray>();
		out.m0(mat->Get(worker, 0)->ToNumberValue(worker));
		out.m1(mat->Get(worker, 1)->ToNumberValue(worker));
		out.m2(mat->Get(worker, 2)->ToNumberValue(worker));
		out.m3(mat->Get(worker, 3)->ToNumberValue(worker));
		out.m4(mat->Get(worker, 4)->ToNumberValue(worker));
		out.m5(mat->Get(worker, 5)->ToNumberValue(worker));
		out.m6(mat->Get(worker, 6)->ToNumberValue(worker));
		out.m7(mat->Get(worker, 7)->ToNumberValue(worker));
		out.m8(mat->Get(worker, 8)->ToNumberValue(worker));
		out.m9(mat->Get(worker, 9)->ToNumberValue(worker));
		out.m10(mat->Get(worker, 10)->ToNumberValue(worker));
		out.m11(mat->Get(worker, 11)->ToNumberValue(worker));
		out.m12(mat->Get(worker, 12)->ToNumberValue(worker));
		out.m13(mat->Get(worker, 13)->ToNumberValue(worker));
		out.m14(mat->Get(worker, 14)->ToNumberValue(worker));
		out.m15(mat->Get(worker, 15)->ToNumberValue(worker));
	});
}

bool ValueProgram::parseValue(Local<JSValue> in, Value& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out.type = ValueType::PIXEL;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(Value, {
		out.type = (ValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = obj->Get(worker, worker->strs()->value())->ToNumberValue(worker);
	});
}

bool ValueProgram::parseBackgroundPosition(Local<JSValue> in,
																					 BackgroundPosition& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out.type = BackgroundPositionType::PIXEL;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(BackgroundPosition, {
		out.type = (BackgroundPositionType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = obj->Get(worker, worker->strs()->value())->ToNumberValue(worker);
	});
}

bool ValueProgram::parseBackgroundSize(Local<JSValue> in, BackgroundSize& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out.type = BackgroundSizeType::PIXEL;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(BackgroundSize, {
		out.type = (BackgroundSizeType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = obj->Get(worker, worker->strs()->value())->ToNumberValue(worker);
	});
}

bool ValueProgram::parseBackground(Local<JSValue> in, BackgroundPtr& out, cChar* desc) {
	if (in->IsNull()) {
		out = nullptr;
		return true;
	}
	js_parse(Background, {
		out = Wrap<Background>::unpack(obj)->self();
	});
}

bool ValueProgram::parseBackgroundPositionCollection(Local<JSValue> in, BackgroundPositionCollection& out, cChar* desc) {
	js_parse(BackgroundPositionCollection, {
		auto x = obj->Get(worker, worker->strs()->x()).To();
		auto y = obj->Get(worker, worker->strs()->y()).To();
		out.x = BackgroundPosition({
			(BackgroundPositionType)x->Get(worker, worker->strs()->type())->ToUint32Value(worker),
			(float)                 x->Get(worker, worker->strs()->value())->ToNumberValue(worker)
		});
		out.y = BackgroundPosition({
			(BackgroundPositionType)y->Get(worker, worker->strs()->type())->ToUint32Value(worker),
			(float)                 y->Get(worker, worker->strs()->value())->ToNumberValue(worker)
		});
	});
}

bool ValueProgram::parseBackgroundSizeCollection(Local<JSValue> in, BackgroundSizeCollection& out, cChar* desc) {
	js_parse(BackgroundSizeCollection, {
		auto x = obj->Get(worker, worker->strs()->x()).To();
		auto y = obj->Get(worker, worker->strs()->y()).To();
		out.x = BackgroundSize({
			(BackgroundSizeType)x->Get(worker, worker->strs()->type())->ToUint32Value(worker),
			(float)             x->Get(worker, worker->strs()->value())->ToNumberValue(worker)
		});
		out.y = BackgroundSize({
			(BackgroundSizeType)y->Get(worker, worker->strs()->type())->ToUint32Value(worker),
			(float)             y->Get(worker, worker->strs()->value())->ToNumberValue(worker)
		});
	});
}

bool ValueProgram::parseValues(Local<JSValue> in, Array<Value>& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out.push(Value{ ValueType::PIXEL, (float)in->ToNumberValue(worker) });
		return true;
	}
	js_parse(Values, {
		Local<JSArray> arr = obj.To<JSArray>();
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			Local<JSObject> obj = arr->Get(worker, i).To<JSObject>();
			out.push(Value{
				(ValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker),
				(float)    obj->Get(worker, worker->strs()->value())->ToNumberValue(worker)
			});
		}
	});
}

bool ValueProgram::parseFloats(Local<JSValue> in, Array<float>& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out.push(in->ToNumberValue(worker));
		return true;
	}
	js_parse(Floats, {
		Local<JSArray> arr = obj.To<JSArray>();
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			out.push( arr->Get(worker, i)->ToNumberValue(worker) );
		}
	});
}

bool ValueProgram::parseAligns(Local<JSValue> in, Array<Align>& out, cChar* desc) {
	js_parse(Aligns, {
		Local<JSArray> arr = obj.To<JSArray>();
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			out.push( Align(arr->Get(worker, i)->ToUint32Value(worker)) );
		}
	});
	
//	
//	Local<JSObject> obj;
//	Local<JSValue> val;
//	if (desc) {
//		Local<JSValue> args[] = { in, worker->New(desc) };
//		val = _parseAligns.local()->Call(worker, 2, args);
//	} else {
//		val = _parseAligns.local()->Call(worker, 1, &in);
//	}
//	if ( val.IsEmpty() ) {
//		return false;
//	} else {
//		obj = val.To<JSObject>();
//	}
//	{
//		Local<JSArray> arr = obj.To<JSArray>();
//		for(int i = 0, len = arr->Length(worker); i < len; i++) {
//			out.push( Align(arr->Get(worker, i)->ToUint32Value(worker)) );
//		}
//	}
//	return true;
//	
}

bool ValueProgram::parseTextColor(Local<JSValue> in, TextColor& out, cChar* desc) {
	js_parse(TextColor, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value.r(obj->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.value.g(obj->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.value.b(obj->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.value.a(obj->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}

bool ValueProgram::parseTextSize(Local<JSValue> in, TextSize& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out.type = TextValueType::VALUE;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(TextSize, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = obj->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseTextFamily(Local<JSValue> in, TextFamily& out, cChar* desc) {
	js_parse(TextFamily, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		String fonts = obj->Get(worker, worker->strs()->value())->ToStringValue(worker);
		out.value = FontPool::get_font_familys_id(fonts);
	});
}
bool ValueProgram::parseTextSlant(Local<JSValue> in, TextSlant& out, cChar* desc) {
	js_parse(TextSlant, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextSlantEnum)obj->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseTextShadow(Local<JSValue> in, TextShadow& out, cChar* desc) {
	js_parse(TextShadow, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value.offset_x = obj->Get(worker, worker->strs()->offsetX())->ToNumberValue(worker);
		out.value.offset_y = obj->Get(worker, worker->strs()->offsetY())->ToNumberValue(worker);
		out.value.size = obj->Get(worker, worker->strs()->size())->ToNumberValue(worker);
		out.value.color.r(obj->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.value.color.g(obj->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.value.color.b(obj->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.value.color.a(obj->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}
bool ValueProgram::parseTextLineHeight(Local<JSValue> in,
																			 TextLineHeight& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out.type = TextValueType::VALUE;
		out.value.height = in->ToNumberValue(worker);
		return true;
	}
	js_parse(TextLineHeight, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value.height = obj->Get(worker, worker->strs()->height())->ToNumberValue(worker);
	});
}
bool ValueProgram::parseTextDecoration(Local<JSValue> in,
																			 TextDecoration& out, cChar* desc) {
	js_parse(TextDecoration, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextDecorationEnum)obj->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseString(Local<JSValue> in, String& out, cChar* desc) {
	out = in->ToStringValue(worker);
	return true;
}
bool ValueProgram::parsebool(Local<JSValue> in, bool& out, cChar* desc) {
	out = in->ToBooleanValue(worker);
	return true;
}
bool ValueProgram::parsefloat(Local<JSValue> in, float& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out = in->ToNumberValue(worker);
		return true;
	}
	if (in->IsString(worker)) {
		if (in->ToStringValue(worker).to_float(&out)) {
			return true;
		}
	}
	throw_error(worker, in, desc);
	return false;
}
bool ValueProgram::parseint(Local<JSValue> in, int& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out = in->ToInt32Value(worker);
		return true;
	}
	if (in->IsString(worker)) {
		if (in->ToStringValue(worker).to_int(&out)) {
			return true;
		}
	}
	throw_error(worker, in, desc);
	return false;
}
bool ValueProgram::parseuint(Local<JSValue> in, uint& out, cChar* desc) {
	if (in->IsNumber(worker)) {
		out = in->ToUint32Value(worker);
		return true;
	}
	if (in->IsString(worker)) {
		if (in->ToStringValue(worker).to_uint(&out)) {
			return true;
		}
	}
	throw_error(worker, in, desc);
	return false;
}
bool ValueProgram::parseTextOverflow(Local<JSValue> in, TextOverflow& out, cChar* desc) {
	js_parse(TextOverflow, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextOverflowEnum)obj->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseTextWhiteSpace(Local<JSValue> in,
																			TextWhiteSpace& out, cChar* desc) {
	js_parse(TextWhiteSpace, {
		out.type = (TextValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextWhiteSpaceEnum)obj->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}

bool ValueProgram::isBase(Local<JSValue> arg) {
	return arg->InstanceOf(worker, _Base.local());
}

void binding_background(Local<JSObject> exports, Worker* worker);

/**
 * @class NativeValue
 */
class NativeValue {
 public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		// binding Background / BackgroundImage / BackgroundGradient
		binding_background(exports, worker);
		
		Local<JSObject> _prve = worker->NewObject();
		exports->Set(worker, worker->NewAscii("_priv"), _prve);
		
		{
			TryCatch try_catch;

			if (worker->runNativeScript(WeakBuffer((Char*)
							native_js::INL_native_js_code__value_,
							native_js::INL_native_js_code__value_count_), "_value.js", exports).IsEmpty()) {
				if ( try_catch.HasCaught() ) {
					worker->reportException(&try_catch);
				}
				Qk_FATAL("Could not initialize native/_value.js");
			}
		}
		worker->_inl->_values = new ValueProgram(worker, exports, _prve);
	}
};

JS_REG_MODULE(_value, NativeValue);
JS_END
