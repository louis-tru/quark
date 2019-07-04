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

#include "langou/js/js-1.h"
#include "langou/js/langou.h"
#include "langou/scroll.h"

/**
 * @ns langou::js
 */

JS_BEGIN

#define js_scroll_self() \
BasicScroll* self = Wrap<View>::unpack(args.This())->self()->as_basic_scroll()

class WrapBasicScroll {
 public:

	/**
	 * @func scrollTo(scroll[,duration[,curve]])
	 * @arg scroll {Vec2}
	 * @arg [duration] {uint} ms
	 * @arg [curve] {Curve}
	 */
	static void scroll_to(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		int64 duration = 0;
		
		if ( args.Length() == 0 ) {
			JS_THROW_ERR(
				"* @func scrollTo(scroll[,duration[,curve]])\n"
				"* @arg scroll {Vec2}\n"
				"* @arg [duration] {uint} ms\n"
				"* @arg [curve] {Curve}\n"
			);
		}
		
		js_parse_value(Vec2, args[0], "BasicScroll.scrollTo(%s)");
		
		Vec2 scroll = out;
		
		js_scroll_self();
		
		if ( args.Length() > 1 && args[1]->IsNumber(worker) ) {
			duration = args[1]->ToNumberValue(worker) / 1000;
			duration = XX_MAX(duration, 0);
		}
		
		if ( args.Length() > 2 ) {
			js_parse_value(Curve, args[2], "BasicScroll.scrollTo(vec2, %s)");
			self->scroll_to(scroll, duration, out);
			return;
		}
		
		self->scroll_to(scroll, duration);
	}
	
	static void scroll(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		JS_RETURN( worker->values()->New(self->scroll()) );
	}
	
