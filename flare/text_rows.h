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

#ifndef __flare__text_rows__
#define __flare__text_rows__

#include "./layout/layout.h"

/**
 * @ns flare
 */

namespace flare {

	/**
	* @class TextRows
	*/
	class F_EXPORT TextRows {
	public:
		struct F_EXPORT Row {
			Vec2 offset_start;
			Vec2 offset_end;
			float baseline;
			float ascender;
			float descender;
			uint32_t  row_num;
		};

		TextRows();
		
		inline Row* last() { return _last; }
		inline uint32_t count() const { return (uint32_t)_values.size(); }
		inline uint32_t last_num() const { return _last_num; }
		inline bool clip() const { return _is_clip; }
		inline void mark_clip() { _is_clip = true; }
		inline float max_width() const { return _max_width; }
		inline float max_height() const { return _last->offset_end.y(); }
		inline const Array<Row>& rows() const { return _values; }
		inline Row& operator[](uint32_t index) { return _values[index]; }
		
		void push_row(float ascender, float descender);
		void update_row(float ascender, float descender);
		void reset();
		void set_width(float value);

	private:
		Array<Row>  _values;
		Row*        _last;       // 最后行
		uint32_t        _last_num;   // 最后行号
		float       _max_width;  // 最大宽度
		bool        _is_clip;    // 修剪结束
	};

}
#endif
