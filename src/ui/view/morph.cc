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

#include "./morph.h"

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue

namespace qk {

	MorphView::MorphView(View* host)
		: _origin_value(0)
		, _translate(0), _scale(1), _skew(0), _rotate_z(0)
		, _origin_x{0, BoxOriginKind::Auto}
		, _origin_y{0, BoxOriginKind::Auto}
		, _host(host)
	{
	}

	/**
		* Set the matrix `translate` properties of the view object
		*/
	void MorphView::set_translate(Vec2 val) {
		_host->mark_style_flag(kX_CssProp);
		_host->mark_style_flag(kY_CssProp);
		if (_translate != val) {
			_host->mark(View::kTransform); // mark transform
			_translate = val;
		}
	}

	void MorphView::set_translate_rt(Vec2 val) {
		if (_translate != val) {
			_translate = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Set the matrix `scale` properties of the view object
		*/
	void MorphView::set_scale(Vec2 val) {
		_host->mark_style_flag(kSCALE_X_CssProp);
		_host->mark_style_flag(kSCALE_Y_CssProp);
		if (_scale != val) {
			_host->mark(View::kTransform); // mark transform
			_scale = val;
		}
	}

	void MorphView::set_scale_rt(Vec2 val) {
		if (_scale != val) {
			_scale = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Set the matrix `skew` properties of the view object
		*/
	void MorphView::set_skew(Vec2 val) {
		_host->mark_style_flag(kSKEW_X_CssProp);
		_host->mark_style_flag(kSKEW_Y_CssProp);
		if (_skew != val) {
			_host->mark(View::kTransform); // mark transform
			_skew = val;
		}
	}

	void MorphView::set_skew_rt(Vec2 val) {
		if (_skew != val) {
			_skew = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Set the z-axis  matrix `rotate` properties of the view object
		*/
	void MorphView::set_rotate_z(float val) {
		_host->mark_style_flag(kROTATE_Z_CssProp);
		val *= Qk_PI_RATIO_180;
		if (_rotate_z != val) {
			_host->mark(View::kTransform); // mark transform
			_rotate_z = val;
		}
	}

	void MorphView::set_rotate_z_rt(float val) {
		val *= Qk_PI_RATIO_180;
		if (_rotate_z != val) {
			_rotate_z = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
	 * Returns the matrix origin of the view object
	*/
	void MorphView::set_origin_x(BoxOrigin val) {
		_host->mark_style_flag(kORIGIN_X_CssProp);
		if (_origin_x != val) {
			_host->mark(View::kTransform);
			_origin_x = val;
		}
	}

	void MorphView::set_origin_x_rt(BoxOrigin val) {
		if (_origin_x != val) {
			_origin_x = val;
			_host->mark<true>(View::kTransform);
		}
	}

	void MorphView::set_origin_y(BoxOrigin val) {
		_host->mark_style_flag(kORIGIN_Y_CssProp);
		if (_origin_y != val) {
			_host->mark(View::kTransform);
			_origin_y = val;
		}
	}

	void MorphView::set_origin_y_rt(BoxOrigin val) {
		if (_origin_y != val) {
			_origin_y = val;
			_host->mark<true>(View::kTransform);
		}
	}

	ArrayOrigin MorphView::origin() const {
		return ArrayOrigin{_origin_x, _origin_y};
	}

	void MorphView::set_origin(ArrayOrigin val) {
		switch (val.length()) {
			case 1:
				set_origin_x(val[0]);
				set_origin_y(val[0]);
				break;
			case 2:
				set_origin_x(val[0]);
				set_origin_y(val[1]);
				break;
			default: break;
		}
	}

	void MorphView::set_origin_rt(ArrayOrigin val) {
		switch (val.length()) {
			case 1:
				set_origin_x_rt(val[0]);
				set_origin_y_rt(val[0]);
				break;
			case 2:
				set_origin_x_rt(val[0]);
				set_origin_y_rt(val[1]);
				break;
			default: break;
		}
	}

	/**
		* Returns x-axis matrix displacement for the view
		*/
	float MorphView::x() const { return _translate[0]; }

	/**
		* Returns y-axis matrix displacement for the view
		*/
	float MorphView::y() const { return _translate[1]; }

	/**
		* Returns x-axis matrix scaling for the view
		*/
	float MorphView::scale_x() const { return _scale[0]; }

	/**
		* Returns y-axis matrix scaling for the view
		*/
	float MorphView::scale_y() const { return _scale[1]; }

	/**
		* Returns x-axis matrix skew for the view
		*/
	float MorphView::skew_x() const { return _skew[0]; }

	/**
		* Returns y-axis matrix skew for the view
		*/
	float MorphView::skew_y() const { return _skew[1]; }

	/**
		* Setting x-axis matrix displacement for the view
		*/
	void MorphView::set_x(float val) {
		_host->mark_style_flag(kX_CssProp);
		if (_translate[0] != val) {
			_host->mark(View::kTransform); // mark transform
			_translate[0] = val;
		}
	}

	void MorphView::set_x_rt(float val) {
		if (_translate[0] != val) {
			_translate[0] = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Setting y-axis matrix displacement for the view
		*/
	void MorphView::set_y(float val) {
		_host->mark_style_flag(kY_CssProp);
		if (_translate[1] != val) {
			_host->mark(View::kTransform); // mark transform
			_translate[1] = val;
		}
	}

	void MorphView::set_y_rt(float val) {
		if (_translate[1] != val) {
			_translate[1] = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Returns x-axis matrix scaling for the view
		*/
	void MorphView::set_scale_x(float val) {
		_host->mark_style_flag(kSCALE_X_CssProp);
		if (_scale[0] != val) {
			_host->mark(View::kTransform); // mark transform
			_scale[0] = val;
		}
	}

	void MorphView::set_scale_x_rt(float val) {
		if (_scale[0] != val) {
			_scale[0] = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Returns y-axis matrix scaling for the view
		*/
	void MorphView::set_scale_y(float val) {
		_host->mark_style_flag(kSCALE_Y_CssProp);
		if (_scale[1] != val) {
			_host->mark(View::kTransform); // mark transform
			_scale[1] = val;
		}
	}

		void MorphView::set_scale_y_rt(float val) {
		if (_scale[1] != val) {
			_scale[1] = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Returns x-axis matrix skew for the view
		*/
	void MorphView::set_skew_x(float val) {
		_host->mark_style_flag(kSKEW_X_CssProp);
		if (_skew[0] != val) {
			_host->mark(View::kTransform); // mark transform
			_skew[0] = val;
		}
	}

	void MorphView::set_skew_x_rt(float val) {
		if (_skew[0] != val) {
			_skew[0] = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	/**
		* Returns y-axis matrix skew for the view
		*/
	void MorphView::set_skew_y(float val) {
		_host->mark_style_flag(kSKEW_Y_CssProp);
		if (_skew[1] != val) {
			_host->mark(View::kTransform); // mark transform
			_skew[1] = val;
		}
	}

	void MorphView::set_skew_y_rt(float val) {
		if (_skew[1] != val) {
			_skew[1] = val;
			_host->mark<true>(View::kTransform); // mark transform
		}
	}

	// solve the origin value by client size
	// The origin value is the final value by computing the origin.
	void MorphView::solve_origin_value() {
		auto _client_size = _host->client_size();
		switch (_origin_x.kind) {
			default:
			case BoxOriginKind::Auto:  _origin_value.set_x(_client_size.x() * 0.5); break; // center
			case BoxOriginKind::Value: _origin_value.set_x(_origin_x.value); break;
			case BoxOriginKind::Ratio: _origin_value.set_x(_client_size.x() * _origin_x.value); break;
		}
		switch (_origin_y.kind) {
			default:
			case BoxOriginKind::Auto:  _origin_value.set_y(_client_size.y() * 0.5); break; // center
			case BoxOriginKind::Value: _origin_value.set_y(_origin_y.value); break;
			case BoxOriginKind::Ratio: _origin_value.set_y(_client_size.y() * _origin_y.value); break;
		}
	}

	void MorphView::solve_box_Bounds(const Mat &mat, Vec2 outBounds[4]) {
		Vec2 origin(-_origin_value.x(), -_origin_value.y());
		Vec2 end = origin + _host->client_size();
		outBounds[0] = mat * origin;
		outBounds[1] = mat * Vec2(end.x(), origin.y());
		outBounds[2] = mat * end;
		outBounds[3] = mat * Vec2(origin.x(), end.y());
	}

	/////////////////////////////////////////////////////////////

	Morph::Morph()
		: Box(), MorphView(this)
	{
	}

	MorphView* Morph::asMorphView() {
		return this;
	}

	ViewType Morph::view_type() const {
		return kMorph_ViewType;
	}

	void Morph::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			solve_origin_value(); // check transform_origin change
			unmark(kTransform | kVisible_Region); // unmark
			auto v = parent->layout_offset_inside() + layout_offset()
				+ Vec2(margin_left(), margin_top()) + _translate + _origin_value;
			_matrix = Mat(mat).set_translate(parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]); // the origin world coords
			solve_visible_area(_matrix);
		} else if (mark & kVisible_Region) {
			unmark(kVisible_Region); // unmark
			solve_visible_area(_matrix.set_translate(_position));
		}
	}

	void Morph::solve_visible_area(const Mat &mat) {
		solve_box_Bounds(mat, _boxBounds);
		_visible_area = compute_visible_area(mat, _boxBounds);
	}

	Vec2 Morph::layout_offset_inside() {
		return Box::layout_offset_inside() - _origin_value;
	}

	Region Morph::client_region() {
		auto begin = _origin_value;
		return Region{-begin, _client_size - begin, _translate + _layout_offset + begin};
	}

	void Morph::layout_reverse(uint32_t mark) {
		Box::layout_reverse(mark);
		if (mark & kLayout_Typesetting) {
			// If the layout is typesetting,
			// maybe size has changed there may well be a change in the origin value,
			// mark the transform if the origin value has changed.
			// but now not check, just force mark the kTransform temporarily.
			this->mark<true>(kTransform); // mark transform
			// auto last = _origin_value;
			// solve_origin_value(client_size());
			// if (last != _origin_value) {
			// 	mark<true>(kTransform);
			// }
		}
	}
}
