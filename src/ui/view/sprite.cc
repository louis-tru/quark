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
#include "../action/keyframe.h"

#define _async_call preRender().async_call

namespace qk {

	Sprite::Sprite(): View(), ImageSourceHold(), MatrixView(this)
		, _width(0), _height(0)
		, _frames(1), _frame(0), _gap(0), _direction(Direction::Row)
		, _keyAction(nullptr)
	{
		_visible_region = true;
	}

	View* Sprite::init(Window* win) {
		View::init(win);
		_keyAction = NewRetain<KeyframeAction>(win);
		_keyAction->set_speed(25); // Default 25 frames per second
		_keyAction->set_loop(0xffffffff); // 0xffffffff means loop forever
		_keyAction->set_target(this);
		return this;
	}

	void Sprite::destroy() {
		Releasep(_keyAction); // Delete action
		View::destroy(); // Call parent destroy
	}

	String Sprite::src() const {
		return ImageSourceHold::src();
	}

	void Sprite::set_src(String val, bool isRt) {
		ImageSourceHold::set_src(val);
	}

	void Sprite::set_width(float val, bool isRt) {
		if (_width != val) {
			_width = val;
			mark(kTransform, isRt);
		}
	}

	void Sprite::set_height(float val, bool isRt) {
		if (_height != val) {
			_height = val;
			mark(kTransform, isRt);
		}
	}

	void Sprite::set_frame(uint32_t val, bool isRt) {
		if (_frame != val) {
			_frame = val;
			if (!isRt) { // is main thread
				_keyAction->seek(val * 1e3); // Seek to frame in milliseconds
			}
			mark(kLayout_None, isRt);
		}
	}

	void Sprite::set_frames(uint32_t val, bool isRt) {
		if (isRt) // return when render thread call, not allow call in the render thread
			return;
		if (_frames != val) {
			_frames = Qk_Min(1, val);
			if (_frame >= _frames) {
				_frame = _frames - 1;
			}
			_async_call([](auto self, auto arg) {
				auto action = self->_keyAction;
				auto frames = self->_frames;
				if (action->length() != frames + 1) {
					action->unsafe_clear(true);
					if (frames > 1) {
						for (uint32_t i = 0, count = frames + 1; i < count; i++) {
							action->unsafe_add(i * 1e3, LINEAR, true)->set_frame_Rt(i);
						}
					}
				}
			}, this, 0);
		}
	}

	uint8_t Sprite::fsp() const {
		return _keyAction->speed();
	}

	void Sprite::set_fsp(uint8_t val) {
		_keyAction->set_speed(Qk_Min(60, val)); // Use speed as fsp
	}

	void Sprite::set_gap(uint16_t val, bool isRt) {
		if (_gap != val) {
			_gap = val;
			mark(kLayout_None, isRt);
		}
	}

	void Sprite::set_direction(Direction val, bool isRt) {
		if (_direction != val) {
			_direction = val;
			mark(kLayout_None, isRt);
		}
	}

	bool Sprite::playing() const {
		return _keyAction->playing();
	}

	void Sprite::set_playing(bool val) {
		_keyAction->set_playing(val);
	}

	void Sprite::play(bool all) {
		if (all && action()) {
			action()->play();
		}
		_keyAction->play();
	}

	void Sprite::stop(bool all) {
		if (all && action()) {
			action()->stop();
		}
		_keyAction->stop();
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

	bool Sprite::overlap_test(Vec2 point) {
		// TODO ...
		return false;
	}

	Vec2 Sprite::center() {
		return { _width * 0.5f - _origin_value.x(), _height * 0.5f - _origin_value.y() };
	}

	void Sprite::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & kTransform) { // Update transform matrix
			solve_origin_value({_width, _height}); // Check transform_origin change
			unmark(kTransform); // Unmark
			auto v = layout_offset() + parent->layout_offset_inside()
				+ _origin_value + _translate;
			_matrix = Mat(mat).set_translate(parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]);
			// solve_visible_region(_matrix);
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
