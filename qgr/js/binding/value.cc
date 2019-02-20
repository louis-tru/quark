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

#include "qgr/js/str.h"
#include "qgr/js/js-1.h"
#include "qgr/js/qgr.h"
#include "qgr/js/value.h"
#include "qgr/font.h"
#include "qgr/draw.h"
#include "native-ext-js.h"

/**
 * @ns qgr::js
 */

JS_BEGIN


ValueProgram::ValueProgram(Worker* worker,
												 Local<JSObject> exports,
												 Local<JSObject> priv): worker(worker) {
#define Ascii(s) worker->New(s,1)
	
	Local<JSValue> help = Ascii("help");
	
	_isBase.Reset(worker, priv->Get(worker,Ascii("_isBase")).To<JSFunction>());
	
#define js_init_func(Name, Type) \
	XX_DEBUG("init value %s", #Name);\
	_constructor##Name .Reset(worker, priv->Get(worker,Ascii(#Name)).To<JSFunction>()); \
	_parse##Name       .Reset(worker, priv->Get(worker,Ascii("parse"#Name)).To<JSFunction>()); \
	if (_constructor##Name.local()->IsObject()) { \
		_parse##Name##Help.Reset(worker, \
			_constructor##Name.local()->Get(worker,help).To<JSFunction>()); \
	}\
	_##Name.Reset(worker, priv->Get(worker,Ascii("_"#Name)).To<JSFunction>());
	
	js_value_table(js_init_func)
	
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
Local<JSValue> ValueProgram::New(const CGRect& value) {
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
	XX_UNIMPLEMENTED();
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
Local<JSValue> ValueProgram::New(const TextStyle& value) {
	Local<JSValue> args[] = {
		worker->New((uint)value.type),
		worker->New((uint)value.value),
	};
	return _TextStyle.local()->Call(worker, 2, args);
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
Local<JSValue> ValueProgram::New(const String& value) {
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

static void parse_error_throw(Worker* worker, Local<JSValue> value,
															cchar* msg, Local<JSFunction> help = Local<JSFunction>()) {
	String help_str;
	
	if (!help.IsEmpty() && help->IsFunction()) {
		Local<JSValue> o = help->Call(worker);
		if (!o.IsEmpty()) {
			help_str = o->ToStringValue(worker);
		}
	}
	// Bad argument. Input.type = `Bad`
	// reference value, "
	
	String msg2 = String::format(msg, *String::format("`%s`", *value->ToStringValue(worker)) );
	Local<JSValue> err;
	
	if (help_str.is_empty()) {
		err = worker->NewTypeError("Bad argument. %s.", *msg2);
	} else {
		err = worker->NewTypeError("Bad argument. %s. Examples: %s", *msg2, *help_str);
	}
	worker->throw_err(err);
}

void ValueProgram::throwError(Local<JSValue> value, cchar* msg, Local<JSFunction> help) {
	parse_error_throw(worker, value, msg, help);
}

#define DEF_STRING_VALUES(NAME, NAME2) #NAME2,
#define INIT_CONST_MAP(TYPE, NAME) { string_values[(int)TYPE::NAME], TYPE::NAME },
#define DEF_CONST_MAP(NAME, TYPE) \
static const Map<String, TYPE> NAME({ XX_##NAME(INIT_CONST_MAP) })

static cchar* string_values[] = {
	JS_ENUM_VALUE(DEF_STRING_VALUES)
};

DEF_CONST_MAP(TEXT_ALIGN, TextAlign);
DEF_CONST_MAP(ALIGN, Align);
DEF_CONST_MAP(CONTENT_ALIGN, ContentAlign);
DEF_CONST_MAP(REPEAT, Repeat);
DEF_CONST_MAP(DIRECTION, Direction);
DEF_CONST_MAP(KEYBOARD_TYPE, KeyboardType);
DEF_CONST_MAP(KEYBOARD_RETURN_TYPE, KeyboardReturnType);

static const Map<String, cCurve*> CURCE({
	{"linear", &LINEAR },
	{"ease", &EASE },
	{"ease_in", &EASE_IN },
	{"ease_out", &EASE_OUT },
	{"ease_in_out", &EASE_IN_OUT },
});
static Map<uint, cCurve*> CURCE2({
	{ 0, &LINEAR },
	{ 1, &EASE },
	{ 2, &EASE_IN },
	{ 3, &EASE_OUT },
	{ 4, &EASE_IN_OUT },
});

#define js_parse(Type, ok) { \
Local<JSObject> object;\
if ( in->IsString(worker) ) {\
	Local<JSValue> o = _parse##Type.local()->Call(worker, 1, &in);\
	if ( o.IsEmpty() ) {\
		return false;\
	} else if (o->IsNull(worker)) { \
		goto err;\
	}else {\
		object = o.To<JSObject>();\
	} \
} else if ( is##Type(in) ) {\
	object = in.To<JSObject>();\
} else {\
err:\
	parse_error_throw(worker, in, desc, _parse##Type##Help.local());\
	return false;\
}\
ok \
return true;\
}

#define js_parse2(Type, FIND_MAP, ok) { \
Local<JSObject> object; \
if ( in->IsString(worker) ) { \
	auto it = FIND_MAP.find(in->ToStringValue(worker)); \
	if ( it.is_null() ) { /* err */ \
		goto err; \
	} else { \
		out = it.value(); \
		return true; \
	} \
} else if ( is##Type(in) ) { \
	object = in.To<JSObject>(); \
} else { \
 err: \
	parse_error_throw(worker, in, desc, _parse##Type##Help.local()); \
	return false; \
} \
ok \
return true; \
}

// parse
bool ValueProgram::parseTextAlign(Local<JSValue> in, TextAlign& out, cchar* desc) {
	js_parse2(TextAlign, TEXT_ALIGN, {
		out = (TextAlign)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseAlign(Local<JSValue> in, Align& out, cchar* desc) {
	js_parse2(Align, ALIGN, {
		out = (Align)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseContentAlign(Local<JSValue> in, ContentAlign& out, cchar* desc) {
	js_parse2(ContentAlign, CONTENT_ALIGN, {
		out = (ContentAlign)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseRepeat(Local<JSValue> in, Repeat& out, cchar* desc) {
	js_parse2(Repeat, REPEAT, {
		out = (Repeat)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseDirection(Local<JSValue> in, Direction& out, cchar* desc) {
	js_parse2(Direction, DIRECTION, {
		out = (Direction)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseKeyboardType(Local<JSValue> in, KeyboardType& out, cchar* desc) {
	js_parse2(KeyboardType, KEYBOARD_TYPE, {
		out = (KeyboardType)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseKeyboardReturnType(Local<JSValue> in, KeyboardReturnType& out, cchar* desc) {
	js_parse2(KeyboardReturnType, KEYBOARD_RETURN_TYPE, {
		out = (KeyboardReturnType)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseBorder(Local<JSValue> in, Border& out, cchar* desc) {
	Local<JSObject> object;
	if ( in->IsString(worker) ) {
		Local<JSValue> o = worker->New(_parseBorder)->Call(worker, 1, &in);
		if ( o.IsEmpty() ) { // err
			return false;
		} else if (o->IsNull(worker)) {
			goto err;
		} else {
			object = o.To<JSObject>();
		}
	} else if ( isBorder(in) ) {
		object = in.To<JSObject>();
	} else {
	err:
		parse_error_throw(worker, in, desc, _parseBorderHelp.local());
		return false;
	}
	out.width = object->Get(worker, worker->strs()->width())->ToNumberValue(worker);
	out.color.r(object->Get(worker, worker->strs()->r())->ToUint32Value(worker));
	out.color.g(object->Get(worker, worker->strs()->g())->ToUint32Value(worker));
	out.color.b(object->Get(worker, worker->strs()->b())->ToUint32Value(worker));
	out.color.a(object->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	return true;
}
bool ValueProgram::parseShadow(Local<JSValue> in, Shadow& out, cchar* desc) {
	js_parse(Shadow, {
		out.offset_x = object->Get(worker, worker->strs()->offsetX())->ToNumberValue(worker);
		out.offset_y = object->Get(worker, worker->strs()->offsetY())->ToNumberValue(worker);
		out.size = object->Get(worker, worker->strs()->size())->ToNumberValue(worker);
		out.color.r(object->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.color.g(object->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.color.b(object->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.color.a(object->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}
bool ValueProgram::parseColor(Local<JSValue> in, Color& out, cchar* desc) {
	js_parse(Color, {
		out.r(object->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.g(object->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.b(object->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.a(object->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}
bool ValueProgram::parseVec2(Local<JSValue> in, Vec2& out, cchar* desc) {
	js_parse(Vec2, {
		out.x(object->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.y(object->Get(worker, worker->strs()->y())->ToNumberValue(worker));
	});
}
bool ValueProgram::parseVec3(Local<JSValue> in, Vec3& out, cchar* desc) {
	js_parse(Vec3, {
		out.x(object->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.y(object->Get(worker, worker->strs()->y())->ToNumberValue(worker));
		out.z(object->Get(worker, worker->strs()->z())->ToNumberValue(worker));
	});
}
bool ValueProgram::parseVec4(Local<JSValue> in, Vec4& out, cchar* desc) {
	js_parse(Vec4, {
		out.x(object->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.y(object->Get(worker, worker->strs()->y())->ToNumberValue(worker));
		out.z(object->Get(worker, worker->strs()->z())->ToNumberValue(worker));
		out.w(object->Get(worker, worker->strs()->w())->ToNumberValue(worker));
	});
}
bool ValueProgram::parseCurve(Local<JSValue> in, Curve& out, cchar* desc) {
	Local<JSObject> object;
	
	if ( in->IsString(worker) ) {
		JS_WORKER();
		auto it = CURCE.find( in->ToStringValue(worker,1) );
		if ( !it.is_null() ) {
			out = *it.value(); return true;
		}
		Local<JSValue> o = worker->New(_parseCurve)->Call(worker, 1, &in);
		if ( o.IsEmpty() || o->IsNull(worker) ) {
			return false;
		} else {
			object = o.To<JSObject>();
		}
	} else if ( in->IsUint32(worker) ) {
		auto it = CURCE2.find(in->ToUint32Value(worker));
		if ( !it.is_null() ) {
			out = *it.value(); return true;
		}
		return false;
	} else if ( isCurve(in) ) {
		object = in.To<JSObject>();
	} else {
		return false;
	}
	
	out = Curve(object->Get(worker, worker->strs()->point1X())->ToNumberValue(worker),
							object->Get(worker, worker->strs()->point1Y())->ToNumberValue(worker),
							object->Get(worker, worker->strs()->point2X())->ToNumberValue(worker),
							object->Get(worker, worker->strs()->point2Y())->ToNumberValue(worker));
	return true;
}
bool ValueProgram::parseRect(Local<JSValue> in, CGRect& out, cchar* desc) {
	js_parse(Rect, {
		out.origin.x(object->Get(worker, worker->strs()->x())->ToNumberValue(worker));
		out.origin.y(object->Get(worker, worker->strs()->y())->ToNumberValue(worker));
		out.size.width(object->Get(worker, worker->strs()->width())->ToNumberValue(worker));
		out.size.height(object->Get(worker, worker->strs()->height())->ToNumberValue(worker));
	});
}
bool ValueProgram::parseMat(Local<JSValue> in, Mat& out, cchar* desc) {
	js_parse(Mat, {
		Local<JSArray> mat = object->Get(worker, worker->strs()->_value()).To<JSArray>();
		out.m0(mat->Get(worker, 0)->ToNumberValue(worker));
		out.m1(mat->Get(worker, 1)->ToNumberValue(worker));
		out.m2(mat->Get(worker, 2)->ToNumberValue(worker));
		out.m3(mat->Get(worker, 3)->ToNumberValue(worker));
	});
}
bool ValueProgram::parseMat4(Local<JSValue> in, Mat4& out, cchar* desc) {
	js_parse(Mat4, {
		Local<JSArray> mat = object->Get(worker, worker->strs()->_value()).To<JSArray>();
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
bool ValueProgram::parseValue(Local<JSValue> in, Value& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out.type = ValueType::PIXEL;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(Value, {
		out.type = (ValueType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = object->Get(worker, worker->strs()->value())->ToNumberValue(worker);
	});
}
bool ValueProgram::parseBackgroundPosition(Local<JSValue> in,
																					 BackgroundPosition& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out.type = BackgroundPositionType::PIXEL;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(BackgroundPosition, {
		out.type = (BackgroundPositionType)
		object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = object->Get(worker, worker->strs()->value())->ToNumberValue(worker);
	});
}
bool ValueProgram::parseBackgroundSize(Local<JSValue> in, BackgroundSize& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out.type = BackgroundSizeType::PIXEL;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(BackgroundSize, {
		out.type = (BackgroundSizeType)
		object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = object->Get(worker, worker->strs()->value())->ToNumberValue(worker);
	});
}
bool ValueProgram::parseBackground(Local<JSValue> in, BackgroundPtr& out, cchar* desc) {
	if (in->IsNull()) {
		out = nullptr;
		return true;
	}
	js_parse(Background, {
		out = Wrap<Background>::unpack(object)->self();
	});
}
bool ValueProgram::parseBackgroundImage(Local<JSValue> in, BackgroundPtr& out, cchar* desc) {
	if (in->IsNull()) {
		out = nullptr;
		return true;
	}
	Local<JSObject> object;
	if ( in->IsString(worker) ) {
		Local<JSValue> args[] = { in, worker->New(true) };
		Local<JSValue> o = worker->New(_parseBackground)->Call(worker, 2, args);
		if ( o.IsEmpty() ) { // err
			return false;
		} else if (o->IsNull(worker)) {
			goto err;
		} else {
			object = o.To<JSObject>();
		}
	} else if ( isBackground(in) ) {
		object = in.To<JSObject>();
	} else {
	err:
		parse_error_throw(worker, in, desc, _parseBackgroundHelp.local());
		return false;
	}
	out = Wrap<Background>::unpack(object)->self();
	return true;
}
bool ValueProgram::parseRepeats(Local<JSValue> in, Array<Repeat>& out, cchar* desc) {
	Local<JSArray> arr;
	if (in->IsString(worker)) {
		Local<JSValue> o = _parseRepeats.local()->Call(worker, 1, &in);
		if (o.IsEmpty()) {
			return false;
		}
		if ( !o->IsNull(worker)) {
			arr = o.To<JSArray>();
		}
	} else if ( isRepeat(in) ) {
		out.push(Repeat(in.To()->Get(worker, worker->strs()->value())->ToUint32Value(worker)));
		return true;
	}
	if (!arr.IsEmpty()) {
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			Local<JSObject> obj = arr->Get(worker, i).To();
			out.push(Repeat(obj->Get(worker, worker->strs()->value())->ToUint32Value(worker)));
		}
		return true;
	}
	parse_error_throw(worker, in, desc, _parseRepeatsHelp.local());
	return false;
}
bool ValueProgram::parseBackgroundPositions(Local<JSValue> in,
																						Array<BackgroundPositionCollection>& out, cchar* desc) {
	Local<JSArray> arr;
	if (in->IsString(worker)) {
		Local<JSValue> o = _parseBackgroundPositions.local()->Call(worker, 1, &in);
		if (o.IsEmpty()) {
			return false;
		}
		if ( !o->IsNull(worker)) {
			arr = o.To<JSArray>();
		}
	} else if ( isBackgroundPosition(in) ) {
		auto pos = BackgroundPosition({
			(BackgroundPositionType)in.To()->Get(worker, worker->strs()->type())->ToUint32Value(worker),
			(float)in.To()->Get(worker, worker->strs()->value())->ToNumberValue(worker)
		});
		out.push({ pos, pos });
		return true;
	}
	if (!arr.IsEmpty()) {
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			Local<JSArray> items = arr->Get(worker, i).To<JSArray>();
			BackgroundPositionCollection poss;
			for (int j = 0; j < 2; j++) {
				Local<JSObject> obj = items->Get(worker, i).To();
				auto pos = BackgroundPosition({
					(BackgroundPositionType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker),
					(float)obj->Get(worker, worker->strs()->value())->ToNumberValue(worker)
				});
				if (j) {
					poss.y = pos;
				} else {
					poss.x = pos;
				}
			}
			out.push(poss);
		}
		return true;
	}
	parse_error_throw(worker, in, desc, _parseBackgroundPositionsHelp.local());
	return false;
}
bool ValueProgram::parseBackgroundSizes(Local<JSValue> in,
																				Array<BackgroundSizeCollection>& out, cchar* desc) {
	Local<JSArray> arr;
	if (in->IsString(worker)) {
		Local<JSValue> o = _parseBackgroundSizes.local()->Call(worker, 1, &in);
		if (o.IsEmpty()) {
			return false;
		}
		if ( !o->IsNull(worker)) {
			arr = o.To<JSArray>();
		}
	} else if ( isBackgroundSize(in) ) {
		auto s = BackgroundSize({
			(BackgroundSizeType)in.To()->Get(worker, worker->strs()->type())->ToUint32Value(worker),
			(float)in.To()->Get(worker, worker->strs()->value())->ToNumberValue(worker)
		});
		out.push({s,s});
		return true;
	}
	if (!arr.IsEmpty()) {
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			Local<JSArray> items = arr->Get(worker, i).To<JSArray>();
			BackgroundSizeCollection ss;
			for (int j = 0; j < 2; j++) {
				Local<JSObject> obj = items->Get(worker, i).To();
				auto s = BackgroundSize({
					(BackgroundSizeType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker),
					(float)obj->Get(worker, worker->strs()->value())->ToNumberValue(worker)
				});
				if (j) {
					ss.y = s;
				} else {
					ss.x = s;
				}
			}
			out.push(ss);
		}
		return true;
	}
	parse_error_throw(worker, in, desc, _parseBackgroundSizesHelp.local());
	return false;
}
bool ValueProgram::parseValues(Local<JSValue> in, Array<Value>& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out.push(Value{ ValueType::PIXEL, (float)in->ToNumberValue(worker) });
		return true;
	}
	Local<JSArray> arr;
	if (in->IsString(worker)) {
		Local<JSValue> o = _parseValues.local()->Call(worker, 1, &in);
		if (o.IsEmpty()) {
			return false;
		}
		if ( !o->IsNull(worker)) {
			arr = o.To<JSArray>();
		}
	} else if ( isValue(in) ) {
		out.push(Value({
			(ValueType)in.To()->Get(worker, worker->strs()->type())->ToUint32Value(worker),
			(float)in.To()->Get(worker, worker->strs()->value())->ToNumberValue(worker)
		}));
		return true;
	}
	if (!arr.IsEmpty()) {
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			Local<JSObject> obj = arr->Get(worker, i).To<JSObject>();
			out.push(Value({
				(ValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker),
				(float)obj->Get(worker, worker->strs()->value())->ToNumberValue(worker)
			}));
		}
		return true;
	}
	parse_error_throw(worker, in, desc, _parseValuesHelp.local());
	return false;
}
bool ValueProgram::parseFloats(Local<JSValue> in, Array<float>& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out.push(in->ToNumberValue(worker));
		return true;
	}
	Local<JSArray> arr;
	if (in->IsString(worker)) {
		Local<JSValue> o = _parseFloats.local()->Call(worker, 1, &in);
		if (o.IsEmpty()) {
			return false;
		}
		if ( !o->IsNull(worker) ) {
			arr = o.To<JSArray>();
		}
	}
	if (!arr.IsEmpty()) {
		for(int i = 0, len = arr->Length(worker); i < len; i++) {
			out.push( arr->Get(worker, i)->ToNumberValue(worker) );
		}
		return true;
	}
	parse_error_throw(worker, in, desc, _parseFloatsHelp.local());
	return false;
}
bool ValueProgram::parseAligns(Local<JSValue> in, Array<Align>& out, cchar* desc) {
	Local<JSObject> object;
	if ( in->IsString(worker) ) {
		auto strs = in->ToStringValue(worker).split(' ');
		for (auto& i : strs) {
			if (!i.value().is_blank()) {
				auto it = ALIGN.find(i.value());
				if ( it.is_null() ) { /* err */
					goto err;
				} else {
					out.push(it.value());
				}
			}
		}
		if (out.length()) {
			if (out.length() == 1) {
				out.push(out[0]);
			}
			return true;
		}
	} else if ( isAlign(in) ) {
		auto align = (Align)in.To()->Get(worker, worker->strs()->value())->ToUint32Value(worker);
		out = { align, align };
		return true;
	}
err:
	parse_error_throw(worker, in, desc, _parseAlignsHelp.local());
	return false;
}
bool ValueProgram::parseTextColor(Local<JSValue> in, TextColor& out, cchar* desc) {
	js_parse(TextColor, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value.r(object->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.value.g(object->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.value.b(object->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.value.a(object->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}
bool ValueProgram::parseTextSize(Local<JSValue> in, TextSize& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out.type = TextAttrType::VALUE;
		out.value = in->ToNumberValue(worker);
		return true;
	}
	js_parse(TextSize, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseTextFamily(Local<JSValue> in, TextFamily& out, cchar* desc) {
	js_parse(TextFamily, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		String fonts = object->Get(worker, worker->strs()->value())->ToStringValue(worker);
		out.value = FontPool::get_font_familys_id(fonts);
	});
}
bool ValueProgram::parseTextStyle(Local<JSValue> in, TextStyle& out, cchar* desc) {
	js_parse(TextStyle, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextStyleEnum)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseTextShadow(Local<JSValue> in, TextShadow& out, cchar* desc) {
	js_parse(TextShadow, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value.offset_x = object->Get(worker, worker->strs()->offsetX())->ToNumberValue(worker);
		out.value.offset_y = object->Get(worker, worker->strs()->offsetY())->ToNumberValue(worker);
		out.value.size = object->Get(worker, worker->strs()->size())->ToNumberValue(worker);
		out.value.color.r(object->Get(worker, worker->strs()->r())->ToUint32Value(worker));
		out.value.color.g(object->Get(worker, worker->strs()->g())->ToUint32Value(worker));
		out.value.color.b(object->Get(worker, worker->strs()->b())->ToUint32Value(worker));
		out.value.color.a(object->Get(worker, worker->strs()->a())->ToUint32Value(worker));
	});
}
bool ValueProgram::parseTextLineHeight(Local<JSValue> in,
																			 TextLineHeight& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out.type = TextAttrType::VALUE;
		out.value.height = in->ToNumberValue(worker);
		return true;
	}
	js_parse(TextLineHeight, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value.height = object->Get(worker, worker->strs()->height())->ToNumberValue(worker);
	});
}
bool ValueProgram::parseTextDecoration(Local<JSValue> in,
																			 TextDecoration& out, cchar* desc) {
	js_parse(TextDecoration, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextDecorationEnum)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseString(Local<JSValue> in, String& out, cchar* desc) {
	out = in->ToStringValue(worker);
	return true;
}
bool ValueProgram::parsebool(Local<JSValue> in, bool& out, cchar* desc) {
	out = in->ToBooleanValue(worker);
	return true;
}
bool ValueProgram::parsefloat(Local<JSValue> in, float& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out = in->ToNumberValue(worker);
		return true;
	}
	if (in->IsString(worker)) {
		if (in->ToStringValue(worker).to_float(&out)) {
			return true;
		}
	}
	parse_error_throw(worker, in, desc);
	return false;
}
bool ValueProgram::parseint(Local<JSValue> in, int& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out = in->ToInt32Value(worker);
		return true;
	}
	if (in->IsString(worker)) {
		if (in->ToStringValue(worker).to_int(&out)) {
			return true;
		}
	}
	parse_error_throw(worker, in, desc);
	return false;
}
bool ValueProgram::parseuint(Local<JSValue> in, uint& out, cchar* desc) {
	if (in->IsNumber(worker)) {
		out = in->ToUint32Value(worker);
		return true;
	}
	if (in->IsString(worker)) {
		if (in->ToStringValue(worker).to_uint(&out)) {
			return true;
		}
	}
	parse_error_throw(worker, in, desc);
	return false;
}
bool ValueProgram::parseTextOverflow(Local<JSValue> in, TextOverflow& out, cchar* desc) {
	js_parse(TextOverflow, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextOverflowEnum)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}
bool ValueProgram::parseTextWhiteSpace(Local<JSValue> in,
																			TextWhiteSpace& out, cchar* desc) {
	js_parse(TextWhiteSpace, {
		out.type = (TextAttrType)object->Get(worker, worker->strs()->type())->ToUint32Value(worker);
		out.value = (TextWhiteSpaceEnum)object->Get(worker, worker->strs()->value())->ToUint32Value(worker);
	});
}

// -------------------------------- is --------------------------------

bool ValueProgram::isBase(Local<JSValue> value) {
	return _isBase.local()->Call(worker, 1, &value)->ToBooleanValue(worker);
}
bool ValueProgram::isTextAlign(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextAlign.local());
}
bool ValueProgram::isAlign(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorAlign.local());
}
bool ValueProgram::isContentAlign(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorContentAlign.local());
}
bool ValueProgram::isRepeat(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorRepeat.local());
}
bool ValueProgram::isDirection(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorDirection.local());
}
bool ValueProgram::isKeyboardType(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorKeyboardType.local());
}
bool ValueProgram::isKeyboardReturnType(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorKeyboardReturnType.local());
}
bool ValueProgram::isBorder(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorBorder.local());
}
bool ValueProgram::isShadow(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorShadow.local());
}
bool ValueProgram::isColor(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorColor.local());
}
bool ValueProgram::isVec2(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorVec2.local());
}
bool ValueProgram::isVec3(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorVec3.local());
}
bool ValueProgram::isVec4(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorVec4.local());
}
bool ValueProgram::isCurve(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorCurve.local());
}
bool ValueProgram::isRect(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorRect.local());
}
bool ValueProgram::isMat(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorMat.local());
}
bool ValueProgram::isMat4(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorMat4.local());
}
bool ValueProgram::isValue(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorValue.local());
}
bool ValueProgram::isBackgroundPosition(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorBackgroundPosition.local());
}
bool ValueProgram::isBackgroundSize(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorBackgroundSize.local());
}
bool ValueProgram::isBackground(Local<JSValue> value) {
	return worker->has_instance<Background>(value);
}
bool ValueProgram::isTextColor(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextColor.local());
}
bool ValueProgram::isTextSize(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextSize.local());
}
bool ValueProgram::isTextFamily(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextFamily.local());
}
bool ValueProgram::isTextStyle(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextStyle.local());
}
bool ValueProgram::isTextShadow(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextShadow.local());
}
bool ValueProgram::isTextLineHeight(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextLineHeight.local());
}
bool ValueProgram::isTextDecoration(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextDecoration.local());
}
bool ValueProgram::isString(Local<JSValue> value) {
	return true;
}
bool ValueProgram::isbool(Local<JSValue> value) {
	return true;
}
bool ValueProgram::isfloat(Local<JSValue> value) {
	return value->IsNumber(worker);
}
bool ValueProgram::isint(Local<JSValue> value) {
	return value->IsInt32(worker);
}
bool ValueProgram::isuint(Local<JSValue> value) {
	return value->IsUint32(worker);
}
bool ValueProgram::isTextOverflow(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextOverflow.local());
}
bool ValueProgram::isTextWhiteSpace(Local<JSValue> value) {
	return value->InstanceOf(worker, _constructorTextWhiteSpace.local());
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
		exports->Set(worker, worker->New("_priv", 1), _prve);
		
		{
			TryCatch try_catch;

			if (worker->run_native_script(WeakBuffer((char*)
							native_js::EXT_native_js_code_value_,
							native_js::EXT_native_js_code_value_count_), "value.js", exports).IsEmpty()) {
				if ( try_catch.HasCaught() ) {
					worker->print_exception(&try_catch);
				}
				XX_FATAL("Could not initialize native/value.js");
			}
		}
		worker->m_inl->m_value_program = new ValueProgram(worker, exports, _prve);
	}
};

JS_REG_MODULE(_value, NativeValue);
JS_END
