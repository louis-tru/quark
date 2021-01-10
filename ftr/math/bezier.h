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

#ifndef __ftr__bezier__
#define __ftr__bezier__

#include "ftr/mathe.h"
#include "ftr/util/buffer.h"

/**
 * @ns ftr
 */

FX_NS(ftr)

/**
 * @class QuadraticBezier # 二次贝塞尔曲线
 */
class FX_EXPORT QuadraticBezier {
 public:
	
	/**
	 * @constructor
	 */
	QuadraticBezier(Vec2 p0, Vec2 p1, Vec2 p2);
	
	/**
	 * @func sample_curve_x
	 */
	float sample_curve_x(float t) const;
	
	/**
	 * @func sample_curve_y
	 */
	float sample_curve_y(float t) const;
	
	/**
	 * @func compute_bezier_points
	 */
	void sample_curve_points(uint sample_count, float* out) const;
	
	/**
	 * @func sample_curve_points
	 */
	ArrayBuffer<Vec2> sample_curve_points(uint sample_count) const;
	
 private:
	
	float p0x, p0y;
	float p1x, p1y;
	float p2x, p2y;
};

/**
 * @class CubicBezier # 三次贝塞尔曲线
 */
class FX_EXPORT CubicBezier {
 public:
	
	/**
	 * @constructor
	 */
	inline CubicBezier() { }
	
	/**
	 * @constructor
	 */
	CubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3);
		
	/**
	 * @func sample_curve_x
	 */
	inline float sample_curve_x(float t) const {
		// `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
		return ((ax * t + bx) * t + cx) * t + p0x;
	}
	
	/**
	 * @func sample_curve_y
	 */
	inline float sample_curve_y(float t) const {
		return ((ay * t + by) * t + cy) * t + p0y;
	}
	
	/**
	 * @func compute_bezier_points
	 */
	void sample_curve_points(uint sample_count, float* out) const;
	
	/**
	 * @func sample_curve_points
	 */
	ArrayBuffer<Vec2> sample_curve_points(uint sample_count) const;
	
 protected:
	
	float ax, bx, cx;
	float ay, by, cy;
	float p0x, p0y;
};

/**
 * @class FixedCubicBezier
 * @bases CubicBezier
 */
class FX_EXPORT FixedCubicBezier: public CubicBezier {
 public:
	
	/**
	 * @constructor
	 */
	FixedCubicBezier();
	
	/**
	 * Calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1).
	 * @constructor
	 */
	FixedCubicBezier(Vec2 p1, Vec2 p2);
	
	/**
	 * @constructor
	 */
	inline FixedCubicBezier(float p1x, float p1y, float p2x, float p2y)
		: FixedCubicBezier(Vec2(p1x, p1y), Vec2(p2x, p2y))
	{ }
	
	/**
	 * @func point1
	 */
	inline Vec2 point1() const { return m_p1; }
	
	/**
	 * @func point2
	 */
	inline Vec2 point2() const { return m_p2; }
	
	/**
	 * @func sample_curve_derivative_x
	 */
	inline float sample_curve_derivative_x(float t) const {
		return (3.0 * ax * t + 2.0 * bx) * t + cx;
	}
	
	/**
	 * @func solve_curve_x # Given an x value, find a parametric value it came from.
	 */
	float solve_curve_x(float x, float epsilon) const;
	
	/**
	 * @func solve
	 */
	inline float solve(float x, float epsilon) const {
		return (this->*m_solve)(x, epsilon);
	}
	
 private:
	
	typedef float (FixedCubicBezier::*Solve)(float x, float epsilon) const;
	
	Solve m_solve;
	Vec2 m_p1;
	Vec2 m_p2;
	
	FX_DEFINE_INLINE_CLASS(Inl);
};

typedef FixedCubicBezier Curve;
typedef const Curve cCurve;

FX_EXPORT extern const FixedCubicBezier LINEAR;
FX_EXPORT extern const FixedCubicBezier EASE;
FX_EXPORT extern const FixedCubicBezier EASE_IN;
FX_EXPORT extern const FixedCubicBezier EASE_OUT;
FX_EXPORT extern const FixedCubicBezier EASE_IN_OUT;

FX_END
#endif
