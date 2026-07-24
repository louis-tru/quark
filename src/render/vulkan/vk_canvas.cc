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

	void VkFramebufferData::clearSafe(VkDevice device) {
		if (framebuffer)
			vk_deleteFramebufferSafe(device, framebuffer);
		if (view && holdView)
			vk_deleteImageView(device, view);
		framebuffer = VK_NULL_HANDLE;
		view = VK_NULL_HANDLE;
		holdView = false;
	}

	VkDescriptorPool VkDescriptorPools::createDescriptorPool(VkDevice device) {
		VkDescriptorPoolSize sizes[] = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 2048},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048},
		};
		VkDescriptorPoolCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
		info.maxSets = 2048;
		info.poolSizeCount = sizeof(sizes) / sizeof(sizes[0]);
		info.pPoolSizes = sizes;
		VkDescriptorPool pool = VK_NULL_HANDLE;
		vk_check("vkCreateDescriptorPool", vkCreateDescriptorPool(device, &info, nullptr, &pool));
		return pool;
	}

	VkDescriptorSet VkDescriptorPools::allocateDescriptorSet(VkDevice device, VkDescriptorSetLayout setLayout) {
		VkDescriptorSetAllocateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
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

	void VkCmdPack::initialize(VkDevice device) {
		allocator[0] = new VkMemBufferAllocator();
		allocator[1] = new VkMemBufferAllocator(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		descriptorPools.pools.pushBack(descriptorPools.createDescriptorPool(device));
		descriptorPools.iter = descriptorPools.pools.begin();
	}

	VulkanCanvas::VulkanCanvas(VulkanRender *render, Render::Options opts)
		: GPUCanvas(render, opts)
		, _vkRender(render)
		, _resource(getSharedRenderVulkanResource())
		, _device(_resource->device())
		, _commandPool(VK_NULL_HANDLE)
		, _target(nullptr)
		, _outTex(nullptr)
		, _emptyTex(_resource->emptyTexture())
		, _cmdPack{}, _cmdPackFront{}
		, _shaders(_resource->shaders()) // copy shared shader resource
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kBGRA_8888_ColorType;
		auto format = vk_pixelFormat(_opts.colorType);
		Qk_ASSERT_NE(format, VK_FORMAT_UNDEFINED, "Invalid Vulkan canvas color format");

		VkCommandPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = _resource->queueFamily();
		auto result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool);
		Qk_ASSERT_EQ(result, VK_SUCCESS, "Failed to create Vulkan canvas command pool");

		_cmdPack.initialize(_device);
		_cmdPackFront.initialize(_device);
		resetCmdPack(_cmdPack);
	}

	VulkanCanvas::~VulkanCanvas() {
		_mutex.lock();
		_resource->queueWaitIdle();
		if (_framebuffer.framebuffer)
			vkDestroyFramebuffer(_device, _framebuffer.framebuffer, nullptr);
		if (_framebuffer.view && _framebuffer.holdView)
			vkDestroyImageView(_device, _framebuffer.view, nullptr);
		for (auto &i: _renderPasss)
			vkDestroyRenderPass(_device, i.second, nullptr);
		Releasep(_outTex);
		vkDestroyCommandPool(_device, _commandPool, nullptr);
		_cmdPack.descriptorPools.destroy(_device);
		_cmdPackFront.descriptorPools.destroy(_device);
		_cmdPack = {};
		_cmdPackFront = {};
		_commandPool = VK_NULL_HANDLE;
		_device = VK_NULL_HANDLE;
		_framebuffer.view = VK_NULL_HANDLE;
		_framebuffer.framebuffer = VK_NULL_HANDLE;
		_mutex.unlock();
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

	void VulkanCanvas::resetCmdPack(VkCmdPack &pack) {
		if (pack.commands.length())
			vkFreeCommandBuffers(_device, _commandPool, pack.commands.length(), pack.commands.val());
		if (pack.current)
			vkFreeCommandBuffers(_device, _commandPool, 1, &pack.current);
		pack.descriptorPools.reset(_device);
		Qk_ASSERT_EQ(VK_SUCCESS, vk_beginCommandBuffer(_device, _commandPool, &pack.current),
			"Failed to begin Vulkan command buffer");
		pack.commands.clear();
		pack.allocator[0]->reset();
		pack.allocator[1]->reset();
		pack.set0 = VK_NULL_HANDLE;
		pack.pipeline = VK_NULL_HANDLE;
		pack.renderPass = VK_NULL_HANDLE;
		pack.commonSetDirty = false;
		pack.beginPass = false;
		pack.recorded = false;
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

	void VulkanCanvas::updateRootMatrixSet() {
		Qk_ASSERT(_cmdPack.renderPass, "Vulkan render pass should be begun before updating root matrix");
		SpvColor::RootMatrixBlock root{
			.value = _rootMatrix.transpose(),
			.noScale = _rootMatrixNoScale.transpose(),
			.surfaceScale = _surfaceScale,
		};
		auto oldBuff = _cmdPack.buffers[0];
		auto newBuff = makeBufferInfoT(_cmdPack, &root);
		_cmdPack.buffers[0] = newBuff;
		if (newBuff.buffer != oldBuff.buffer)
			updateCommonDescriptorSet(false); // update descriptor set
		_cmdPack.commonSetDirty = true;
	}

	void VulkanCanvas::updateViewMatrixSet() {
		Qk_ASSERT(_cmdPack.renderPass, "Vulkan render pass should be begun before updating view matrix");
		SpvColor::ViewMatrixBlock view{
			.value = Mat4(_state->matrix).transpose(),
		};
		auto oldBuff = _cmdPack.buffers[1].buffer;
		auto newBuff = makeBufferInfoT(_cmdPack, &view);
		_cmdPack.buffers[1] = newBuff; // dynamic buffer is same as previous, no need to update descriptor set
		if (newBuff.buffer != oldBuff)
			updateCommonDescriptorSet(false); // update descriptor set
		_cmdPack.commonSetDirty = true;
	}

	void VulkanCanvas::updateDescriptorSet(VkDescriptorSet set, uint32_t binding,
		VkTexture tex, VkSampler sampler)
	{
		VkDescriptorImageInfo image{sampler};
		image.imageView = tex.view;
		image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkWriteDescriptorSet write{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		write.dstSet = set;
		write.dstBinding = binding;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		write.pImageInfo = &image;
		vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
	}

	void VulkanCanvas::updateDescriptorSet(VkDescriptorSet set, uint32_t binding, cVkMemBlock& buffer) {
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
		uint32_t bindSet, VkPipelineBindPoint bindPoint)
	{
		static constexpr uint32_t dynamicOffsets[32]{0};
		Qk_ASSERT(bindSet < 32, "Vulkan descriptor set bind index overflow");
		vkCmdBindDescriptorSets(_cmdPack.current, bindPoint, shader.layout(),
			0, 1, &set, bindSet, dynamicOffsets);
	}

	void VulkanCanvas::updateCommonDescriptorSet(bool allocBuff) {
		Qk_ASSERT(_cmdPack.renderPass, "Vulkan render pass should be begun before updating common descriptor set");
		_cmdPack.set0 = allocateDescriptorSet(_shaders.color.sets(0));
		// Update image sampler descriptor sets
		VkWriteDescriptorSet writes[4]{};
		VkDescriptorImageInfo image{_resource->nearestSampler()};
		image.imageView = _clipState ? vk_get_texture(_clipState->mask.get())->view: _emptyTex->view;
		image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writes[0].dstSet = _cmdPack.set0;
		writes[0].dstBinding = 0;
		writes[0].descriptorCount = 1;
		writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writes[0].pImageInfo = &image;

		// Update uniform buffer descriptor sets
		if (allocBuff) {
			SpvColor::RootMatrixBlock root{
				.value = _rootMatrix.transpose(),
				.noScale = _rootMatrixNoScale.transpose(),
				.surfaceScale = _surfaceScale,
			};
			SpvColor::ViewMatrixBlock view{
				.value = Mat4(_state->matrix).transpose(),
			};
			SpvColor::ClipStatBlock clip{};
			if (_clipState) {
				clip.bounds = *(Vec4*)_clipState->bounds.begin.val;
				clip.op = _clipState->op;
			}
			_cmdPack.buffers[0] = makeBufferInfoT(_cmdPack, &root);
			_cmdPack.buffers[1] = makeBufferInfoT(_cmdPack, &view);
			_cmdPack.buffers[2] = makeBufferInfoT(_cmdPack, &clip);
		}
		VkDescriptorBufferInfo infos[3];
		for (uint32_t i = 1; i < 4; i++) {
			infos[i - 1] = _cmdPack.buffers[i - 1];
			infos[i - 1].offset = 0;
			writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writes[i].dstSet = _cmdPack.set0;
			writes[i].dstBinding = i;
			writes[i].descriptorCount = 1;
			writes[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			writes[i].pBufferInfo = infos + i - 1;
		}
		vkUpdateDescriptorSets(_device, 4, writes, 0, nullptr);
		_cmdPack.commonSetDirty = true;
	}

	void VulkanCanvas::bindCommonDescriptorSet(VkShader& shader) {
		uint32_t dynamicOffsets[3] = {
			uint32_t(_cmdPack.buffers[0].offset),
			uint32_t(_cmdPack.buffers[1].offset), uint32_t(_cmdPack.buffers[2].offset),
		};
		vkCmdBindDescriptorSets(_cmdPack.current, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.layout(),
			0, 1, &_cmdPack.set0, 3, dynamicOffsets);
	}

	void VulkanCanvas::beginPass(int level, bool loadColor, Color4f *clearColor) {
	 #if DEBUG
		if (_cmdPack.beginPass) {
			if (_cmdPack.target == _target && _cmdPack.level == level) {
				Qk_Fatal("Same render target should not begin a new pass without ending the previous pass.");
			}
		}
	 #endif
		endPass();

		Qk_ASSERT(_surfaceSize.x() > 0 && _surfaceSize.y() > 0, "Vulkan canvas surface size is invalid");
		Qk_ASSERT(_target, "Output color texture should be created before beginning a pass");

		_cmdPack.target = _target;
		_cmdPack.level = level;

		if (clearColor) {
			_cmdPack.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			_cmdPack.clearColor = {.float32 = {clearColor->r(), clearColor->g(), clearColor->b(), clearColor->a()}};
		} else if (_cmdPack.isRecorded() && loadColor) {
			_cmdPack.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		} else {
			_cmdPack.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
		_cmdPack.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		_cmdPack.beginPass = true;
	}

	void VulkanCanvas::endPass() {
		if (_cmdPack.beginPass) {
			if (!_cmdPack.renderPass)
				beginRenderPassReady();
			vkCmdEndRenderPass(_cmdPack.current);
			_cmdPack.beginPass = false;
			_cmdPack.recorded = true;
			_cmdPack.renderPass = VK_NULL_HANDLE;
		}
	}

	void VulkanCanvas::beginRenderPassReady() {
		if (!_cmdPack.beginPass)
			beginPass();

		auto target = _cmdPack.target;
		_cmdPack.renderPass = getRenderPass(target->format, _cmdPack.loadOp, _cmdPack.storeOp);

		VkRenderPassBeginInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		info.renderArea.extent = { uint32_t(_surfaceSize.x()), uint32_t(_surfaceSize.y()) };
		info.renderPass = _cmdPack.renderPass;

		if (_framebuffer.view != target->view) {
			_framebuffer.clearSafe(_device);
			if (_cmdPack.level) {
				_framebuffer.view = vk_createLevelView(_device, target, _cmdPack.level);
				_framebuffer.holdView = true;
			} else {
				_framebuffer.view = target->view;
			}
			_framebuffer.framebuffer = vk_create_framebuffer(
				_device, info.renderPass, _framebuffer.view, info.renderArea.extent);
		}
		info.framebuffer = _framebuffer.framebuffer;

		if (_cmdPack.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR) {
			VkClearValue clearValue{.color = _cmdPack.clearColor};
			info.clearValueCount = 1;
			info.pClearValues = &clearValue;
		}
		vkCmdBeginRenderPass(_cmdPack.current, &info, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanCanvas::beginRenderPass() {
		if (_cmdPack.renderPass)
			return; // render pass already begun for this pack

		beginRenderPassReady();

		VkViewport viewport{.width=_surfaceSize.x(), .height=_surfaceSize.y(), .maxDepth=1.0f};
		vkCmdSetViewport(_cmdPack.current, 0, 1, &viewport);

		VkExtent2D extent{ uint32_t(_surfaceSize.x()), uint32_t(_surfaceSize.y()) };
		VkRect2D scissor{{0, 0}, extent};
		vkCmdSetScissor(_cmdPack.current, 0, 1, &scissor);

		updateCommonDescriptorSet(true); // update descriptor set with new buffers
	}

	VkCommandBuffer VulkanCanvas::usePipeline(VkShader &shader) {
		Qk_ASSERT(_cmdPack.beginPass, "Vulkan canvas should begin pass before setting pipeline");
		beginRenderPass();

		auto cmd = _cmdPack.current;
		auto pipeline = shader.getPipeline(_blendMode, _target->format);
		bool pipelineChanged = _cmdPack.pipeline != pipeline;

		if (_cmdPack.pipeline != pipeline) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			_cmdPack.pipeline = pipeline;
		}

		if (pipelineChanged || _cmdPack.commonSetDirty) {
			bindCommonDescriptorSet(shader);
			_cmdPack.commonSetDirty = false;
		}
		return cmd;
	}

	VkCommandBuffer VulkanCanvas::usePipeline(VkShader &shader, const VertexData &vertex) {
		auto cmd = usePipeline(shader);
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceSize offset = 0;
		if (Render::useVertexData(vertex.id)) {
			auto vertexBuffer = static_cast<VkVertexBuffer*>(vertex.id->ptr);
			buffer = vertexBuffer->buffer;
		} else if (vertex.vertex.val()) {
			// Create a temporary vertex buffer for the vertex data
			Qk_ASSERT_EQ(vertex.vertex.length(), vertex.vCount, "Vertex data length should match vertex count");
			auto &block = makeBufferT(_cmdPack, vertex.vertex.val(), vertex.vertex.length());
			buffer = block.val->buffer;
			offset = block.begin;
		}
		Qk_ASSERT(buffer, "Vertex buffer should not be null");
		vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
		return _cmdPack.current;
	}

	bool VulkanCanvas::swapBuffer() {
		return true;
	}

	Array<VkCommandBuffer> VulkanCanvas::flushBuffer() {
		return {};
	}
}
