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

#include "limit.h"
#include "limit-indep.h"

FX_NS(ftr)

void _box_inl__solve_horizontal_size_with_auto_width(Box* box, float parent);
void _box_inl__solve_horizontal_size_with_full_width(Box* box, float parent);
void _box_inl__solve_horizontal_size_with_explicit_width(Box* box, float parent);
void _box_inl__solve_vertical_size_with_auto_height(Box* box, float parent);
void _box_inl__solve_vertical_size_with_full_height(Box* box, float parent);
void _box_inl__solve_vertical_size_with_explicit_height(Box* box, float parent);
void _box_inl__solve_final_horizontal_size_with_full_width(Box* box, float parent);
void _box_inl__solve_final_vertical_size_with_full_height(Box* box, float parent);

/**
 * @class Limit::Inl
 */
template<class T> class Limit::Inl: public T {
public:
#define _inl_limit(self) static_cast<Limit::Inl<Limit>*>(self)
#define _inl_limit_indep(self) static_cast<Limit::Inl<LimitIndep>*>(self)
	
	/**
	 * @func solve_explicit_size
	 */
	bool solve_explicit_size() {
		
		bool h_change = false;
		bool v_change = false;
		bool change = false;
		uint child_mark = M_NONE;
		
		if ( this->mark_value & Layout::M_SIZE_HORIZONTAL ) {
			float limit_min_width = this->m_limit_min_width;
			float limit_max_width = this->m_limit.width();
			
			solve_explicit_horizontal_size();
			// 限制宽度变化可能对子视图造成影响
			if (limit_min_width != this->m_limit_min_width ||
					limit_max_width != this->m_limit.width() ) {
				if ( this->m_content_align == ContentAlign::RIGHT ) { 
					// 右对齐宽度变化一定会影响到子盒子的最终位置偏移
					child_mark = M_MATRIX;
				}
				h_change = true;
			}
			change = true;
		}
		
		if ( this->mark_value & Layout::M_SIZE_VERTICAL ) {
			float limit_min_height = this->m_limit_min_height;
			float limit_max_height = this->m_limit.height();
			
			solve_explicit_vertical_size();
			// 同上
			if (limit_min_height != this->m_limit_min_height ||
					limit_max_height != this->m_limit.height() ) {
				if ( this->m_content_align == ContentAlign::BOTTOM ) {
					// 底部对齐高度变化一定会影响到子盒子的最终位置偏移
					child_mark = M_MATRIX;
				}
				v_change = true;
			}
			change = true;
		}
		
		this->solve_explicit_size_after(h_change, v_change, child_mark);
		
		return change;
	}
	
	/**
	 * @func solve_explicit_horizontal_size
	 */
	void solve_explicit_horizontal_size() {
		
		Box* parent = View::parent()->as_box();
		
		// 布局视图加入到普通视图内或父视图没有明确宽度时,属性类型AUTO/PARENT/PERCENT/MINUS会失效.
		if ( parent && parent->m_explicit_width ) {
			
			float parent_width = parent->m_final_width;
			
			if (this->m_width.type == this->m_max_width.type &&
					this->m_width.value == this->m_max_width.value ) {
				
				switch ( this->m_width.type ) {
					case ValueType::AUTO:
						_box_inl__solve_horizontal_size_with_auto_width(this, parent_width);
						this->m_limit_min_width = 0;
						this->m_limit.width(Float::max);
						break;
					case ValueType::FULL:
						_box_inl__solve_horizontal_size_with_full_width(this, parent_width);
						this->m_limit_min_width = this->m_final_width;
						this->m_limit.width(this->m_final_width);
						break;
					case ValueType::PIXEL:
						this->m_final_width = this->m_width.value;
						this->m_limit_min_width = this->m_final_width;
						this->m_limit.width(this->m_final_width);
						_box_inl__solve_horizontal_size_with_explicit_width(this, parent_width);
						break;
					case ValueType::PERCENT:
						this->m_final_width = this->m_width.value * parent_width;
						this->m_limit_min_width = this->m_final_width;
						this->m_limit.width(this->m_final_width);
						_box_inl__solve_horizontal_size_with_explicit_width(this, parent_width);
						break;
					case ValueType::MINUS:
						this->m_final_width = FX_MAX(parent_width - this->m_width.value, 0);
						this->m_limit_min_width = this->m_final_width;
						this->m_limit.width(this->m_final_width);
						_box_inl__solve_horizontal_size_with_explicit_width(this, parent_width);
						break;
				}
				
			} else {
				
				_box_inl__solve_horizontal_size_with_auto_width(this, parent_width);
				
				switch ( this->m_width.type ) {
					case ValueType::AUTO:
						this->m_limit_min_width = 0;
						break;
					case ValueType::FULL:
						this->m_limit_min_width = (parent_width -
																			 this->m_final_margin_left - this->m_final_margin_right -
																			 this->m_border_left_width - this->m_border_right_width);
						break;
					case ValueType::PIXEL:
						this->m_limit_min_width = this->m_width.value;
						break;
					case ValueType::PERCENT:
						this->m_limit_min_width = this->m_width.value * parent_width;
						break;
					case ValueType::MINUS:
						this->m_limit_min_width = FX_MAX(parent_width - this->m_width.value, 0);
						break;
				}
				
				switch ( this->m_max_width.type ) {
					case ValueType::AUTO:
						break;
					case ValueType::FULL:
						this->m_limit.width(parent_width -
																this->m_final_margin_left - this->m_final_margin_right -
																this->m_border_left_width - this->m_border_right_width);
						break;
					case ValueType::PIXEL:
						this->m_limit.width(this->m_max_width.value);
						break;
					case ValueType::PERCENT:
						this->m_limit.width(this->m_max_width.value * parent_width);
						break;
					case ValueType::MINUS:
						this->m_limit.width(FX_MAX(parent_width - this->m_max_width.value, 0));
						break;
				}
				
				if ( this->m_limit_min_width > this->m_limit.width() ) {
					this->m_limit_min_width = this->m_limit.width();
				}
			}
			
		} else {
			
			if (this->m_margin_left.type == ValueType::PIXEL) {
				this->m_final_margin_left = this->m_margin_left.value;
			} else {
				this->m_final_margin_left = 0;
			}
			
			if (this->m_margin_right.type == ValueType::PIXEL) {
				this->m_final_margin_right = this->m_margin_right.value;
			} else {
				this->m_final_margin_right = 0;
			}
			
			this->m_raw_client_width =  this->m_border_left_width + this->m_border_right_width +
																	this->m_final_margin_left + this->m_final_margin_right;
			
			// 父视图没有明确的宽度或者父视图为普通视图,只有像素值才生效否则为不明确的值
			if (this->m_width.type == ValueType::PIXEL &&
					this->m_max_width.type == ValueType::PIXEL &&
					this->m_width.value == this->m_max_width.value) {
				this->m_final_width = this->m_width.value;
				this->m_limit_min_width = this->m_final_width;
				this->m_limit.width(this->m_final_width);
				this->m_explicit_width = true;  // 明确的宽度,不受内部挤压
			} else {
				// 自动与百分比都无效,只能从内部挤压宽度.
				this->m_limit_min_width = 0;
				this->m_limit.width(Float::max);
				if ( this->m_width.type == ValueType::PIXEL ) {
					this->m_limit_min_width = this->m_width.value;
				}
				if ( this->m_max_width.type == ValueType::PIXEL ) {
					this->m_limit.width(this->m_max_width.value);
				} else {
					if ( parent && this->m_max_width.type == ValueType::FULL ) { // 使用父视图的限制
						this->m_limit.width(parent->m_limit.width() - this->m_raw_client_width);
					}
				}
				if ( this->m_limit_min_width > this->m_limit.width() ) {
					this->m_limit_min_width = this->m_limit.width();
				}
				this->m_explicit_width = false; // 不是明确的宽度,受内部挤压
			}

			this->m_raw_client_width += this->m_final_width;
		}
	}
	
	/**
	 * @func solve_explicit_vertical_size
	 */
	void solve_explicit_vertical_size() {
		
		Box* parent = View::parent()->as_box();
		
		// 布局视图加入到普通视图内或父视图没有明确高度时,属性类型AUTO/PARENT/PERCENT/MINUS会失效.
		if (parent && parent->m_explicit_height) {
			
			float parent_height = parent->m_final_height;
			
			if ( this->m_height.type == this->m_max_height.type &&
					this->m_height.value == this->m_max_height.value ) {
				
				switch ( this->m_height.type ) {
					case ValueType::AUTO:
						_box_inl__solve_vertical_size_with_auto_height(this, parent_height);
						this->m_limit_min_height = 0;
						this->m_limit.height(Float::max);
						break;
					case ValueType::FULL:
						_box_inl__solve_vertical_size_with_full_height(this, parent_height);
						this->m_limit_min_height = this->m_final_height;
						this->m_limit.height(this->m_final_height);
						break;
					case ValueType::PIXEL:
						this->m_final_height = this->m_height.value;
						this->m_limit_min_height = this->m_final_height;
						this->m_limit.height(this->m_final_height);
						_box_inl__solve_vertical_size_with_explicit_height(this, parent_height);
						break;
					case ValueType::PERCENT:
						this->m_final_height = this->m_height.value * parent_height;
						this->m_limit_min_height = this->m_final_height;
						this->m_limit.height(this->m_final_height);
						_box_inl__solve_vertical_size_with_explicit_height(this, parent_height);
						break;
					case ValueType::MINUS:
						this->m_final_height = FX_MAX(parent_height - this->m_height.value, 0);
						this->m_limit_min_height = this->m_final_height;
						this->m_limit.height(this->m_final_height);
						_box_inl__solve_vertical_size_with_explicit_height(this, parent_height);
						break;
				}
				
			} else {
				
				_box_inl__solve_vertical_size_with_auto_height(this, parent_height);
				
				switch ( this->m_height.type ) {
					case ValueType::AUTO:
						this->m_limit_min_height = 0;
						break;
					case ValueType::FULL:
						this->m_limit_min_height = (parent_height -
																				this->m_final_margin_top - this->m_final_margin_bottom -
																				this->m_border_top_width - this->m_border_bottom_width);
						break;
					case ValueType::PIXEL:
						this->m_limit_min_height = this->m_height.value;
						break;
					case ValueType::PERCENT:
						this->m_limit_min_height = this->m_height.value * parent_height;
						break;
					case ValueType::MINUS:
						this->m_limit_min_height = FX_MAX(parent_height - this->m_height.value, 0);
						break;
				}
				
				switch ( this->m_max_height.type ) {
					case ValueType::AUTO:
						break;
					case ValueType::FULL:
						this->m_limit.height(parent_height -
																 this->m_final_margin_top - this->m_final_margin_bottom -
																 this->m_border_top_width - this->m_border_bottom_width);
						break;
					case ValueType::PIXEL:
						this->m_limit.height(this->m_max_height.value);
						break;
					case ValueType::PERCENT:
						this->m_limit.height(this->m_max_height.value * parent_height);
						break;
					case ValueType::MINUS:
						this->m_limit.height(FX_MAX(parent_height - this->m_max_height.value, 0));
						break;
				}
				
				if ( this->m_limit_min_height > this->m_limit.height() ) {
					this->m_limit_min_height = this->m_limit.height();
				}
			}
			
		} else {
			
			if (this->m_margin_top.type == ValueType::PIXEL) {
				this->m_final_margin_top = this->m_margin_top.value;
			} else {
				this->m_final_margin_top = 0;
			}
			
			if (this->m_margin_bottom.type == ValueType::PIXEL) {
				this->m_final_margin_bottom = this->m_margin_bottom.value;
			} else {
				this->m_final_margin_bottom = 0;
			}
			
			this->m_raw_client_height = this->m_border_top_width + this->m_border_bottom_width +
																	this->m_final_margin_top + this->m_final_margin_bottom;
			
			// 父视图没有明确的高度或者父视图为普通视图,只有像素值才生效否则为不明确的值
			if (this->m_height.type == ValueType::PIXEL &&
					this->m_max_height.type == ValueType::PIXEL &&
					this->m_height.value == this->m_max_height.value) {
				this->m_final_height = this->m_height.value;
				this->m_limit_min_height = this->m_final_height;
				this->m_limit.height(this->m_final_height);
				this->m_explicit_height = true;  // 明确的高度,不受内部挤压
			} else {
				// 自动与百分比都无效,只能从内部挤压高度.
				this->m_limit_min_height = 0;
				this->m_limit.height(Float::max);
				if ( this->m_height.type == ValueType::PIXEL ) {
					this->m_limit_min_height = this->m_height.value;
				}
				if ( this->m_max_height.type == ValueType::PIXEL ) {
					this->m_limit.height(this->m_max_height.value);
				} else {
					if ( parent && this->m_max_height.type == ValueType::FULL ) { // 使用父视图的限制
						this->m_limit.height(parent->m_limit.height() - this->m_raw_client_height);
					}
				}
				if ( this->m_limit_min_height > this->m_limit.height() ) {
					this->m_limit_min_height = this->m_limit.height();
				}
				this->m_explicit_height = false; // 不是明确的高度,受内部挤压
			}

			this->m_raw_client_height += this->m_final_height;
		}
	}

	/**
	 * @func set_horizontal_active_mark_
	 */
	void set_horizontal_active_mark_() {
		uint value = M_NONE;
		// 如果这些值都不为像素,父视图会可能影响到子视图的M_SIZE_HORIZONTAL
		// 相当于标记了这个子视图M_SIZE_HORIZONTAL
		if ((this->m_width.type != ValueType::PIXEL && this->m_width.type != ValueType::AUTO) ||
				(this->m_max_width.type != ValueType::PIXEL && this->m_max_width.type != ValueType::AUTO)) {
			value = (M_LAYOUT | M_SHAPE | M_SIZE_HORIZONTAL);
		}
		if (this->m_margin_left.type != ValueType::PIXEL) {
			value |= (M_LAYOUT | M_MATRIX | M_SIZE_HORIZONTAL);
		} else if (this->m_margin_right.type != ValueType::PIXEL) {
			value |= (M_LAYOUT | M_SIZE_HORIZONTAL);
		}
		this->horizontal_active_mark_value = value;
	}
	
	/**
	 * @func set_vertical_active_mark_
	 */
	void set_vertical_active_mark_() {
		uint value = M_NONE;
		// 如果这些值都不为像素,父视图将会可能影响到子视图的M_SIZE_VERTICAL
		// 相当于标记了这个子视图M_SIZE_VERTICAL
		if ((this->m_height.type != ValueType::PIXEL && this->m_height.type != ValueType::AUTO) ||
				(this->m_max_height.type != ValueType::PIXEL && this->m_max_height.type != ValueType::AUTO)) {
			value |= (M_LAYOUT | M_SHAPE | M_SIZE_VERTICAL);
		}
		if (this->m_margin_top.type != ValueType::PIXEL) {
			value |= (M_LAYOUT | M_MATRIX | M_SIZE_VERTICAL);
		} else if (this->m_margin_bottom.type != ValueType::PIXEL) {
			value |= (M_LAYOUT | M_SIZE_VERTICAL);
		}
		this->vertical_active_mark_value = value;
	}
	
};

