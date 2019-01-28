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

#include "span.h"
#include "text-node.h"
#include "text-rows.h"

XX_NS(qgr)

extern void _view_inl__safe_delete_mark(View* view);

Span::Span() {
	m_need_draw = true;
}

/**
 * @overwrie
 */
void Span::set_visible(bool value) {
	if (m_visible != value) { 
		View::set_visible(value);
		// 这会影响其它兄弟视图的位置
		mark_pre(M_LAYOUT | M_SIZE_HORIZONTAL | M_SIZE_VERTICAL/* | M_TEXT_FONT*/);
	}
}

/**
 * @overwrite
 */
View* Span::append_text(cUcs2String& str) throw(Error) {
	TextNode* text = new TextNode();
	text->set_value( str );
	append(text);
	return text;
}

/**
 * @overwrite
 */
Vec2 Span::layout_offset() {
	return m_offset_start;
}

/**
 * @overwrite
 */
void Span::set_layout_explicit_size() {
	
	if ( m_final_visible ) {
		if ( mark_value & M_TEXT_FONT ) { // 文本属性的变化会影响后代文本视图属性
			solve_text_layout_mark();
		}
	}
	if ( mark_value & (M_SIZE_HORIZONTAL | M_SIZE_VERTICAL) ) {
		TextLayout* text = parent()->as_text_layout();
		if ( text ) {
			text->view()->mark_pre( M_CONTENT_OFFSET ); // 布局尺寸改变可能会影响到内容的偏移值
		} else {
			mark_pre( M_CONTENT_OFFSET );
		}
	}
}

/**
 * @overwrite
 */
void Span::set_layout_content_offset() {
	
	if (m_final_visible) {
		
		TextLayout* text = parent()->as_text_layout();
		if ( text ) { // 父视图必需是TextLayout
			text->view()->mark_pre( M_CONTENT_OFFSET ); // 内容偏移不能直接设置,传递给父视图
		} else { // 否则做简单处理
			TextRows rows;
			set_offset_in_hybrid(&rows, Vec2(Float::max, Float::max), nullptr);
		}
	}
}

/**
 * @overwrite
 */
void Span::set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid) {
	if ( m_visible ) {
		
		View* view = first();
		
		while ( view ) {
			Layout* layout = view->as_layout();
			if ( layout ) {
				layout->set_offset_in_hybrid(rows, limit, hybrid);
			}
			view = view->next();
		}
	}
}

/**
 * @overwrite
 */
void Span::set_layout_three_times(bool horizontal, bool hybrid) {
	if ( m_visible ) {
		View* view = first();
		while (view) {
			Layout* layout = view->as_layout();
			if ( layout ) {
				layout->set_layout_three_times(horizontal, hybrid);
				_view_inl__safe_delete_mark(layout);
			}
			view = view->next();
		}
	}
}

XX_END
