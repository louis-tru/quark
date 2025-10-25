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

#include "./pathv_cache.h"
#include "./render.h"

namespace qk {

	PathvCache::PathvCache(uint32_t maxCapacity, RenderBackend *render, ClearSync *sync)
		: _render(render), _sync(sync), _capacity(0), _maxCapacity(maxCapacity), _afterClear(nullptr) {
	}

	PathvCache::~PathvCache() {
		// First call
		clearAll(true);
		// When called a second time, the final deletion from the previous call is performed
		// clear(true);
	}

	const Path& PathvCache::getNormalizedPath(const Path &path) {
		if (path.isNormalized()) return path;
		auto hash = path.hashCode();
		Path *const *out;
		if (_NormalizedPathCache.get(hash, out)) return **out;
		auto p = new Path(path.normalizedPath(1));
		_capacity += p->ptsLen() * sizeof(Vec2);
		Qk_ASSERT(p->isNormalized());
		return *_NormalizedPathCache.set(hash, p);
	}

	const Path& PathvCache::getStrokePath(
		const Path &path, float width, Path::Cap cap, Path::Join join, float miterLimit) 
	{
		auto hash = path.hashCode();
		auto hash_part = ((*(int64_t*)&width) << 32) | *(int32_t*)&miterLimit;
		hash += (hash << 5) + hash_part + ((cap << 2) | join);
		Path *const*out;
		if (_StrokePathCache.get(hash, out))
			return **out;
		auto stroke = path.strokePath(width,cap,join,miterLimit);
		auto p = new Path(stroke.isNormalized() ? std::move(stroke): stroke.normalizedPath(1));
		_capacity += p->ptsLen() * sizeof(Vec2);
		return *_StrokePathCache.set(hash, p);
	}

	const VertexData& PathvCache::getPathTriangles(const Path &path) {
		auto hash = path.hashCode();
		Wrap<VertexData> *const*out;
		if (_PathTrianglesCache.get(hash, out))
			return (*out)->base;
		auto gb = new Wrap<VertexData>{path.getTriangles(1),{{this,0,0,0}}};
		gb->base.id = gb->id;
		gb->id->self = &gb->base;
		_capacity += gb->base.vCount * sizeof(Vec3);
		return _PathTrianglesCache.set(hash, gb)->base;
	}

	const VertexData& PathvCache::getAAFuzzStrokeTriangle(const Path &path, float width) {
		auto hash = path.hashCode();
		hash += (hash << 5) + (int32_t&)width;
		//Qk_DLog("getAAFuzzTriangle, %lu", hash);
		Wrap<VertexData> *const *out;
		if (_AAFuzzStrokeTriangleCache.get(hash, out))
			return (*out)->base;
		auto gb = new Wrap<VertexData>{path.getAAFuzzStrokeTriangle(width, 1),{{this,0,0,0}}};
		gb->base.id = gb->id;
		gb->id->self = &gb->base;
		_capacity += gb->base.vCount * sizeof(Vec3);
		return _AAFuzzStrokeTriangleCache.set(hash, gb)->base;
	}

	const RectPath& PathvCache::setRRectPathFromHash(uint64_t hash, RectPath&& rect) {
		auto gb = new Wrap<RectPath>{std::move(rect),{{this,0,0,0}}};
		gb->base.id = gb->id;
		gb->id->self = &gb->base;
		_capacity += gb->base.vCount * sizeof(Vec3);
		return _RectPathCache.set(hash, gb)->base;
	}

	const RectOutlinePath& PathvCache::setRRectOutlinePathFromHash(uint64_t hash, RectOutlinePath&& outline) {
		auto gb = new Wrap<RectOutlinePath,4>{std::move(outline),{{this,0,0,0},{this,0,0,0},{this,0,0,0},{this,0,0,0}}};
		gb->base.top.id = gb->id;
		gb->base.right.id = gb->id+1;
		gb->base.bottom.id = gb->id+2;
		gb->base.left.id = gb->id+3;
		gb->id[0].self = &gb->base.top;
		gb->id[1].self = &gb->base.right;
		gb->id[2].self = &gb->base.bottom;
		gb->id[3].self = &gb->base.left;
		_capacity += (
			gb->base.top.vCount +
			gb->base.right.vCount +
			gb->base.bottom.vCount + gb->base.left.vCount) * sizeof(Vec3);
		return _RectOutlinePathCache.set(hash, gb)->base;
	}

	const RectPath* PathvCache::getRRectPathFromHash(uint64_t hash) {
		Wrap<RectPath> *const *out;
		if (_RectPathCache.get(hash, out))
			return &(*out)->base;
		return nullptr;
	}

	const RectOutlinePath* PathvCache::getRRectOutlinePathFromHash(uint64_t hash) {
		Wrap<RectOutlinePath,4> *const *out;
		if (_RectOutlinePathCache.get(hash, out))
			return &(*out)->base;
		return nullptr;
	}

	const RectPath& PathvCache::getRectPath(const Rect &rect) {
		Hash5381 hash;
		hash.updatefv4(rect.begin.val);
		Wrap<RectPath> *const *out;
		if (_RectPathCache.get(hash.hashCode(), out))
			return (*out)->base;
		return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
	}

