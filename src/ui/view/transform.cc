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

#include "./transform.h"

namespace qk {

	Transform::Transform()
		: _translate(0), _scale(1), _skew(0), _rotate_z(0)
		, _origin_x{0, BoxOriginKind::kPixel}, _origin_y{0, BoxOriginKind::kPixel}
	{
	}

	/**
		* Set the matrix `translate` properties of the view object
		*
		* @method set_translate(val)
		*/
	void Transform::set_translate(Vec2 val) {
		if (_translate != val) {
			_translate = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* Set the matrix `scale` properties of the view object
		*
		* @method set_scale(val)
		*/
	void Transform::set_scale(Vec2 val) {
		if (_scale != val) {
			_scale = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* Set the matrix `skew` properties of the view object
		*
		* @method set_skew(val)
		*/
	void Transform::set_skew(Vec2 val) {
		if (_skew != val) {
			_skew = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* Set the z-axis  matrix `rotate` properties of the view object
		*
		* @method set_rotate_z(val)
		*/
	void Transform::set_rotate_z(float val) {
		val *= Qk_PI_RATIO_180;
		if (_rotate_z != val) {
			_rotate_z = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix displacement for the view
		*
		* @method x()
		*/
	float Transform::x() const { return _translate[0]; }

	/**
		* 
		* Returns y-axis matrix displacement for the view
		*
		* @method y()
		*/
	float Transform::y() const { return _translate[1]; }

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @method scale_x()
		*/
	float Transform::scale_x() const { return _scale[0]; }

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @method scale_y()
		*/
	float Transform::scale_y() const { return _scale[1]; }

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @method skew_x()
		*/
	float Transform::skew_x() const { return _skew[0]; }

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @method skew_y()
		*/
	float Transform::skew_y() const { return _skew[1]; }

	/**
		* 
		* Setting x-axis matrix displacement for the view
		*
		* @method set_x(val)
		*/
	void Transform::set_x(float val) {
		if (_translate[0] != val) {
			_translate[0] = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Setting y-axis matrix displacement for the view
		*
		* @method set_y(val)
		*/
	void Transform::set_y(float val) {
		if (_translate[1] != val) {
			_translate[1] = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix scaling for the view
		*
		* @method set_scale_x(val)
		*/
	void Transform::set_scale_x(float val) {
		if (_scale[0] != val) {
			_scale[0] = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix scaling for the view
		*
		* @method set_scale_y(val)
		*/
	void Transform::set_scale_y(float val) {
		if (_scale[1] != val) {
			_scale[1] = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns x-axis matrix skew for the view
		*
		* @method set_skew_x(val)
		*/
	void Transform::set_skew_x(float val) {
		if (_skew[0] != val) {
			_skew[0] = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	/**
		* 
		* Returns y-axis matrix skew for the view
		*
		* @method set_skew_y(val)
		*/
	void Transform::set_skew_y(float val) {
		if (_skew[1] != val) {
			_skew[1] = val;
			async_mark(kRecursive_Transform); // mark transform
		}
	}

	void Transform::set_origin_x(BoxOrigin val) {
		if (_origin_x != val) {
			_origin_x = val;
			async_mark_layout(kTransform_Origin);
		}
	}

	void Transform::set_origin_y(BoxOrigin val) {
		if (_origin_y != val) {
			_origin_y = val;
			async_mark_layout(kTransform_Origin);
		}
	}

	// ----------------------------------------------------------------------------------

	Vec2 Transform::center() {
		auto size = client_size();
		Vec2 point(
			size.x() * 0.5 - _origin_value.x(),
			size.y() * 0.5 - _origin_value.y()
		);
		return point;
	}

	void Transform::solve_marks(const Mat &mat, uint32_t mark) {
		if (mark & kRecursive_Transform) { // update transform matrix
			unmark(kRecursive_Transform | kRecursive_Visible_Region); // unmark
			auto v = layout_offset() + parent_Rt()->layout_offset_inside()
				+ Vec2(margin_left(), margin_top()) + _origin_value;
			_matrix = Mat(mat).set_translate(parent_Rt()->position()) * Mat(v, _scale, -_rotate_z, _skew);
			_position = Vec2(_matrix[2],_matrix[5]);
			_visible_region = solve_visible_region(_matrix);
			_matrix.set_translate(Vec2(0)); // clear translate, use position value
		} else if (mark & kRecursive_Visible_Region) {
			unmark(kRecursive_Visible_Region); // unmark
			_visible_region = solve_visible_region(Mat(mat).set_translate(_position));
		}
	}

	void Transform::solve_rect_vertex(const Mat &mat, Vec2 vertex[4]) {
		Vec2 origin(-_origin_value.x(), -_origin_value.y());
		Vec2 end = origin + client_size();
		vertex[0] = mat * origin;
		vertex[1] = mat * Vec2(end.x(), origin.y());
		vertex[2] = mat * end;
		vertex[3] = mat * Vec2(origin.x(), end.y());
	}

	Vec2 Transform::layout_offset_inside() {
		return Box::layout_offset_inside() - _origin_value;
	}

	bool Transform::layout_forward(uint32_t mark) {
		auto ok = Box::layout_forward(mark);
		if (ok) {
			if (mark & kTransform_Origin) {
				solve_origin_value(); // check transform_origin change
			}
		}
		return ok;
	}

	bool Transform::layout_reverse(uint32_t mark) {
		auto ok = Box::layout_reverse(mark);
		if (ok) {
			if (mark & kLayout_Typesetting) {
				solve_origin_value(); // check transform_origin change
			}
		}
		return ok;
	}

	void Transform::solve_origin_value() {
		auto old = _origin_value;
		auto _client_size = client_size();

		switch (_origin_x.kind) {
			default:
			case BoxOriginKind::kAuto:  _origin_value.set_x(_client_size.x() * 0.5); break; // center
			case BoxOriginKind::kPixel: _origin_value.set_x(_origin_x.value); break;
			case BoxOriginKind::kRatio: _origin_value.set_x(_client_size.x() * _origin_x.value); break;
		}
		switch (_origin_y.kind) {
			default:
			case BoxOriginKind::kAuto:  _origin_value.set_y(_client_size.y() * 0.5); break; // center
			case BoxOriginKind::kPixel: _origin_value.set_y(_origin_y.value); break;
			case BoxOriginKind::kRatio: _origin_value.set_y(_client_size.y() * _origin_y.value); break;
		}

		unmark(kTransform_Origin);

		if (old != _origin_value) {
			mark(kRecursive_Transform);
		}
	}

	Transform* Transform::asTransform() {
		return this;
	}

	ViewType Transform::viewType() const {
		return kTransform_ViewType;
	}

}
