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

#include "./pathv_cache.h"
#include "./render.h"

namespace qk {

	inline static bool is_not_Zero(const float radius[4]) {
		return *reinterpret_cast<const uint64_t*>(radius) != 0 ||
			*reinterpret_cast<const uint64_t*>(radius+2) != 0;
	}

	struct PathvCacheInl: PathvCache {
		void clear(int flags) {
			PathvCache::clear(flags);
		}
		void clearExec() {
			if (!_clearExecs || _clearExecs->length() == 0)
				return;
			for (auto &i: *_clearExecs) {
				i->resolve();
			}
			_clearExecs->reset(0);
		}
	};

	void clear_PathvCache(PathvCache *cache, int flags) {
		static_cast<PathvCacheInl*>(cache)->clear(flags);
	}

	void clearExec_PathvCache(PathvCache *cache) {
		static_cast<PathvCacheInl*>(cache)->clearExec();
 	}

	PathvCache::PathvCache(uint32_t maxCapacity, RenderResource *render)
		: _render(render), _capacity(0), _maxCapacity(maxCapacity)
		, _clearExecs(new Array<Cb>) {
	}

	PathvCache::~PathvCache() {
		clearAll(true);
	}

	const Path& PathvCache::getNormalizedPath(const Path &path) {
		if (path.isNormalized()) return path;
		auto key = path.hashCode();
		Path *const *out;
		if (_normalizedPath.get(key, out)) return **out;
		auto p = new Path(Path(path).normalizedPath(1));
		_capacity += p->sizeOf();
		Qk_ASSERT(p->isNormalized());
		return *_normalizedPath.set(key, p);
	}

	const Path& PathvCache::getStrokePath(
		const Path &path, float width, Path::Cap cap, Path::Join join, float miterLimit) 
	{
		auto hash = path.hash();
		hash.updateu64(((*(int64_t*)&width) << 32) | *(int32_t*)&miterLimit);
		hash.updateu32((cap << 2) | join);
		auto key = hash.hashCode();
		Path *const*out;
		if (_strokePath.get(key, out))
			return **out;
		auto stroke = path.strokePath(width,cap,join,miterLimit);
		auto p = new Path(stroke.isNormalized() ? std::move(stroke): stroke.normalizedPath(1));
		_capacity += p->sizeOf();
		return *_strokePath.set(key, p);
	}

	const VertexData& PathvCache::getPathTriangles(const Path &path) {
		auto hash = path.hashCode();
		Wrap<VertexData> *const*out;
		if (_pathTriangles.get(hash, out))
			return (*out)->base;
		auto gb = new Wrap<VertexData>{path.getTriangles(1),{{this,0,0,0}}};
		gb->base.id = gb->id;
		gb->id->data = &gb->base;
		_capacity += gb->base.vCount * sizeof(Vec3);
		return _pathTriangles.set(hash, gb)->base;
	}

	const VertexData& PathvCache::getAASideTriangle(const Path &path, float radius, bool onlyAASide) {
		auto hash = path.hash();
		float floatBits[2] = {radius, static_cast<float>(onlyAASide)};
		hash.update2f(floatBits);
		auto key = hash.hashCode();
		Wrap<VertexData> *const *out;
		if (_aaSideTriangle.get(key, out))
			return (*out)->base;
		auto gb = new Wrap<VertexData>{path.getAASideTriangle(radius, 1, onlyAASide),{{this,0,0,0}}};
		gb->base.id = gb->id;
		gb->id->data = &gb->base;
		_capacity += gb->base.vCount * sizeof(Vec3);
		return _aaSideTriangle.set(key, gb)->base;
	}

	const RectPath& PathvCache::setRRectPathFromHash(uint64_t hash, RectPath&& rect) {
		auto gb = new Wrap<RectPath>{std::move(rect),{{this,0,0,0}}};
		_capacity += gb->base.sizeOf();
		return _rectPath.set(hash, gb)->base;
	}

