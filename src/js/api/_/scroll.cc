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

#include "../js_.h"
#include "../_view.h"
#include "../../views2/scroll.h"

/**
 * @ns qk::js
 */

Js_BEGIN

#define js_scroll_self() \
BasicScroll* self = Wrap<View>::unpack(args.This())->self()->as_basic_scroll()

class WrapBasicScroll {
	public:

	/**
	 * @method scrollTo(scroll[,duration[,curve]])
	 * @param scroll {Vec2}
	 * @param [duration] {uint} ms
	 * @param [curve] {Curve}
	 */
	static void scroll_to(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		int64_t duration = 0;
		
		if ( args.length() == 0 ) {
			Js_Throw(
				"* @method scrollTo(scroll[,duration[,curve]])\n"
				"* @param scroll {Vec2}\n"
				"* @param [duration] {uint} ms\n"
				"* @param [curve] {Curve}\n"
			);
		}
		
		js_parse_value(Vec2, args[0], "BasicScroll.scrollTo(%s)");
		
		Vec2 scroll = out;
		
		js_scroll_self();
		
		if ( args.length() > 1 && args[1]->IsNumber(worker) ) {
			duration = args[1]->ToNumberValue(worker) / 1000;
			duration = Qk_MAX(duration, 0);
		}
		
		if ( args.length() > 2 ) {
			js_parse_value(Curve, args[2], "BasicScroll.scrollTo(vec2, %s)");
			self->scroll_to(scroll, duration, out);
			return;
		}
		
		self->scroll_to(scroll, duration);
	}
	
	static void scroll(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		Js_Return( worker->values()->New(self->scroll()) );
	}
	
