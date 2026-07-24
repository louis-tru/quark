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

namespace qk {
	uint32_t vk_sampler_key(const PaintImage* paint) {
		constexpr uint32_t bitfields =
			(0b11 << 8) |  // tile mode x
			(0b11 << 10) | // tile mode y
			(0b1 << 12) |  // filter mode
			(0b111 << 13); // mipmap mode
		return bitfields & paint->bitfields;
	}

	uint64_t vk_pipeline_key(VkPipelineKind kind, BlendMode mode, VkFormat format) {
		return (uint64_t(kind) << 48) | (uint64_t(mode) << 32) | format;
	}

	VkSamplerAddressMode vk_sampler_address_mode(PaintImage::TileMode mode) {
		switch (mode) {
			case PaintImage::kClamp_TileMode: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case PaintImage::kRepeat_TileMode: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case PaintImage::kMirror_TileMode: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case PaintImage::kDecal_TileMode: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		}
		Qk_ASSERT(0, "Invalid Vulkan sampler address mode");
		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	}

	VkFilter vk_sampler_mag_filter(PaintImage::FilterMode filter) {
		switch (filter) {
			case PaintImage::kNearest_FilterMode: return VK_FILTER_NEAREST;
			case PaintImage::kLinear_FilterMode: return VK_FILTER_LINEAR;
		}
		Qk_ASSERT(0, "Invalid Vulkan sampler filter");
		return VK_FILTER_NEAREST;
	}

	void vk_set_sampler_min_mip_filter(VkSamplerCreateInfo *info, PaintImage::MipmapMode mode) {
		info->minLod = 0.0f;
		info->maxLod = VK_LOD_CLAMP_NONE;
		switch (mode) {
			case PaintImage::kNone_MipmapMode:
				info->minFilter = VK_FILTER_NEAREST;
				info->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				info->maxLod = 0.0f;
				break;
			case PaintImage::kNearest_MipmapMode:
				info->minFilter = VK_FILTER_NEAREST;
				info->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				break;
			case PaintImage::kLinearNearest_MipmapMode:
				info->minFilter = VK_FILTER_LINEAR;
				info->mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				break;
			case PaintImage::kNearestLinear_MipmapMode:
				info->minFilter = VK_FILTER_NEAREST;
				info->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				break;
			case PaintImage::kLinear_MipmapMode:
				info->minFilter = VK_FILTER_LINEAR;
				info->mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				break;
		}
	}

	template<>
	MemBlockAllocator<VkMemBuffer*>::MemBlock*
	MemBlockAllocator<VkMemBuffer*>::createBlock(uint32_t capacity) {
		auto resource = getSharedRenderVulkanResource();
		auto device = resource->device();
		auto buffer = new VkMemBuffer();

		VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = capacity;
		bufferInfo.usage =
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		Qk_ASSERT_EQ(VK_SUCCESS,
			vkCreateBuffer(device, &bufferInfo, nullptr, &buffer->buffer),
			"Failed to create Vulkan memory block buffer");

		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(device, buffer->buffer, &requirements);
		uint32_t memoryType = 0;
		auto memoryFlags = VkMemoryPropertyFlags(_flags);
		if (!memoryFlags) {
			memoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		}
		auto preferredFlags = memoryFlags;
		if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		auto result = vk_findMemoryType(
			resource->physicalDevice(), requirements.memoryTypeBits,
			preferredFlags,
			&memoryType
		);
		if (result != VK_SUCCESS && preferredFlags != memoryFlags) {
			result = vk_findMemoryType(
				resource->physicalDevice(), requirements.memoryTypeBits,
				memoryFlags,
				&memoryType
			);
		}
		Qk_ASSERT_EQ(VK_SUCCESS, result, "No compatible Vulkan memory type available");
		VkPhysicalDeviceMemoryProperties memoryProperties{};
		vkGetPhysicalDeviceMemoryProperties(resource->physicalDevice(), &memoryProperties);
		buffer->properties = memoryProperties.memoryTypes[memoryType].propertyFlags;

		VkMemoryAllocateInfo memoryInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		Qk_ASSERT_EQ(VK_SUCCESS,
			vkAllocateMemory(device, &memoryInfo, nullptr, &buffer->memory),
			"Failed to allocate Vulkan memory block");
		Qk_ASSERT_EQ(VK_SUCCESS,
			vkBindBufferMemory(device, buffer->buffer, buffer->memory, 0),
			"Failed to bind Vulkan memory block");
		if (buffer->properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
			Qk_ASSERT_EQ(VK_SUCCESS,
				vkMapMemory(device, buffer->memory, 0, VK_WHOLE_SIZE, 0, &buffer->mapped),
				"Failed to map Vulkan memory block");
		}

		return new MemBlock(buffer, capacity);
	}

	template<>
	void MemBlockAllocator<VkMemBuffer*>::deleteBlock(MemBlock *block, uint32_t flags) {
		auto device = getSharedRenderVulkanResource()->device();
		auto buffer = block->val;
		if (buffer->mapped)
			vkUnmapMemory(device, buffer->memory);
		if (flags & kSafeDeleteVkMemBlock_Flag) {
			vk_delayTask(Cb([device, buffer=buffer->buffer, memory=buffer->memory](auto) {
				if (buffer)
					vkDestroyBuffer(device, buffer, nullptr);
				if (memory)
					vkFreeMemory(device, memory, nullptr);
			}));
		} else {
			if (buffer->buffer)
				vkDestroyBuffer(device, buffer->buffer, nullptr);
			if (buffer->memory)
				vkFreeMemory(device, buffer->memory, nullptr);
		}
		delete buffer;
		delete block;
	}

