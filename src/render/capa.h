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
#include "./source.h"
#include "src/render/math.h"

namespace qk {
	struct GC_ImageDrawInfo;
	class ImageSource;
	struct PaintGradient;
	class GPUCanvas;

	constexpr int kCAPATileSize = 16;
	constexpr int kCAPATileSizeShift = __builtin_ctz(kCAPATileSize);
	// Keep CAPA image sampling on the portable side: Vulkan's guaranteed
	// sampled-image count is low, so larger image batches are split on CPU.
	constexpr uint32_t kCAPAMaxImageCount = 16;
	// Short-edge tasks are bounded so one task can touch at most three tiles in
	// the bin pass, which gives each task fixed node ownership.
	constexpr float kCAPAShortEdgeLength = 8.0f;
	// CAPA nil value.
	constexpr uint32_t kCAPA_NIL = 0xffffffffu;
	// Paint source is guaranteed to produce alpha == 1 for every sampled point.
	constexpr uint32_t kCAPA_FLAG_PAINT_OPAQUE = 1u << 2;
	// Paint source is guaranteed to produce no mipmap filtering.
	constexpr uint32_t kCAPA_FLAG_NONE_MIPMAP_MODE = 1u << 3;
	// Composite pass should clear the destination before drawing.
	constexpr uint32_t kCAPA_FLAG_COMPOSITE_CLEAR_DST = 1u << 4;
	// Quantize completed coverage groups as a display experiment.
	constexpr uint32_t kCAPA_FLAG_COMPOSITE_QUANTIZE_COVERAGE = 1u << 5;

	// CAPA paint types
	enum CAPAPaintType {
		kCAPA_PAINT_SOLID = 0u,
		kCAPA_PAINT_GRADIENT = 1u,
		kCAPA_PAINT_IMAGE = 2u,
	};
	typedef MSLCapaPrepare::CAPAEdge CAPAEdge;
	typedef MSLCapaPrepare::CAPAPath CAPAPath;
	typedef MSLCapaComposite::CAPAGradientPaint CAPAGradientPaint;
	typedef MSLCapaComposite::CAPAImagePaint CAPAImagePaint;

	struct CAPABudget {
		// CPU budget is deliberately conservative. Shader passes cap real counts
		// against these limits and publish indirect dispatch sizes from the caps.
		Range globalBounds{{F32::limit_max}, {F32::limit_min}};
		IRange globalTileBounds;
		uint32_t globalTileCount;
		uint32_t maxPathTileRowCount = 0;
		uint32_t maxPathTileCount = 0;
		uint32_t maxShortEdgeCount = 0;
		uint32_t maxBoundaryTileCount = 0;
	};

	struct CAPADrawData {
		Array<CAPAPath> paths;
		Array<CAPAEdge> edges;
		Array <CAPAGradientPaint> gradientPaints;
		Array <CAPAImagePaint> imagePaints;
		Array<Sp<ImageSource>> imageSources;
		Array<PaintImage> imageSamplers;
		Array <Color4f> colors;
		Array <float> positions;
		CAPABudget budget;
		IVec2 surfaceOffset;
	};

	struct CAPAPaint {
		union {
			const PaintGradient *gradient;
			const GC_ImageDrawInfo *image;
		};
		CAPAPaintType type;
	};

	template<> struct ObjectTraits<CAPAEdge>: ObjectTraitsBase<CAPAEdge> {
		static constexpr bool isOrdinary = true;
	};
	template<> struct ObjectTraits<CAPAPath>: ObjectTraitsBase<CAPAPath> {
		static constexpr bool isOrdinary = true;
	};
	template<> struct ObjectTraits<CAPAGradientPaint>: ObjectTraitsBase<CAPAGradientPaint> {
		static constexpr bool isOrdinary = true;
	};
	template<> struct ObjectTraits<CAPAImagePaint>: ObjectTraitsBase<CAPAImagePaint> {
		static constexpr bool isOrdinary = true;
	};

	IVec2 capa_floor_tile_origin(Vec2 origin);
	IVec2 capa_ceil_tile_end(Vec2 end);

	/**
	* CAPA Builder is a batch builder for CAPA draw data.
	* It accumulates paths and edges, computes budgets, and prepares the data for rendering.
	*/
	struct CAPABuilder {
		CAPABuilder(GPUCanvas *owner);
		bool build(const Path &path, const Color4f& color, CAPAPaint* paint = nullptr);
		bool buildGradient(const Path &path, const PaintGradient *gradient, const Color4f &color);
		bool buildImage(const Path &path, const GC_ImageDrawInfo &info);
		void flush(); // Flush the accumulated CAPA data to the GPUCanvas for rendering.
		void reset(bool clear = false);
		inline Color4f premul_alpha(const Color4f &color) const {
			return color.premul_alpha();
		}
		inline IVec2 surfaceOffset() const { return _data.surfaceOffset; }
		inline void setSurfaceOffset(IVec2 offset) { _data.surfaceOffset = offset; }
		FillRule fillRule = kNonZero_FillRule;
	private:
		void steupPaint(CAPAPath &path, CAPAPaint* paint, const Mat& mat);
		int findImageTexture(const PaintImage *paint) const;
		int findImageSampler(const PaintImage *paint) const;
		uint32_t addImageTexture(const PaintImage *paint);
		uint32_t addImageSampler(const PaintImage *paint);
		bool canAddImageTexture(const PaintImage *paint) const;
		CAPADrawData _data;
		GPUCanvas *_owner;
		LinearAllocator _alloc;
		float _totalEdgeLength;
	};
}
#endif
