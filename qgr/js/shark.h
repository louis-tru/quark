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

#ifndef __qgr__js__qgr__
#define __qgr__js__qgr__

#include "js.h"
#include "qgr/app.h"
#include "qgr/value.h"
#include "qgr/view.h"
#include "qgr/bezier.h"
#include "qgr/background.h"

/**
 * @ns qgr::js
 */

JS_BEGIN

using namespace qgr;
using namespace qgr::value;

#define js_check_gui_app() if ( ! app() ) { \
	JS_WORKER(args); JS_THROW_ERR("Need to create a `new GUIApplication()`"); }

#define js_parse_value(Type, value, desc) js_parse_value2(Type, Type, value, desc)
#define js_parse_value2(Type, Name, value, desc) \
	Type out; \
	if ( !worker->value_program()->parse##Name(value, out, desc)) \
	{ return; /*JS_THROW_ERR("Bad argument.");*/ }

#define js_throw_value_err(value, msg, ...)\
	worker->value_program()->throwError(t, msg, ##__VA_ARGS__)

// ------------- values -------------

#define js_value_table(F) \
F(String, String) \
F(bool, bool) \
F(float, float) \
F(int, int) \
F(uint, uint) \
F(TextAlign, TextAlign) \
F(Align, Align) \
F(ContentAlign, ContentAlign) \
F(Border, Border) \
F(Shadow, Shadow) \
F(Color, Color) \
F(Vec2, Vec2) \
F(Vec3, Vec3) \
F(Vec4, Vec4) \
F(Rect, CGRect) \
F(Mat, Mat) \
F(Mat4, Mat4) \
F(Value, Value) \
F(TextColor, TextColor) \
F(TextSize, TextSize)  \
F(TextFamily, TextFamily) \
F(TextStyle, TextStyle) \
F(TextShadow, TextShadow)  \
F(TextLineHeight, TextLineHeight) \
F(TextDecoration, TextDecoration) \
F(Repeat, Repeat) \
F(Curve, Curve) \
F(Direction, Direction) \
F(TextOverflow, TextOverflow) \
F(TextWhiteSpace, TextWhiteSpace) \
F(KeyboardType, KeyboardType) \
F(KeyboardReturnType, KeyboardReturnType) \
F(Background, BackgroundPtr) \
F(BackgroundPosition, BackgroundPosition) \
F(BackgroundSize, BackgroundSize) \
/* Append, no actual type */\
F(Values, Array<Value>) \
F(Floats, Array<float>) \
F(Aligns, Array<Align>) \
F(Repeats, Array<Repeat>) \
F(BackgroundPositions, Array<BackgroundPositionCollection>) \
F(BackgroundSizes, Array<BackgroundSizeCollection>) \

/**
 * @class ValueProgram
 */
class XX_EXPORT ValueProgram: public Object {
 public:
	#define def_attr_fn(Name, Type)           \
		Local<JSValue> New(const Type& value);  \
		bool parse##Name(Local<JSValue> in, Type& out, cchar* err_msg); \
		bool is##Name(Local<JSValue> value);
	
	#define def_attr(Name, Type) \
		Persistent<JSFunction> _constructor##Name; \
		Persistent<JSFunction> _parse##Name; \
		Persistent<JSFunction> _parse##Name##Help; \
		Persistent<JSFunction> _##Name;
	
	ValueProgram(Worker* worker, Local<JSObject> exports, Local<JSObject> native);
	virtual ~ValueProgram();
	void throwError(Local<JSValue> value, cchar* msg, Local<JSFunction> help = Local<JSFunction>());
	bool isBase(Local<JSValue> value);
	bool parseBackgroundImage(Local<JSValue> in, BackgroundPtr& out, cchar* err_msg);
	js_value_table(def_attr_fn);
 private:
	js_value_table(def_attr)
	Worker* worker;
	Persistent<JSFunction> _isBase;
	#undef def_attr_fn
	#undef def_attr
};

/**
 * @class WrapViewBase
 */
class XX_EXPORT WrapViewBase: public WrapObject {
 public:

	/**
	 * @func overwrite
	 */
	virtual void destroy();
	
	/**
	 * @func overwrite
	 */
	virtual bool add_event_listener(cString& name, cString& func, int id);
	
	/**
	 * @func overwrite
	 */
	virtual bool remove_event_listener(cString& name, int id);
	
	/**
	 * @func inherit_text_font
	 */
	static void inherit_text_font(Local<JSClass> cls, Worker* worker);
	
	/**
	 * @func inherit_text_layout
	 */
	static void inherit_text_layout(Local<JSClass> cls, Worker* worker);
	
	/**
	 * @func inherit_scroll
	 */
	static void inherit_scroll(Local<JSClass> cls, Worker* worker);
	
};

JS_END
#endif
