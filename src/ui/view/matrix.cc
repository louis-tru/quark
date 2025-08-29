/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include "./matrix.h"

#define _Parent() auto _parent = this->parent()
#define _IfParent() _Parent(); if (_parent)
#define _CheckParent(defaultValue) _Parent(); if (!_parent) return defaultValue

namespace qk {

	MatrixView::MatrixView(View* host)
		: _translate(0), _scale(1), _skew(0)
		, _rotate_z(0), _origin_value(0)
		, _origin_x{0, BoxOriginKind::Auto}
		, _origin_y{0, BoxOriginKind::Auto}
		, _host(host)
	{}

	/**
		* Set the matrix `translate` properties of the view object
		*/
	void MatrixView::set_translate(Vec2 val, bool isRt) {
		if (_translate != val) {
			_translate = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Set the matrix `scale` properties of the view object
		*/
	void MatrixView::set_scale(Vec2 val, bool isRt) {
		if (_scale != val) {
			_scale = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Set the matrix `skew` properties of the view object
		*/
	void MatrixView::set_skew(Vec2 val, bool isRt) {
		if (_skew != val) {
			_skew = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Set the z-axis  matrix `rotate` properties of the view object
		*/
	void MatrixView::set_rotate_z(float val, bool isRt) {
		val *= Qk_PI_RATIO_180;
		if (_rotate_z != val) {
			_rotate_z = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
	 * Returns the matrix origin of the view object
	*/
	void MatrixView::set_origin_x(BoxOrigin val, bool isRt) {
		if (_origin_x != val) {
			_origin_x = val;
			_host->mark(View::kTransform, isRt);
		}
	}

	void MatrixView::set_origin_y(BoxOrigin val, bool isRt) {
		if (_origin_y != val) {
			_origin_y = val;
			_host->mark(View::kTransform, isRt);
		}
	}

	ArrayOrigin MatrixView::origin() const {
		return ArrayOrigin{_origin_x, _origin_y};
	}

	void MatrixView::set_origin(ArrayOrigin val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_origin_x(val[0], isRt);
				set_origin_y(val[0], isRt);
				break;
			case 2:
				set_origin_x(val[0], isRt);
				set_origin_y(val[1], isRt);
				break;
			default: break;
		}
	}

	/**
		* Returns x-axis matrix displacement for the view
		*/
	float MatrixView::x() const { return _translate[0]; }

	/**
		* Returns y-axis matrix displacement for the view
		*/
	float MatrixView::y() const { return _translate[1]; }

	/**
		* Returns x-axis matrix scaling for the view
		*/
	float MatrixView::scale_x() const { return _scale[0]; }

	/**
		* Returns y-axis matrix scaling for the view
		*/
	float MatrixView::scale_y() const { return _scale[1]; }

	/**
		* Returns x-axis matrix skew for the view
		*/
	float MatrixView::skew_x() const { return _skew[0]; }

	/**
		* Returns y-axis matrix skew for the view
		*/
	float MatrixView::skew_y() const { return _skew[1]; }

	/**
		* Setting x-axis matrix displacement for the view
		*/
	void MatrixView::set_x(float val, bool isRt) {
		if (_translate[0] != val) {
			_translate[0] = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Setting y-axis matrix displacement for the view
		*/
	void MatrixView::set_y(float val, bool isRt) {
		if (_translate[1] != val) {
			_translate[1] = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Returns x-axis matrix scaling for the view
		*/
	void MatrixView::set_scale_x(float val, bool isRt) {
		if (_scale[0] != val) {
			_scale[0] = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Returns y-axis matrix scaling for the view
		*/
	void MatrixView::set_scale_y(float val, bool isRt) {
		if (_scale[1] != val) {
			_scale[1] = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Returns x-axis matrix skew for the view
		*/
	void MatrixView::set_skew_x(float val, bool isRt) {
		if (_skew[0] != val) {
			_skew[0] = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	/**
		* Returns y-axis matrix skew for the view
		*/
	void MatrixView::set_skew_y(float val, bool isRt) {
		if (_skew[1] != val) {
			_skew[1] = val;
			_host->mark(View::kTransform, isRt); // mark transform
		}
	}

	// solve the origin value by client size
	// The origin value is the final value by computing the origin.
	void MatrixView::solve_origin_value() {
		auto client_size = _host->client_size();
		switch (_origin_x.kind) {
			default:
			case BoxOriginKind::Auto:  _origin_value.set_x(client_size.x() * 0.5); break; // center
			case BoxOriginKind::Value: _origin_value.set_x(_origin_x.value); break;
			case BoxOriginKind::Ratio: _origin_value.set_x(client_size.x() * _origin_x.value); break;
		}
		switch (_origin_y.kind) {
			default:
			case BoxOriginKind::Auto:  _origin_value.set_y(client_size.y() * 0.5); break; // center
			case BoxOriginKind::Value: _origin_value.set_y(_origin_y.value); break;
			case BoxOriginKind::Ratio: _origin_value.set_y(client_size.y() * _origin_y.value); break;
		}
	}

	/////////////////////////////////////////////////////////////

	Matrix::Matrix()
		: Box(), MatrixView(this)
	{
	}

	MatrixView* Matrix::asMatrixView() {
		return this;
	}

	ViewType Matrix::viewType() const {
		return kMatrix_ViewType;
	}

	void Matrix::solve_marks(const Mat &mat, View *parent, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			// _CheckParent();
			solve_origin_value(); // check transform_origin change
			unmark(kTransform | kVisible_Region); // unmark
			auto v = layout_offset() + parent->layout_offset_inside()
				+ Vec2(margin_left(), margin_top()) + _origin_value + _translate;
			_matrix = Mat(mat).set_translate(parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]);
			solve_visible_region(_matrix);
		}
		else if (mark & kVisible_Region) {
			unmark(kVisible_Region); // unmark
			solve_visible_region(_matrix.set_translate(_position));
		}
		//_matrix.set_translate(Vec2(0)); // clear translate, use position value
	}

	void Matrix::solve_rect_vertex(const Mat &mat, Vec2 vertex[4]) {
		Vec2 origin(-_origin_value.x(), -_origin_value.y());
		Vec2 end = origin + client_size();
		vertex[0] = mat * origin;
		vertex[1] = mat * Vec2(end.x(), origin.y());
		vertex[2] = mat * end;
		vertex[3] = mat * Vec2(origin.x(), end.y());
	}

	Vec2 Matrix::layout_offset_inside() {
		return Box::layout_offset_inside() - _origin_value;
	}

	void Matrix::layout_reverse(uint32_t mark) {
		Box::layout_reverse(mark);
		if (mark & kLayout_Typesetting) {
			// If the layout is typesetting,
			// maybe size has changed there may well be a change in the origin value,
			// mark the transform if the origin value has changed.
			// but now not check, just force mark the kTransform temporarily.
			this->mark(kTransform, true); // mark transform
			// auto last = _origin_value;
			// solve_origin_value(client_size());
			// if (last != _origin_value) {
			// 	mark(kTransform, true);
			// }
		}
	}
}
