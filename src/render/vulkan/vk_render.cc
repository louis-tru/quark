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
#include "src/util/thread.h"

namespace qk {
	#define vk_call(call, fail, ...) result = call(__VA_ARGS__); if (result != VK_SUCCESS) fail

	void vk_check(const char *call, VkResult result) {
		Qk_CHECK(result == VK_SUCCESS, "%s failed: %d", call, int(result));
	}
	bool vk_createInstance(VkInstance *instance);
	bool vk_selectBestDevice(
		VkInstance instance, VkPhysicalDevice *selectedDevice,
		uint32_t *selectedQueueFamily, bool *selectedComputeSupport
	);
	bool vk_supportsDeviceExtension(VkPhysicalDevice device, const char *extension);

	void add_delay_task_for_app(Cb cb, bool recursion);

	static uint32_t mipLevelCount(Vec2 size) {
		uint32_t width = uint32_t(size.x());
		uint32_t height = uint32_t(size.y());
		uint32_t levels = 1;
		while (width > 1 || height > 1) {
			width = std::max(width >> 1, 1u);
			height = std::max(height >> 1, 1u);
			levels++;
		}
		return levels;
	}

	void VkTexture::generateMipmaps(VkCommandBuffer cmd) const {
		VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		int32_t width = int32_t(extent.width);
		int32_t height = int32_t(extent.height);
		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);

