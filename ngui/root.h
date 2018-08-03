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

#ifndef __ngui__root__
#define __ngui__root__

#include "panel.h"

/**
 * @ns ngui
 */

XX_NS(ngui)

/**
 * @class Root 这个类型的视图尺寸为显示端口的尺寸
 */
class XX_EXPORT Root: public Panel {
 public:
	XX_DEFINE_GUI_VIEW(ROOT, Root, root);
	
	inline Root() {}
	
	/**
	 * @destructor
	 */
	virtual ~Root();
	
	/**
	 * @func initialize()
	 */
	void initialize() throw(Error);
	
	/**
	 * @overwrite
	 */
	virtual void prepend(View* child) throw(Error);
	virtual void append(View* child) throw(Error);
	virtual View* append_text(cUcs2String& str) throw(Error);
	virtual Vec2 layout_offset();
	virtual void draw(Draw* draw);
	virtual bool can_become_focus();
	
 protected:
	/**
	 * @overwrite
	 */
	virtual void set_parent(View* parent) throw(Error);
	virtual void set_layout_explicit_size();
	virtual void set_layout_content_offset();
};

XX_END
#endif
