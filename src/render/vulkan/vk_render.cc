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

#include "./vk_render.h"
#include "src/util/macros.h"

namespace qk {
	uint32_t vk_uniformBufferAlignment;
	uint32_t vk_maxPushConstantsSize;

	VulkanRenderResource::VulkanRenderResource()
		: _instance(VK_NULL_HANDLE)
		, _physicalDevice(VK_NULL_HANDLE)
		, _device(VK_NULL_HANDLE)
		, _commandQueue(VK_NULL_HANDLE)
		, _queueFamily(U32::limit_max)
		, _computeSupport(false)
		, _pvrtcSupport(false)
		, _pipelineCache(VK_NULL_HANDLE)
		, _emptyTexture(nullptr)
		, _commandPool(VK_NULL_HANDLE)
		, _nextAsyncWaitCheckTime(0)
	{
		Qk_CHECK(vk_createInstance(&_instance), "Unable to create Vulkan instance");
		Qk_CHECK(vk_selectBestDevice(
			_instance,
			&_physicalDevice,
			&_queueFamily,
			&_computeSupport
		), "Unable to select Vulkan physical device");

		createDevice();

		VkPipelineCacheCreateInfo pipelineInfo{VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO};
		vk_check("vkCreatePipelineCache", vkCreatePipelineCache(_device, &pipelineInfo, nullptr, &_pipelineCache));

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		poolInfo.queueFamilyIndex = _queueFamily;
		vk_check("vkCreateCommandPool", vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool));

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
		vk_uniformBufferAlignment = (uint32_t)std::max<VkDeviceSize>(
			16, properties.limits.minUniformBufferOffsetAlignment
		);
		vk_maxPushConstantsSize = U32::min(256, properties.limits.maxPushConstantsSize);

		_emptyTexture = newTexture(Vec2(1), kRGBA_8888_ColorType, 1, kNone_TextureFlags);
		_nearestSampler = get_sampler(PaintImage::kNearest_FilterMode, PaintImage::kNearest_MipmapMode);
		_linearSampler = get_sampler(PaintImage::kLinear_FilterMode, PaintImage::kNearest_MipmapMode);

