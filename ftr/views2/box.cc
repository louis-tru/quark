/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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
#include "text.h"
#include "box.h"
#include "text-font.h"
#include "draw.h"
#include "app.h"
#include "display-port.h"
#include "css.h"
#include "limit.h"
#include "limit-indep.h"
#include "background.h"
#include "texture.h"

namespace ftr {

FX_DEFINE_INLINE_MEMBERS(Box, Inl) {
 public:
 #define _inl(self) static_cast<Box::Inl*>(static_cast<Box*>(self))
	
	template<bool hybrid> bool solve_explicit_size() {
		
		bool h_change = false;
		bool v_change = false;
		bool change = false;
		uint child_mark = M_NONE;
		
		if ( mark_value & M_SIZE_HORIZONTAL ) {
			
			float raw_limit_width = _limit.width();
			
			solve_explicit_horizontal_size();
			
			// 造成子盒子变化有（包括尺寸变化与偏移变化）:
			// 1.最终宽度变化可能对子盒子造成影响
			// 2.明确与不明确的变化可能对子盒子造成影响
			// 3.宽度限制的变化可能会影响子盒子偏移变化
			//  前两项的变化都会造成第三项的变化..
			
			if ( raw_limit_width != _limit.width() ) {
				if ( hybrid ) {
					TextAlign align = reinterpret_cast<Hybrid*>(this)->text_align();
					if ( align != TextAlign::LEFT && align != TextAlign::LEFT_REVERSE ) {
						child_mark = M_MATRIX;
					}
				} else { // Div
					if ( reinterpret_cast<Div*>(this)->content_align() == ContentAlign::RIGHT ) {
						child_mark = M_MATRIX;
					}
				}
				h_change = true;
			}
			change = true; // 不管client_width是否真的变化都对父盒子发出通知
		}
		
		if ( mark_value & M_SIZE_VERTICAL ) {
			
			float raw_limit_height = _limit.height();
			
			solve_explicit_vertical_size();
			// 同上
			if ( raw_limit_height != _limit.height() ) {
				if ( !hybrid ) {
					if ( reinterpret_cast<Div*>(this)->content_align() == ContentAlign::BOTTOM ) {
						child_mark = M_MATRIX;
					}
				}
				v_change = true;
			}
			change = true;
		}
		
		solve_explicit_size_after(h_change, v_change, child_mark);
		
		return change;
	}
	
	inline void solve_horizontal_size_with_auto_width(float parent) {
		
		_explicit_width = false;
		
		switch (_margin_left.type) {
			case ValueType::AUTO:
			case ValueType::FULL:   _final_margin_left = 0; break;
			case ValueType::PIXEL:  _final_margin_left = _margin_left.value; break;
			case ValueType::PERCENT:_final_margin_left = parent * _margin_left.value; break;
			default:                _final_margin_left = FX_MAX(parent - _margin_left.value, 0); break;
		}
		
		switch (_margin_right.type) {
			case ValueType::AUTO:
			case ValueType::FULL:   _final_margin_right = 0; break;
			case ValueType::PIXEL:  _final_margin_right = _margin_right.value; break;
			case ValueType::PERCENT:_final_margin_right = parent * _margin_right.value; break;
			default:                _final_margin_right = FX_MAX(parent - _margin_right.value, 0); break;
		}
		
		_raw_client_width = _final_margin_left + _final_margin_right +
												 _border_left_width + _border_right_width + _final_width;
		_limit.width(Float::max);
	}
	
	inline void solve_horizontal_size_with_full_width(float parent) {
		
		_explicit_width = true; // 明确的宽度
		
		switch (_margin_left.type) {
			case ValueType::AUTO:
			case ValueType::FULL:   _final_margin_left = 0; break;   // 宽度为FULL,边距就不能为FULL
			case ValueType::PIXEL:  _final_margin_left = _margin_left.value; break;
			case ValueType::PERCENT:_final_margin_left = parent * _margin_left.value; break;
			default:                _final_margin_left = FX_MAX(parent - _margin_left.value, 0); break;
		}
		
		switch (_margin_right.type) {
			case ValueType::AUTO:
			case ValueType::FULL:   _final_margin_right = 0; break;  // 宽度为FULL,边距就不能为FULL
			case ValueType::PIXEL:  _final_margin_right = _margin_right.value; break;
			case ValueType::PERCENT:_final_margin_right = parent * _margin_right.value; break;
			default:                _final_margin_right = FX_MAX(parent - _margin_right.value, 0); break;
		}
		
		_raw_client_width = _final_margin_left +
												 _final_margin_right + _border_left_width + _border_right_width;
		
		if ( _raw_client_width < parent ) {
			_final_width = parent - _raw_client_width;
			_raw_client_width = parent;
		} else {
			_final_width = 0;
		}
		_limit.width(_final_width);
	}
	
	inline void solve_horizontal_size_with_explicit_width(float parent) {
		
		_explicit_width = true;
		
		_raw_client_width = _final_width + _border_left_width + _border_right_width;
		
		// 剩下的宽度
		float width = parent - _raw_client_width;
		
		if (_margin_left.type == ValueType::AUTO || _margin_left.type == ValueType::FULL) {
			if (_margin_right.type == ValueType::AUTO || _margin_right.type == ValueType::FULL) {
				// 左右边距都为自动
				_final_margin_left = _final_margin_right = FX_MAX(width / 2, 0);
			} else { // 只有左边距为自动
				switch(_margin_right.type) {
					case ValueType::PIXEL:   _final_margin_right = _margin_right.value; break;
					case ValueType::PERCENT: _final_margin_right = _margin_right.value * parent; break;
					default:                 _final_margin_right = FX_MAX(parent - _margin_right.value, 0);
						break;
				}
				_final_margin_left = FX_MAX(width - _final_margin_right, 0);
			}
		}
		else if (_margin_right.type == ValueType::AUTO || _margin_right.type == ValueType::FULL) {
			// 只有右边距为自动
			switch(_margin_left.type) {
				case ValueType::PIXEL:   _final_margin_left = _margin_left.value; break;
				case ValueType::PERCENT: _final_margin_left = _margin_left.value * parent; break;
				default:                 _final_margin_left = FX_MAX(parent - _margin_left.value, 0); break;
			}
			_final_margin_right = FX_MAX(width - _final_margin_left, 0);
		}
		else { // 左右边距都不为自动
			switch(_margin_left.type) {
				case ValueType::PIXEL:    _final_margin_left = _margin_left.value; break;
				case ValueType::PERCENT:  _final_margin_left = _margin_left.value * parent; break;
				default:                  _final_margin_left = FX_MAX(parent - _margin_left.value, 0);
					break;
			}
			switch(_margin_right.type) {
				case ValueType::PIXEL:   _final_margin_right = _margin_right.value; break;
				case ValueType::PERCENT: _final_margin_right = _margin_right.value * parent; break;
				default:                 _final_margin_right = FX_MAX(parent - _margin_right.value, 0);
					break;
			}
		}
		_raw_client_width += (_final_margin_left + _final_margin_right);
	}
	
	inline void solve_final_horizontal_size_with_full_width(float parent) {
		
		float width = _final_margin_left +
									_final_margin_right + _border_left_width + _border_right_width;
		if ( width < parent ) {
			_final_width = parent - width;
			width = parent;
		} else {
			_final_width = 0;
		}
		_offset_end.x(_offset_start.x() + width);
	}

	void solve_explicit_horizontal_size() {
		
		Box* parent = View::parent()->as_box();
		
		// 布局视图加入到普通视图内或父视图没有明确宽度时,属性类型AUTO/FULL/PERCENT/MINUS会失效.
		if ( parent && parent->_explicit_width ) {
			
			float parent_width = parent->_final_width;
			
			// 1.AUTO
			//   尺寸会受内部挤压影响
			// 2.FULL
			//   当一个属性被设置为自动的时候,这个属性会以自身视图水平尺寸占满父视图的全部宽度为目标,
			//   它的值会等于父视图宽度减视图水平尺寸其它属性(margin/border)的全部宽度,如果得到的这个值小于0则取0.
			//   自动宽度为最高优先级,即就是同时出现宽度与其它属性都为自动的情况,让宽度取自动值其它则取0,
			//   原则是同时只能有一类属性为生效自动值.
			// 3.像素值
			//   取固定像素值
			// 4.百分比值
			//   取父视图宽度的百分比值
			// 5.减法值
			
			switch ( _width.type ) {
				case ValueType::AUTO:  // 自动宽度,由内部挤压确定 (width=auto)
					solve_horizontal_size_with_auto_width(parent_width);
					break;
				case ValueType::FULL:  // 吸附父视图 (width=full)
					solve_horizontal_size_with_full_width(parent_width);
					break;
				case ValueType::PIXEL:  // 像素值 (width=50)
					_final_width = _width.value;
					_limit.width(_final_width);
					solve_horizontal_size_with_explicit_width(parent_width);
					break;
				case ValueType::PERCENT: // 百分比 (width=50%)
					_final_width = _width.value * parent_width;
					_limit.width(_final_width);
					solve_horizontal_size_with_explicit_width(parent_width);
					break;
				default:  // 减法值 (width=50!)
					_final_width = FX_MAX(parent_width - _width.value, 0);
					_limit.width(_final_width);
					solve_horizontal_size_with_explicit_width(parent_width);
					break;
			}
			
		} else {
			
			_limit.width(Float::max);
			
			if (_margin_left.type == ValueType::PIXEL) {
				_final_margin_left = _margin_left.value;
			} else {
				_final_margin_left = 0;
			}
			
			if (_margin_right.type == ValueType::PIXEL) {
				_final_margin_right = _margin_right.value;
			} else {
				_final_margin_right = 0;
			}
			
			_raw_client_width =  _border_left_width + _border_right_width +
														_final_margin_left + _final_margin_right;
			
			// 父视图没有明确的宽度或者父视图为普通视图,只有像素值才生效否则为不明确的值
			if (_width.type == ValueType::PIXEL) {
				_final_width = _width.value;
				_limit.width(_final_width);
				_explicit_width = true;  // 明确的宽度,不受内部挤压
			} else {
				
				// 如果父盒子为水平布局同时没有明确的宽度,FULL宽度会导致三次布局
				if ( parent && _width.type == ValueType::FULL ) { // 使用父视图的限制
					_limit.width(parent->_limit.width() - _raw_client_width);
					// mark_pre(M_LAYOUT_THREE_TIMES); // TODO
				}
				
				// 自动与百分比都无效,只能从内部挤压宽度.
				_explicit_width = false; // 不是明确的宽度,受内部挤压
			}

			_raw_client_width += _final_width;
		}
	}
	
	inline void solve_vertical_size_with_auto_height(float parent) {
		
		_explicit_height = false; // 高度由内部挤压
		
		switch (_margin_top.type) {
			case ValueType::AUTO:
			case ValueType::FULL: _final_margin_top = 0; break;   // 高度为自动值,边距就不能为自动值
			case ValueType::PIXEL:  _final_margin_top = _margin_top.value; break;
			case ValueType::PERCENT:_final_margin_top = parent * _margin_top.value; break;
			default:      _final_margin_top = FX_MAX(parent - _margin_top.value, 0); break;
		}
		
		switch (_margin_bottom.type) {
			case ValueType::AUTO:
			case ValueType::FULL: _final_margin_bottom = 0; break;  // 高度为自动值,边距就不能为自动值
			case ValueType::PIXEL:  _final_margin_bottom = _margin_bottom.value; break;
			case ValueType::PERCENT:_final_margin_bottom = parent * _margin_bottom.value; break;
			default:   _final_margin_bottom = FX_MAX(parent - _margin_bottom.value, 0); break;
		}
		
		_raw_client_height = _final_margin_top + _final_margin_bottom +
													_border_top_width + _border_bottom_width + _final_height;
		_limit.height(Float::max);
	}
	
	inline void solve_vertical_size_with_full_height(float parent) {
		
		_explicit_height = true; // 明确的高度
		
		switch (_margin_top.type) {
			case ValueType::AUTO:
			case ValueType::FULL:   _final_margin_top = 0; break;   // 高度为自动值,边距就不能为自动值
			case ValueType::PIXEL:  _final_margin_top = _margin_top.value; break;
			case ValueType::PERCENT:_final_margin_top = parent * _margin_top.value; break;
			default:                _final_margin_top = FX_MAX(parent - _margin_top.value, 0); break;
		}
		
		switch (_margin_bottom.type) {
			case ValueType::AUTO:
			case ValueType::FULL:   _final_margin_bottom = 0; break;  // 高度为自动值,边距就不能为自动值
			case ValueType::PIXEL:  _final_margin_bottom = _margin_bottom.value; break;
			case ValueType::PERCENT:_final_margin_bottom = parent * _margin_bottom.value; break;
			default:                _final_margin_bottom = FX_MAX(parent - _margin_bottom.value, 0);
				break;
		}
		
		_raw_client_height = _final_margin_top +
													_final_margin_bottom + _border_top_width + _border_bottom_width;
		
		if (_raw_client_height < parent) {
			_final_height = parent - _raw_client_height;
			_raw_client_height = parent;
		} else {
			_final_height = 0;
		}
		_limit.height(_final_height);
	}
	
	inline void solve_vertical_size_with_explicit_height(float parent) {
		
		_explicit_height = true; // 明确高度
		
		_raw_client_height = _final_height + _border_top_width + _border_bottom_width;
		
		// 剩下的高度
		float height = parent - _raw_client_height;
		
		if (_margin_top.type == ValueType::AUTO) {
			if (_margin_bottom.type == ValueType::AUTO) { // 上下边距都为自动
				_final_margin_top = _final_margin_bottom = FX_MAX(height / 2, 0);
			} else { // 只有上边距为自动
				switch(_margin_bottom.type) {
					case ValueType::PIXEL:   _final_margin_bottom = _margin_bottom.value; break;
					case ValueType::FULL: _final_margin_bottom = _margin_bottom.value * parent; break;
					default: _final_margin_bottom = FX_MAX(parent - _margin_bottom.value, 0); break;
				}
				_final_margin_top = FX_MAX(height - _final_margin_bottom, 0);
			}
		}
		else if (_margin_bottom.type == ValueType::AUTO) { // 只有下边距为自动
			switch(_margin_top.type) {
				case ValueType::PIXEL:   _final_margin_top = _margin_top.value; break;
				case ValueType::PERCENT: _final_margin_top = _margin_top.value * parent; break;
				default: _final_margin_top = FX_MAX(parent - _margin_top.value, 0); break;
			}
			_final_margin_bottom = FX_MAX(height - _final_margin_top, 0);
		}
		else { // 上下边距都不为自动
			switch(_margin_top.type) {
				case ValueType::PIXEL:   _final_margin_top = _margin_top.value; break;
				case ValueType::PERCENT: _final_margin_top = _margin_top.value * parent; break;
				default: _final_margin_top = FX_MAX(parent - _margin_top.value, 0); break;
			}
			switch(_margin_bottom.type) {
				case ValueType::PIXEL:   _final_margin_bottom = _margin_bottom.value; break;
				case ValueType::PERCENT: _final_margin_bottom = _margin_bottom.value * parent; break;
				default: _final_margin_bottom = FX_MAX(parent - _margin_bottom.value, 0); break;
			}
		}
		_raw_client_height += (_final_margin_top + _final_margin_bottom);
	}
	
	inline void solve_final_vertical_size_with_full_height(float parent) {
		
		float height = _final_margin_top +
									 _final_margin_bottom + _border_top_width + _border_bottom_width;
		if (height < parent) {
			_final_height = parent - height;
			height = parent;
		} else {
			_final_height = 0;
		}
		_offset_end.y(_offset_start.y() + height);
	}

	void solve_explicit_vertical_size() {
		
		Box* parent = View::parent()->as_box();
		
		// 布局视图加入到普通视图内或父视图没有明确高度时,属性类型AUTO/PARENT/PERCENT/MINUS会失效.
		if (parent && parent->_explicit_height) {
			
			float parent_height = parent->_final_height;
			
			switch ( _height.type ) {
				case ValueType::AUTO:
					solve_vertical_size_with_auto_height(parent_height);
					break;
				case ValueType::FULL: // 接近父视图
					solve_vertical_size_with_full_height(parent_height);
					break;
				case ValueType::PIXEL: // 像素值
					_final_height = _height.value;
					_limit.height(_final_height);
					solve_vertical_size_with_explicit_height(parent_height);
					break;
				case ValueType::PERCENT: // 百分比
					_final_height = _height.value * parent_height;
					_limit.height(_final_height);
					solve_vertical_size_with_explicit_height(parent_height);
					break;
				default: // 减法值
					_final_height = FX_MAX(parent_height - _height.value, 0);
					_limit.height(_final_height);
					solve_vertical_size_with_explicit_height(parent_height);
					break;
			}
			
		} else {
			
			_limit.height(Float::max);
			
			if (_margin_top.type == ValueType::PIXEL) {
				_final_margin_top = _margin_top.value;
			} else {
				_final_margin_top = 0;
			}
			
			if (_margin_bottom.type == ValueType::PIXEL) {
				_final_margin_bottom = _margin_bottom.value;
			} else {
				_final_margin_bottom = 0;
			}
			
			_raw_client_height = _border_top_width + _border_bottom_width +
														_final_margin_top + _final_margin_bottom;
			
			// 如果父盒子为垂直布局同时没有明确的高度,FULL高度会导致三次布局
			if (_height.type == ValueType::PIXEL) {
				_final_height = _height.value;
				_limit.height(_final_height);
				_explicit_height = true;  // 明确的高度,不受内部挤压
			} else {
				
				// 父视图没有明确的高度,当高度为FULL时需要第三次排版计算最终高度
				if ( parent && _width.type == ValueType::FULL ) {// 使用父视图的限制
					_limit.height(parent->_limit.height() - _raw_client_height);
					// mark_pre(M_LAYOUT_THREE_TIMES); // TODO
				}
				
				// 自动与百分比都无效,只能从内部挤压高度.
				_explicit_height = false; // 不是明确的高度,受内部挤压
			}

			_raw_client_height += _final_height;
		}
	}
	
	inline float get_horizontal_reverse_offset(float outer_width) const {
		return outer_width - _offset_end.x();
	}
	
	inline float get_vertical_reverse_offset(float outer_width) const {
		return outer_width - _offset_end.y();
	}
	
	inline Box* set_offset_horizontal_(Box* prev, Vec2& squeeze,
																		 float limit_width, Div* div, bool layout_three) {
		ASSERT(div);
		
		_parent_layout = div;
		
		if ( ! _visible ) { // 如果没有显示不做处理
			set_default_offset_value(); return prev;
		}
		
		Vec2 old_offset_start = _offset_start;
		
		Box* rv = this;
		
		if ( layout_three && _width.type == ValueType::FULL ) {
			// 父盒子无明确宽度意味着父盒子的宽度接下来还会受到子盒子的挤压影响
			if ( !div->_explicit_width ) {
				// 那么父盒子的宽度变化会严重影响到full属性的子盒子
				// 所以这里需要标记三次布局,在第三次布局中才能最终确定盒子的宽度
				mark_pre(M_LAYOUT_THREE_TIMES);
				// 三次成局将会一定程度影响性能,所以非必要尽量设置明确的尺寸
				rv = nullptr;
			}
			
		} else {
			
			if ( ! _newline/*not newline*/ && prev ) { // 需要上面兄弟布局视图来帮忙解决
				
				float offset_end_x = prev->_offset_end.x() + _raw_client_width;
				
				if ( offset_end_x <= limit_width ) { // 可以将这个兄弟视图紧挨排列在右边
					_offset_start.x(prev->_offset_end.x());
					_offset_start.y(prev->_offset_start.y());
					_offset_end.x(offset_end_x);
					_offset_end.y(prev->_offset_start.y() + _raw_client_height);
					
					if ( old_offset_start != _offset_start ) {
						// 偏移值发生改变,还需计算最终的变换矩阵,所以这里标记变换做进一步处理.
						mark(M_MATRIX);
					}
					
					if ( squeeze.width() < _offset_end.x() ) { // 设置挤压宽度
						squeeze.width(_offset_end.x());
					}
					if (squeeze.height() < _offset_end.y()) { // 设置挤压高度
						squeeze.height(_offset_end.y());
					}
					
					return this;
				}
			}
		}
		
		// 从最开始位置布局
		_offset_start.x(0);
		_offset_start.y(squeeze.height());
		_offset_end.x(_raw_client_width);
		_offset_end.y(squeeze.height() + _raw_client_height);
		
		if ( old_offset_start != _offset_start ) {
			// 偏移值发生改变,还需计算最终的变换矩阵,所以这里标记变换做进一步处理.
			mark(M_MATRIX);
		}
		
		if ( squeeze.width() < _offset_end.x() ) { // 设置挤压宽度
			squeeze.width(_offset_end.x());
		}
		squeeze.height(_offset_end.y()); // 设置挤压高度
		
		return rv;
	}
	
	inline Box* set_offset_vertical_(Box* prev, Vec2& squeeze,
																	 float limit_height, Div* div, bool layout_three) {
		ASSERT(div);
		
		_parent_layout = div;
		
		if ( ! _visible ) { // 如果没有显示不做处理
			set_default_offset_value(); return prev;
		}
		
		Vec2 old_offset_start = _offset_start;
		Box* rv = this;
		
		if ( layout_three && _height.type == ValueType::FULL ) {
			// 父盒子无明确高度意味着父盒子的高度接下来还会受到子盒子的挤压影响
			if ( !div->_explicit_height ) {
				// 那么父盒子的高度变化会严重影响到full属性的子盒子
				// 所以这里需要标记三次布局,在第三次布局中才能最终确定盒子的高度
				mark_pre(M_LAYOUT_THREE_TIMES);
				// 三次成局将会一定程度影响性能,所以非必要尽量设置明确的尺寸
				rv = nullptr;
			}
			
		} else {
			
			if ( ! _newline/*not newline*/ && prev ) { // 需要上面兄弟来帮忙解决
				
				float offset_end_y = prev->_offset_end.y() + _raw_client_height;
				
				if ( offset_end_y <= limit_height ) { // 可以将这个兄弟视图紧挨排列在下边
					_offset_start.x(prev->_offset_start.x());
					_offset_start.y(prev->_offset_end.y());
					_offset_end.y(offset_end_y);
					_offset_end.x(prev->_offset_start.x() + _raw_client_width);
					
					if ( old_offset_start != _offset_start ) {
						// 偏移值发生改变,还需计算最终的变换矩阵,所以这里标记变换做进一步处理.
						mark(M_MATRIX);
					}
					
					if (squeeze.width() < _offset_end.x()) { // 设置挤压宽度
						squeeze.width(_offset_end.x());
					}
					if (squeeze.height() < _offset_end.y()) { // 设置挤压高度
						squeeze.height(_offset_end.y());
					}
					return this;
				}
			}
		}
		
		// 从最开始位置布局
		_offset_start.y(0);
		_offset_start.x(squeeze.width());
		_offset_end.y(_raw_client_height);
		_offset_end.x(squeeze.width() + _raw_client_width);
		
		if ( old_offset_start != _offset_start ) {
			// 偏移值发生改变,还需计算最终的变换矩阵,所以这里标记变换做进一步处理.
			mark(M_MATRIX);
		}
		
		if ( squeeze.height() < _offset_end.y() ) { // 设置挤压高度
			squeeze.height(_offset_end.y());
		}
		squeeze.width(_offset_end.x());    // 设置挤压宽度
		
		return rv;
	}
	
	inline void set_offset_in_hybrid_(TextRows* rows, Vec2 limit,
																		Hybrid* hybrid, bool layout_three) {
		_parent_layout = hybrid;
		
		if ( ! _visible ) {
			set_default_offset_value(); return;
		}
		if ( rows->clip() ) { // 已经被修剪,不进行布局
			_linenum = -1;
			_draw_visible = false;
			set_default_offset_value(); return;
		}

		TextRows::Row* row = rows->last();
		float offset_end_x = row->offset_end.x() + _raw_client_width;
		Vec2  old_offset_start = _offset_start;
		Vec2  old_offset_end = _offset_end;
		
		bool newline = _newline;
		bool monopoly_line = false;
		
		if ( layout_three && _width.type == ValueType::FULL ) {
			// 父盒子无明确宽度意味着父盒子的宽度接下来还会受到子盒子的挤压影响
			if ( hybrid && !hybrid->_explicit_width ) {
				// 那么父盒子的宽度变化会严重影响到full属性的子盒子
				// 所以这里需要标记三次布局,在第三次布局中才能最终确定盒子的宽度
				mark_pre(M_LAYOUT_THREE_TIMES);
				// 三次成局将会一定程度影响性能,所以非必要尽量设置明确的尺寸
				
				newline = true;
				monopoly_line = true; /* monopoly line */
			}
		}
		
		if (row->offset_end.x() > 0 && /*要换新行当前行必须要有内容*/
				(newline /*force newline*/ ||
				 (Hybrid::is_auto_wrap(hybrid) /*自动换行*/ && offset_end_x > limit.width() /*当前行无法排版*/))
			)
		{ // 换行
			rows->push_row(_raw_client_height, 0);
			row = rows->last();
			offset_end_x = _raw_client_width;
		} else {
			rows->update_row(_raw_client_height, 0);
		}
		
		_linenum = rows->last_num(); // 记录行号
		_offset_start.x(row->offset_end.x());  // 距离最外层Text的偏移距离
		_offset_end.x(offset_end_x);
		row->offset_end.x(offset_end_x); // 设置offset_end.x
		_offset_end.y(row->baseline);
		_offset_start.y(row->baseline - _raw_client_height);

		// LOG("x:%f,x1:%f,y:%f,y2:%f", old_offset_end[0], _offset_end[0], old_offset_end[1], _offset_end[1]);
		
		if ( old_offset_start != _offset_start || old_offset_end != _offset_end ) {
			// 偏移值发生改变,还需计算最终的变换矩阵,所以这里标记变换做进一步处理.
			mark(M_MATRIX);
		}
		
		if ( monopoly_line ) {
			rows->push_row(0, 0); // 添加行
		}
	}
	
	void remove_background() {
		if (_background) {
			_background->release();
			_background = nullptr;
		}
	}
	
};

void _box_inl__solve_horizontal_size_with_auto_width(Box* box, float parent) {
	_inl(box)->solve_horizontal_size_with_auto_width(parent);
}
void _box_inl__solve_horizontal_size_with_full_width(Box* box, float parent) {
	_inl(box)->solve_horizontal_size_with_full_width(parent);
}
void _box_inl__solve_horizontal_size_with_explicit_width(Box* box, float parent) {
	_inl(box)->solve_horizontal_size_with_explicit_width(parent);
}
void _box_inl__solve_vertical_size_with_auto_height(Box* box, float parent) {
	_inl(box)->solve_vertical_size_with_auto_height(parent);
}
void _box_inl__solve_vertical_size_with_full_height(Box* box, float parent) {
	_inl(box)->solve_vertical_size_with_full_height(parent);
}
void _box_inl__solve_vertical_size_with_explicit_height(Box* box, float parent) {
	_inl(box)->solve_vertical_size_with_explicit_height(parent);
}
void _box_inl__solve_final_horizontal_size_with_full_width(Box* box, float parent) {
	_inl(box)->solve_final_horizontal_size_with_full_width(parent);
}
void _box_inl__solve_final_vertical_size_with_full_height(Box* box, float parent) {
	_inl(box)->solve_final_vertical_size_with_full_height(parent);
}

void Box::set_horizontal_active_mark() {
	uint value = M_NONE;
	// 如果这些值都不为像素,父视图会可能影响到子视图的M_SIZE_HORIZONTAL
	// 相当于标记了这个子视图M_SIZE_HORIZONTAL
	if (_width.type != ValueType::AUTO && _width.type != ValueType::PIXEL ) {
		value = (M_LAYOUT | M_SHAPE | M_SIZE_HORIZONTAL);
	}
	if (_margin_left.type != ValueType::PIXEL) {
		value |= (M_LAYOUT | M_MATRIX | M_SIZE_HORIZONTAL);
	}
	else if (_margin_right.type != ValueType::PIXEL) {
		value |= (M_LAYOUT | M_SIZE_HORIZONTAL);
	}
	horizontal_active_mark_value = value;
}

void Box::set_vertical_active_mark() {
	uint value = M_NONE;
	// 如果这些值都不为像素,父视图将会可能影响到子视图的M_SIZE_VERTICAL
	// 相当于标记了这个子视图M_SIZE_VERTICAL
	if (_height.type != ValueType::AUTO && _height.type != ValueType::PIXEL) {
		value |= (M_LAYOUT | M_SHAPE | M_SIZE_VERTICAL);
	}
	if (_margin_top.type != ValueType::PIXEL) {
		value |= (M_LAYOUT | M_MATRIX | M_SIZE_VERTICAL);
	}
	else if (_margin_bottom.type != ValueType::PIXEL) {
		value |= (M_LAYOUT | M_SIZE_VERTICAL);
	}
	vertical_active_mark_value = value;
}

Box::Box()
: _width(ValueType::AUTO)
, _height(ValueType::AUTO)
, _margin_top(ValueType::PIXEL)
, _margin_right(ValueType::PIXEL)
, _margin_bottom(ValueType::PIXEL)
, _margin_left(ValueType::PIXEL)
, _border_top_color()
, _border_right_color()
, _border_bottom_color()
, _border_left_color()
, _border_top_width(0)
, _border_right_width(0)
, _border_bottom_width(0)
, _border_left_width(0)
, _border_radius_left_top(0)
, _border_radius_right_top(0)
, _border_radius_right_bottom(0)
, _border_radius_left_bottom(0)
, _background_color(0, 0, 0, 0)
, _background(nullptr)
, _final_width(0)
, _final_height(0)
, _final_margin_left(0)
, _final_margin_top(0)
, _final_margin_right(0)
, _final_margin_bottom(0)
, _final_border_radius_left_top(0)
, _final_border_radius_right_top(0)
, _final_border_radius_right_bottom(0)
, _final_border_radius_left_bottom(0)
, _raw_client_width(0)
, _raw_client_height(0)
, _limit()
, horizontal_active_mark_value(0)
, vertical_active_mark_value(0)
, _linenum(0)
, _newline(false)
, _clip(false)
, _explicit_width(false)
, _explicit_height(false)
, _is_draw_border(false)
, _is_draw_border_radius(false)
{
}

Box::~Box() {
	_inl(this)->remove_background();
}

void Box::remove() {
	_inl(this)->remove_background();
	Layout::remove();
}

void Box::set_parent(View* parent) throw(Error) {
	Layout::set_parent(parent);
	_linenum = 0;
}

/**
 * @func overlap_test 重叠测试,测试屏幕上的点是否与视图重叠
 */
bool Box::overlap_test(Vec2 point) {
	return View::overlap_test_from_convex_quadrilateral( _final_vertex, point );
}

Vec2 Box::layout_offset() {
	// 经过布局计算得到的偏移值
	
	float offset_start_x = _offset_start.x();
	float offset_start_y = _offset_start.y();
	
	if ( _parent_layout ) {
		
		Hybrid* hybrid = _parent_layout->as_hybrid();
		
		if ( hybrid ) { // Hybrid inner layout
			
			if ( _linenum != -1 ) {
				
				TextRows::Row& row = hybrid->rows()[_linenum];
				
				switch ( hybrid->text_align() ) {
					default: /* LEFT */ break;
					case TextAlign::CENTER:
						offset_start_x += (hybrid->_final_width - row.offset_end.x()) / 2.0;
						break;
					case TextAlign::RIGHT:
						offset_start_x += (hybrid->_final_width - row.offset_end.x());
						break;
					case TextAlign::LEFT_REVERSE:
						offset_start_x = _inl(this)->get_horizontal_reverse_offset(hybrid->_final_width);
						offset_start_x -= (hybrid->_final_width - row.offset_end.x());
						break;
					case TextAlign::CENTER_REVERSE:
						offset_start_x = _inl(this)->get_horizontal_reverse_offset(hybrid->_final_width);
						offset_start_x -= (hybrid->_final_width - row.offset_end.x()) / 2.0;
						break;
					case TextAlign::RIGHT_REVERSE:
						offset_start_x = _inl(this)->get_horizontal_reverse_offset(hybrid->_final_width);
						break;
				}
				offset_start_y = row.baseline - _raw_client_height; // 底部对齐文本行基线
			}
			
		} else { // Div inner layout
			Div* div = static_cast<Div*>(_parent_layout);
			
			switch ( div->content_align() ) {
				case ContentAlign::RIGHT: // horizontal reverse
					offset_start_x = _inl(this)->get_horizontal_reverse_offset(div->_final_width);
					break;
				case ContentAlign::BOTTOM: // vertical reverse
					offset_start_y = _inl(this)->get_vertical_reverse_offset(div->_final_height);
					break;
				default: break;
			}
		}
	}
	
	return Vec2(offset_start_x + _final_margin_left + _border_left_width,
							offset_start_y + _final_margin_top + _border_top_width);
}

CGRect Box::screen_rect() {
	final_matrix();
	compute_box_vertex(_final_vertex);
	return View::screen_rect_from_convex_quadrilateral(_final_vertex);
}

void Box::compute_box_vertex(Vec2 vertex[4]) {
	Vec2 start(-_border_left_width - _origin.x(), -_border_top_width - _origin.y() );
	Vec2 end  (_final_width  + _border_right_width - _origin.x(),
						 _final_height + _border_bottom_width - _origin.y() );
	vertex[0] = _final_matrix * start;
	vertex[1] = _final_matrix * Vec2(end.x(), start.y());
	vertex[2] = _final_matrix * end;
	vertex[3] = _final_matrix * Vec2(start.x(), end.y());
}

Region Box::get_screen_region() {
	return screen_region_from_convex_quadrilateral(_final_vertex);
}

void Box::solve() {
	View::solve();
	
	uint mark_value = this->mark_value;
	
	// if ( mark_value & View::M_BACKGROUND_COLOR ) { // 背景颜色
	// }
	// if (mark_value & (M_BACKGROUND | M_TRANSFORM | M_SHAPE)) { // 背景
	// }
	if ( mark_value & View::M_BORDER ) { // 边框
		_is_draw_border = (
			_border_left_width != 0 ||
			_border_right_width  != 0 ||
			_border_top_width  != 0 ||
			_border_bottom_width != 0
		);
		mark_value |= Box::M_BORDER_RADIUS; // 边框会影响圆角
	}
	
	// 形状变化包括M_SHAPE (width、height、border、margin, 设置顶点数据), 这个会影响圆角
	if ( mark_value & (View::M_BORDER_RADIUS | View::M_SHAPE) ) { // 圆角标记
		float w = (_final_width + _border_left_width + _border_right_width) / 2.0;
		float h = (_final_height + _border_top_width + _border_bottom_width) / 2.0;
		float max = FX_MIN(w, h);
		_final_border_radius_left_top = FX_MIN(_border_radius_left_top, max);
		_final_border_radius_right_top = FX_MIN(_border_radius_right_top, max);
		_final_border_radius_right_bottom = FX_MIN(_border_radius_right_bottom, max);
		_final_border_radius_left_bottom = FX_MIN(_border_radius_left_bottom, max);
		_is_draw_border_radius = (
			_final_border_radius_left_top != 0 ||
			_final_border_radius_right_top != 0 ||
			_final_border_radius_right_bottom != 0 ||
			_final_border_radius_left_bottom != 0
		);
	}
}

void Box::draw(Draw* draw) {
	if ( _visible ) {
		if ( mark_value ) {
			solve();
		}
		draw->draw(this);
		mark_value = M_NONE;
	}
}

void Box::set_draw_visible() {
	
	_draw_visible = false;
	
	if ( _linenum == -1 ) { return; } // 没有布局,不显示在屏幕上
	
	compute_box_vertex(_final_vertex);
	
	/*
	 * 这里考虑到性能不做精确的多边形重叠测试，只测试图形在横纵轴是否与当前绘图区域是否为重叠。
	 * 这种模糊测试在大多数时候都是正确有效的。
	 */
	Region dre = display_port()->draw_region();
	Region re = get_screen_region();
	
	if (FX_MAX( dre.y2, re.y2 ) - FX_MIN( dre.y, re.y ) <= re.h + dre.h &&
			FX_MAX( dre.x2, re.x2 ) - FX_MIN( dre.x, re.x ) <= re.w + dre.w
	) {
		_draw_visible = true;
	}
}

View* Box::append_text(cUcs2String& str) throw(Error) {
	Ucs2String str2 = str.trim();
	Text* text = new Text();
	text->set_value(str2);
	append(text);
	return text;
}

void Box::set_width(Value value) {
	_width = value;
	mark_pre(M_SHAPE | M_LAYOUT | M_SIZE_HORIZONTAL);
	set_horizontal_active_mark();
}

void Box::set_height(Value value) {
	_height = value;
	mark_pre(M_SHAPE | M_LAYOUT | M_SIZE_VERTICAL);
	set_vertical_active_mark();
}

void Box::set_margin(Value value) {
	value.value = FX_MAX(value.value, 0);
	_margin_top = value;
	_margin_right = value;
	_margin_bottom = value;
	_margin_left = value;
	mark_pre(M_MATRIX | M_SHAPE | M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL);
	set_horizontal_active_mark();
	set_vertical_active_mark();
}

void Box::set_margin_top(Value value) {
	value.value = FX_MAX(value.value, 0);
	_margin_top = value;
	mark_pre(M_MATRIX | M_SHAPE | M_LAYOUT | M_SIZE_VERTICAL);
	set_vertical_active_mark();
}

void Box::set_margin_right(Value value) {
	value.value = FX_MAX(value.value, 0);
	_margin_right = value;
	mark_pre(M_SHAPE | M_LAYOUT | M_SIZE_HORIZONTAL);
	set_horizontal_active_mark();
}

void Box::set_margin_bottom(Value value) {
	value.value = FX_MAX(value.value, 0);
	_margin_bottom = value;
	mark_pre(M_SHAPE | M_LAYOUT | M_SIZE_VERTICAL);
	set_vertical_active_mark();
}

void Box::set_margin_left(Value value) {
	value.value = FX_MAX(value.value, 0);
	_margin_left = value;
	mark_pre(M_MATRIX | M_SHAPE | M_LAYOUT | M_SIZE_HORIZONTAL);
	set_horizontal_active_mark();
}

void Box::set_border(Border value) {
	float width = FX_MAX(value.width, 0);
	_border_top_color = value.color;
	_border_right_color = value.color;
	_border_bottom_color = value.color;
	_border_left_color = value.color;
	_border_top_width = width;
	_border_right_width = width;
	_border_bottom_width = width;
	_border_left_width = width;
	mark_pre(M_MATRIX | M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL);
}

void Box::set_border_top(Border value) {
	_border_top_width = FX_MAX(value.width, 0);
	_border_top_color = value.color;
	mark_pre(M_MATRIX | M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_VERTICAL);
}

void Box::set_border_right(Border value) {
	_border_right_width = FX_MAX(value.width, 0);
	_border_right_color = value.color;
	mark_pre(M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_HORIZONTAL);
}

void Box::set_border_bottom(Border value) {
	_border_bottom_width = FX_MAX(value.width, 0);
	_border_bottom_color = value.color;
	mark_pre(M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_VERTICAL);
}

void Box::set_border_left(Border value) {
	_border_left_width = FX_MAX(value.width, 0);
	_border_left_color = value.color;
	mark_pre(M_MATRIX | M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_HORIZONTAL);
}

void Box::set_border_width(float value) {
	value = FX_MAX(value, 0);
	_border_top_width = value;
	_border_right_width = value;
	_border_bottom_width = value;
	_border_left_width = value;
	mark_pre(M_MATRIX | M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL);
}

void Box::set_border_top_width(float value) {
	value = FX_MAX(value, 0);
	_border_top_width = value;
	mark_pre(M_MATRIX | M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_VERTICAL);
}

void Box::set_border_right_width(float value) {
	value = FX_MAX(value, 0);
	_border_right_width = value;
	mark_pre(M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_HORIZONTAL);
}

void Box::set_border_bottom_width(float value) {
	value = FX_MAX(value, 0);
	_border_bottom_width = value;
	mark_pre(M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_VERTICAL);
}

void Box::set_border_left_width(float value) {
	value = FX_MAX(value, 0);
	_border_left_width = value;
	mark_pre(M_MATRIX | M_SHAPE | M_BORDER | M_LAYOUT | M_SIZE_HORIZONTAL);
}

void Box::set_border_color(Color value) {
	_border_top_color = value;
	_border_right_color = value;
	_border_bottom_color = value;
	_border_left_color = value;
	mark(M_BORDER);
}

void Box::set_border_top_color(Color value) {
	_border_top_color = value;
	mark(M_BORDER);
}

void Box::set_border_right_color(Color value) {
	_border_right_color = value;
	mark(M_BORDER);
}

void Box::set_border_bottom_color(Color value) {
	_border_bottom_color = value;
	mark(M_BORDER);
}

void Box::set_border_left_color(Color value) {
	_border_left_color = value;
	mark(M_BORDER);
}

void Box::set_border_radius(float value) {
	value = FX_MAX(value, 0);
	_border_radius_right_top = value;
	_border_radius_right_bottom = value;
	_border_radius_left_bottom = value;
	_border_radius_left_top = value;
	mark(M_BORDER_RADIUS);
}

void Box::set_border_radius_right_top(float value) {
	value = FX_MAX(value, 0);
	_border_radius_right_top = value;
	mark(M_BORDER_RADIUS);
}

void Box::set_border_radius_right_bottom(float value) {
	value = FX_MAX(value, 0);
	_border_radius_right_bottom = value;
	mark(M_BORDER_RADIUS);
}

void Box::set_border_radius_left_bottom(float value) {
	value = FX_MAX(value, 0);
	_border_radius_left_bottom = value;
	mark(M_BORDER_RADIUS);
}

void Box::set_border_radius_left_top(float value) {
	value = FX_MAX(value, 0);
	_border_radius_left_top = value;
	mark(M_BORDER_RADIUS);
}

void Box::set_background_color(Color value) {
	// FX_DEBUG("color,%d", value.a());
	_background_color = value;
	mark(M_BACKGROUND_COLOR);
}

void Box::set_background(Background* value) {
	_background = Background::assign(_background, value);
	if (_background) {
		_background->set_host(this);
	}
	mark(M_BACKGROUND);
}

void Box::set_visible(bool value) {
	if (_visible != value) {
		View::set_visible(value);
		// 这会影响其它兄弟视图的位置
		mark_pre(M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL);
	}
}

void Box::set_newline(bool value) {
	_newline = value;
	mark_pre(M_LAYOUT | M_SIZE_HORIZONTAL);
}

void Box::set_clip(bool value) {
	if (_clip != value) {
		_clip = value;
		mark(M_CLIP);
	}
}

void Box::set_layout_explicit_size() {
	
	if ( _final_visible ) {
		// 只需要解决 explicit size
		if ( ! _inl(this)->solve_explicit_size<0>() ) {
			return;
		}
	} else {
		//if ( ! (mark_value & (M_SIZE_HORIZONTAL | M_SIZE_VERTICAL)) ) {
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

void Hybrid::set_layout_explicit_size() {
	
	if ( _final_visible ) {
		if ( mark_value & M_TEXT_FONT ) {
			solve_text_layout_mark(); // 文本属性的变化会影响后代文本视图属性
		}
		if ( ! _inl(this)->solve_explicit_size<1>() ) { // 解决明确尺寸
			return;
		}
	} else {
		//if ( ! (mark_value & (M_SIZE_HORIZONTAL | M_SIZE_VERTICAL)) ) {
		if ( ! (mark_value & M_VISIBLE) ) {
			return;
		}
	}
	//  M_SIZE_HORIZONTAL | M_SIZE_VERTICAL
	// 布局尺寸改变可能会影响到所有的兄弟视图的偏移值
	
	Layout* layout = parent()->as_layout();
	if ( layout ) {
		layout->mark_pre(M_CONTENT_OFFSET);
	} else { 
		set_default_offset_value(); // 父视图只是个普通视图,默认将偏移设置为0
	}
}

void Indep::set_layout_explicit_size() {
	
	if ( _final_visible ) {
		// 只需要解决 explicit size
		if ( _inl(this)->solve_explicit_size<0>() ) {
				
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

void Box::solve_explicit_size_after(bool change_horizontal, bool change_vertical, uint child_mark) {
	
	/* 
	 * 有非常大的可能性导致自身位置偏移:
	 * 1.右对齐底部对齐时的尺寸变化
	 * 2.margin-left/margin-top 值变化
	 * 3.border-left/border-top 宽度变化
	 */
	mark(M_MATRIX);

	#define solve_explicit_size_after_(func)  \
	/* 内部的宽度高度发生变化可能会影响到所有的子视图偏移值变化,所以尽可能标记这个变化 */ \
	mark_pre(M_SHAPE | M_CONTENT_OFFSET); /* 改变将影响子布局视图偏移 */ \
	View* view = first(); \
	while (view) { \
		Box* box = view->as_box(); \
		if (box) { uint mark; func; if ( mark ) box->mark_pre(mark); } \
		view = view->next(); \
	}
	
	if ( change_horizontal ) {
		if ( change_vertical ) { // width || height
			solve_explicit_size_after_(
				mark = box->horizontal_active_mark_value |
							 box->vertical_active_mark_value | child_mark;
			);
		} else { // width
			solve_explicit_size_after_(
				mark = box->horizontal_active_mark_value | child_mark;
			);
		}
	} else if ( change_vertical ) { // height
		solve_explicit_size_after_(
			mark = box->vertical_active_mark_value | child_mark;
		);
	}
}

/**
 * @func set_default_offset_value 设置为起始默认偏移值
 */
void Box::set_default_offset_value() {
	_offset_start.x(0);
	_offset_start.y(0);
	_offset_end.x(_raw_client_width);
	_offset_end.y(_raw_client_height);
	_linenum = 0;
}

/**
 * @func set_offset_horizontal 设置水平布局偏移值,并且返回一个可辅助下一个兄弟视图定位的兄弟视图
 * @arg prev {Box*}  上面有效辅助定位的兄弟视图
 * @arg squeeze {float&}  返回能够最大限度挤压父视图尺寸的值
 * @arg limit {float} 限制布局的水平宽度
 * @arg div {Div*} 父布局视图
 */
Box* Box::set_offset_horizontal(Box* prev, Vec2& squeeze, float limit, Div* div) {
	return _inl(this)->set_offset_horizontal_(prev, squeeze, limit, div, 1);
}

/**
 * @func set_offset_vertical 设置垂直布局偏移值,并且返回一个可辅助下一个兄弟视图定位的兄弟视图
 * @arg prev {Box*}  上面有效辅助定位的兄弟视图
 * @arg squeeze {float&}  返回能够最大限度挤压父视图尺寸的值
 * @arg limit {float} 限制布局的水平高度
 * @arg div {Div*} 父布局视图
 */
Box* Box::set_offset_vertical(Box* prev, Vec2& squeeze, float limit, Div* div) {
	return _inl(this)->set_offset_vertical_(prev, squeeze, limit, div, 1);
}

void Box::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
	_inl(this)->set_offset_in_hybrid_(rows, limit, hybrid, 1);
}

Box* Limit::set_offset_horizontal(Box* prev, Vec2& squeeze, float limit, Div* div) {
	return _inl(this)->set_offset_horizontal_(prev, squeeze, limit, div, 0);
}

Box* Limit::set_offset_vertical(Box* prev, Vec2& squeeze, float limit, Div* div) {
	return _inl(this)->set_offset_vertical_(prev, squeeze, limit, div, 0);
}

void Limit::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
	_inl(this)->set_offset_in_hybrid_(rows, limit, hybrid, 0);
}

Box* LimitIndep::set_offset_horizontal(Box* prev, Vec2& squeeze, float limit, Div* div) {
	return _inl(this)->set_offset_horizontal_(prev, squeeze, limit, div, 0);
}

Box* LimitIndep::set_offset_vertical(Box* prev, Vec2& squeeze, float limit, Div* div) {
	return _inl(this)->set_offset_vertical_(prev, squeeze, limit, div, 0);
}

void LimitIndep::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
	_inl(this)->set_offset_in_hybrid_(rows, limit, hybrid, 0);
}

}
