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

#include "./_property.h"
#include "./views2/view.h"
#include "./views2/box.h"
#include "./views2/div.h"
#include "./views2/hybrid.h"
#include "./views2/text.h"
#include "./views2/limit.h"
#include "./views2/limit-indep.h"
#include "./views2/indep.h"
#include "./views2/text-node.h"
#include "./views2/label.h"
#include "./views2/image.h"
#include "./views2/sprite.h"
#include "./views2/box-shadow.h"

namespace flare {

	#define set_func(view, cls, Name, func) \
		view.set(Name, Accessor(&cls::func, &cls::set_##func))

	static PropertysAccessor* fx_accessor_shared = nullptr;

	/**
	 * @func shared
	 */
	PropertysAccessor* PropertysAccessor::shared() {
		if ( ! fx_accessor_shared ) {
			fx_accessor_shared = new PropertysAccessor();
		}
		return fx_accessor_shared;
	}

	PropertysAccessor::PropertysAccessor() {
		
		Dict<PropertyName, Accessor> view;
		
		set_func(view, View, PROPERTY_X, x);
		set_func(view, View, PROPERTY_Y, y);
		set_func(view, View, PROPERTY_SCALE_X, scale_x);
		set_func(view, View, PROPERTY_SCALE_Y, scale_y);
		set_func(view, View, PROPERTY_SKEW_X, skew_x);
		set_func(view, View, PROPERTY_SKEW_Y, skew_y);
		set_func(view, View, PROPERTY_ROTATE_Z, rotate_z);
		set_func(view, View, PROPERTY_ORIGIN_X, origin_x);
		set_func(view, View, PROPERTY_ORIGIN_Y, origin_y);
		set_func(view, View, PROPERTY_OPACITY, opacity);
		view[PROPERTY_VISIBLE] = Accessor(&View::visible, &View::set_visible_1);
		
		Dict<PropertyName, Accessor> box = view;
		
		set_func(box, Box, PROPERTY_WIDTH, width);  // Value
		set_func(box, Box, PROPERTY_HEIGHT, height); // Value
		set_func(box, Box, PROPERTY_MARGIN_LEFT, margin_left);
		set_func(box, Box, PROPERTY_MARGIN_TOP, margin_top);
		set_func(box, Box, PROPERTY_MARGIN_RIGHT, margin_right);
		set_func(box, Box, PROPERTY_MARGIN_BOTTOM, margin_bottom);
		set_func(box, Box, PROPERTY_BORDER_LEFT_WIDTH, border_left_width);
		set_func(box, Box, PROPERTY_BORDER_TOP_WIDTH, border_top_width);
		set_func(box, Box, PROPERTY_BORDER_RIGHT_WIDTH, border_right_width);
		set_func(box, Box, PROPERTY_BORDER_BOTTOM_WIDTH, border_bottom_width);
		set_func(box, Box, PROPERTY_BORDER_LEFT_COLOR, border_left_color);
		set_func(box, Box, PROPERTY_BORDER_TOP_COLOR, border_top_color);
		set_func(box, Box, PROPERTY_BORDER_RIGHT_COLOR, border_right_color);
		set_func(box, Box, PROPERTY_BORDER_BOTTOM_COLOR, border_bottom_color);
		set_func(box, Box, PROPERTY_BORDER_RADIUS_LEFT_TOP, border_radius_left_top);
		set_func(box, Box, PROPERTY_BORDER_RADIUS_RIGHT_TOP, border_radius_right_top);
		set_func(box, Box, PROPERTY_BORDER_RADIUS_RIGHT_BOTTOM, border_radius_right_bottom);
		set_func(box, Box, PROPERTY_BORDER_RADIUS_LEFT_BOTTOM, border_radius_left_bottom);
		set_func(box, Box, PROPERTY_BACKGROUND_COLOR, background_color);
		set_func(box, Box, PROPERTY_BACKGROUND, background);
		set_func(box, Box, PROPERTY_NEWLINE, newline);
		set_func(box, Box, PROPERTY_CLIP, clip);

		Dict<PropertyName, Accessor> div = box;
		Dict<PropertyName, Accessor> hybrid = box;
		set_func(div, Div, PROPERTY_CONTENT_ALIGN, content_align);
		set_func(hybrid, Hybrid, PROPERTY_TEXT_ALIGN, text_align);

		_property_func_table[View::VIEW] = view;
		_property_func_table[View::BOX] = box;
		_property_func_table[View::DIV] = div;
		_property_func_table[View::INDEP] = div;
		_property_func_table[View::SCROLL] = div;
		_property_func_table[View::ROOT] = div;
		_property_func_table[View::LIMIT] = div;
		_property_func_table[View::IMAGE] = div;
		_property_func_table[View::BOX_SHADOW] = div;
		_property_func_table[View::PANEL] = div;
		_property_func_table[View::SPAN] = view;
		_property_func_table[View::LABEL] = view;
		_property_func_table[View::SPRITE] = view;
		_property_func_table[View::HYBRID] = hybrid;
		// indep/limit_indep
		set_func(_property_func_table[View::INDEP], Indep, PROPERTY_ALIGN_X, align_x);
		set_func(_property_func_table[View::INDEP], Indep, PROPERTY_ALIGN_Y, align_y);
		_property_func_table[View::LIMIT_INDEP] = _property_func_table[View::INDEP];
		// shadow
		set_func(_property_func_table[View::BOX_SHADOW], BoxShadow, PROPERTY_REPEAT, shadow);
		// limit/limit_indep
		set_func(_property_func_table[View::LIMIT], Limit, PROPERTY_MAX_WIDTH, max_width);
		set_func(_property_func_table[View::LIMIT], Limit, PROPERTY_MAX_HEIGHT, max_height);
		set_func(_property_func_table[View::LIMIT_INDEP], LimitIndep, PROPERTY_MAX_WIDTH, max_width);
		set_func(_property_func_table[View::LIMIT_INDEP], LimitIndep, PROPERTY_MAX_HEIGHT, max_height);
		// video/image
		set_func(_property_func_table[View::IMAGE], Image, PROPERTY_SRC, src);
		_property_func_table[View::VIDEO] = _property_func_table[View::IMAGE];
		// sprite
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_WIDTH, width_1);
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_HEIGHT, height_1);
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_START_X, start_x);
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_START_Y, start_y);
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_RATIO_X, ratio_x);
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_RATIO_Y, ratio_y);
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_REPEAT, repeat);
		set_func(_property_func_table[View::SPRITE], Sprite, PROPERTY_SRC, src);
		// label
		set_func(_property_func_table[View::LABEL], Label, PROPERTY_TEXT_ALIGN, text_align);
		
		// text-font
		Dict<PropertyName, Accessor> font;
		set_func(font, TextFont, PROPERTY_TEXT_BACKGROUND_COLOR, text_background_color);
		set_func(font, TextFont, PROPERTY_TEXT_COLOR, text_color);
		set_func(font, TextFont, PROPERTY_TEXT_SIZE, text_size);
		set_func(font, TextFont, PROPERTY_TEXT_STYLE, text_style);
		set_func(font, TextFont, PROPERTY_TEXT_FAMILY, text_family);
		set_func(font, TextFont, PROPERTY_TEXT_SHADOW, text_shadow);
		set_func(font, TextFont, PROPERTY_TEXT_LINE_HEIGHT, text_line_height);
		set_func(font, TextFont, PROPERTY_TEXT_DECORATION, text_decoration);
		
		for (auto& i : font) { // extend
			_property_func_table[View::LABEL].set(i.key, i.value); // label
		}
		
		// text-layout
		set_func(font, TextLayout, PROPERTY_TEXT_OVERFLOW, text_overflow);
		set_func(font, TextLayout, PROPERTY_TEXT_WHITE_SPACE, text_white_space);
		
		for (auto& i : font) { // extend
			_property_func_table[View::HYBRID].set(i.key, i.value);  // hybrid
			_property_func_table[View::SPAN].set(i.key, i.value);  // span
		}
		
		_property_func_table[View::BUTTON] = _property_func_table[View::HYBRID];
		_property_func_table[View::TEXT] = _property_func_table[View::HYBRID];
		_property_func_table[View::INPUT] = _property_func_table[View::HYBRID];
		_property_func_table[View::TEXTAREA] = _property_func_table[View::HYBRID];
		_property_func_table[View::TEXT_NODE] = _property_func_table[View::SPAN];
	}

	/**
	 * @func accessor
	 */
	PropertysAccessor::Accessor
	PropertysAccessor::accessor(ViewType type, PropertyName name) {
		return _property_func_table[type][name];
	}

	/**
	 * @func has_accessor
	 */
	bool PropertysAccessor::has_accessor(int view_type, PropertyName name) {
		return _property_func_table[view_type].count(name);
	}

}
