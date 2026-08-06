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

#include "./vk_canvas.h"
#include "./vk_render.h"

namespace qk {
	void clear_PathvCache(PathvCache *cache, int flags);

	VkDescriptorPool VkDescriptorPools::createDescriptorPool(VkDevice device) {
		VkDescriptorPoolSize sizes[] = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2048},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 16384},
			{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 2048},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4096},
			{VK_DESCRIPTOR_TYPE_SAMPLER, 4096},
		};
		VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
		info.maxSets = 2048;
		info.poolSizeCount = sizeof(sizes) / sizeof(sizes[0]);
		info.pPoolSizes = sizes;
		VkDescriptorPool pool = VK_NULL_HANDLE;
		vk_check("vkCreateDescriptorPool", vkCreateDescriptorPool(device, &info, nullptr, &pool));
		return pool;
	}

	VkDescriptorSet VkDescriptorPools::allocDescriptorSet(VkDevice device,
		VkDescriptorSetLayout setLayout, uint32_t *variableCount
	) {
		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT variableInfo{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT};
		variableInfo.descriptorSetCount = 1;
		variableInfo.pDescriptorCounts = variableCount;
		VkDescriptorSetAllocateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		info.pNext = variableCount ? &variableInfo: nullptr;
		info.descriptorPool = *iter;
		info.descriptorSetCount = 1;
		info.pSetLayouts = &setLayout;
		VkDescriptorSet set = VK_NULL_HANDLE;
		auto result = vkAllocateDescriptorSets(device, &info, &set);

		if (result == VK_ERROR_FRAGMENTED_POOL || result == VK_ERROR_OUT_OF_POOL_MEMORY) {
			iter++;
			if (iter == pools.end())
				iter = pools.pushBack(createDescriptorPool(device));
			info.descriptorPool = *iter;
			result = vkAllocateDescriptorSets(device, &info, &set);
		}
		Qk_ASSERT_EQ(result, VK_SUCCESS, "Failed to allocate Vulkan descriptor set");
		return set;
	}

	void VkDescriptorPools::reset(VkDevice device) {
		auto end = pools.end();
		while (iter != end) {
			Qk_ASSERT_EQ(VK_SUCCESS, vkResetDescriptorPool(device, *iter, 0),
				"Failed to reset Vulkan descriptor pool");
			iter--;
		}
		iter = pools.begin();
	}

	void VkDescriptorPools::destroy(VkDevice device) {
		for (auto &pool: pools)
			vkDestroyDescriptorPool(device, pool, nullptr);
		pools.clear();
		iter = pools.begin();
	}

	// -----------------------------------------------------------------------

	VkCmdPack::VkCmdPack(VkDevice device) {
		descPools.pools.pushBack(descPools.createDescriptorPool(device));
		descPools.iter = descPools.pools.begin();
		refs = Array<VkRef*>(&allocator);
		completeCallbacks = Array<Cb>(&allocator);
		subCanvas = Array<Sp<VulkanCanvas>>(&allocator);
		commands = Array<VkCommandBuffer>(&allocator);
		ownCommands = Array<VkCommandBuffer>();
	}

	VkCmdPack::~VkCmdPack() {
		auto device = getSharedRenderVulkanResource()->device();
		descPools.destroy(device);
		if (auto submit = completion.load(std::memory_order_acquire))
			submit->unref();
		finish();
	}

	void VkCmdPack::clearAllocator() {
		addCompleteCallback(Cb([](auto, auto self) {
			self->vkAllocator[0].clear();
			self->vkAllocator[1].clear();
		}, this));
	}

	void VkCmdPack::reset(VulkanCanvas *h, Lock* lock, bool finished) {
		commands.clear();
		set0 = VK_NULL_HANDLE;
		pipeline = VK_NULL_HANDLE;
		renderPass = VK_NULL_HANDLE;
		beginPass = false;
		recorded = false;
		commonSetDirty = false;
		vkAllocator[0].reset();
		vkAllocator[1].reset();

		if (finished)
			allocator.reset(); // reset linear allocator if finished, otherwise keep the memory
		if (lock)
			lock->unlock();

		nextIdx = 0; // reset next command buffer index
		h->beginNextCommand(this);

		descPools.reset(h->_device);
		subCanvas.clear();

		if (finished)
			finish();
	}

	void VkCmdPack::finish() {
		// Qk_DLog("VkCmdPack::finish() completeCallbacks: %d, refs: %d",
		// 	completeCallbacks.length(), refs.length());
		for (auto &cb: completeCallbacks)
			cb->resolve();
		completeCallbacks.clear();
		for (auto &ref: refs)
			ref->unref();
		refs.clear();
	}

	// -----------------------------------------------------------------------

	VulkanCanvas::VulkanCanvas(VulkanRender *render, Render::Options opts)
		: GPUCanvas(render, opts)
		, _vkRender(render)
		, _resource(getSharedRenderVulkanResource())
		, _device(_resource->device())
		, _commandPool(VK_NULL_HANDLE)
		, _target(nullptr)
		, _outTex(nullptr)
		, _emptyTexture(nullptr), _emptyR8Texture(nullptr)
		, _cmdPack(nullptr), _cmdPackFront(nullptr)
		, _shaders(_resource->shaders()) // copy shared shader resource
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kRGBA_8888_ColorType;
		auto format = vk_pixelFormat(_opts.colorType);
		Qk_ASSERT_NE(format, VK_FORMAT_UNDEFINED, "Invalid Vulkan canvas color format");

		VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = _resource->queueFamily();
		auto result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);
		Qk_ASSERT_EQ(result, VK_SUCCESS, "Failed to create Vulkan canvas command pool");

		_cmdPack = new VkCmdPack(_device);
		_cmdPack->reset(this);
		_cmdPackFront = new VkCmdPack(_device);
		_emptyTexture = _resource->newTexture(Vec2(1), kRGBA_8888_ColorType);
		_emptyR8Texture = _resource->newTexture(Vec2(1), kLuminance_8_ColorType);

		if (opts.enableCAPA && _resource->capaSupport()) {
			_capaMaxImageCount = _resource->capaMaxImageCount();
			_capaBuilder = new CAPABuilder(this);
		}
	}

	VulkanCanvas::~VulkanCanvas() {
		ScopeLock lock(_mutex);
		_resource->queueWaitIdle();
		for (auto &i: _renderPasss)
			vkDestroyRenderPass(_device, i.second, nullptr);
		vkDestroyCommandPool(_device, _commandPool, nullptr);
		Releasep(_cmdPack);
		Releasep(_cmdPackFront);
		_outTex = nullptr;
		_emptyTexture = nullptr;
		_emptyR8Texture = nullptr;
		_commandPool = VK_NULL_HANDLE;
		_device = VK_NULL_HANDLE;
	}

	void VulkanCanvas::beginNextCommand(VkCmdPack *pack) {
		if (pack->nextIdx >= pack->ownCommands.length())
			pack->ownCommands.push(VK_NULL_HANDLE);
		Qk_ASSERT_EQ(VK_SUCCESS,
			vk_beginCommandBuffer(_device, _commandPool, &pack->ownCommands[pack->nextIdx]),
			"Failed to begin Vulkan command buffer");
		pack->current = pack->ownCommands[pack->nextIdx++];
	}

	VkRenderPass VulkanCanvas::getRenderPass(
		VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
		VkImageLayout initialLayout, VkImageLayout finalLayout
	) {
		Qk_ASSERT(uint32_t(format) < VK_FORMAT_MAX_ENUM &&
			uint32_t(loadOp) <= 0xff &&
			uint32_t(storeOp) <= 0xff &&
			uint32_t(initialLayout) <= 0xff &&
			uint32_t(finalLayout) <= 0xff,
			"Vulkan render pass cache key parameter overflow");
		uint64_t key = uint64_t(format) |
			uint64_t(loadOp) << 32 |
			uint64_t(storeOp) << 40 |
			uint64_t(initialLayout) << 48 |
			uint64_t(finalLayout) << 56;
		VkRenderPass renderPass;
		if (_renderPasss.get(key, renderPass)) {
			return renderPass;
		}
		renderPass = vk_create_render_pass(_device, format, loadOp, storeOp, initialLayout, finalLayout);
		return _renderPasss.set(key, renderPass);
	}

	VkSampler VulkanCanvas::get_sampler(const PaintImage* paint) {
		uint32_t key = vk_sampler_key(paint);
		VkSampler sampler;
		if (!_texSamplers.get(key, sampler)) {
			sampler = _resource->get_sampler(paint);
			_texSamplers.set(key, sampler);
		}
		return sampler;
	}

	VkSampler VulkanCanvas::get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap) {
		PaintImage img;
		img.tileModeX = PaintImage::kDecal_TileMode;
		img.tileModeY = PaintImage::kDecal_TileMode;
		img.filterMode = filter;
		img.mipmapMode = mipmap;
		return get_sampler(&img);
	}

	void VulkanCanvas::setRootMatrixBuffer() {
		Qk_ASSERT(_cmdPack->renderPass, "Vulkan render pass should be begun before setting root matrix buffer");
		SpvColor::RootMatrixBlock root{
			.value = _rootMatrix.transpose(),
			.noScale = _rootMatrixNoScale.transpose(),
			.surfaceScale = _surfaceScale,
		};
		auto oldBuff = _cmdPack->buffers[0];
		_cmdPack->buffers[0] = makeBufferInfoT(_cmdPack, &root);
		if (_cmdPack->buffers[0].buffer != oldBuff.buffer)
			updateCommonDescriptorSet(false); // update descriptor set
		_cmdPack->commonSetDirty = true;
	}

	void VulkanCanvas::setViewMatrixBuffer() {
		Qk_ASSERT(_cmdPack->renderPass, "Vulkan render pass should be begun before setting view matrix buffer");
		auto vMat = _state->matrix.val;
		SpvColor::ViewMatrixBlock view{ .value = Vec4(vMat[0], vMat[3], vMat[1], vMat[4]) };
		auto oldBuff = _cmdPack->buffers[1].buffer;
		_cmdPack->buffers[1] = makeBufferInfoT(_cmdPack, &view);
		if (_cmdPack->buffers[1].buffer != oldBuff)
			updateCommonDescriptorSet(false); // update descriptor set
		_cmdPack->commonSetDirty = true;
	}

	VkDescriptorSet VulkanCanvas::useTexture0(VkDescriptorSet set, const PaintImage *paint, int dstSlot, bool* isYuv) {
		if (paint->_isCanvas) { // flush canvas to current canvas
			auto srcC = static_cast<VulkanCanvas*>(paint->canvas);
			if (srcC == this || !srcC->isGpu())
				return VK_NULL_HANDLE; // if the source canvas is the same as current canvas or not gpu, skip
			if (srcC->render() != _vkRender)
				return VK_NULL_HANDLE; // if the source canvas is not from the same render, skip
			if (!srcC->_outTex)
				return VK_NULL_HANDLE; // if the source canvas has no output texture, skip
			flushSubcanvasCmd(srcC); // flush subcanvas to current canvas
			makeTextureMipReadable(srcC->_outTex.get());
			setTextureParam(set, dstSlot, srcC->_outTex.get(), get_sampler(paint));
			return set;
		} else {
			if (kYUV420P_Y_8_ColorType == paint->image->type()) { // yuv420p or yuv420sp
				set = allocDescriptorSet(_shaders.imageYuv.sets(1));
				if (isYuv) *isYuv = true;
			}
			if (useTexture(set, paint->image, 0, dstSlot, paint))
				return set;
			return VK_NULL_HANDLE;
		}
	}

	bool VulkanCanvas::useTexture(VkDescriptorSet set, ImageSource *src, int srcSlot, int dstSlot,
			const PaintImage *paint) {
		auto index = paint->srcIndex + srcSlot;
		Qk_ASSERT_LT(index, 8, "Texture slot index out of range, srcIndex: %d, slot: %d", paint->srcIndex, srcSlot);
		// mark texture for this render, and try to create texture immediately
		src->markAsTexture();
		if (!src->texture(index)->ptr()) {
			Qk_DLog("Texture not ready for paint image, src index: %d, slot: %d", paint->srcIndex, srcSlot);
			return false;
		}
		setTextureParam(set, dstSlot, vk_get_texture(src, index), get_sampler(paint));
		return true;
	}

	void VulkanCanvas::setTextureParam(VkDescriptorSet set, uint32_t binding,
			VkTexture *tex, VkSampler sampler, uint32_t level) {
		VkDescriptorImageInfo image{sampler ? sampler: _resource->nearestSampler()};
		image.imageView = tex->levelView(level);
		image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstSet = set;
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &image;
		vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
		_cmdPack->ref(tex);
	}

	void VulkanCanvas::makeTextureMipReadable(VkTexture *tex, uint32_t level) {
		Qk_ASSERT(!_cmdPack->renderPass, "Texture layout transition must be outside render pass");
		if (tex->transitionLayout(_cmdPack->current, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, level))
			_cmdPack->recorded = true;
	}

	void VulkanCanvas::setBuffer(VkDescriptorSet set, uint32_t binding, cVkMemBlock& buffer) {
		VkDescriptorBufferInfo info{buffer.val->buffer};
		info.offset = buffer.begin;
		info.range = buffer.end - buffer.begin;
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstSet = set;
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		write.pBufferInfo = &info;
		vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
	}

	void VulkanCanvas::bindDescriptorSet(VkDescriptorSet set, VkShader& shader,
		uint32_t setIdx, uint32_t dynamicOffsetCount, VkPipelineBindPoint bindPoint)
	{
		static constexpr uint32_t dynamicOffsets[32]{0};
		Qk_ASSERT(dynamicOffsetCount < 32, "Vulkan descriptor set bind index overflow");
		vkCmdBindDescriptorSets(_cmdPack->current, bindPoint, shader.layout(),
			setIdx, 1, &set, dynamicOffsetCount, dynamicOffsetCount ? dynamicOffsets: nullptr);
	}

	void VulkanCanvas::updateCommonDescriptorSet(bool allocBuff) {
		Qk_ASSERT(_cmdPack->renderPass, "Vulkan render pass should be begun before updating common descriptor set");
		_cmdPack->set0 = allocDescriptorSet(_shaders.color.sets(0));
		// Update image sampler descriptor sets
		VkWriteDescriptorSet writes[4]{};
		VkDescriptorImageInfo image{_resource->nearestSampler()};
		// auto clipTex = _emptyTexture.get();
		auto clipTex = _clipState ? vk_get_texture(_clipState->mask.get()): _emptyTexture.get();
		clipTex->transitionLayout(_cmdPack->current, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		_cmdPack->ref(clipTex); // ref clip texture for this pack
		image.imageView = clipTex->view;
		image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = _cmdPack->set0;
		writes[0].dstBinding = 0;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[0].pImageInfo = &image;

		// Update uniform buffer descriptor sets
		if (allocBuff) {
			auto vMat = _state->matrix.val;
			SpvColor::RootMatrixBlock root{
				.value = _rootMatrix.transpose(),
				.noScale = _rootMatrixNoScale.transpose(),
				.surfaceScale = _surfaceScale,
			};
			SpvColor::ViewMatrixBlock view{
				.value = Vec4(vMat[0], vMat[3], vMat[1], vMat[4]),
			};
			SpvColor::ClipStatBlock clip{};
			if (_clipState) {
				clip.bounds = _clipState->bounds.iVec4();
				clip.op = _clipState->op;
			}
			_cmdPack->buffers[0] = makeBufferInfoT(_cmdPack, &root);
			_cmdPack->buffers[1] = makeBufferInfoT(_cmdPack, &view);
			_cmdPack->buffers[2] = makeBufferInfoT(_cmdPack, &clip);
		}
		VkDescriptorBufferInfo infos[3];
		for (uint32_t i = 1; i < 4; i++) {
			infos[i - 1] = _cmdPack->buffers[i - 1];
			infos[i - 1].offset = 0;
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].dstSet = _cmdPack->set0;
			writes[i].dstBinding = i;
			writes[i].descriptorCount = 1;
			writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			writes[i].pBufferInfo = infos + i - 1;
		}
		vkUpdateDescriptorSets(_device, 4, writes, 0, nullptr);
		_cmdPack->commonSetDirty = true;
	}

	void VulkanCanvas::beginPass(int level, bool loadColor, const Color4f *clearColor) {
	 #if DEBUG
		if (_cmdPack->beginPass) {
			if (_cmdPack->target == _target && _cmdPack->level == level) {
				Qk_Fatal("Same render target should not begin a new pass without ending the previous pass.");
			}
		}
	 #endif
		endPass();

		Qk_ASSERT(_surfaceSize.x() > 0 && _surfaceSize.y() > 0, "Vulkan canvas surface size is invalid");
		Qk_ASSERT(_target, "Output color texture should be created before beginning a pass");

		_cmdPack->target = _target;
		_cmdPack->level = level;

		if (clearColor) {
			_cmdPack->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			_cmdPack->clearColor = {.float32 = {clearColor->r(), clearColor->g(), clearColor->b(), clearColor->a()}};
		} else if (_cmdPack->isRecorded() && loadColor) {
			_cmdPack->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		} else {
			_cmdPack->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		_cmdPack->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		_cmdPack->beginPass = true;
	}

	void VulkanCanvas::endPass() {
		if (_cmdPack->beginPass) {
			if (!_cmdPack->renderPass)
				beginRenderPassReady();
			vkCmdEndRenderPass(_cmdPack->current);
			_cmdPack->beginPass = false;
			_cmdPack->recorded = true;
			_cmdPack->renderPass = VK_NULL_HANDLE;
		}
		// _cmdPack->pipeline = VK_NULL_HANDLE;
	}

	bool VulkanCanvas::onlyEndEncoderPass(Color4f &color) {
		if (_cmdPack->beginPass && !_cmdPack->renderPass) {
			if (_cmdPack->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
				auto clr = _cmdPack->clearColor;
				color = Color4f(clr.float32[0], clr.float32[1], clr.float32[2], clr.float32[3]);
				_cmdPack->beginPass = false;
				return true;
			}
		}
		endPass();
		return false;
	}

	void VulkanCanvas::beginRenderPassReady() {
		if (!_cmdPack->beginPass)
			beginPass();
		auto target = _cmdPack->target;
		Qk_ASSERT_LT(_cmdPack->level, target->mipLevels(), "Invalid Vulkan render target mip level");

		// CLEAR/DONT_CARE may discard the previous contents by using UNDEFINED.
		// TODO: If CAPA/compute has just written this target, add an explicit
		// compute-to-color-attachment barrier before beginning this pass; discarding
		// the contents does not by itself synchronize the two GPU writes.
		auto initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		if (_cmdPack->loadOp == VK_ATTACHMENT_LOAD_OP_LOAD) {
			target->transitionLayout(_cmdPack->current,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _cmdPack->level);
			initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		_cmdPack->renderPass = getRenderPass(target->format,
			_cmdPack->loadOp, _cmdPack->storeOp, initialLayout);

		VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		info.renderArea.extent = {
			std::max(target->extent.width >> _cmdPack->level, 1u),
			std::max(target->extent.height >> _cmdPack->level, 1u)
		};
		info.renderPass = _cmdPack->renderPass;
		info.framebuffer = target->framebuffer(_cmdPack->level);

		VkClearValue clearValue{.color = _cmdPack->clearColor};
		if (_cmdPack->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
			info.clearValueCount = 1;
			info.pClearValues = &clearValue;
		}
		vkCmdBeginRenderPass(_cmdPack->current, &info, VK_SUBPASS_CONTENTS_INLINE);
		target->levels[_cmdPack->level].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}

	void VulkanCanvas::beginRenderPass() {
		if (_cmdPack->renderPass)
			return; // render pass already begun for this pack
		beginRenderPassReady();

		VkViewport viewport{.width=_surfaceSize.x(), .height=_surfaceSize.y(), .maxDepth=1.0f};
		vkCmdSetViewport(_cmdPack->current, 0, 1, &viewport);

		// VkRect2D scissor{{0, 0}, {uint32_t(_surfaceSize.x()), uint32_t(_surfaceSize.y())}};
		VkRect2D scissor{{0, 0}, _cmdPack->target->extent};
		vkCmdSetScissor(_cmdPack->current, 0, 1, &scissor);

		updateCommonDescriptorSet(true); // update descriptor set with new buffers
	}

	VkCommandBuffer VulkanCanvas::usePipeline(VkShader &shader) {
		beginRenderPass();

		auto cmd = _cmdPack->current;
		auto pipeline = shader.getPipeline(_blendMode, _cmdPack->target->format);
		Qk_ASSERT(pipeline, "Vulkan pipeline should not be null");
		bool pipelineChanged = _cmdPack->pipeline != pipeline;

		if (pipelineChanged) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			_cmdPack->pipeline = pipeline;
		}
		if (pipelineChanged || _cmdPack->commonSetDirty) {
			uint32_t dynamicOffsets[3] = {
				uint32_t(_cmdPack->buffers[0].offset),
				uint32_t(_cmdPack->buffers[1].offset), uint32_t(_cmdPack->buffers[2].offset),
			};
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.layout(),
				0, 1, &_cmdPack->set0, 3, dynamicOffsets);
			_cmdPack->commonSetDirty = false;
		}
		return cmd;
	}

	VkCommandBuffer VulkanCanvas::usePipeline(VkShader &shader, float vertex[], uint32_t vCount) {
		auto cmd = usePipeline(shader);
		auto &block = makeBufferT(_cmdPack, vertex, vCount);
		VkBuffer buffer = block.val->buffer;
		VkDeviceSize offset = block.begin;
		Qk_ASSERT(buffer, "Vertex buffer should not be null");
		vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
		return cmd;
	}

	VkCommandBuffer VulkanCanvas::usePipeline(VkShader &shader, const VertexData &vertex) {
		auto cmd = usePipeline(shader);
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceSize offset = 0;
		if (Render::useVertexData(vertex.id)) {
			auto vertexBuffer = static_cast<VkVertexBuffer*>(vertex.id->ptr);
			buffer = vertexBuffer->buffer;
			_cmdPack->ref(vertexBuffer); // ref vertex buffer to this command pack
		} else {
			// Create a temporary vertex buffer for the vertex data
			Qk_ASSERT(vertex.vertex.val(), "Vertex data should not be null for draw call");
			Qk_ASSERT_EQ(vertex.vertex.length(), vertex.vCount, "Vertex data length should match vertex count");
			auto &block = makeBufferT(_cmdPack, vertex.vertex.val(), vertex.vertex.length());
			buffer = block.val->buffer;
			offset = block.begin;
		}
		Qk_ASSERT(buffer, "Vertex buffer should not be null");
		vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
		return cmd;
	}

	bool VulkanCanvas::swapBuffer() {
		if (_capaBuilder)
			_capaBuilder->flush();
		endPass(); // end current pass

		if (_cmdPack->recorded) {
			_target->transitionLayout(_cmdPack->current,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _cmdPack->level);
		}
		auto result = vkEndCommandBuffer(_cmdPack->current);
		Qk_ASSERT_EQ(VK_SUCCESS, result, "Failed to end command buffer");

		Lock lock(_mutex);
		bool canSwap = _cmdPackFront->isRecorded() == false;
		if (!canSwap) {
			auto submit = _cmdPackFront->completion.load(std::memory_order_acquire);
			if (submit && _resource->isSubmitCompleted(submit))
				canSwap = true;
		}

		// only swap if there are recorded commands and front cmd pack is empty
		if (canSwap && _cmdPack->isRecorded()) {
			auto submit = _cmdPackFront->completion.exchange(nullptr, std::memory_order_acq_rel);
			if (submit)
				submit->unref();
			std::swap(_cmdPackFront, _cmdPack);

			clear_PathvCache(_cache, 0);
			_cmdPack->reset(this, &lock, true);
		} else {
			_cmdPack->reset(this, &lock, false);
		}
		return canSwap;
	}

	Array<VkCommandBuffer> VulkanCanvas::flushBuffer() {
		ScopeLock lock(_mutex);
		if (_cmdPackFront->current == VK_NULL_HANDLE)
			return Array<VkCommandBuffer>(); // no command buffer to flush
		if (_cmdPackFront->recorded) {
			_cmdPackFront->commands.push(_cmdPackFront->current);
			_cmdPackFront->recorded = false;
		}
		_cmdPackFront->current = VK_NULL_HANDLE; // mark current command buffer as flushed
		return _cmdPackFront->commands;
	}

	void VulkanCanvas::flushSubcanvasCmd(GPUCanvas *sub) {
		if (sub == this)
			return;
		if (sub->render() != _render)
			return;
		auto vkCanvas = static_cast<VulkanCanvas*>(sub);
		auto commands = vkCanvas->flushBuffer(); // flush subcanvas to get command buffers
		if (commands.isNull())
			return;
		endPass();

		if (_cmdPack->recorded) {
			_cmdPack->commands.push(_cmdPack->current); // merge
			_cmdPack->recorded = false;
			Qk_ASSERT_EQ(VK_SUCCESS,
				vkEndCommandBuffer(_cmdPack->current), "Failed to end command buffer");
			// begin a new command buffer for next pass
			beginNextCommand(_cmdPack);
		}
		_cmdPack->commands.concat(commands); // merge subcanvas commands
		_cmdPack->subCanvas.push(vkCanvas); // ref subcanvas to this command pack
	}

}
