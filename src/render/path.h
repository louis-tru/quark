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

#include "../util/array.h"
#include "../util/hash.h"
#include "./bezier.h"
#include "./paint.h"

namespace qk {

	struct VertexData {
		struct ID;
		const  ID   *id; // id of pathv cache object
		uint32_t    vCount; // vertex count
		Array<Vec3> vertex; // hold pointer triangle vertex {x,y,aafuzz or <z>}
	};

	class Qk_EXPORT Path: public Object {
	public:
		enum PathVerb: uint8_t {
			kMove_Verb,  // move
			kLine_Verb,  // straight line
			kQuad_Verb,  // quadratic bezier
			kCubic_Verb, // Cubic bezier
			kClose_Verb, // close
		};
		struct BorderRadius {
			Vec2 leftTop,     rightTop;
			Vec2 rightBottom, leftBottom;
		};
		typedef Paint::Join Join;
		typedef Paint::Cap  Cap;
		static Path MakeOval(const Rect& rect, bool ccw = false);
		static Path MakeArc (const Rect& rect, float startAngle,
													float sweepAngle, bool useCenter = false, bool close = true);
		static Path MakeRect(const Rect& rect, bool ccw = false);
		static Path MakeCircle(Vec2 center, float radius, bool ccw = false);
		static Path MakeRRect(const Rect& rect, const BorderRadius &radius);
		static Path MakeRRectOutline(const Rect &outside, const Rect &inside, const BorderRadius &radius);
		Path();
		Path(Vec2 move);

		// add path points
		void moveTo(Vec2 to);
		void lineTo(Vec2 to);
		void quadTo(Vec2 control, Vec2 to);
		void cubicTo(Vec2 control1, Vec2 control2, Vec2 to);
		void ovalTo(const Rect& rect, bool ccw = false);
		void rectTo(const Rect& rect, bool ccw = false);
		/**
		 * Call arc(rect.origin + rect.size * 0.5, rect.size * 0.5, startAngle, sweepAngle, useCenter)
		*/
		void arcTo(const Rect& rect, float startAngle, float sweepAngle, bool useCenter = false);
		void arc(Vec2 center, Vec2 radius, float startAngle, float sweepAngle, bool useCenter = false);
		void close(); // close line
		void concat(const Path& path);

		// point ptr
		inline const Vec2* pts() const { return (const Vec2*)*_pts; }
		inline const Vec2* ptsBack() const { return (const Vec2*)&_pts.lastAt(1); }
		inline const PathVerb* verbs() const { return (const PathVerb*)*_verbs; }
		inline uint32_t ptsLen() const { return _pts.length() >> 1; }
		inline uint32_t verbsLen() const { return _verbs.length(); }
		inline uint64_t hashCode() const { return _hash.hashCode(); }
		inline bool isNormalized() const { return _IsNormalized; }

		// convert func
		/**
		 * @method getEdgeLines() convert to edge lines
		 * @return {Array<Vec2>} points { x, y }[]
		*/
		Array<Vec2> getEdgeLines(float epsilon = 1.0) const;

		/**
		 * @method getTriangles() Convert to fixed size polygon vertices
		 * @return {VertexData} { .vertex={ x, y, 0.0 }[] }
		*/
		VertexData getTriangles(float epsilon = 1.0) const;

		/**
		 * @method getAAFuzzStrokeTriangle() returns aa fuzz value stroke triangle vertices
		 * @return {VertexData} {.vertex={ x, y, aa fuzz value renge 1 to -1 }[]}
		*/
		VertexData getAAFuzzStrokeTriangle(float width, float epsilon = 1.0) const;

		/**
		 * @method dashPath() returns the dash path
		*/
		Path dashPath(float *stage, int stageCount, float offset = 0) const;

		// modification to stroke path
		Path strokePath(float width,
			Cap cap = Cap::kButt_Cap, Join join = Join::kMiter_Join, float miterLimit = 0) const;

		// normalized path, transform kVerb_Quad and kVerb_Cubic spline to kVerb_Line
		Path normalizedPath(float epsilon = 1.0) const; // normal
		// matrix transform
		void transform(const Mat& matrix);
		// scale transform
		void scale(Vec2 scale);

		// get path region bounds, first check if the matrix is a unit matrix
		Range getBounds(const Mat* matrix = nullptr) const;
		// get region bounds from pts, do not check unit matrix
		static Range getBoundsFromPoints(const Vec2 pts[], uint32_t ptsLen, const Mat* matrix = nullptr);
	private:
		Path* normalized(Path *out, float epsilon, bool updateHash) const;
		void quadTo2(float *p);
		void cubicTo2(float *p);
		// Props field:
		Array<float> _pts;
		Array<uint8_t> _verbs;
		Hash5381 _hash;
		bool _IsNormalized;
		friend class RectPath;
		friend class RectOutlinePath;
	};

	// combination of paths and triangle vertices
	struct Pathv: VertexData {
		Path        path;
	};

	// Optimizing rect vertex generation algorithm
	struct Qk_EXPORT RectPath: Pathv {
		Rect    rect;
		int     rrectMask; // border radius mask
		static RectPath MakeRect(const Rect& rect);
		static RectPath MakeRRect(const Rect& rect, const Path::BorderRadius &radius);
	};

	// Optimizing rect outline vertex generation algorithm
	struct Qk_EXPORT RectOutlinePath {
		Pathv top,right,bottom,left;
		static RectOutlinePath MakeRectOutline(const Rect &rect, const float border[4]);
		static RectOutlinePath MakeRRectOutline(
			const Rect &rect, const float border[4], const Path::BorderRadius &radius
		);
	};

}
#endif
