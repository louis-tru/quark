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

#ifndef __quark__render__path__
#define __quark__render__path__

#include "../types.h"
#include "../bezier.h"
#include "../util/array.h"
#include "./paint.h"

namespace quark {

	class Qk_EXPORT Path: public Object {
	public:
		enum PathVerb: uint8_t {
			kVerb_Move = 0, // move
			kVerb_Line,  // straight line
			kVerb_Quad,  // quadratic bezier
			kVerb_Cubic, // Cubic bezier
			kVerb_Close, // close
		};
		enum StrokeMode {
			kCenter_StrokeMode,
			kOutside_StrokeMode,
			kInside_StrokeMode,
		};
		typedef Paint::Join Join;
		static Path Oval(const Rect& rect);
		static Path Arc (const Rect& rect, float startAngle, float sweepAngle, bool useCenter);
		static Path Rect(const Rect& rect);
		static Path Circle(Vec2 center, float radius);
		Path();
		Path(Vec2 move);
		Path(Vec2* pts, int len, PathVerb* verbs, int verbsLen);
		// add path points
		void move_to(Vec2 to);
		void line_to(Vec2 to);
		void quad_to(Vec2 control, Vec2 to);
		void cubic_to(Vec2 control1, Vec2 control2, Vec2 to);
		void oval_to(const quark::Rect& rect);
		void rect_to(const quark::Rect& rect);
		void arc_to (const quark::Rect& rect, float startAngle, float sweepAngle, bool useCenter);
		void close(); // close line
		// point ptr
		inline const Vec2* pts() const { return (Vec2*)*_pts; }
		inline const PathVerb* verbs() const { return (PathVerb*)*_verbs; }
		inline uint32_t pts_len() const { return _pts.length() >> 1; }
		inline uint32_t verbs_len() const { return _verbs.length(); }
		inline bool isNormalized() const { return _IsNormalized; }
		// convert func
		/**
		 * @brief toPolygons() convert to polygons and use anti alias
		 * @return {Array<Vec3>} points Vec3 { x, y, weight }[]
		*/
		Array<Vec3> getPolygons(int polySize = 3, bool antiAlias = false, float epsilon = 1.0) const;
		Array<Vec2> getEdgeLines(float epsilon = 1.0) const;
		// modification to stroke path
		Path strokePath(float width, Join join, float offset = 0) const;
		// Expand or shrink path
		Path extendPath(float width, Join join) const;
		// normalized path, transform kVerb_Quad and kVerb_Cubic spline to kVerb_Line
		Path normalizedPath(float epsilon = 1.0) const; // normal
		// matrix transfrom
		void transfrom(const Mat& matrix);
		// scale transfrom
		void scale(Vec2 scale);
		// clip path
		Path clip(const Path& path) const;
		// estimate sample rate
		static int get_quadratic_bezier_sample(const QuadraticBezier& curve, float epsilon = 1.0);
		static int get_cubic_bezier_sample(const CubicBezier& curve, float epsilon = 1.0);
	private:
		void quad_to2(float *p);
		void cubic_to2(float *p);
		Array<float> _pts;
		Array<uint8_t> _verbs;
		bool _IsNormalized;
	};

}
#endif
