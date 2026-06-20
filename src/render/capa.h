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
#include "../util/handle.h"

namespace qk {
	class GPUCanvas;

	constexpr int kCAPATileSize = 16;
	constexpr float kInvCAPATileSize = 1.0f / float(kCAPATileSize);
	constexpr int kCAPATileSizeShift = __builtin_ctz(kCAPATileSize);

	typedef MSLCapaPrepare::CAPAEdge CAPAEdge;
	typedef MSLCapaTile::CAPAPath CAPAPath;

	struct CAPADrawData {
		Sp<ImageSource> atlas;
		Array<CAPAPath> paths;
		Array<CAPAEdge> edges;
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
		bool buildColor(const Path &path, const Color4f &color);
		cCAPADrawData& endBuild();
		void reset(bool clear = false);
		cCAPADrawData& getDrawData() const { return _data; }
	private:
		CAPADrawData _data;
		GPUCanvas *_owner;
		LinearAllocator _alloc;
	};
}
#endif
