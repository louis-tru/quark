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

#ifndef __flare__layout__flex__
#define __flare__layout__flex__

#include "./box.h"

namespace flare {

	/**
	 * @class FlexLayout
	 */
	class FX_EXPORT FlexLayout: public Box {
		FX_Define_View(FlexLayout);
		public:

		// 项目在主轴上的对齐方式
		enum ItemsAlign: uint8_t {
			START = value::START, // 左对齐
			CENTER = value::CENTER, // 居中
			END = value::END, // 右对齐
			SPACE_BETWEEN = value::SPACE_BETWEEN, // 两端对齐，项目之间的间隔都相等
			SPACE_AROUND = value::SPACE_AROUND, // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
			SPACE_EVENLY = value::SPACE_EVENLY, // 每个项目两侧的间隔相等,这包括边框的间距
		};

		// 项目在交叉轴内如何对齐
		enum CrossAlign: uint8_t {
			START = value::START, // 与交叉轴内的起点对齐
			CENTER = value::CENTER, // 与交叉轴内的中点对齐
			END = value::END, // 与交叉轴内的终点对齐
			STRETCH = value::STRETCH, // 如果项目未设置高度或设为auto,将占满交叉轴内空间
			// BASELINE = value::BASELINE, // 项目的第一行文字的基线对齐
		};

		// 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
		enum Wrap: uint8_t {
			NO_WRAP = value::NO_WRAP, // 只有一根交叉轴线
			WRAP = value::WRAP, // 溢出后会有多根交叉轴线
			WRAP_REVERSE = value::WRAP_REVERSE, // 多根交叉轴线反向排列
		};

		// 多根交叉轴线的对齐方式。如果项目只有一根交叉轴，该属性不起作用
		enum WrapAlign: uint8_t {
			START = value::START, // 与交叉轴的起点对齐
			CENTER = value::CENTER, // 与交叉轴的中点对齐
			END = value::END, // 与交叉轴的终点对齐
			SPACE_BETWEEN = value::SPACE_BETWEEN, // 与交叉轴两端对齐,轴线之间的间隔平均分布
			SPACE_AROUND = value::SPACE_AROUND, // 每根轴线两侧的间隔都相等,所以轴线之间的间隔比轴线与边框的间隔大一倍
			SPACE_EVENLY = value::SPACE_EVENLY, // 每根轴线两侧的间隔都相等,这包括边框的间距
			STRETCH = value::STRETCH, // 轴线占满整个交叉轴，平均分配剩余的交叉轴空间
		};

		/**
		 * @constructors
		 */
		FlexLayout();

		// define props
		FX_Define_Prop(Direction, direction); // direction 主轴的方向
		FX_Define_Prop(ItemsAlign, items_align); // items_align 主轴的对齐方式
		FX_Define_Prop(CrossAlign, cross_align); // cross_align 交叉轴的对齐方式
		FX_Define_Prop(Wrap, wrap); // wrap 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
		FX_Define_Prop(WrapAlign, wrap_align); // wrap_align 多根交叉轴的对齐方式,如果项目只有一根交叉轴,该属性不起作用

		// @overwrite
		virtual bool layout_forward(uint32_t mark);
		virtual bool layout_reverse(uint32_t mark);
		virtual void layout_typesetting_change_from_child_weight(Layout* child, float weight);

		// --------------- m e m b e r . f i e l d ---------------
		private:

	};

}

#endif