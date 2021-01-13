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

#include "div.h"
#include "hybrid.h"

FX_NS(ftr)

extern void _view_inl__safe_delete_mark(View* view);
extern void _box_inl__solve_final_horizontal_size_with_full_width(Box* box, float parent);
extern void _box_inl__solve_final_vertical_size_with_full_height(Box* box, float parent);

FX_DEFINE_INLINE_MEMBERS(Div, Inl) {
 public:
	
	void set_layout_three_times2(bool horizontal) {
		
		View* view = first();
		while (view) {
			Box* box = view->as_box();
			if ( box ) {
				box->set_layout_three_times(horizontal, false);
				_view_inl__safe_delete_mark(box);
			}
			view = view->next();
		}
	}
	
};

Div::Div(): _content_align(ContentAlign::LEFT) {
}

void Div::set_content_align(ContentAlign value) {
	if (value != _content_align) {
		_content_align = value;
		mark_pre(M_CONTENT_OFFSET);
	}
}

bool Div::set_div_content_offset(Vec2& squeeze, Vec2 limit_min) {
	
	# define loop(func)  \
	while (view) {  \
		Box* box = view->as_box(); \
		if (box) { func; }  \
		view = view->next();  \
 	}
	
	View* view  = first();
	Box*  prev  = nullptr;
	
	switch ( _content_align ) {
		case ContentAlign::LEFT:
		case ContentAlign::RIGHT:
			loop( prev = box->set_offset_horizontal(prev, squeeze, _limit.width(), this) );
			break;
		default:
			loop( prev = box->set_offset_vertical(prev, squeeze, _limit.height(), this) );
			break;
	}
	
	# undef loop
	
	bool size_change = false;
	
	// 没有明确宽度
	if ( ! _explicit_width ) {
		
		if ( squeeze.width() > _limit.width() ) {// 限制宽度
			squeeze.width(_limit.width());
		} else if ( limit_min.width() > squeeze.width() ) {
			squeeze.width(limit_min.width());
		}
		
		if ( _final_width != squeeze.width() ) { // 宽度发生改变
			_final_width       = squeeze.width();
			_raw_client_width  = _final_margin_left + _final_margin_right +
														_border_left_width + _border_right_width + _final_width;
			size_change = true;
		}
	}
	
	// 没有明确高度,高度会受到子视图的挤压
	if ( ! _explicit_height ) {
		
		if ( squeeze.height() > _limit.height() ) { // 限制高度
			squeeze.height(_limit.height());
		} else if ( limit_min.height() > squeeze.height() ) {
			squeeze.height(limit_min.height());
		}
		
		if ( _final_height != squeeze.height() ) { // 高度发生改变
			_final_height      = squeeze.height();
			_raw_client_height = _final_margin_top + _final_margin_bottom +
														_border_top_width + _border_bottom_width + _final_height;
			size_change = true;
		}
	}
	
	return size_change;
}

void Div::set_layout_content_offset() {
	
	if (_final_visible) {
		
		Vec2 squeeze;
		
		if ( set_div_content_offset(squeeze, Vec2()) ) { //
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

void Div::set_layout_three_times(bool horizontal, bool hybrid) {
	
	if ( !_visible ) { return; }
	
	ASSERT(_parent_layout);
	
	if ( horizontal ) { // horizontal layout
		
		if ( _width.type == ValueType::FULL ) {
			float width = static_cast<Box*>(_parent_layout)->_final_width;
			float raw_final_width = _final_width;
			
			_box_inl__solve_final_horizontal_size_with_full_width(this, width);
			
			if ( raw_final_width != _final_width ) {
				mark_pre(M_SHAPE);
				
				if ( hybrid ) { // update row offset
					ASSERT( _linenum != -1 );
					static_cast<Hybrid*>(_parent_layout)->_rows[_linenum].offset_end.x(_offset_end.x());
				}
				
				if (_content_align == ContentAlign::LEFT ||
						_content_align == ContentAlign::RIGHT ) {
					Inl_Div(this)->set_layout_three_times2(true);
				}
			}
		}
	} else {
		if ( _height.type == ValueType::FULL ) {
			float parent = static_cast<Box*>(_parent_layout)->_final_height;
			float raw_final_height = _final_height;
			
			_box_inl__solve_final_vertical_size_with_full_height(this, parent);
			
			if ( raw_final_height != _final_height ) {
				mark_pre(M_SHAPE);
				if (_content_align == ContentAlign::TOP ||
						_content_align == ContentAlign::BOTTOM) {
					Inl_Div(this)->set_layout_three_times2(false);
				}
			}
		}
	}
}

FX_END