	static void set_scroll(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Vec2, value, "BasicScroll.scroll = %s");
		js_scroll_self();
		self->set_scroll(out);
	}
	
	static void scroll_x(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		Js_Return( self->scroll_x() );
	}
	
	static void scroll_y(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		Js_Return( self->scroll_y() );
	}
	
	/**
	 * @set scroll_x {float} 
	 */
	static void set_scroll_x(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set scrollX {float} ");
		js_scroll_self();
		self->set_scroll_x(value->ToNumberValue(worker));
	}
	
	/**
	 * @set scroll_y {float} 
	 */
	static void set_scroll_y(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set scrollY {float} ");
		js_scroll_self();
		self->set_scroll_y(value->ToNumberValue(worker));
	}
	
	static void scroll_width(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		Js_Return( self->scroll_width() );
	}
	
	static void scroll_height(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		Js_Return( self->scroll_height() );
	}
	
	static void scrollbar(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->scrollbar() );
	}
	
	static void set_scrollbar(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		self->set_scrollbar(value->ToBooleanValue(worker));
	}
	
	static void resistance(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->resistance() );
	}
	
	/**
	 * @set resistance {float} 
	 */
	static void set_resistance(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set resistance {float} ");
		js_scroll_self();
		self->set_resistance(value->ToNumberValue(worker));
	}
	
	static void bounce(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->bounce() );
	}
	
	static void set_bounce(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		self->set_bounce(value->ToBooleanValue(worker));
	}
	
	static void bounce_lock(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->bounce_lock() );
	}
	
	static void set_bounce_lock(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		self->set_bounce_lock(value->ToBooleanValue(worker));
	}
	
	static void momentum(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->momentum() );
	}
	
	static void set_momentum(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		self->set_momentum(value->ToBooleanValue(worker));
	}
	
	static void lock_direction(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->lock_direction() );
	}
	
	static void set_lock_direction(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		self->set_lock_direction(value->ToBooleanValue(worker));
	}
	
	static void catch_position_x(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->catch_position_x() );
	}
	
	static void catch_position_y(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->catch_position_y() );
	}
	
	/**
	 * @set catch_position_x {float} 
	 */
	static void set_catch_position_x(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set catchPositionX {float} ");
		js_scroll_self();
		self->set_catch_position_x(value->ToNumberValue(worker));
	}
	
	/**
	 * @set catch_position_y {float} 
	 */
	static void set_catch_position_y(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set catchPositionY {float} ");
		js_scroll_self();
		self->set_catch_position_y(value->ToNumberValue(worker));
	}
	
	static void scrollbar_color(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( worker->values()->New(self->scrollbar_color()) );
	}
	
	static void set_scrollbar_color(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Color, value, "BasicScroll.scrollbarColor = %s");
		js_scroll_self();
		self->set_scrollbar_color(out);
	}
	
	static void h_scrollbar(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		Js_Return( self->h_scrollbar() );
	}
	
	static void v_scrollbar(JSString* name, PropertyArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		Js_Return( self->v_scrollbar() );
	}
	
	static void scrollbar_width(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->scrollbar_width() );
	}
	
	/**
	 * @set scrollbar_width {float} 
	 */
	static void set_scrollbar_width(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set scrollbarWidth {float} ");
		js_scroll_self();
		self->set_scrollbar_width( value->ToNumberValue(worker) );
	}
	
	static void scrollbar_margin(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->scrollbar_margin() );
	}
	
	/**
	 * @set scrollbar_margin {float} 
	 */
	static void set_scrollbar_margin(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set scrollbarMargin {float} ");
		js_scroll_self();
		self->set_scrollbar_margin( value->ToNumberValue(worker) );
	}
	
	static void default_scroll_duration(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( self->default_scroll_duration() / 1000 );
	}
	
	/**
	 * @set default_scroll_duration {uint} ms
	 */
	static void set_default_scroll_duration(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( ! value->IsNumber(worker) ) Js_Throw("* @set defaultScrollDuration {uint} ms");
		js_scroll_self();
		self->set_default_scroll_duration( Qk_MAX(value->ToNumberValue(worker), 0) );
	}
	
	static void default_scroll_curve(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		js_scroll_self();
		Js_Return( worker->values()->New(self->default_scroll_curve()) );
	}
	
	static void set_default_scroll_curve(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Curve, value, "BasicScroll.defaultScrollCurve = %s");
		js_scroll_self();
		self->set_default_scroll_curve(out);
	}
	
	static void terminate(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		js_scroll_self();
		self->terminate();
	}

	static void inherit(Local<JSClass> cls, Worker* worker) {
		Js_Set_Class_Method(scrollTo, scroll_to);
		Js_Set_Class_Method(terminate, terminate);
		Js_Set_Class_Accessor(scroll, scroll, set_scroll);
		Js_Set_Class_Accessor(scrollX, scroll_x, set_scroll_x);
		Js_Set_Class_Accessor(scrollY, scroll_y, set_scroll_y);
		Js_Set_Class_Accessor(scrollWidth, scroll_width);
		Js_Set_Class_Accessor(scrollHeight, scroll_height);
		Js_Set_Class_Accessor(scrollbar, scrollbar, set_scrollbar);
		Js_Set_Class_Accessor(resistance, resistance, set_resistance);
		Js_Set_Class_Accessor(bounce, bounce, set_bounce);
		Js_Set_Class_Accessor(bounceLock, bounce_lock, set_bounce_lock);
		Js_Set_Class_Accessor(momentum, momentum, set_momentum);
		Js_Set_Class_Accessor(lockDirection, lock_direction, set_lock_direction);
		Js_Set_Class_Accessor(catchPositionX, catch_position_x, set_catch_position_x);
		Js_Set_Class_Accessor(catchPositionY, catch_position_y, set_catch_position_y);
		Js_Set_Class_Accessor(scrollbarColor, scrollbar_color, set_scrollbar_color);
		Js_Set_Class_Accessor(hScrollbar, h_scrollbar);
		Js_Set_Class_Accessor(vScrollbar, v_scrollbar);
		Js_Set_Class_Accessor(scrollbarWidth, scrollbar_width, set_scrollbar_width);
		Js_Set_Class_Accessor(scrollbarMargin, scrollbar_margin, set_scrollbar_margin);
		Js_Set_Class_Accessor(defaultScrollDuration, default_scroll_duration, set_default_scroll_duration);
		Js_Set_Class_Accessor(defaultScrollCurve, default_scroll_curve, set_default_scroll_curve);
	}
};

