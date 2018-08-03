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

#include "textarea.h"
#include "display-port.h"
#include "app.h"

XX_NS(ngui)

Textarea::Textarea(): Input(), BasicScroll(this) {
	
}

void Textarea::draw(Draw* draw) {
	if ( m_visible ) {
		
		if ( mark_value ) {
			
			BasicScroll::solve();
			
			if (m_rows.max_width() <= final_width()) {
				input_text_offset_x_ = 0;
			} else {
				switch ( m_text_align ) {
					default:
					case TextAlign::LEFT_REVERSE:
						input_text_offset_x_ = 0; break;
					case TextAlign::CENTER:
					case TextAlign::CENTER_REVERSE:
						input_text_offset_x_ = (final_width() - m_rows.max_width()) / 2.0;
						break;
					case TextAlign::RIGHT:
					case TextAlign::RIGHT_REVERSE:
						input_text_offset_x_ = final_width() - m_rows.max_width();
						break;
				}
			}
			
			if ( mark_value & (M_CONTENT_OFFSET | M_LAYOUT_THREE_TIMES) ) {
				set_text_align_offset(text_margin_);
			}
			
			if ( mark_value & (M_CONTENT_OFFSET | M_INPUT_STATUS) ) {
				refresh_cursor_screen_position(); // text layout
			}
			
			if ( mark_value & M_SCROLL ) {
				mark_value |= M_SHAPE; // 设置这个标记只为了重新调用 set_screen_visible()
			}
			
			Input::solve();
			
			if ( mark_value & (M_TRANSFORM | M_TEXT_SIZE) ) {
				set_glyph_texture_level(m_data);
			}
		}
		
		draw->draw(this);
		
		mark_value = M_NONE;
	}
}

void Textarea::set_layout_content_offset() {
	if ( m_final_visible ) {
		Input::set_layout_content_offset();
		BasicScroll::set_scroll_size(Vec2(m_rows.max_width(), m_rows.max_height()));
	}
}

bool Textarea::is_multi_line_input() {
	return true;
}

Vec2 Textarea::input_text_offset() {
	return Vec2( -scroll_x() - input_text_offset_x_, -scroll_y() );
}

void Textarea::set_input_text_offset(Vec2 value) {
	set_scroll( Vec2(-value.x() - input_text_offset_x_, -value.y()) );
}

void Textarea::set_screen_visible() {
	
	compute_box_vertex(m_final_vertex);
	
	m_screen_visible =
	
		compute_text_visible_draw(m_final_vertex, m_data, 0, m_final_width, scroll_y());
}

XX_END