void Limit::set_horizontal_active_mark() {
	_inl_limit(this)->set_horizontal_active_mark_();
}

void Limit::set_vertical_active_mark() {
	_inl_limit(this)->set_vertical_active_mark_();
}

void LimitIndep::set_horizontal_active_mark() {
	_inl_limit_indep(this)->set_horizontal_active_mark_();
}

void LimitIndep::set_vertical_active_mark() {
	_inl_limit_indep(this)->set_vertical_active_mark_();
}

/**
 * @constructor
 */
Limit::Limit()
: m_max_width(ValueType::AUTO)
, m_max_height(ValueType::AUTO)
, m_limit_min_width(0)
, m_limit_min_height(0) {
	
}

/**
 * @func min_width
 */
void Limit::set_max_width(Value value) {
	m_max_width = value;
	mark_pre(M_SHAPE | M_LAYOUT | M_SIZE_HORIZONTAL);
	set_horizontal_active_mark();
}

/**
 * @func min_width
 */
void Limit::set_max_height(Value value) {
	m_max_height = value;
	mark_pre(M_SHAPE | M_LAYOUT | M_SIZE_VERTICAL);
	set_vertical_active_mark();
}

void Limit::set_layout_explicit_size() {
	if ( m_final_visible ) {
		// 只需要解决 explicit size
		if ( ! _inl_limit(this)->solve_explicit_size() ) {
			return;
		}
	} else {
		if ( ! (mark_value & M_VISIBLE) ) {
			return;
		}
	}
	// M_SIZE_HORIZONTAL | M_SIZE_VERTICAL
	// 布局尺寸改变可能会影响到所有的兄弟视图的偏移值
	
	Layout* layout = parent()->as_layout();
	if ( layout ) {
		layout->mark_pre(M_CONTENT_OFFSET);
	} else {
		set_default_offset_value(); // 父视图只是个普通视图,默认将偏移设置为0
	}
}

