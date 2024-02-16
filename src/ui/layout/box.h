/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark__layout__box__
#define __quark__layout__box__

#include "./layout.h"
#include "../filter.h"

namespace qk {

	/**
		* @class BoxLayout
		*/
	class Qk_EXPORT BoxLayout: public Layout {
	public:
		// define props
		Qk_DEFINE_PROP_GET(bool,       layout_wrap_x, Const); // Returns the x-axis is wrap content
		Qk_DEFINE_PROP_GET(bool,       layout_wrap_y, Const); // Returns the y-axis is wrap content
		Qk_DEFINE_PROP    (bool,       clip, Const); // is clip box display range
		Qk_DEFINE_PROP    (BoxSize,    width, Const); // size width
		Qk_DEFINE_PROP    (BoxSize,    height, Const); // size height
		Qk_DEFINE_PROP    (BoxSize,    width_limit, Const); // limit max size
		Qk_DEFINE_PROP    (BoxSize,    height_limit, Const);
		Qk_DEFINE_PROP    (float,      margin_top, Const); // margin
		Qk_DEFINE_PROP    (float,      margin_right, Const);
		Qk_DEFINE_PROP    (float,      margin_bottom, Const);
		Qk_DEFINE_PROP    (float,      margin_left, Const);
		Qk_DEFINE_PROP    (float,      padding_top, Const); // padding
		Qk_DEFINE_PROP    (float,      padding_right, Const);
		Qk_DEFINE_PROP    (float,      padding_bottom, Const);
		Qk_DEFINE_PROP    (float,      padding_left, Const);
		Qk_DEFINE_PROP    (float,      border_radius_left_top, Const); // border_radius
		Qk_DEFINE_PROP    (float,      border_radius_right_top, Const);
		Qk_DEFINE_PROP    (float,      border_radius_right_bottom, Const);
		Qk_DEFINE_PROP    (float,      border_radius_left_bottom, Const);
		Qk_DEFINE_PROP_ACC(Color,      border_color_top, Const); // border_color
		Qk_DEFINE_PROP_ACC(Color,      border_color_right, Const);
		Qk_DEFINE_PROP_ACC(Color,      border_color_bottom, Const);
		Qk_DEFINE_PROP_ACC(Color,      border_color_left, Const);
		Qk_DEFINE_PROP_ACC(float,      border_width_top, Const); // border_width
		Qk_DEFINE_PROP_ACC(float,      border_width_right, Const);
		Qk_DEFINE_PROP_ACC(float,      border_width_bottom, Const);
		Qk_DEFINE_PROP_ACC(float,      border_width_left, Const);
		Qk_DEFINE_PROP    (Color,      background_color, Const); // fill background color
		Qk_DEFINE_PROP    (BoxFill*,   background); // fill background, image|gradient
		Qk_DEFINE_PROP    (BoxShadow*, box_shadow); // box shadow, shadow
		Qk_DEFINE_PROP    (float,      weight, Const); // layout weight
		Qk_DEFINE_PROP_ACC(Align,      align, Const); // layout align
		Qk_DEFINE_PROP_GET(Vec2,       content_size, Const); // width,height, no include padding
		Qk_DEFINE_PROP_GET(Vec2,       client_size, Const); // border + padding + content

		/**
		 * @constructor
		*/
		BoxLayout(Window *win);

		/**
		 * @destructor
		*/
		virtual ~BoxLayout();

