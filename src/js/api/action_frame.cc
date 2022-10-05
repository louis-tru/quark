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

#include "./_view.h"
#include "../srt.h"
#include "../../action/action.h"

/**
 * @ns quark::js
 */

JS_BEGIN

typedef KeyframeAction::Frame Frame;

#define def_get_property_from_type0(Name, block)\
	static void Name(Local<JSString> name, PropertyCall args) {\
	JS_WORKER(args);\
	JS_SELF(Frame);\
	if (self->host()) { block; } \
	else JS_THROW_ERR("No host property, unbind `KeyframeAction`");\
}
#define def_set_property_from_type0(Name, Type, Parser, block)\
static void set_##Name(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {\
	JS_WORKER(args); UILock lock;\
	JS_SELF(Frame);\
	if (self->host()) {\
		js_parse_value2(Type, Parser, value, "Action."#Name" = %s");\
		block;\
	}\
}

// ----------------------------------------------------------------

#define def_property_from_type(Name, Type) \
	def_get_property_from_type0(Name,JS_RETURN(worker->values()->New(self->Name()))) \
	def_set_property_from_type0(Name, Type, Type, self->set_##Name(out))

#define def_property_from_type2(Name, Type, get, set)\
	def_get_property_from_type0(Name,get) \
	def_set_property_from_type0(Name, Type, Type, set)

#define def_property_from_type3(Name, Type, Parser, get, set)\
	def_get_property_from_type0(Name,get) \
	def_set_property_from_type0(Name, Type, Parser, set)

/**
 * @class WrapFrame
 */
class WrapFrame: public WrapObject {
	public: 
	typedef Frame Type;

	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Forbidden access abstract");
	}
	
	/**
	 * @func fetch([view]) fetch style attribute by view
	 * @arg [view] {View}
	 */
	static void fetch(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		View* view = nullptr;
		if ( args.Length() > 0 && worker->hasInstance(args[0], View::VIEW) ) {
			view = Wrap<View>::unpack(args[0].To<JSObject>())->self();
		}
		JS_SELF(Frame);
		self->fetch(view);
	}

	/**
	 * @func flush() flush frame restore default values
	 */
	static void flush(FunctionCall args) {
		JS_WORKER(args); UILock lock;
		JS_SELF(Frame);
		self->flush();
	}

	/**
	 * @get index {uint} frame index in action
	 */
	static void index(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Frame);
		JS_RETURN( self->index() );
	}

	/**
	 * @get time {uint} ms
	 */
	static void time(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Frame);
		JS_RETURN( self->time() / 1000 );
	}

	/**
	 * @set time {uint} ms
	 */
	static void set_time(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			JS_THROW_ERR(
				"* @set time {uint} ms\n"
			);
		}
		JS_SELF(Frame);
		self->set_time(uint64(1000) * value->ToNumberValue(worker));
	}

	/**
	 * @get host {KeyframeAction}
	 */
	static void host(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(Frame);
		KeyframeAction* host = self->host();
		if ( host ) {
			JS_RETURN( Wrap<KeyframeAction>::pack(host)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}
	
	def_property_from_type(curve, Curve)
	
	// -------------------- get/set Meta attribute --------------------
	
	def_property_from_type(x, float);
	def_property_from_type(y, float);
	def_property_from_type(scale_x, float);
	def_property_from_type(scale_y, float);
	def_property_from_type(skew_x, float);
	def_property_from_type(skew_y, float);
	def_property_from_type(origin_x, float);
	def_property_from_type(origin_y, float);
	def_property_from_type(rotate_z, float);
	def_property_from_type(opacity, float);
	def_property_from_type(visible, bool);
	def_property_from_type(width, Value);
	def_property_from_type(height, Value);
	def_property_from_type(margin_left, Value);
	def_property_from_type(margin_top, Value);
	def_property_from_type(margin_right, Value);
	def_property_from_type(margin_bottom, Value);
	def_property_from_type(border_left_width, float);
	def_property_from_type(border_top_width, float);
	def_property_from_type(border_right_width, float);
	def_property_from_type(border_bottom_width, float);
	def_property_from_type(border_left_color, Color);
	def_property_from_type(border_top_color, Color);
	def_property_from_type(border_right_color, Color);
	def_property_from_type(border_bottom_color, Color);
	def_property_from_type(border_radius_left_top, float);
	def_property_from_type(border_radius_right_top, float);
	def_property_from_type(border_radius_right_bottom, float);
	def_property_from_type(border_radius_left_bottom, float);
	def_property_from_type(background_color, Color);
	def_property_from_type(newline, bool);
	def_property_from_type(clip, bool);
	def_property_from_type(content_align, ContentAlign);
	def_property_from_type(text_align, TextAlign);
	def_property_from_type(max_width, Value);
	def_property_from_type(max_height, Value);
	def_property_from_type(start_x, float);
	def_property_from_type(start_y, float);
	def_property_from_type(ratio_x, float);
	def_property_from_type(ratio_y, float);
	def_property_from_type(repeat, Repeat);
	def_property_from_type(text_background_color, TextColor);
	def_property_from_type(text_color, TextColor);
	def_property_from_type(text_size, TextSize);
	def_property_from_type(text_slant, TextSlant);
	def_property_from_type(text_family, TextFamily);
	def_property_from_type(text_line_height, TextLineHeight);
	def_property_from_type(text_shadow, TextShadow);
	def_property_from_type(text_decoration, TextDecoration);
	def_property_from_type(text_overflow, TextOverflow);
	def_property_from_type(text_white_space, TextWhiteSpace);
	def_property_from_type(align_x, Align);
	def_property_from_type(align_y, Align);
	def_property_from_type(shadow, Shadow);
	def_property_from_type(src, String);
	
	def_property_from_type3(background, BackgroundPtr, Background, {
		auto bg = self->background();
		if (bg) {
			JS_RETURN( pack(bg)->that() );
		} else {
			JS_RETURN_NULL();
		}
	}, {
		self->set_background(out);
	});
	
	// -------------------- get/set Non meta attribute --------------------
	
	def_property_from_type2(translate, Vec2, {
		JS_RETURN( worker->values()->New(Vec2(self->x(), self->y())) );
	}, {
		self->set_x(out.x());
		self->set_y(out.y());
	});
	def_property_from_type2(scale, Vec2, {
		JS_RETURN( worker->values()->New(Vec2(self->scale_x(), self->scale_y())) );
	},{
		self->set_scale_x(out.x());
		self->set_scale_y(out.y());
	});
	def_property_from_type2(skew, Vec2, {
		JS_RETURN( worker->values()->New(Vec2(self->skew_x(), self->skew_y())) );
	},{
		self->set_skew_x(out.x());
		self->set_skew_y(out.y());
	});
	def_property_from_type2(origin, Vec2, {
		JS_RETURN( worker->values()->New(Vec2(self->origin_x(), self->origin_y())) );
	},{
		self->set_origin_x(out.x());
		self->set_origin_y(out.y());
	});
	def_property_from_type3(margin, Array<Value>, Values, {
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->margin_top()) );
		arr->Set(worker, 1, worker->values()->New(self->margin_right()) );
		arr->Set(worker, 2, worker->values()->New(self->margin_bottom()) );
		arr->Set(worker, 3, worker->values()->New(self->margin_left()) );
		JS_RETURN( arr );
	},{ // set
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
	});
	def_property_from_type2(border, Border, {
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New({ self->border_top_width(), self->border_top_color() }) );
		arr->Set(worker, 1, worker->values()->New({ self->border_right_width(), self->border_right_color() }) );
		arr->Set(worker, 2, worker->values()->New({ self->border_bottom_width(), self->border_bottom_color() }) );
		arr->Set(worker, 3, worker->values()->New({ self->border_left_width(), self->border_left_color() }) );
		JS_RETURN( arr );
	}, { // set
		self->set_border_top_color(out.color);
		self->set_border_right_color(out.color);
		self->set_border_bottom_color(out.color);
		self->set_border_left_color(out.color);
		self->set_border_top_width(out.width);
		self->set_border_right_width(out.width);
		self->set_border_bottom_width(out.width);
		self->set_border_left_width(out.width);
	});
	def_property_from_type2(border_left, Border, {
		Border border(self->border_left_width(), self->border_left_color());
		JS_RETURN(worker->values()->New(border));
	}, {
		self->set_border_left_color(out.color);
		self->set_border_left_width(out.width);
	});
	def_property_from_type2(border_top, Border, {
		Border border(self->border_top_width(), self->border_top_color());
		JS_RETURN(worker->values()->New(border));
	}, {
		self->set_border_top_color(out.color);
		self->set_border_top_width(out.width);
	});
	def_property_from_type2(border_right, Border, {
		Border border(self->border_right_width(), self->border_right_color());
		JS_RETURN(worker->values()->New(border));
	}, {
		self->set_border_right_color(out.color);
		self->set_border_right_width(out.width);
	});
	def_property_from_type2(border_bottom, Border, {
		Border border(self->border_bottom_width(), self->border_bottom_color());
		JS_RETURN(worker->values()->New(border));
	}, {
		self->set_border_bottom_color(out.color);
		self->set_border_bottom_width(out.width);
	});
	def_property_from_type2(border_width, float, {
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->border_top_width()) );
		arr->Set(worker, 1, worker->values()->New(self->border_right_width()) );
		arr->Set(worker, 2, worker->values()->New(self->border_bottom_width()) );
		arr->Set(worker, 3, worker->values()->New(self->border_left_width()) );
		JS_RETURN( arr );
	}, { // set
		self->set_border_right_width(out);
		self->set_border_bottom_width(out);
		self->set_border_left_width(out);
		self->set_border_top_width(out);
	});
	def_property_from_type2(border_color, Color, {
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->border_top_color()) );
		arr->Set(worker, 1, worker->values()->New(self->border_right_color()) );
		arr->Set(worker, 2, worker->values()->New(self->border_bottom_color()) );
		arr->Set(worker, 3, worker->values()->New(self->border_left_color()) );
		JS_RETURN( arr );
	}, { // set
		self->set_border_top_color(out);
		self->set_border_right_color(out);
		self->set_border_bottom_color(out);
		self->set_border_left_color(out);
	});
	def_property_from_type2(border_radius, float, {
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->border_radius_left_top()) );
		arr->Set(worker, 1, worker->values()->New(self->border_radius_right_top()) );
		arr->Set(worker, 2, worker->values()->New(self->border_radius_left_bottom()) );
		arr->Set(worker, 3, worker->values()->New(self->border_radius_right_bottom()) );
		JS_RETURN( arr );
	}, {
		self->set_border_radius_left_top(out);
		self->set_border_radius_right_top(out);
		self->set_border_radius_left_bottom(out);
		self->set_border_radius_right_bottom(out);
	});
	def_property_from_type2(min_width, float, {
		JS_RETURN(worker->values()->New(self->width()));
	}, {
		self->set_width(out);
	});
	def_property_from_type2(min_height, float, {
		JS_RETURN(worker->values()->New(self->height()));
	}, {
		self->set_height(out);
	});
	def_property_from_type2(start, Vec2, {
		JS_RETURN(worker->values()->New(Vec2(self->start_x(), self->start_y())));
	}, {
		self->set_start_x(out.x());
		self->set_start_y(out.y());
	});
	def_property_from_type2(ratio, Vec2, {
		JS_RETURN(worker->values()->New(Vec2(self->ratio_x(), self->ratio_y())));
	}, {
		self->set_ratio_x(out.x());
		self->set_ratio_y(out.y());
	});
	def_property_from_type3(align, Array<Align>, Aligns, {
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->align_x()) );
		arr->Set(worker, 1, worker->values()->New(self->align_y()) );
		JS_RETURN( arr );
	}, {
		self->set_align_x(out[0]);
		self->set_align_y(out[1]);
	});
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_DEFINE_CLASS(Frame, constructor, {
			JS_SET_CLASS_METHOD(fetch, fetch);
			JS_SET_CLASS_METHOD(flush, flush);
			JS_SET_CLASS_ACCESSOR(index, index);
			JS_SET_CLASS_ACCESSOR(host, host);
			JS_SET_CLASS_ACCESSOR(time, time, set_time);
			JS_SET_CLASS_ACCESSOR(curve, curve, set_curve);
			// Meta attribute
			JS_SET_CLASS_ACCESSOR(x, x, set_x);
			JS_SET_CLASS_ACCESSOR(y, y, set_y);
			JS_SET_CLASS_ACCESSOR(scaleX, scale_x, set_scale_x);
			JS_SET_CLASS_ACCESSOR(scaleY, scale_y, set_scale_y);
			JS_SET_CLASS_ACCESSOR(skewX, skew_x, set_skew_x);
			JS_SET_CLASS_ACCESSOR(skewY, skew_y, set_skew_y);
			JS_SET_CLASS_ACCESSOR(originX, origin_x, set_origin_x);
			JS_SET_CLASS_ACCESSOR(originY, origin_y, set_origin_y);
			JS_SET_CLASS_ACCESSOR(rotateZ, rotate_z, set_rotate_z);
			JS_SET_CLASS_ACCESSOR(opacity, opacity, set_opacity);
			JS_SET_CLASS_ACCESSOR(visible, visible, set_visible);
			JS_SET_CLASS_ACCESSOR(width, width, set_width);
			JS_SET_CLASS_ACCESSOR(height, height, set_height);
			JS_SET_CLASS_ACCESSOR(marginLeft, margin_left, set_margin_left);
			JS_SET_CLASS_ACCESSOR(marginTop, margin_top, set_margin_top);
			JS_SET_CLASS_ACCESSOR(marginRight, margin_right, set_margin_right);
			JS_SET_CLASS_ACCESSOR(marginBottom, margin_bottom, set_margin_bottom);
			JS_SET_CLASS_ACCESSOR(borderLeftWidth, border_left_width, set_border_left_width);
			JS_SET_CLASS_ACCESSOR(borderTopWidth, border_top_width, set_border_top_width);
			JS_SET_CLASS_ACCESSOR(borderRightWidth, border_right_width, set_border_right_width);
			JS_SET_CLASS_ACCESSOR(borderBottomWidth, border_bottom_width, set_border_bottom_width);
			JS_SET_CLASS_ACCESSOR(borderLeftColor, border_left_color, set_border_left_color);
			JS_SET_CLASS_ACCESSOR(borderTopColor, border_top_color, set_border_top_color);
			JS_SET_CLASS_ACCESSOR(borderRightColor, border_right_color, set_border_right_color);
			JS_SET_CLASS_ACCESSOR(borderBottomColor, border_bottom_color, set_border_bottom_color);
			JS_SET_CLASS_ACCESSOR(borderRadiusLeftTop, border_radius_left_top, set_border_radius_left_top);
			JS_SET_CLASS_ACCESSOR(borderRadiusRightTop, border_radius_right_top, set_border_radius_right_top);
			JS_SET_CLASS_ACCESSOR(borderRadiusRightBottom, border_radius_right_bottom, set_border_radius_right_bottom);
			JS_SET_CLASS_ACCESSOR(borderRadiusLeftBottom, border_radius_left_bottom, set_border_radius_left_bottom);
			JS_SET_CLASS_ACCESSOR(backgroundColor, background_color, set_background_color);
			JS_SET_CLASS_ACCESSOR(background, background, set_background);
			JS_SET_CLASS_ACCESSOR(newline, newline, set_newline);
			JS_SET_CLASS_ACCESSOR(clip, clip, set_clip);
			JS_SET_CLASS_ACCESSOR(contentAlign, content_align, set_content_align);
			JS_SET_CLASS_ACCESSOR(textAlign, text_align, set_text_align);
			JS_SET_CLASS_ACCESSOR(maxWidth, max_width, set_max_width);
			JS_SET_CLASS_ACCESSOR(maxHeight, max_height, set_max_height);
			JS_SET_CLASS_ACCESSOR(startX, start_x, set_start_x);
			JS_SET_CLASS_ACCESSOR(startY, start_y, set_start_y);
			JS_SET_CLASS_ACCESSOR(ratioX, ratio_x, set_ratio_x);
			JS_SET_CLASS_ACCESSOR(ratioY, ratio_y, set_ratio_y);
			JS_SET_CLASS_ACCESSOR(repeat, repeat, set_repeat);
			JS_SET_CLASS_ACCESSOR(textBackgroundColor, text_background_color, set_text_background_color);
			JS_SET_CLASS_ACCESSOR(textColor, text_color, set_text_color);
			JS_SET_CLASS_ACCESSOR(textSize, text_size, set_text_size);
			JS_SET_CLASS_ACCESSOR(TextSlant, text_slant, set_text_slant);
			JS_SET_CLASS_ACCESSOR(textFamily, text_family, set_text_family);
			JS_SET_CLASS_ACCESSOR(textLineHeight, text_line_height, set_text_line_height);
			JS_SET_CLASS_ACCESSOR(textShadow, text_shadow, set_text_shadow);
			JS_SET_CLASS_ACCESSOR(textDecoration, text_decoration, set_text_decoration);
			JS_SET_CLASS_ACCESSOR(textOverflow, text_overflow, set_text_overflow);
			JS_SET_CLASS_ACCESSOR(textWhiteSpace, text_white_space, set_text_white_space);
			JS_SET_CLASS_ACCESSOR(alignX, align_x, set_align_x);
			JS_SET_CLASS_ACCESSOR(alignY, align_y, set_align_y);
			JS_SET_CLASS_ACCESSOR(shadow, shadow, set_shadow);
			JS_SET_CLASS_ACCESSOR(src, src, set_src);
			// Non meta attributecurve
			JS_SET_CLASS_ACCESSOR(translate, translate, set_translate);
			JS_SET_CLASS_ACCESSOR(scale, scale, set_scale);
			JS_SET_CLASS_ACCESSOR(skew, skew, set_skew);
			JS_SET_CLASS_ACCESSOR(origin, origin, set_origin);
			JS_SET_CLASS_ACCESSOR(margin, margin, set_margin);
			JS_SET_CLASS_ACCESSOR(border, border, set_border);
			JS_SET_CLASS_ACCESSOR(borderLeft, border_left, set_border_left);
			JS_SET_CLASS_ACCESSOR(borderTop, border_top, set_border_top);
			JS_SET_CLASS_ACCESSOR(borderRight, border_right, set_border_right);
			JS_SET_CLASS_ACCESSOR(borderBottom, border_bottom, set_border_bottom);
			JS_SET_CLASS_ACCESSOR(borderWidth, border_width, set_border_width);
			JS_SET_CLASS_ACCESSOR(borderColor, border_color, set_border_color);
			JS_SET_CLASS_ACCESSOR(borderRadius, border_radius, set_border_radius);
			JS_SET_CLASS_ACCESSOR(minWidth, min_width, set_min_width);
			JS_SET_CLASS_ACCESSOR(minHeight, min_height, set_min_height);
			JS_SET_CLASS_ACCESSOR(start, start, set_start);
			JS_SET_CLASS_ACCESSOR(ratio, ratio, set_ratio);
			JS_SET_CLASS_ACCESSOR(align, align, set_align);
		}, nullptr);
	}
};

void binding_frame(Local<JSObject> exports, Worker* worker) {
	WrapFrame::binding(exports, worker);
}

JS_END
