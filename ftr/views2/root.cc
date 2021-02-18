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

#include "app-1.h"
#include "display-port.h"
#include "root.h"
#include "draw.h"

namespace ftr {

/**
 * @destructor
 */
Root::~Root() {
	FX_DEBUG("destructor root");
}

/**
 * @func initialize()
 */
void Root::initialize() throw(Error) {
	auto app = ftr::app();
	FX_CHECK(app, "Before you create a root, you need to create a GUIApplication");
	_background_color = Color(255, 255, 255); // 默认白色背景
	_level = 1; // 根视图为1
	_final_visible = true;
	_explicit_width = true;
	_explicit_height = true;
	// _receive = true;
	Vec2 size = display_port()->size();
	set_width(size.width());
	set_height(size.height());
	mark(M_MATRIX);
	Inl_GUIApplication(app)->set_root(this);
}

/**
 * @overwrite
 */
void Root::draw(Draw* draw) {
	
	if ( _visible ) {
		
		if ( mark_value ) {
			if ( mark_value & M_BASIC_MATRIX ) { // 变换
				Vec2 offset = layout_offset(); // xy 位移
				
				offset.x( offset.x() + origin().x() + translate().x() );
				offset.y( offset.y() + origin().y() + translate().y() );
				// 更新基础矩阵
				_matrix = Mat(offset, scale(), -rotate_z(), skew());
			}
			
			if ( mark_value & M_OPACITY ) {
				_final_opacity = opacity();
			}
			if ( mark_value & M_TRANSFORM ) {
				_final_matrix = _matrix;
			}
			
			if ( mark_value & (M_TRANSFORM | M_SHAPE) ) {
				set_draw_visible();
			}
		}
		
		draw->clear_color(_background_color);
		draw->draw(this);
		
		mark_value = M_NONE;
	} else {
		draw->clear_color(Color(0, 0, 0));
	}
}

/**
 * @overwrite
 */
void Root::set_parent(View* parent) throw(Error) {
	FX_UNREACHABLE();
}

/**
 * @overwrite
 */
bool Root::can_become_focus() {
	return true;
}

/**
 * @overwrite
 */
void Root::set_layout_explicit_size() {
	
	float final_width = _final_width;
	float final_height = _final_height;
	
	Vec2 port_size = display_port()->size();

	if ( _width.type == ValueType::PIXEL ) {
		_final_width = _width.value;
	} else {
		_final_width = port_size.width();
	}

	if ( _height.type == ValueType::PIXEL ) {
		_final_height = _height.value;
	} else {
		_final_height = port_size.height();
	}
	
	_raw_client_width = _final_width;
	_raw_client_height = _final_height;
	_limit.width(_final_width);
	_limit.height(_final_height);
	
	Box::set_default_offset_value();
	
	bool h = _final_width != final_width;
	bool v = _final_height != final_height;
	uint32_t child_mark = M_NONE;
	
	if ( h ) {
		if ( _content_align == ContentAlign::RIGHT ) {
			child_mark = M_MATRIX;
		}
	}
	if ( v ) {
		if ( _content_align == ContentAlign::BOTTOM ) {
			child_mark = M_MATRIX;
		}
	}
	
	solve_explicit_size_after(h, v, child_mark);
}

/**
 * @overwrite
 */
void Root::set_layout_content_offset() {
	Vec2 squeeze;
	Div::set_div_content_offset(squeeze, Vec2());
}

/**
 * @overwrite
 */
Vec2 Root::layout_offset() {
	return Vec2(_offset_start.x() + _final_margin_left + _border_left_width,
							_offset_start.y() + _final_margin_top + _border_top_width);
}

}
