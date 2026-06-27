/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_capa__
#define __quark_render_capa__

#include "./metal/mtl_shaders.h"
#include "./path.h"

namespace qk {
	class GPUCanvas;

	constexpr int kCAPATileSize = 16;
	constexpr int kCAPATileSizeShift = __builtin_ctz(kCAPATileSize);
	constexpr float kCAPAShortEdgeLength = 8.0f;
	constexpr int kCAPAShortEdgeChunkSize = 4;

	typedef MSLCapaPrepare::CAPAEdge CAPAEdge;
	typedef MSLCapaPrepare::CAPAPath CAPAPath;

	struct CAPABudget {
		Range globalBounds{{F32::limit_max}, {F32::limit_min}};
		IRange globalTileBounds;
		uint32_t globalTileCount;
		uint32_t maxPathTileRowCount = 0;
		uint32_t maxPathTileCount = 0;
		uint32_t maxShortEdgeCount = 0;
		uint32_t maxShortEdgeChunkCount = 0;
		uint32_t maxBoundaryTileCount = 0;
	};

	struct CAPADrawData {
		Array<CAPAPath> paths;
		Array<CAPAEdge> edges;
		CAPABudget budget;
	};

	typedef const CAPADrawData cCAPADrawData;

	template<> struct ObjectTraits<CAPAEdge>: ObjectTraitsBase<CAPAEdge> {
		static constexpr bool isOrdinary = true;
	};
	template<> struct ObjectTraits<CAPAPath>: ObjectTraitsBase<CAPAPath> {
		static constexpr bool isOrdinary = true;
	};
	template<> struct AllocatorConfig<CAPAEdge> { static constexpr uint32_t kMinCapacity = 4; };

	struct CAPABuilder {
		CAPABuilder(GPUCanvas *owner);
		bool build(const Path &path, const Color4f &color);
		void commit();
		void reset(bool clear = false);
		cCAPADrawData& getDrawData() const { return _data; }
		FillRule fillRule = kNonZero_FillRule;
	private:
		CAPADrawData _data;
		GPUCanvas *_owner;
		LinearAllocator _alloc;
		float _totalEdgeLength;
	};
}
#endif
