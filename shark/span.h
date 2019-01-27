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

#ifndef __shark__span__
#define __shark__span__

#include "layout.h"
#include "text-font.h"
#include "font.h"

/**
 * @ns shark
 */

XX_NS(shark)

/**
 * @class Span
 */
class XX_EXPORT Span: public Layout, public TextLayout {
 public:
	XX_DEFINE_GUI_VIEW(SPAN, Span, span);
	
	typedef ReferenceTraits Traits;
	
	Span();

	/**
	 * @overwrite
	 */
	virtual TextFont* as_text_font() { return this; }
	virtual TextLayout* as_text_layout() { return this; }
	virtual View* view() { return this; }
	virtual void set_visible(bool value);
	virtual View* append_text(cUcs2String& str) throw(Error);
	virtual Vec2 layout_offset();
	
 protected:
	
	/**
	 * @overwrite
	 */
	virtual void set_layout_explicit_size();
	virtual void set_layout_content_offset();
	virtual void set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid);
	virtual void set_layout_three_times(bool horizontal, bool hybrid);
	
};

XX_END
#endif
