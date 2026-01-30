/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, Louis.chu
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

#include "./box.h"
#include "../text/text_lines.h"
#include "../text/text_opts.h"
#include <limits>

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue
#define _BorderV(self) auto _border = self->_border.load()
#define _IfBorderV(self) _BorderV(self); if (_border)
#define _Border() _BorderV(this)
#define _IfBorder() _IfBorderV(this)

namespace qk {
	typedef Box::Container::Pre Pre;

	constexpr float Max_Float = std::numeric_limits<float>::max();

	Vec2 free_typesetting(View* view, const View::Container &container) {
		auto cur = container.content;
		auto v = view->first();
		if (v) {
			if (container.float_x() || container.float_y()) { // float width
				Vec2 maxSize;
				do {
					if (v->visible()) {
						auto size = v->layout_size();
						if (size[0] > maxSize[0])
							maxSize[0] = size[0];
						if (size[1] > maxSize[1])
							maxSize[1] = size[1];
					}
					v = v->next();
				} while(v);
				if (container.float_x())
					cur[0] = container.clamp_width(maxSize[0]);
				if (container.float_y())
					cur[1] = container.clamp_height(maxSize[1]);
			}

			auto v = view->first();
			do { // lazy free layout
				if (v->visible())
					v->set_layout_offset_free(cur); // free layout
				v = v->next();
			} while(v);
		} else {
			if (container.float_x())
				cur[0] = container.clamp_width(0);
			if (container.float_y())
				cur[1] = container.clamp_height(0);
		}
		view->unmark(View::kLayout_Typesetting);
		return cur;
	}

	Pre Box::solve_layout_content_pre_width(const Container &pContainer) {
		float size = pContainer.content[0];
		float min = 0, max = Max_Float;
		auto pFloat = pContainer.float_x();
		auto state = pContainer.state_x;

		static auto computeMatch = [](Box* self, float pSi) {
			float val = pSi - self->_margin_left 
				-self->_margin_right - self->_padding_left - self->_padding_right;
			_IfBorderV(self)
				val -= (_border->width[3] + _border->width[1]); // left + right
			return Qk_Max(val,0);
		};

		#define is_PreWidth1_Ne_Max_Float() (pContainer.pre_width[1] != Max_Float)

		switch (_min_width.kind) {
			default: /* None default wrap content */
			case BoxSizeKind::Auto: /* 包裹内容 wrap content */
				state = kNone_FloatState;
				break;
			case BoxSizeKind::Value: /* 明确值 value rem */
				state = kFixed_FloatState;
				min = max = _min_width.value; // explicit value
				break;
			case BoxSizeKind::Match: /* 匹配父视图 match parent */
				if (pFloat) { // wrap
					if (pContainer.pre_width[0]) {
						min = computeMatch(this, pContainer.pre_width[0]);
					}
					if (is_PreWidth1_Ne_Max_Float()) {
						max = computeMatch(this, pContainer.pre_width[1]);
					}
				} else {
					min = max = computeMatch(this, size);
				}
				break;
			case BoxSizeKind::Ratio: /* 百分比 value % */
				if (pFloat) {
					if (pContainer.pre_width[0]) {
						min = Float32::max(pContainer.pre_width[0] * _min_width.value, 0);
					}
					if (is_PreWidth1_Ne_Max_Float()) {
						max = Float32::max(pContainer.pre_width[1] * _min_width.value, 0);
					}
				} else {
					min = max = Float32::max(size * _min_width.value, 0);
				}
				break;
			case BoxSizeKind::Minus: /* 减法(parent-value) value ! */
				if (pFloat) { // compute min and max
					if (pContainer.pre_width[0]) {
						min = Float32::max(pContainer.pre_width[0] - _min_width.value, 0);
					}
					if (is_PreWidth1_Ne_Max_Float()) {
						max = Float32::max(pContainer.pre_width[1] - _min_width.value, 0);
					}
				} else {
					min = max = Float32::max(size - _min_width.value, 0);
				}
				break;
		}

		switch (_max_width.kind) {
			case BoxSizeKind::None:
				return {{min, max}, state};
			case BoxSizeKind::Auto:
				return {{min, Max_Float}, kNone_FloatState};
			case BoxSizeKind::Value:
				max = _max_width.value;
				break;
			case BoxSizeKind::Match: {
				if (!pFloat) {
					max = computeMatch(this, size);
				} else if (is_PreWidth1_Ne_Max_Float()) {
					max = computeMatch(this, pContainer.pre_width[1]);
				}
				break;
			}
			case BoxSizeKind::Ratio:
				if (!pFloat) {
					max = size * _max_width.value;
				} else if (is_PreWidth1_Ne_Max_Float()) {
					max = pContainer.pre_width[1] * _max_width.value;
				}
				break;
			case BoxSizeKind::Minus:
				if (!pFloat) {
					max = size - _max_width.value;
				} else if (is_PreWidth1_Ne_Max_Float()) {
					max = pContainer.pre_width[1] - _max_width.value;
				}
				break;
		}

		if (max < min) {
			max = min;
		}
		return {{min, max},kNone_FloatState};
	}

