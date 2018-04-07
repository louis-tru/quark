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

#include "ngui/js/ngui.h"
#include "ngui/css.h"

/**
 * @ns ngui::js
 */

JS_BEGIN

// ---------------------------------------------------

#define def_get_property(Name) \
static void Name(Local<JSString> name, PropertyCall args) { }
#define def_set_property(Name)\
static void set_##Name(Local<JSString> name, Local<JSValue> value, PropertySetCall args) { \
  JS_WORKER(args); \
  JS_SELF(StyleSheets); \
  if ( ! value->IsNumber(worker) ) { JS_THROW_ERR("Bad argument."); } \
  self->set_##Name( value->ToNumberValue(worker) ); \
}
#define def_property(Name) def_get_property(Name) def_set_property(Name)

// ---------------------------------------------------

#define def_get_property_from_type(Name)\
static void Name(Local<JSString> name, PropertyCall args) { }
#define def_set_property_from_type(Name, Type) def_set_property_from_type2(Name, Type, Type)
#define def_set_property_from_type2(Name, Type, parse_func)\
static void set_##Name(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {\
  JS_WORKER(args);\
  JS_SELF(StyleSheets);\
  js_parse_value2(Type, parse_func, value, "CSS."#Name" = %s");\
  self->set_##Name(out);\
}
#define def_property_from_type(Name, Type) def_property_from_type2(Name, Type, Type)
#define def_property_from_type2(Name, Type, parse_func) \
def_get_property_from_type(Name)\
def_set_property_from_type2(Name, Type, parse_func)

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
  
  // -------------------- get/set property --------------------
  
  def_property_from_type(translate, Vec2);
  def_property_from_type(scale, Vec2);
  def_property_from_type(skew, Vec2);
  def_property_from_type(origin, Vec2);
  def_property_from_type(margin, Value);
  def_property_from_type(border, Border);
  def_property(border_width);
  def_property_from_type(border_color, Color);
  def_property(border_radius);
  def_property_from_type(min_width, Value);
  def_property_from_type(min_height, Value);
  def_property_from_type(start, Vec2);
  def_property_from_type(ratio, Vec2);
  //def_property_from_type(align, Align, align);
  
  // --------------------
  
  static void width(Local<JSString> name, PropertyCall args) { }
  static void height(Local<JSString> name, PropertyCall args) { }
  
  static void set_width(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args);
    JS_SELF(StyleSheets);
    if ( value->IsNumber(worker) ) {
      self->set_width( Value(value->ToNumberValue(worker)) );
      self->set_width2( value->ToNumberValue(worker) );
    } else {
      js_parse_value(Value, value, "CSS.width = %s");
      self->set_width(out);
    }
  }
  static void set_height(Local<JSString> name, Local<JSValue> value, PropertySetCall args) {
    JS_WORKER(args);
    JS_SELF(StyleSheets);
    if ( value->IsNumber(worker) ) {
      self->set_height( Value( value->ToNumberValue(worker) ) );
      self->set_height2( value->ToNumberValue(worker) );
    } else {
      js_parse_value(Value, value, "CSS.height = %s");
      self->set_height(out);
    }
  }
  
  def_property(x);
  def_property(y);
  def_property(scale_x);
  def_property(scale_y);
  def_property(skew_x);
  def_property(skew_y);
  def_property(origin_x);
  def_property(origin_y);
  def_property(rotate_z);
  def_property(opacity);
  def_property_from_type(visible, bool);
  def_property_from_type(margin_left, Value);
  def_property_from_type(margin_top, Value);
  def_property_from_type(margin_right, Value);
  def_property_from_type(margin_bottom, Value);
  def_property_from_type(border_left, Border);
  def_property_from_type(border_top, Border);
  def_property_from_type(border_right, Border);
  def_property_from_type(border_bottom, Border);
  def_property(border_left_width);
  def_property(border_top_width);
  def_property(border_right_width);
  def_property(border_bottom_width);
  def_property_from_type(border_left_color, Color);
  def_property_from_type(border_top_color, Color);
  def_property_from_type(border_right_color, Color);
  def_property_from_type(border_bottom_color, Color);
  def_property(border_radius_left_top);
  def_property(border_radius_right_top);
  def_property(border_radius_right_bottom);
  def_property(border_radius_left_bottom);
  def_property_from_type(background_color, Color);
  def_property_from_type(content_align, ContentAlign);
  def_property_from_type(text_align, TextAlign);
  def_property_from_type(max_width, Value);
  def_property_from_type(max_height, Value);
  def_property(start_x);
  def_property(start_y);
  def_property(ratio_x);
  def_property(ratio_y);
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
  def_property_from_type2(shadow, Shadow, Shadow);
  def_property_from_type(src, String);
  // def_property_from_type(background_image, String);
  
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

