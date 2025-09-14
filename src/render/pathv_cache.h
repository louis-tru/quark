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

#ifndef __quark__render__pathv_cache__
#define __quark__render__pathv_cache__

#include "./path.h"
#include "../util/dict.h"
#include "../util/cb.h"

namespace qk
{
	class RenderBackend;
	class Canvas;
	class PathvCache;
	struct VertexData::ID {
		PathvCache  *host;
		VertexData  *self;
		uint32_t    vao,vbo; // gpu buffer
	};

	// paths and gpu vertices data caching
	class Qk_EXPORT PathvCache: public Object {
		Qk_DISABLE_COPY(PathvCache);
	public:
		template<class T, int N = 1>
		struct Wrap {
			T              base; // VertexData
			VertexData::ID id[N];
		};

		class ClearSync {
		public:
			virtual void lock() = 0;
			virtual void unlock() = 0;
		};

		Qk_DEFINE_PROP_GET(uint32_t, capacity, Const); // Used memory capacity
		Qk_DEFINE_PROP_GET(uint32_t, maxCapacity, Const); // max memory capacity

		PathvCache(uint32_t maxCapacity, RenderBackend *render, ClearSync *lock);
		~PathvCache();

		/**
		 * @dev get normalized path cache
		 */
		const Path& getNormalizedPath(const Path &path);

		/**
		 * @dev get stroke path cache
		 */
		const Path& getStrokePath(const Path &path,
			float width, Path::Cap cap, Path::Join join, float miterLimit
		);

		/**
		 * @dev get path triangles cache
		*/
		const VertexData& getPathTriangles(const Path &path);

		/**
		 * @dev get aa fuzz stroke path triangle cache
		*/
		const VertexData& getAAFuzzStrokeTriangle(const Path &path, float width);

		/**
		 * @dev get radius rect path cache from hash code
		*/
		const RectPath* getRRectPathFromHash(uint64_t hash);

		/**
		 * @dev set radius rect path cache from hash code
		*/
		const RectPath& setRRectPathFromHash(uint64_t hash, RectPath&& rect);

		/**
		 * @dev get radius rect outline path cache from hash code
		*/
		const RectOutlinePath* getRRectOutlinePathFromHash(uint64_t hash);

		/**
		 * @dev set rect outline path cache from hash code
		*/
		const RectOutlinePath& setRRectOutlinePathFromHash(uint64_t hash, RectOutlinePath&& outline);

		/**
		 * @dev get rect path cache
		 */
		const RectPath& getRectPath(const Rect &rect);

		/**
		 * @dev get radius rect path cache
		 * @param rect {Rect} rect
		 * @param radius {BorderRadius} border radius
		*/
		const RectPath& getRRectPath(const Rect &rect, const Path::BorderRadius &radius);

		/**
		 * @dev get radius rect path cache and limit radius size
		 * @param rect {Rect} rect
		 * @param radius {float[4]} border radius leftTop,rightTop,rightBottom,leftBottom
		*/
		const RectPath& getRRectPath(const Rect &rect, const float radius[4]);

		/**
		 * @dev get radius rect outline path cache and limit radius size
		 * @param rect {Rect} outside border rect
		 * @param border {float[4]} inside border width top,right,bottom,left
		 * @param radius {float[4]} outside border radius leftTop,rightTop,rightBottom,leftBottom
		 */
		const RectOutlinePath& getRRectOutlinePath(const Rect &rect, const float border[4], const float radius[4]);

		/**
		 * @dev get rect outline path cache and limit radius size
		 * @param rect {Rect} outside border rect
		 * @param border {float[4]} inside border width top,right,bottom,left
		*/
		const RectOutlinePath& getRectOutlinePath(const Rect &rect, const float border[4]);

		/**
		 * Setting and use gpu vertex data
		 * @dev If the incoming data belongs to the self updated data to the GPU, and update the id
		 * @note that this function can only be called on the rendering thread
		 * @returns {bool} Returns true if data is successfully set to GPU
		 * @thread gpu render thread
		*/
		bool newVertexData(const VertexData::ID *vertexInThis);

		/**
		 * @dev clear cache data
		 * @note Must be called in a worker thread,
		 *  these methods are called on the same thread as `getRRectOutlinePath()`
		 *  or displayed thread mutual exclusion measures
		*/
		void clear(bool all = false);

	protected:
		void clearUnsafe(int flags);
		void clearAll(bool immediately);
		void clearPart(uint32_t capacity);
		RenderBackend *_render;
		ClearSync     *_sync;
		Cb            _afterClear;
		Dict<uint64_t, Path*> _NormalizedPathCache, _StrokePathCache; // path hash => path
		Dict<uint64_t, Wrap<VertexData>*> _PathTrianglesCache; // path hash => triangles
		Dict<uint64_t, Wrap<VertexData>*> _AAFuzzStrokeTriangleCache; // path hash => aa fuzz stroke triangles
		Dict<uint64_t, Wrap<RectPath>*> _RectPathCache; // rect hash => rect path
		Dict<uint64_t, Wrap<RectOutlinePath,4>*> _RectOutlinePathCache; // rect hash => rect outline path
	};

} // namespace qk

#endif
