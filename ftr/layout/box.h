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

#ifndef __ftr__layout__box__
#define __ftr__layout__box__

#include "./view.h"
#include "../core/fill.h"

namespace ftr {

	/**
	 * @class Box
	 */
	class FX_EXPORT Box: public View {
		FX_Define_View(Box);
		public:

		/**
		* @enum ValueType
		*/
		enum ValueType: uint8_t {
			AUTO = value::AUTO,    /* 自动值  auto */
			FULL = value::FULL,    /* 吸附到父视图(client边距与父视图重叠) full */
			PIXEL = value::PIXEL,   /* 像素值  px */
			PERCENT = value::PERCENT, /* 百分比  % */
			MINUS = value::MINUS,   /* 减法(parent-value) ! */
		};

		typedef value::ValueTemplate<ValueType, ValueType::AUTO> Value;

		/**
		 * @constructors
		 */
		Box();

		/**
		 * @destructor
		 */
		virtual ~Box();

		/**
		 * @func width()
		 */
		inline Value width() const {
			return _width;
		}
		
		/**
		 * @func height()
		 */
		inline Value height() const {
			return _height;
		}

		/**
		 * @func fill()
		 */
		inline FillPtr fill() {
			return _fill;
		}

		/**
		 *
		 * 设置布局对齐方式
		 *
		 * @func set_layout_align(align)
		 */
		void set_layout_align(LayoutAlign align);

		/**
		 * @overwrite
		 */
		virtual bool layout_forward(uint32_t mark);
		virtual bool layout_reverse(uint32_t mark);
		virtual void layout_recursive(uint32_t mark);
		virtual Vec2 layout_offset();
		virtual Vec2 layout_size();
		virtual Vec2 layout_content_size(bool& is_explicit_out);
		virtual float layout_weight(Direction direction);
		virtual LayoutAlign layout_align();
		virtual Vec2 layout_transform_origin(Transform& t);
		virtual void lock_layout_size(bool is_lock, Vec2 layout_size);
		virtual void set_layout_offset(Vec2 val);
		virtual void set_layout_offset_lazy();
		virtual void layout_content_change_notice(Layout* child);
		virtual void layout_size_change_notice_from_parent(Layout* parent);

		private:
		// box attrs
		Vec2  _layout_offset; // 相对父视图的开始偏移位置（box包含margin值）
		Vec2  _layout_size; // 在布局中所占用的尺寸（margin+border+padding+content）
		// size
		Value _width, _height; // width,height
		Vec2  _size; // width,height
		Vec4  _margin, _padding; // top,right,bottom,left
		// fill
		FillPtr _fill; // color|shadow|image|gradient|border|border-radius
		// layout align
		LayoutAlign _layout_align;
	};

}

#endif