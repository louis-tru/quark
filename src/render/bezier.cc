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

#include "./bezier.h"
#include <math.h>

namespace qk {

	// De Casteljau's algorithm

	QuadraticBezier::QuadraticBezier(Vec2 p0, Vec2 p1, Vec2 p2)
		: _p0(p0), _p1(p1), _p2(p2)
	{}

	float QuadraticBezier::sample_curve_x(float t) const {
		float t2 = 1.0 - t;
		return t2 * t2 * _p0.x() + 2 * t * t2 * _p1.x() + t * t * _p2.x();
	}

	float QuadraticBezier::sample_curve_y(float t) const {
		float t2 = 1.0 - t;
		return t2 * t2 * _p0.y() + 2 * t * t2 * _p1.y() + t * t * _p2.y();
	}

	void QuadraticBezier::sample_curve_points(uint32_t sample_count, float* out, int stride) const {
		// |0|1| = sample_count = 3
		sample_count--;
		reinterpret_cast<Vec2*>(out)[0] = _p0; out += 2;
		float dt = 1.0 / sample_count;
		for( uint32_t i = 1; i < sample_count; i++) {
			float t  = i * dt;
			float t2 = 1.0 - t;
			float t3 = t2 * t2;    // t'^2
			float t4 = t * t2 * 2; // 2tt'
			float t5 = t * t;      // t^2
			out[0]  = t3 * _p0.x() + t4 * _p1.x() + t5 * _p2.x(); // x
			out[1]  = t3 * _p0.y() + t4 * _p1.y() + t5 * _p2.y(); // y
			out += stride;
		}
		reinterpret_cast<Vec2*>(out)[0] = _p2;
	}

	Array<Vec2> QuadraticBezier::sample_curve_points(uint32_t sample_count) const {
		Array<Vec2> rev(sample_count);
		sample_curve_points(sample_count, reinterpret_cast<float*>(*rev));
		return rev;
	}

	// ------ CubicBezier ------

	CubicBezier::CubicBezier(): CubicBezier(Vec2(0, 0), Vec2(0, 0), Vec2(1, 1), Vec2(1, 1)) {
	}

	CubicBezier::CubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3)
		: _p0(p0), _p1(p1), _p2(p2), _p3(p3)
	{
		cx = 3.0 * (p1.x() - p0.x());
		bx = 3.0 * (p2.x() - p1.x()) - cx;
		ax = p3.x() - p0.x() - cx - bx;
		cy = 3.0 * (p1.y() - p0.y());
		by = 3.0 * (p2.y() - p1.y()) - cy;
		ay = p3.y() - p0.y() - cy - by;
	}

	void CubicBezier::sample_curve_points(uint32_t sample_count, float* out, int stride) const {
		// |0|1| = sample_count = 3
		sample_count--;
		reinterpret_cast<Vec2*>(out)[0] = _p0; out += 2;
		float dt = 1.0 / sample_count;
		for( uint32_t i = 1; i < sample_count; i++) {
			float t = i * dt;
			out[0] = sample_curve_x(t);
			out[1] = sample_curve_y(t);
			out += stride;
		}
		reinterpret_cast<Vec2*>(out)[0] = _p3;
	}

	Array<Vec2> CubicBezier::sample_curve_points(uint32_t sample_count) const {
		Array<Vec2> rev(sample_count);
		sample_curve_points(sample_count, reinterpret_cast<float*>(*rev));
		return rev;
	}

	// ------ FixedCubicBezier ------

	FixedCubicBezier::FixedCubicBezier(): CubicBezier(), _isLINEAR(true) {
	}

	FixedCubicBezier::FixedCubicBezier(Vec2 p1, Vec2 p2): CubicBezier(Vec2(), p1, p2, Vec2(1)) {
		_isLINEAR = (p1 == Vec2() && p2 == Vec2(1));
	}

	float FixedCubicBezier::solve_t(float x, float epsilon) const {
		if (_isLINEAR) return x;

		float t0,t1,t2,x2,d2;
		int i;
		// First try a few iterations of Newton's method -- normally very fast.
		for (t2 = x, i = 0; i < 8; i++) {
			x2 = sample_curve_x(t2) - x;
			if (fabsf (x2) < epsilon)
				return t2;
			d2 = sample_curve_derivative_x(t2);
			if (fabsf(d2) < 1e-6)
				break;
			t2 = t2 - x2 / d2;
		}
		// Fall back to the bisection method for reliability.
		t0 = 0.0;
		t1 = 1.0;
		t2 = x;
		if (t2 < t0)
			return t0;
		if (t2 > t1)
			return t1;
		while (t0 < t1) {
			x2 = sample_curve_x(t2);
			if (fabsf(x2 - x) < epsilon)
				return t2;
			if (x > x2)
				t0 = t2;
			else
				t1 = t2;
			t2 = (t1 - t0) * .5 + t0;
		}
		// Failure.
		return t2;
	}

	float FixedCubicBezier::solve_y(float x, float epsilon) const {
		return _isLINEAR ? x: sample_curve_y(solve_t(x, epsilon));
	}

	cCurve LINEAR;
	cCurve EASE(FixedCubicBezier({0.25, 0.1}, {0.25, 1}));
	cCurve EASE_IN(FixedCubicBezier({0.42, 0}, {1, 1}));
	cCurve EASE_OUT(FixedCubicBezier({0, 0}, {0.58, 1}));
	cCurve EASE_IN_OUT(FixedCubicBezier({0.42, 0}, {0.58, 1}));
}
