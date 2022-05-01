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

#ifndef __flare__layout__label__
#define __flare__layout__label__

#include "./view.h"

namespace flare {

	class F_EXPORT TextBasic {
	public:
		TextBasic();
		F_DEFINE_PROP(TextColor, text_background_color);
		F_DEFINE_PROP(TextColor, text_color);
		F_DEFINE_PROP(TextSize, text_size);
		F_DEFINE_PROP(TextWeight, text_weight);
		F_DEFINE_PROP(TextStyle, text_style);
		F_DEFINE_PROP(TextFamily, text_family);
		F_DEFINE_PROP(TextShadow, text_shadow);
		F_DEFINE_PROP(TextLineHeight, text_line_height);
		F_DEFINE_PROP(TextDecoration, text_decoration);
		F_DEFINE_PROP(TextOverflow, text_overflow);
		F_DEFINE_PROP(TextWhiteSpace, text_white_space);
	protected:
		virtual void onTextChange(uint32_t mark);
	};

	class F_EXPORT Label: public View, public TextBasic {
		F_Define_View(Label);
	public:
		F_DEFINE_PROP(String, text_value);
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextRows *rows) override;
		virtual void set_layout_offset_lazy(Vec2 origin, Vec2 size) override;
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) override;
	protected:
		virtual void onTextChange(uint32_t mark) override;
	};

}
#endif
