/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#import "./mtl_canvas.h"
#include "src/render/math.h"
#include "src/render/render.h"
#include "src/render/source.h"
#import "./mtl_render.h"

namespace qk {

	cMTLMemBlock& makeBuffer(MTL_CmdPack &cmd, const void *src, uint32_t size, uint32_t minSize = 0) {
		auto &block = cmd.allocator->alloc(size, minSize);
		Qk_ASSERT(block.end >= block.begin + size, "Not enough space in buffer block");
		if (size)
			memcpy((char*)block.val.contents + block.begin, src, size);
		return block;
	};

	template<typename T>
	cMTLMemBlock& makeBufferT(MTL_CmdPack &cmd, const T *src, uint32_t length) {
		return makeBuffer(cmd, src, length * sizeof(T), sizeof(T));
	}
	MTLTextureID mtl_get_texture_from(const ImageSource* src, MTLTextureID _else = nil);

	bool MetalCanvas::drawCAPACmd(CAPADrawData &data) {
		auto edgeCount = data.edges.length();
		if (!edgeCount)
			return false;
		Color4f clearColor;
		bool clearDst = onlyEndEncoderPass(clearColor);

		auto &budget = data.budget;
		auto pathCount = data.paths.length();
		// The CPU allocates conservative pools once. GPU passes publish real
		// counts into env and then use indirect dispatch for the dependent passes.
		auto env = _cmdPack.allocator->alloc<MSLCapaPrepare::CAPAEnvironment>(1);
		auto envData = (MSLCapaPrepare::CAPAEnvironment*)((char*)env.val.contents + env.begin);
		envData->globalTileBounds = IVec4(0x7fffffff, 0x7fffffff, -0x7fffffff, -0x7fffffff);
		envData->globalTileCount = 0;
		envData->taskCount = 0;
		envData->pathTileCount = 0;
		envData->pathTileRowCount = 0;
		envData->boundaryTileCount = 0;
		envData->boundaryDoneCount = 0;
		envData->layerPlanPathTileCount = 0;

		if (clearDst) {
			auto surfaceSize = _state->output ? _state->output->size() : _surfaceSize;
			budget.globalBounds = Range{{0, 0}, surfaceSize};
			budget.globalTileBounds = IRange{
				capa_floor_tile_origin(budget.globalBounds.begin),
				capa_ceil_tile_end(budget.globalBounds.end),
			};
			envData->globalTileBounds = *(IVec4*)budget.globalTileBounds.begin.val;
			auto tileSpan = budget.globalTileBounds.size();
			budget.globalTileCount = tileSpan.x() * tileSpan.y() * kCAPABudgetMultiplier;
		}
		// Upload path metadata and path-space edges. The remaining buffers are
		// GPU-owned staging/final pools for the 12-pass CAPA pipeline.
		auto paths = makeBuffer(_cmdPack, data.paths.val(), data.paths.size());
		auto edges = makeBuffer(_cmdPack, data.edges.val(), data.edges.size());
		auto gradientPaints = makeBufferT(_cmdPack, data.gradientPaints.val(), data.gradientPaints.length());
		auto imagePaints = makeBufferT(_cmdPack, data.imagePaints.val(), data.imagePaints.length());
		auto colors = makeBufferT(_cmdPack, data.colors.val(), data.colors.length());
		auto positions = makeBufferT(_cmdPack, data.positions.val(), data.positions.length());
		// allocate budget space for the CAPA pipeline
		auto shortTasks = _cmdPack.alloc<MSLCapaPrepare::CAPAShortEdgeTask>(budget.maxShortEdgeCount);
		auto shortEdges = _cmdPack.alloc<MSLCapaBin::CAPAShortEdgeNode>(budget.maxShortEdgeCount * 3);
		auto globalTiles = _cmdPack.alloc<MSLCapaLayerPlan::CAPAGlobalTile>(budget.globalTileCount);
		auto pathTiles = _cmdPack.alloc<MSLCapaLayerPlan::CAPAPathTile>(budget.maxPathTileCount);
		auto smallTiles = _cmdPack.alloc<MSLCapaTile::CAPASmallTile>(budget.maxPathTileCount);
		auto boundaryTiles = _cmdPack.alloc<MSLCapaCoverage::CAPABoundaryTile>(budget.maxBoundaryTileCount);
		auto coverageTiles = _cmdPack.alloc<MSLCapaCoverage::CAPACoverageTile>(budget.maxBoundaryTileCount);
		auto tileRows = _cmdPack.alloc<MSLCapaPrepareTiles::CAPAPathTileRow>(budget.maxPathTileRowCount);

		// CAPAEnvironment stores Metal-compatible indirect dispatch structs, so
		// each later pass can launch without CPU readback.
		auto envIndirectOffset = [&](auto field) -> NSUInteger {
			return NSUInteger(env.begin + field);
		};

#if DEBUG && 0
		Qk_DLog("Budget globalTileBounds: (%d, %d, %d, %d), "
						"globalTileCount: %d, "
						"maxShortEdgeCount: %d, "
						"maxPathTileCount: %d, "
						"maxPathTileRowCount: %d, "
						"maxBoundaryTileCount: %d",
			budget.globalTileBounds.begin.x(),
			budget.globalTileBounds.begin.y(),
			budget.globalTileBounds.end.x(),
			budget.globalTileBounds.end.y(),
			budget.globalTileCount,
			budget.maxShortEdgeCount,
			budget.maxPathTileCount,
			budget.maxPathTileRowCount,
			budget.maxBoundaryTileCount);
		[_cmdPack.current addCompletedHandler:^(MTLCommandBufferID buffer) {
			Qk_DLog("Real   globalTileBounds: (%d, %d, %d, %d), "
							"globalTileCount: %d, "
							"taskCount        : %d, "
							"pathTileCount   : %d, "
							"pathTileRowCount   : %d, "
							"boundaryTileCount   : %d, "
							"layerPlanPathTileCount   : %d "
				,
				envData->globalTileBounds.x(),
				envData->globalTileBounds.y(),
				envData->globalTileBounds.z(),
				envData->globalTileBounds.w(),
				envData->globalTileCount,
				envData->taskCount,
				envData->pathTileCount,
				envData->pathTileRowCount,
				envData->boundaryTileCount,
				envData->layerPlanPathTileCount
			);
		}];
#endif

		{ // prepare edge data
			auto &shader = _shaders.capaPrepare;
			MSLCapaPrepare::PcArgs pc{
				.shortEdgeLength=kCAPAShortEdgeLength,
				.edgeCount=edgeCount,
				.maxTaskCount=budget.maxShortEdgeCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA prepare";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:edges.val offset:edges.begin atIndex:shader.compute.edges];
			[enc setBuffer:shortTasks.val offset:shortTasks.begin atIndex:shader.compute.shortEdgeTasks];
			[enc dispatchThreadgroups:MTLSizeMake((edgeCount + 31) >> 5, 1, 1)
					threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // prepare path tiles and tile rows
			auto &shader = _shaders.capaPrepareTiles;
			MSLCapaPrepareTiles::PcArgs pc{
				.pathCount=pathCount,
				.maxPathTileCount=budget.maxPathTileCount,
				.maxPathTileRowCount=budget.maxPathTileRowCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA prepare path tiles";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:tileRows.val offset:tileRows.begin atIndex:shader.compute.tileRows];
			[enc setBuffer:imagePaints.val offset:imagePaints.begin atIndex:shader.compute.imagePaints];
			[enc dispatchThreadgroups:MTLSizeMake((pathCount + 31) >> 5, 1, 1)
				threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // prepare indirect dispatch group counts
			auto &shader = _shaders.capaPrepareDispatch;
			MSLCapaPrepareDispatch::PcArgs pc{
				.maxTaskCount=budget.maxShortEdgeCount,
				.maxPathTileCount=budget.maxPathTileCount,
				.maxPathTileRowCount=budget.maxPathTileRowCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA prepare dispatch args";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc dispatchThreadgroups:MTLSizeMake(1, 1, 1)
				threadsPerThreadgroup:MTLSizeMake(1, 1, 1)];
			[enc endEncoding];
		}
		{ // initialize small tiles
			auto &shader = _shaders.capaTile;
			MSLCapaTile::PcArgs pc{
				.maxPathTileCount=budget.maxPathTileCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA small tile init";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:smallTiles.val offset:smallTiles.begin atIndex:shader.compute.smallTiles];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, tilePassGroups_Size32))
				threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // bin short-edge tasks into tile-local lists
			auto &shader = _shaders.capaBin;
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA short-edge bin";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:edges.val offset:edges.begin atIndex:shader.compute.edges];
			[enc setBuffer:shortTasks.val offset:shortTasks.begin atIndex:shader.compute.shortEdgeTasks];
			[enc setBuffer:smallTiles.val offset:smallTiles.begin atIndex:shader.compute.smallTiles];
			[enc setBuffer:shortEdges.val offset:shortEdges.begin atIndex:shader.compute.shortEdges];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, binPassGroups_Size32))
				threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // allocate boundary tiles and write boundary-tile dispatch group counts
			auto &shader = _shaders.capaBoundary;
			MSLCapaBoundary::PcArgs pc{
				.maxBoundaryTileCount=budget.maxBoundaryTileCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA boundary tile alloc";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:smallTiles.val offset:smallTiles.begin atIndex:shader.compute.smallTiles];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:tileRows.val offset:tileRows.begin atIndex:shader.compute.tileRows];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, classifyPassGroups_Size32))
				threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // compute boundary-tile row backdrop
			auto &shader = _shaders.capaBackdrop;
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA backdrop";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:shortEdges.val offset:shortEdges.begin atIndex:shader.compute.shortEdges];
			[enc setBuffer:tileRows.val offset:tileRows.begin atIndex:shader.compute.tileRows];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, backdropPassGroups_Size16_2))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize, 2, 1)];
			[enc endEncoding];
		}
		{ // classify edge-free full tiles
			auto &shader = _shaders.capaClassify;
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA classify";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:smallTiles.val offset:smallTiles.begin atIndex:shader.compute.smallTiles];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:tileRows.val offset:tileRows.begin atIndex:shader.compute.tileRows];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, classifyPassGroups_Size32))
				threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // build global-tile layer plans and z-linear coverage tiles
			auto &shader = _shaders.capaLayerPlan;
			MSLCapaLayerPlan::PcArgs pc{
				.pathCount=pathCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA layer plan";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:globalTiles.val offset:globalTiles.begin atIndex:shader.compute.globalTiles];
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			[enc setBuffer:smallTiles.val offset:smallTiles.begin atIndex:shader.compute.smallTiles];
			// Generated wrappers do not expose this binding yet; keep the shader
			// binding number visible here instead of pretending it is arbitrary.
			[enc setBuffer:coverageTiles.val offset:coverageTiles.begin atIndex:6];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, layerPlanPassGroups_Size32))
				threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // compute boundary row prefix
			auto &shader = _shaders.capaPrefix;
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA prefix";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:tileRows.val offset:tileRows.begin atIndex:shader.compute.tileRows];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, prefixPassGroups_Size16_2))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize, 2, 1)];
			[enc endEncoding];
		}
		{ // compute retained boundary tile coverage
			auto &shader = _shaders.capaCoverage;
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA coverage";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:shortEdges.val offset:shortEdges.begin atIndex:shader.compute.shortEdges];
			[enc setBuffer:coverageTiles.val offset:coverageTiles.begin atIndex:5];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, coveragePassGroups_Size16_2))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize, 2, 1)];
			[enc endEncoding];
		}
		{ // ordered color composite into the current target texture
			auto &shader = _shaders.capaComposite;
			MSLCapaComposite::PcArgs pc{
				.clearColor=clearColor,
				.surfaceOffset=data.surfaceOffset,
				.flags=_flags | (clearDst ? kCAPA_FLAG_COMPOSITE_CLEAR_DST : 0)
					| (_opts.enableCAPAQuantizeCoverage ? kCAPA_FLAG_COMPOSITE_QUANTIZE_COVERAGE : 0)
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA composite";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setTexture:_outColorTex atIndex:shader.compute.dstImage];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:globalTiles.val offset:globalTiles.begin atIndex:shader.compute.globalTiles];
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			// See capa_composite.glsl binding=7 for z-linear coverage pages.
			[enc setBuffer:coverageTiles.val offset:coverageTiles.begin atIndex:shader.compute.coverageTiles];
			[enc setBuffer:gradientPaints.val offset:gradientPaints.begin atIndex:shader.compute.gradientPaints];
			[enc setBuffer:imagePaints.val offset:imagePaints.begin atIndex:shader.compute.imagePaints];
			[enc setBuffer:colors.val offset:colors.begin atIndex:shader.compute.colors];
			[enc setBuffer:positions.val offset:positions.begin atIndex:shader.compute.positions];
			[enc setBuffer:positions.val offset:positions.begin atIndex:shader.compute.positions];
			if (_clipState) {
				float *v = _clipState->bounds.begin.val;
				MSLCapaComposite::ClipStatBlock block{ IVec2(v[0], v[1]), _clipState->op };
				auto clipStat = makeBuffer(_cmdPack, &block, sizeof(block));
				[enc setBuffer:clipStat.val offset:clipStat.begin atIndex:shader.compute.clipStat];
				[enc setTexture:mtl_get_texture_from(_clipState->mask.get()) atIndex:shader.compute.clipTex];
			} else {
				[enc setBuffer:_mtlrender->_emptyBuffer offset:0 atIndex:shader.compute.clipStat];
			}
			auto imagesEncoder = _capaCompositeSet2Encoder;
			auto samplersEncoder = _capaCompositeSet3Encoder;
			auto imagesBuffer = _cmdPack.allocator->alloc(
				uint32_t(imagesEncoder.encodedLength), 0, uint32_t(imagesEncoder.alignment)
			);
			auto samplersBuffer = _cmdPack.allocator->alloc(
				uint32_t(samplersEncoder.encodedLength), 0, uint32_t(samplersEncoder.alignment)
			);
			[imagesEncoder setArgumentBuffer:imagesBuffer.val offset:imagesBuffer.begin];
			[samplersEncoder setArgumentBuffer:samplersBuffer.val offset:samplersBuffer.begin];

			for (uint32_t i = 0; i < data.imageSources.length(); i++) {
				auto texture = mtl_get_texture_from(data.imageSources[i].get());
				[imagesEncoder setTexture:texture atIndex:shader.compute.set2.images.id + i];
				[enc useResource:texture usage:MTLResourceUsageRead];
			}
			for (uint32_t i = 0; i < data.imageSamplers.length(); i++) {
				auto sampler = get_sampler(&data.imageSamplers[i]);
				[samplersEncoder setSamplerState:sampler atIndex:shader.compute.set3.samplers.id + i];
			}
			[enc setBuffer:imagesBuffer.val offset:imagesBuffer.begin atIndex:shader.compute.set2.bufferIndex];
			[enc setBuffer:samplersBuffer.val offset:samplersBuffer.begin atIndex:shader.compute.set3.bufferIndex];

			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, compositePassGroups_Size16_16))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize >> 1, kCAPATileSize >> 1, 1)];
			[enc endEncoding];
		}
		_cmdPack.recorded = true;
		return true;
	}
}
