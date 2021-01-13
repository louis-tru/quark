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

#include "hybrid.h"
#include "text.h"
#include "text-node.h"
#include "ftr/util/codec.h"

FX_NS(ftr)

extern void _view_inl__safe_delete_mark(View* v);
extern void _box_inl__solve_final_horizontal_size_with_full_width(Box* box, float parent);
extern void _box_inl__solve_final_vertical_size_with_full_height(Box* box, float parent);

Hybrid::Hybrid()
: TextLayout()
, _text_align(TextAlign::LEFT) {
	
}

View* Hybrid::append_text(cUcs2String& str) throw(Error) {
	TextNode* text = new TextNode();
	text->set_value( str );
	append(text);
	return text;
}

void Hybrid::set_text_align(TextAlign value) {
	if (value != _text_align) {
		_text_align = value;
		mark_pre( M_CONTENT_OFFSET );
	}
}

void Hybrid::set_visible(bool value) {
	if (_visible != value) { 
		View::set_visible(value);
		// 这会影响其它兄弟视图的位置
		mark_pre( M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL/* | M_TEXT_FONT*/ );
	}
}

void Hybrid::set_layout_content_offset() {
	
	if ( _final_visible ) {
		
		_rows.reset();
		
		View* view = first();
		
		while ( view ) {
			Layout* layout = view->as_layout();
			if (layout) {
				// LOG("BBBB-0, %f", _height.value);
				layout->set_offset_in_hybrid(&_rows, _limit, this);
			}
			view = view->next();
		}
		
		set_layout_content_offset_after();
	}
}

void Hybrid::set_layout_three_times(bool horizontal, bool hybrid) {
	
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
				
				View* view = first();
				while (view) {
					Layout* layout = view->as_layout();
					if ( layout ) {
						layout->set_layout_three_times(horizontal, true);
						_view_inl__safe_delete_mark(layout);
					}
					view = view->next();
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
			}
		}
	}
}

void Hybrid::set_layout_content_offset_after() {
	
	TextRows* rows = &_rows;
	
	// 检查最后行x结束位置是否为最大挤压宽度
	rows->set_width(rows->last()->offset_end.x());
	
	bool size_change = false; // 是否被挤压
	
	if ( ! _explicit_width ) { // 没有明确宽度
		
		float max_width = rows->max_width();
		if ( max_width > _limit.width() ) { // 限制宽度
			max_width = _limit.width();
		}
		
		if (_final_width != max_width) { // 宽度发生改变
			_final_width       = max_width;
			_raw_client_width  = _final_margin_left + _final_margin_right +
														_border_left_width + _border_right_width + _final_width;
			size_change = true;
		}
	}
	
	if ( ! _explicit_height ) { // 没有明确高度,高度会受到子视图的挤压
		
		float max_height = rows->max_height();
		if ( max_height > _limit.height() ) { // 限制高度
			max_height = _limit.height();
		}
		
		if (_final_height != max_height) { // 高度发生改变
			_final_height      = max_height;
			_raw_client_height = _final_margin_top + _final_margin_bottom +
														_border_top_width + _border_bottom_width + _final_height;
			size_change = true;
		}
	}

	if ( size_change ) { //
		mark( M_SHAPE ); // 高度或宽度被挤压即形状发生变化
		
		// 被挤压会影响到所有的兄弟视图的偏移值, 所以标记父视图 M_CONTENT_OFFSET
		Layout* layout = parent()->as_layout();
		if (layout) {
			//LOG("AAA-1, %f", _height.value);
			layout->mark_pre( M_CONTENT_OFFSET );
		} else { // 父视图只是个普通视图,默认将偏移设置为0
			set_default_offset_value();
		}
	}
}

#undef loop
FX_END
