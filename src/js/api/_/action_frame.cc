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
 * @ns qk::js
 */

Js_BEGIN

typedef KeyframeAction::Frame Frame;

#define def_get_property_from_type0(Name, block)\
	static void Name(JSString* name, PropertyArgs args) {\
	Js_Worker(args);\
	Js_Self(Frame);\
	if (self->host()) { block; } \
	else Js_Throw("No host property, unbind `KeyframeAction`");\
}
#define def_set_property_from_type0(Name, Type, Parser, block)\
static void set_##Name(JSString* name, JSValue* value, PropertySetArgs args) {\
	Js_Worker(args); UILock lock;\
	Js_Self(Frame);\
	if (self->host()) {\
		js_parse_value2(Type, Parser, value, "Action."#Name" = %s");\
		block;\
	}\
}

// ----------------------------------------------------------------

#define def_property_from_type(Name, Type) \
	def_get_property_from_type0(Name,Js_Return(worker->values()->New(self->Name()))) \
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

	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_Worker(args);
		Js_Throw("Forbidden access abstract");
	}
	
	/**
	 * @method fetch([view]) fetch style attribute by view
	 * @param [view] {View}
	 */
	static void fetch(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		View* view = nullptr;
		if ( args.length() > 0 && worker->hasInstance(args[0], View::VIEW) ) {
			view = Wrap<View>::unpack(args[0].To<JSObject>())->self();
		}
		Js_Self(Frame);
		self->fetch(view);
	}

	/**
	 * @method flush() flush frame restore default values
	 */
	static void flush(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		Js_Self(Frame);
		self->flush();
	}

	/**
	 * @get index {uint} frame index in action
	 */
	static void index(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Frame);
		Js_Return( self->index() );
	}

	/**
	 * @get time {uint} ms
	 */
	static void time(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Frame);
		Js_Return( self->time() / 1000 );
	}

	/**
	 * @set time {uint} ms
	 */
	static void set_time(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args); UILock lock;
		if ( !value->IsNumber(worker) ) {
			Js_Throw(
				"* @set time {uint} ms\n"
			);
		}
		Js_Self(Frame);
		self->set_time(uint64(1000) * value->ToNumberValue(worker));
	}

	/**
	 * @get host {KeyframeAction}
	 */
	static void host(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(Frame);
		KeyframeAction* host = self->host();
		if ( host ) {
			Js_Return( Wrap<KeyframeAction>::pack(host)->that() );
		} else {
			Js_Return_Null();
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
			Js_Return( pack(bg)->that() );
		} else {
			Js_Return_Null();
		}
	}, {
		self->set_background(out);
	});
	
	// -------------------- get/set Non meta attribute --------------------
	
	def_property_from_type2(translate, Vec2, {
		Js_Return( worker->values()->New(Vec2(self->x(), self->y())) );
	}, {
		self->set_x(out.x());
		self->set_y(out.y());
	});
	def_property_from_type2(scale, Vec2, {
		Js_Return( worker->values()->New(Vec2(self->scale_x(), self->scale_y())) );
	},{
		self->set_scale_x(out.x());
		self->set_scale_y(out.y());
	});
	def_property_from_type2(skew, Vec2, {
		Js_Return( worker->values()->New(Vec2(self->skew_x(), self->skew_y())) );
	},{
		self->set_skew_x(out.x());
		self->set_skew_y(out.y());
	});
	def_property_from_type2(origin, Vec2, {
		Js_Return( worker->values()->New(Vec2(self->origin_x(), self->origin_y())) );
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
		Js_Return( arr );
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
		Js_Return( arr );
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
		Js_Return(worker->values()->New(border));
	}, {
		self->set_border_left_color(out.color);
		self->set_border_left_width(out.width);
	});
	def_property_from_type2(border_top, Border, {
		Border border(self->border_top_width(), self->border_top_color());
		Js_Return(worker->values()->New(border));
	}, {
		self->set_border_top_color(out.color);
		self->set_border_top_width(out.width);
	});
	def_property_from_type2(border_right, Border, {
		Border border(self->border_right_width(), self->border_right_color());
		Js_Return(worker->values()->New(border));
	}, {
		self->set_border_right_color(out.color);
		self->set_border_right_width(out.width);
	});
	def_property_from_type2(border_bottom, Border, {
		Border border(self->border_bottom_width(), self->border_bottom_color());
		Js_Return(worker->values()->New(border));
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
		Js_Return( arr );
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
		Js_Return( arr );
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
		Js_Return( arr );
	}, {
		self->set_border_radius_left_top(out);
		self->set_border_radius_right_top(out);
		self->set_border_radius_left_bottom(out);
		self->set_border_radius_right_bottom(out);
	});
	def_property_from_type2(min_width, float, {
		Js_Return(worker->values()->New(self->width()));
	}, {
		self->set_width(out);
	});
	def_property_from_type2(min_height, float, {
		Js_Return(worker->values()->New(self->height()));
	}, {
		self->set_height(out);
	});
	def_property_from_type2(start, Vec2, {
		Js_Return(worker->values()->New(Vec2(self->start_x(), self->start_y())));
	}, {
		self->set_start_x(out.x());
		self->set_start_y(out.y());
	});
	def_property_from_type2(ratio, Vec2, {
		Js_Return(worker->values()->New(Vec2(self->ratio_x(), self->ratio_y())));
	}, {
		self->set_ratio_x(out.x());
		self->set_ratio_y(out.y());
	});
	def_property_from_type3(align, Array<Align>, Aligns, {
		auto arr = worker->NewArray();
		arr->Set(worker, 0, worker->values()->New(self->align_x()) );
		arr->Set(worker, 1, worker->values()->New(self->align_y()) );
		Js_Return( arr );
	}, {
		self->set_align_x(out[0]);
		self->set_align_y(out[1]);
	});
	
	static void binding(JSObject* exports, Worker* worker) {
		Js_Define_Class(Frame, constructor, {
			Js_Set_Class_Method(fetch, fetch);
			Js_Set_Class_Method(flush, flush);
			Js_Set_Class_Accessor(index, index);
			Js_Set_Class_Accessor(host, host);
			Js_Set_Class_Accessor(time, time, set_time);
			Js_Set_Class_Accessor(curve, curve, set_curve);
			// Meta attribute
			Js_Set_Class_Accessor(x, x, set_x);
			Js_Set_Class_Accessor(y, y, set_y);
			Js_Set_Class_Accessor(scaleX, scale_x, set_scale_x);
			Js_Set_Class_Accessor(scaleY, scale_y, set_scale_y);
			Js_Set_Class_Accessor(skewX, skew_x, set_skew_x);
			Js_Set_Class_Accessor(skewY, skew_y, set_skew_y);
			Js_Set_Class_Accessor(originX, origin_x, set_origin_x);
			Js_Set_Class_Accessor(originY, origin_y, set_origin_y);
			Js_Set_Class_Accessor(rotateZ, rotate_z, set_rotate_z);
			Js_Set_Class_Accessor(opacity, opacity, set_opacity);
			Js_Set_Class_Accessor(visible, visible, set_visible);
			Js_Set_Class_Accessor(width, width, set_width);
			Js_Set_Class_Accessor(height, height, set_height);
			Js_Set_Class_Accessor(marginLeft, margin_left, set_margin_left);
			Js_Set_Class_Accessor(marginTop, margin_top, set_margin_top);
			Js_Set_Class_Accessor(marginRight, margin_right, set_margin_right);
			Js_Set_Class_Accessor(marginBottom, margin_bottom, set_margin_bottom);
			Js_Set_Class_Accessor(borderLeftWidth, border_left_width, set_border_left_width);
			Js_Set_Class_Accessor(borderTopWidth, border_top_width, set_border_top_width);
			Js_Set_Class_Accessor(borderRightWidth, border_right_width, set_border_right_width);
			Js_Set_Class_Accessor(borderBottomWidth, border_bottom_width, set_border_bottom_width);
			Js_Set_Class_Accessor(borderLeftColor, border_left_color, set_border_left_color);
			Js_Set_Class_Accessor(borderTopColor, border_top_color, set_border_top_color);
			Js_Set_Class_Accessor(borderRightColor, border_right_color, set_border_right_color);
			Js_Set_Class_Accessor(borderBottomColor, border_bottom_color, set_border_bottom_color);
			Js_Set_Class_Accessor(borderRadiusLeftTop, border_radius_left_top, set_border_radius_left_top);
			Js_Set_Class_Accessor(borderRadiusRightTop, border_radius_right_top, set_border_radius_right_top);
			Js_Set_Class_Accessor(borderRadiusRightBottom, border_radius_right_bottom, set_border_radius_right_bottom);
			Js_Set_Class_Accessor(borderRadiusLeftBottom, border_radius_left_bottom, set_border_radius_left_bottom);
			Js_Set_Class_Accessor(backgroundColor, background_color, set_background_color);
			Js_Set_Class_Accessor(background, background, set_background);
			Js_Set_Class_Accessor(newline, newline, set_newline);
			Js_Set_Class_Accessor(clip, clip, set_clip);
			Js_Set_Class_Accessor(contentAlign, content_align, set_content_align);
			Js_Set_Class_Accessor(textAlign, text_align, set_text_align);
			Js_Set_Class_Accessor(maxWidth, max_width, set_max_width);
			Js_Set_Class_Accessor(maxHeight, max_height, set_max_height);
			Js_Set_Class_Accessor(startX, start_x, set_start_x);
			Js_Set_Class_Accessor(startY, start_y, set_start_y);
			Js_Set_Class_Accessor(ratioX, ratio_x, set_ratio_x);
			Js_Set_Class_Accessor(ratioY, ratio_y, set_ratio_y);
			Js_Set_Class_Accessor(repeat, repeat, set_repeat);
			Js_Set_Class_Accessor(textBackgroundColor, text_background_color, set_text_background_color);
			Js_Set_Class_Accessor(textColor, text_color, set_text_color);
			Js_Set_Class_Accessor(textSize, text_size, set_text_size);
			Js_Set_Class_Accessor(TextSlant, text_slant, set_text_slant);
			Js_Set_Class_Accessor(textFamily, text_family, set_text_family);
			Js_Set_Class_Accessor(textLineHeight, text_line_height, set_text_line_height);
			Js_Set_Class_Accessor(textShadow, text_shadow, set_text_shadow);
			Js_Set_Class_Accessor(textDecoration, text_decoration, set_text_decoration);
			Js_Set_Class_Accessor(textOverflow, text_overflow, set_text_overflow);
			Js_Set_Class_Accessor(textWhiteSpace, text_white_space, set_text_white_space);
			Js_Set_Class_Accessor(alignX, align_x, set_align_x);
			Js_Set_Class_Accessor(alignY, align_y, set_align_y);
			Js_Set_Class_Accessor(shadow, shadow, set_shadow);
			Js_Set_Class_Accessor(src, src, set_src);
			// Non meta attributecurve
			Js_Set_Class_Accessor(translate, translate, set_translate);
			Js_Set_Class_Accessor(scale, scale, set_scale);
			Js_Set_Class_Accessor(skew, skew, set_skew);
			Js_Set_Class_Accessor(origin, origin, set_origin);
			Js_Set_Class_Accessor(margin, margin, set_margin);
			Js_Set_Class_Accessor(border, border, set_border);
			Js_Set_Class_Accessor(borderLeft, border_left, set_border_left);
			Js_Set_Class_Accessor(borderTop, border_top, set_border_top);
			Js_Set_Class_Accessor(borderRight, border_right, set_border_right);
			Js_Set_Class_Accessor(borderBottom, border_bottom, set_border_bottom);
			Js_Set_Class_Accessor(borderWidth, border_width, set_border_width);
			Js_Set_Class_Accessor(borderColor, border_color, set_border_color);
			Js_Set_Class_Accessor(borderRadius, border_radius, set_border_radius);
			Js_Set_Class_Accessor(minWidth, min_width, set_min_width);
			Js_Set_Class_Accessor(minHeight, min_height, set_min_height);
			Js_Set_Class_Accessor(start, start, set_start);
			Js_Set_Class_Accessor(ratio, ratio, set_ratio);
			Js_Set_Class_Accessor(align, align, set_align);
		}, nullptr);
	}
};

void binding_frame(JSObject* exports, Worker* worker) {
	WrapFrame::binding(exports, worker);
}

Js_END
