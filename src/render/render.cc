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
		, _default_scale(1)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kColor_Type_RGBA_8888;//kColor_Type_BGRA_8888;
		_opts.msaaSampleCnt = massSample(_opts.msaaSampleCnt);
	}

	RenderBackend::~RenderBackend() {}

	void RenderBackend::activate(bool isActive) {
	}

	const Array<Vec2>& RenderBackend::getPathTriangles(const Path &path) {
		auto hash = path.hashCode();
		const Array<Vec2> *out;
		if (_PathTrianglesCache.get(hash, out)) return *out;
		if (_PathTrianglesCache.length() >= 1024) _PathTrianglesCache.clear();
		return _PathTrianglesCache.set(hash, path.getTriangles(1));
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

	const Array<Vec3>& RenderBackend::getSDFStrokeTriangleStrip(const Path &path, float width) {
		auto hash = path.hashCode();
		hash += (hash << 5) + *(int32_t*)&width;
		//Qk_DEBUG("getSDFStrokeTriangleStrip, %lu", hash);
		const Array<Vec3> *out;
		if (_SDFStrokeTriangleStripCache.get(hash, out)) return *out;
		if (_SDFStrokeTriangleStripCache.length() >= 1024) _SDFStrokeTriangleStripCache.clear();
		return _SDFStrokeTriangleStripCache.set(hash, path.getSDFStrokeTriangleStrip(width, 1));
	}

	const Path& RenderBackend::getNormalizedPath(const Path &path) {
		if (path.isNormalized()) return path;
		auto hash = path.hashCode();
		const Path *out;
		if (_NormalizedPathCache.get(hash, out)) return *out;
		if (_NormalizedPathCache.length() >= 1024) _NormalizedPathCache.clear();
		return _NormalizedPathCache.set(hash, path.normalizedPath(1));
	}

	const RectPath& RenderBackend::getRectPath(const Rect &rect) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024) _RectPathCache.clear();
		return _RectPathCache.set(hash.hashCode(), RectPath::MakeRect(rect));
	}

	const RectPath& RenderBackend::getRRectPath(const Rect &rect, const float radius[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024) _RectPathCache.clear();

		if (*reinterpret_cast<const uint64_t*>(radius) == 0 && *reinterpret_cast<const uint64_t*>(radius+2) == 0)
		{
			return _RectPathCache.set(hash.hashCode(), RectPath::MakeRect(rect));
		} else {
			return _RectPathCache.set(hash.hashCode(), RectPath::MakeRRect(rect, {
				radius[0],radius[1],radius[2],radius[3]
			}));
		}
	}

	const RectPath& RenderBackend::getRRectPath(const Rect &rect, const float radius[4], const float radius_lessen[4]) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius);
		hash.updatefv4(radius_lessen);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024) _RectPathCache.clear();
		return _RectPathCache.set(hash.hashCode(), RectPath::MakeRRect(rect, {
			{radius[0]-radius_lessen[3], radius[0]-radius_lessen[0]},
			{radius[1]-radius_lessen[1], radius[1]-radius_lessen[0]},
			{radius[2]-radius_lessen[1], radius[2]-radius_lessen[2]},
			{radius[3]-radius_lessen[3], radius[3]-radius_lessen[2]},
		}));
	}

	const RectPath& RenderBackend::getRRectPath(const Rect &rect, const Path::BorderRadius &radius) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(radius.leftTop.val);
		hash.updatefv4(radius.rightBottom.val);
		const RectPath *out;
		if (_RectPathCache.get(hash.hashCode(), out)) return *out;
		if (_RectPathCache.length() >= 1024) _RectPathCache.clear();
		return _RectPathCache.set(hash.hashCode(), RectPath::MakeRRect(rect, radius));
	}

	const RectOutlinePath& RenderBackend::getRRectOutlinePath(const Rect &rect, const float border[4], const float radius[4], bool fixAA) {
		Hash5381 hash;
		hash.updatefv4(rect.origin.val);
		hash.updatefv4(border);
		hash.updatefv4(radius);
		if (fixAA) {
			hash.updatef(getAAUnitPixel());
		}
		const RectOutlinePath *out;
		if (_RectOutlinePathCache.get(hash.hashCode(), out)) return *out;
		if (_RectOutlinePathCache.length() >= 1024) _RectOutlinePathCache.clear();

		auto oR{rect};
		float Bo[4]{border[0],border[1],border[2],border[3]};
		if (fixAA) { // anti alias compensate
			auto up = getAAUnitPixel();
			oR.origin += (up * 0.5);
			oR.size -= up;
			Bo[0] -= up;
			Bo[1] -= up;
			Bo[2] -= up;
			Bo[3] -= up;
		}

		if (*reinterpret_cast<const uint64_t*>(radius) == 0 && *reinterpret_cast<const uint64_t*>(radius+2) == 0)
		{
			return _RectOutlinePathCache.set(hash.hashCode(), RectOutlinePath::MakeRectOutline(oR, Bo));
		} else {
			return _RectOutlinePathCache.set(hash.hashCode(), RectOutlinePath::MakeRRectOutline(oR, Bo, {
				radius[0],radius[1],radius[2],radius[3] 
			}));
		}
	}

}
