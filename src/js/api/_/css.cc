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

#include "../js.h"
#include "../../css/css.h"

/**
 * @ns qk::js
 */

Js_BEGIN

#define def_get_property_from_type0(Name) \
	/*static void Name(JSString* name, PropertyArgs args) {}*/
#define def_set_property_from_type0(Name, Type, Parser, block)\
static void set_##Name(JSString* name, JSValue* value, PropertySetArgs args) {\
	Js_Worker(args);\
	Js_Self(StyleSheets);\
	js_parse_value2(Type, Parser, value, "StyleSheets."#Name" = %s");\
	block;\
}

// ---------------------------------------------------

#define def_property_from_type(Name, Type)\
	def_get_property_from_type0(Name)\
	def_set_property_from_type0(Name,Type,Type,self->set_##Name(out))

#define def_property_from_type2(Name, Type, block)\
	def_get_property_from_type0(Name)\
	def_set_property_from_type0(Name,Type,Type,block)

#define def_property_from_type3(Name, Type, Parser, block)\
	def_get_property_from_type0(Name)\
	def_set_property_from_type0(Name,Type,Parser,block)

/**
 * @class WrapFrame
 */
class WrapStyleSheets: public WrapObject {
	public:
	
	static void constructor(FunctionArgs args) {
		Js_ATTACH(args);
		Js_Worker(args);
		Js_Throw("Forbidden access abstract");
	}
	
	static void create(FunctionArgs args) {
		Js_Worker(args); UILock lock;
		
		if ( args.length() < 1 || !args[0]->IsObject(worker) || args[0]->IsNull(worker) ) {
			Js_Throw("Bad argument.");
		}
		
		Js_Handle_Scope();
		
		JSObject* arg = args[0].To<JSObject>();
		Local<JSArray> names = arg->GetPropertyNames(worker);
		
		for ( uint32_t i = 0, len = names->Length(worker); i < len; i++ ) {
			JSValue* key = names->Get(worker, i);
			JSValue* val = arg->Get(worker, key);
			if ( val.IsEmpty() ) {
				return; // js error
			}
			if ( val.IsEmpty() || ! val->IsObject(worker) ) {
				Js_Throw("Bad argument.");
			}
			
			Array<StyleSheets*> arr =
				root_styles()->instances( key->toStringValue(worker) ); // new instances
			
			if ( arr.length() ) {
				JSObject* propertys = val.To<JSObject>();
				Local<JSArray> names = propertys->GetPropertyNames(worker);
				
				for ( uint32_t j = 0, len = names->Length(worker); j < len; j++ ) {
					JSValue* key;
					key = names->Get(worker, j);
					val = propertys->Get(worker, key);
					if ( val.IsEmpty() ) {
						return; // js error
					}
					for ( auto& i : arr ) {
						StyleSheets* ss = i.value();
						JSObject* local = Wrap<StyleSheets>::pack(ss)->that();
						if ( !local->Set(worker, key, val) ) {
							return; // js error
						}
					}
				}
				// if (arr.length)
			}
		}
	}
	
	static void time(JSString* name, PropertyArgs args) {
		Js_Worker(args);
		Js_Self(StyleSheets);
		Js_Return( self->time() / 1000 );
	}
	
	static void set_time(JSString* name, JSValue* value, PropertySetArgs args) {
		Js_Worker(args);
		if ( ! value->IsNumber(worker) ) {
			Js_Throw("Bad argument.");
		}
		Js_Self(StyleSheets);
		self->set_time(uint64(1000) * value->ToNumberValue(worker));
	}
	
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
	def_property_from_type3(background, BackgroundPtr, Background, { self->set_background(out); });
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
	def_property_from_type(newline, bool);
	def_property_from_type(clip, bool);
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
	
	// -------------------- get/set Non meta attribute --------------------
	
	def_property_from_type2(translate, Vec2, {
		self->set_x(out.x()); self->set_y(out.y());
	});
	def_property_from_type2(scale, Vec2, {
		self->set_scale_x(out.x()); self->set_scale_y(out.y());
	});
	def_property_from_type2(skew, Vec2, {
		self->set_skew_x(out.x()); self->set_skew_y(out.y());
	});
	def_property_from_type2(origin, Vec2, {
		self->set_origin_x(out.x()); self->set_origin_y(out.y());
	});
	def_property_from_type3(margin, Array<Value>, Values, {
		switch (out.length()) {
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
	def_property_from_type3(align, Array<Align>, Aligns, {
		self->set_align_x(out[0]);
		self->set_align_y(out[1]);
	});
	def_property_from_type2(border, Border, {
		self->set_border_left_color(out.color);
		self->set_border_top_color(out.color);
		self->set_border_right_color(out.color);
		self->set_border_bottom_color(out.color);
		self->set_border_left_width(out.width);
		self->set_border_top_width(out.width);
		self->set_border_right_width(out.width);
		self->set_border_bottom_width(out.width);
	});
	def_property_from_type2(border_left, Border, {
		self->set_border_left_color(out.color);
		self->set_border_left_width(out.width);
	});
	def_property_from_type2(border_top, Border, {
		self->set_border_top_color(out.color);
		self->set_border_top_width(out.width);
	});
	def_property_from_type2(border_right, Border, {
		self->set_border_right_color(out.color);
		self->set_border_right_width(out.width);
	});
	def_property_from_type2(border_bottom, Border, {
		self->set_border_bottom_color(out.color);
		self->set_border_bottom_width(out.width);
	});
	def_property_from_type2(border_width, float, {
		self->set_border_left_width(out);
		self->set_border_top_width(out);
		self->set_border_right_width(out);
		self->set_border_bottom_width(out);
	});
	def_property_from_type2(border_color, Color, {
		self->set_border_left_color(out);
		self->set_border_top_color(out);
		self->set_border_right_color(out);
		self->set_border_bottom_color(out);
	});
	def_property_from_type2(border_radius, float, {
		self->set_border_radius_left_top(out);
		self->set_border_radius_right_top(out);
		self->set_border_radius_left_bottom(out);
		self->set_border_radius_right_bottom(out);
	});
	def_property_from_type2(min_width, float, {
		self->set_width(out);
	});
	def_property_from_type2(min_height, float, {
		self->set_height(out);
	});
	def_property_from_type2(start, Vec2, {
		self->set_start_x(out.x());
		self->set_start_y(out.y());
	});
	def_property_from_type2(ratio, Vec2, {
		self->set_ratio_x(out.x());
		self->set_ratio_y(out.y());
	});

	static void binding(JSObject* exports, Worker* worker) {
		worker->bindingModule("_value");
		
		// Js_Define_Class
		Js_Define_Class_NO_EXPORTS(StyleSheets, constructor, {
			// action
			Js_Set_Class_Accessor(time, time, set_time);
			// Meta attribute
			Js_Set_Class_Accessor(x, nullptr, set_x);
			Js_Set_Class_Accessor(y, nullptr, set_y);
			Js_Set_Class_Accessor(scaleX, nullptr, set_scale_x);
			Js_Set_Class_Accessor(scaleY, nullptr, set_scale_y);
			Js_Set_Class_Accessor(skewX, nullptr, set_skew_x);
			Js_Set_Class_Accessor(skewY, nullptr, set_skew_y);
			Js_Set_Class_Accessor(originX, nullptr, set_origin_x);
			Js_Set_Class_Accessor(originY, nullptr, set_origin_y);
			Js_Set_Class_Accessor(rotateZ, nullptr, set_rotate_z);
			Js_Set_Class_Accessor(opacity, nullptr, set_opacity);
			Js_Set_Class_Accessor(visible, nullptr, set_visible);
			Js_Set_Class_Accessor(width, nullptr, set_width);
			Js_Set_Class_Accessor(height, nullptr, set_height);
			Js_Set_Class_Accessor(marginLeft, nullptr, set_margin_left);
			Js_Set_Class_Accessor(marginTop, nullptr, set_margin_top);
			Js_Set_Class_Accessor(marginRight, nullptr, set_margin_right);
			Js_Set_Class_Accessor(marginBottom, nullptr, set_margin_bottom);
			Js_Set_Class_Accessor(borderLeftWidth, nullptr, set_border_left_width);
			Js_Set_Class_Accessor(borderTopWidth, nullptr, set_border_top_width);
			Js_Set_Class_Accessor(borderRightWidth, nullptr, set_border_right_width);
			Js_Set_Class_Accessor(borderBottomWidth, nullptr, set_border_bottom_width);
			Js_Set_Class_Accessor(borderLeftColor, nullptr, set_border_left_color);
			Js_Set_Class_Accessor(borderTopColor, nullptr, set_border_top_color);
			Js_Set_Class_Accessor(borderRightColor, nullptr, set_border_right_color);
			Js_Set_Class_Accessor(borderBottomColor, nullptr, set_border_bottom_color);
			Js_Set_Class_Accessor(borderRadiusLeftTop, nullptr, set_border_radius_left_top);
			Js_Set_Class_Accessor(borderRadiusRightTop, nullptr, set_border_radius_right_top);
			Js_Set_Class_Accessor(borderRadiusRightBottom, nullptr, set_border_radius_right_bottom);
			Js_Set_Class_Accessor(borderRadiusLeftBottom, nullptr, set_border_radius_left_bottom);
			Js_Set_Class_Accessor(backgroundColor, nullptr, set_background_color);
			Js_Set_Class_Accessor(background, nullptr, set_background);
			Js_Set_Class_Accessor(newline, nullptr, set_newline);
			Js_Set_Class_Accessor(clip, nullptr, set_clip);
			Js_Set_Class_Accessor(contentAlign, nullptr, set_content_align);
			Js_Set_Class_Accessor(textAlign, nullptr, set_text_align);
			Js_Set_Class_Accessor(maxWidth, nullptr, set_max_width);
			Js_Set_Class_Accessor(maxHeight, nullptr, set_max_height);
			Js_Set_Class_Accessor(startX, nullptr, set_start_x);
			Js_Set_Class_Accessor(startY, nullptr, set_start_y);
			Js_Set_Class_Accessor(ratioX, nullptr, set_ratio_x);
			Js_Set_Class_Accessor(ratioY, nullptr, set_ratio_y);
			Js_Set_Class_Accessor(repeat, nullptr, set_repeat);
			Js_Set_Class_Accessor(textBackgroundColor, nullptr, set_text_background_color);
			Js_Set_Class_Accessor(textColor, nullptr, set_text_color);
			Js_Set_Class_Accessor(textSize, nullptr, set_text_size);
			Js_Set_Class_Accessor(TextSlant, nullptr, set_text_slant);
			Js_Set_Class_Accessor(textFamily, nullptr, set_text_family);
			Js_Set_Class_Accessor(textLineHeight, nullptr, set_text_line_height);
			Js_Set_Class_Accessor(textShadow, nullptr, set_text_shadow);
			Js_Set_Class_Accessor(textDecoration, nullptr, set_text_decoration);
			Js_Set_Class_Accessor(textOverflow, nullptr, set_text_overflow);
			Js_Set_Class_Accessor(textWhiteSpace, nullptr, set_text_white_space);
			Js_Set_Class_Accessor(alignX, nullptr, set_align_x);
			Js_Set_Class_Accessor(alignY, nullptr, set_align_y);
			Js_Set_Class_Accessor(shadow, nullptr, set_shadow);
			Js_Set_Class_Accessor(src, nullptr, set_src);
			// Non meta attribute
			Js_Set_Class_Accessor(translate, nullptr, set_translate);
			Js_Set_Class_Accessor(scale, nullptr, set_scale);
			Js_Set_Class_Accessor(skew, nullptr, set_skew);
			Js_Set_Class_Accessor(origin, nullptr, set_origin);
			Js_Set_Class_Accessor(margin, nullptr, set_margin);
			Js_Set_Class_Accessor(border, nullptr, set_border);
			Js_Set_Class_Accessor(borderLeft, nullptr, set_border_left);
			Js_Set_Class_Accessor(borderTop, nullptr, set_border_top);
			Js_Set_Class_Accessor(borderRight, nullptr, set_border_right);
			Js_Set_Class_Accessor(borderBottom, nullptr, set_border_bottom);
			Js_Set_Class_Accessor(borderWidth, nullptr, set_border_width);
			Js_Set_Class_Accessor(borderColor, nullptr, set_border_color);
			Js_Set_Class_Accessor(borderRadius, nullptr, set_border_radius);
			Js_Set_Class_Accessor(minWidth, nullptr, set_min_width);
			Js_Set_Class_Accessor(minHeight, nullptr, set_min_height);
			Js_Set_Class_Accessor(start, nullptr, set_start);
			Js_Set_Class_Accessor(ratio, nullptr, set_ratio);
			Js_Set_Class_Accessor(align, nullptr, set_align);
		}, nullptr);
		
		Js_Set_Method(create, create);
	}
};

Js_REG_MODULE(_css, WrapStyleSheets);
Js_END