			VkImageBlit blit = {};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.layerCount = 1;
			blit.srcOffsets[1] = { width, height, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.layerCount = 1;
			blit.dstOffsets[1] = { std::max(width >> 1, 1), std::max(height >> 1, 1), 1 };
			vkCmdBlitImage(cmd,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit, VK_FILTER_LINEAR);

			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
			width = std::max(width >> 1, 1);
			height = std::max(height >> 1, 1);
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);
	}

	static VkFormat pixelFormat(ColorType type) {
		switch (type) {
			case kAlpha_8_ColorType:
			case kLuminance_8_ColorType:
			case kYUV420P_Y_8_ColorType:
			case kYUV420P_U_8_ColorType:
				return VK_FORMAT_R8_UNORM;
			case kLuminance_Alpha_88_ColorType:
			case kYUV420SP_UV_88_ColorType:
				return VK_FORMAT_R8G8_UNORM;
			case kRGB_565_ColorType:
				return VK_FORMAT_R5G6B5_UNORM_PACK16;
			case kRGBA_4444_ColorType:
			case kRGB_444X_ColorType:
				return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
			case kRGBA_8888_ColorType:
			case kRGB_888X_ColorType:
				return VK_FORMAT_R8G8B8A8_UNORM;
			case kBGRA_8888_ColorType:
			case kBGR_888X_ColorType:
				return VK_FORMAT_B8G8R8A8_UNORM;
			case kRGBA_1010102_ColorType:
			case kRGB_101010X_ColorType:
				// Candidate mapping. Verify channel order and packed-bit layout with
				// actual uploaded pixel samples on Vulkan before relying on it.
				return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
			case kRGB_888_ColorType:
				return VK_FORMAT_R8G8B8_UNORM;
			case kRGBA_5551_ColorType:
				return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
			case kSDF_F32_ColorType:
			case kSDF_Unsigned_F32_ColorType:
				return VK_FORMAT_R32_SFLOAT;
			case kETC1_ColorType:
			case kETC2_RGB_ColorType:
				return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
			case kETC2_RGBA_ColorType:
				return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
			case kETC2_RGB_A1_ColorType:
				return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
			case kPVRTCI_2BPP_RGB_ColorType:
			case kPVRTCI_2BPP_RGBA_ColorType:
				return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
			case kPVRTCI_4BPP_RGB_ColorType:
			case kPVRTCI_4BPP_RGBA_ColorType:
				return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
			case kPVRTCII_2BPP_ColorType:
				return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
			case kPVRTCII_4BPP_ColorType:
				return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
			case kDXT1_ColorType: // kBC1_ColorType
				return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case kDXT2_ColorType:
			case kDXT3_ColorType: // kBC2_ColorType
				return VK_FORMAT_BC2_UNORM_BLOCK;
			case kDXT4_ColorType:
			case kDXT5_ColorType: // kBC3_ColorType
				return VK_FORMAT_BC3_UNORM_BLOCK;
			case kBC4_ColorType:
				return VK_FORMAT_BC4_UNORM_BLOCK;
			case kBC5_ColorType:
				return VK_FORMAT_BC5_UNORM_BLOCK;
			case kBC6_UFloat_ColorType:
				return VK_FORMAT_BC6H_UFLOAT_BLOCK;
			case kBC6_SFloat_ColorType:
				return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			case kBC7_ColorType:
				return VK_FORMAT_BC7_UNORM_BLOCK;
			case kEAC_R11_ColorType:
				return VK_FORMAT_EAC_R11_UNORM_BLOCK;
			case kEAC_RG11_ColorType:
				return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
			case kSharedExponentR9G9B9E5_ColorType:
				return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
			// Packed YUV requires a sampler YCbCr conversion path.
			case kUYVY_ColorType:
			case kYUY2_ColorType:
			// Multi-planar YUV requires plane-specific image creation and upload.
			case kYUV420P_ColorType:
			case kYUV420SP_ColorType:
			case kYUV411P_ColorType:
			case kYUV411SP_ColorType:
			// No directly compatible ordinary sampled-image format.
			case kBW1BPP_ColorType:
			case kRGBG_8888_ColorType:
			case kGRGB_8888_ColorType:
			case kInvalid_ColorType:
				return VK_FORMAT_UNDEFINED;
			default:
				return VK_FORMAT_UNDEFINED;
		}
	}

	static VkResult findMemoryType(
		VkPhysicalDevice physicalDevice, uint32_t typeBits,
		VkMemoryPropertyFlags flags, uint32_t *typeIndex
	) {
		VkPhysicalDeviceMemoryProperties properties = {};
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &properties);
		for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
			if ((typeBits & (1u << i)) && (properties.memoryTypes[i].propertyFlags & flags) == flags) {
				*typeIndex = i;
				return VK_SUCCESS;
			}
		}
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	VulkanRenderResource* getSharedRenderVulkanResource() {
		static VulkanRenderResource *resource = new VulkanRenderResource();
		return resource;
	}

	RenderResource* getSharedRenderResource() {
		return getSharedRenderVulkanResource();
	}

	// A simple Vulkan application that draws a single color to the screen.
	VkResult vk_beginCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer *cmd) {
		VkResult result;
		VkCommandBufferAllocateInfo cmdInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
		cmdInfo.commandPool = pool;
		cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdInfo.commandBufferCount = 1;
		vk_call(vkAllocateCommandBuffers, return result, device, &cmdInfo, cmd);
		VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		return vkBeginCommandBuffer(*cmd, &beginInfo);
	}

	VkResult vk_submitCommand(const VkCommandBuffer* cmd, Cb cb) {
		VkSubmitInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.commandBufferCount = 1;
		info.pCommandBuffers = cmd;
		return getSharedRenderVulkanResource()->submitCommand(&info, cb);
	}

	void vk_deleteTexture(VkDevice device, VkTexture *tex) {
		if (!tex)
			return;
		if (tex->view)
			vkDestroyImageView(device, tex->view, nullptr);
		if (tex->image)
			vkDestroyImage(device, tex->image, nullptr);
		if (tex->memory)
			vkFreeMemory(device, tex->memory, nullptr);
		delete tex;
	}

	void vk_deleteVertexBuffer(VkDevice device, VkVertexBuffer *vertex) {
		if (!vertex)
			return;
		if (vertex->buffer)
			vkDestroyBuffer(device, vertex->buffer, nullptr);
		if (vertex->memory)
			vkFreeMemory(device, vertex->memory, nullptr);
		delete vertex;
	}

	VulkanRenderResource::VulkanRenderResource()
		: _instance(VK_NULL_HANDLE)
		, _physicalDevice(VK_NULL_HANDLE)
		, _device(VK_NULL_HANDLE)
		, _commandQueue(VK_NULL_HANDLE)
		, _queueFamily(U32::limit_max)
		, _computeSupport(false)
		, _pvrtcSupport(false)
		, _pipelineCache(VK_NULL_HANDLE)
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
	}

	VulkanRenderResource::~VulkanRenderResource() {
		if (_device) {
			vkDeviceWaitIdle(_device);
			for (auto &task: _asyncWaitTasksPool) {
				vkDestroyFence(_device, task.fence, nullptr);
			}
			for (auto &task: _asyncWaitTasks) {
				task.cb->resolve(); // Resolve the callback before destroying the fence
				vkDestroyFence(_device, task.fence, nullptr);
			}
			if (_commandPool)
				vkDestroyCommandPool(_device, _commandPool, nullptr);
			if (_pipelineCache)
				vkDestroyPipelineCache(_device, _pipelineCache, nullptr);
			vkDestroyDevice(_device, nullptr);
		}
		if (_instance)
			vkDestroyInstance(_instance, nullptr);
	}

	void VulkanRenderResource::createDevice() {
		uint32_t familyCount = 0;
		Array<VkQueueFamilyProperties> families(_queueFamily + 1);
		vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &familyCount, families.val());
		Qk_ASSERT(_queueFamily < familyCount, "Invalid graphics queue family index");

		uint32_t queueCount = std::min(families[_queueFamily].queueCount, 1u);
		Qk_ASSERT(queueCount > 0, "No queues available in graphics queue family");

		float priorities[] = { 1.0f, 1.0f };
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = _queueFamily;
		queueInfo.queueCount = queueCount;
		queueInfo.pQueuePriorities = priorities;

		VkPhysicalDeviceFeatures supportedFeatures = {};
		vkGetPhysicalDeviceFeatures(_physicalDevice, &supportedFeatures);
		VkPhysicalDeviceFeatures enabledFeatures = {};
		enabledFeatures.textureCompressionETC2 = supportedFeatures.textureCompressionETC2;
		enabledFeatures.textureCompressionBC = supportedFeatures.textureCompressionBC;

		const char *extensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_IMG_FORMAT_PVRTC_EXTENSION_NAME, // PVRTC support
		};
		_pvrtcSupport = vk_supportsDeviceExtension(_physicalDevice, extensions[1]);
		uint32_t extensionCount = _pvrtcSupport ? 2 : 1;
		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.enabledExtensionCount = extensionCount;
		deviceInfo.ppEnabledExtensionNames = extensions;
		deviceInfo.pEnabledFeatures = &enabledFeatures;
		vk_check("vkCreateDevice", vkCreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device));

		vkGetDeviceQueue(_device, _queueFamily, 0, &_commandQueue);
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
			VkFenceCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
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

	VkTexture* VulkanRenderResource::newTexture(
		Vec2 size, ColorType type, uint32_t mipLevels, uint8_t flags
	) {
		VkFormat format = pixelFormat(type);
		if (format == VK_FORMAT_UNDEFINED || size.x() <= 0 || size.y() <= 0)
			return nullptr;
		if (type >= kPVRTCI_2BPP_RGB_ColorType && type <= kPVRTCII_4BPP_ColorType &&
			!_pvrtcSupport)
			return nullptr;

		VkFormatProperties formatProperties = {};
		vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &formatProperties);
		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
			return nullptr;
		if ((flags & kComputeWrite_TextureFlags) &&
			!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
			return nullptr;

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT)
			usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (flags & kComputeWrite_TextureFlags)
			usage |= VK_IMAGE_USAGE_STORAGE_BIT;

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.extent = { uint32_t(size.x()), uint32_t(size.y()), 1 };
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		auto vk_tex = new VkTexture{
			.format = format,
			.extent = { uint32_t(size.x()), uint32_t(size.y()) },
			.mipLevels = mipLevels,
			.usage = usage,
		};
		auto fail = [&]() {
			vk_deleteTexture(_device, vk_tex);
			return nullptr;
		};
		VkResult result;
		vk_call(vkCreateImage, return fail(), _device, &imageInfo, nullptr, &vk_tex->image);

		uint32_t memoryType;
		VkMemoryRequirements requirements = {};
		vkGetImageMemoryRequirements(_device, vk_tex->image, &requirements);
		vk_call(findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo = {};
		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, return fail(), _device, &memoryInfo, nullptr, &vk_tex->memory);
		vk_call(vkBindImageMemory, return fail(), _device, vk_tex->image, vk_tex->memory, 0);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = vk_tex->image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.layerCount = 1;
		vk_call(vkCreateImageView, return fail(), _device, &viewInfo, nullptr, &vk_tex->view);

		return vk_tex;
	}

	VkVertexBuffer* VulkanRenderResource::newVertexBuffer(uint32_t size) {
		auto vertex = new VkVertexBuffer{
			.size = VkDeviceSize(size),
		};
		auto fail = [&]() {
			vk_deleteVertexBuffer(_device, vertex);
			return nullptr;
		};
		VkResult result;
		VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = vertex->size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vk_call(vkCreateBuffer, return fail(), _device, &bufferInfo, nullptr, &vertex->buffer);

		uint32_t memoryType;
		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(_device, vertex->buffer, &requirements);
		vk_call(findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, return fail(), _device, &memoryInfo, nullptr, &vertex->memory);
		vk_call(vkBindBufferMemory, return fail(), _device, vertex->buffer, vertex->memory, 0);

		return vertex;
	}

	TexStat VulkanRenderResource::createTextureStat(Vec2 size, ColorType type, uint8_t flags) {
		if (size.x() <= 0 || size.y() <= 0)
			return TexStat();
		uint32_t levels = flags & kMipmap_TextureFlags ? mipLevelCount(size) : 1;
		return TexStat(newTexture(size, type, levels, flags));
	}

	bool VulkanRenderResource::uploadTexture(Pixel *pix, int levels, TexStat *out, bool mipmap) {
		Qk_ASSERT_GT(levels, 0, "Levels must be greater than 0");
		if (!pix || !pix->length())
			return false;

		VkFormat format = pixelFormat(pix->type());
		if (format == VK_FORMAT_UNDEFINED)
			return false;

		VkDeviceSize uploadSize = 0;
		for (int i = 0; i < levels; i++) {
			if (!pix[i].val() || !pix[i].length())
				return false;
			uploadSize = (uploadSize + 15) & ~VkDeviceSize(15);
			uploadSize += pix[i].length();
		}

		bool generateMipmaps = levels == 1 && mipmap;
		if (generateMipmaps) {
			VkFormatProperties properties = {};
			vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &properties);
			const VkFormatFeatureFlags required =
				VK_FORMAT_FEATURE_BLIT_SRC_BIT |
				VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
			generateMipmaps = (properties.optimalTilingFeatures & required) == required;
		}
		auto imageLevels = generateMipmaps ? mipLevelCount(pix->size()) : uint32_t(levels);
		auto vk_tex = newTexture(pix->size(), pix->type(), imageLevels, 0);
		if (!vk_tex)
			return false;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		auto fail = [&]() {
			if (stagingBuffer)
				vkDestroyBuffer(_device, stagingBuffer, nullptr);
			if (stagingMemory)
				vkFreeMemory(_device, stagingMemory, nullptr);
			if (cmd)
				vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
			vk_deleteTexture(_device, vk_tex);
			return false;
		};

		VkResult result;
		VkBufferCreateInfo bufferInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = uploadSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		vk_call(vkCreateBuffer, return fail(), _device, &bufferInfo, nullptr, &stagingBuffer);

		uint32_t memoryType;
		VkMemoryRequirements requirements = {};
		vkGetBufferMemoryRequirements(_device, stagingBuffer, &requirements);
		vk_call(findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryType
		);
		void *mapped;
		VkMemoryAllocateInfo memoryInfo = {};
		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, return fail(), _device, &memoryInfo, nullptr, &stagingMemory);
		vk_call(vkBindBufferMemory, return fail(), _device, stagingBuffer, stagingMemory, 0);
		vk_call(vkMapMemory, return fail(), _device, stagingMemory, 0, uploadSize, 0, &mapped);

		VkDeviceSize offset = 0;
		for (int i = 0; i < levels; i++) {
			offset = (offset + 15) & ~VkDeviceSize(15);
			memcpy(static_cast<uint8_t*>(mapped) + offset, pix[i].val(), pix[i].length());
			offset += pix[i].length();
		}
		vkUnmapMemory(_device, stagingMemory);

		ScopeLock lock(_mutex); // Lock the mutex to ensure thread safety during texture upload

		vk_call(vk_beginCommandBuffer, return fail(), _device, _commandPool, &cmd);
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = vk_tex->image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = imageLevels;
		barrier.subresourceRange.layerCount = 1;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		offset = 0;
		for (int i = 0; i < levels; i++) {
			offset = (offset + 15) & ~VkDeviceSize(15); // align to 16 bytes for optimal transfer
			VkBufferImageCopy copy = {};
			copy.bufferOffset = offset;
			copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copy.imageSubresource.mipLevel = i;
			copy.imageSubresource.layerCount = 1;
			copy.imageExtent = { uint32_t(pix[i].width()), uint32_t(pix[i].height()), 1 };
			vkCmdCopyBufferToImage(cmd,
				stagingBuffer, vk_tex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
			offset += pix[i].length();
		}

		if (generateMipmaps) {
			vk_tex->generateMipmaps(cmd);
		} else {
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = imageLevels;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		vk_call(vkEndCommandBuffer, return fail(), cmd);
		vk_call(vk_submitCommand, return fail(), &cmd, Cb([this,stagingBuffer,stagingMemory,cmd](auto e) {
			ScopeLock lock(_mutex);
			vkDestroyBuffer(_device, stagingBuffer, nullptr);
			vkFreeMemory(_device, stagingMemory, nullptr);
			vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
		}));

		VulkanRenderResource::unloadTexture(out);
		out->set_ptr(vk_tex);
		return true;
	}

	bool VulkanRenderResource::uploadVertexData(VertexData::ID *id) {
		if (id->ptr)
			return true;

		auto &data = id->data->vertex;
		if (!data.length())
			return false;

		auto vertex = newVertexBuffer(data.size());
		if (!vertex)
			return false;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		auto fail = [&]() {
			if (stagingBuffer)
				vkDestroyBuffer(_device, stagingBuffer, nullptr);
			if (stagingMemory)
				vkFreeMemory(_device, stagingMemory, nullptr);
			if (cmd)
				vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
			vk_deleteVertexBuffer(_device, vertex);
			return false;
		};

		VkResult result;
		VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = vertex->size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vk_call(vkCreateBuffer, return fail(), _device, &bufferInfo, nullptr, &stagingBuffer);

		uint32_t memoryType;
		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(_device, stagingBuffer, &requirements);
		vk_call(findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, return fail(), _device, &memoryInfo, nullptr, &stagingMemory);
		vk_call(vkBindBufferMemory, return fail(), _device, stagingBuffer, stagingMemory, 0);

		void *mapped = nullptr;
		vk_call(vkMapMemory, return fail(), _device, stagingMemory, 0, vertex->size, 0, &mapped);
		memcpy(mapped, data.val(), data.size());
		vkUnmapMemory(_device, stagingMemory);

		ScopeLock lock(_mutex);

		vk_call(vk_beginCommandBuffer, return fail(), _device, _commandPool, &cmd);

		VkBufferCopy copy{.size = vertex->size};
		vkCmdCopyBuffer(cmd, stagingBuffer, vertex->buffer, 1, &copy);

		VkBufferMemoryBarrier barrier{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = vertex->buffer;
		barrier.size = VK_WHOLE_SIZE;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
			0, 0, nullptr, 1, &barrier, 0, nullptr);

		vk_call(vkEndCommandBuffer, return fail(), cmd);
		vk_call(vk_submitCommand, return fail(), &cmd,
			Cb([this, stagingBuffer, stagingMemory, cmd](auto e) {
				ScopeLock lock(_mutex);
				vkDestroyBuffer(_device, stagingBuffer, nullptr);
				vkFreeMemory(_device, stagingMemory, nullptr);
				vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
			})
		);
		id->ptr = vertex;
		return true;
	}

	void VulkanRenderResource::unloadTexture(TexStat *tex) {
		auto *vk_tex = static_cast<VkTexture*>(tex->ptr());
		if (!vk_tex)
			return;
		// We cannot delete the texture immediately, because it may be in use by the GPU. Instead,
		// we schedule a delayed task to delete the texture after the GPU has finished using it.
		add_delay_task_for_app(Cb([this, vk_tex](auto e) {
			vk_deleteTexture(_device, vk_tex);
		}), false);
		tex->set_ptr(nullptr);
	}

	void VulkanRenderResource::unloadVertexData(VertexData::ID *id) {
		auto vertex = static_cast<VkVertexBuffer*>(id->ptr);
		if (!vertex)
			return;
		add_delay_task_for_app(Cb([this, vertex](auto e) {
			vk_deleteVertexBuffer(_device, vertex);
		}), false);
		id->ptr = nullptr;
	}

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
