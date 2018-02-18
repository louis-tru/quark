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

#include "ngui/js/str.h"
#include "ngui/js/js-1.h"
#include "ngui/js/ngui.h"
#include "ngui/font.h"
#include "ngui/draw.h"
#include "native-core-js.h"

/**
 * @ns ngui::js
 */

JS_BEGIN


ValueProgram::ValueProgram(Worker* worker,
                         Local<JSObject> exports,
                         Local<JSObject> native): worker(worker) {
#define NewS(s) worker->New(s,1).To<JSValue>()
  
#define js_init_func(Name, Type) \
  _constructor##Name .Reset(worker, exports->Get(worker,NewS(#Name)).To<JSFunction>()); \
  _parse##Name       .Reset(worker, exports->Get(worker,NewS("parse"#Name)).To<JSFunction>()); \
  _parse##Name##Description \
  .Reset(worker, native->Get(worker,NewS("_parse"#Name"Description")).To<JSFunction>()); \
  _##Name            .Reset(worker, native->Get(worker,NewS("_"#Name)).To<JSFunction>());
  
  js_value_table(js_init_func)
  _BorderRgba     .Reset(worker, native->Get(worker,NewS("_BorderRgba")).To<JSFunction>());
  _ShadowRgba     .Reset(worker, native->Get(worker,NewS("_ShadowRgba")).To<JSFunction>());
  _TextColorRgba  .Reset(worker, native->Get(worker,NewS("_TextColorRgba")).To<JSFunction>());
  _TextShadowRgba .Reset(worker, native->Get(worker,NewS("_TextShadowRgba")).To<JSFunction>());
  _isBase         .Reset(worker, native->Get(worker,NewS("_isBase")).To<JSFunction>());
  _parseValues    .Reset(worker, native->Get(worker,NewS("_parseValues")).To<JSFunction>());
  _parseFloatValues.Reset(worker,native->Get(worker,NewS("_parseFloatValues")).To<JSFunction>());
  
#undef NewS
#undef js_init_func
}

ValueProgram::~ValueProgram() {}

Local<JSValue> ValueProgram::New(const TextAlign& value) {
  Local<JSValue> arg = worker->New((uint)value);
  return _TextAlign.strong()->Call(worker, 1, &arg);
}
Local<JSValue> ValueProgram::New(const Align& value) {
  Local<JSValue> arg = worker->New((uint)value);
  return _Align.strong()->Call(worker, 1, &arg);
}
Local<JSValue> ValueProgram::New(const ContentAlign& value) {
  Local<JSValue> arg = worker->New((uint)value);
  return _ContentAlign.strong()->Call(worker, 1, &arg);
}
Local<JSValue> ValueProgram::New(const Repeat& value) {
  Local<JSValue> arg = worker->New((uint)value);
  return _Repeat.strong()->Call(worker, 1, &arg);
}
Local<JSValue> ValueProgram::New(const Direction& value) {
  Local<JSValue> arg = worker->New((uint)value);
  return _Direction.strong()->Call(worker, 1, &arg);
}
Local<JSValue> ValueProgram::New(const KeyboardType& value) {
  Local<JSValue> arg = worker->New((uint)value);
  return _KeyboardType.strong()->Call(worker, 1, &arg);
}
Local<JSValue> ValueProgram::New(const KeyboardReturnType& value) {
  Local<JSValue> arg = worker->New((uint)value);
  return _KeyboardReturnType.strong()->Call(worker, 1, &arg);
}
Local<JSValue> ValueProgram::New(const Border& value) {
  Array<Local<JSValue>> args(5);
  args[0] = worker->New(value.width);
  args[1] = worker->New(value.color.r());
  args[2] = worker->New(value.color.g());
  args[3] = worker->New(value.color.b());
  args[4] = worker->New(value.color.a());
  return _BorderRgba.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const Shadow& value) {
  Array<Local<JSValue>> args(7);
  args[0] = worker->New(value.offset_x);
  args[1] = worker->New(value.offset_y);
  args[2] = worker->New(value.size);
  args[3] = worker->New(value.color.b());
  args[4] = worker->New(value.color.g());
  args[5] = worker->New(value.color.b());
  args[6] = worker->New(value.color.a());
  return _ShadowRgba.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const Color& value) {
  Array<Local<JSValue>> args(4);
  args[0] = worker->New(value.r());
  args[1] = worker->New(value.g());
  args[2] = worker->New(value.b());
  args[3] = worker->New(value.a());
  return _Color.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const Vec2& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New(value.x());
  args[1] = worker->New(value.y());
  return _Vec2.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const Vec3& value) {
  Array<Local<JSValue>> args(3);
  args[0] = worker->New(value.x());
  args[1] = worker->New(value.y());
  args[2] = worker->New(value.z());
  return _Vec3.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const Vec4& value) {
  Array<Local<JSValue>> args(4);
  args[0] = worker->New(value.x());
  args[1] = worker->New(value.y());
  args[2] = worker->New(value.z());
  args[3] = worker->New(value.w());
  return _Vec4.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(cCurve& value) {
  Array<Local<JSValue>> args(4);
  args[0] = worker->New(value.point1().x());
  args[1] = worker->New(value.point1().y());
  args[2] = worker->New(value.point2().x());
  args[3] = worker->New(value.point2().y());
  return _Curve.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const CGRect& value) {
  Array<Local<JSValue>> args(4);
  args[0] = worker->New(value.origin.x());
  args[1] = worker->New(value.origin.y());
  args[2] = worker->New(value.size.width());
  args[3] = worker->New(value.size.height());
  return _Rect.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const Mat& value) {
  Local<JSArray> arr = worker->NewArray(6);
  arr->Set(worker, 0, worker->New( value[0] ));
  arr->Set(worker, 1, worker->New( value[1] ));
  arr->Set(worker, 2, worker->New( value[2] ));
  arr->Set(worker, 3, worker->New( value[3] ));
  arr->Set(worker, 4, worker->New( value[4] ));
  arr->Set(worker, 5, worker->New( value[5] ));
  return _Mat.strong()->Call(worker, 1, reinterpret_cast<Local<JSValue>*>(&arr));
}
Local<JSValue> ValueProgram::New(const Mat4& value) {
  Local<JSArray> arr = worker->NewArray(16);
  arr->Set(worker, 0, worker->New( value[0] ));
  arr->Set(worker, 1, worker->New( value[1] ));
  arr->Set(worker, 2, worker->New( value[2] ));
  arr->Set(worker, 3, worker->New( value[3] ));
  arr->Set(worker, 4, worker->New( value[4] ));
  arr->Set(worker, 5, worker->New( value[5] ));
  arr->Set(worker, 6, worker->New( value[6] ));
  arr->Set(worker, 7, worker->New( value[7] ));
  arr->Set(worker, 8, worker->New( value[8] ));
  arr->Set(worker, 9, worker->New( value[9] ));
  arr->Set(worker, 10, worker->New( value[10] ));
  arr->Set(worker, 11, worker->New( value[11] ));
  arr->Set(worker, 12, worker->New( value[12] ));
  arr->Set(worker, 13, worker->New( value[13] ));
  arr->Set(worker, 14, worker->New( value[14] ));
  arr->Set(worker, 15, worker->New( value[15] ));
  return _Mat4.strong()->Call(worker, 1, reinterpret_cast<Local<JSValue>*>(&arr));
}
Local<JSValue> ValueProgram::New(const Value& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New(value.value);
  return _Value.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextColor& value) {
  Array<Local<JSValue>> args(5);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New(value.value.r());
  args[2] = worker->New(value.value.g());
  args[3] = worker->New(value.value.b());
  args[4] = worker->New(value.value.a());
  return _TextColorRgba.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextSize& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New(value.value);
  return _TextSize.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextFamily& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New(value.name());
  return _TextFamily.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextStyle& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New((uint)value.value);
  return _TextStyle.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextShadow& value) {
  Array<Local<JSValue>> args(8);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New(value.value.offset_x);
  args[2] = worker->New(value.value.offset_y);
  args[3] = worker->New(value.value.size);
  args[4] = worker->New(value.value.color.r());
  args[5] = worker->New(value.value.color.g());
  args[6] = worker->New(value.value.color.b());
  args[7] = worker->New(value.value.color.a());
  return _TextShadowRgba.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextLineHeight& value) {
  Array<Local<JSValue>> args(3);
  args[0] = worker->New((uint)value.type);
  args[2] = worker->New(value.value.height);
  return _TextLineHeight.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextDecoration& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New((uint)value.value);
  return _TextDecoration.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const String& value) {
  return worker->New(value);
}
Local<JSValue> ValueProgram::New(const bool& value) {
  return worker->New(value);
}
Local<JSValue> ValueProgram::New(const TextOverflow& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New((uint)value.value);
  return _TextOverflow.strong()->Call(worker, args.length(), &args[0]);
}
Local<JSValue> ValueProgram::New(const TextWhiteSpace& value) {
  Array<Local<JSValue>> args(2);
  args[0] = worker->New((uint)value.type);
  args[1] = worker->New((uint)value.value);
  return _TextWhiteSpace.strong()->Call(worker, args.length(), &args[0]);
}

static void parse_error_throw(Worker* worker, Local<JSValue> value,
                              cchar* desc, Local<JSFunction> func) {
  String msg;
  
  if ( !func.IsEmpty() ) {
    Local<JSValue> o = func->Call(worker);
    
    if ( !o.IsEmpty() ) {
      msg = o->ToStringValue(worker);
    }
  }
  
  // Bad argument. Input.type = `Bad`
  // reference value, "
  
  String val = String::format(desc, *String::format("`%s`", *value->ToStringValue(worker)) );
  Local<JSValue> err;
  
  if ( msg.is_empty() ) {
    err = worker->NewTypeError("Bad argument. %s.", *val);
  } else {
    err = worker->NewTypeError("Bad argument. %s. Examples: %s", *val, *msg);
  }
  worker->throw_err(err);
}

void ValueProgram::throwError(Local<JSValue> value, cchar* msg, Local<JSFunction> more_msg) {
  parse_error_throw(worker, value, msg, more_msg);
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
  Local<JSValue> o = _parse##Type.strong()->Call(worker, 1, &in);\
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
  parse_error_throw(worker, in, desc, _parse##Type##Description.strong());\
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
  parse_error_throw(worker, in, desc, _parse##Type##Description.strong()); \
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
    parse_error_throw(worker, in, desc, _parseBorderDescription.strong());
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
bool ValueProgram::parseValues(Local<JSValue> in, Array<Value>& out, cchar* desc) {
  if (in->IsNumber(worker)) {
    out.push(Value{ ValueType::PIXEL, (float)in->ToNumberValue(worker) }); return true;
  }
  Local<JSArray> arr;
  if (in->IsString(worker)) {
    Local<JSValue> o = _parseValues.strong()->Call(worker, 1, &in);
    if ( o.IsEmpty() || o->IsNull(worker)) {
      return false;
    } else {
      arr = o.To<JSArray>();
    }
  } else if ( isValue(in) ) {
    out.push(Value({
      (ValueType)in.To<JSObject>()->Get(worker, worker->strs()->type())->ToUint32Value(worker),
      (float)in.To<JSObject>()->Get(worker, worker->strs()->value())->ToNumberValue(worker)
    }));
    return true;
  } else {
    return false;
  }
  
  for(int i = 0, len = arr->Length(worker); i < len; i++) {
    Local<JSObject> obj = arr->Get(worker, i).To<JSObject>();
    out.push(Value({
      (ValueType)obj->Get(worker, worker->strs()->type())->ToUint32Value(worker),
      (float)obj->Get(worker, worker->strs()->value())->ToNumberValue(worker)
    }));
  }
  return true;
}
bool ValueProgram::parseFloatValues(Local<JSValue> in, Array<float>& out, cchar* desc) {
  if (in->IsNumber(worker)) {
    out.push(in->ToNumberValue(worker)); return true;
  }
  Local<JSArray> arr;
  if (in->IsString(worker)) {
    
    Local<JSValue> o = _parseFloatValues.strong()->Call(worker, 1, &in);
    if ( o.IsEmpty() || o->IsNull(worker) ) {
      return false;
    } else {
      arr = o.To<JSArray>();
    }
  } else {
    return false;
  }
  for(int i = 0, len = arr->Length(worker); i < len; i++) {
    out.push( arr->Get(worker, i)->ToNumberValue(worker) );
  }
  return true;
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
  return 1;
}
bool ValueProgram::parsebool(Local<JSValue> in, bool& out, cchar* desc) {
  out = in->ToBooleanValue(worker);
  return 1;
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
// is
bool ValueProgram::isTextAlign(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextAlign.strong());
}
bool ValueProgram::isAlign(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorAlign.strong());
}
bool ValueProgram::isContentAlign(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorContentAlign.strong());
}
bool ValueProgram::isRepeat(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorRepeat.strong());
}
bool ValueProgram::isDirection(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorDirection.strong());
}
bool ValueProgram::isKeyboardType(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorKeyboardType.strong());
}
bool ValueProgram::isKeyboardReturnType(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorKeyboardReturnType.strong());
}
bool ValueProgram::isBorder(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorBorder.strong());
}
bool ValueProgram::isShadow(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorShadow.strong());
}
bool ValueProgram::isColor(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorColor.strong());
}
bool ValueProgram::isVec2(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorVec2.strong());
}
bool ValueProgram::isVec3(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorVec3.strong());
}
bool ValueProgram::isVec4(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorVec4.strong());
}
bool ValueProgram::isCurve(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorCurve.strong());
}
bool ValueProgram::isRect(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorRect.strong());
}
bool ValueProgram::isMat(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorMat.strong());
}
bool ValueProgram::isMat4(Local<JSValue> value) {
//  if ( ! value->IsObject(worker)) return false;
//  Local<JSValue> constructor = value.To<JSObject>()->Get(worker, worker->strs()->constructor());
//  return _constructorMat4.strong()->Equals(constructor);
  return value->InstanceOf(worker, _constructorMat4.strong());
}
bool ValueProgram::isValue(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorValue.strong());
}
bool ValueProgram::isTextColor(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextColor.strong());
}
bool ValueProgram::isTextSize(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextSize.strong());
}
bool ValueProgram::isTextFamily(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextFamily.strong());
}
bool ValueProgram::isTextStyle(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextStyle.strong());
}
bool ValueProgram::isTextShadow(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextShadow.strong());
}
bool ValueProgram::isTextLineHeight(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextLineHeight.strong());
}
bool ValueProgram::isTextDecoration(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextDecoration.strong());
}
bool ValueProgram::isBase(Local<JSValue> value) {
  return _isBase.strong()->Call(worker, 1, &value)->ToBooleanValue(worker);
}
bool ValueProgram::isString(Local<JSValue> value) {
  return true;
}
bool ValueProgram::isbool(Local<JSValue> value) {
  return true;
}
bool ValueProgram::isTextOverflow(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextOverflow.strong());
}
bool ValueProgram::isTextWhiteSpace(Local<JSValue> value) {
  return value->InstanceOf(worker, _constructorTextWhiteSpace.strong());
}

/**
 * @class NativeValue
 */
class NativeValue {
 public:
  static void binding(Local<JSObject> exports, Worker* worker) {
    Local<JSObject> _native = worker->NewObject();
    exports->Set(worker, worker->strs()->_native(), _native);
    
    {
      TryCatch try_catch;
      
      if (!worker->run_native_script(exports, WeakBuffer((char*)
                                     native_js::CORE_native_js_code_value_,
                                     native_js::CORE_native_js_code_value_count_), "value.js")) {
        if ( try_catch.HasCaught() ) {
          worker->report_exception(&try_catch);
        }
        XX_FATAL("Could not initialize gui-value.js");
      }
    }
    worker->m_value_program = new ValueProgram(worker, exports, _native);
  }
};

JS_REG_MODULE(ngui_value, NativeValue);
JS_END