	static void set_scroll(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Vec2, value, "BasicScroll.scroll = %s");
		js_scroll_self();
		self->set_scroll(out);
	}
	
	static void scroll_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		JS_RETURN( self->scroll_x() );
	}
	
	static void scroll_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		JS_RETURN( self->scroll_y() );
	}
	
	/**
	 * @set scroll_x {float} 
	 */
	static void set_scroll_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set scrollX {float} ");
		js_scroll_self();
		self->set_scroll_x(value->ToNumberValue(worker));
	}
	
	/**
	 * @set scroll_y {float} 
	 */
	static void set_scroll_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set scrollY {float} ");
		js_scroll_self();
		self->set_scroll_y(value->ToNumberValue(worker));
	}
	
	static void scroll_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		JS_RETURN( self->scroll_width() );
	}
	
	static void scroll_height(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		JS_RETURN( self->scroll_height() );
	}
	
	static void scrollbar(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->scrollbar() );
	}
	
	static void set_scrollbar(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		self->set_scrollbar(value->ToBooleanValue(worker));
	}
	
	static void resistance(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->resistance() );
	}
	
	/**
	 * @set resistance {float} 
	 */
	static void set_resistance(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set resistance {float} ");
		js_scroll_self();
		self->set_resistance(value->ToNumberValue(worker));
	}
	
	static void bounce(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->bounce() );
	}
	
	static void set_bounce(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		self->set_bounce(value->ToBooleanValue(worker));
	}
	
	static void bounce_lock(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->bounce_lock() );
	}
	
	static void set_bounce_lock(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		self->set_bounce_lock(value->ToBooleanValue(worker));
	}
	
	static void momentum(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->momentum() );
	}
	
	static void set_momentum(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		self->set_momentum(value->ToBooleanValue(worker));
	}
	
	static void lock_direction(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->lock_direction() );
	}
	
	static void set_lock_direction(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		self->set_lock_direction(value->ToBooleanValue(worker));
	}
	
	static void catch_position_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->catch_position_x() );
	}
	
	static void catch_position_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->catch_position_y() );
	}
	
	/**
	 * @set catch_position_x {float} 
	 */
	static void set_catch_position_x(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set catchPositionX {float} ");
		js_scroll_self();
		self->set_catch_position_x(value->ToNumberValue(worker));
	}
	
	/**
	 * @set catch_position_y {float} 
	 */
	static void set_catch_position_y(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set catchPositionY {float} ");
		js_scroll_self();
		self->set_catch_position_y(value->ToNumberValue(worker));
	}
	
	static void scrollbar_color(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( worker->values()->New(self->scrollbar_color()) );
	}
	
	static void set_scrollbar_color(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Color, value, "BasicScroll.scrollbarColor = %s");
		js_scroll_self();
		self->set_scrollbar_color(out);
	}
	
	static void h_scrollbar(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		JS_RETURN( self->h_scrollbar() );
	}
	
	static void v_scrollbar(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		JS_RETURN( self->v_scrollbar() );
	}
	
	static void scrollbar_width(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->scrollbar_width() );
	}
	
	/**
	 * @set scrollbar_width {float} 
	 */
	static void set_scrollbar_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set scrollbarWidth {float} ");
		js_scroll_self();
		self->set_scrollbar_width( value->ToNumberValue(worker) );
	}
	
	static void scrollbar_margin(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->scrollbar_margin() );
	}
	
	/**
	 * @set scrollbar_margin {float} 
	 */
	static void set_scrollbar_margin(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set scrollbarMargin {float} ");
		js_scroll_self();
		self->set_scrollbar_margin( value->ToNumberValue(worker) );
	}
	
	static void default_scroll_duration(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( self->default_scroll_duration() / 1000 );
	}
	
	/**
	 * @set default_scroll_duration {uint} ms
	 */
	static void set_default_scroll_duration(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( ! value->IsNumber(worker) ) JS_THROW_ERR("* @set defaultScrollDuration {uint} ms");
		js_scroll_self();
		self->set_default_scroll_duration( XX_MAX(value->ToNumberValue(worker), 0) );
	}
	
	static void default_scroll_curve(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		js_scroll_self();
		JS_RETURN( worker->values()->New(self->default_scroll_curve()) );
	}
	
	static void set_default_scroll_curve(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Curve, value, "BasicScroll.defaultScrollCurve = %s");
		js_scroll_self();
		self->set_default_scroll_curve(out);
	}
	
	static void terminate(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		js_scroll_self();
		self->terminate();
	}

	static void inherit(Local<JSClass> cls, Worker* worker) {
		JS_SET_CLASS_METHOD(scrollTo, scroll_to);
		JS_SET_CLASS_METHOD(terminate, terminate);
		JS_SET_CLASS_ACCESSOR(scroll, scroll, set_scroll);
		JS_SET_CLASS_ACCESSOR(scrollX, scroll_x, set_scroll_x);
		JS_SET_CLASS_ACCESSOR(scrollY, scroll_y, set_scroll_y);
		JS_SET_CLASS_ACCESSOR(scrollWidth, scroll_width);
		JS_SET_CLASS_ACCESSOR(scrollHeight, scroll_height);
		JS_SET_CLASS_ACCESSOR(scrollbar, scrollbar, set_scrollbar);
		JS_SET_CLASS_ACCESSOR(resistance, resistance, set_resistance);
		JS_SET_CLASS_ACCESSOR(bounce, bounce, set_bounce);
		JS_SET_CLASS_ACCESSOR(bounceLock, bounce_lock, set_bounce_lock);
		JS_SET_CLASS_ACCESSOR(momentum, momentum, set_momentum);
		JS_SET_CLASS_ACCESSOR(lockDirection, lock_direction, set_lock_direction);
		JS_SET_CLASS_ACCESSOR(catchPositionX, catch_position_x, set_catch_position_x);
		JS_SET_CLASS_ACCESSOR(catchPositionY, catch_position_y, set_catch_position_y);
		JS_SET_CLASS_ACCESSOR(scrollbarColor, scrollbar_color, set_scrollbar_color);
		JS_SET_CLASS_ACCESSOR(hScrollbar, h_scrollbar);
		JS_SET_CLASS_ACCESSOR(vScrollbar, v_scrollbar);
		JS_SET_CLASS_ACCESSOR(scrollbarWidth, scrollbar_width, set_scrollbar_width);
		JS_SET_CLASS_ACCESSOR(scrollbarMargin, scrollbar_margin, set_scrollbar_margin);
		JS_SET_CLASS_ACCESSOR(defaultScrollDuration, default_scroll_duration, set_default_scroll_duration);
		JS_SET_CLASS_ACCESSOR(defaultScrollCurve, default_scroll_curve, set_default_scroll_curve);
	}
};