public:
  static void binding(Local<JSObject> exports, Worker* worker) {
    worker->binding_module("ngui_value");
    
    // PROPERTY
#define xx_def_property(ENUM, TYPE, NAME) \
JS_SET_PROPERTY(ENUM, ENUM);
    XX_EACH_PROPERTY_TABLE(xx_def_property)
#undef xx_def_property
    
    exports->Delete(worker, worker->New("PROPERTY_WIDTH2",1) );
    exports->Delete(worker, worker->New("PROPERTY_HEIGHT2",1) );
    
    JS_SET_PROPERTY(PROPERTY_TIME, -1);
    JS_SET_PROPERTY(PROPERTY_TRANSLATE, -2);
    JS_SET_PROPERTY(PROPERTY_SCALE, -3);
    JS_SET_PROPERTY(PROPERTY_SKEW, -4);
    JS_SET_PROPERTY(PROPERTY_ORIGIN, -5);
    JS_SET_PROPERTY(PROPERTY_MARGIN, -6);
    JS_SET_PROPERTY(PROPERTY_BORDER, -7);
    JS_SET_PROPERTY(PROPERTY_BORDER_WIDTH, -8);
    JS_SET_PROPERTY(PROPERTY_BORDER_COLOR, -9);
    JS_SET_PROPERTY(PROPERTY_BORDER_RADIUS, -10);
    JS_SET_PROPERTY(PROPERTY_MIN_WIDTH, -11);
    JS_SET_PROPERTY(PROPERTY_MIN_HEIGHT, -12);
    JS_SET_PROPERTY(PROPERTY_START, -13);
    JS_SET_PROPERTY(PROPERTY_RATIO, -14);
    
    JS_DEFINE_CLASS_NO_EXPORTS(StyleSheets, constructor, {
      
      JS_SET_CLASS_ACCESSOR(time, time, set_time);
      //
      JS_SET_CLASS_ACCESSOR(translate, translate, set_translate);
      JS_SET_CLASS_ACCESSOR(scale, scale, set_scale);
      JS_SET_CLASS_ACCESSOR(skew, skew, set_skew);
      JS_SET_CLASS_ACCESSOR(origin, origin, set_origin);
      JS_SET_CLASS_ACCESSOR(margin, margin, set_margin);
      JS_SET_CLASS_ACCESSOR(border, border, set_border);
      JS_SET_CLASS_ACCESSOR(border_width, border_width, set_border_width);
      JS_SET_CLASS_ACCESSOR(borderWidth, border_width, set_border_width);
      JS_SET_CLASS_ACCESSOR(border_color, border_color, set_border_color);
      JS_SET_CLASS_ACCESSOR(borderColor, border_color, set_border_color);
      JS_SET_CLASS_ACCESSOR(border_radius, border_radius, set_border_radius);
      JS_SET_CLASS_ACCESSOR(borderRadius, border_radius, set_border_radius);
      JS_SET_CLASS_ACCESSOR(min_width, min_width, set_min_width);
      JS_SET_CLASS_ACCESSOR(minWidth, min_width, set_min_width);
      JS_SET_CLASS_ACCESSOR(min_height, min_height, set_min_height);
      JS_SET_CLASS_ACCESSOR(minHeight, min_height, set_min_height);
      JS_SET_CLASS_ACCESSOR(start, start, set_start);
      JS_SET_CLASS_ACCESSOR(ratio, ratio, set_ratio);
      // JS_SET_CLASS_ACCESSOR(align, align, set_align);
      //
      JS_SET_CLASS_ACCESSOR(x, x, set_x);
      JS_SET_CLASS_ACCESSOR(y, y, set_y);
      JS_SET_CLASS_ACCESSOR(scale_x, scale_x, set_scale_x);
      JS_SET_CLASS_ACCESSOR(scaleX, scale_x, set_scale_x);
      JS_SET_CLASS_ACCESSOR(scale_y, scale_y, set_scale_y);
      JS_SET_CLASS_ACCESSOR(scaleY, scale_y, set_scale_y);
      JS_SET_CLASS_ACCESSOR(skew_x, skew_x, set_skew_x);
      JS_SET_CLASS_ACCESSOR(skewX, skew_x, set_skew_x);
      JS_SET_CLASS_ACCESSOR(skew_y, skew_y, set_skew_y);
      JS_SET_CLASS_ACCESSOR(skewY, skew_y, set_skew_y);
      JS_SET_CLASS_ACCESSOR(origin_x, origin_x, set_origin_x);
      JS_SET_CLASS_ACCESSOR(originX, origin_x, set_origin_x);
      JS_SET_CLASS_ACCESSOR(origin_y, origin_y, set_origin_y);
      JS_SET_CLASS_ACCESSOR(originY, origin_y, set_origin_y);
      JS_SET_CLASS_ACCESSOR(rotate_z, rotate_z, set_rotate_z);
      JS_SET_CLASS_ACCESSOR(rotateZ, rotate_z, set_rotate_z);
      JS_SET_CLASS_ACCESSOR(opacity, opacity, set_opacity);
      JS_SET_CLASS_ACCESSOR(visible, visible, set_visible);
      JS_SET_CLASS_ACCESSOR(width, width, set_width);
      JS_SET_CLASS_ACCESSOR(height, height, set_height);
      JS_SET_CLASS_ACCESSOR(margin_left, margin_left, set_margin_left);
      JS_SET_CLASS_ACCESSOR(marginLeft, margin_left, set_margin_left);
      JS_SET_CLASS_ACCESSOR(margin_top, margin_top, set_margin_top);
      JS_SET_CLASS_ACCESSOR(margin_right, margin_right, set_margin_right);
      JS_SET_CLASS_ACCESSOR(marginRight, margin_right, set_margin_right);
      JS_SET_CLASS_ACCESSOR(margin_bottom, margin_bottom, set_margin_bottom);
      JS_SET_CLASS_ACCESSOR(marginBottom, margin_bottom, set_margin_bottom);
      JS_SET_CLASS_ACCESSOR(border_left, border_left, set_border_left);
      JS_SET_CLASS_ACCESSOR(borderLeft, border_left, set_border_left);
      JS_SET_CLASS_ACCESSOR(border_top, border_top, set_border_top);
      JS_SET_CLASS_ACCESSOR(borderTop, border_top, set_border_top);
      JS_SET_CLASS_ACCESSOR(border_right, border_right, set_border_right);
      JS_SET_CLASS_ACCESSOR(borderRight, border_right, set_border_right);
      JS_SET_CLASS_ACCESSOR(border_bottom, border_bottom, set_border_bottom);
      JS_SET_CLASS_ACCESSOR(borderBottom, border_bottom, set_border_bottom);
      JS_SET_CLASS_ACCESSOR(border_left_width, border_left_width, set_border_left_width);
      JS_SET_CLASS_ACCESSOR(borderLeftWidth, border_left_width, set_border_left_width);
      JS_SET_CLASS_ACCESSOR(border_top_width, border_top_width, set_border_top_width);
      JS_SET_CLASS_ACCESSOR(borderTopWidth, border_top_width, set_border_top_width);
      JS_SET_CLASS_ACCESSOR(border_right_width, border_right_width, set_border_right_width);
      JS_SET_CLASS_ACCESSOR(borderRightWidth, border_right_width, set_border_right_width);
      JS_SET_CLASS_ACCESSOR(border_bottom_width, border_bottom_width, set_border_bottom_width);
      JS_SET_CLASS_ACCESSOR(borderBottomWidth, border_bottom_width, set_border_bottom_width);
      JS_SET_CLASS_ACCESSOR(border_left_color, border_left_color, set_border_left_color);
      JS_SET_CLASS_ACCESSOR(borderLeftColor, border_left_color, set_border_left_color);
      JS_SET_CLASS_ACCESSOR(border_top_color, border_top_color, set_border_top_color);
      JS_SET_CLASS_ACCESSOR(borderTopColor, border_top_color, set_border_top_color);
      JS_SET_CLASS_ACCESSOR(border_right_color, border_right_color, set_border_right_color);
      JS_SET_CLASS_ACCESSOR(borderRightColor, border_right_color, set_border_right_color);
      JS_SET_CLASS_ACCESSOR(border_bottom_color, border_bottom_color, set_border_bottom_color);
      JS_SET_CLASS_ACCESSOR(borderBottomColor, border_bottom_color, set_border_bottom_color);
      JS_SET_CLASS_ACCESSOR(border_radius_left_top, border_radius_left_top, set_border_radius_left_top);
      JS_SET_CLASS_ACCESSOR(borderRadiusLeftTop, border_radius_left_top, set_border_radius_left_top);
      JS_SET_CLASS_ACCESSOR(border_radius_right_top, border_radius_right_top, set_border_radius_right_top);
      JS_SET_CLASS_ACCESSOR(borderRadiusRightTop, border_radius_right_top, set_border_radius_right_top);
      JS_SET_CLASS_ACCESSOR(border_radius_right_bottom, border_radius_right_bottom, set_border_radius_right_bottom);
      JS_SET_CLASS_ACCESSOR(borderRadiusRightBottom, border_radius_right_bottom, set_border_radius_right_bottom);
      JS_SET_CLASS_ACCESSOR(border_radius_left_bottom, border_radius_left_bottom, set_border_radius_left_bottom);
      JS_SET_CLASS_ACCESSOR(borderRadiusLeftBottom, border_radius_left_bottom, set_border_radius_left_bottom);
      JS_SET_CLASS_ACCESSOR(background_color, background_color, set_background_color);
      JS_SET_CLASS_ACCESSOR(backgroundColor, background_color, set_background_color);
      JS_SET_CLASS_ACCESSOR(newline, newline, set_newline);
      JS_SET_CLASS_ACCESSOR(clip, clip, set_clip);
      JS_SET_CLASS_ACCESSOR(content_align, content_align, set_content_align);
      JS_SET_CLASS_ACCESSOR(contentAlign, content_align, set_content_align);
      JS_SET_CLASS_ACCESSOR(text_align, text_align, set_text_align);
      JS_SET_CLASS_ACCESSOR(textAlign, text_align, set_text_align);
      JS_SET_CLASS_ACCESSOR(max_width, max_width, set_max_width);
      JS_SET_CLASS_ACCESSOR(maxWidth, max_width, set_max_width);
      JS_SET_CLASS_ACCESSOR(max_height, max_height, set_max_height);
      JS_SET_CLASS_ACCESSOR(maxHeight, max_height, set_max_height);
      JS_SET_CLASS_ACCESSOR(start_x, start_x, set_start_x);
      JS_SET_CLASS_ACCESSOR(startX, start_x, set_start_x);
      JS_SET_CLASS_ACCESSOR(start_y, start_y, set_start_y);
      JS_SET_CLASS_ACCESSOR(startY, start_y, set_start_y);
      JS_SET_CLASS_ACCESSOR(ratio_x, ratio_x, set_ratio_x);
      JS_SET_CLASS_ACCESSOR(ratioX, ratio_x, set_ratio_x);
      JS_SET_CLASS_ACCESSOR(ratio_y, ratio_y, set_ratio_y);
      JS_SET_CLASS_ACCESSOR(ratioY, ratio_y, set_ratio_y);
      JS_SET_CLASS_ACCESSOR(repeat, repeat, set_repeat);
      JS_SET_CLASS_ACCESSOR(text_background_color, text_background_color, set_text_background_color);
      JS_SET_CLASS_ACCESSOR(textBackgroundColor, text_background_color, set_text_background_color);
      JS_SET_CLASS_ACCESSOR(text_color, text_color, set_text_color);
      JS_SET_CLASS_ACCESSOR(textColor, text_color, set_text_color);
      JS_SET_CLASS_ACCESSOR(text_size, text_size, set_text_size);
      JS_SET_CLASS_ACCESSOR(textSize, text_size, set_text_size);
      JS_SET_CLASS_ACCESSOR(text_style, text_style, set_text_style);
      JS_SET_CLASS_ACCESSOR(textStyle, text_style, set_text_style);
      JS_SET_CLASS_ACCESSOR(text_family, text_family, set_text_family);
      JS_SET_CLASS_ACCESSOR(textFamily, text_family, set_text_family);
      JS_SET_CLASS_ACCESSOR(text_line_height, text_line_height, set_text_line_height);
      JS_SET_CLASS_ACCESSOR(textLineHeight, text_line_height, set_text_line_height);
      JS_SET_CLASS_ACCESSOR(text_shadow, text_shadow, set_text_shadow);
      JS_SET_CLASS_ACCESSOR(textShadow, text_shadow, set_text_shadow);
      JS_SET_CLASS_ACCESSOR(text_decoration, text_decoration, set_text_decoration);
      JS_SET_CLASS_ACCESSOR(textDecoration, text_decoration, set_text_decoration);
      JS_SET_CLASS_ACCESSOR(text_overflow, text_overflow, set_text_overflow);
      JS_SET_CLASS_ACCESSOR(textOverflow, text_overflow, set_text_overflow);
      JS_SET_CLASS_ACCESSOR(text_white_space, text_white_space, set_text_white_space);
      JS_SET_CLASS_ACCESSOR(textWhiteSpace, text_white_space, set_text_white_space);
      JS_SET_CLASS_ACCESSOR(align_x, align_x, set_align_x);
      JS_SET_CLASS_ACCESSOR(alignX, align_x, set_align_x);
      JS_SET_CLASS_ACCESSOR(align_y, align_y, set_align_y);
      JS_SET_CLASS_ACCESSOR(alignY, align_y, set_align_y);
      JS_SET_CLASS_ACCESSOR(shadow, shadow, set_shadow);
      JS_SET_CLASS_ACCESSOR(src, src, set_src);
    }, nullptr);
    
    JS_SET_METHOD(create, create);
  }
};

JS_REG_MODULE(ngui_css, WrapStyleSheets);
JS_END
