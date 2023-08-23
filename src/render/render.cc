/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include <math.h>
#include "../util/loop.h"
#include "../util/codec.h"
#include "./render.h"
#include "./gl/gl_render.h"

namespace qk {

	static uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		return Qk_MIN(n, 8);
	}

	RenderBackend::RenderBackend(Options opts)
		: _opts(opts)
		, _canvas(nullptr)
		, _delegate(nullptr)
		, _defaultScale(1)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kColor_Type_RGBA_8888;//kColor_Type_BGRA_8888;
		_opts.msaa = massSample(_opts.msaa);
	}

	RenderBackend::~RenderBackend() {}

	void RenderBackend::activate(bool isActive) {
	}

	const Path& RenderBackend::getNormalizedPath(const Path &path) {
		if (path.isNormalized()) return path;
		auto hash = path.hashCode();
		const Path *out;
		if (_NormalizedPathCache.get(hash, out)) return *out;
		if (_NormalizedPathCache.length() >= 1024) _NormalizedPathCache.clear();
		return _NormalizedPathCache.set(hash, path.normalizedPath(1));
	}

	const Path& RenderBackend::getStrokePath(
		const Path &path, float width, Path::Cap cap, Path::Join join, float miterLimit) 
	{
		auto hash = path.hashCode();
		auto hash_part = ((*(int64_t*)&width) << 32) | *(int32_t*)&miterLimit;
		hash += (hash << 5) + hash_part + ((cap << 2) | join);
		const Path *out;
		if (_StrokePathCache.get(hash, out)) return *out;
		if (_StrokePathCache.length() >= 1024) _StrokePathCache.clear();
		auto stroke = path.strokePath(width,cap,join,miterLimit);
		return _StrokePathCache.set(hash, stroke.isNormalized() ? std::move(stroke): stroke.normalizedPath(1));
	}

	const Array<Vec3>& RenderBackend::getPathTriangles(const Path &path) {
		auto hash = path.hashCode();
		const Array<Vec3> *out;
		if (_PathTrianglesCache.get(hash, out)) return *out;
		if (_PathTrianglesCache.length() >= 1024) {
			_PathTrianglesCache.clear();
		}
		return _PathTrianglesCache.set(hash, path.getTriangles(1));
	}

	const Array<Vec3>& RenderBackend::getAAFuzzStrokeTriangle(const Path &path, float width) {
		auto hash = path.hashCode();
		hash += (hash << 5) + *(int32_t*)&width;
		//Qk_DEBUG("getAAFuzzTriangle, %lu", hash);
		const Array<Vec3> *out;
		if (_AAFuzzStrokeTriangleCache.get(hash, out)) return *out;
		if (_AAFuzzStrokeTriangleCache.length() >= 1024) {
			_AAFuzzStrokeTriangleCache.clear();
		}
		return _AAFuzzStrokeTriangleCache.set(hash, path.getAAFuzzStrokeTriangle(width, 1));
	}

	const RectPath* RenderBackend::getRRectPathFromHash(uint64_t hash) {
		const RectPath *out = nullptr;
		_RectPathCache.get(hash, out);
		return out;
	}

	const RectPath& RenderBackend::setRRectPathFromHash(uint64_t hash, RectPath&& rect) {
		if (_RectPathCache.length() >= 1024) _RectPathCache.clear();
		return _RectPathCache.set(hash, std::move(rect));
	}

	const RectOutlinePath* RenderBackend::getRRectOutlinePathFromHash(uint64_t hash) {
		const RectOutlinePath *out = nullptr;
		_RectOutlinePathCache.get(hash, out);
		return out;
	}

	const RectOutlinePath& RenderBackend::setRRectOutlinePathFromHash(uint64_t hash, RectOutlinePath&& outline) {
		if (_RectOutlinePathCache.length() >= 1024) {
			_RectPathCache.clear();
		}
		return _RectOutlinePathCache.set(hash, std::move(outline));
	}

	const RectPath& RenderBackend::getRectPath(const Rect &rect) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
	}

	const RectPath& RenderBackend::getRRectPath(const Rect &rect, const Path::BorderRadius &radius) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius.leftTop.val);
		hash.updatefv4(radius.rightBottom.val);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, radius));
	}

	const RectPath& RenderBackend::getRRectPath(const Rect &rect, const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;

		if (*reinterpret_cast<const uint64_t*>(radius) == 0 && *reinterpret_cast<const uint64_t*>(radius+2) == 0)
		{
			return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRect(rect));
		} else {
			float xy_0_5 = Float::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius Br{
				Qk_MIN(radius[0], xy_0_5), Qk_MIN(radius[1], xy_0_5),
				Qk_MIN(radius[2], xy_0_5), Qk_MIN(radius[3], xy_0_5),
			};
			return setRRectPathFromHash(hash.hashCode(), RectPath::MakeRRect(rect, Br));
		}
	}

	const RectOutlinePath& RenderBackend::getRRectOutlinePath(const Rect &rect, const float border[4], const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(border);
		hash.updatefv4(radius);
		const RectOutlinePath *out;
		if (_RectOutlinePathCache.get(hash.hashCode(), out)) return *out;

		if (*reinterpret_cast<const uint64_t*>(radius) == 0 && *reinterpret_cast<const uint64_t*>(radius+2) == 0)
		{
				return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRectOutline(rect, border));
		} else {
			float xy_0_5 = Float::min(rect.size.x() * 0.5f, rect.size.y() * 0.5f);
			Path::BorderRadius Br{
				{Qk_MIN(radius[0],xy_0_5)}, {Qk_MIN(radius[1],xy_0_5)},
				{Qk_MIN(radius[2],xy_0_5)}, {Qk_MIN(radius[3],xy_0_5)},
			};
			return setRRectOutlinePathFromHash(hash.hashCode(), RectOutlinePath::MakeRRectOutline(rect, border, Br));
		}
	}

}