class WrapScroll: public WrapViewBase {
 public:

	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_CHECK_APP();
		New<WrapScroll>(args, new Scroll());
	}
	
	static void focus_margin_left(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( self->focus_margin_left() );
	}
	
	static void focus_margin_right(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( self->focus_margin_right() );
	}
	
	static void focus_margin_top(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( self->focus_margin_top() );
	}
	
	static void focus_margin_bottom(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( self->focus_margin_bottom() );
	}
	
	static void focus_align_x(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( worker->values()->New( self->focus_align_x() ) );
	}
	
	static void focus_align_y(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( worker->values()->New( self->focus_align_y() ) );
	}
	
	/**
	 * @set focus_margin_left {float}
	 */
	static void set_focus_margin_left(Local<JSString> name,
																		 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( !value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(Scroll);
		self->set_focus_margin_left( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_margin_right {float}
	 */
	static void set_focus_margin_right(Local<JSString> name,
																			Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( !value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(Scroll);
		self->set_focus_margin_right( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_margin_top {float}
	 */
	static void set_focus_margin_top(Local<JSString> name,
																		Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( !value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(Scroll);
		self->set_focus_margin_top( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_margin_bottom {float}
	 */
	static void set_focus_margin_bottom(Local<JSString> name,
																			 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		if ( !value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(Scroll);
		self->set_focus_margin_bottom( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_align_x {float}
	 */
	static void set_focus_align_x(Local<JSString> name,
																 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Align, value, "BasicScroll.selectMotionAlignX = %s");
		JS_SELF(Scroll);
		self->set_focus_align_x(out);
	}
	
	static void set_focus_align_y(Local<JSString> name,
																 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Align, value, "BasicScroll.selectMotionAlignY = %s");
		JS_SELF(Scroll);
		self->set_focus_align_y(out);
	}
	
	static void enable_focus_align(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( self->enable_focus_align() );
	}
	
	static void set_enable_focus_align(Local<JSString> name,
																		 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		JS_SELF(Scroll);
		self->set_enable_focus_align( value->ToBooleanValue(worker) );
	}
	
	static void enable_fixed_scroll_size(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Scroll);
		JS_RETURN( self->enable_fixed_scroll_size() );
	}
	
	static void set_enable_fixed_scroll_size(Local<JSString> name,
																					 Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); GUILock lock;
		js_parse_value(Vec2, value, "BasicScroll.enableFixedScrollSize = %s");
		JS_SELF(Scroll);
		self->set_enable_fixed_scroll_size( out );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Scroll, constructor, {
			JS_SET_CLASS_ACCESSOR(focusMarginLeft, focus_margin_left, set_focus_margin_left);
			JS_SET_CLASS_ACCESSOR(focusMarginRight, focus_margin_right, set_focus_margin_right);
			JS_SET_CLASS_ACCESSOR(focusMarginTop, focus_margin_top, set_focus_margin_top);
			JS_SET_CLASS_ACCESSOR(focusMarginBottom, focus_margin_bottom, set_focus_margin_bottom);
			JS_SET_CLASS_ACCESSOR(focusAlignX, focus_align_x, set_focus_align_x);
			JS_SET_CLASS_ACCESSOR(focusAlignY, focus_align_y, set_focus_align_y);
			JS_SET_CLASS_ACCESSOR(enableFocusAlign, enable_focus_align, set_enable_focus_align);
			JS_SET_CLASS_ACCESSOR(enableFixedScrollSize,
													enable_fixed_scroll_size, set_enable_fixed_scroll_size);
			WrapBasicScroll::inherit(cls, worker);
		}, Panel);
		IMPL::js_class(worker)->set_class_alias(JS_TYPEID(Scroll), View::SCROLL);
	}
};

void WrapViewBase::inherit_scroll(Local<JSClass> cls, Worker* worker) {
	WrapBasicScroll::inherit(cls, worker);
}

void binding_scroll(Local<JSObject> exports, Worker* worker) {
	WrapScroll::binding(exports, worker);
}

JS_END
