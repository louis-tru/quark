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

#ifndef __quark_render_vulkan_vk_util__
#define __quark_render_vulkan_vk_util__

#include "../mem_allocator.h"
#include "../pixel.h"
#include "../paint.h"
#include "../source.h"
#include "./vk_shader.h"

#define vk_call(call, ...) \
	result = call(__VA_ARGS__); if (result != VK_SUCCESS) return fail()
#define vk_call_if(call, ...) if (!call(__VA_ARGS__)) return fail()

namespace qk {
	class VulkanRenderResource;
	class VulkanRender;
	struct VkCmdPack;
	struct VkMemoryAllocator;
	struct VkMemory;
	typedef const VkMemory cVkMemory;
	struct VkTexture;

	inline void vk_check(const char *call, VkResult result) {
		Qk_CHECK(result == VK_SUCCESS, "%s failed: %d", call, int(result));
	}

	struct VkRef {
		std::atomic_int refCount{0}; // ownership is added explicitly when the handle is published
		virtual ~VkRef() = default;
		void ref();
		void unref();
	};

	struct VkTextureLevelInfo {
	private:
		std::atomic<VkImageView> view;
		std::atomic<VkFramebuffer> framebuffer;
	public:
		VkImageLayout layout;
		VkTextureLevelInfo(VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED)
			: layout(layout), view(VK_NULL_HANDLE), framebuffer(VK_NULL_HANDLE) {}
		VkTextureLevelInfo(VkTextureLevelInfo&& value)
			: VkTextureLevelInfo(value.layout) {}
		VkTextureLevelInfo(const VkTextureLevelInfo& value)
			: VkTextureLevelInfo(value.layout) {}
		friend class VkTexture;
	};

