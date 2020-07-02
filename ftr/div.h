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

#ifndef __ftr__div__
#define __ftr__div__

#include "ftr/box.h"

FX_NS(ftr)

/**
 * @class Div
 */
class FX_EXPORT Div: public Box {
 public:
	FX_DEFINE_GUI_VIEW(DIV, Div, div);
	
	Div();
	
	/**
	 * @get content_align
	 */
	inline ContentAlign content_align() const { return m_content_align; };
	
	/**
	 * @set set_content_align
	 */
	void set_content_align(ContentAlign value);
	
 protected:
	
	/**
	 * @overwrite
	 */
	virtual void set_layout_content_offset();
	virtual void set_layout_three_times(bool horizontal, bool hybrid);
	
	/**
	 * @func set_div_content_offset 设置div内容偏移,返回是否挤压父视图
	 */
	bool set_div_content_offset(Vec2& squeeze, Vec2 limit_min);
	
 protected:
	
	// 内容对齐方式默认为left（即默认为水平内容布局）
	// left|right, 水平布局,一行宽度不够向下挤压
	// top|bottom, 垂直布局,一列高度不够向右挤压
	ContentAlign m_content_align;

	FX_DEFINE_INLINE_CLASS(Inl);
	
};

FX_END

#endif
