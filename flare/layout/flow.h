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
		* @class Flow
		*/
	class F_EXPORT FlowLayout: public Box {
		F_Define_View(FlowLayout);
	 public:
		FlowLayout();

		// define props
		F_DEFINE_PROP(Direction, direction); // direction 排版方向
		F_DEFINE_PROP(CrossAlign, cross_align); // cross_align 交叉轴的对齐方式
		F_DEFINE_PROP(Wrap, wrap); // wrap 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
		F_DEFINE_PROP(WrapAlign, wrap_align); // wrap_align 多根交叉轴的对齐方式,如果项目只有一根交叉轴,该属性不起作用

		// --------------- o v e r w r i t e ---------------
		// @overwrite
		virtual bool layout_reverse(uint32_t mark);

		// --------------- m e m b e r . f i e l d ---------------
	 private:
		friend class Flex;
		F_DEFINE_INLINE_CLASS(Inl);
	};

}

#endif