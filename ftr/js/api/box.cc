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

#include "../_js.h"
#include "./_view.h"
#include "../../views2/box.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

/**
 * @class WrapBox
 */
class WrapBox: public WrapObject {
	public:

	static void constructor(FunctionCall args) {
		JS_WORKER(args);
		JS_THROW_ERR("Forbidden access abstract");
	}
	
	/**
	 * @get width {Value}
	 */
	static void width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->width()) );
	}

	/**
	 * @get width {Value}
	 */
	static void height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->height()) );
	}

	/**
	 * @get margin_left {Value}
	 */
	static void margin_left(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->margin_left()) );
	}

	/**
	 * @get margin_top {Value}
	 */
	static void margin_top(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->margin_top()) );
	}

	/**
	 * @get margin_right {Value}
	 */
	static void margin_right(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->margin_right()) );
	}

	/**
	 * @get margin_bottom {Value}
	 */
	static void margin_bottom(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->margin_bottom()) );
	}

	/**
	 * @get border_left {Border}
	 */
	static void border_left(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_left()) );
	}

	/**
	 * @get border_top {Border}
	 */
	static void border_top(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_top()) );
	}

	/**
	 * @get border_right {Border}
	 */
	static void border_right(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_right()) );
	}

	/**
	 * @get border_bottom {Border}
	 */
	static void border_bottom(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_bottom()) );
	}

	/**
	 * @get border_left_width {float}
	 */
	static void border_left_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_left().width );
	}

	/**
	 * @get border_top_width {float}
	 */
	static void border_top_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_top().width );
	}

	/**
	 * @get border_right_width {float}
	 */
	static void border_right_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_right().width );
	}

	/**
	 * @get border_bottom_width {float}
	 */
	static void border_bottom_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_bottom().width );
	}

	/**
	 * @get border_left_color {Color}
	 */
	static void border_left_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_left().color) );
	}

	/**
	 * @get border_top_color {Color}
	 */
	static void border_top_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_top().color) );
	}

	/**
	 * @get border_right_color {Color}
	 */
	static void border_right_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_right().color) );
	}

	/**
	 * @get border_bottom_color {Color}
	 */
	static void border_bottom_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->border_bottom().color) );
	}

	/**
	 * @get border_radius_left_top {float}
	 */
	static void border_radius_left_top(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_radius_left_top() );
	}

	/**
	 * @get border_radius_right_top {float}
	 */
	static void border_radius_right_top(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_radius_left_top() );
	}

	/**
	 * @get border_radius_right_bottom {float}
	 */
	static void border_radius_right_bottom(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_radius_right_bottom() );
	}

	/**
	 * @get border_radius_left_bottom {float}
	 */
	static void border_radius_left_bottom(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->border_radius_left_bottom() );
	}

	/**
	 * @get newline {bool}
	 */
	static void newline(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->newline() );
	}

	/**
	 * @get clip {bool}
	 */
	static void clip(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( self->clip() );
	}

	static void margin(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->margin_top()) );
		arr->Set(worker, 1, worker->values()->New(self->margin_right()) );
		arr->Set(worker, 2, worker->values()->New(self->margin_bottom()) );
		arr->Set(worker, 3, worker->values()->New(self->margin_left()) );
		JS_RETURN( arr );
	}

	static void border(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->border_top()) );
		arr->Set(worker, 1, worker->values()->New(self->border_right()) );
		arr->Set(worker, 2, worker->values()->New(self->border_bottom()) );
		arr->Set(worker, 3, worker->values()->New(self->border_left()) );
		JS_RETURN( arr );
	}

	static void border_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->New(self->border_top_width()) );
		arr->Set(worker, 1, worker->New(self->border_right_width()) );
		arr->Set(worker, 2, worker->New(self->border_bottom_width()) );
		arr->Set(worker, 3, worker->New(self->border_left_width()) );
		JS_RETURN( arr );
	}

	static void border_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->border_top_color()) );
		arr->Set(worker, 1, worker->values()->New(self->border_right_color()) );
		arr->Set(worker, 2, worker->values()->New(self->border_bottom_color()) );
		arr->Set(worker, 3, worker->values()->New(self->border_left_color()) );
		JS_RETURN( arr );
	}

	static void border_radius(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->New(self->border_radius_left_top()) );
		arr->Set(worker, 1, worker->New(self->border_radius_right_top()) );
		arr->Set(worker, 2, worker->New(self->border_radius_right_bottom()) );
		arr->Set(worker, 3, worker->New(self->border_radius_left_bottom()) );
		JS_RETURN( arr );
	}

	/**
	 * @get final_width {float}
	 */
	static void final_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		JS_RETURN( self->final_width() );
	}

	/**
	 * @get final_height {float}
	 */
	static void final_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		JS_RETURN( self->final_height() );
	}

	/**
	 * @get final_margin_left {float}
	 */
	static void final_margin_left(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		JS_RETURN( self->final_margin_left() );
	}

	/**
	 * @get final_margin_top {float}
	 */
	static void final_margin_top(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		JS_RETURN( self->final_margin_top() );
	}

	/**
	 * @get final_margin_right {float}
	 */
	static void final_margin_right(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		JS_RETURN( self->final_margin_right() );
	}

	/**
	 * @get final_margin_bottom {float}
	 */
	static void final_margin_bottom(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); // GUILock lock;
		JS_SELF(Box);
		JS_RETURN( self->final_margin_bottom() );
	}

	/**
	 * @set width {Value}
	 */
	static void set_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Box.width = %s");
		JS_SELF(Box);
		self->set_width(out);
	}

	/**
	 * @set height {Value}
	 */
	static void set_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Box.height = %s");
		JS_SELF(Box);
		self->set_height(out);
	}
	
	/**
	 * @set margin {Value}
	 */
	static void set_margin(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value2(Array<Value>, Values, value, "Box.margin = %s");
		JS_SELF(Box);
		switch(out.length()) {
			case 1:
				self->set_margin_left(out[0]);
				self->set_margin_top(out[0]);
				self->set_margin_right(out[0]);
				self->set_margin_bottom(out[0]);
				break;
			case 2:
				self->set_margin_top(out[0]);
				self->set_margin_bottom(out[0]);
				self->set_margin_left(out[1]);
				self->set_margin_right(out[1]);
				break;
			case 3:
				self->set_margin_top(out[0]);
				self->set_margin_left(out[1]);
				self->set_margin_right(out[1]);
				self->set_margin_bottom(out[2]);
				break;
			default: // 4
				self->set_margin_top(out[0]);
				self->set_margin_right(out[1]);
				self->set_margin_bottom(out[2]);
				self->set_margin_left(out[3]);
				break;
		}
	}

	/**
	 * @set margin_left {Value}
	 */
	static void set_margin_left(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Box.marginLeft = %s");
		JS_SELF(Box);
		self->set_margin_left(out);
	}

	/**
	 * @set margin_top {Value}
	 */
	static void set_margin_top(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Box.marginTop = %s");
		JS_SELF(Box);
		self->set_margin_top(out);
	}

	/**
	 * @set margin_right {Value}
	 */
	static void set_margin_right(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Box.marginRight = %s");
		JS_SELF(Box);
		self->set_margin_right(out);
	}

	/**
	 * @set margin_bottom {Value}
	 */
	static void set_margin_bottom(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Value, value, "Box.marginBottom = %s");
		JS_SELF(Box);
		self->set_margin_bottom(out);
	}

	/**
	 * @set border {Border}
	 */
	static void set_border(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Border, value, "Box.border = %s");
		JS_SELF(Box);
		self->set_border(out);
	}

	/**
	 * @set border_left {Border}
	 */
	static void set_border_left(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Border, value, "Box.borderLeft = %s");
		JS_SELF(Box);
		self->set_border_left(out);
	}

	/**
	 * @set border_top {Border}
	 */
	static void set_border_top(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Border, value, "Box.borderTop = %s");
		JS_SELF(Box);
		self->set_border_top(out);
	}

	/**
	 * @set border_right {Border}
	 */
	static void set_border_right(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Border, value, "Box.borderRight = %s");
		JS_SELF(Box);
		self->set_border_right(out);
	}

	/**
	 * @set border_bottom {Border}
	 */
	static void set_border_bottom(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Border, value, "Box.borderBottom = %s");
		JS_SELF(Box);
		self->set_border_bottom(out);
	}

	/**
	 * @set border_width {float}
	 */
	static void set_border_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) {
			JS_THROW_ERR("* @set borderWidth {float}");
		}
		JS_SELF(Box);
		self->set_border_width( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_left_width {float}
	 */
	static void set_border_left_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) {
			JS_THROW_ERR("* @set borderLeftWidth {float}");
		}
		JS_SELF(Box);
		self->set_border_left_width( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_top_width {float}
	 */
	static void set_border_top_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) {
			JS_THROW_ERR("* @set borderTopWidth {float}");
		}
		JS_SELF(Box);
		self->set_border_top_width( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_right_width {float}
	 */
	static void set_border_right_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) {
			JS_THROW_ERR("* @set borderRightWidth {float}");
		}
		JS_SELF(Box);
		self->set_border_right_width( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_bottom_width {float}
	 */
	static void set_border_bottom_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) {
			JS_THROW_ERR("* @set borderBottomWidth {float}");
		}
		JS_SELF(Box);
		self->set_border_bottom_width( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_color {Color}
	 */
	static void set_border_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Color, value, "Box.borderColor = %s");
		JS_SELF(Box);
		self->set_border_color(out);
	}

	/**
	 * @set border_left_color {Color}
	 */
	static void set_border_left_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Color, value, "Box.borderLeftColor = %s");
		JS_SELF(Box);
		self->set_border_left_color(out);
	}

	/**
	 * @set border_top_color {Color}
	 */
	static void set_border_top_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Color, value, "Box.borderTopColor = %s");
		JS_SELF(Box);
		self->set_border_top_color(out);
	}

	/**
	 * @set border_right_color {Color}
	 */
	static void set_border_right_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Color, value, "Box.borderRightColor = %s");
		JS_SELF(Box);
		self->set_border_right_color(out);
	}

	/**
	 * @set border_bottom_color {Color}
	 */
	static void set_border_bottom_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Color, value, "Box.borderBottomColor = %s");
		JS_SELF(Box);
		self->set_border_bottom_color(out);
	}

	/**
	 * @set border_radius {float}
	 */
	static void set_border_radius(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) JS_THROW_ERR("* @set borderRadius {float}");
		JS_SELF(Box);
		self->set_border_radius( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_radius_left_top {float}
	 */
	static void set_border_radius_left_top(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) JS_THROW_ERR("* @set borderRadiusLeftTop {float}");
		JS_SELF(Box);
		self->set_border_radius_left_top( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_radius_right_top {float}
	 */
	static void set_border_radius_right_top(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) JS_THROW_ERR("* @set borderRadiusRightTop {float}");
		JS_SELF(Box);
		self->set_border_radius_right_top( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_radius_right_bottom {float}
	 */
	static void set_border_radius_right_bottom(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) JS_THROW_ERR("* @set borderRadiusRightBottom {float}");
		JS_SELF(Box);
		self->set_border_radius_right_bottom( value->ToNumberValue(worker) );
	}

	/**
	 * @set border_radius_left_bottom {float}
	 */
	static void set_border_radius_left_bottom(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker)) JS_THROW_ERR("* @set borderRadiusLeftBottom {float}");
		JS_SELF(Box);
		self->set_border_radius_left_bottom( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set newline {bool}
	 */
	static void set_newline(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Box);
		self->set_newline( value->ToBooleanValue(worker) );
	}
	
	/**
	 * @set newline {bool}
	 */
	static void set_clip(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Box);
		self->set_clip( value->ToBooleanValue(worker) );
	}

	/**
	 * @get background_color {Color}
	 */
	static void background_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		JS_RETURN( worker->values()->New(self->background_color()) );
	}
	
	/**
	 * @set background_color {Color}
	 */
	static void set_background_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Color, value, "Box.backgroundColor = %s");
		JS_SELF(Box);
		self->set_background_color(out);
	}

	/**
	 * @get background {BackgroundPtr}
	 */
	static void background(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Box);
		auto bg = self->background();
		if (bg) {
			JS_RETURN( pack(bg)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}

	/**
	 * @set background {BackgroundPtr}
	 */
	static void set_background(Local<JSString> name,
														 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value2(BackgroundPtr, Background, value, "Box.background = %s");
		JS_SELF(Box);
		self->set_background(out);
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS_NO_EXPORTS(Box, constructor, {
			JS_SET_CLASS_ACCESSOR(width, width, set_width);
			JS_SET_CLASS_ACCESSOR(height, height, set_height);
			JS_SET_CLASS_ACCESSOR(margin, margin, set_margin);
			JS_SET_CLASS_ACCESSOR(marginLeft, margin_left, set_margin_left);
			JS_SET_CLASS_ACCESSOR(marginTop, margin_top, set_margin_top);
			JS_SET_CLASS_ACCESSOR(marginRight, margin_right, set_margin_right);
			JS_SET_CLASS_ACCESSOR(marginBottom, margin_bottom, set_margin_bottom);
			JS_SET_CLASS_ACCESSOR(border, border, set_border);
			JS_SET_CLASS_ACCESSOR(borderLeft, border_left, set_border_left);
			JS_SET_CLASS_ACCESSOR(borderTop, border_top, set_border_top);
			JS_SET_CLASS_ACCESSOR(borderRight, border_right, set_border_right);
			JS_SET_CLASS_ACCESSOR(borderBottom, border_bottom, set_border_bottom);
			JS_SET_CLASS_ACCESSOR(borderWidth, border_width, set_border_width);
			JS_SET_CLASS_ACCESSOR(borderLeftWidth, border_left_width, set_border_left_width);
			JS_SET_CLASS_ACCESSOR(borderTopWidth, border_top_width, set_border_top_width);
			JS_SET_CLASS_ACCESSOR(borderRightWidth, border_right_width, set_border_right_width);
			JS_SET_CLASS_ACCESSOR(borderBottomWidth, border_bottom_width, set_border_bottom_width);
			JS_SET_CLASS_ACCESSOR(borderColor, border_color, set_border_color);
			JS_SET_CLASS_ACCESSOR(borderLeftColor, border_left_color, set_border_left_color);
			JS_SET_CLASS_ACCESSOR(borderTopColor, border_top_color, set_border_top_color);
			JS_SET_CLASS_ACCESSOR(borderRightColor, border_right_color, set_border_right_color);
			JS_SET_CLASS_ACCESSOR(borderBottomColor, border_bottom_color, set_border_bottom_color);
			JS_SET_CLASS_ACCESSOR(borderRadius, border_radius, set_border_radius);
			JS_SET_CLASS_ACCESSOR(borderRadiusLeftTop, border_radius_left_top, set_border_radius_left_top);
			JS_SET_CLASS_ACCESSOR(borderRadiusRightTop, border_radius_right_top, set_border_radius_right_top);
			JS_SET_CLASS_ACCESSOR(borderRadiusRightBottom, border_radius_right_bottom, set_border_radius_right_bottom);
			JS_SET_CLASS_ACCESSOR(borderRadiusLeftBottom, border_radius_left_bottom, set_border_radius_left_bottom);
			JS_SET_CLASS_ACCESSOR(backgroundColor, background_color, set_background_color); // background color
			JS_SET_CLASS_ACCESSOR(background, background, set_background); // background
			JS_SET_CLASS_ACCESSOR(newline, newline, set_newline);
			JS_SET_CLASS_ACCESSOR(clip, clip, set_clip);
			JS_SET_CLASS_ACCESSOR(finalWidth, final_width);
			JS_SET_CLASS_ACCESSOR(finalHeight, final_height);
			JS_SET_CLASS_ACCESSOR(finalMarginLeft, final_margin_left);
			JS_SET_CLASS_ACCESSOR(finalMarginTop, final_margin_top);
			JS_SET_CLASS_ACCESSOR(finalMarginRight, final_margin_right);
			JS_SET_CLASS_ACCESSOR(finalMarginBottom, final_margin_bottom);
		}, Layout);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Box), View::BOX);
	}
};

void binding_box(Local<JSObject> exports, Worker* worker) {
	WrapBox::binding(exports, worker);
}

JS_END
