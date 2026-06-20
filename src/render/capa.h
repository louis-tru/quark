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

// @private head

#ifndef __quark_render_capa__
#define __quark_render_capa__

#include "./metal/mtl_shaders.h"
#include "./path.h"
#include "../util/handle.h"

namespace qk {
	class GPUCanvas;
	constexpr int kCAPATileSize = 16;
	constexpr int kCAPASampleGrid = 4;
	static_assert(kCAPATileSize * kCAPASampleGrid <= 64,
		"Compute AA inside mask only supports up to 64 X samples per tile");
	constexpr float kInvCAPATileSize = 1.0f / float(kCAPATileSize);
	constexpr int kCAPATileSizeShift = __builtin_ctz(kCAPATileSize);

	enum CAPAFillRule {
		kCAPANonZero_FillRule = 0,
		kCAPAEvenOdd_FillRule = 1,
		kCAPAPositive_FillRule = 2,
		kCAPANegative_FillRule = 3,
	};

	typedef MSLImage::CAPAPath CAPAPath;
	typedef MSLImage::CAPACompositeTile CAPACompositeTile;
	typedef MSLCapa::CAPAEdge CAPAEdge;
	typedef MSLCapa::CAPATileEdge CAPATileEdge;
	typedef MSLCapa::CAPATile CAPATile;
	static_assert(sizeof(CAPACompositeTile) == sizeof(uint16_t) * 6, "Metal edge ABI mismatch");
	static_assert(sizeof(CAPAEdge) == sizeof(float) * 6, "Metal edge ABI mismatch");
	static_assert(sizeof(CAPATileEdge) == 12, "Metal tile-edge ABI mismatch");
	static_assert(sizeof(CAPATile) == sizeof(uint32_t) * 4, "Metal tile ABI mismatch");

	struct CAPADrawData {
		Sp<ImageSource> atlas; // atlas
		Array<CAPAPath> paths;
		Array<CAPAEdge> edges;
		Array<CAPATileEdge> tileEdges;
		Array<CAPATile> tiles; // boundary tiles that require GPU edge testing
		Array<CAPACompositeTile> compositeTiles;
	};

	typedef const CAPADrawData cCAPADrawData;

	template<> struct ObjectTraits<CAPAEdge>: ObjectTraitsBase<CAPAEdge> {
		static constexpr bool isOrdinary = true;
	};
	template<> struct ObjectTraits<CAPAPath>: ObjectTraitsBase<CAPAPath> {
		static constexpr bool isOrdinary = true;
	};
	// 配置最小容量以避频繁扩容，分配器默认最小容量为 1。
	template<> struct AllocatorConfig<CAPAEdge> { static constexpr uint32_t kMinCapacity = 4; };
	template<> struct AllocatorConfig<CAPATileEdge> { static constexpr uint32_t kMinCapacity = 4; };

	struct CAPABuilder {
		CAPABuilder(GPUCanvas *owner);
		// 将路径边界数据追加到 CAPA 批处理中，用于 GPU 边缘测试和覆盖率计算。
		// 如果数据超过限制则自动提交绘制命令；如果路径过大而无法绘制，则返回 false。
		// 也可以手动结束构建并提交当前批次，或者重置丢弃当前批次数据。
		bool build(const Path &path, Range *clip, const Mat *mat, float precision = 1.0f);
		bool build(const Path &path);
		cCAPADrawData& endBuild(); // end build and create atlas texture, should be called before drawing CAPA paths
		void commit(); // end buffer and commit current CAPA data to GPU.
		void reset(bool clear = false); // reset and clear all CAPA data.
		// 为后续 CAPA 路径设置混合模式，如果混合模式更改，将自动提交当前批次。
		void setBlendMode(BlendMode mode);
		CAPAPath& getPath(int index) { return _data.paths[index]; }
		cCAPADrawData& getDrawData() const { return _data; }
		Color4f color; // color state for CAPABuilder
		CAPAFillRule fillRule = kCAPANonZero_FillRule;
	private:
		bool buildTileEdges(Range &bounds, int edgeIndex, int edgeEnd, int tileCountX, int tileCountY);
		CAPADrawData _data;
		GPUCanvas *_owner;
		LinearAllocator _alloc,_alloc2;
		BlendMode _blendMode;
	};
}
#endif
