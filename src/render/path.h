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
#include "../render/bezier.h"
#include "../util/array.h"
#include "./paint.h"
#include "../util/hash.h"

namespace qk {

	class Qk_EXPORT Path: public Object {
	public:
		enum PathVerb: uint8_t {
			kVerb_Move,  // move
			kVerb_Line,  // straight line
			kVerb_Quad,  // quadratic bezier
			kVerb_Cubic, // Cubic bezier
			kVerb_Close, // close
		};
		struct BorderRadius {
			Vec2 leftTop,     rightTop;
			Vec2 rightBottom, leftBottom;
		};
		typedef Paint::Join Join;
		typedef Paint::Cap  Cap;
		static Path MakeOval(const Rect& rect, bool ccw = false);
		static Path MakeArc (const Rect& rect, float startAngle, float sweepAngle, bool useCenter);
		static Path MakeRect(const Rect& rect, bool ccw = false);
		static Path MakeCircle(Vec2 center, float radius, bool ccw = false);
		static Path MakeRRect(const Rect& rect, const BorderRadius &radius);
		static Path MakeRRectOutline(const Rect& outside, const Rect &inside, const BorderRadius &radius);
		Path();
		Path(Vec2 move);
		// add path points
		void moveTo(Vec2 to);
		void lineTo(Vec2 to);
		void quadTo(Vec2 control, Vec2 to);
		void cubicTo(Vec2 control1, Vec2 control2, Vec2 to);
		void ovalTo(const Rect& rect, bool ccw = false);
		void rectTo(const Rect& rect, bool ccw = false);
		void arcTo (const Rect& rect, float startAngle, float sweepAngle, bool useCenter);
		void close(); // close line
		void startTo(Vec2 p); // call move to or line to
		// point ptr
		inline const Vec2* pts() const { return (const Vec2*)*_pts; }
		inline const PathVerb* verbs() const { return (const PathVerb*)*_verbs; }
		inline uint32_t ptsLen() const { return _pts.length() >> 1; }
		inline uint32_t verbsLen() const { return _verbs.length(); }
		inline bool isNormalized() const { return _IsNormalized; }
		inline uint64_t hashCode() const { return _hash.hash_code(); }

		// convert func
		/**
		 * @method getEdgeLines() convert to edge lines
		 * @arg close {bool} is auto close lines
		 * @return {Array<Vec2>} points { x, y }[]
		*/
		Array<Vec2> getEdgeLines(bool close, float epsilon = 1.0) const;

		/**
		 * @method getVertexs() Convert to fixed size polygon vertices
		 * @return {Array<Vec2>} points point { x, y }[]
		*/
		inline Array<Vec2> getVertexs(int polySize = 3, float epsilon = 1.0) const {
			return getVertexsFromPaths(this, 1, polySize, epsilon);
		}

		/**
		 * @method dashPath() returns the dash path
		*/
		Path dashPath(float *stage, int stage_count) const;

		// modification to stroke path
		Path strokePath(float width,
			Cap cap = Cap::kButt_Cap, Join join = Join::kMiter_Join, float miter_limit = 0) const;
		// normalized path, transform kVerb_Quad and kVerb_Cubic spline to kVerb_Line
		Path normalizedPath(float epsilon = 1.0) const; // normal
		// matrix transfrom
		void transfrom(const Mat& matrix);
		// scale transfrom
		void scale(Vec2 scale);
		// estimate sample rate
		static int getQuadraticBezierSample(const QuadraticBezier& curve, float epsilon);
		static int getCubicBezierSample(const CubicBezier& curve, float epsilon);
		static Array<Vec2> getVertexsFromPaths(const Path *paths, int pathsLen, int polySize, float epsilon);
	private:
		Path* normalized(Path *out, bool updateHash, float epsilon) const;
		void quadTo2(float *p);
		void cubicTo2(float *p);
		Array<float> _pts; // Vec2 {x,y}
		Array<uint8_t> _verbs;
		SimpleHash _hash;
		bool _IsNormalized;
	};

	// Optimizing rect vertex generation algorithm
	struct Qk_EXPORT RectPath {
		Path        path;
		Array<Vec2> vertex; // triangle vertex {x,y}[3]
		inline uint64_t hashCode() const { return path.hashCode(); }
		static RectPath MakeRect(const Rect& rect);
		static RectPath MakeRRect(const Rect& rect, const Path::BorderRadius &radius);
	};

	// Optimizing rect outline vertex generation algorithm
	struct Qk_EXPORT RectOutlinePath {
		Path         outside,inside;
		// triangle vertex items {
		//   x,y,length-offset,width-offset,border-direction
		// }[3]
		Array<float> vertex; // triangle vertex
		inline uint64_t hashCode() const {
			return (outside.hashCode() << 32) | (inside.hashCode() & 0xFFFFFFFF);
		}
		static RectOutlinePath MakeRectOutline(const Rect &outside, const Rect &inside);
		static RectOutlinePath MakeRRectOutline(
			const Rect &outside, const Rect &inside, const Path::BorderRadius &radius);
	};

}
#endif
