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

#ifndef __ftr__box__
#define __ftr__box__

#include "ftr/layout.h"

FX_NS(ftr)

class BasicScroll;
class Background;

/**
 * 布局盒子视图尺寸的变化会影响到:
 *  1.子视图位置与宽度受宽度的影响
 *  2.后续兄弟视图的位置受影响
 *  3.父视图高度可能受到挤压膨胀
 * @class Box
 */
class FX_EXPORT Box: public Layout {
 public:
	FX_DEFINE_GUI_VIEW(BOX, Box, box);
	
	Box();
	
	/**
	 * @overwrite
	 */
	virtual ~Box();
	virtual void remove();
		
	/**
	 * @func width
	 */
	inline Value width() const { return m_width; }
	
	/**
	 * @func set_width
	 */
	void set_width(Value value);
	
	/**
	 * @func height
	 */
	inline Value height() const { return m_height; }
	
	/**
	 * @func set_height
	 */
	void set_height(Value value);
	
	/**
	 * @func set_margin 设置全部margin值
	 */
	void set_margin(Value value);
	
	/**
	 * @func margin_left
	 */
	inline Value margin_left() const { return m_margin_left; }
	
	/**
	 * @func set_margin_left
	 */
	void set_margin_left(Value value);
	
	/**
	 * @func margin_top
	 */
	inline Value margin_top() const { return m_margin_top; }
	
	/**
	 * @func set_margin_top
	 */
	void set_margin_top(Value value);
	
	/**
	 * @func margin_right
	 */
	inline Value margin_right() const { return m_margin_right; }
	
	/**
	 * @func set_margin_right
	 */
	void set_margin_right(Value value);
	
	/**
	 * @func margin_bottom
	 */
	inline Value margin_bottom() const { return m_margin_bottom; }
	
	/**
	 * @func set_margin_bottom
	 */
	void set_margin_bottom(Value value);
	
	/**
	 * @func set_border 设置全部边框值
	 */
	void set_border(Border value);
	
	/**
	 * @func border_left
	 */
	inline Border border_left() const {
		return Border(m_border_left_width, m_border_left_color);
	}
	
	/**
	 * @func set_border_left
	 */
	void set_border_left(Border value);
	
	/**
	 * @func border_top
	 */
	inline Border border_top() const {
		return Border(m_border_top_width, m_border_top_color);
	}
	
	/**
	 * @func set_border_top
	 */
	void set_border_top(Border value);
	
	/**
	 * @func border_right
	 */
	inline Border border_right() const {
		return Border(m_border_right_width, m_border_right_color);
	}
	
	/**
	 * @func set_border_right
	 */
	void set_border_right(Border value);
	
	/**
	 * @func border_bottom
	 */
	inline Border border_bottom() const {
		return Border(m_border_bottom_width, m_border_bottom_color);
	}
	
	/**
	 * @func set_border_bottom
	 */
	void set_border_bottom(Border value);
	
	/**
	 * @func set_border_width 设置全部边框宽度
	 */
	void set_border_width(float value);
	
	/**
	 * @func set_border_left_width
	 */
	inline float border_left_width() const { return m_border_left_width; }
	
	/**
	 * @func set_border_left_width
	 */
	void set_border_left_width(float value);
	
	/**
	 * @func border_top_width
	 */
	inline float border_top_width() const { return m_border_top_width; }
	
	/**
	 * @func set_border_top_width
	 */
	void set_border_top_width(float value);
	
	/**
	 * @func border_right_width
	 */
	inline float border_right_width() const { return m_border_right_width; }
	
	/**
	 * @func set_border_right_width
	 */
	void set_border_right_width(float value);
	
	/**
	 * @func border_bottom_width
	 */
	inline float border_bottom_width() const { return m_border_bottom_width; }
	
	/**
	 * @func set_border_bottom_width
	 */
	void set_border_bottom_width(float value);
	
	/**
	 * @func set_border_color 设置全部边框颜色
	 */
	void set_border_color(Color value);
	
	/**
	 * @func border_left_color
	 */
	inline Color border_left_color() const { return m_border_left_color; }
	
	/**
	 * @func set_border_left_color
	 */
	void set_border_left_color(Color value);
	
	/**
	 * @func border_top_color
	 */
	inline Color border_top_color() const { return m_border_top_color; }
	
	/**
	 * @func set_border_top_color
	 */
	void set_border_top_color(Color value);
	