void LimitIndep::set_layout_explicit_size() {
	
	if ( m_final_visible ) {
		// 只需要解决 explicit size
		if ( _inl_limit_indep(this)->solve_explicit_size() ) {
			
			Box* box = parent()->as_box();
			if ( box ) {
				m_parent_layout = box;
				mark_pre(M_LAYOUT_THREE_TIMES);
			} else { // 父视图只是个普通视图,默认将偏移设置为0
				set_default_offset_value();
			}
		}
	}
}

void Limit::set_layout_content_offset() {
	
	if (m_final_visible) {
		
		Vec2 squeeze;
		
		if ( set_div_content_offset(squeeze, Vec2(m_limit_min_width, m_limit_min_height)) ) { //
			// 高度或宽度被挤压即形状发生变化
			mark(M_SHAPE);
			
			// 被挤压会影响到所有的兄弟视图的偏移值
			// 所以标记父视图M_CONTENT_OFFSET
			Layout* layout = parent()->as_layout();
			if (layout) {
				layout->mark_pre(M_CONTENT_OFFSET);
			} else { // 父视图只是个普通视图,默认将偏移设置为0
				set_default_offset_value();
			}
		}
	}
}

void LimitIndep::set_layout_content_offset() {
	
	if (m_final_visible) {
		Vec2 squeeze;
		
		if ( set_div_content_offset(squeeze, Vec2(m_limit_min_width, m_limit_min_height)) ) { //
			// 高度或宽度被挤压即形状发生变化
			mark(M_SHAPE);
			
			Box* box = parent()->as_box();
			if ( box ) {
				m_parent_layout = box;
				mark_pre(M_LAYOUT_THREE_TIMES);
			} else { // 父视图只是个普通视图,默认将偏移设置为0
				set_default_offset_value();
			}
		}
	}
}

void Limit::set_layout_three_times(bool horizontal, bool hybrid) {
	// NOOP
}

FX_END
