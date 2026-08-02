/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./vk_canvas.h"
#include "./vk_render.h"

namespace qk {

	struct VulkanCanvas::CAPABuffer {
		VkShaderBinding *binding;
		cVkMemBlock *buffer;
	};

	static void vk_capa_compute_barrier(VkCommandBuffer cmd) {
		VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
		barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT |
			VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
			0, 1, &barrier, 0, nullptr, 0, nullptr);
	}

	void VulkanCanvas::bindCAPABuffers(VkShader &shader,
		const CAPABuffer *buffers, uint32_t count)
	{
		auto set = allocDescriptorSet(shader.sets(0));
		Array<VkDescriptorBufferInfo> infos(&_cmdPack->allocator);
		infos.extend(count);
		Array<VkWriteDescriptorSet> writes(&_cmdPack->allocator);
		writes.extend(count);
		for (uint32_t i = 0; i < count; i++) {
			auto &block = *buffers[i].buffer;
			infos[i].buffer = block.val->buffer;
			infos[i].offset = block.begin;
			// A descriptor range cannot be zero. Empty CAPA tables are never
			// indexed by the shader. makeBufferT() reserves one element at
			// block.begin, so expose a valid byte without allocating fake data.
			infos[i].range = U32::max(1, block.end - block.begin);
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].pNext = nullptr;
			writes[i].dstSet = set;
			writes[i].dstBinding = buffers[i].binding->binding;
			writes[i].dstArrayElement = 0;
			writes[i].descriptorCount = 1;
			writes[i].descriptorType = buffers[i].binding->descriptorType;
			writes[i].pImageInfo = nullptr;
			writes[i].pBufferInfo = &infos[i];
			writes[i].pTexelBufferView = nullptr;
		}
		vkUpdateDescriptorSets(_device, count, writes.val(), 0, nullptr);
		bindDescriptorSet(set, shader, 0, 0, VK_PIPELINE_BIND_POINT_COMPUTE);
	}

	void VulkanCanvas::useComputePipeline(VkShader &shader) {
		auto pipeline = shader.getComputePipeline();
		Qk_ASSERT(pipeline, "Vulkan CAPA compute pipeline is unavailable: %s",
			shader.source.name);
		vkCmdBindPipeline(_cmdPack->current, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	}

	bool VulkanCanvas::drawCAPACmd(CAPADrawData &data) {
		auto edgeCount = data.edges.length();
		if (!edgeCount)
			return false;
		Qk_ASSERT(_target->usage & VK_IMAGE_USAGE_STORAGE_BIT,
				"Vulkan CAPA target must support storage image writes");
		Qk_ASSERT_EQ(_target->format, VK_FORMAT_R8G8B8A8_UNORM,
			"Vulkan CAPA target must use VK_FORMAT_R8G8B8A8_UNORM");
		Color4f clearColor;
		bool clearDst = onlyEndEncoderPass(clearColor);

		auto cmd = _cmdPack->current;
		auto &budget = data.budget;
		auto pathCount = data.paths.length();
		// The CPU allocates conservative pools once. GPU passes publish real
		// counts into env and then use indirect dispatch for the dependent passes.
		auto env = _cmdPack->alloc<SpvCapaPrepare::CAPAEnvironment>(1);
		auto envData = (SpvCapaPrepare::CAPAEnvironment*)((char*)env.val->mapped + env.begin);
		envData->globalTileBounds = IVec4(0x7fffffff, 0x7fffffff, -0x7fffffff, -0x7fffffff);
		envData->globalTileCount = 0;
		envData->taskCount = 0;
		envData->pathTileCount = 0;
		envData->pathTileRowCount = 0;
		envData->boundaryTileCount = 0;
		envData->boundaryDoneCount = 0;
		envData->layerPlanPathTileCount = 0;

		if (clearDst) {
			auto surfaceSize = _state->output ? _state->output->size(): _surfaceSize;
			budget.globalBounds = Range{{0, 0}, surfaceSize};
			budget.globalTileBounds = IRange{
				capa_floor_tile_origin(budget.globalBounds.begin),
				capa_ceil_tile_end(budget.globalBounds.end),
			};
			envData->globalTileBounds = budget.globalTileBounds.iVec4();
			auto tileSpan = budget.globalTileBounds.size();
			budget.globalTileCount = tileSpan.x() * tileSpan.y() * kCAPABudgetMultiplier;
		}
		// Upload path metadata and path-space edges. The remaining buffers are
		// GPU-owned staging/final pools for the 12-pass CAPA pipeline.
		auto paths = makeBufferT(_cmdPack, data.paths.val(), data.paths.length());
		auto edges = makeBufferT(_cmdPack, data.edges.val(), data.edges.length());
		auto gradientPaints = makeBufferT(_cmdPack, data.gradientPaints.val(), data.gradientPaints.length());
		auto imagePaints = makeBufferT(_cmdPack, data.imagePaints.val(), data.imagePaints.length());
		auto colors = makeBufferT(_cmdPack, data.colors.val(), data.colors.length());
		auto positions = makeBufferT(_cmdPack, data.positions.val(), data.positions.length());
		// allocate budget space for the CAPA pipeline
		auto shortTasks = _cmdPack->alloc<SpvCapaPrepare::CAPAShortEdgeTask>(budget.maxShortEdgeCount, 1);
		auto shortEdges = _cmdPack->alloc<SpvCapaBin::CAPAShortEdgeNode>(budget.maxShortEdgeCount * 3, 1);
		auto globalTiles = _cmdPack->alloc<SpvCapaLayerPlan::CAPAGlobalTile>(budget.globalTileCount, 1);
		auto pathTiles = _cmdPack->alloc<SpvCapaLayerPlan::CAPAPathTile>(budget.maxPathTileCount, 1);
		auto smallTiles = _cmdPack->alloc<SpvCapaTile::CAPASmallTile>(budget.maxPathTileCount, 1);
		auto boundaryTiles = _cmdPack->alloc<SpvCapaCoverage::CAPABoundaryTile>(budget.maxBoundaryTileCount, 1);
		auto coverageTiles = _cmdPack->alloc<SpvCapaCoverage::CAPACoverageTile>(budget.maxBoundaryTileCount, 1);
		auto tileRows = _cmdPack->alloc<SpvCapaPrepareTiles::CAPAPathTileRow>(budget.maxPathTileRowCount, 1);

		auto dispatchIndirect = [&](uint32_t offset) {
			vkCmdDispatchIndirect(cmd, env.val->buffer, env.begin + offset);
			vk_capa_compute_barrier(cmd);
		};

		{ // prepare edge data
			auto &shader = _shaders.capaPrepare;
			SpvCapaPrepare::PcArgs pc{
				.shortEdgeLength=kCAPAShortEdgeLength,
				.edgeCount=edgeCount,
				.maxTaskCount=budget.maxShortEdgeCount,
			};
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.edges, &edges},
				{&shader.shortEdgeTasks, &shortTasks},
			};
			bindCAPABuffers(shader, buffers, 4);
			pushConstants(shader, &pc);
			vkCmdDispatch(cmd, (edgeCount + 31) >> 5, 1, 1);
			vk_capa_compute_barrier(cmd);
		}
		{ // prepare path tiles and tile rows
			auto &shader = _shaders.capaPrepareTiles;
			SpvCapaPrepareTiles::PcArgs pc{
				.pathCount=pathCount,
				.maxPathTileCount=budget.maxPathTileCount,
				.maxPathTileRowCount=budget.maxPathTileRowCount,
			};
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.tileRows, &tileRows},
				{&shader.imagePaints, &imagePaints},
			};
			bindCAPABuffers(shader, buffers, 4);
			pushConstants(shader, &pc);
			vkCmdDispatch(cmd, (pathCount + 31) >> 5, 1, 1);
			vk_capa_compute_barrier(cmd);
		}
		{ // prepare indirect dispatch counts
			auto &shader = _shaders.capaPrepareDispatch;
			SpvCapaPrepareDispatch::PcArgs pc{
				.maxTaskCount=budget.maxShortEdgeCount,
				.maxPathTileCount=budget.maxPathTileCount,
				.maxPathTileRowCount=budget.maxPathTileRowCount,
			};
			useComputePipeline(shader);
			CAPABuffer buffers[] = {{&shader.env, &env}};
			bindCAPABuffers(shader, buffers, 1);
			pushConstants(shader, &pc);
			vkCmdDispatch(cmd, 1, 1, 1);
			vk_capa_compute_barrier(cmd);
		}
		{ // initialize small tiles
			auto &shader = _shaders.capaTile;
			SpvCapaTile::PcArgs pc{.maxPathTileCount=budget.maxPathTileCount};
			useComputePipeline(shader);
			CAPABuffer buffers[] = {{&shader.smallTiles, &smallTiles}};
			bindCAPABuffers(shader, buffers, 1);
			pushConstants(shader, &pc);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, tilePassGroups_Size32));
		}
		{ // bin short edges
			auto &shader = _shaders.capaBin;
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.edges, &edges},
				{&shader.shortEdges, &shortEdges}, {&shader.shortEdgeTasks, &shortTasks},
				{&shader.smallTiles, &smallTiles},
			};
			bindCAPABuffers(shader, buffers, 6);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, binPassGroups_Size32));
		}
		{ // allocate boundary tiles
			auto &shader = _shaders.capaBoundary;
			SpvCapaBoundary::PcArgs pc{.maxBoundaryTileCount=budget.maxBoundaryTileCount};
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.smallTiles, &smallTiles},
				{&shader.boundaryTiles, &boundaryTiles}, {&shader.tileRows, &tileRows},
			};
			bindCAPABuffers(shader, buffers, 5);
			pushConstants(shader, &pc);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, classifyPassGroups_Size32));
		}
		{ // boundary row backdrop
			auto &shader = _shaders.capaBackdrop;
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.tileRows, &tileRows}, {&shader.paths, &paths},
				{&shader.shortEdges, &shortEdges}, {&shader.boundaryTiles, &boundaryTiles},
			};
			bindCAPABuffers(shader, buffers, 5);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, backdropPassGroups_Size16_2));
		}
		{ // classify edge-free tiles
			auto &shader = _shaders.capaClassify;
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.smallTiles, &smallTiles},
				{&shader.boundaryTiles, &boundaryTiles}, {&shader.tileRows, &tileRows},
			};
			bindCAPABuffers(shader, buffers, 5);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, classifyPassGroups_Size32));
		}
		{ // build layer plans
			auto &shader = _shaders.capaLayerPlan;
			SpvCapaLayerPlan::PcArgs pc{.pathCount=pathCount};
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.globalTiles, &globalTiles},
				{&shader.pathTiles, &pathTiles}, {&shader.smallTiles, &smallTiles},
				{&shader.coverageTiles, &coverageTiles},
			};
			bindCAPABuffers(shader, buffers, 6);
			pushConstants(shader, &pc);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, layerPlanPassGroups_Size32));
		}
		{ // boundary row prefix
			auto &shader = _shaders.capaPrefix;
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.tileRows, &tileRows}, {&shader.paths, &paths},
				{&shader.boundaryTiles, &boundaryTiles},
			};
			bindCAPABuffers(shader, buffers, 4);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, prefixPassGroups_Size16_2));
		}
		{ // retained boundary coverage
			auto &shader = _shaders.capaCoverage;
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.shortEdges, &shortEdges},
				{&shader.boundaryTiles, &boundaryTiles}, {&shader.coverageTiles, &coverageTiles},
			};
			bindCAPABuffers(shader, buffers, 5);
			dispatchIndirect(offsetof(SpvCapaPrepare::CAPAEnvironment, coveragePassGroups_Size16_2));
		}

		{ // ordered composite into the current target
			auto &shader = _shaders.capaComposite;
			SpvCapaComposite::PcArgs pc{
				.clearColor=clearColor,
				.surfaceOffset=data.surfaceOffset,
				.flags=_flags | (clearDst ? kCAPA_FLAG_COMPOSITE_CLEAR_DST: 0) |
					(_opts.enableCAPAQuantizeCoverage ?
						kCAPA_FLAG_COMPOSITE_QUANTIZE_COVERAGE: 0),
			};
			SpvCapaComposite::ClipStatBlock clip{};
			auto clipTex = _emptyR8Texture.get();
			if (_clipState) {
				clip = { _clipState->bounds.iBegin(), _clipState->op };
				clipTex = vk_get_texture(_clipState->mask.get());
			}
			clipTex->transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			_cmdPack->ref(clipTex);
			auto clipStat = makeBufferT(_cmdPack, &clip, 1);

			_target->transitionLayout(cmd, VK_IMAGE_LAYOUT_GENERAL, _cmdPack->level);
			_cmdPack->ref(_target);
			useComputePipeline(shader);
			CAPABuffer buffers[] = {
				{&shader.env, &env}, {&shader.paths, &paths}, {&shader.globalTiles, &globalTiles},
				{&shader.pathTiles, &pathTiles}, {&shader.coverageTiles, &coverageTiles},
				{&shader.gradientPaints, &gradientPaints}, {&shader.imagePaints, &imagePaints},
				{&shader.colors, &colors}, {&shader.positions, &positions},
				{&shader.clipStat, &clipStat},
			};
			bindCAPABuffers(shader, buffers, 10);

			auto set1 = allocDescriptorSet(shader.sets(1));
			VkDescriptorImageInfo images[2]{{
				.sampler = _resource->nearestSampler(),
				.imageView = clipTex->view,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			},{
				.imageView = _target->levelView(_cmdPack->level),
				.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
			}};
			VkWriteDescriptorSet imageWrites[2];
			VkShaderBinding *imageBindings[] = {&shader.clipTex, &shader.dstImage};
			for (uint32_t i = 0; i < 2; i++) {
				imageWrites[i] = {
					.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = set1,
					.dstBinding = imageBindings[i]->binding,
					.descriptorCount = 1,
					.descriptorType = imageBindings[i]->descriptorType,
					.pImageInfo = images + i,
				};
			}
			vkUpdateDescriptorSets(_device, 2, imageWrites, 0, nullptr);
			bindDescriptorSet(set1, shader, 1, 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			auto imageCount = data.imageSources.length();
			auto set2 = allocDescriptorSet(shader.sets(2), imageCount);
			if (imageCount) {
				Array<VkDescriptorImageInfo> imageInfos(&_cmdPack->allocator);
				imageInfos.extend(imageCount);
				for (uint32_t i = 0; i < imageCount; i++) {
					auto tex = vk_get_texture(data.imageSources[i].get());
					tex->transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
					imageInfos[i] = {.imageView=tex->view,
						.imageLayout=VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
					_cmdPack->ref(tex);
				}
				VkWriteDescriptorSet imageWrite{
					.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = set2,
					.dstBinding = shader.images.binding,
					.descriptorCount = imageCount,
					.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
					.pImageInfo = imageInfos.val(),
				};
				vkUpdateDescriptorSets(_device, 1, &imageWrite, 0, nullptr);
			}
			bindDescriptorSet(set2, shader, 2, 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			auto samplerCount = data.imageSamplers.length();
			auto set3 = allocDescriptorSet(shader.sets(3), samplerCount);
			if (samplerCount) {
				Array<VkDescriptorImageInfo> samplerInfos(&_cmdPack->allocator);
				samplerInfos.extend(samplerCount);
				for (uint32_t i = 0; i < samplerCount; i++)
					samplerInfos[i] = {.sampler = get_sampler(&data.imageSamplers[i])};
				VkWriteDescriptorSet samplerWrite{
					.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
					.dstSet = set3,
					.dstBinding = shader.samplers.binding,
					.descriptorCount = samplerCount,
					.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
					.pImageInfo = samplerInfos.val(),
				};
				vkUpdateDescriptorSets(_device, 1, &samplerWrite, 0, nullptr);
			}
			bindDescriptorSet(set3, shader, 3, 0, VK_PIPELINE_BIND_POINT_COMPUTE);

			pushConstants(shader, &pc);
			vkCmdDispatchIndirect(cmd, env.val->buffer, env.begin +
				offsetof(SpvCapaPrepare::CAPAEnvironment, compositePassGroups_Size16_16));
			// Keep a following CAPA batch or graphics pass from observing the
			// destination before this composite dispatch has finished writing it.
			vk_capa_compute_barrier(cmd);
		}

		_cmdPack->recorded = true;
		return true;
	}

} // namespace qk
