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

#include "./sprite.h"

namespace qk {

	Sprite::Sprite() : View(), MatrixView(this) {
		// Initialize sprite specific properties if needed
	}

	ViewType Sprite::viewType() const {
		return kSprite_ViewType;
	}

	MatrixView* Sprite::asMatrixView() {
		return this;
	}

	Vec2 Sprite::layout_offset_inside() {
		return _origin;
	}

	Vec2 Sprite::center() {
		// return _origin;
		return {};
	}

	void Sprite::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			// _CheckParent();
			unmark(kTransform | kVisible_Region); // unmark

			// auto v = layout_offset() + _parent->layout_offset_inside()
			// 	+ Vec2(margin_left(), margin_top()) + _origin + _translate;
			// _matrix = Mat(mat).set_translate(_parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			// _position = Vec2(_matrix[2],_matrix[5]);
			// solve_visible_region(_matrix);
		}
		else if (mark & kVisible_Region) {
			// unmark(kVisible_Region); // unmark
			// solve_visible_region(_matrix.set_translate(_position));
		}
		//_matrix.set_translate(Vec2(0)); // clear translate, use position value
	}
}
