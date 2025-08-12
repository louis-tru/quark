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

	MatrixBase::MatrixBase()
		: _translate(0), _scale(1), _skew(0), _rotate_z(0), _origin(0)
	{
	}

	/**
		* Set the matrix `translate` properties of the view object
		*
		* @method set_translate(val)
		*/
	void MatrixBase::set_translate(Vec2 val, bool isRt) {
		if (_translate != val) {
			_translate = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* Set the matrix `scale` properties of the view object
		*
		* @method set_scale(val)
		*/
	void MatrixBase::set_scale(Vec2 val, bool isRt) {
		if (_scale != val) {
			_scale = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* Set the matrix `skew` properties of the view object
		*
		* @method set_skew(val)
		*/
	void MatrixBase::set_skew(Vec2 val, bool isRt) {
		if (_skew != val) {
			_skew = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* Set the z-axis  matrix `rotate` properties of the view object
		*
		* @method set_rotate_z(val)
		*/
	void MatrixBase::set_rotate_z(float val, bool isRt) {
		val *= Qk_PI_RATIO_180;
		if (_rotate_z != val) {
			_rotate_z = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	void MatrixBase::set_origin(Vec2 origin, bool isRt) {
		if (_origin != val) {
			_origin = val;
			// 	mark_layout(kTransform_Origin, isRt);
		}
	}

	// void MatrixBase::set_origin_y(BoxOrigin val, bool isRt) {
		// if (_origin_y != val) {
		// 	_origin_y = val;
		// 	mark_layout(kTransform_Origin, isRt);
		// }
	// }

	/**
		* 
		* Returns x-axis matrix displacement for the view
		*
		* @method x()
		*/
	float MatrixBase::x() const { return _translate[0]; }

	/**
		* 
		* Returns y-axis matrix displacement for the view
		*
		* @method y()
		*/
	float MatrixBase::y() const { return _translate[1]; }

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @method scale_x()
		*/
	float MatrixBase::scale_x() const { return _scale[0]; }

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @method scale_y()
		*/
	float MatrixBase::scale_y() const { return _scale[1]; }

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @method skew_x()
		*/
	float MatrixBase::skew_x() const { return _skew[0]; }

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @method skew_y()
		*/
	float MatrixBase::skew_y() const { return _skew[1]; }

	/**
		* 
		* Setting x-axis matrix displacement for the view
		*
		* @method set_x(val)
		*/
	void MatrixBase::set_x(float val, bool isRt) {
		if (_translate[0] != val) {
			_translate[0] = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* 
		* Setting y-axis matrix displacement for the view
		*
		* @method set_y(val)
		*/
	void MatrixBase::set_y(float val, bool isRt) {
		if (_translate[1] != val) {
			_translate[1] = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @method set_scale_x(val)
		*/
	void MatrixBase::set_scale_x(float val, bool isRt) {
		if (_scale[0] != val) {
			_scale[0] = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @method set_scale_y(val)
		*/
	void MatrixBase::set_scale_y(float val, bool isRt) {
		if (_scale[1] != val) {
			_scale[1] = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @method set_skew_x(val)
		*/
	void MatrixBase::set_skew_x(float val, bool isRt) {
		if (_skew[0] != val) {
			_skew[0] = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @method set_skew_y(val)
		*/
	void MatrixBase::set_skew_y(float val, bool isRt) {
		if (_skew[1] != val) {
			_skew[1] = val;
			mark(kTransform, isRt); // mark transform
		}
	}

	// ----------------------------------------------------------------------------------

	Matrix::Matrix()
		: Box(), MatrixBase()
		, _box_origin_x{0, BoxOriginKind::Auto}
		, _box_origin_y{0, BoxOriginKind::Auto}
	{
	}

	void Matrix::set_box_origin_x(BoxOrigin val, bool isRt) {
		if (_box_origin_x != val) {
			_box_origin_x = val;
			mark_layout(kTransform_Origin, isRt);
		}
	}

	void Matrix::set_box_origin_y(BoxOrigin val, bool isRt) {
		if (_box_origin_y != val) {
			_box_origin_y = val;
			mark_layout(kTransform_Origin, isRt);
		}
	}

	ArrayOrigin Matrix::box_origin() const {
		return ArrayOrigin{_origin_x, _origin_y};
	}

	void Matrix::set_box_origin(ArrayOrigin val, bool isRt) {
		switch (val.length()) {
			case 1:
				set_box_origin_x(val[0], isRt);
				set_box_origin_y(val[0], isRt);
				break;
			case 2:
				set_box_origin_x(val[0], isRt);
				set_box_origin_y(val[1], isRt);
				break;
			default: break;
		}
	}

	Vec2 Matrix::center() {
		auto size = client_size();
		Vec2 point(
			size.x() * 0.5 - _origin_value.x(),
			size.y() * 0.5 - _origin_value.y()
		);
		return point;
	}

	void Matrix::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kTransform) { // update transform matrix
			_CheckParent();
			unmark(kTransform | kVisible_Region); // unmark

			auto v = layout_offset() + _parent->layout_offset_inside()
				+ Vec2(margin_left(), margin_top()) + _origin_value + _translate;
			_matrix = Mat(mat).set_translate(_parent->position()) * Mat(v, _scale, -_rotate_z, _skew);
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
		if (mark & (kTransform_Origin | kLayout_Typesetting) ) {
			solve_origin_value(); // check transform_origin change
		}
	}

	void Matrix::solve_origin_value() {
		auto old = _origin_value;
		auto _client_size = client_size();

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

		unmark(kTransform_Origin);

		if (old != _origin_value) {
			mark(kTransform, true);
		}
	}

	Matrix* Matrix::asMatrix() {
		return this;
	}

	ViewType Matrix::viewType() const {
		return kMatrix_ViewType;
	}

}