	const RectPath& PathvCache::getRRectPath(const Rect &rect, const Path::BorderRadius &radius) {
		Hash hash;
		hash.update4f(rect.begin.val);
		hash.update4f(radius.leftTop.val);
		hash.update4f(radius.rightBottom.val);
		hash.updateu32(1); // flag for border radius
		Wrap<RectPath> *const *out;
		if (_rectPath.get(hash.hashCode(), out))
			return (*out)->base;
		return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, radius));
	}

	const RectPath& PathvCache::getRectPath(const Rect &rect) {
		return getInsideRRectPath(rect, nullptr, nullptr);
	}

	const RectPath& PathvCache::getRRectPath(const Rect &rect, const float radius[4]) {
		return getInsideRRectPath(rect, radius, nullptr);
	}

	const RectPath& PathvCache::getInsideRRectPath(const Rect &rect, const float radius[4], const float border[4]) {
		Hash hash;
		hash.update4f(rect.begin.val);
		hash.update4f(radius ? radius: (const float[4]){0,0,0,0});
		if (border)
			hash.update4f(border);
		auto key = hash.hashCode();
		Wrap<RectPath> *const *out;
		if (_rectPath.get(key, out)) {
			return (*out)->base;
		}
		if (border && is_not_Zero(border)) {
			auto newRect = rect;
			auto limit = std::min(newRect.size.x() * 0.5f, newRect.size.y() * 0.5f);
			newRect.begin[0] += border[3]; // left
			newRect.begin[1] += border[0]; // top
			newRect.size [0] -= border[3] + border[1]; // left + right
			newRect.size [1] -= border[0] + border[2]; // top + bottom

			if (radius && is_not_Zero(radius)) {
				float leftTop     = Qk_Min(radius[0],limit), rightTop   = Qk_Min(radius[1],limit);
				float rightBottom = Qk_Min(radius[2],limit), leftBottom = Qk_Min(radius[3],limit);
				Path::BorderRadius br{
					{leftTop-border[3],     leftTop-border[0]},     {rightTop-border[1],  rightTop-border[0]},
					{rightBottom-border[1], rightBottom-border[2]}, {leftBottom-border[3], leftBottom-border[2]},
				};
				return setRRectPathFromHash(key, RectPath::MakeRRect(newRect, br));
			} else {
				return setRRectPathFromHash(key, RectPath::MakeRect(newRect));
			}
		} else if (radius && is_not_Zero(radius)) {
			float limit = std::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius br{
				Qk_Min(radius[0], limit), Qk_Min(radius[1], limit),
				Qk_Min(radius[2], limit), Qk_Min(radius[3], limit),
			};
			return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, br));
		} else {
			return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
		}
	}

	const RectOutlinePath& PathvCache::setRRectOutlinePathFromHash(uint64_t hash, RectOutlinePath&& outline) {
		auto gb = new Wrap<RectOutlinePath,4>{std::move(outline),{{this,0,0,0},{this,0,0,0},{this,0,0,0},{this,0,0,0}}};
		_capacity +=
			gb->base.top.sizeOf() +
			gb->base.right.sizeOf() +
			gb->base.bottom.sizeOf() + gb->base.left.sizeOf();
		return _rectOutlinePath.set(hash, gb)->base;
	}

	const RectOutlinePath& PathvCache::getRectOutlinePath(const Rect &rect, const float border[4]) {
		return getRRectOutlinePath(rect, border, nullptr);
	}

	const RectOutlinePath& PathvCache::getRRectOutlinePath(const Rect &rect, const float border[4], const float radius[4]) {
		Hash hash;
		hash.update4f(rect.begin.val);
		hash.update4f(border);
		if (radius)
			hash.update4f(radius);
		Wrap<RectOutlinePath,4> *const *out;
		if (_rectOutlinePath.get(hash.hashCode(), out)) {
			return (*out)->base;
		}
		if (radius && is_not_Zero(radius)) {
			float limit = std::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius br{
				{Qk_Min(radius[0],limit)}, {Qk_Min(radius[1],limit)},
				{Qk_Min(radius[2],limit)}, {Qk_Min(radius[3],limit)},
			};
			return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRRectOutline(rect, border, br));
		} else {
			return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRectOutline(rect, border));
		}
	}

	const Array<Vec2>& PathvCache::getEdgeLines(const Path &path, float precision) {
		Hash hash = path.hash();
		hash.update1f(precision);
		auto key = hash.hashCode();
		Array<Vec2> const *out;
		if (_edgeLines.get(key, out))
			return *out;
		auto lines = path.getEdgeLines(precision);
		_capacity += lines.size();
		return _edgeLines.set(key, std::move(lines));
	}

	void PathvCache::clear(int flags) {
		if (flags == 0) {
			if (_capacity > _maxCapacity) { // max limit clear
				clearPart(_capacity - _maxCapacity);
			}
		} else if (flags == 1) { // memory warning, clean half or clean to max limit
			clearPart(I32::max(_capacity * 0.5, _capacity - _maxCapacity));
		} else { // clear all
			clearAll(false);
		}
	}

	void PathvCache::clearPart(uint32_t capacity) {
		clearAll(false); // TODO: Not yet realized
	}

	void PathvCache::clearAll(bool destroy) {
		for (auto &it: {&_normalizedPath, &_strokePath}) {
			for (auto &i: *it)
				Release(i.second);
			it->clear();
		}
		_capacity = 0;
		_edgeLines.clear();

		// If the render backend is not available, directly clear the cache data and return
		if (!_render) {
			for (auto &i: _pathTriangles) {
				delete i.second;
			}
			for (auto &i: _aaSideTriangle) {
				delete i.second;
			}
			for (auto &i: _rectPath) {
				delete i.second;
			}
			for (auto &i: _rectOutlinePath) {
				delete i.second;
			}
			_pathTriangles.clear();
			_aaSideTriangle.clear();
			_rectPath.clear();
			_rectOutlinePath.clear();
			return; // No render backend, skip GPU resource deletion
		}

		auto a0 = new Dict<uint64_t, Wrap<VertexData>*>(std::move(_pathTriangles));
		auto a1 = new Dict<uint64_t, Wrap<VertexData>*>(std::move(_aaSideTriangle));
		auto b = new Dict<uint64_t, Wrap<RectPath>*>(std::move(_rectPath));
		auto c = new Dict<uint64_t, Wrap<RectOutlinePath,4>*>(std::move(_rectOutlinePath));

		// Must be called after rendering is complete
		Cb exec([render=_render,a0,a1,b,c](auto &e) {
			for (auto &i: *a0) {
				render->unloadVertexData(i.second->id);
				delete i.second;
			}
			for (auto &i: *a1) {
				render->unloadVertexData(i.second->id);
				delete i.second;
			}
			for (auto &i: *b) {
				render->unloadVertexData(i.second->id);
				delete i.second;
			}
			for (auto &i: *c) {
				render->unloadVertexData(i.second->id);
				render->unloadVertexData(i.second->id+1);
				render->unloadVertexData(i.second->id+2);
				render->unloadVertexData(i.second->id+3);
				delete i.second;
			}
			Releasep(a0);
			Releasep(a1);
			Releasep(b);
			Releasep(c);
		});

		Qk_ASSERT(_clearExecs, "clear callback is null");

		if (destroy) {
			_render->post_message(Cb([execs=_clearExecs,exec](auto e) {
				for (auto &i: *execs)
					i->resolve(); // execute clear callback
				exec->resolve(); // execute clear callback
				Release(execs); // release array
			}));
			_clearExecs = nullptr;
		} else {
			_render->post_message(Cb([execs=_clearExecs,exec](auto e) {
				execs->push(exec); // add to render thread queue
			}));
		}

		Qk_CHECK(_normalizedPath.length() == 0);
		Qk_CHECK(_strokePath.length() == 0);
		Qk_CHECK(_pathTriangles.length() == 0);
		Qk_CHECK(_aaSideTriangle.length() == 0);
		Qk_CHECK(_rectPath.length() == 0);
		Qk_CHECK(_rectOutlinePath.length() == 0);
	}

}