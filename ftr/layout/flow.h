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

#ifndef __ftr__layout__flow__
#define __ftr__layout__flow__

#include "./box.h"

namespace ftr {

	/**
	 * @class FlowLayout
	 */
	class FX_EXPORT FlowLayout: public Box {
		FX_Define_View(FlowLayout);
		public:

		enum ItemsAlign: uint8_t {
			START, // 左对齐
			END, // 右对齐
			CENTER, // 居中
			SPACE_BETWEEN, // 两端对齐，项目之间的间隔都相等
			SPACE_AROUND, // 每个项目两侧的间隔相等。所以，项目之间的间隔比项目与边框的间隔大一倍
		};

		enum CrossAlign: uint8_t {
			START, // 与交叉轴的起点对齐
			END, // 与交叉轴的终点对齐
			CENTER, // 与交叉轴的中点对齐
			BASELINE, // 项目的第一行文字的基线对齐
			STRETCH, // 如果项目未设置高度或设为auto，将占满整个容器的高度
		};

		enum MultiAlign: uint8_t {
			START, // 与交叉轴的起点对齐
			END, // 与交叉轴的终点对齐
			CENTER, // 与交叉轴的中点对齐
			SPACE_BETWEEN, // 与交叉轴两端对齐，轴线之间的间隔平均分布
			SPACE_AROUND, // 每根轴线两侧的间隔都相等。所以，轴线之间的间隔比轴线与边框的间隔大一倍
			STRETCH, // 轴线占满整个交叉轴
		};

		// TODO ...
		private:
		Direction  _direction;
		ItemsAlign _items_align;
		CrossAlign _cross_align;
		MultiAlign _multi_align;
	};

}

#endif