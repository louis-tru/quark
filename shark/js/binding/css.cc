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

#include "shark/js/shark.h"
#include "shark/css.h"

/**
 * @ns shark::js
 */

JS_BEGIN

#define def_get_property_from_type0(Name) \
	/*static void Name(Local<JSString> name, PropertyCall args) {}*/
#define def_set_property_from_type0(Name, Type, Parser, block)\
static void set_##Name(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {\
	JS_WORKER(args);\
	JS_SELF(StyleSheets);\
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
	
	static void constructor(FunctionCall args) {
		JS_ATTACH(args);
		JS_WORKER(args);
		JS_THROW_ERR("Forbidden access abstract");
	}
	
	static void create(FunctionCall args) {
		JS_WORKER(args); GUILock lock;
		
		if ( args.Length() < 1 || !args[0]->IsObject(worker) || args[0]->IsNull(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		
		JS_HANDLE_SCOPE();
		
		Local<JSObject> arg = args[0].To<JSObject>();
		Local<JSArray> names = arg->GetPropertyNames(worker);
		
		for ( uint i = 0, len = names->Length(worker); i < len; i++ ) {
			Local<JSValue> key = names->Get(worker, i);
			Local<JSValue> val = arg->Get(worker, key);
			if ( val.IsEmpty() ) {
				return; // js error
			}
			if ( val.IsEmpty() || ! val->IsObject(worker) ) {
				JS_THROW_ERR("Bad argument.");
			}
			
			Array<StyleSheets*> arr =
			root_styles()->instances( key->ToStringValue(worker) ); // new instances
			
			if ( arr.length() ) {
				Local<JSObject> propertys = val.To<JSObject>();
				Local<JSArray> names = propertys->GetPropertyNames(worker);
				
				for ( uint j = 0, len = names->Length(worker); j < len; j++ ) {
					Local<JSValue> key;
					key = names->Get(worker, j);
					val = propertys->Get(worker, key);
					if ( val.IsEmpty() ) {
						return; // js error
					}
					for ( auto& i : arr ) {
						StyleSheets* ss = i.value();
						Local<JSObject> local = Wrap<StyleSheets>::pack(ss)->that();
						if ( !local->Set(worker, key, val) ) {
							return; // js error
						}
					}
				}
				// if (arr.length)
			}
		}
	}
	
	static void time(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_SELF(StyleSheets);
		JS_RETURN( self->time() / 1000 );
	}
	
	static void set_time(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
		JS_WORKER(args);
		if ( ! value->IsNumber(worker) ) {
			JS_THROW_ERR("Bad argument.");
		}
		JS_SELF(StyleSheets);
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
	def_property_from_type(text_style, TextStyle);
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
	
	inline static BackgroundImage* as_background_image(StyleSheets* self) {
		return self->background()->as_image();
	}
	
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
	def_property_from_type3(background_image, BackgroundPtr, BackgroundImage, {
		out->set_holder_mode(Background::M_DISABLE); // 禁止被持有
		self->set_background(out);
	});
	
#define set_background_attrs(block) { \
	auto bg = as_background_image(self);\
	int i = 0;\
	while(bg) {\
		block; \
		i++;\
		bg = bg->next() ? bg->next()->as_image(): nullptr; \
	} \
}
	def_property_from_type3(background_repeat, Array<Repeat>, Repeats, {
		set_background_attrs({
			bg->set_repeat(out[i]);
		});
	});
	def_property_from_type3(background_position,
													Array<BackgroundPositionCollection>, BackgroundPositions, {
		set_background_attrs({
			bg->set_position_x(out[i].x);
			bg->set_position_y(out[i].y);
		});
	});
	def_property_from_type3(background_position_x,
													Array<BackgroundPositionCollection>, BackgroundPositions, {
		set_background_attrs({
			bg->set_position_x(out[i].x);
		});
	});
	def_property_from_type3(background_position_y,
													Array<BackgroundPositionCollection>, BackgroundPositions, {
		set_background_attrs({
			bg->set_position_y(out[i].x);
		});
	});
	def_property_from_type3(background_size, Array<BackgroundSizeCollection>, BackgroundSizes, {
		set_background_attrs({
			bg->set_size_x(out[i].x);
			bg->set_size_y(out[i].y);
		});
	});
	def_property_from_type3(background_size_x, Array<BackgroundSizeCollection>, BackgroundSizes, {
		set_background_attrs({
			bg->set_size_x(out[i].x);
		});
	});
	def_property_from_type3(background_size_y, Array<BackgroundSizeCollection>, BackgroundSizes, {
		set_background_attrs({
			bg->set_size_y(out[i].x);
		});
	});
	
public:
	static void binding(Local<JSObject> exports, Worker* worker) {
		worker->binding_module("shark_value");
		
		// PROPERTY
# define xx_def_property(ENUM, TYPE, NAME) \
	JS_SET_PROPERTY(ENUM, ENUM);
		XX_EACH_PROPERTY_TABLE(xx_def_property)
# undef xx_def_property
		
		JS_SET_PROPERTY(PROPERTY_TIME, -1);
		JS_SET_PROPERTY(PROPERTY_TRANSLATE, -2);
		JS_SET_PROPERTY(PROPERTY_SCALE, -3);
		JS_SET_PROPERTY(PROPERTY_SKEW, -4);
		JS_SET_PROPERTY(PROPERTY_ORIGIN, -5);
		JS_SET_PROPERTY(PROPERTY_MARGIN, -6);
		JS_SET_PROPERTY(PROPERTY_BORDER, -7);
		JS_SET_PROPERTY(PROPERTY_BORDER_WIDTH, -8);
		JS_SET_PROPERTY(PROPERTY_BORDER_COLOR, -9);
		JS_SET_PROPERTY(ROPERTY_BORDER_RADIUS, -10);
		JS_SET_PROPERTY(PROPERTY_BORDER_LEFT, -11);
		JS_SET_PROPERTY(PROPERTY_BORDER_TOP, -12);
		JS_SET_PROPERTY(PROPERTY_BORDER_RIGHT, -13);
		JS_SET_PROPERTY(PROPERTY_BORDER_BOTTOM, -14);
		JS_SET_PROPERTY(PROPERTY_BORDER_RADIUS, -15);
		JS_SET_PROPERTY(PROPERTY_MIN_WIDTH, -16);
		JS_SET_PROPERTY(PROPERTY_MIN_HEIGHT, -17);
		JS_SET_PROPERTY(PROPERTY_START, -18);
		JS_SET_PROPERTY(PROPERTY_RATIO, -19);
		JS_SET_PROPERTY(PROPERTY_BACKGROUND_IMAGE, -20);
		JS_SET_PROPERTY(PROPERTY_BACKGROUND_REPEAT, -21);
		JS_SET_PROPERTY(PROPERTY_BACKGROUND_POSITION, -22);
		JS_SET_PROPERTY(PROPERTY_BACKGROUND_POSITION_X, -23);
		JS_SET_PROPERTY(PROPERTY_BACKGROUND_POSITION_Y, -24);
		JS_SET_PROPERTY(PROPERTY_BACKGROUND_SIZE, -25);
		JS_SET_PROPERTY(PROPERTY_BACKGROUND_SIZE_X, -26);
		JS_SET_PROPERTY(PROPERTY_ALIGN, -27);
		
		JS_DEFINE_CLASS(StyleSheets, constructor, {
			JS_SET_CLASS_ACCESSOR(time, time, set_time);
			// Meta attribute
			JS_SET_CLASS_ACCESSOR(x, nullptr, set_x);
			JS_SET_CLASS_ACCESSOR(y, nullptr, set_y);
			JS_SET_CLASS_ACCESSOR(scaleX, nullptr, set_scale_x);
			JS_SET_CLASS_ACCESSOR(scaleY, nullptr, set_scale_y);
			JS_SET_CLASS_ACCESSOR(skewX, nullptr, set_skew_x);
			JS_SET_CLASS_ACCESSOR(skewY, nullptr, set_skew_y);
			JS_SET_CLASS_ACCESSOR(originX, nullptr, set_origin_x);
			JS_SET_CLASS_ACCESSOR(originY, nullptr, set_origin_y);
			JS_SET_CLASS_ACCESSOR(rotateZ, nullptr, set_rotate_z);
			JS_SET_CLASS_ACCESSOR(opacity, nullptr, set_opacity);
			JS_SET_CLASS_ACCESSOR(visible, nullptr, set_visible);
			JS_SET_CLASS_ACCESSOR(width, nullptr, set_width);
			JS_SET_CLASS_ACCESSOR(height, nullptr, set_height);
			JS_SET_CLASS_ACCESSOR(marginLeft, nullptr, set_margin_left);
			JS_SET_CLASS_ACCESSOR(marginTop, nullptr, set_margin_top);
			JS_SET_CLASS_ACCESSOR(marginRight, nullptr, set_margin_right);
			JS_SET_CLASS_ACCESSOR(marginBottom, nullptr, set_margin_bottom);
			JS_SET_CLASS_ACCESSOR(borderLeftWidth, nullptr, set_border_left_width);
			JS_SET_CLASS_ACCESSOR(borderTopWidth, nullptr, set_border_top_width);
			JS_SET_CLASS_ACCESSOR(borderRightWidth, nullptr, set_border_right_width);
			JS_SET_CLASS_ACCESSOR(borderBottomWidth, nullptr, set_border_bottom_width);
			JS_SET_CLASS_ACCESSOR(borderLeftColor, nullptr, set_border_left_color);
			JS_SET_CLASS_ACCESSOR(borderTopColor, nullptr, set_border_top_color);
			JS_SET_CLASS_ACCESSOR(borderRightColor, nullptr, set_border_right_color);
			JS_SET_CLASS_ACCESSOR(borderBottomColor, nullptr, set_border_bottom_color);
			JS_SET_CLASS_ACCESSOR(borderRadiusLeftTop, nullptr, set_border_radius_left_top);
			JS_SET_CLASS_ACCESSOR(borderRadiusRightTop, nullptr, set_border_radius_right_top);
			JS_SET_CLASS_ACCESSOR(borderRadiusRightBottom, nullptr, set_border_radius_right_bottom);
			JS_SET_CLASS_ACCESSOR(borderRadiusLeftBottom, nullptr, set_border_radius_left_bottom);
			JS_SET_CLASS_ACCESSOR(backgroundColor, nullptr, set_background_color);
			JS_SET_CLASS_ACCESSOR(background, nullptr, set_background);
			JS_SET_CLASS_ACCESSOR(newline, nullptr, set_newline);
			JS_SET_CLASS_ACCESSOR(clip, nullptr, set_clip);
			JS_SET_CLASS_ACCESSOR(contentAlign, nullptr, set_content_align);
			JS_SET_CLASS_ACCESSOR(textAlign, nullptr, set_text_align);
			JS_SET_CLASS_ACCESSOR(maxWidth, nullptr, set_max_width);
			JS_SET_CLASS_ACCESSOR(maxHeight, nullptr, set_max_height);
			JS_SET_CLASS_ACCESSOR(startX, nullptr, set_start_x);
			JS_SET_CLASS_ACCESSOR(startY, nullptr, set_start_y);
			JS_SET_CLASS_ACCESSOR(ratioX, nullptr, set_ratio_x);
			JS_SET_CLASS_ACCESSOR(ratioY, nullptr, set_ratio_y);
			JS_SET_CLASS_ACCESSOR(repeat, nullptr, set_repeat);
			JS_SET_CLASS_ACCESSOR(textBackgroundColor, nullptr, set_text_background_color);
			JS_SET_CLASS_ACCESSOR(textColor, nullptr, set_text_color);
			JS_SET_CLASS_ACCESSOR(textSize, nullptr, set_text_size);
			JS_SET_CLASS_ACCESSOR(textStyle, nullptr, set_text_style);
			JS_SET_CLASS_ACCESSOR(textFamily, nullptr, set_text_family);
			JS_SET_CLASS_ACCESSOR(textLineHeight, nullptr, set_text_line_height);
			JS_SET_CLASS_ACCESSOR(textShadow, nullptr, set_text_shadow);
			JS_SET_CLASS_ACCESSOR(textDecoration, nullptr, set_text_decoration);
			JS_SET_CLASS_ACCESSOR(textOverflow, nullptr, set_text_overflow);
			JS_SET_CLASS_ACCESSOR(textWhiteSpace, nullptr, set_text_white_space);
			JS_SET_CLASS_ACCESSOR(alignX, nullptr, set_align_x);
			JS_SET_CLASS_ACCESSOR(alignY, nullptr, set_align_y);
			JS_SET_CLASS_ACCESSOR(shadow, nullptr, set_shadow);
			JS_SET_CLASS_ACCESSOR(src, nullptr, set_src);
			// Non meta attribute
			JS_SET_CLASS_ACCESSOR(translate, nullptr, set_translate);
			JS_SET_CLASS_ACCESSOR(scale, nullptr, set_scale);
			JS_SET_CLASS_ACCESSOR(skew, nullptr, set_skew);
			JS_SET_CLASS_ACCESSOR(origin, nullptr, set_origin);
			JS_SET_CLASS_ACCESSOR(margin, nullptr, set_margin);
			JS_SET_CLASS_ACCESSOR(border, nullptr, set_border);
			JS_SET_CLASS_ACCESSOR(borderLeft, nullptr, set_border_left);
			JS_SET_CLASS_ACCESSOR(borderTop, nullptr, set_border_top);
			JS_SET_CLASS_ACCESSOR(borderRight, nullptr, set_border_right);
			JS_SET_CLASS_ACCESSOR(borderBottom, nullptr, set_border_bottom);
			JS_SET_CLASS_ACCESSOR(borderWidth, nullptr, set_border_width);
			JS_SET_CLASS_ACCESSOR(borderColor, nullptr, set_border_color);
			JS_SET_CLASS_ACCESSOR(borderRadius, nullptr, set_border_radius);
			JS_SET_CLASS_ACCESSOR(minWidth, nullptr, set_min_width);
			JS_SET_CLASS_ACCESSOR(minHeight, nullptr, set_min_height);
			JS_SET_CLASS_ACCESSOR(start, nullptr, set_start);
			JS_SET_CLASS_ACCESSOR(ratio, nullptr, set_ratio);
			JS_SET_CLASS_ACCESSOR(align, nullptr, set_align);
			JS_SET_CLASS_ACCESSOR(backgroundImage, nullptr, set_background_image);
			JS_SET_CLASS_ACCESSOR(backgroundRepeat, nullptr, set_background_repeat);
			JS_SET_CLASS_ACCESSOR(backgroundPosition, nullptr, set_background_position);
			JS_SET_CLASS_ACCESSOR(backgroundPositionX, nullptr, set_background_position_x);
			JS_SET_CLASS_ACCESSOR(backgroundPositionY, nullptr, set_background_position_y);
			JS_SET_CLASS_ACCESSOR(backgroundSize, nullptr, set_background_size);
			JS_SET_CLASS_ACCESSOR(backgroundSizeX, nullptr, set_background_size_x);
			JS_SET_CLASS_ACCESSOR(backgroundSizeY, nullptr, set_background_size_y);
		}, nullptr);
		
		JS_SET_METHOD(create, create);
	}
};

JS_REG_MODULE(shark_css, WrapStyleSheets);
JS_END
