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

#include "./text_rows.h"

namespace flare {

	TextRows::TextRows() {
		clear();
	}

	void TextRows::clear() {
		_rows.clear();
		_rows.push({ Vec2(), Vec2(), 0, 0, 0, 0 });
		_last = &_rows[0];
		_max_width = 0;
		_is_clip = false;
	}

	void TextRows::push(float ascender, float descender) {

		float line_height = ascender + descender;
		
		if (_last->offset_start.y() == _last->offset_end.y()) { // 只有第一行才会这样
			_last->offset_end.set_y(_last->offset_start.y() + line_height);
			_last->baseline = ascender;
			_last->ascender = ascender;
			_last->descender = descender;
		}

		set_max_width( _last->offset_end.x() );

		auto row_num = _rows.length();

		_rows.push({
			Vec2(0, _last->offset_end.y()),
			Vec2(0, _last->offset_end.y() + line_height),
			_last->offset_end.y() + ascender,
			ascender,
			descender,
			row_num,
		});

		_last = &_rows[row_num];
	}

	void TextRows::update(float asc, float desc) {
		bool change = false;
		if (asc > _last->ascender) {
			_last->ascender = asc;
			change = true;
		}
		if (desc > _last->descender) {
			_last->descender = desc;
			change = true;
		}
		if ( change ) {
			_last->baseline = _last->offset_start.y() + _last->ascender;
			_last->offset_end.set_y(_last->baseline + _last->descender);
		}
	}

	void TextRows::set_max_width(float value) {
		if ( value > _max_width ) {
			_max_width = value;
		}
	}

	void TextRows::set_is_clip(bool value) {
		_is_clip = value;
	}

}
