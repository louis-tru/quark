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

#ifndef __flare__layout__flow__
#define __flare__layout__flow__

#include "./box.h"

namespace flare {

	/**
	 * @class FlowLayout
	 */
	class FX_EXPORT FlowLayout: public Box {
		FX_Define_View(FlowLayout);
		public:

		// layout direction
		enum Direction: uint8_t {
			ROW = value::ROW,
			ROW_REVERSE = value::ROW_REVERSE,
			COLUMN = value::COLUMN,
			COLUMN_REVERSE = value::COLUMN_REVERSE,
		};

		// 项目在主轴上的对齐方式
		enum ItemsAlign: uint8_t {
			START = value::START, // 左对齐
			CENTER = value::CENTER, // 居中
			END = value::END, // 右对齐
			SPACE_BETWEEN = value::SPACE_BETWEEN, // 两端对齐，项目之间的间隔都相等
			SPACE_AROUND = value::SPACE_AROUND, // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
		};

		// 项目在交叉轴上如何对齐
		enum CrossAlign: uint8_t {
			START = value::START, // 与交叉轴的起点对齐
			CENTER = value::CENTER, // 与交叉轴的中点对齐
			END = value::END, // 与交叉轴的终点对齐
			BASELINE = value::BASELINE, // 项目的第一行文字的基线对齐
			STRETCH = value::STRETCH, // 如果项目未设置高度或设为auto，将占满整个容器的高度
		};

		// 多根轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
		enum WrapAlign: uint8_t {
			START = value::START, // 与交叉轴的起点对齐
			CENTER = value::CENTER, // 与交叉轴的中点对齐
			END = value::END, // 与交叉轴的终点对齐
			SPACE_BETWEEN = value::SPACE_BETWEEN, // 与交叉轴两端对齐，轴线之间的间隔平均分布
			SPACE_AROUND = value::SPACE_AROUND, // 每根轴线两侧的间隔都相等。所以，轴线之间的间隔比轴线与边框的间隔大一倍
			STRETCH = value::STRETCH, // 轴线占满整个交叉轴
		};

		/**
		 * @constructors
		 */
		FlowLayout();

		/**
		 * 
		 * 获取主轴的方向
		 *
		 * @func direction()
		 */
		inline Direction direction() const {
			return _direction;
		}

		/**
		 * 
		 * 获取主轴的对齐方式
		 *
		 * @func items_align()
		 */
		inline ItemsAlign items_align() const {
			return _items_align;
		}

		/**
		 *
		 * 获取交叉轴的对齐方式
		 *
		 * @func corss_align()
		 */
		inline CrossAlign cross_align() const {
			return _cross_align;
		}

		/**
		 * 
		 * 获取多根交叉轴的对齐方式
		 *
		 * @func wrap_align() 
		 */
		inline WrapAlign wrap_align() const {
			return _wrap_align;
		}

		/**
		 * 
		 * 获取，多根交叉轴是否反向排列
		 *
		 * @func wrap_reverse()
		 */
		inline bool wrap_reverse() const {
			return _wrap_reverse;
		}
		
		/**
		 *
		 * 设置主轴的方向
		 *
		 * @func set_direction(val)
		 */
		void set_direction(Direction val);

		/**
		 * 
		 * 设置主轴的对齐方式
		 *
		 * @func stt_items_align(val)
		 */
		void set_items_align(ItemsAlign align);

		/**
		 * 
		 * 设置交叉轴的对齐方式
		 *
		 * @func set_cross_align(align)
		 */
		void set_cross_align(CrossAlign align);

		/**
		 * 
		 * 设置多根交叉轴的对齐方式
		 *
		 * @func set_wrap_align(val)
		 */
		void set_wrap_align(WrapAlign align);

		/**
		 * 
		 * 设置多根交叉轴是否反向排列
		 *
		 * @func set_wrap_reverse(reverse)
		 */
		void set_wrap_reverse(bool reverse);

		// --------------- o v e r w r i t e ---------------
		// @overwrite
		virtual bool layout_forward(uint32_t mark);
		virtual bool layout_reverse(uint32_t mark);

		// --------------- m e m b e r . f i e l d ---------------
		private:
		Direction  _direction; // 主轴的方向
		ItemsAlign _items_align; // 主轴的对齐方式
		CrossAlign _cross_align; // 交叉轴的对齐方式
		WrapAlign _wrap_align; // 多根交叉轴的对齐方式，如果项目只有一根交叉轴，该属性不起作用
		bool _wrap_reverse; // 多根交叉轴是否反向排列
	};

}

#endif