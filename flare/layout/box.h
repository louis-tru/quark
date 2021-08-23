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

#ifndef __flare__layout__box__
#define __flare__layout__box__

#include "./view.h"
#include "../fill.h"

namespace flare {

	/**
	 * @class Box
	 */
	class FX_EXPORT Box: public View {
		FX_Define_View(Box);
		public:

		/**
		* @enum SizeType
		*/
		enum SizeType: uint8_t {
			NONE  = value::NONE,    /* none default wrap content */
			WRAP  = value::WRAP,    /* 包裹内容 wrap content */
			MATCH = value::MATCH,   /* 匹配父视图 match parent */
			PIXEL = value::PIXEL,   /* 明确值 value px */
			RATIO = value::RATIO,   /* 百分比 value % */
			MINUS = value::MINUS,   /* 减法(parent-value) value ! */
		};

		typedef value::TemplateValue<SizeType, SizeType::WRAP> SizeValue;

		/**
		 * @constructors
		 */
		Box();

		/**
		 * @destructor
		 */
		virtual ~Box();

		// define props
		FX_Define_Prop(SizeValue, width); // size width
		FX_Define_Prop(SizeValue, height); // size height
		FX_Define_Prop(SizeValue, limit_width); // limit max size
		FX_Define_Prop(SizeValue, limit_height);
		FX_Define_Prop(float, margin_top); // margin
		FX_Define_Prop(float, margin_right);
		FX_Define_Prop(float, margin_bottom);
		FX_Define_Prop(float, margin_left);
		FX_Define_Prop(float, padding_top); // padding
		FX_Define_Prop(float, padding_right);
		FX_Define_Prop(float, padding_bottom);
		FX_Define_Prop(float, padding_left);
		FX_Define_Prop(FillPtr, fill); // color|shadow|image|gradient|border|border-radius

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

		// --------------- o v e r w r i t e ---------------
		/**
		 * @overwrite
		 */
		virtual bool layout_forward(uint32_t mark);
		virtual bool layout_reverse(uint32_t mark);
		virtual Vec2 layout_offset();
		virtual Size layout_size();
		virtual Size layout_raw_size();
		/*
		* 这里定义项目的放大与缩小比例，默认为0，即如果存在剩余空间，不放大也不缩小 
		* 在flex中：size = size_raw + overflow * weight / weight_total * min(weight_total, 1)
		*/
		virtual float layout_weight();
		virtual Align layout_align();
		virtual Vec2 layout_lock(Vec2 layout_size);
		virtual Vec2 solve_transform_origin();
		virtual void set_layout_offset(Vec2 val);
		virtual void set_layout_offset_lazy(Vec2 origin, Vec2 size);
		virtual void layout_content_size_change(Layout* parent, uint32_t mark);

		// overwrite set_parent
	 protected:
		/**
		 * 
		 * is ready layout layout typesetting in the `layout_reverse() or layout_forward()` func
		 *
		 * @func is_ready_layout_typesetting()
		 */
		bool is_ready_layout_typesetting();

		/**
		 * @func solve_layout_size()
		 */
		uint32_t solve_layout_size(uint32_t mark);
		Vec2     solve_layout_lock(Vec2 layout_size, bool is_mark_child);

		// --------------- m e m b e r . f i e l d ---------------
	 private:
		// box attrs
		Vec2  _layout_offset; // 相对父视图的开始偏移位置（box包含margin值）
		Vec2  _layout_size; // 在布局中所占用的尺寸（margin+content+padding）
		float _layout_weight; // layout weight
		Align _layout_align; // layout align
		Vec2  _layout_content_size; // width,height / size
		bool  _wrap_x, _wrap_y; // layout content size wrap

		FX_DEFINE_INLINE_CLASS(Inl);
		FX_DEFINE_INLINE_CLASS(Inl_FlexLayout);
	};

}

#endif