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

#ifndef __ngui__layout__
#define __ngui__layout__

#include "ngui/view.h"
#include "ngui/value.h"
#include "ngui/text-font.h"

XX_NS(ngui)

class TextRows;

/**
 * @class Layout 布局排版抽象类
 */
class XX_EXPORT Layout: public View {
 public:
	XX_DEFINE_GUI_VIEW(LAYOUT, Layout, layout);
	
	Layout();
	
	/**
	 * @overwrite
	 */
	virtual void remove();
	virtual void remove_all_child();
	
	/**
	 * @func client_width 客户端宽度,视图所占用的所有水平尺寸
	 */
	inline float client_width() const {
		return m_offset_end.x() - m_offset_start.x();
	}
	
	/**
	 * @func client_height 客户端高度,视图所占用的所有垂直尺寸
	 */
	inline float client_height() const {
		return m_offset_end.y() - m_offset_start.y();
	}
	
 protected:
	
	/**
	 * @overwrite
	 */
	virtual void set_parent(View* parent) throw(Error);
	
	/**
	 * 设置视图的尺寸
	 * @func set_layout_explicit_size Explicit dimensions from outside to inside
	 */
	virtual void set_layout_explicit_size() = 0;
	
	/**
	 * 设置内容视图的偏移量
	 * @func set_layout_content_offset Content migration from inside to outside
	 */
	virtual void set_layout_content_offset() = 0;
	
	/**
	 * @func set_layout_three_times 第三次布局
	 */
	virtual void set_layout_three_times(bool horizontal, bool hybrid) = 0;
	
	/**
	 * @func set_offset_in_hybrid 设置在混合视图中的偏移量
	 */
	virtual void set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) = 0;
	
 protected:
	Vec2    m_offset_start;   /* 相对父视图的开始偏移位置 */
	Vec2    m_offset_end;     /* 相对父视图的结束偏移位置 */
	Layout* m_parent_layout;  /* 父关联的布局视图，在一般为父布局视图，在Text布局中时为顶层Text视图
														 * 应该在做布局运算时被设置 
														 */
	friend class PreRender;
	friend class Box;
	friend class TextLayout;
	friend class Hybrid;
	friend class Text;
	friend class Div;
	friend class Limit;
	friend class Span;
	friend class Image;
	friend class Scroll;
};

XX_END

#endif