class WrapScroll: public WrapViewBase {
	public:

	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_CHECK_APP();
		New<WrapScroll>(args, new Scroll());
	}
	
	static void focus_margin_left(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( self->focus_margin_left() );
	}
	
	static void focus_margin_right(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( self->focus_margin_right() );
	}
	
	static void focus_margin_top(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( self->focus_margin_top() );
	}
	
	static void focus_margin_bottom(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( self->focus_margin_bottom() );
	}
	
	static void focus_align_x(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( worker->values()->New( self->focus_align_x() ) );
	}
	
	static void focus_align_y(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( worker->values()->New( self->focus_align_y() ) );
	}
	
	/**
	 * @set focus_margin_left {float}
	 */
	static void set_focus_margin_left(JSString* name,
																		 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			Js_Throw("Bad argument.");
		}
		Js_Self(Scroll);
		self->set_focus_margin_left( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_margin_right {float}
	 */
	static void set_focus_margin_right(JSString* name,
																			JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			Js_Throw("Bad argument.");
		}
		Js_Self(Scroll);
		self->set_focus_margin_right( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_margin_top {float}
	 */
	static void set_focus_margin_top(JSString* name,
																		JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			Js_Throw("Bad argument.");
		}
		Js_Self(Scroll);
		self->set_focus_margin_top( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_margin_bottom {float}
	 */
	static void set_focus_margin_bottom(JSString* name,
																			 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			Js_Throw("Bad argument.");
		}
		Js_Self(Scroll);
		self->set_focus_margin_bottom( value->ToNumberValue(worker) );
	}
	
	/**
	 * @set focus_align_x {float}
	 */
	static void set_focus_align_x(JSString* name,
																 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Align, value, "BasicScroll.selectMotionAlignX = %s");
		Js_Self(Scroll);
		self->set_focus_align_x(out);
	}
	
	static void set_focus_align_y(JSString* name,
																 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		js_parse_value(Align, value, "BasicScroll.selectMotionAlignY = %s");
		Js_Self(Scroll);
		self->set_focus_align_y(out);
	}
	
	static void enable_focus_align(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( self->enable_focus_align() );
	}
	
	static void set_enable_focus_align(JSString* name,
																		 JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Scroll);
		self->set_enable_focus_align( value->ToBooleanValue(worker) );
	}
	
	static void enable_fixed_scroll_size(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Scroll);
		Js_Return( self->enable_fixed_scroll_size() );
	}
	
	static void set_enable_fixed_scroll_size(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		if ( args.length() < 1 ) {
			Js_Throw(
				"* @method BasicScroll.setFixedScrollSize(size: Vec2)\n"
			);
		}
		js_parse_value(Vec2, args[0], "BasicScroll.enableFixedScrollSize = %s");
		Js_Self(Scroll);
		self->set_enable_fixed_scroll_size( out );
	}

	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(Scroll, constructor, {
			Js_Set_Class_Accessor(focusMarginLeft, focus_margin_left, set_focus_margin_left);
			Js_Set_Class_Accessor(focusMarginRight, focus_margin_right, set_focus_margin_right);
			Js_Set_Class_Accessor(focusMarginTop, focus_margin_top, set_focus_margin_top);
			Js_Set_Class_Accessor(focusMarginBottom, focus_margin_bottom, set_focus_margin_bottom);
			Js_Set_Class_Accessor(focusAlignX, focus_align_x, set_focus_align_x);
			Js_Set_Class_Accessor(focusAlignY, focus_align_y, set_focus_align_y);
			Js_Set_Class_Accessor(enableFocusAlign, enable_focus_align, set_enable_focus_align);
			Js_Set_Class_Accessor(isFixedScrollSize, enable_fixed_scroll_size);
			Js_Set_Class_Method(setFixedScrollSize, set_enable_fixed_scroll_size);
			WrapBasicScroll::inherit(cls, worker);
		}, Panel);
		IMPL::js_class(worker)->set_class_alias(Js_Typeid(Scroll), View::SCROLL);
	}
};

void WrapViewBase::inheritScroll(Local<JSClass> cls, Worker* worker) {
	WrapBasicScroll::inherit(cls, worker);
}

void binding_scroll(JSObject* exports, Worker* worker) {
	WrapScroll::binding(exports, worker);
}

Js_END