		Qk_CHECK(createEmptyTexture(), "Failed to create Vulkan empty texture");
		_shaders.buildAll();
	}

	VulkanRenderResource::~VulkanRenderResource() {
		ScopeLock lock(_commitMutex);
		vkQueueWaitIdle(_commandQueue);
		for (auto &pipeline: _pipelines) {
			vkDestroyPipeline(_device, pipeline.second, nullptr);
		}
		for (auto &layout: _pipelineLayouts) {
			vkDestroyPipelineLayout(_device, layout.second.layout, nullptr);
			for (auto setLayout: layout.second.sets)
				vkDestroyDescriptorSetLayout(_device, setLayout, nullptr);
		}
		for (auto &module: _shaderModules) {
			vkDestroyShaderModule(_device, module.second, nullptr);
		}
		for (auto &sampler: _samplers) {
			vkDestroySampler(_device, sampler.second, nullptr);
		}
		for (auto &task: _asyncWaitTasksPool) {
			vkDestroyFence(_device, task.fence, nullptr);
		}
		for (auto &task: _asyncWaitTasks) {
			task.cb->resolve(); // Resolve the callback before destroying the fence
			vkDestroyFence(_device, task.fence, nullptr);
		}
		_emptyTexture = nullptr;
		if (_commandPool)
			vkDestroyCommandPool(_device, _commandPool, nullptr);
		if (_pipelineCache)
			vkDestroyPipelineCache(_device, _pipelineCache, nullptr);
		vkDestroyDevice(_device, nullptr);
		if (_instance)
			vkDestroyInstance(_instance, nullptr);
	}

	VkShaderModule VulkanRenderResource::getShaderModule(VkPipelineKind kind, VkShaderStageFlagBits stage) {
		auto key = (uint32_t(kind) << 8) | uint32_t(stage);
		VkShaderModule module;
		if (_shaderModules.get(key, module))
			return module;

		auto &source = getShader(kind).source;
		const VkShaderCode *code = nullptr;
		switch (stage) {
			case VK_SHADER_STAGE_VERTEX_BIT: code = &source.vertex; break;
			case VK_SHADER_STAGE_FRAGMENT_BIT: code = &source.fragment; break;
			case VK_SHADER_STAGE_COMPUTE_BIT: code = &source.compute; break;
			default: return VK_NULL_HANDLE;
		}
		Qk_ASSERT(code, "Invalid Vulkan shader stage: %d", stage);
		Qk_ASSERT(code->words && code->size, "Vulkan shader stage is unavailable: %s", source.name);

		VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
		info.codeSize = code->size;
		info.pCode = code->words;
		Qk_ASSERT_EQ(VK_SUCCESS, vkCreateShaderModule(_device, &info, nullptr, &module),
			"Failed to create Vulkan shader module");
		_shaderModules.set(key, module);
		return module;
	}

	VkPipelineLayoutData* VulkanRenderResource::getPipelineLayout(VkPipelineKind kind) {
		ScopeLock lock(_mutex);
		return getPipelineLayoutNoLock(kind);
	}

	VkPipelineLayoutData* VulkanRenderResource::getPipelineLayoutNoLock(VkPipelineKind kind) {
		VkPipelineLayoutData *cached;
		if (_pipelineLayouts.get(uint32_t(kind), cached))
			return cached;

		auto &shader = getShader(kind);
		Array<VkPushConstantRange> pushConstants;
		for (auto binding: shader.bindings) {
			if (binding->set == UINT32_MAX) {
				pushConstants.push({binding->stages, 0, binding->sizeOf});
			}
		}

		VkPipelineLayoutData data;
		auto &setLayouts = data.sets;
		uint32_t setCount = 0;
		for (auto binding: shader.bindings) {
			if (binding->set != UINT32_MAX)
				setCount = std::max(setCount, binding->set + 1);
		}
		Array<Array<VkDescriptorSetLayoutBinding>> descriptorBindings(setCount);
		for (auto binding: shader.bindings) {
			if (binding->set != UINT32_MAX) {
				descriptorBindings[binding->set].push({
					binding->binding,
					binding->descriptorType,
					binding->arrayCount ? binding->arrayCount : 1,
					binding->stages,
					nullptr,
				});
			}
		}
		setLayouts = Array<VkDescriptorSetLayout>(descriptorBindings.length());
		for (uint32_t i = 0; i < descriptorBindings.length(); i++) {
			auto &bindings = descriptorBindings[i];
			VkDescriptorSetLayoutCreateInfo info{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
			info.bindingCount = bindings.length();
			info.pBindings = bindings.val();
			auto result = vkCreateDescriptorSetLayout(_device, &info, nullptr, &setLayouts[i]);
			Qk_ASSERT_EQ(result, VK_SUCCESS,
				"Failed to create Vulkan descriptor set layout");
		}

		VkPipelineLayoutCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
		info.setLayoutCount = setLayouts.length();
		info.pSetLayouts = setLayouts.val();
		info.pushConstantRangeCount = pushConstants.length();
		info.pPushConstantRanges = pushConstants.val();
		Qk_ASSERT_EQ(VK_SUCCESS, vkCreatePipelineLayout(_device, &info, nullptr, &data.layout),
			"Failed to create Vulkan pipeline layout");
		return &_pipelineLayouts.set(uint32_t(kind), std::move(data));
	}

	VkPipeline VulkanRenderResource::getPipeline(VkPipelineKind kind, BlendMode mode, VkFormat format) {
		Qk_ASSERT_NE(VK_FORMAT_UNDEFINED, format, "Vulkan graphics pipeline requires color format");
		ScopeLock lock(_mutex);
		auto key = vk_pipeline_key(kind, mode, format);
		VkPipeline pipeline;
		if (_pipelines.get(key, pipeline))
			return pipeline;

		auto &shader = getShader(kind);
		Qk_ASSERT(shader.source.vertex.words && shader.source.fragment.words,
			"Vulkan graphics shader stages are unavailable: %s", shader.source.name);

		auto vertexModule = getShaderModule(kind, VK_SHADER_STAGE_VERTEX_BIT);
		auto fragmentModule = getShaderModule(kind, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkPipelineShaderStageCreateInfo stages[2] = {
			{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
				VK_SHADER_STAGE_VERTEX_BIT, vertexModule, "main"},
			{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0,
				VK_SHADER_STAGE_FRAGMENT_BIT, fragmentModule, "main"},
		};

		VkVertexInputBindingDescription vertexBinding{};
		vertexBinding.binding = 0;
		vertexBinding.stride = shader.vertexStride;
		vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		Array<VkVertexInputAttributeDescription> attributes(shader.attributes.length());
		for (uint32_t i = 0; i < shader.attributes.length(); i++) {
			auto &src = shader.attributes[i];
			auto &dst = attributes[i];
			dst.location = src.location;
			dst.binding = 0;
			dst.format = src.format;
			dst.offset = src.offset;
		}
		VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
		if (attributes.length()) {
			vertexInput.vertexBindingDescriptionCount = 1;
			vertexInput.pVertexBindingDescriptions = &vertexBinding;
			vertexInput.vertexAttributeDescriptionCount = attributes.length();
			vertexInput.pVertexAttributeDescriptions = attributes.val();
		}

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewport{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
		viewport.viewportCount = 1;
		viewport.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
		raster.polygonMode = VK_POLYGON_MODE_FILL;
		raster.cullMode = VK_CULL_MODE_NONE;
		raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		raster.lineWidth = 1.0f;

		VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
		multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		auto blendAttachment = vk_blend_state(mode);
		VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
		blend.attachmentCount = 1;
		blend.pAttachments = &blendAttachment;

		VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamic{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
		dynamic.dynamicStateCount = 2;
		dynamic.pDynamicStates = dynamicStates;

		VkGraphicsPipelineCreateInfo info{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
		info.stageCount = 2;
		info.pStages = stages;
		info.pVertexInputState = &vertexInput;
		info.pInputAssemblyState = &inputAssembly;
		info.pViewportState = &viewport;
		info.pRasterizationState = &raster;
		info.pMultisampleState = &multisample;
		info.pColorBlendState = &blend;
		info.pDynamicState = &dynamic;
		info.layout = getPipelineLayoutNoLock(kind)->layout;
		info.renderPass = vk_create_pipeline_render_pass(_device, format);
		info.subpass = 0;
		Qk_ASSERT_EQ(VK_SUCCESS, vkCreateGraphicsPipelines(
			_device, _pipelineCache, 1, &info, nullptr, &pipeline), "Failed to create Vulkan graphics pipeline");
		vkDestroyRenderPass(_device, info.renderPass, nullptr);
		_pipelines.set(key, pipeline);
		return pipeline;
	}

	VkPipeline VulkanRenderResource::getComputePipeline(VkPipelineKind kind) {
		ScopeLock lock(_mutex);
		auto key = vk_pipeline_key(kind, (BlendMode)0, VK_FORMAT_UNDEFINED);
		VkPipeline pipeline = VK_NULL_HANDLE;
		if (_pipelines.get(key, pipeline))
			return pipeline;

		auto &shader = getShader(kind);
		Qk_ASSERT(shader.source.compute.words,
			"Vulkan compute shader stage is unavailable: %s", shader.source.name);

		auto module = getShaderModule(kind, VK_SHADER_STAGE_COMPUTE_BIT);
		VkComputePipelineCreateInfo info{VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
		info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		info.stage.module = module;
		info.stage.pName = "main";
		info.layout = getPipelineLayoutNoLock(kind)->layout;

		Qk_ASSERT_EQ(VK_SUCCESS, vkCreateComputePipelines(
			_device, _pipelineCache, 1, &info, nullptr, &pipeline),
			"Failed to create Vulkan compute pipeline");
		_pipelines.set(key, pipeline);
		return pipeline;
	}

	void VulkanRenderResource::queueWaitIdle() {
		ScopeLock lock(_commitMutex);
		vkQueueWaitIdle(_commandQueue);
	}

	VkResult VulkanRenderResource::submitCommand(const VkSubmitInfo* submit, VkFence fence) {
		VkResult result;
		Array<Cb> callbacks;
		{
			ScopeLock lock(_commitMutex);
			result = vkQueueSubmit(_commandQueue, 1, submit, fence);
			checkAsyncWaitTasks(&callbacks);
		}
		for (auto &cb: callbacks) {
			cb->resolve();
		}
		return result;
	}

	VkResult VulkanRenderResource::submitCommand(const VkSubmitInfo* submit, Cb cb) {
		ScopeLock lock(_commitMutex);
		if (!cb) {
			return vkQueueSubmit(_commandQueue, 1, submit, VK_NULL_HANDLE);
		}
		VkFence fence = VK_NULL_HANDLE;
		auto it = _asyncWaitTasksPool.begin();
		if (it != _asyncWaitTasksPool.end()) {
			fence = it->fence;
			_asyncWaitTasksPool.erase(it);
		} else {
			VkFenceCreateInfo info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
			auto res = vkCreateFence(_device, &info, nullptr, &fence);
			if (res != VK_SUCCESS)
				return res;
		}
		auto result = vkQueueSubmit(_commandQueue, 1, submit, fence);
		if (result == VK_SUCCESS) {
			_asyncWaitTasks.pushBack({fence, cb}); // Add the task to the active list if submission succeeded
		} else {
			_asyncWaitTasksPool.pushBack({fence}); // Return the fence to the pool if submission failed
		}
		return result;
	}

	void VulkanRenderResource::checkAsyncWaitTasks(Array<Cb> *out) {
		if (_asyncWaitTasks.length() == 0)
			return;

		int64_t now = time_monotonic();
		// 16ms 真正检查一次
		if (now < _nextAsyncWaitCheckTime)
			return;
		_nextAsyncWaitCheckTime = now + 16 * 1000;

		for (auto it = _asyncWaitTasks.begin(); it != _asyncWaitTasks.end(); ) {
			if (vkGetFenceStatus(_device, it->fence) == VK_SUCCESS) {
				out->push(it->cb);
				vkResetFences(_device, 1, &it->fence);
				it->cb = nullptr;
				_asyncWaitTasksPool.pushBack(*it);
				it = _asyncWaitTasks.erase(it);
			} else {
				++it;
			}
		}
	}

	// -----------------------------------------------------------------------

	VulkanRender::VulkanRender(Options opts)
		: RenderBackend(opts)
		, _resource(getSharedRenderVulkanResource())
		, _device(_resource->device())
		, _vkCanvas(nullptr)
	{
		_vkCanvas = NewRetain<VulkanCanvas>(this, _opts);
		_opts.colorType = _vkCanvas->opts().colorType;
		_canvas = _vkCanvas;
	}

	VulkanRender::~VulkanRender() {
		Qk_CHECK(_vkCanvas == nullptr);
	}

	void VulkanRender::release() {
		Releasep(_vkCanvas);
		_canvas = nullptr;
		_device = VK_NULL_HANDLE;
	}

	Canvas* VulkanRender::createCanvas(Options opts) {
		return new VulkanCanvas(this, opts);
	}

	TexStat VulkanRender::createTextureStat(Vec2 size, ColorType type, uint8_t flags) {
		return _resource->VulkanRenderResource::createTextureStat(size, type, flags);
	}

	bool VulkanRender::uploadTexture(Pixel *pix, int levels, TexStat *tex, bool mipmap) {
		return _resource->VulkanRenderResource::uploadTexture(pix, levels, tex, mipmap);
	}

	void VulkanRender::unloadTexture(TexStat *tex) {
		_resource->VulkanRenderResource::unloadTexture(tex);
	}

	bool VulkanRender::uploadVertexData(VertexData::ID *id) {
		return _resource->uploadVertexData(id);
	}

	void VulkanRender::unloadVertexData(VertexData::ID *id) {
		_resource->unloadVertexData(id);
	}

} // namespace qk
