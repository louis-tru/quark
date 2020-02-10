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

#include "label.h"
#include "layout.h"
#include "app.h"
#include "display-port.h"
#include "text-rows.h"

XX_NS(ngui)

class Label::Inl: public Label {
public:
#define _inl(self) static_cast<Label::Inl*>(self)
	
	template<TextAlign T>
	XX_INLINE void end_line(Cell& cell, float baseline, float descender, float max_offset) {
		
		uint count = cell.offset.length() - 1;
		if ( count ) { /* 结束上一行 */
			switch (T) {
				default: cell.offset_start = 0; break; // LEFT
				case TextAlign::CENTER:
					cell.offset_start = -max_offset / 2.0;
					break;
				case TextAlign::RIGHT:
					cell.offset_start = -max_offset;
					break;
				case TextAlign::LEFT_REVERSE:
					cell.reverse = 1;
					cell.offset_start = max_offset;
					break;
				case TextAlign::CENTER_REVERSE:
					cell.reverse = 1;
					cell.offset_start = max_offset / 2.0;
					break;
				case TextAlign::RIGHT_REVERSE:
					cell.reverse = 1;
					cell.offset_start = max_offset;
					break;
			}
			cell.baseline = baseline;
			m_data.cells.push(move(cell));
			cell.offset.push(0);     /* 添加第一个初始位置 */
		}
		// set box size
		if ( max_offset > m_box_size.width() ) {
			m_box_size.width(max_offset);
			m_box_offset_start = cell.offset_start;
		}
		m_box_size.height(baseline + descender);
	}
	
	/**
	 * @func set_content_offset
	 */
	template<TextAlign T> void set_layout_content_offset() {
		
		m_data.cells.clear(); // 清空旧布局
		m_box_size = Vec2();
		m_box_offset_start = 0;
		m_data.cell_draw_begin = m_data.cell_draw_end = 0;
		
		if ( m_data.string.is_empty() ) { return; }
		
		// 在这里进行文本排版布局
		
		FontGlyphTable* table = get_font_glyph_table_and_height(m_data, m_text_line_height.value);
		float ascender = m_data.text_ascender;    // 行顶与基线的距离
		float descender = m_data.text_descender;  // 基线与行底的距离
		float ratio = 4096.0 / m_text_size.value; /* 64.0 * 64.0 = 4096.0 */
		
		float baseline = ascender;
		float offset = 0;
		uint  line = 0;
		uint begin = 0;
		uint end = m_data.string.length();
		
		Cell cell = {
			0, 0, 0, 0, Array<float>(), Array<uint16>(), 0
		};
		cell.offset.push(0);
		
		while ( begin < end ) {
			uint16 unicode = m_data.string[begin];
			
			if ( unicode == 0x0A ) { // \n 换行
				uint count = cell.offset.length() - 1;
				
				end_line<T>( cell, baseline, descender, offset );
				
				baseline += ascender + descender; /* 增加新行 */
				line++;
				/* 重置新行数据 */
				cell.line_num = line; /* 文本单元所在的行 */
				cell.begin = begin;
				offset = 0;
				
			} else {
				float hori_advance = table->glyph(unicode)->hori_advance() / ratio; // 字符宽度
				offset += hori_advance;
				cell.offset.push(offset);
				cell.chars.push(unicode);
			}
			begin++;
		}
		
		end_line<T>( cell, baseline, descender, offset );
	}
	
	/**
	 * @func compute_final_vertex
	 */
	void compute_final_vertex(Vec2* final_vertex) {
		Vec2 A, B, C, D; // rect vertex
		Vec2 start(m_box_offset_start - m_origin.x(), -m_origin.y());
		Vec2 end  (m_box_offset_start + m_box_size.width() - m_origin.x(),
							 m_box_size.height() - m_origin.y() );
		
		final_vertex[0] = m_final_matrix * start;
		final_vertex[1] = m_final_matrix * Vec2(end.x(), start.y());
		final_vertex[2] = m_final_matrix * end;
		final_vertex[3] = m_final_matrix * Vec2(start.x(), end.y());
	}
	
};

Label::Label()
: m_text_align(TextAlign::LEFT)
, m_box_offset_start(0)
{
	m_need_draw = false;
}

/**
 * @overwrite
 */
void Label::prepend(View* child) throw(Error) {
	XX_ERR("%s", "Error: Label can not have a child view");
}

/**
 * @overwrite
 */
