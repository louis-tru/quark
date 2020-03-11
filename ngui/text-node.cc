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
#include "nxkit/codec.h"
#include "text-rows.h"
#include "hybrid.h"
#include "app.h"
#include "display-port.h"

NX_NS(ngui)

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
		
		Hybrid* hybrid = static_cast<Hybrid*>(m_parent_layout);
		TextRows& rows = hybrid->rows();
		float final_width = hybrid->final_width(), start, end;
		
		for ( auto& i : m_data.cells ) {
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
				start = end - (cell.offset[cell.chars.length()] - cell.offset[0]);
			} else {
				start = cell.offset_start + cell.offset[0];
				end = cell.offset_start + cell.offset[cell.chars.length()];
			}
			
			if ( start < offset_start_x ) { offset_start_x = start; }
			if ( end > offset_end_x ) { offset_end_x = end; }
		}
		
		m_offset_start.x(offset_start_x);
		m_offset_end.x(offset_end_x);
	}

	/**
	 * @func set_layout_offset_and_cell_offset_start
	 */
	void set_layout_offset_and_cell_offset_start() {
		if ( m_data.cells.length() ) {
			
			Hybrid* hybrid = m_parent_layout ? m_parent_layout->as_hybrid() : nullptr;
			
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
				
				for ( auto& i : m_data.cells ) {
					float start = i.value().offset[0];
					float end = i.value().offset[i.value().chars.length()];
					if ( start < offset_start_x ) { offset_start_x = start; }
					if ( end > offset_end_x ) { offset_end_x = end; }
				}
				m_offset_start.x(offset_start_x);
				m_offset_end.x(offset_end_x);
			}
		} else {
			m_offset_start = m_offset_end = Vec2();
		}
	}
	
	/**
	 * @func compute_final_vertex
	 */
	void compute_box_vertex(Vec2* final_vertex) {
		Vec2 start( -m_origin.x(), -m_origin.y() );
		Vec2 end  (m_offset_end.x() - m_offset_start.x() - m_origin.x(),
							 m_offset_end.y() - m_offset_start.y() - m_origin.y() );
		final_vertex[0] = m_final_matrix * start;
		final_vertex[1] = m_final_matrix * Vec2(end.x(), start.y());
		final_vertex[2] = m_final_matrix * end;
		final_vertex[3] = m_final_matrix * Vec2(start.x(), end.y());
	}
	
};

TextNode::TextNode(): m_valid_layout_offset(false) { }

void TextNode::prepend(View* child) throw(Error) {
	NX_ERR("%s", "Error: TextNode can not have a child view");
}

void TextNode::append(View* child) throw(Error) {
	NX_ERR("%s", "Error: TextNode can not have a child view");
}

void TextNode::accept_text(Ucs2StringBuilder& output) const {
	output.push(m_data.string);
}

View* TextNode::append_text(cUcs2String& str) throw(Error) {
	m_data.string.push(str);
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
	return nullptr;
}

/**
 * @set value
 */
void TextNode::set_value(cUcs2String& str) {
	m_data.string = str;
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
}

void TextNode::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
	
	m_parent_layout = hybrid;
	m_valid_layout_offset = false;
	
	m_data.cells.clear(); // 清空旧布局
	m_data.cell_draw_begin = m_data.cell_draw_end = 0;
	
	if ( !m_visible || m_data.string.is_empty() || rows->clip() ) {
		return;
	}
	
	mark( M_MATRIX | M_SHAPE ); // 标记变换
	
	m_offset_start = m_offset_end = Vec2(rows->last()->offset_end.x(),
																			 rows->last()->offset_start.y());
	
	Options opts = get_options(hybrid);
	
	// text layout ..
	set_text_layout_offset(rows, limit, m_data, &opts);
	
	if ( m_data.cells.length() ) {
		m_offset_end = rows->last()->offset_end;
	}
}

void TextNode::draw(Draw* draw) {
	if ( m_visible ) {
		
		if ( mark_value ) {
			
			solve();
			
			if ( mark_value & (M_TRANSFORM | M_TEXT_SIZE) ) {
				set_glyph_texture_level(m_data);
			}
		}
		
		draw->draw(this);
		
		mark_value = View::M_NONE;
	}
}

Vec2 TextNode::layout_offset() {
	if ( !m_valid_layout_offset ) {
		m_valid_layout_offset = true;
		_inl(this)->set_layout_offset_and_cell_offset_start();
	}
	return m_offset_start;
}

/**
 * @overwrite
 */
void TextNode::set_layout_three_times(bool horizontal, bool hybrid) {
	if ( m_visible ) {
		if ( hybrid ) {
			m_valid_layout_offset = false;
		}
	}
}

bool TextNode::overlap_test(Vec2 point) {
	return View::overlap_test_from_convex_quadrilateral( m_final_vertex, point );
}

CGRect TextNode::screen_rect() {
	final_matrix();
	_inl(this)->compute_box_vertex(m_final_vertex);
	return View::screen_rect_from_convex_quadrilateral(m_final_vertex);
}

/**
 * @func set_draw_visible
 */
void TextNode::set_draw_visible() {
	_inl(this)->compute_box_vertex(m_final_vertex);
	
	m_draw_visible =
	
	compute_text_visible_draw(m_final_vertex, m_data,
														0, m_offset_end.x() - m_offset_start.x(), m_offset_start.y());
}

NX_END
