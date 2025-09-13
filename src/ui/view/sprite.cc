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
#include "../painter.h"

#define _async_call preRender().async_call

namespace qk {

	Vec2 free_typesetting(View* view, View::Container &container);

	SpriteView::SpriteView(): View(), MatrixView(this), _boundsOk(false) {
		_test_visible_region = false;
		set_receive(false);
	}

	MatrixView* SpriteView::asMatrixView() {
		return this;
	}

	Vec2 SpriteView::layout_offset_inside() {
		return -_origin_value;
	}

	// solve the origin value by client size
	// The origin value is the final value by computing the origin.
	void SpriteView::solve_origin_value(Vec2 client, Vec2 from) {
		switch (_origin_x.kind) {
			default:
			case BoxOriginKind::Auto:  _origin_value.set_x(0); break; // use skeleton origin
			case BoxOriginKind::Value: _origin_value.set_x(_origin_x.value); break;
			case BoxOriginKind::Ratio: _origin_value.set_x(client.x() * _origin_x.value); break;
		}
		switch (_origin_y.kind) {
			default:
			case BoxOriginKind::Auto:  _origin_value.set_y(0); break;
			case BoxOriginKind::Value: _origin_value.set_y(_origin_y.value); break;
			case BoxOriginKind::Ratio: _origin_value.set_y(client.y() * _origin_y.value); break;
		}
		_origin_value += from;
	}

	bool SpriteView::overlap_test(Vec2 point) {
		if (!_boundsOk) {
			_boundsOk = true;
			solve_bounds(_matrix, _bounds);
		}
		return overlap_test_from_convex_quadrilateral(_bounds, point);
	}

	void SpriteView::solve_visible_region(const Mat &mat) {
		if (test_visible_region()) {
			solve_bounds(mat, _bounds);
			_boundsOk = true;
			_visible_region = is_visible_region(mat, _bounds);
		} else {
			_boundsOk = false;
			_visible_region = true;
		}
	}

	void SpriteView::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & (kTransform | kVisible_Region)) { // Update transform matrix
			solve_origin_value(client_size(), client_size() * 0.5f); // Check transform_origin change
			unmark(kTransform | kVisible_Region); // Unmark
			auto v = parent->layout_offset_inside() + layout_offset() + _translate;
			_matrix = Mat(mat).set_translate(parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]); // the origin world coords
			solve_visible_region(_matrix);
		}
	}

	void SpriteView::trigger_listener_change(uint32_t name, int count, int change) {
		if ( change > 0 ) {
			set_receive(true);
		}
	}

	void SpriteView::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			Container c{
				client_size(), {}, {}, {}, kFixed_FloatState, kFixed_FloatState, false, false
			};
			free_typesetting(this, c);
		}
	}

	/////////////////////////////////////////////////////////////

	Sprite::Sprite(): SpriteView(), ImageSourceHold()
		, _width(0), _height(0)
		, _frame(0), _frames(1), _item(0), _items(1)
		, _gap(0), _direction(Direction::Row)
		, _keyAction(nullptr)
	{
	}

	void Sprite::destroy() {
		_keyAction->del_target(this);
		Releasep(_keyAction); // Delete action
		View::destroy(); // Call parent destroy
	}

	View* Sprite::init(Window* win) {
		View::init(win);
		_keyAction = NewRetain<KeyframeAction>(win);
		_keyAction->set_speed(25); // Default 25 frames per second
		_keyAction->set_loop(0xffffffff); // 0xffffffff means loop forever
		_keyAction->set_target(this);
		return this;
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
				_keyAction->seek(val * 1e3); // Seek to frame in milliseconds
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

	void Sprite::set_item(uint16_t val) {
		if (_item != val) {
			_item = val;
			mark(kLayout_None, false);
		}
	}

	void Sprite::set_items(uint16_t val) {
		if (_items != val) {
			_items = Qk_Max(1, val);
			mark(kLayout_None, false);
		}
	}

	void Sprite::set_gap(uint8_t val) {
		if (_gap != val) {
			_gap = val;
			mark(kLayout_None, false);
		}
	}

	uint8_t Sprite::fsp() const {
		return _keyAction->speed();
	}

	void Sprite::set_fsp(uint8_t val) {
		_keyAction->set_speed(Qk_Min(60, val)); // Use speed as fsp
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

	Vec2 Sprite::client_size() {
		return { _width, _height };
	}

	void Sprite::onSourceState(ImageSource::State state) {
		if (state == ImageSource::kSTATE_NONE) {
			mark(kLayout_None, false); // mark re-render
		}
		else if (state & ImageSource::kSTATE_LOAD_COMPLETE) {
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
		if (!src || !src->load()) {
			return painter->visitView(this, &matrix());
		}
		auto lastMatrix = painter->matrix();
		painter->set_matrix(&matrix());

		float w = src->width(), h = src->height();
		float x = 0, y = 0;
		auto gap = _gap;
		auto frame = _frame % _frames; // current frame
		auto item = _item % _items; // current item

		if (_direction == Direction::RowReverse || _direction == Direction::ColumnReverse) {
			frame = _frames - 1 - frame; // reverse frame
		}

		if (_direction == Direction::Row || _direction == Direction::RowReverse) {
			w /= _frames; // horizontal frames
			h /= _items; // adjust height
			x = w * frame + gap; // adjust x position
			y = h * item + gap; // adjust y position
		} else {
			h /= _frames; // vertical frames
			w /= _items; // adjust width
			y = h * frame + gap;
			x = w * item + gap; // adjust x position
		}

		w -= gap + gap;
		h -= gap + gap;

		Paint paint;
		ImagePaint img;
		paint.antiAlias = false;
		paint.type = Paint::kBitmap_Type;
		paint.image = &img;
		//paint.color.set_a(painter->opacity());
		paint.color *= painter->color();

		Rect dest{/*Vec2(_AAShrink * 0.5)*/ -origin_value(), {_width, _height}};

		img.setImage(src.get(), dest, {{x,y}, {w,h}});
		img.filterMode = ImagePaint::kLinear_FilterMode;
		img.mipmapMode = ImagePaint::kLinearNearest_MipmapMode;

		painter->canvas()->drawPathv(painter->cache()->getRectPath(dest), paint);
		painter->visitView(this);
		painter->set_matrix(lastMatrix); // restore previous matrix
	}

}