		// --------------- o v e r w r i t e ---------------
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextLines *lines, TextConfig *cfg) override;
		virtual Vec2 layout_offset() override;
		virtual Size layout_size() override; // context size + padding + border + margin
		virtual Size layout_raw_size(Size parent_content_size) override;
		/*
		* 这里定义项目的放大与缩小比例，默认为0，即如果存在剩余空间，不放大也不缩小 
		* 在flex中：size = size_raw + overflow * weight / weight_total * min(weight_total, 1)
		*/
		virtual float layout_weight() override;
		virtual Align layout_align() override;
		virtual Vec2 layout_offset_inside() override;
		virtual Vec2 layout_lock(Vec2 layout_size) override;
		virtual void set_layout_offset(Vec2 val) override;
		virtual void set_layout_offset_lazy(Vec2 size) override;
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) override;
		virtual void solve_marks(const Mat &mat, uint32_t mark) override;
		virtual bool solve_visible_region(const Mat &mat) override; // compute visible region
		virtual bool overlap_test(Vec2 point) override;
		virtual Vec2 center() override;
		virtual bool is_clip() override;
		virtual void draw(UIRender *render) override;
		virtual ViewType viewType() const override;

		/**
			* client rect = border + padding + content
			* @func solve_rect_vertex(mat, vertex)
			*/
		virtual void solve_rect_vertex(const Mat &mat, Vec2 vertexOut[4]); // compute rect vertex

	protected:
		/**
			* 
			* is ready layout layout typesetting in the `layout_reverse() or layout_forward()` func
			*
			* @func is_ready_layout_typesetting()
			*/
		bool is_ready_layout_typesetting();

		/**
			* @func solve_layout_size_forward(mark)
			*/
		uint32_t solve_layout_size_forward(uint32_t mark);

		/**
			* @func set_layout_size(layout_size, is_wrap, is_lock_child)
			*/
		void set_layout_size(Vec2 layout_size, bool is_wrap[2], bool is_lock_child = false);

		/**
			* @func set_content_size(content_size)
			*/
		void set_content_size(Vec2 content_size);

		// @func solve_layout_content_width
		virtual float solve_layout_content_width(Size &parent_layout_size);
		virtual float solve_layout_content_height(Size &parent_layout_size);

		void mark_size(uint32_t mark);

	protected:
		// define private props
		BoxBorder *_border; // BoxBorder, top/right/bottom/left
		// box layout attrs
		Vec2  _layout_offset; // 相对父视图的开始偏移位置（box包含margin值）
		Vec2  _layout_size; // 在布局中所占用的尺寸（margin+border+padding+content）
		Vec2  _vertex[4]; // box vertex
		Align _align;  // layout align

		friend class UIRender;
	};

	class Qk_EXPORT Box: public View {
	public:
		Qk_Define_View(Box, View);
		Qk_DEFINE_PROP_ACC(bool,       clip, Const); // is clip box display range
		Qk_DEFINE_PROP_ACC(BoxSize,    width, Const); // size width
		Qk_DEFINE_PROP_ACC(BoxSize,    height, Const); // size height
		Qk_DEFINE_PROP_ACC(BoxSize,    width_limit, Const); // limit max size
		Qk_DEFINE_PROP_ACC(BoxSize,    height_limit, Const);
		Qk_DEFINE_PROP_ACC(float,      margin_top, Const); // margin
		Qk_DEFINE_PROP_ACC(float,      margin_right, Const);
		Qk_DEFINE_PROP_ACC(float,      margin_bottom, Const);
		Qk_DEFINE_PROP_ACC(float,      margin_left, Const);
		Qk_DEFINE_PROP_ACC(float,      padding_top, Const); // padding
		Qk_DEFINE_PROP_ACC(float,      padding_right, Const);
		Qk_DEFINE_PROP_ACC(float,      padding_bottom, Const);
		Qk_DEFINE_PROP_ACC(float,      padding_left, Const);
		Qk_DEFINE_PROP_ACC(float,      border_radius_left_top, Const); // border_radius
		Qk_DEFINE_PROP_ACC(float,      border_radius_right_top, Const);
		Qk_DEFINE_PROP_ACC(float,      border_radius_right_bottom, Const);
		Qk_DEFINE_PROP_ACC(float,      border_radius_left_bottom, Const);
		Qk_DEFINE_PROP_ACC(Color,      border_color_top, Const); // border_color
		Qk_DEFINE_PROP_ACC(Color,      border_color_right, Const);
		Qk_DEFINE_PROP_ACC(Color,      border_color_bottom, Const);
		Qk_DEFINE_PROP_ACC(Color,      border_color_left, Const);
		Qk_DEFINE_PROP_ACC(float,      border_width_top, Const); // border_width
		Qk_DEFINE_PROP_ACC(float,      border_width_right, Const);
		Qk_DEFINE_PROP_ACC(float,      border_width_bottom, Const);
		Qk_DEFINE_PROP_ACC(float,      border_width_left, Const);
		Qk_DEFINE_PROP_ACC(Color,      background_color, Const); // fill background color
		Qk_DEFINE_PROP_ACC(BoxFill*,   background); // fill background, image|gradient
		Qk_DEFINE_PROP_ACC(BoxShadow*, box_shadow); // box shadow, shadow
		Qk_DEFINE_PROP_ACC(float,      weight, Const);
		Qk_DEFINE_PROP_ACC(Align,      align, Const);
	};

	/**
	* @method overlap_test_from_convex_quadrilateral
	*/
	Qk_EXPORT bool overlap_test_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4], Vec2 point);
	
	/**
	 * @method screen_region_from_convex_quadrilateral
	*/
	Qk_EXPORT Region screen_region_from_convex_quadrilateral(Vec2 quadrilateral_vertex[4]);

}
#endif
