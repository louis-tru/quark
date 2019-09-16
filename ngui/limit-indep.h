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

#ifndef __ngui__limit_indep__
#define __ngui__limit_indep__

#include "ngui/indep.h"
#include "ngui/limit.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

/**
 * @class LimitIndep
 */
class XX_EXPORT LimitIndep: public Indep {
 public:
	XX_DEFINE_GUI_VIEW(LIMIT_INDEP, LimitIndep, limit_indep);
	
	friend class Limit::Inl<LimitIndep>;
	
	LimitIndep();
	
	/**
	 * @func min_width
	 */
	inline Value min_width() const { return m_width; }
	
	/**
	 * @func min_width
	 */
	inline Value min_height() const { return m_height; }
	
	/**
	 * @func min_width
	 */
	inline Value max_width() const { return m_max_width; }
	
	/**
	 * @func min_width
	 */
	inline Value max_height() const { return m_max_height; }
	
	/**
	 * @func set_min_width
	 */
	inline void set_min_width(Value value) { set_width(value); }
	
	/**
	 * @func set_min_width
	 */
	inline void set_min_height(Value value) { set_height(value); }
	
	/**
	 * @func set_min_width
	 */
	void set_max_width(Value value);
	
	/**
	 * @func min_width
	 */
	void set_max_height(Value value);
	
 protected:
	
	/**
	 * @overwrite
	 */
	virtual void set_horizontal_active_mark();
	virtual void set_vertical_active_mark();
	virtual void set_layout_explicit_size();
	virtual void set_layout_content_offset();
	virtual Box* set_offset_horizontal(Box* prev, Vec2& squeeze, float limit, Div* div);
	virtual Box* set_offset_vertical(Box* prev, Vec2& squeeze, float limit, Div* div);
	virtual void set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid);
	virtual void set_layout_three_times(bool horizontal, bool hybrid);
	
 private:
	Value m_max_width;
	Value m_max_height;
	float m_limit_min_width;
	float m_limit_min_height;
	
};

XX_END
#endif
