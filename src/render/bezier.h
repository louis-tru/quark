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

#ifndef __quark_render_bezier__
#define __quark_render_bezier__

#include "./math.h"

namespace qk {

	// Bezier curve formula of order N
	// F(t) = P_0(1-t)^n + P_nt^n + Σ(i=1,n-1) P_i(1-t)^(n-i) t^i, t∈[0,1]

	/**
	* @class QuadraticBezier quadratic Bezier curve
	*
	* F(t) = P_0(1-t)^2 t^0 + 2P_1(1-t)t^1 + P_2(1-t)^0 t^2, t∈[0,1]
	* F(t) = P_0(1-t)^2     + 2P_1(1-t)t   + P_2t^2        , t∈[0,1]
	*/
	class Qk_EXPORT QuadraticBezier {
	public:
		// define props
		Qk_DEFINE_PROP_GET(Vec2, p0, Const);
		Qk_DEFINE_PROP_GET(Vec2, p1, Const);
		Qk_DEFINE_PROP_GET(Vec2, p2, Const);

		/**
		 * @constructor
		*/
		QuadraticBezier(Vec2 p0, Vec2 p1, Vec2 p2);
		
		/**
		 * @method sample_curve_x
		*/
		float sample_curve_x(float t) const;
		
		/**
		 * @method sample_curve_y
		*/
		float sample_curve_y(float t) const;

		/**
		 * @method compute_bezier_points
		*/
		void sample_curve_points(uint32_t sample_count, float* out, int stride = 2) const;

		/**
		 * @method sample_curve_points
		*/
		Array<Vec2> sample_curve_points(uint32_t sample_count) const;
	};

	/**
	* @class CubicBezier cubic bezier curve
	*
	* F(t) = P_0(1-t)^3 t^0 + 3P_1(1-t)^2 t^1 + 3P_2(1-t)^1 t^2 + P_3(1-t)^0 t^3, t∈[0,1]
	* F(t) = P_0(1-t)^3     + 3P_1(1-t)^2 t   + 3P_2(1-t)t^2    + P_3t^3        , t∈[0,1]
	*/
	class Qk_EXPORT CubicBezier {
	public:
		Qk_DEFINE_PROP_GET(Vec2, p0, Const);
		Qk_DEFINE_PROP_GET(Vec2, p1, Const);
		Qk_DEFINE_PROP_GET(Vec2, p2, Const);
		Qk_DEFINE_PROP_GET(Vec2, p3, Const);

		CubicBezier(); // p0=0,p1=0,p2=1,p3=1
		CubicBezier(Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3);

		/**
		 * @method sample_curve_x
		*/
		inline float sample_curve_x(float t) const {
			// `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
			return ((ax * t + bx) * t + cx) * t + _p0.x();
		}
		
		/**
		 * @method sample_curve_y
		*/
		inline float sample_curve_y(float t) const {
			return ((ay * t + by) * t + cy) * t + _p0.y();
		}
		
		/**
		 * @method compute_bezier_points
		*/
		void sample_curve_points(uint32_t sample_count, float* out, int stride = 2) const;
		
		/**
		 * @method sample_curve_points
		*/
		Array<Vec2> sample_curve_points(uint32_t sample_count) const;

		/**
		 * @method sample_curve_derivative_x
		*/
		inline float sample_curve_derivative_x(float t) const {
			return (3.0 * ax * t + 2.0 * bx) * t + cx;
		}

	private:
		float ax, bx, cx;
		float ay, by, cy;
	};

	/**
	 * @class FixedCubicBezier fixed points p0=0 ... p3=1
	*/
	class Qk_EXPORT FixedCubicBezier: public CubicBezier {
	public:
		FixedCubicBezier(); // Fixed LINEAR, p1=0,p2=1
		FixedCubicBezier(Vec2 p1, Vec2 p2);

		/**
		 * @method solve_t Given an x value, find a parametric value it came from.
		 * p0=0 and p1=1 valid
		*/
		float solve_t(float x, float epsilon) const;

		/**
		 * @method solve_y p0=0, p1=1 valid
		*/
		float solve_y(float x, float epsilon) const;

	private:
		bool _isLINEAR;
	};

	typedef       FixedCubicBezier Curve;
	typedef const FixedCubicBezier cCurve;

	// Fixed value p0=0 ... p3=1
	Qk_EXPORT extern cCurve LINEAR;
	Qk_EXPORT extern cCurve EASE;
	Qk_EXPORT extern cCurve EASE_IN;
	Qk_EXPORT extern cCurve EASE_OUT;
	Qk_EXPORT extern cCurve EASE_IN_OUT;

}
#endif
