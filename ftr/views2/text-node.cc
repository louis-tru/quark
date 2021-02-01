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

#include <math.h>
#include "text-node.h"
#include "ftr/util/codec.h"
#include "text-rows.h"
#include "hybrid.h"
#include "app.h"
#include "display-port.h"

namespace ftr {

#define revoke_mark_value(mark_value, mark) mark_value &= ~(mark)

/**
 * @class TextNode::Inl
 */
class TextNode::Inl: public TextNode {
 public:
#define _inl(self) static_cast<TextNode::Inl*>(self)

	/**
	 * @func set_offset
	 */
	template<TextAlign T> inline void set_offset() {
		
		float offset_start_x = Float::max, offset_end_x = Float::min;
		
		Hybrid* hybrid = static_cast<Hybrid*>(_parent_layout);
		TextRows& rows = hybrid->rows();
		float final_width = hybrid->final_width(), start, end;
		
		for ( auto& i : _data.cells ) {
			Cell& cell = i.value();
			
			TextRows::Row& row = rows[cell.line_num];
			cell.baseline = row.baseline;
			
			switch (T) {
				default: break;
				case TextAlign::CENTER:
					cell.offset_start = (final_width - row.offset_end.x()) / 2.0;
					break;
				case TextAlign::RIGHT:
					cell.offset_start = final_width - row.offset_end.x();
					break;
				case TextAlign::LEFT_REVERSE:
					cell.reverse = 1;
					cell.offset_start = row.offset_end.x();
					break;
				case TextAlign::CENTER_REVERSE:
					cell.reverse = 1;
					cell.offset_start = (final_width - row.offset_end.x()) / 2.0 + row.offset_end.x();
					break;
				case TextAlign::RIGHT_REVERSE:
					cell.reverse = 1;
					cell.offset_start = final_width;
					break;
			}
			
			if (T == TextAlign::LEFT_REVERSE ||
					T == TextAlign::CENTER_REVERSE || T == TextAlign::RIGHT_REVERSE ) { // reverse
				end = cell.offset_start + cell.offset[0];
				start = end - (cell.offset[cell.Chars.length()] - cell.offset[0]);
			} else {
				start = cell.offset_start + cell.offset[0];
				end = cell.offset_start + cell.offset[cell.Chars.length()];
			}
			
			if ( start < offset_start_x ) { offset_start_x = start; }
			if ( end > offset_end_x ) { offset_end_x = end; }
		}
		
		_offset_start.x(offset_start_x);
		_offset_end.x(offset_end_x);
	}

	/**
	 * @func set_layout_offset_and_cell_offset_start
	 */
	void set_layout_offset_and_cell_offset_start() {
		if ( _data.cells.length() ) {
			
			Hybrid* hybrid = _parent_layout ? _parent_layout->as_hybrid() : nullptr;
			
			if ( hybrid ) {
				switch ( hybrid->text_align() ) {
					default:
						set_offset<TextAlign::LEFT>(); break;
					case TextAlign::CENTER:
						set_offset<TextAlign::CENTER>(); break;
					case TextAlign::RIGHT:
						set_offset<TextAlign::RIGHT>(); break;
					case TextAlign::LEFT_REVERSE:
						set_offset<TextAlign::LEFT_REVERSE>(); break;
					case TextAlign::CENTER_REVERSE:
						set_offset<TextAlign::CENTER_REVERSE>(); break;
					case TextAlign::RIGHT_REVERSE:
						set_offset<TextAlign::RIGHT_REVERSE>(); break;
				}
			} else { // 非Text视图内的文本布局
				float offset_start_x = Float::max, offset_end_x = Float::min;
				
				for ( auto& i : _data.cells ) {
					float start = i.value().offset[0];
					float end = i.value().offset[i.value().Chars.length()];
					if ( start < offset_start_x ) { offset_start_x = start; }
					if ( end > offset_end_x ) { offset_end_x = end; }
				}
				_offset_start.x(offset_start_x);
				_offset_end.x(offset_end_x);
			}
		} else {
			_offset_start = _offset_end = Vec2();
		}
	}
	
	/**
	 * @func compute_final_vertex
	 */
	void compute_box_vertex(Vec2* final_vertex) {
		Vec2 start( -_origin.x(), -_origin.y() );
		Vec2 end  (_offset_end.x() - _offset_start.x() - _origin.x(),
							 _offset_end.y() - _offset_start.y() - _origin.y() );
		final_vertex[0] = _final_matrix * start;
		final_vertex[1] = _final_matrix * Vec2(end.x(), start.y());
		final_vertex[2] = _final_matrix * end;
		final_vertex[3] = _final_matrix * Vec2(start.x(), end.y());
	}
	
};

TextNode::TextNode(): _valid_layout_offset(false) { }

void TextNode::prepend(View* child) throw(Error) {
	FX_ERR("%s", "Error: TextNode can not have a child view");
}

void TextNode::append(View* child) throw(Error) {
	FX_ERR("%s", "Error: TextNode can not have a child view");
}

void TextNode::accept_text(Ucs2StringBuilder& output) const {
	output.push(_data.string);
}

View* TextNode::append_text(cUcs2String& str) throw(Error) {
	_data.string.push(str);
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
	return nullptr;
}

/**
 * @set value
 */
void TextNode::set_value(cUcs2String& str) {
	_data.string = str;
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
}

void TextNode::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
	
	_parent_layout = hybrid;
	_valid_layout_offset = false;
	
	_data.cells.clear(); // 清空旧布局
	_data.cell_draw_begin = _data.cell_draw_end = 0;
	
	if ( !_visible || _data.string.is_empty() || rows->clip() ) {
		return;
	}
	
	mark( M_MATRIX | M_SHAPE ); // 标记变换
	
	_offset_start = _offset_end = Vec2(rows->last()->offset_end.x(),
																			 rows->last()->offset_start.y());
	
	Options opts = get_options(hybrid);
	
	// text layout ..
	set_text_layout_offset(rows, limit, _data, &opts);
	
	if ( _data.cells.length() ) {
		_offset_end = rows->last()->offset_end;
	}
}

void TextNode::draw(Draw* draw) {
	if ( _visible ) {
		
		if ( mark_value ) {
			
			solve();
			
			if ( mark_value & (M_TRANSFORM | M_TEXT_SIZE) ) {
				set_glyph_texture_level(_data);
			}
		}
		
		draw->draw(this);
		
		mark_value = View::M_NONE;
	}
}

Vec2 TextNode::layout_offset() {
	if ( !_valid_layout_offset ) {
		_valid_layout_offset = true;
		_inl(this)->set_layout_offset_and_cell_offset_start();
	}
	return _offset_start;
}

/**
 * @overwrite
 */
void TextNode::set_layout_three_times(bool horizontal, bool hybrid) {
	if ( _visible ) {
		if ( hybrid ) {
			_valid_layout_offset = false;
		}
	}
}

bool TextNode::overlap_test(Vec2 point) {
	return View::overlap_test_from_convex_quadrilateral( _final_vertex, point );
}

CGRect TextNode::screen_rect() {
	final_matrix();
	_inl(this)->compute_box_vertex(_final_vertex);
	return View::screen_rect_from_convex_quadrilateral(_final_vertex);
}

/**
 * @func set_draw_visible
 */
void TextNode::set_draw_visible() {
	_inl(this)->compute_box_vertex(_final_vertex);
	
	_draw_visible =
	
	compute_text_visible_draw(_final_vertex, _data,
														0, _offset_end.x() - _offset_start.x(), _offset_start.y());
}

}