	struct VkTexture: VkRef {
		VkImage image = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		cVkMemory *memory = nullptr;
		VkExtent2D extent;
		VkFormat format;
		VkImageUsageFlags usage;
		Array<VkTextureLevelInfo> levels;
		~VkTexture() override;
		VkImageView levelView(uint32_t level = 0);
		VkFramebuffer framebuffer(uint32_t level = 0);
		uint32_t mipLevels() const { return levels.length(); }
		Vec2 size() const { return Vec2(extent.width, extent.height); }
		bool transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout,
			uint32_t level = 0, uint32_t levelCount = 1);
		void generateMipmaps(VkCommandBuffer cmd);
	};

	template <>
	struct ObjectTraits<VkTexture>: ObjectTraitsBase<VkTexture> {
		static constexpr bool isRef = true;
		inline static void Retain(VkTexture* tex) {
			if (tex)
				tex->ref();
		}
		inline static void Release(VkTexture* tex) {
			if (tex)
				tex->unref();
		}
	};

	template<>
	inline void ObjectTraitsBase<VkFramebuffer_T>::Retain(VkFramebuffer obj) {}
	template<>
	inline void ObjectTraitsBase<VkFramebuffer_T>::Release(VkFramebuffer obj) {
		Qk_ASSERT(0, "Disable Release VkFramebuffer from ObjectTraitsBase, use vkDestroyFramebuffer instead");
	}
	template<>
	inline void ObjectTraitsBase<VkImageView_T>::Retain(VkImageView obj) {}
	template<>
	inline void ObjectTraitsBase<VkImageView_T>::Release(VkImageView obj) {
		Qk_ASSERT(0, "Disable Release VkImageView from ObjectTraitsBase, use vkDestroyImageView instead");
	}

	struct VkVertexBuffer: VkRef {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize size = 0;
		~VkVertexBuffer() override;
	};

	struct VkSubmitResult {
		VkFence fence;
		VkResult result;
		std::atomic_int refCount;
		std::atomic_bool completed;
		VkSubmitResult(VkFence fence, VkResult result, int refCount, bool completed)
			: fence(fence), result(result), refCount(refCount), completed(completed) {}
		VkSubmitResult(VkSubmitResult&& value)
			: fence(value.fence)
			, result(value.result)
			, refCount(value.refCount.load(std::memory_order_relaxed))
			, completed(value.completed.load(std::memory_order_relaxed)) {}
		VkSubmitResult(const VkSubmitResult&) = delete;
		inline void ref() {
			refCount.fetch_add(1, std::memory_order_relaxed);
		}
		inline void unref() {
			refCount.fetch_sub(1, std::memory_order_relaxed);
		}
	};

	struct VkMemBuffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void *mapped = nullptr;
		VkMemoryPropertyFlags properties = 0;
	};

	typedef MemBlockAllocator<VkMemBuffer*> VkMemBufferAllocator;
	typedef VkMemBufferAllocator::MemBlock VkMemBlock;
	typedef const VkMemBlock cVkMemBlock;

	extern uint32_t vk_minBufferAlignment;

	inline VkTexture* vk_cast_texture(cTexStat *texStat) {
		return static_cast<VkTexture*>(texStat->ptr());
	}

	inline VkTexture* vk_get_texture(const ImageSource* src, uint32_t index = 0) {
		return static_cast<VkTexture*>(src->texture(index)->ptr());
	}

	inline VkTexture* vk_get_texture_from(const ImageSource* src, VkTexture* _else = nullptr) {
		return src ? static_cast<VkTexture*>(src->texture(0)->ptr()) : _else;
	}

	bool vk_createInstance(VkInstance *instance);

	bool vk_selectBestDevice(
		VkInstance instance, VkPhysicalDevice *selectedDevice,
		uint32_t *selectedQueueFamily, bool *selectedComputeSupport
	);

	void vk_logDeviceInfo(VkPhysicalDevice device, const VkPhysicalDeviceProperties &properties);

	bool vk_createDevice(VkPhysicalDevice physicalDevice,
		const VkPhysicalDeviceProperties &properties,
		uint32_t queueFamily, VkDevice *device, bool *pvrtcSupport,
		bool *capaSupport, uint32_t *capaMaxImageCount);

	uint32_t vk_mipLevelCount(Vec2 size);

	VkPipelineColorBlendAttachmentState vk_blend_state(BlendMode mode);

	uint64_t vk_pipeline_key(VkPipelineKind kind, BlendMode mode, VkFormat format);

	uint32_t vk_sampler_key(const PaintImage* paint);

	VkSamplerAddressMode vk_sampler_address_mode(PaintImage::TileMode mode);

	VkFilter vk_sampler_mag_filter(PaintImage::FilterMode filter);

	void vk_set_sampler_min_mip_filter(VkSamplerCreateInfo *info, PaintImage::MipmapMode mode);

	VkRenderPass vk_create_pipeline_render_pass(VkDevice device, VkFormat format);

	VkRenderPass vk_create_render_pass(
		VkDevice device, VkFormat format, VkAttachmentLoadOp loadOp,
		VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	);

	VkFramebuffer vk_create_framebuffer(
		VkDevice device, VkRenderPass renderPass, VkImageView view, VkExtent2D extent
	);

	VkImageView vk_createLevelView(VkDevice device, const VkTexture *texture, uint32_t level);

	cTexStat* vk_rebuild_texture(Vec2 size, ColorType type, cTexStat* texStat,
		TexStat &storeStat, uint8_t flags);

	VkFormat vk_pixelFormat(ColorType type);

	VkResult vk_findMemoryType(
		VkPhysicalDevice physicalDevice, uint32_t typeBits,
		VkMemoryPropertyFlags flags, uint32_t *typeIndex
	);

	// A simple Vulkan application that draws a single color to the screen.
	VkResult vk_beginCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer *cmd,
		VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	cVkMemBlock& makeBuffer(VkCmdPack *cmd, const void *src, uint32_t size, uint32_t reserve = 0);

	template<typename T>
	cVkMemBlock& makeBufferT(VkCmdPack *cmd, const T *src, uint32_t length = 1) {
		return makeBuffer(cmd, src, length * sizeof(T), sizeof(T));
	}

	VkDescriptorBufferInfo makeBufferInfo(VkCmdPack *cmd, const void *src, uint32_t size, uint32_t reserve = 0);

	template<typename T>
	VkDescriptorBufferInfo makeBufferInfoT(VkCmdPack *cmd, const T *src, uint32_t length = 1) {
		return makeBufferInfo(cmd, src, length * sizeof(T), sizeof(T));
	}

} // namespace qk
#endif
