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

#ifndef __quark__math__bezier__
#define __quark__math__bezier__

#include "./math.h"
#include "./util/array.h"

namespace qk {

	// Bezier curve formula of order N
	// F(t) = E(i=0,n) P_i(1-t)^(n-i)t^i, t<-|0,1|

	/**
	* @class QuadraticBezier 二次贝塞尔曲线
	*
	* F(t) = A(1-t)^2 + B2t(1-t) + Ct^2, t<-|0,1|
	*/
	class Qk_EXPORT QuadraticBezier {
	public:
		// define props
		Qk_DEFINE_PROP_GET(Vec2, p0);
		Qk_DEFINE_PROP_GET(Vec2, p1);
		Qk_DEFINE_PROP_GET(Vec2, p2);

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
		void sample_curve_points(uint32_t sample_count, float* out) const;
		
		/**
		* @func sample_curve_points
		*/
		Array<Vec2> sample_curve_points(uint32_t sample_count) const;
	};

	/**
	* @class CubicBezier 三次贝塞尔曲线
	*
	* F(t) = A(1-t)^3 + B3t(1-t)^2 + C3t^2(1-t) + D3t, t<-|0,1|
	*/
	class Qk_EXPORT CubicBezier {
	public:
		// define props
		Qk_DEFINE_PROP_GET(Vec2, p0);
		Qk_DEFINE_PROP_GET(Vec2, p1);
		Qk_DEFINE_PROP_GET(Vec2, p2);
		Qk_DEFINE_PROP_GET(Vec2, p3);

		/**
		* @constructor
		*/
		inline CubicBezier() {}
		
		/**
		* @constructor
		*/
		CubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3);
			
		/**
		* @func sample_curve_x
		*/
		inline float sample_curve_x(float t) const {
			// `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
			return ((ax * t + bx) * t + cx) * t + _p0.x();
		}
		
		/**
		* @func sample_curve_y
		*/
		inline float sample_curve_y(float t) const {
			return ((ay * t + by) * t + cy) * t + _p0.y();
		}
		
		/**
		* @func compute_bezier_points
		*/
		void sample_curve_points(uint32_t sample_count, float* out) const;
		
		/**
		* @func sample_curve_points
		*/
		Array<Vec2> sample_curve_points(uint32_t sample_count) const;

	protected:
		float ax, bx, cx;
		float ay, by, cy;
	};

	/**
	* @class FixedCubicBezier
	* @bases CubicBezier
	*/
	class Qk_EXPORT FixedCubicBezier: public CubicBezier {
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
		{}

		/**
		* @func sample_curve_derivative_x
		*/
		inline float sample_curve_derivative_x(float t) const {
			return (3.0 * ax * t + 2.0 * bx) * t + cx;
		}
		
		/**
		* @func solve_t Given an x value, find a parametric value it came from.
		*/
		float solve_t(float x, float epsilon) const;
		
		/**
		* @func solve
		*/
		inline float solve_y(float x, float epsilon) const {
			return (this->*_solve_y)(x, epsilon);
		}
		
	private:
		
		typedef float (FixedCubicBezier::*Solve)(float x, float epsilon) const;
		
		Solve _solve_y;

		Qk_DEFINE_INLINE_CLASS(Inl);
	};

	typedef FixedCubicBezier Curve;
	typedef const Curve cCurve;

	Qk_EXPORT extern const FixedCubicBezier LINEAR;
	Qk_EXPORT extern const FixedCubicBezier EASE;
	Qk_EXPORT extern const FixedCubicBezier EASE_IN;
	Qk_EXPORT extern const FixedCubicBezier EASE_OUT;
	Qk_EXPORT extern const FixedCubicBezier EASE_IN_OUT;

}
#endif