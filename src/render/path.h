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

#ifndef __quark__render__path__
#define __quark__render__path__

#include "../util/array.h"
#include "../util/hash.h"
#include "./bezier.h"
#include "./paint.h"

namespace qk {

	/**
	 * CPU/GPU hybrid triangle vertex data.
	 *
	 * VertexData stores path/shape triangle vertices. When no GPU backend is used,
	 * the CPU-side vertex array is used directly for rendering. When a GPU backend
	 * is available, the data may be lazily uploaded to backend-local GPU storage
	 * through VertexData::ID, and the CPU-side vertex array may then be cleared to
	 * save memory.
	 *
	 * @note id refers to the backend-local GPU cache object, if available.
	 * @note vertex may be empty after successful GPU upload.
	 */
	struct VertexData {
		struct ID;
		const ID   *id = nullptr; ///< Backend-local GPU cache id, if uploaded.
		uint32_t    vCount = 0;   ///< Number of vertices.
		Array<Vec3> vertex;       ///< CPU-side vertices: {x, y, aaSide/z}.
	};

	class Qk_EXPORT Path: public Reference {
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
		Path(const Path& path); // copy constructor
		Path(Path&& path) = default;

		Path& operator=(const Path& path);
		Path& operator=(Path&& path) = default;

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

		// point ptr access
		inline Vec2 atPt(uint32_t index) const { return _pts[index]; }
		inline PathVerb atVerb(uint32_t index) const { return (PathVerb)_verbs[index]; }
		inline cArray<Vec2>& pts() const { return _pts; }
		// inline cArray<PathVerb>& verbs() const { return (cArray<PathVerb>&)_verbs; }
		cArray<PathVerb>& verbs() const;
		inline uint32_t ptsLen() const { return _pts.length(); }
		inline uint32_t verbsLen() const { return _verbs.length(); }
		inline uint32_t sizeOf() const { return ptsLen() * sizeof(Vec2) + verbsLen() * sizeof(PathVerb); }
		inline const Hash& hash() const { return _hash; }
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
		 * @return {VertexData} { .vertex={ x, y, z }[] }
		*/
		VertexData getTriangles(float epsilon = 1.0, float z = 0.0) const;

		/**
		 * @method getAASideTriangle() returns signed aa side triangle vertices and body triangles
		 * @return {VertexData} {.vertex={ x, y, aaSide }[]}, aaSide < 0 inside, aaSide > 0 outside
		*/
		VertexData getAASideTriangle(float width, float epsilon = 1.0) const;

		/**
		 * @method dashPath() returns the dash path
		 * example: stage = {10, 5, 2, 5} means dash 10 and gap 5 and dash 2 and gap 5, then repeat,
		 * offset is the start offset of the stage
		*/
		Path dashPath(float *stage, int stageCount, float offset = 0) const;

		// modification to stroke path
		Path strokePath(float width,
			Cap cap = Cap::kButt_Cap, Join join = Join::kMiter_Join, float miterLimit = 0) const;

		// normalized path, transform kVerb_Quad and kVerb_Cubic spline to kVerb_Line
		Path& normalizedPath(float epsilon = 1.0); // normal
		// matrix transform
		void transform(const Mat& matrix);
		// scale transform
		void scale(Vec2 scale);

		/**
		 * seal path, after sealed, path data can not be modified
		*/
		void seal();

		/** is sealed */
		bool isSealed() const { return _sealed; }

		// get path region bounds, first check if the matrix is a unit matrix
		Range getBounds(const Mat* matrix = nullptr) const;
		// get region bounds from pts, do not check unit matrix
		static Range getBoundsFromPoints(const Vec2 pts[], uint32_t ptsLen, const Mat* matrix = nullptr);
	private:
		const Path* normalized(Path *out, float epsilon, bool updateHash) const;
		const Path* boundaryPath(Path *out, float epsilon) const;
		void quadTo2(float *p);
		void cubicTo2(float *p);
		// Props field:
		Array<Vec2> _pts;
		Array<uint8_t> _verbs;
		Hash _hash;
		bool _IsNormalized, _sealed, _isBoundaryPath;
		friend class RectPath;
		friend class RectOutlinePath;
	};

	/**
	 *
	 * RectPath avoids repeatedly rebuilding common rectangle geometry.
	 */
	struct Qk_EXPORT RectPath: Path {
		Rect rect;   ///< Rectangle bounds.
		int  flags;  ///< Border-radius corner flags.
		static RectPath MakeRect(const Rect& rect);
		static RectPath MakeRRect(const Rect& rect, const Path::BorderRadius &radius);
	};

	/**
	 *
	 * Stores the four outline sides separately so Canvas can draw/update border
	 * geometry efficiently.
	 */
	struct Qk_EXPORT RectOutlinePath {
		Path top, right, bottom, left;
		int flags; ///< Border is non zero for each side
		static RectOutlinePath MakeRectOutline(const Rect &rect, const float border[4]);
		static RectOutlinePath MakeRRectOutline(
			const Rect &rect, const float border[4], const Path::BorderRadius &radius
		);
	};
}
#endif
