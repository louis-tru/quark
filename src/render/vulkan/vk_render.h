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

// @private head

#ifndef __quark_render_vulkan_vk_render__
#define __quark_render_vulkan_vk_render__

#include "./vk_util.h"
#include "../render.h"
#include "../blend.h"
#include "./vk_canvas.h"
#include "./vk_shaders.h"

namespace qk {

	// Global render resource,
	// used for texture/vertex data creation, shader function and pipeline state caching
	class VulkanRenderResource: public RenderResource {
	public:
		~VulkanRenderResource();
		bool uploadTexture(Pixel *pix, int levels, TexStat *out, bool mipmap) override;
		void unloadTexture(TexStat *tex) override;
		TexStat createTextureStat(Vec2 size, ColorType type, uint8_t flags) override;
		bool uploadVertexData(VertexData::ID *id);
		void unloadVertexData(VertexData::ID *id);
		VkInstance instance() const { return _instance; }
		VkPhysicalDevice physicalDevice() const { return _physicalDevice; }
		VkDevice device() const { return _device; }
		VkQueue commandQueue() const { return _commandQueue; }
		uint32_t queueFamily() const { return _queueFamily; }
		bool computeSupport() const { return _computeSupport; }
		bool pvrtcSupport() const { return _pvrtcSupport; }
		VkTexture* emptyTexture() { return _emptyTexture.get(); }
		VkSampler nearestSampler() { return _nearestSampler; }
		VkSampler linearSampler() { return _linearSampler; }
		VkShaders& shaders() { return _shaders; }
		VkPipelineLayoutData* getPipelineLayout(VkPipelineKind kind);
		VkPipeline getPipeline(VkPipelineKind kind, BlendMode mode, VkFormat format);
		VkPipeline getComputePipeline(VkPipelineKind kind);
		VkSampler get_sampler(const PaintImage* paint);
		VkSampler get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap);
		VkResult submitCommand(const VkSubmitInfo* submit, VkFence fence = VK_NULL_HANDLE);
		VkResult submitCommand(const VkSubmitInfo* submit, Cb cb);
		VkTexture* newTexture(Vec2 size, ColorType type, uint32_t mipLevels, uint8_t flags);
		void queueWaitIdle();
	private:
		explicit VulkanRenderResource();
		void createDevice();
		bool createEmptyTexture();
		VkVertexBuffer* newVertexBuffer(uint32_t size);
		VkPipelineLayoutData* getPipelineLayoutNoLock(VkPipelineKind kind);
		void checkAsyncWaitTasks(Array<Cb> *out);
		inline VkShader& getShader(VkPipelineKind kind) {
			Qk_ASSERT(kind < kVkPipelineCount, "Invalid Vulkan pipeline kind: %d", kind);
			return *_shaders.allShaders[kind];
		}
		VkShaderModule getShaderModule(VkPipelineKind kind, VkShaderStageFlagBits stage);
		// fields:
		struct AsyncWaitTask { VkFence fence; Cb cb; };
		Mutex _mutex, _commitMutex;
		VkInstance _instance;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		VkQueue _commandQueue;
		uint32_t _queueFamily;
		bool _computeSupport, _pvrtcSupport;
		VkPipelineCache _pipelineCache;
		Sp<VkTexture> _emptyTexture;
		VkSampler _nearestSampler; // sampler state for nearest filter mode
		VkSampler _linearSampler; // sampler state for linear filter mode
		VkShaders _shaders;
		Dict<uint32_t, VkShaderModule> _shaderModules;
		Dict<uint32_t, VkPipelineLayoutData> _pipelineLayouts;
		Dict<uint64_t, VkPipeline> _pipelines;
		Dict<uint32_t, VkSampler> _samplers;
		List<AsyncWaitTask> _asyncWaitTasksPool, _asyncWaitTasks;
		int64_t _nextAsyncWaitCheckTime;
		VkCommandPool _commandPool;
		friend VulkanRenderResource* getSharedRenderVulkanResource();
		friend class VulkanRender;
	};

	// Vulkan render backend implementation for Android and Linux and Windows
	class VulkanRender: public RenderBackend {
	public:
		~VulkanRender() override;
		void release() override;
		Canvas* createCanvas(Options opts) override;
		TexStat createTextureStat(Vec2 size, ColorType type, uint8_t flags) override;
		bool uploadTexture(Pixel *pix, int levels, TexStat *out, bool mipmap) override;
		void unloadTexture(TexStat *tex) override;
		bool uploadVertexData(VertexData::ID *id) override;
		void unloadVertexData(VertexData::ID *id) override;
	protected:
		explicit VulkanRender(Options opts);
	// fields:
		VulkanRenderResource* _resource;
		VkDevice _device;
		VulkanCanvas *_vkCanvas;
	};

	VulkanRenderResource* getSharedRenderVulkanResource();

} // namespace qk

#endif
