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

namespace ftr {

FX_DEFINE_INLINE_MEMBERS(Text, Inl) {
 public:
	
	template<TextAlign T>
	void set_text_align_offset(float text_margin) {
		
		float final_width = _final_width;
		
		for ( auto& i : _data.cells ) {
			Cell& cell = i.value();
			
			TextRows::Row& row = _rows[cell.line_num];
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
	FX_ERR("%s", "Error: Text can not have a child view");
}

void Text::append(View* child) throw(Error) {
	FX_ERR("%s", "Error: Text can not have a child view");
}

View* Text::append_text(cUcs2String& str) throw(Error) {
	_data.string.push(str);
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
	return nullptr;
}

void Text::set_value(cUcs2String& str) {
	_data.string = str;
	mark_pre( M_CONTENT_OFFSET ); // 标记内容变化
}

void Text::accept_text(Ucs2StringBuilder& output) const {
	output.push(_data.string);
}

void Text::set_text_align_offset(float text_margin) {
	switch ( _text_align ) {
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
	if ( _visible ) {
		
		if ( mark_value ) {
		
			if ( mark_value & (M_CONTENT_OFFSET | M_LAYOUT_THREE_TIMES) ) { // text layout
				set_text_align_offset();
			}
			solve();
			
			if ( mark_value & (M_TRANSFORM | M_TEXT_SIZE) ) {
				set_glyph_texture_level(_data);
			}
		}
		
		draw->draw(this);
		
		mark_value = M_NONE;
	}
}

void Text::set_layout_content_offset() {
	
	if ( _final_visible ) {
		
		_rows.reset();
		_data.cells.clear(); // 清空旧布局
		_data.cell_draw_begin = _data.cell_draw_end = 0;
		
		if ( !_data.string.is_empty() ) {
			
			mark( M_SHAPE );
			
			// set layout ...
			set_text_layout_offset(&_rows, _limit, _data);
		}
		
		set_layout_content_offset_after();
	}
}

void Text::set_draw_visible() {
	
	compute_box_vertex(_final_vertex);
	
	_draw_visible =
	
	compute_text_visible_draw(_final_vertex, _data, 0, _final_width, 0);
}

}
