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

#include "indep.h"
#include "hybrid.h"
#include "limit-indep.h"

namespace ftr {

FX_DEFINE_INLINE_MEMBERS(Indep, Inl) {
	public:
	#define _inl(self) static_cast<Indep::Inl*>(static_cast<Indep*>(self))
	
	/**
	 * @func set_indep_offset
	 */
	void set_indep_offset(Box* parent) { ASSERT(parent);
		
		if (_align_x == LayoutAlign::RIGHT) { // 水平右对齐
			_offset_start.x(parent->_final_width - _raw_client_width);
			_offset_end.x(parent->_final_width);
		} else if (_align_x == LayoutAlign::CENTER) { // 水平居中
			_offset_start.x(-_raw_client_width / 2 + parent->_final_width / 2);
			_offset_end.x(_raw_client_width / 2 + parent->_final_width / 2);
		} else { // 水平左对齐
			_offset_start.x(0);
			_offset_end.x(_raw_client_width);
		}
		
		if (_align_y == LayoutAlign::BOTTOM) { // 垂直底部对齐
			_offset_start.y(parent->_final_height - _raw_client_height);
			_offset_end.y(parent->_final_height);
		} else if (_align_y == LayoutAlign::CENTER) { // 垂直居中
			_offset_start.y(-_raw_client_height / 2 + parent->_final_height / 2);
			_offset_end.y(_raw_client_height / 2 + parent->_final_height / 2);
		} else { // 垂直顶部对齐
			_offset_start.y(0);
			_offset_end.y(_raw_client_height);
		}
		
		mark(M_MATRIX); // mark transform
	}
};

Indep::Indep(): _align_x(Align::LEFT), _align_y(Align::TOP) { }

void Indep::set_align_x(Align value) {
	if (value == LayoutAlign::LEFT || value == LayoutAlign::RIGHT || value == LayoutAlign::CENTER) {
		_align_x = value;
		mark_pre(M_MATRIX | M_LAYOUT | M_SIZE_HORIZONTAL);
	}
}

void Indep::set_align_y(Align value) {
	if (value == LayoutAlign::TOP || value == LayoutAlign::BOTTOM || value == LayoutAlign::CENTER) {
		_align_y = value;
		mark_pre(M_MATRIX | M_LAYOUT | M_SIZE_VERTICAL);
	}
}

void Indep::set_parent(View* parent) throw(Error) {
	if (parent != View::parent()) {
		View::set_parent(parent);
		mark_pre(M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL);
	}
}

void Indep::set_layout_content_offset() {
	
	if (_final_visible) {
		Vec2 squeeze;
		if (  set_div_content_offset(squeeze, Vec2()) ) { //
			// 高度或宽度被挤压即形状发生变化
			mark(M_SHAPE);
			
			Box* box = parent()->as_box();
			if ( box ) {
				_parent_layout = box;
				mark_pre(M_LAYOUT_THREE_TIMES);
			} else { // 父视图只是个普通视图,默认将偏移设置为0
				set_default_offset_value();
			}
		}
	}
}

Box* Indep::set_offset_horizontal(Box* prev, Vec2& squeeze, float limit, Div* div) {
	if ( _visible ) {
		_parent_layout = div; ASSERT(div);
		mark_pre(M_LAYOUT_THREE_TIMES);
	} else {
		set_default_offset_value();
	}
	return prev;
}

Box* Indep::set_offset_vertical(Box* prev, Vec2& squeeze, float limit, Div* div) {
	if ( _visible ) {
		_parent_layout = div; ASSERT(div);
		mark_pre(M_LAYOUT_THREE_TIMES);
	} else {
		set_default_offset_value();
	}
	return prev;
}

void Indep::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
	if ( _visible ) {
		if ( hybrid ) {
			_parent_layout = hybrid;
			mark_pre(M_LAYOUT_THREE_TIMES);
		}
	} else {
		set_default_offset_value();
	}
}


void Indep::set_layout_three_times(bool horizontal, bool hybrid) {
	if ( _visible ) {
		// Div::set_layout_three_times(horizontal, false);
		_inl(this)->set_indep_offset(static_cast<Box*>(_parent_layout));
	}
}

void LimitIndep::set_layout_three_times(bool horizontal, bool hybrid) {
	if ( _visible ) {
		_inl(this)->set_indep_offset(static_cast<Box*>(_parent_layout));
	}
}

Vec2 Indep::layout_offset() {
	return Vec2( _offset_start.x() + _final_margin_left + _border_left_width,
							 _offset_start.y() + _final_margin_top + _border_top_width);
}

}