	/**
	 * @func border_right_color
	 */
	inline Color border_right_color() const { return m_border_right_color; }
	
	/**
	 * @func set_border_right_color
	 */
	void set_border_right_color(Color value);
	
	/**
	 * @func border_bottom_color
	 */
	inline Color border_bottom_color() const { return m_border_bottom_color; }
	
	/**
	 * @func set_border_bottom_color
	 */
	void set_border_bottom_color(Color value);
	
	/**
	 * @func set_border_radius 设置全部4个圆角值
	 */
	void set_border_radius(float value);
	
	/**
	 * @func border_radius_left_top
	 */
	inline float border_radius_left_top() const { return m_border_radius_left_top; }
	
	/**
	 * @func set_border_radius_left_top
	 */
	void set_border_radius_left_top(float value);
	
	/**
	 * @func border_radius_right_top
	 */
	
	inline float border_radius_right_top() const { return m_border_radius_right_top; }
	
	/**
	 * @func set_border_radius_right_top
	 */
	void set_border_radius_right_top(float value);
	
	/**
	 * @func border_radius_right_bottom
	 */
	inline float border_radius_right_bottom() const { return m_border_radius_right_bottom; }
	
	/**
	 * @func set_border_radius_right_bottom
	 */
	void set_border_radius_right_bottom(float value);
	
	/**
	 * @func border_radius_left_bottom
	 */
	inline float border_radius_left_bottom() const { return m_border_radius_left_bottom; }
	
	/**
	 * @func set_border_radius_left_bottom
	 */
	void set_border_radius_left_bottom(float value);
	
	/**
	 * @func background_color
	 */
	inline Color background_color() const { return m_background_color; }
	
	/**
	 * @func set_background_color
	 */
	void set_background_color(Color value);
	
	/**
	 * @get newline 值为true布局会重启一行或一列的开始
	 */
	inline bool newline() const { return m_newline; }
	
	/**
	 * @set set_newline
	 */
	void set_newline(bool value);
	
	/**
	 * @func final_width 最终的宽度
	 */
	inline float final_width() const { return m_final_width; }
	
	/**
	 * @func final_height 最终的高度
	 */
	inline float final_height() const { return m_final_height; }
	
	/**
	 * @func final_margin_left 最终的左边距
	 */
	inline float final_margin_left() const { return m_final_margin_left; }
	
	/**
	 * @func final_margin_top 最终的顶边距
	 */
	inline float final_margin_top() const { return m_final_margin_top; }
	
	/**
	 * @func final_margin_right 最终的右边距
	 */
	inline float final_margin_right() const { return m_final_margin_right; }
	
	/**
	 * @func final_margin_bottom 最终的底边距
	 */
	inline float final_margin_bottom() const { return m_final_margin_bottom; }
	
	/**
	 * @func clip()
	 */
	inline bool clip() const { return m_clip; }
	
	/**
	 * @func set_clip(bool)
	 */
	void set_clip(bool value);

	/**
	 * @func background()
	 */
	inline Background* background() { return m_background; }

	/**
	 * @func set_background(value)
	 */
	void set_background(Background* value);
	
	/**
	 * @overwrite
	 */
	virtual View* append_text(cUcs2String& str) throw(Error);
	virtual void set_visible(bool value);
	virtual bool overlap_test(Vec2 point);
	virtual Vec2 layout_offset();
	virtual CGRect screen_rect();
	
 protected:
	
	/**
	 * @overwrite
	 */
	virtual void draw(Draw* draw);
	virtual void set_parent(View* parent) throw(Error);
	virtual void set_layout_explicit_size();
	virtual void set_offset_in_hybrid(TextRows* rows, Vec2 limit, Hybrid* hybrid);
	
	/**
	 * @func set_offset_horizontal 设置水平布局偏移值,并且返回一个可辅助下一个兄弟视图定位的兄弟视图
	 * @arg prev {Box*}  上面有效辅助定位的兄弟视图
	 * @arg squeeze {float&}  返回能够最大限度挤压父视图尺寸的值
	 * @arg limit {float} 限制布局的水平宽度
	 * @arg div {Div*} 父布局视图
	 */
	virtual Box* set_offset_horizontal(Box* prev, Vec2& squeeze, float limit, Div* div);
	