void Label::append(View* child) throw(Error) {
	XX_ERR("%s", "Error: Label can not have a child view");
}

/**
 * @overwrite
 */
View* Label::append_text(cUcs2String& str) throw(Error) {
	m_data.string.push(str);
	mark( Layout::M_CONTENT_OFFSET );
	return nullptr;
}

/**
 * @set set_value
 */
void Label::set_value(cUcs2String& str) {
	m_data.string = str;
	mark( Layout::M_CONTENT_OFFSET );
}

/**
 * @overwrite
 */
void Label::mark_text(uint value) {
	mark(value);
}

/**
 * @overwrite
 */
void Label::accept_text(Ucs2StringBuilder& out) const {
	out.push(m_data.string);
}

/**
 * @set text_align
 */
void Label::set_text_align(TextAlign value) {
	if (value != m_text_align) {
		m_text_align = value;
		mark( M_CONTENT_OFFSET );
	}
}

/**
 * @overwrite
 */
void Label::draw(Draw* draw) {
	if ( m_visible ) {
		
		if ( mark_value ) {
			
			if ( mark_value & M_TEXT_FONT ) {
				if (m_text_background_color.type == TextValueType::INHERIT) {
					m_text_background_color.value = app()->default_text_background_color().value;
				}
				if (m_text_color.type == TextValueType::INHERIT) {
					m_text_color.value = app()->default_text_color().value;
				}
				if (m_text_size.type == TextValueType::INHERIT) {
					m_text_size.value = app()->default_text_size().value;
				}
				if (m_text_style.type == TextValueType::INHERIT) {
					m_text_style.value = app()->default_text_style().value;
				}
				if (m_text_family.type == TextValueType::INHERIT) {
					m_text_family.value = app()->default_text_family().value;
				}
				if (m_text_line_height.type == TextValueType::INHERIT) {
					m_text_line_height.value = app()->default_text_line_height().value;
				}
				if (m_text_shadow.type == TextValueType::INHERIT) {
					m_text_shadow.value = app()->default_text_shadow().value;
				}
				if (m_text_decoration.type == TextValueType::INHERIT) {
					m_text_decoration.value = app()->default_text_decoration().value;
				}
			}
			
			if ( mark_value & Layout::M_CONTENT_OFFSET ) {
				mark_value |= (M_SHAPE); // 标记 M_SHAPE 是为了调用 set_draw_visible()
				switch (m_text_align) {
					case TextAlign::LEFT:
						_inl(this)->set_layout_content_offset<TextAlign::LEFT>(); break;
					case TextAlign::CENTER:
						_inl(this)->set_layout_content_offset<TextAlign::CENTER>(); break;
					case TextAlign::RIGHT:
						_inl(this)->set_layout_content_offset<TextAlign::RIGHT>(); break;
					case TextAlign::LEFT_REVERSE:
						_inl(this)->set_layout_content_offset<TextAlign::LEFT_REVERSE>(); break;
					case TextAlign::CENTER_REVERSE:
						_inl(this)->set_layout_content_offset<TextAlign::CENTER_REVERSE>(); break;
					case TextAlign::RIGHT_REVERSE:
						_inl(this)->set_layout_content_offset<TextAlign::RIGHT_REVERSE>(); break;
				}
			}
			
			solve();
			
			if ( mark_value & (M_TRANSFORM | M_TEXT_SIZE) ) {
				set_glyph_texture_level(m_data);
			}
		}
		
		draw->draw(this);
		
		mark_value = M_NONE;
	}
}

/**
 * @overwrite
 */
bool Label::overlap_test(Vec2 point) {
	return View::overlap_test_from_convex_quadrilateral( m_final_vertex, point );
}

/**
 * @overwrite
 */
CGRect Label::screen_rect() {
	final_matrix();
	_inl(this)->compute_final_vertex(m_final_vertex);
	return View::screen_rect_from_convex_quadrilateral(m_final_vertex);
}

/**
 * @func set_draw_visible
 */
void Label::set_draw_visible() {
	
	_inl(this)->compute_final_vertex(m_final_vertex);
	
	m_draw_visible =
	
	compute_text_visible_draw(m_final_vertex,
														m_data,
														-m_box_offset_start,
														m_box_offset_start + m_box_size.width(), 0);
}

/**
 * @overwrite
 */
void Label::set_parent(View* parent) throw(Error) {
	if ( parent != m_parent ) {
		View::set_parent(parent);
		mark(M_TEXT_FONT);
	}
}

XX_END
