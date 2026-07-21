/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_vulkan_vk_render__
#define __quark_render_vulkan_vk_render__

#include <vulkan/vulkan.h>
#include "../render.h"
#include "./vk_canvas.h"

namespace qk {
	struct VkTexture {
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkFormat format;
		VkExtent2D extent;
		uint32_t mipLevels;
		VkImageUsageFlags usage;
		void generateMipmaps(VkCommandBuffer cmd) const;
	};

	struct VkVertexBuffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize size = 0;
	};

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
		bool computeSupport() const { return _computeSupport; }
		bool pvrtcSupport() const { return _pvrtcSupport; }
		VkTexture* newTexture(Vec2 size, ColorType type, uint32_t mipLevels, uint8_t flags);
		VkVertexBuffer* newVertexBuffer(uint32_t size);
		VkResult submitCommand(const VkSubmitInfo* submit, VkFence fence = VK_NULL_HANDLE);
		VkResult submitCommand(const VkSubmitInfo* submit, Cb cb);
	private:
		explicit VulkanRenderResource();
		void createDevice();
		void checkAsyncWaitTasks(Array<Cb> *out);
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
		Dict<uint32_t, VkPipeline> _pipelines;
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