	/**
	 * @func set_offset_vertical 设置垂直布局偏移值,并且返回一个可辅助下一个兄弟视图定位的兄弟视图
	 * @arg prev {Box*}  上面有效辅助定位的兄弟视图
	 * @arg squeeze {float&}  返回能够最大限度挤压父视图尺寸的值
	 * @arg limit {float} 限制布局的水平高度
	 * @arg div {Div*} 父布局视图
	 */
	virtual Box* set_offset_vertical(Box* prev, Vec2& squeeze, float limit, Div* div);
	
	/**
	 * @func set_horizontal_active_mark
	 */
	virtual void set_horizontal_active_mark();
	
	/**
	 * @func set_vertical_active_mark
	 */
	virtual void set_vertical_active_mark();
	
	/**
	 * @func solve_explicit_size_after()
	 */
	void solve_explicit_size_after(bool change_horizontal, bool change_vertical, uint child_mark);
	
	/**
	 * @func set_default_offset_value
	 */
	void set_default_offset_value();
	
	/**
	 * @func compute_box_vertex
	 */
	void compute_box_vertex(Vec2 vertex[4]);
	
	/**
	 * @func set_draw_visible
	 */
	virtual void set_draw_visible();
	
	/**
	 * @func get_screen_region
	 */
	Region get_screen_region();
	
	/**
	 * @func solve() solve draw param
	 */
	void solve();
	
 private:
	
	Value     m_width;  // width 宽度
	Value     m_height; // height 高度
	Value     m_margin_top;   // 顶边缘距离
	Value     m_margin_right; // 右边缘距离
	Value     m_margin_bottom; // 底边缘距离
	Value     m_margin_left;  // 左边缘距离
	Color     m_border_top_color;
	Color     m_border_right_color;
	Color     m_border_bottom_color;
	Color     m_border_left_color;
	float     m_border_top_width;
	float     m_border_right_width;
	float     m_border_bottom_width;
	float     m_border_left_width;
	float     m_border_radius_left_top; // 左上圆角
	float     m_border_radius_right_top; // 右上圆角
	float     m_border_radius_right_bottom; // 右下圆角
	float     m_border_radius_left_bottom; // 左下圆角
	Color     m_background_color;     // 背景颜色
	Background* m_background;         // 盒子背景
 protected:
	float     m_final_width; // 最终的宽度
	float     m_final_height; // 最终的高度
	float     m_final_margin_top; // 最终的顶边距
	float     m_final_margin_right; // 最终的右边距
	float     m_final_margin_bottom; // 最终的底边距
	float     m_final_margin_left; // 最终的左边距
	float     m_final_border_radius_left_top; // 最终的左上圆角
	float     m_final_border_radius_right_top; // 最终的右上圆角
	float     m_final_border_radius_right_bottom; // 最终的右下圆角
	float     m_final_border_radius_left_bottom; // 最终的左下圆角
	float     m_raw_client_width;    // 原客户端宽度,视图所占用的所有水平尺寸,三次布局以前的宽度
	float     m_raw_client_height;   // 原客户端高度,视图所占用的所有垂直尺寸,三次布局以前的高度
	Vec2      m_limit;            // 限制内部偏移排版的尺寸,有明确尺寸时与final_width或final_height相等
	Vec2      m_final_vertex[4];  // 最终在屏幕上显示的真实顶点位置，左上/右上/右下/左下
	uint      horizontal_active_mark_value; // 父视图尺寸改变时,这个值会被加入到当前主标记中
	uint      vertical_active_mark_value;   //
	int       m_linenum;          /* 盒子在Hybrid视图布局中的行索引,-1表式没有行无需显示 */
	bool      m_newline;          // 新行或新列
	bool      m_clip;             // 是否溢出修剪
	bool      m_explicit_width;   // 是否拥有明确宽度,明确宽度不会受到子布局视图的挤压影响
	bool      m_explicit_height;  // 是否拥有明确高度,明确高度不会受到子布局视图的挤压影响
	bool      m_is_draw_border;   // 是否需要绘制边框
	bool      m_is_draw_border_radius;  // 是否要绘制圆角
	
	FX_DEFINE_INLINE_CLASS(Inl);
	
	friend class Div;
	friend class Hybrid;
	friend class Limit;
	friend class LimitIndep;
	friend class Image;
	friend class Indep;
	friend class Root;
	friend class BackgroundImage;
};

FX_END

#endif