	uint32_t vk_mipLevelCount(Vec2 size) {
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

	VkResult vk_findMemoryType(
		VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags flags, uint32_t *typeIndex
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

	VkFormat vk_pixelFormat(ColorType type) {
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

	VkPipelineColorBlendAttachmentState vk_blend_state(BlendMode mode) {
		VkPipelineColorBlendAttachmentState state{};
		state.blendEnable = VK_TRUE;
		state.colorBlendOp = VK_BLEND_OP_ADD;
		state.alphaBlendOp = VK_BLEND_OP_ADD;
		state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		switch (mode) {
			case kClear_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				break;
			case kSrc_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				break;
			case kSrcOver_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			case kDst_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				break;
			case kDstOver_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				break;
			case kSrcIn_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				break;
			case kDstIn_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				break;
			case kSrcOut_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				break;
			case kDstOut_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			case kSrcATop_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			case kDstATop_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				break;
			case kXor_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			case kPlus_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				break;
			case kSrcOverLegacy_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			case kPlusLegacy_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				break;
			case kModulateLegacy_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
				break;
			case kScreenLegacy_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			case kMultiplyLegacy_BlendMode:
				state.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
				state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				state.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
				state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				break;
			default:
				state.blendEnable = VK_FALSE;
				break;
		}
		return state;
	}

	VkRenderPass vk_create_render_pass(
		VkDevice device, VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
		VkImageLayout initialLayout, VkImageLayout finalLayout
	) {
		VkAttachmentDescription attachment{};
		attachment.format = format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = loadOp;
		attachment.storeOp = storeOp;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = initialLayout;
		attachment.finalLayout = finalLayout;

		VkAttachmentReference colorAttachment{};
		colorAttachment.attachment = 0;
		colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachment;

		VkRenderPassCreateInfo info{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;

		VkRenderPass renderPass = VK_NULL_HANDLE;
		auto result = vkCreateRenderPass(device, &info, nullptr, &renderPass);
		Qk_ASSERT_EQ(result, VK_SUCCESS, "Failed to create Vulkan render pass");
		return renderPass;
	}

	VkRenderPass vk_create_pipeline_render_pass(VkDevice device, VkFormat format) {
		return vk_create_render_pass(
			device, format, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
	}

	VkFramebuffer vk_create_framebuffer(
		VkDevice device, VkRenderPass renderPass, VkImageView view, VkExtent2D extent
	) {
		VkFramebufferCreateInfo info{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
		info.renderPass = renderPass;
		info.attachmentCount = 1;
		info.pAttachments = &view;
		info.width = extent.width;
		info.height = extent.height;
		info.layers = 1;
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		auto result = vkCreateFramebuffer(device, &info, nullptr, &framebuffer);
		Qk_ASSERT_EQ(result, VK_SUCCESS, "Failed to create Vulkan framebuffer");
		return framebuffer;
	}

	VkImageView vk_createLevelView(VkDevice device, const VkTexture *texture, uint32_t level) {
		Qk_ASSERT(texture && level < texture->mipLevels, "Invalid Vulkan texture mip level");
		if (!texture || level >= texture->mipLevels)
			return VK_NULL_HANDLE;

		VkImageViewCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		info.image = texture->image;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.format = texture->format;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = level;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;

		VkImageView view = VK_NULL_HANDLE;
		auto result = vkCreateImageView(device, &info, nullptr, &view);
		Qk_ASSERT_EQ(result, VK_SUCCESS, "Failed to create Vulkan mip level image view");
		return view;
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

	void add_delay_task_for_app(Cb cb, bool recursion);

	void vk_delayTask(Cb cb) {
		add_delay_task_for_app(cb, false);
	}

	void vk_deleteTextureSafe(VkDevice device, VkTexture *tex) {
		vk_delayTask(Cb([device, tex](auto e) {
			vk_deleteTexture(device, tex);
		}));
	}

	void vk_deleteVertexBufferSafe(VkDevice device, VkVertexBuffer *vertex) {
		vk_delayTask(Cb([device, vertex](auto e) {
			vk_deleteVertexBuffer(device, vertex);
		}));
	}

	void vk_deleteImageView(VkDevice device, VkImageView view) {
		vk_delayTask(Cb([device, view](auto e) {
			vkDestroyImageView(device, view, nullptr);
		}));
	}

	void vk_deleteFramebufferSafe(VkDevice device, VkFramebuffer framebuffer) {
		vk_delayTask(Cb([device, framebuffer](auto e) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}));
	}

	template<>
	void ObjectTraitsBase<VkTexture>::Release(VkTexture* tex) {
		vk_deleteTexture(getSharedRenderVulkanResource()->device(), tex);
	}

	cVkMemBlock& makeBuffer(VkCmdPack &cmd, const void *src, uint32_t size, uint32_t reserve) {
		auto &block = cmd.allocator[0]->alloc(size, reserve, vk_uniformBufferAlignment);
		Qk_ASSERT(block.end >= block.begin + size, "Not enough space in buffer block");
		if (size)
			memcpy((char*)block.val->mapped + block.begin, src, size);
		return block;
	}

	VkDescriptorBufferInfo makeBufferInfo(VkCmdPack &cmd, const void *src, uint32_t size, uint32_t reserve) {
		cVkMemBlock &block = makeBuffer(cmd, src, size, reserve);
		return {
			.buffer = block.val->buffer,
			.offset = block.begin,
			.range = size,
		};
	}

} // namespace qk
