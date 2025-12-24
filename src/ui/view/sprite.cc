/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./sprite.h"
#include "../../errno.h"
#include "../window.h"
#include "../app.h"
#include "../action/keyframe.h"
#include "../painter.h"
#include "../geometry.h"

#define _IfAct(...)  auto act = _keyAction; if (!act) return __VA_ARGS__
#define _async_call preRender().async_call

namespace qk {

	Sprite::Sprite(): Agent(), ImageSourceHold()
		, _width(0), _height(0)
		, _frame(0), _frames(1), _set(0), _sets(1)
		, _spacing(0), _direction(Direction::Row)
		, _keyAction(nullptr)
	{
		// sizeof(Sprite); // ensure complete type
	}

	void Sprite::destroy() {
		if (_keyAction) {
			_keyAction->del_target(this);
			Releasep(_keyAction); // Delete action
		}
		View::destroy(); // Call parent destroy
	}

	KeyframeAction *Sprite::getKeyAction() {
		if (!_keyAction) {
			_keyAction = NewRetain<KeyframeAction>(window());
			_keyAction->set_speed(25); // Default 25 frames per second
			_keyAction->set_loop(0xffffffff); // 0xffffffff means loop forever
			_keyAction->set_target(this);
		}
		return _keyAction;
	}

	void Sprite::set_width(float val, bool isRt) {
		if (_width != val) {
			_width = val;
			// mark(kTransform, isRt);
			mark_layout(kLayout_Typesetting | kTransform, isRt);
		}
	}

	void Sprite::set_height(float val, bool isRt) {
		if (_height != val) {
			_height = val;
			// mark(kTransform, isRt);
			mark_layout(kLayout_Typesetting | kTransform, isRt);
		}
	}

	void Sprite::set_frame(uint16_t val, bool isRt) {
		if (_frame != val) {
			_frame = val;
			if (!isRt) { // is main thread
				getKeyAction()->seek(val * 1e3); // Seek to frame in milliseconds
			}
			mark(kLayout_None, isRt);
		}
	}

	void Sprite::set_frames(uint16_t val) {
		if (_frames != val) {
			_frames = Qk_Max(1, val);
			if (_frame >= _frames) {
				_frame = _frames - 1;
			}
			getKeyAction(); // ensure action created
			_async_call([](auto self, auto arg) {
				auto action = self->_keyAction;
				auto frames = self->_frames;
				if (action->length() != frames + 1) {
					action->unsafe_clear(true);
					if (frames > 1) {
						for (uint32_t i = 0, count = frames + 1; i < count; i++) {
							action->unsafe_add(i * 1e3, LINEAR, true)->set_frame_rt(i);
						}
					}
				}
			}, this, 0);
		}
	}

	void Sprite::set_set(uint16_t val) {
		if (_set != val) {
			_set = val;
			mark(kLayout_None, false);
		}
	}

	void Sprite::set_sets(uint16_t val) {
		if (_sets != val) {
			_sets = Qk_Max(1, val);
			mark(kLayout_None, false);
		}
	}

	void Sprite::set_spacing(uint8_t val) {
		if (_spacing != val) {
			_spacing = val;
			mark(kLayout_None, false);
		}
	}

	uint8_t Sprite::frequency() const {
		_IfAct(25); // default 25
		return act->speed();
	}

	void Sprite::set_frequency(uint8_t val) {
		getKeyAction()->set_speed(Qk_Min(60, val)); // Use speed as fsp
	}

	void Sprite::set_direction(Direction val, bool isRt) {
		if (_direction != val) {
			_direction = val;
			mark(kLayout_None, isRt);
		}
	}

	String Sprite::src() const {
		return ImageSourceHold::src();
	}

	void Sprite::set_src(String val, bool isRt) {
		if (ImageSourceHold::set_src(val)) {
			mark(kLayout_None, isRt); // mark re-render
		}
	}

	bool Sprite::playing() const {
		_IfAct(false);
		return act->playing();
	}

	void Sprite::set_playing(bool val) {
		getKeyAction()->set_playing(val);
	}

	void Sprite::play() {
		getKeyAction()->play();
	}

	void Sprite::stop() {
		getKeyAction()->stop();
	}

	ViewType Sprite::viewType() const {
		return kSprite_ViewType;
	}

	Vec2 Sprite::client_size() {
		return {_width, _height};
	}

	Region Sprite::client_region() {
		auto begin = -_origin_value;
		return { begin, begin + Vec2{_width, _height}, _translate };
	}

	void Sprite::onSourceState(ImageSource::State state) {
		if (state & ImageSource::kSTATE_LOAD_COMPLETE) {
			mark(kLayout_None, false); // mark re-render
			Sp<UIEvent> evt = new UIEvent(this);
			trigger(UIEvent_Load, **evt);
		}
		else if (state & (ImageSource::kSTATE_LOAD_ERROR | ImageSource::kSTATE_DECODE_ERROR)) {
			Sp<UIEvent> evt = new UIEvent(this, new Error(ERR_IMAGE_LOAD_ERROR, "ERR_IMAGE_LOAD_ERROR"));
			trigger(UIEvent_Error, **evt);
		}
	}

	ImagePool* Sprite::imgPool() {
		return window()->host()->imgPool();
	}

	void Sprite::draw(Painter *painter) {
		auto src = source();
		if (!src || !src->load() || !painter->color().a()) {
			return Entity::draw(painter);
		}

		auto lastMatrix = painter->matrix();
		painter->set_matrix(&matrix());

		float w = src->width(), h = src->height();
		float x = 0, y = 0;
		auto spacing = _spacing;
		auto frame = _frame % _frames; // current frame
		auto set = _set % _sets; // current set

		if (_direction == Direction::RowReverse || _direction == Direction::ColumnReverse) {
			frame = _frames - 1 - frame; // reverse frame
		}

		if (_direction == Direction::Row || _direction == Direction::RowReverse) {
			w /= _frames; // horizontal frames
			h /= _sets; // adjust height
			x = w * frame + spacing; // adjust x position
			y = h * set + spacing; // adjust y position
		} else {
			h /= _frames; // vertical frames
			w /= _sets; // adjust width
			y = h * frame + spacing;
			x = w * set + spacing; // adjust x position
		}

		w -= spacing + spacing;
		h -= spacing + spacing;

		Paint paint;
		PaintImage img;
		paint.antiAlias = aa();
		paint.fill.image = &img;
		paint.fill.color = painter->color();

		if (!src->premultipliedAlpha()) {
			paint.fill.color = paint.fill.color.recover_unpremul_alpha();
			paint.blendMode = kSrcOver_BlendMode;
		}

		auto aaShrink = aa() ? painter->AAShrink() : 0;
		Rect rect{-origin_value(), {_width, _height}};
		Rect rectAA{
			Vec2(aaShrink * 0.5)-origin_value(),
			{_width-aaShrink, _height-aaShrink},
		};

		img.setImage(src.get(), rect, {{x,y}, {w,h}});
		img.filterMode = default_FilterMode;
		img.mipmapMode = default_MipmapMode;

		painter->canvas()->drawPathv(painter->cache()->getRectPath(rectAA), paint);
		debugDraw(painter); // draw debug bounds
		painter->visitView(this);
		painter->set_matrix(lastMatrix); // restore previous matrix
	}

}
