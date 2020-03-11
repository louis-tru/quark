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

#include "text-rows.h"

NX_NS(ngui)

TextRows::TextRows() {
	reset();
}

void TextRows::push_row(float ascender, float descender) {
	
	float line_height = ascender + descender;
	
	if (m_last->offset_start.y() == m_last->offset_end.y()) { // 只有第一行才会这样
		m_last->offset_end.y(m_last->offset_start.y() + line_height);
		m_last->baseline = ascender;
		m_last->ascender = ascender;
		m_last->descender = descender;
	}
	
	set_width( m_last->offset_end.x() );
	
	m_last_num++;
	
	m_values.push({
		Vec2(0, m_last->offset_end.y()),
		Vec2(0, m_last->offset_end.y() + line_height),
		m_last->offset_end.y() + ascender,
		ascender,
		descender,
		m_last_num
	});
	
	m_last = &m_values[m_last_num];
}

void TextRows::update_row(float asc, float desc) {
	
	bool change = false;
	
	if (asc > m_last->ascender) {
		m_last->ascender = asc;
		change = true;
	}
	
	if (desc > m_last->descender) {
		m_last->descender = desc;
		change = true;
	}
	
	if ( change ) {
		m_last->baseline = m_last->offset_start.y() + m_last->ascender;
		m_last->offset_end.y(m_last->baseline + m_last->descender);
	}
}

void TextRows::reset() {
	m_values.clear();
	m_values.push({ Vec2(), Vec2(), 0, 0, 0, 0 });
	m_last_num = 0;
	m_last = &m_values[0];
	m_max_width = 0;
	m_is_clip = false;
}

void TextRows::set_width(float value) {
	if ( value > m_max_width ) {
		m_max_width = value;
	}
}

NX_END