	Pre Box::solve_layout_content_pre_height(const Container &pContainer) {
		float size = pContainer.content[1];
		float min = 0, max = Max_Float;
		auto pFloat = pContainer.float_y();
		auto state = pContainer.state_y;

		static auto computeMatch = [](Box* self, float pSi) {
			float val = pSi - self->_margin_top
				-self->_margin_bottom - self->_padding_top - self->_padding_bottom;
			_IfBorderV(self)
				val -= (_border->width[0] + _border->width[2]); // top + bottom
			return Qk_Max(val,0);
		};

		#define is_PreHeight1_Ne_Max_Float() (pContainer.pre_height[1] != Max_Float)

		switch (_min_height.kind) {
			default: /* None default wrap content */
			case BoxSizeKind::Auto: /* 包裹内容 wrap content */
				state = kNone_FloatState;
				break;
			case BoxSizeKind::Value: /* 明确值 value rem */
				state = kFixed_FloatState;
				min = max = _min_height.value; // explicit value
				break;
			case BoxSizeKind::Match: /* 匹配父视图 match parent */
				if (pFloat) { // wrap
					if (pContainer.pre_height[0]) {
						min = computeMatch(this, pContainer.pre_height[0]);
					}
					if (is_PreHeight1_Ne_Max_Float()) {
						max = computeMatch(this, pContainer.pre_height[1]);
					}
				} else {
					min = max = computeMatch(this, size);
				}
				break;
			case BoxSizeKind::Ratio: /* 百分比 value % */
				if (pFloat) {
					if (pContainer.pre_height[0]) {
						min = Float32::max(pContainer.pre_height[0] * _min_height.value, 0);
					}
					if (is_PreHeight1_Ne_Max_Float()) {
						max = Float32::max(pContainer.pre_height[1] * _min_height.value, 0);
					}
				} else {
					min = max = Float32::max(size * _min_height.value, 0);
				}
				break;
			case BoxSizeKind::Minus: /* 减法(parent-value) value ! */
				if (pFloat) { // compute min and max
					if (pContainer.pre_height[0]) {
						min = Float32::max(pContainer.pre_height[0] - _min_height.value, 0);
					}
					if (is_PreHeight1_Ne_Max_Float()) {
						max = Float32::max(pContainer.pre_height[1] - _min_height.value, 0);
					}
				} else {
					min = max = Float32::max(size - _min_height.value, 0);
				}
				break;
		}

		switch (_max_height.kind) {
			case BoxSizeKind::None:
				return {{min, max}, state};
			case BoxSizeKind::Auto:
				return {{min, Max_Float}, kNone_FloatState};
			case BoxSizeKind::Value:
				max = _max_height.value;
				break;
			case BoxSizeKind::Match:
				if (!pFloat) {
					max = computeMatch(this, size);
				} else if (is_PreHeight1_Ne_Max_Float()) {
					max = computeMatch(this, pContainer.pre_height[1]);
				}
				break;
			case BoxSizeKind::Ratio:
				if (!pFloat) {
					max = size * _max_height.value;
				} else if (is_PreHeight1_Ne_Max_Float()) {
					max = pContainer.pre_height[1] * _max_height.value;
				}
				break;
			case BoxSizeKind::Minus:
				if (!pFloat) {
					max = size - _max_height.value;
				} else if (is_PreHeight1_Ne_Max_Float()) {
					max = pContainer.pre_height[1] - _max_height.value;
				}
				break;
		}

		if (max < min) {
			max = min;
		}
		return {{min, max},kNone_FloatState};
	}

