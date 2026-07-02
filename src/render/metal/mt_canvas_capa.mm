/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#import "./mtl_canvas.h"
#include "src/render/render.h"
#import "./mtl_render.h"

namespace qk {
	const MemBlockAllocator<MTLBufferID>::MemBlock& makeBuffer(MTL_CmdPack &cmd, const void *bytes, uint32_t length);
	bool MetalCanvas::drawCAPACmd(cCAPADrawData &data) {
		auto edgeCount = data.edges.length();
		if (!edgeCount)
			return false;
		endPass();

		auto &budget = data.budget;
		auto pathCount = data.paths.length();
		// The CPU allocates conservative pools once. GPU passes publish real
		// counts into env and then use indirect dispatch for the dependent passes.
		auto env = _cmdPack.buffer->alloc<MSLCapaPrepare::CAPAEnvironment>(1);
		auto envData = (MSLCapaPrepare::CAPAEnvironment*)((char*)env.val.contents + env.begin);
		envData->globalTileBounds = IVec4(0x7fffffff, 0x7fffffff, -0x7fffffff, -0x7fffffff);
		envData->globalTileCount = 0;
		envData->taskCount = 0;
		envData->pathTileCount = 0;
		envData->pathTileRowCount = 0;
		envData->boundaryTileCount = 0;
		envData->boundaryDoneCount = 0;
		envData->layerPlanPathTileCount = 0;

		// Upload path metadata and path-space edges. The remaining buffers are
		// GPU-owned staging/final pools for the 12-pass CAPA pipeline.
		auto paths = makeBuffer(_cmdPack, data.paths.val(), data.paths.size());
		auto edges = makeBuffer(_cmdPack, data.edges.val(), data.edges.size());
		auto shortTasks = _cmdPack.buffer->alloc<MSLCapaPrepare::CAPAShortEdgeTask>(budget.maxShortEdgeCount);
		auto shortEdges = _cmdPack.buffer->alloc<MSLCapaBin::CAPAShortEdgeNode>(budget.maxShortEdgeCount * 3);
		auto globalTiles = _cmdPack.buffer->alloc<MSLCapaLayerPlan::CAPAGlobalTile>(budget.globalTileCount);
		auto pathTiles = _cmdPack.buffer->alloc<MSLCapaLayerPlan::CAPAPathTile>(budget.maxPathTileCount);
		auto smallTiles = _cmdPack.buffer->alloc<MSLCapaTile::CAPASmallTile>(budget.maxPathTileCount);
		auto boundaryTiles = _cmdPack.buffer->alloc<MSLCapaCoverage::CAPABoundaryTile>(budget.maxBoundaryTileCount);
		auto coverageTiles = _cmdPack.buffer->alloc<MSLCapaCoverage::CAPACoverageTile>(budget.maxBoundaryTileCount);
		auto tileRows = _cmdPack.buffer->alloc<MSLCapaPrepareTiles::CAPAPathTileRow>(budget.maxPathTileRowCount);
		auto gradientPaints = data.gradientPaints.length()
			? makeBuffer(_cmdPack, data.gradientPaints.val(), data.gradientPaints.size())
			: _cmdPack.buffer->alloc(0);
		auto imagePaints = data.imagePaints.length()
			? makeBuffer(_cmdPack, data.imagePaints.val(), data.imagePaints.size())
			: _cmdPack.buffer->alloc(0);
		auto colors = data.colors.length()
			? makeBuffer(_cmdPack, data.colors.val(), data.colors.size())
			: _cmdPack.buffer->alloc(0);
		auto positions = data.positions.length()
			? makeBuffer(_cmdPack, data.positions.val(), data.positions.size())
			: _cmdPack.buffer->alloc(0);

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
			MSLCapaBin::PcArgs pc{
				.maxTaskCount=budget.maxShortEdgeCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA short-edge bin";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
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
			MSLCapaBackdrop::PcArgs pc{
				.maxBoundaryTileCount=budget.maxBoundaryTileCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA backdrop";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
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
			MSLCapaClassify::PcArgs pc{
				.maxPathTileRowCount=budget.maxPathTileRowCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA classify";
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
			MSLCapaPrefix::PcArgs pc{
				.maxPathTileRowCount=budget.maxPathTileRowCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA prefix";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
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
			MSLCapaCoverage::PcArgs pc{
				.maxBoundaryTileCount=budget.maxBoundaryTileCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA coverage";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
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
				.clearColor=Vec4(0),
				.surfaceOffset={},
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
			// See capa_composite.glsl binding=5 for z-linear coverage pages.
			[enc setBuffer:coverageTiles.val offset:coverageTiles.begin atIndex:5];
			[enc setBuffer:gradientPaints.val offset:gradientPaints.begin atIndex:6];
			[enc setBuffer:imagePaints.val offset:imagePaints.begin atIndex:7];
			[enc setBuffer:colors.val offset:colors.begin atIndex:8];
			[enc setBuffer:positions.val offset:positions.begin atIndex:9];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, compositePassGroups_Size16_16))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize >> 1, kCAPATileSize >> 1, 1)];
			[enc endEncoding];
		}
		_cmdPack.recorded = true;
		return true;
	}
}
