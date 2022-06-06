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

#ifndef __noug__layout__box__
#define __noug__layout__box__

#include "./view.h"
#include "../effect.h"

namespace noug {

	/**
		* @class Box
		*/
	class N_EXPORT Box: public View {
		N_Define_View(Box);
	public:
		Box();
		virtual ~Box();
		
		/**
			*
			* 设置布局对齐方式
			*
			* @func set_layout_align(align)
			*/
		void set_layout_align(Align align);

		/**
			*
			* 设置布局权重
			*
			* @func set_layout_weight(val)
			*/
		void set_layout_weight(float weight);

		/**
			* @func solve_rect_vertex(vertex)
			*/
		void solve_rect_vertex(Vec2 vertex[4]);

		// --------------- o v e r w r i t e ---------------
		virtual bool layout_forward(uint32_t mark) override;
		virtual bool layout_reverse(uint32_t mark) override;
		virtual void layout_text(TextLines *lines, TextConfig* textSet) override;
		virtual Vec2 layout_offset() override;
		virtual Size layout_size() override;
		virtual Size layout_raw_size(Size parent_content_size) override;
		/*
		* 这里定义项目的放大与缩小比例，默认为0，即如果存在剩余空间，不放大也不缩小 
		* 在flex中：size = size_raw + overflow * weight / weight_total * min(weight_total, 1)
		*/
		virtual float layout_weight() override;
		virtual Align layout_align() override;
		virtual Mat  layout_matrix() override;
		virtual Vec2 layout_offset_inside() override;
		virtual Vec2 layout_lock(Vec2 layout_size) override;
		virtual void set_layout_offset(Vec2 val) override;
		virtual void set_layout_offset_lazy(Vec2 size) override;
		virtual void onParentLayoutContentSizeChange(Layout* parent, uint32_t mark) override;
		virtual bool solve_visible_region() override;

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

		void  mark_layout_size(uint32_t mark);

		// --------------- m e m b e r . f i e l d ---------------

		// define props
		N_DEFINE_PROP_READ(bool, layout_wrap_x); // Returns the is use border radius
	protected:
		N_DEFINE_PROP_READ(bool, layout_wrap_y); // Returns the is use border radius
	public:
		N_DEFINE_PROP(bool, is_clip); // is clip box display range
		N_DEFINE_PROP_READ(bool, is_radius); // Returns the is use border radius
		N_DEFINE_PROP(BoxSize, width); // size width
		N_DEFINE_PROP(BoxSize, height); // size height
		N_DEFINE_PROP(BoxSize, width_limit); // limit max size
		N_DEFINE_PROP(BoxSize, height_limit);
		N_DEFINE_PROP(float, margin_top); // margin
		N_DEFINE_PROP(float, margin_right);
		N_DEFINE_PROP(float, margin_bottom);
		N_DEFINE_PROP(float, margin_left);
		N_DEFINE_PROP(float, padding_top); // padding
		N_DEFINE_PROP(float, padding_right);
		N_DEFINE_PROP(float, padding_bottom);
		N_DEFINE_PROP(float, padding_left);
		N_DEFINE_PROP(float, radius_left_top); // border_radius
		N_DEFINE_PROP(float, radius_right_top);
		N_DEFINE_PROP(float, radius_right_bottom);
		N_DEFINE_PROP(float, radius_left_bottom);
		N_DEFINE_ACCESSOR(Color, border_color_top); // border_color
		N_DEFINE_ACCESSOR(Color, border_color_right);
		N_DEFINE_ACCESSOR(Color, border_color_bottom);
		N_DEFINE_ACCESSOR(Color, border_color_left);
		N_DEFINE_ACCESSOR(float, border_width_top); // border_width
		N_DEFINE_ACCESSOR(float, border_width_right);
		N_DEFINE_ACCESSOR(float, border_width_bottom);
		N_DEFINE_ACCESSOR(float, border_width_left);
		N_DEFINE_ACCESSOR(BorderStyle, border_style_top); // border_style
		N_DEFINE_ACCESSOR(BorderStyle, border_style_right);
		N_DEFINE_ACCESSOR(BorderStyle, border_style_bottom);
		N_DEFINE_ACCESSOR(BorderStyle, border_style_left);
		N_DEFINE_PROP(Color, fill_color); // fill color
		N_DEFINE_PROP(Fill*, fill); // fill, image|gradient
		N_DEFINE_PROP(Effect*, effect); // effect, shadow
		N_DEFINE_PROP_READ(Vec2, transform_origin); // Start the matrix transformation from this origin point
		N_DEFINE_PROP_READ(Vec2, content_size); // width,height / size
		N_DEFINE_PROP_READ(Vec2, client_size); // border + padding + content

	protected:
		void alloc_border();
		struct Border {
			Color color_top, color_right, color_bottom, color_left; // border_color
			float width_top, width_right, width_bottom, width_left; // border_widrh
			BorderStyle style_top, style_right, style_bottom, style_left; // border_style
		};
		Border* _border;
		// box attrs
		Vec2  _layout_offset; // 相对父视图的开始偏移位置（box包含margin值）
		Vec2  _layout_size; // 在布局中所占用的尺寸（margin+border+padding+content）
		float _layout_weight; // layout weight
		Align _layout_align; // layout align
	};


}
#endif
