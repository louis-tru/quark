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

#include "text.h"
#include "display-port.h"
#include "app.h"

XX_NS(qgr)

XX_DEFINE_INLINE_MEMBERS(Text, Inl) {
public:
	
	template<TextAlign T>
	void set_text_align_offset(float text_margin) {
		
		float final_width = m_final_width;
		
		for ( auto& i : m_data.cells ) {
			Cell& cell = i.value();
			
			TextRows::Row& row = m_rows[cell.line_num];
			cell.baseline = row.baseline;
			
			switch (T) {
				default: cell.offset_start = text_margin; break;
				case TextAlign::CENTER:
					cell.offset_start = (final_width - row.offset_end.x()) / 2.0;
					break;
				case TextAlign::RIGHT:
					cell.offset_start = final_width - row.offset_end.x() - text_margin;
					break;
				case TextAlign::LEFT_REVERSE:
					cell.reverse = 1;
					cell.offset_start = row.offset_end.x() + text_margin;
					break;
				case TextAlign::CENTER_REVERSE:
					cell.reverse = 1;
					cell.offset_start = (final_width - row.offset_end.x()) / 2.0 + row.offset_end.x();
					break;
				case TextAlign::RIGHT_REVERSE:
					cell.reverse = 1;
					cell.offset_start = final_width - text_margin;
					break;
			}
		}
	}

};

void Text::prepend(View* child) throw(Error) {
	XX_ERR("%s", "TextNode can not have a child view");
}

void Text::append(View* child) throw(Error) {
	XX_ERR("%s", "TextNode can not have a child view");
}

View* Text::append_text(cUcs2String& str) throw(Error) {
	m_data.string.push(str);
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
	return nullptr;
}

void Text::remove_all_child() {
	m_data.cells.clear(); // 清除布局
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
}

void Text::set_value(cUcs2String& str) {
	m_data.string = str;
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
}

void Text::accept_text(Ucs2StringBuilder& output) const {
	output.push(m_data.string);
}

/**
 * @func set_text_align_offset
 */
void Text::set_text_align_offset(float text_margin) {
	switch ( m_text_align ) {
		default:
			Inl_Text(this)->set_text_align_offset<TextAlign::LEFT>(text_margin); break;
		case TextAlign::CENTER:
			Inl_Text(this)->set_text_align_offset<TextAlign::CENTER>(text_margin); break;
		case TextAlign::RIGHT:
			Inl_Text(this)->set_text_align_offset<TextAlign::RIGHT>(text_margin); break;
		case TextAlign::LEFT_REVERSE:
			Inl_Text(this)->set_text_align_offset<TextAlign::LEFT_REVERSE>(text_margin); break;
		case TextAlign::CENTER_REVERSE:
			Inl_Text(this)->set_text_align_offset<TextAlign::CENTER_REVERSE>(text_margin); break;
		case TextAlign::RIGHT_REVERSE:
			Inl_Text(this)->set_text_align_offset<TextAlign::RIGHT_REVERSE>(text_margin); break;
	}
}

void Text::draw(Draw* draw) {
	if ( m_visible ) {
		
		if ( mark_value ) {
		
			if ( mark_value & (M_CONTENT_OFFSET | M_LAYOUT_THREE_TIMES) ) { // text layout
				set_text_align_offset();
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
 * @func set_layout_content_offset
 */
void Text::set_layout_content_offset() {
	
	if ( m_final_visible ) {
		
		m_rows.reset();
		m_data.cells.clear(); // 清空旧布局
		m_data.cell_draw_begin = m_data.cell_draw_end = 0;
		
		if ( !m_data.string.is_empty() ) {
			
			mark( M_SHAPE );
			
			// set layout ...
			set_text_layout_offset(&m_rows, m_limit, m_data);
		}
		
		set_layout_content_offset_after();
	}
}

/**
 * @func set_draw_visible
 */
void Text::set_draw_visible() {
	
	compute_box_vertex(m_final_vertex);
	
	m_draw_visible =
	
	compute_text_visible_draw(m_final_vertex, m_data, 0, m_final_width, 0);
}

XX_END
