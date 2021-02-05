/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
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

namespace ftr {

	// 德卡斯特里奥

	/**
	* @constructor
	*/
	QuadraticBezier::QuadraticBezier(Vec2 p0, Vec2 p1, Vec2 p2)
		: p0x(p0.x()), p0y(p0.y()), p1x(p1.x()), p1y(p1.y()), p2x(p2.x()), p2y(p2.y()) { }

	/**
	* @func sample_curve_x
	*/
	float QuadraticBezier::sample_curve_x(float t) const {
		float t2 = 1 - t;
		return t2 * t2 * p0x + 2 * t * t2 * p1x + t * t * p2x;
	}

	/**
	* @func sample_curve_y
	*/
	float QuadraticBezier::sample_curve_y(float t) const {
		float t2 = 1 - t;
		return t2 * t2 * p0y + 2 * t * t2 * p1y + t * t * p2y;
	}

	/**
	* @func compute_bezier_points
	*/
	void QuadraticBezier::sample_curve_points(uint sample_count, float* out) const {
		float dt = 1.0 / ( sample_count - 1 );
		for( uint i = 0; i < sample_count; i++) {
			float t = i * dt;
			float t2 = 1 - t;
			float t3 = t2 * t2;
			float t4 = 2 * t * t2;
			float t5 = t * t;
			out[i * 2]      = t3 * p0x + t4 * p1x + t5 * p2x;
			out[i * 2 + 1]  = t3 * p0y + t4 * p1y + t5 * p2y;
		}
	}

	/**
	* @func sample_curve_points
	*/
	ArrayBuffer<Vec2> QuadraticBezier::sample_curve_points(uint sample_count) const {
		ArrayBuffer<Vec2> rev(sample_count);
		sample_curve_points(sample_count, reinterpret_cast<float*>(*rev));
		return rev;
	}

	CubicBezier::CubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3): p0x(p0.x()), p0y(p0.y()) {
		cx = 3.0 * (p1.x() - p0x);
		bx = 3.0 * (p2.x() - p1.x()) - cx;
		ax = p3.x() - p0x - cx - bx;
		//
		cy = 3.0 * (p1.y() - p0y);
		by = 3.0 * (p2.y() - p1.y()) - cy;
		ay = p3.y() - p0y - cy - by;
	}

	/**
	* @func compute_bezier_points
	*/
	void CubicBezier::sample_curve_points(uint sample_count, float* out) const {
		float dt = 1.0 / ( sample_count - 1 );
		for( uint i = 0; i < sample_count; i++) {
			out[i * 2] = sample_curve_x(i * dt);
			out[i * 2 + 1] = sample_curve_y(i * dt);
		}
	}

	/**
	* @func sample_curve_points
	*/
	ArrayBuffer<Vec2> CubicBezier::sample_curve_points(uint sample_count) const {
		ArrayBuffer<Vec2> rev(sample_count);
		sample_curve_points(sample_count, reinterpret_cast<float*>(*rev));
		return rev;
	}

	/**
	* @class FixedCubicBezier::Inl
	*/
	class FixedCubicBezier::Inl: public FixedCubicBezier {
	public:
		
		/**
		* @func defalut_solve
		*/
		float defalut_solve(float x, float epsilon) const {
			return sample_curve_y(solve_curve_x(x, epsilon));
		}
		
		/**
		* @func solve_linear
		*/
		float solve_linear(float x, float epsilon) const {
			return x;
		}
	};

	/**
	* @constructor
	*/
	FixedCubicBezier::FixedCubicBezier()
	: CubicBezier(Vec2(0, 0), Vec2(0, 0), Vec2(1, 1), Vec2(1, 1))
	, _solve((Solve)&Inl::solve_linear)
	, _p1(Vec2(0, 0)), _p2(Vec2(1, 1))
	{ }

	/**
	* Calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1).
	* @constructor
	*/
	FixedCubicBezier::FixedCubicBezier(Vec2 p1, Vec2 p2)
	: CubicBezier(Vec2(0, 0), p1, p2, Vec2(1, 1))
	, _solve((Solve)&Inl::defalut_solve)
	, _p1(p1), _p2(p2) {
		if ( p1.x() == 0 && p1.y() == 0 && p2.x() == 1 && p2.y() == 1 ) {
			_solve = (Solve)&Inl::solve_linear;
		}
	}

	/**
	* @func defalut_solve_curve_x # Given an x value, find a parametric value it came from.
	* 通过x值迭代逼近查找t值
	*/
	float FixedCubicBezier::solve_curve_x(float x, float epsilon) const {
		float t0;
		float t1;
		float t2;
		float x2;
		float d2;
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

	const FixedCubicBezier LINEAR;
	const FixedCubicBezier EASE(0.25, 0.1, 0.25, 1);
	const FixedCubicBezier EASE_IN(0.42, 0, 1, 1);
	const FixedCubicBezier EASE_OUT(0, 0, 0.58, 1);
	const FixedCubicBezier EASE_IN_OUT(0.42, 0, 0.58, 1);

}
