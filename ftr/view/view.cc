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

#include "./view.h"

namespace ftr {

	View::View() {
		// TODO ...
	}

	View::~View() {
		// TODO ...
	}

	void View::layout_forward() {
		// TODO ...
	}

	void View::layout_reverse() {
		// TODO ...
	}

	void View::layout_size_lock(bool lock, Vec2 size) {
		// TODO ...
	}

	// 布局内部偏移补偿
	Vec2 View::layout_inside_offset() const {
		return _final_origin;
	}

	void View::draw() {
		// TODO ...
	}

	// 计算基础变换矩阵
	Mat View::matrix() {
		Vec2 offset = _layout_offset_start; // xy 布局偏移
		Vec2 layout_in = _parent ? _parent->layout_inside_offset(): Vec2();
		offset.x( offset.x() + _final_origin.x() + _translate.x() - layout_in.x() );
		offset.y( offset.y() + _final_origin.y() + _translate.y() - layout_in.y() );
		return Mat(offset, _scale, -_rotate, _skew);
	}

	const Mat& View::final_matrix() {
		if (1) { // update final_matrix
			if (_parent) {
				_parent->final_matrix().multiplication(matrix(), _final_matrix);
			} else {
				_final_matrix = matrix();
			}
		}
		return _final_matrix;
	}

}