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
			kVerb_Move = 0, // move
			kVerb_Line,  // straight line
			kVerb_Quad,  // quadratic bezier
			kVerb_Cubic, // Cubic bezier
			kVerb_Close, // close
		};
		typedef Paint::Join Join;
		static Path MakeOval(const Rect& rect);
		static Path MakeArc (const Rect& rect, float startAngle, float sweepAngle, bool useCenter);
		static Path MakeRect(const Rect& rect);
		static Path MakeCircle(Vec2 center, float radius);
		static Path MakeRRect(
			const Rect& rect,
			Vec2 borderRadiusLeftTop, Vec2 borderRadiusRightTop,
			Vec2 borderRadiusRightBottom, Vec2 borderRadiusLeftBottom);
		static Path MakeRRectOutline(
			const Rect& outside, const Rect &inside,
			Vec2 borderRadiusLeftTop, Vec2 borderRadiusRightTop,
			Vec2 borderRadiusRightBottom, Vec2 borderRadiusLeftBottom);
		Path();
		Path(Vec2 move);
		// add path points
		void moveTo(Vec2 to);
		void lineTo(Vec2 to);
		void quadTo(Vec2 control, Vec2 to);
		void cubicTo(Vec2 control1, Vec2 control2, Vec2 to);
		void ovalTo(const Rect& rect);
		void rectTo(const Rect& rect);
		void arcTo (const Rect& rect, float startAngle, float sweepAngle, bool useCenter);
		void close(); // close line
		// point ptr
		inline const Vec2* pts() const { return (const Vec2*)*_pts; }
		inline const PathVerb* verbs() const { return (const PathVerb*)*_verbs; }
		inline uint32_t ptsLen() const { return _pts.length() >> 1; }
		inline uint32_t verbsLen() const { return _verbs.length(); }
		inline bool isNormalized() const { return _IsNormalized; }
		inline uint64_t hashCode() const { return _hash.hash_code(); }
		inline const Array<float> &extData() const { return _ptsExt; }
		/**
		 * @brief extData.length == ptsLen
		 * @method setExtData() set points extend data
		 */
		void setExtData(Array<float> &&extData);

		// convert func
		/**
		 * @method getEdgeLines() convert to edge lines
		 * @arg close {bool} is auto close lines
		 * @return {Array<Vec2>} points { x, y }[]
		*/
		Array<Vec2> getEdgeLines(bool close, float epsilon = 1.0) const;

		/**
		 * @method getEdgeLinesAndGirth() convert to edge lines and girth offset
		 * @arg close {bool} is auto close lines
		 * @return {Array<Vec3>} points point { x, y, length }[]
		*/
		Array<Vec3> getEdgeLinesAndLength(bool close, float epsilon = 1.0) const;

		/**
		 * @method getPolygons() convert to polygons
		 * @return {Array<float>} points point { x, y, extData? }[]
		*/
		Array<float> getPolygons(int polySize = 3, float epsilon = 1.0, bool isExt = false) const;

		/**
		 * @method getPolygonsFromOutline() convert to polygons and girth offset from outline path
		 * @return {Array<Vec3>} points { x, y,length? }[]
		 */
		Array<float> getPolygonsFromOutline(float width, Join join,
			int polySize = 3, float epsilon = 1.0, float offset = 0, bool isLen = false) const;

		// modification to stroke path
		Path strokePath(float width, Join join, float offset = 0) const;
		// normalized path, transform kVerb_Quad and kVerb_Cubic spline to kVerb_Line
		Path normalizedPath(float epsilon = 1.0) const; // normal
		// matrix transfrom
		void transfrom(const Mat& matrix);
		// scale transfrom
		void scale(Vec2 scale);
		// estimate sample rate
		static int getQuadraticBezierSample(const QuadraticBezier& curve, float epsilon = 1.0);
		static int getCubicBezierSample(const CubicBezier& curve, float epsilon = 1.0);
	private:
		Path* normalized(Path *out, bool updateHash, float epsilon) const;
		void quadTo2(float *p);
		void cubicTo2(float *p);
		void startTo(Vec2 p);
		Array<float> _pts; // Vec2 {x,y}
		Array<float> _ptsExt; //
		Array<uint8_t> _verbs;
		SimpleHash _hash;
		bool _IsNormalized;
	};

}
#endif
