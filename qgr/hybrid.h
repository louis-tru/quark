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

#ifndef __qgr__hybrid__
#define __qgr__hybrid__

#include "qgr/box.h"
#include "qgr/text-font.h"
#include "qgr/text-rows.h"
#include "qgr/span.h"

/**
 * @ns qgr
 */

XX_NS(qgr)

/**
 * @class Hybrid
 */
class XX_EXPORT Hybrid: public Box, public TextLayout {
 public:
	XX_DEFINE_GUI_VIEW(HYBRID, Hybrid, hybrid);
	
	typedef ReferenceTraits Traits;
	
	Hybrid();
	
	/**
	 * @overwrite
	 */
	virtual TextFont* as_text_font() { return this; }
	virtual TextLayout* as_text_layout() { return this; }
	virtual View* view() { return this; }
	virtual View* append_text(cUcs2String& str) throw(Error);
	virtual void set_visible(bool value);
	virtual Object* to_object() { return this; }
	
	/**
	 * @func text_align
	 */
	inline TextAlign text_align() const { return m_text_align; }
	
	/**
	 * @func set_text_align
	 */
	void set_text_align(TextAlign value);
	
	/**
	 * @func rows
	 */
	inline TextRows& rows() { return m_rows; }
	
 protected:

	/**
	 * @overwrite
	 */
	virtual void set_layout_explicit_size();
	virtual void set_layout_content_offset();
	virtual void set_layout_three_times(bool horizontal, bool hybrid);
	
	/**
	 * @func set_layout_content_offset_after
	 */
	void set_layout_content_offset_after();
	
 protected:
	TextRows  m_rows;
	TextAlign m_text_align;
	
	friend class Div;
	
};

XX_END
#endif