	uint32_t Box::solve_layout_content_size_pre(uint32_t &mark, const Container &pContainer) {
		uint32_t change_mark = kLayout_None;

		if (mark & kLayout_Inner_Width) {
			if (_container.set_pre_width(solve_layout_content_pre_width(pContainer))) {
				if (_container.state_x) { // fixed width
					_container.content[0] = _container.pre_width[0];
					_container.content_diff_before_locking[0] = 0;
					mark |= kLayout_Outside_Width;
				}
				change_mark = kLayout_Inner_Width;
			}
		}

		if (mark & kLayout_Inner_Height) {
			if (_container.set_pre_height(solve_layout_content_pre_height(pContainer))) {
				if (_container.state_y) { // fixed height
					_container.content[1] = _container.pre_height[0];
					_container.content_diff_before_locking[1] = 0;
					mark |= kLayout_Outside_Height;
				}
				change_mark |= kLayout_Inner_Height;
			}
		}

		return change_mark;
	}

	void Box::layout_forward(uint32_t mark) {
		if (mark & (kLayout_Size_ALL | kLayout_Child_Size/* | kStyle_Class*/)) {
			uint32_t change_mark = kLayout_None;
			_IfParent() {
				change_mark = solve_layout_content_size_pre(mark, _parent->layout_container());

				uint32_t child_layout_change_mark = kLayout_None;

				if (mark & kLayout_Outside_Width) {
					_client_size[0] = _padding_left + _padding_right + _container.content[0];
					_IfBorder() {
						_client_size[0] += _border->width[3] + _border->width[1]; // left + right
					}
					float x = _margin_left + _margin_right + _client_size[0];
					if (_layout_size[0] != x) {
						_layout_size[0] = x;
						child_layout_change_mark = kChild_Layout_Size;
					}
				}

				if (mark & kLayout_Outside_Height) {
					_client_size[1] = _padding_top + _padding_bottom + _container.content[1];
					_IfBorder() {
						_client_size[1] += _border->width[0] + _border->width[2]; // top + bottom
					}
					float y = _margin_top + _margin_bottom + _client_size[1];
					if (_layout_size[1] != y) {
						_layout_size[1] = y;
						child_layout_change_mark = kChild_Layout_Size;
					}
				}
				// Notify the parent view that the subview layout has changed,
				// maybe typesetting again and setting new layout offset.
				_parent->onChildLayoutChange(this, child_layout_change_mark);
			}

			if (mark & kLayout_Child_Size) {
				// it >> 4 to kLayout_Inner_Width and kLayout_Inner_Height
				change_mark |= ((mark & kLayout_Child_Size) >> 4);
			}

			unmark(kLayout_Size_ALL | kLayout_Child_Size);

			if (change_mark) {
				auto v = first();
				while (v) {
					if (v->visible()) {
						v->layout_forward(change_mark | v->mark_value());
					}
					v = v->next();
				}
				mark_layout<true>(kLayout_Typesetting | kVisible_Region); // layout reverse
			}
		}
	}

	void Box::layout_reverse(uint32_t mark) {
		if (mark & kLayout_Typesetting) {
			if (_layout == LayoutType::Free) {
				set_content_size(layout_typesetting_free());
				delete_lock_state();
			} else {
				layout_typesetting_float();
			}
		}
	}

