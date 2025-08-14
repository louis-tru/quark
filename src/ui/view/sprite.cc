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
#include "../window.h"
#include "../app.h"
#include "../../errno.h"

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue

namespace qk {

	Sprite::Sprite(): View(), MatrixView(this)
		, _width(0), _height(0) {
	}

	void Sprite::set_width(float val, bool isRt) {
		if (_width != val) {
			_width = val;
			mark(kLayout_None, isRt);
		}
	}

	void Sprite::set_height(float val, bool isRt) {
		if (_height != val) {
			_height = val;
			mark(kLayout_None, isRt);
		}
	}

	String Sprite::src() const {
		return ImageSourceHold::src();
	}

	void Sprite::set_src(String val, bool isRt) {
		ImageSourceHold::set_src(val);
	}

	ViewType Sprite::viewType() const {
		return kSprite_ViewType;
	}

	MatrixView* Sprite::asMatrixView() {
		return this;
	}

	Vec2 Sprite::layout_offset_inside() {
		return -_origin_value;
	}

	Vec2 Sprite::center() {
		return { _width * 0.5f - _origin_value.x(), _height * 0.5f - _origin_value.y() };
	}

	void Sprite::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			_CheckParent();
			solve_origin_value({_width, _height}); // check transform_origin change
			unmark(kTransform); // unmark
			auto v = layout_offset() + _parent->layout_offset_inside()
				+ _origin_value + _translate;
			_matrix = Mat(mat).set_translate(_parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]);
		}
	}

	void Sprite::onSourceState(Event<ImageSource, ImageSource::State>& evt) {
		if (evt.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
			mark(kLayout_None, false); // mark re-render
			Sp<UIEvent> evt = new UIEvent(this);
			trigger(UIEvent_Load, **evt);
		} else if (evt.data() & (ImageSource::kSTATE_LOAD_ERROR | ImageSource::kSTATE_DECODE_ERROR)) {
			Sp<UIEvent> evt = new UIEvent(this, new Error(ERR_IMAGE_LOAD_ERROR, "ERR_IMAGE_LOAD_ERROR"));
			trigger(UIEvent_Error, **evt);
		}
	}

	ImagePool* Sprite::imgPool() {
		return window()->host()->imgPool();
	}

}
