/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "./flow.h"

namespace flare {

	/**
		*
		* Accepting visitors
		* 
		* @func accept(visitor)
		*/
	void Flow::accept(Visitor *visitor) {
		visitor->visitFlow(this);
	}

	/**
		* @constructors
		*/
	Flow::Flow()
		: _direction(Direction::ROW)
		, _cross_align(CrossAlign::START)
		, _wrap(Wrap::WRAP)
		, _wrap_align(WrapAlign::START)
	{
	}

	/**
		*
		* 设置主轴的方向
		*
		* @func set_direction(val)
		*/
	void Flow::set_direction(Direction val) {
		if (val != _direction) {
			_direction = val;
			mark(M_LAYOUT_TYPESETTING); // 排版参数改变,后续需对子布局重新排版
		}
	}

	/**
		* 
		* 设置交叉轴的对齐方式
		*
		* @func set_cross_align(align)
		*/
	void Flow::set_cross_align(CrossAlign align) {
		if (align != _cross_align) {
			_cross_align = align;
			mark(M_LAYOUT_TYPESETTING);
		}
	}

	/**
		* 
		* 主轴溢出包裹，开启后当主轴溢出时分裂成多根交叉轴
		*
		* @func set_wrap_reverse(wrap)
		*/
	void Flow::set_wrap(Wrap wrap) {
		if (wrap != _wrap) {
			_wrap = wrap;
			mark(M_LAYOUT_TYPESETTING);
		}
	}

	/**
		* 
		* 设置多根交叉轴的对齐方式
		*
		* @func set_wrap_align(align)
		*/
	void Flow::set_wrap_align(WrapAlign align) {
		if (align != _wrap_align) {
			_wrap_align = align;
			mark(M_LAYOUT_TYPESETTING);
		}
	}

	// --------------- o v e r w r i t e ---------------

	bool Flow::layout_reverse(uint32_t mark) {
		if (mark & (M_LAYOUT_TYPESETTING)) {
			if (!is_ready_layout_typesetting()) {
				return true; // continue iteration
			}

			// TODO ...

			unmark(M_LAYOUT_TYPESETTING);
		}
		return false;
	}

}

// *******************************************************************