	float Box::layout_lock_width(float size) {
		float bp_x = _padding_left + _padding_right;
		_IfBorder() {
			bp_x += _border->width[3] + _border->width[1]; // left + right
		}
		float mbp_x = _margin_left + _margin_right + bp_x;
		float content = size - mbp_x;

		content =
			_max_width.kind == BoxSizeKind::None ? // no limit
			Float32::max(content, 0):
			_container.clamp_width(content); // limit

		_client_size[0] = bp_x + content;
		_layout_size[0] = mbp_x + content;

		if (_container.content.x() != content || (_container.float_x() && !_container.locked_x)) {
			_container.content_diff_before_locking[0] += content - _container.content[0];
			_container.content[0] = content;
			_container.state_x |= kFixedByLock_FloatState; // Add lock state
			_container.locked_x = true;
			mark_layout<true>(kLayout_Child_Width);
		}

		// unmark(kLayout_Inner_Width);

		return _layout_size[0];
	}

	float Box::layout_lock_height(float size) {
		float bp_y = _padding_top + _padding_bottom;
		_IfBorder() {
			bp_y += _border->width[0] + _border->width[2]; // top + bottom
		}
		float mbp_y = _margin_top + _margin_bottom + bp_y;
		float content = size - mbp_y;

		content =
			_max_height.kind == BoxSizeKind::None ? // no limit
			Float32::max(content, 0):
			_container.clamp_height(content); // limit

		_client_size[1] = bp_y + content;
		_layout_size[1] = mbp_y + content;

		if (_container.content.y() != content || (_container.float_y() && !_container.locked_y)) {
			_container.content_diff_before_locking[1] += content - _container.content[1];
			_container.content[1] = content;
			_container.state_y |= kFixedByLock_FloatState; // Add lock state
			_container.locked_y = true;
			mark_layout<true>(kLayout_Child_Height);
		}

		// unmark(kLayout_Inner_Height);

		return _layout_size[1];
	}

	Vec2 Box::layout_typesetting_free() {
		return free_typesetting(this, _container);
	}

	Vec2 Box::layout_typesetting_float() {
		Vec2 inner_size;
		auto cur_x = _container.content[0];

		auto v = first();
		if (v) {
			if ( _container.float_x() ) { // float width
				float limitX = _container.pre_width[1];
				float float_x = 0;
				cur_x = 0;
				do {
					if (v->visible()) {
						if (v->layout_align() > Align::NewEnd) { // float
							auto x = v->layout_size().x();
							auto newX = float_x + x;
							float_x = newX > limitX ? x: newX;
						} else { // use float
							float_x = v->layout_size().x();
						}
						cur_x = Float32::max(cur_x, float_x);
					}
					v = v->next();
				} while(v);
				v = first();
				cur_x = _container.clamp_width(cur_x);
			}

			float left = 0, right = 0;
			float offset_y = 0;
			float line_width = 0, max_width = 0;
			float line_height = 0;
			Array<View*> centerV;

			auto solveCenter = [&]() {
				if (centerV.length()) {
					float centerSize = (line_width - left - right);
					float startOffset = (cur_x - centerSize) * 0.5;
					if (left > startOffset) { // can't
						startOffset = left;
					} else {
						float diff = (centerSize + startOffset) - (cur_x - right);
						if (diff > 0) { // can't
							startOffset -= diff;
						}
					}
					for (auto v: centerV) {
						v->set_layout_offset({startOffset, offset_y});
						startOffset += v->layout_size().x();
					}
					centerV.clear();
				}
			};

			auto nextStep = [&](Vec2 size) {
				auto new_line_width = line_width + size.x();
				if (new_line_width > cur_x && line_width != 0) { // new line
					solveCenter();
					max_width = Float32::max(max_width, line_width); // select max
					left = right = 0;
					offset_y += line_height;
					line_width = size.x();
					line_height = size.y();
				} else {
					line_width = new_line_width;
					line_height = Float32::max(line_height, size.y()); // select max
				}
			};

			do {
				if (v->visible()) {
					auto size = v->layout_size();

					switch(v->layout_align()) {
						case Align::Normal:
						case Align::Start: // float start
						case Align::FloatStart:
							nextStep(size);
							v->set_layout_offset(Vec2(left, offset_y));
							left += size.x();
							break;
						case Align::Center: // float center
						case Align::FloatCenter:
							nextStep(size);
							centerV.push(v);
							break;
						case Align::End: // float end
						case Align::FloatEnd:
							nextStep(size);
							v->set_layout_offset(Vec2(cur_x - right - size.x(), offset_y));
							right += size.x();
							break;
						case Align::NewStart: // new left
							solveCenter();
							left = 0;
							goto last;
						case Align::NewEnd: // new right
							solveCenter();
							left = cur_x - size.x();
							goto last;
						case Align::NewCenter: // new center
							solveCenter();
							left = (cur_x - size.x()) * 0.5f;
						last:
							offset_y += line_height;
							v->set_layout_offset({left,offset_y});
							max_width = Float32::max(max_width, left + size.x());
							left = right = 0;
							line_width = line_height = 0;
							offset_y += size.y();
							break;
					}
				}
				v = v->next();
			} while(v);
			solveCenter();
			inner_size = Vec2(Float32::max(max_width, line_width), offset_y + line_height);
		} else {
			if (_container.float_x()) { // float width
				cur_x = _container.clamp_width(0);
			}
		}

		set_content_size({
			cur_x,
			_container.float_y() ?
				_container.clamp_height(inner_size.y()):
				_container.content[1]
		});
		delete_lock_state();
		unmark(kLayout_Typesetting);

		return inner_size;
	}