	const RectPath& PathvCache::getRRectPath(const Rect &rect, const Path::BorderRadius &radius) {
		Hash5381 hash;
		hash.updatefv4(rect.begin.val);
		hash.updatefv4(radius.leftTop.val);
		hash.updatefv4(radius.rightBottom.val);
		Wrap<RectPath> *const *out;
		if (_RectPathCache.get(hash.hashCode(), out))
			return (*out)->base;
		return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, radius));
	}

	inline static bool is_not_Zero(const float radius[4]) {
		return *reinterpret_cast<const uint64_t*>(radius) != 0 ||
			*reinterpret_cast<const uint64_t*>(radius+2) != 0;
	}

	const RectPath& PathvCache::getRRectPath(const Rect &rect, const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.begin.val);
		hash.updatefv4(radius);
		Wrap<RectPath> *const *out;
		if (_RectPathCache.get(hash.hashCode(), out))
			return (*out)->base;

		if (is_not_Zero(radius)) {
			float xy_0_5 = Float32::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius br{
				Qk_Min(radius[0], xy_0_5), Qk_Min(radius[1], xy_0_5),
				Qk_Min(radius[2], xy_0_5), Qk_Min(radius[3], xy_0_5),
			};
			return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, br));
		} else {
			return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
		}
	}

	const RectOutlinePath& PathvCache::getRectOutlinePath(const Rect &rect, const float border[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.begin.val);
		hash.updatefv4(border);
		Wrap<RectOutlinePath,4> *const *out;
		if (_RectOutlinePathCache.get(hash.hashCode(), out))
			return (*out)->base;
		return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRectOutline(rect, border));
	}

	const RectOutlinePath& PathvCache::getRRectOutlinePath(const Rect &rect, const float border[4], const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.begin.val);
		hash.updatefv4(border);
		hash.updatefv4(radius);
		Wrap<RectOutlinePath,4> *const *out;
		if (_RectOutlinePathCache.get(hash.hashCode(), out))
			return (*out)->base;

		if (is_not_Zero(radius)) {
			float xy_0_5 = Float32::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius br{
				{Qk_Min(radius[0],xy_0_5)}, {Qk_Min(radius[1],xy_0_5)},
				{Qk_Min(radius[2],xy_0_5)}, {Qk_Min(radius[3],xy_0_5)},
			};
			return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRRectOutline(rect, border, br));
		} else {
			return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRectOutline(rect, border));
		}
	}

	bool PathvCache::newVertexData(const VertexData::ID *vertexInThis) {
		if (vertexInThis) {
			if (vertexInThis->vao) {
				return true;
			} else if (vertexInThis->host == this) {
				_render->newVertexData(const_cast<VertexData::ID*>(vertexInThis));
				return true;
			}
		}
		return false;
	}

	void PathvCache::clear(bool all) {
		_sync->lock();
		clearUnsafe(all ? 2: 1/*memory warning clear half*/);
		_sync->unlock();
	}

	void PathvCache::clearUnsafe(int flags) {
		if (flags) {
			if (flags == 1) { // memory warning
				if (_capacity > _maxCapacity) {
					clearPart(Int32::max(_capacity * 0.5, _capacity - _maxCapacity)); // clean half
				}
			} else { // clear all
				clearAll(false);
			}
		} else if (_capacity > _maxCapacity) { // max limit clear
			clearPart(_capacity - _maxCapacity);
		}
	}

	void PathvCache::clearPart(uint32_t capacity) {
		clearAll(false); // TODO: Not yet realized
	}

	void PathvCache::clearAll(bool immediately) {
		for (auto &i: _NormalizedPathCache) {
			Release(i.second);
		}
		_NormalizedPathCache.clear();

		for (auto &i: _StrokePathCache) {
			Release(i.second);
		}
		_StrokePathCache.clear();

		auto render = _render;
		auto a0 = new Dict<uint64_t, Wrap<VertexData>*>(std::move(_PathTrianglesCache));
		auto a1 = new Dict<uint64_t, Wrap<VertexData>*>(std::move(_AAFuzzStrokeTriangleCache));
		auto b = new Dict<uint64_t, Wrap<RectPath>*>(std::move(_RectPathCache));
		auto c = new Dict<uint64_t, Wrap<RectOutlinePath,4>*>(std::move(_RectOutlinePathCache));

		// Must be called after rendering is complete
		Cb afterClear([render,a0,a1,b,c](auto &e) {
			for (auto &i: *a0) {
				render->deleteVertexData(i.second->id);
				delete i.second;
			}
			for (auto &i: *a1) {
				render->deleteVertexData(i.second->id);
				delete i.second;
			}
			for (auto &i: *b) {
				render->deleteVertexData(i.second->id);
				delete i.second;
			}
			for (auto &i: *c) {
				render->deleteVertexData(i.second->id);
				render->deleteVertexData(i.second->id+1);
				render->deleteVertexData(i.second->id+2);
				render->deleteVertexData(i.second->id+3);
				delete i.second;
			}
			Release(a0);
			Release(a1);
			Release(b);
			Release(c);
		});

		if (immediately) {
			render->post_message(afterClear);
		} else {
			render->post_message(Cb([this,afterClear](auto e) {
				if (_afterClear) {
					_afterClear->resolve(); // clear prev time
				}
				_afterClear = afterClear;
			}));
		}

		Qk_CHECK(_NormalizedPathCache.length() == 0);
		Qk_CHECK(_StrokePathCache.length() == 0);
		Qk_CHECK(_PathTrianglesCache.length() == 0);
		Qk_CHECK(_AAFuzzStrokeTriangleCache.length() == 0);
		Qk_CHECK(_RectPathCache.length() == 0);
		Qk_CHECK(_RectOutlinePathCache.length() == 0);

		_capacity = 0;
	}

}
