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
	static const uint8_t F = 32; // coverage value for fail boundary tile
	static const uint8_t failBoundaryCoverage[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,F,F,F,F,F,F,F,F,0,0,0,
		0,0,0,F,F,F,F,F,F,F,F,F,F,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,F,F,F,F,F,F,F,0,0,0,0,
		0,0,0,F,F,F,F,F,F,F,F,F,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,F,F,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	};
	bool MetalCanvas::drawCAPACmd(cCAPADrawData &data) {
		auto edgeCount = data.edges.length();
		if (!edgeCount)
			return false;
		endPass();

		auto &budget = data.budget;
		auto pathCount = data.paths.length();
		auto env = _cmdPack.buffer->alloc<MSLCapaPrepare::CAPAEnvironment>(1);
		auto envData = (MSLCapaPrepare::CAPAEnvironment*)((char*)env.val.contents + env.begin);
		envData->globalTileBounds = IVec4(0x7fffffff, 0x7fffffff, -0x7fffffff, -0x7fffffff);
		envData->globalTileCount = 0;
		envData->taskCount = 0;
		envData->pathTileCount = 0;
		envData->pathTileRowCount = 0;
		envData->shortEdgeChunkCount = 0;
		envData->boundaryTileCount = 3;

		// copy data to GPU buffer
		auto paths = makeBuffer(_cmdPack, data.paths.val(), data.paths.size());
		auto edges = makeBuffer(_cmdPack, data.edges.val(), data.edges.size());
		auto shortTasks = _cmdPack.buffer->alloc<MSLCapaPrepare::CAPAShortEdgeTask>(budget.maxShortEdgeCount);
		auto globalTiles = _cmdPack.buffer->alloc<MSLCapaOrder::CAPAGlobalTile>(budget.globalTileCount);
		auto pathTiles = _cmdPack.buffer->alloc<MSLCapaOrder::CAPAPathTile>(budget.maxPathTileCount);
		auto boundaryTiles = _cmdPack.buffer->alloc<MSLCapaCoverage::CAPABoundaryTile>(budget.maxBoundaryTileCount);
		auto shortEdgeChunks = _cmdPack.buffer->alloc<MSLCapaBin::CAPAShortEdgeChunk>(budget.maxShortEdgeChunkCount);
		auto tileRows = _cmdPack.buffer->alloc<uint32_t>(budget.maxPathTileRowCount);
		// copy fail boundary tile to fail boundary tile slot (index 2)
		auto boundaryTilesData= (MSLCapaCoverage::CAPABoundaryTile*)((char*)boundaryTiles.val.contents + boundaryTiles.begin);
		memcpy(boundaryTilesData[2].coverage, failBoundaryCoverage, sizeof(failBoundaryCoverage));

		auto envIndirectOffset = [&](auto field) -> NSUInteger {
			return NSUInteger(env.begin + field);
		};

#if DEBUG
		Qk_DLog("Budget globalTileBounds: (%d, %d, %d, %d), "
						"globalTileCount: %d, "
						"maxShortEdgeCount: %d, "
						"maxPathTileCount: %d, "
						"maxPathTileRowCount: %d, "
						"maxShortEdgeChunkCount: %d, "
						"maxBoundaryTileCount: %d",
			budget.globalTileBounds.begin.x(),
			budget.globalTileBounds.begin.y(),
			budget.globalTileBounds.end.x(),
			budget.globalTileBounds.end.y(),
			budget.globalTileCount,
			budget.maxShortEdgeCount,
			budget.maxPathTileCount,
			budget.maxPathTileRowCount,
			budget.maxShortEdgeChunkCount,
			budget.maxBoundaryTileCount);
		[_cmdPack.current addCompletedHandler:^(MTLCommandBufferID buffer) {
			Qk_DLog("Real   globalTileBounds: (%d, %d, %d, %d), "
							"globalTileCount: %d, "
							"taskCount        : %d, "
							"pathTileCount   : %d, "
							"pathTileRowCount   : %d, "
							"shortEdgeChunkCount   : %d, "
							"boundaryTileCount   : %d",
				envData->globalTileBounds.x(),
				envData->globalTileBounds.y(),
				envData->globalTileBounds.z(),
				envData->globalTileBounds.w(),
				envData->globalTileCount,
				envData->taskCount,
				envData->pathTileCount,
				envData->pathTileRowCount,
				envData->shortEdgeChunkCount,
				envData->boundaryTileCount);
		}];
#endif

		{ // pass1 - prepare edge data
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
			[enc dispatchThreadgroups:MTLSizeMake((edgeCount + 63) >> 6, 1, 1)
					threadsPerThreadgroup:MTLSizeMake(64, 1, 1)];
			[enc endEncoding];
		}
		{ // pass1.1 - allocate path tiles and tile rows
			auto &shader = _shaders.capaPrepare1;
			MSLCapaPrepare1::PcArgs pc{
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
		{ // pass1.2 - write indirect dispatch group counts
			auto &shader = _shaders.capaPrepare2;
			MSLCapaPrepare2::PcArgs pc{
				.maxTaskCount=budget.maxShortEdgeCount,
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
		{ // pass2 - build ordered global-tile layer chains
			auto &shader = _shaders.capaOrder;
			MSLCapaOrder::PcArgs pc{
				.pathCount=pathCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA ordered tile chain";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:globalTiles.val offset:globalTiles.begin atIndex:shader.compute.globalTiles];
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, orderPassGroups_Size32))
				threadsPerThreadgroup:MTLSizeMake(32, 1, 1)];
			[enc endEncoding];
		}
		{ // pass3 - bin short-edge tasks into tile-local lists
			auto &shader = _shaders.capaBin;
			MSLCapaBin::PcArgs pc{
				.maxTaskCount=budget.maxShortEdgeCount,
				.maxShortEdgeChunkCount=budget.maxShortEdgeChunkCount,
				.maxBoundaryTileCount=budget.maxBoundaryTileCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA short-edge bin";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:edges.val offset:edges.begin atIndex:shader.compute.edges];
			[enc setBuffer:shortTasks.val offset:shortTasks.begin atIndex:shader.compute.shortEdgeTasks];
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:shortEdgeChunks.val offset:shortEdgeChunks.begin atIndex:shader.compute.shortEdgeChunks];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, binPassGroups_Size64))
				threadsPerThreadgroup:MTLSizeMake(64, 1, 1)];
			[enc endEncoding];
		}
		{ // pass3.1 - write boundary-tile indirect dispatch group counts
			auto &shader = _shaders.capaBin1;
			MSLCapaBin1::PcArgs pc{
				.maxBoundaryTileCount=budget.maxBoundaryTileCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA bin dispatch args";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc dispatchThreadgroups:MTLSizeMake(1, 1, 1)
				threadsPerThreadgroup:MTLSizeMake(1, 1, 1)];
			[enc endEncoding];
		}
		{ // pass4 - compute pathTile row backdrop
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
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:shortEdgeChunks.val offset:shortEdgeChunks.begin atIndex:shader.compute.shortEdgeChunks];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, backdropPassGroups_Size16_2))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize, 2, 1)];
			[enc endEncoding];
		}
		{ // pass5 - prefix/solid-empty classification
			auto &shader = _shaders.capaPrefix;
			MSLCapaPrefix::PcArgs pc{
				.maxPathTileRowCount=budget.maxPathTileRowCount,
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA prefix";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setBuffer:env.val offset:env.begin atIndex:shader.compute.env];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:tileRows.val offset:tileRows.begin atIndex:shader.compute.tileRows];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, prefixPassGroups_Size16_2))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize, 2, 1)];
			[enc endEncoding];
		}
		{ // pass6 - boundary pathTile coverage
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
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc setBuffer:shortEdgeChunks.val offset:shortEdgeChunks.begin atIndex:shader.compute.shortEdgeChunks];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, backdropPassGroups_Size16_2))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize, 2, 1)];
			[enc endEncoding];
		}
		{ // pass7 - ordered color composite into the current target texture
			auto &shader = _shaders.capaComposite;
			MSLCapaComposite::PcArgs pc{
				.clearColor=Vec4(0),
				.surfaceOffset={},
			};
			auto enc = [_cmdPack.current computeCommandEncoder];
			enc.label = @"CAPA composite";
			[enc setComputePipelineState:shader.getComputePipeline()];
			[enc setBytes:&pc length:sizeof(pc) atIndex:shader.compute.pc];
			[enc setTexture:_outColorTex atIndex:shader.compute.dstImage];
			[enc setBuffer:paths.val offset:paths.begin atIndex:shader.compute.paths];
			[enc setBuffer:globalTiles.val offset:globalTiles.begin atIndex:shader.compute.globalTiles];
			[enc setBuffer:pathTiles.val offset:pathTiles.begin atIndex:shader.compute.pathTiles];
			[enc setBuffer:boundaryTiles.val offset:boundaryTiles.begin atIndex:shader.compute.boundaryTiles];
			[enc dispatchThreadgroupsWithIndirectBuffer:env.val
				indirectBufferOffset:envIndirectOffset(offsetof(MSLCapaPrepare::CAPAEnvironment, compositePassGroups_Size16_16))
				threadsPerThreadgroup:MTLSizeMake(kCAPATileSize, kCAPATileSize, 1)];
			[enc endEncoding];
		}
		_cmdPack.recorded = true;
		return true;
	}
}