	void Box::delete_lock_state() {
		// Delete all lock state
		_container.state_x &= ~kFixedByLock_FloatState;
		_container.state_y &= ~kFixedByLock_FloatState;
	}

	void Box::layout_text(TextLines *lines, TextOptions *opts) {
		auto white_space = opts->white_space_value();
		//auto word_break = opts->word_break_value();
		bool is_auto_wrap = true;
		auto limitX = lines->limit_range().end.x();
		auto origin = lines->pre_width();

		if (white_space == WhiteSpace::NoWrap ||
				white_space == WhiteSpace::Pre
		) { // 不使用自动wrap（不使用自动换行）
			is_auto_wrap = false;
		}

		lines->finish_text_blob_pre(); // finish previous blob

		if (is_auto_wrap) {
			if (origin + _layout_size.x() > limitX) {
				lines->push(opts);
				origin = 0;
			}
		}
		set_layout_offset({origin, 0});
		lines->add_view(this, origin + _layout_size.x());
	}

	Vec2 set_layout_offset_free(Align align, Vec2 hostSize, Vec2 layout_size) {
		Vec2 offset;
		switch(align) {
			case Align::Normal:
			case Align::LeftTop: // left top
				break;
			case Align::CenterTop: // center top
				offset = Vec2((hostSize.x() - layout_size.x()) * .5, 0);
				break;
			case Align::RightTop: // right top
				offset = Vec2(hostSize.x() - layout_size.x(), 0);
				break;
			case Align::LeftMiddle: // left Middle
				offset = Vec2(0, (hostSize.y() - layout_size.y()) * .5);
				break;
			case Align::CenterMiddle: // center Middle
				offset = Vec2(
					(hostSize.x() - layout_size.x()) * .5,
					(hostSize.y() - layout_size.y()) * .5);
				break;
			case Align::RightMiddle: // right Middle
				offset = Vec2(
					(hostSize.x() - layout_size.x()),
					(hostSize.y() - layout_size.y()) * .5);
				break;
			case Align::LeftBottom: // left bottom
				offset = Vec2(0, (hostSize.y() - layout_size.y()));
				break;
			case Align::CenterBottom: // center bottom
				offset = Vec2(
					(hostSize.x() - layout_size.x()) * .5,
					(hostSize.y() - layout_size.y()));
				break;
			case Align::RightBottom: // right bottom
				offset = Vec2(
					(hostSize.x() - layout_size.x()),
					(hostSize.y() - layout_size.y()));
				break;
		}
		return offset;
	}

	void Box::set_layout_offset_free(Vec2 size) {
		auto off = qk::set_layout_offset_free(_align, size, _layout_size);
		set_layout_offset(off);
	}

}
