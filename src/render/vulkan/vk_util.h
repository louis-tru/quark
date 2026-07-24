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

#include "../../util/cb.h"
#include "../mem_allocator.h"
#include "../pixel.h"
#include "../paint.h"
#include "../source.h"
#include "./vk_shader.h"

#define vk_call(call, fail, ...) \
	result = call(__VA_ARGS__); if (result != VK_SUCCESS) fail

namespace qk {
	class VulkanRender;
	struct VkCmdPack;

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

	template<>
	void ObjectTraitsBase<VkTexture>::Release(VkTexture* tex);

	struct VkVertexBuffer {
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkDeviceSize size = 0;
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
	constexpr uint32_t kSafeDeleteVkMemBlock_Flag = 1u;

	extern uint32_t vk_uniformBufferAlignment;

	void vk_delayTask(Cb cb);

	inline void vk_check(const char *call, VkResult result) {
		Qk_CHECK(result == VK_SUCCESS, "%s failed: %d", call, int(result));
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

	bool vk_supportsDeviceExtension(VkPhysicalDevice device, const char *extension);

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

	VkFormat vk_pixelFormat(ColorType type);

	VkResult vk_findMemoryType(
		VkPhysicalDevice physicalDevice, uint32_t typeBits,
		VkMemoryPropertyFlags flags, uint32_t *typeIndex
	);

	// A simple Vulkan application that draws a single color to the screen.
	VkResult vk_beginCommandBuffer(VkDevice device, VkCommandPool pool, VkCommandBuffer *cmd);

	VkResult vk_submitCommand(const VkCommandBuffer* cmd, Cb cb);

	void vk_deleteTexture(VkDevice device, VkTexture *tex);
	void vk_deleteTextureSafe(VkDevice device, VkTexture *tex);
	void vk_deleteVertexBuffer(VkDevice device, VkVertexBuffer *vertex);
	void vk_deleteVertexBufferSafe(VkDevice device, VkVertexBuffer *vertex);
	void vk_deleteImageView(VkDevice device, VkImageView view);
	void vk_deleteFramebufferSafe(VkDevice device, VkFramebuffer framebuffer);

	cVkMemBlock& makeBuffer(VkCmdPack &cmd, const void *src, uint32_t size, uint32_t reserve = 0);

	VkDescriptorBufferInfo makeBufferInfo(VkCmdPack &cmd, const void *src, uint32_t size, uint32_t reserve = 0);

	template<typename T>
	cVkMemBlock& makeBufferT(VkCmdPack &cmd, const T *src, uint32_t length = 1) {
		return makeBuffer(cmd, src, length * sizeof(T), sizeof(T));
	}

	template<typename T>
	VkDescriptorBufferInfo makeBufferInfoT(VkCmdPack &cmd, const T *src, uint32_t length = 1) {
		return makeBufferInfo(cmd, src, length * sizeof(T), sizeof(T));
	}

} // namespace qk
#endif
