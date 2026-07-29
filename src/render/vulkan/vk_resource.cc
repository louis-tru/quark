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

	VulkanRenderResource* getSharedRenderVulkanResource() {
		static VulkanRenderResource *resource = new VulkanRenderResource();
		return resource;
	}

	RenderResource* getSharedRenderResource() {
		return getSharedRenderVulkanResource();
	}

	cTexStat* vk_rebuild_texture(Vec2 size, ColorType type, cTexStat* texStat,
		TexStat &storeStat, uint8_t flags)
	{
		auto fmt = vk_pixelFormat(type);
		auto tex = vk_cast_texture(texStat);
		if (fmt == VK_FORMAT_UNDEFINED)
			return nullptr;
		auto mipmap = flags & kMipmap_TextureFlags;
		if (!tex ||
				tex->size() != size ||
				tex->format != fmt ||
				(mipmap && tex->mipLevels <= 1)
		) {
			uint32_t mipLevels = mipmap ? vk_mipLevelCount(size): 1;
			tex = getSharedRenderVulkanResource()->newTexture(size, type, mipLevels, flags);
			if (!tex)
				return nullptr;
			texStat = &storeStat;
			storeStat.set_ptr(tex);
		}
		return texStat;
	}

	VkTexture::~VkTexture() {
		auto device = getSharedRenderVulkanResource()->device();
		if (view)
			vkDestroyImageView(device, view, nullptr);
		if (image)
			vkDestroyImage(device, image, nullptr);
		if (memory)
			vkFreeMemory(device, memory, nullptr);
	}

	VkVertexBuffer::~VkVertexBuffer() {
		auto device = getSharedRenderVulkanResource()->device();
		if (buffer)
			vkDestroyBuffer(device, buffer, nullptr);
		if (memory)
			vkFreeMemory(device, memory, nullptr);
	}

	void vk_layout_access(VkImageLayout layout,
		VkPipelineStageFlags &stage, VkAccessFlags &access)
	{
		switch (layout) {
			case VK_IMAGE_LAYOUT_UNDEFINED:
				stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				access = 0;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				access = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				access = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				access = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				access = VK_ACCESS_SHADER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_GENERAL:
				stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				access = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
				break;
			default:
				Qk_ASSERT(false, "Unsupported Vulkan image layout transition: %d", layout);
				stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
				access = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
				break;
		}
	}

	bool VkTexture::transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout,
		uint32_t level, uint32_t levelCount)
	{
		Qk_ASSERT(level < mipLevels && levelCount && level + levelCount <= mipLevels,
			"Invalid Vulkan texture mip range");
		bool changed = false;
		auto end = level + levelCount;
		while (level < end) {
			auto oldLayout = layouts[level];
			if (oldLayout == newLayout) {
				level++;
				continue;
			}
			auto runEnd = level + 1;
			while (runEnd < end && layouts[runEnd] == oldLayout)
				runEnd++;

			VkPipelineStageFlags srcStage, dstStage;
			VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
			vk_layout_access(oldLayout, srcStage, barrier.srcAccessMask);
			vk_layout_access(newLayout, dstStage, barrier.dstAccessMask);
			barrier.oldLayout = oldLayout;
			barrier.newLayout = newLayout;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = level;
			barrier.subresourceRange.levelCount = runEnd - level;
			barrier.subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(cmd, srcStage, dstStage,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
			for (auto i = level; i < runEnd; i++)
				layouts[i] = newLayout;
			changed = true;
			level = runEnd;
		}
		return changed;
	}

	void VkTexture::generateMipmaps(VkCommandBuffer cmd) {
		if (mipLevels == 1) {
			transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			return;
		}

		int32_t width = int32_t(extent.width);
		int32_t height = int32_t(extent.height);
		for (uint32_t i = 1; i < mipLevels; i++) {
			transitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i - 1);
			transitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, i);

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

			transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i - 1);
			width = std::max(width >> 1, 1);
			height = std::max(height >> 1, 1);
		}
		transitionLayout(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels - 1);
	}

	VkSampler VulkanRenderResource::get_sampler(const PaintImage* paint) {
		ScopeLock lock(_mutex);
		auto key = vk_sampler_key(paint);
		VkSampler sampler;
		if (_samplers.get(key, sampler))
			return sampler;

		VkSamplerCreateInfo info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
		info.magFilter = vk_sampler_mag_filter(paint->filterMode);
		vk_set_sampler_min_mip_filter(&info, paint->mipmapMode);
		info.addressModeU = vk_sampler_address_mode(paint->tileModeX);
		info.addressModeV = vk_sampler_address_mode(paint->tileModeY);
		info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		Qk_ASSERT_EQ(VK_SUCCESS, vkCreateSampler(_device, &info, nullptr, &sampler),
			"Failed to create Vulkan sampler");
		_samplers.set(key, sampler);
		return sampler;
	}

	VkSampler VulkanRenderResource::get_sampler(
		PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap
	) {
		PaintImage image;
		image.tileModeX = PaintImage::kDecal_TileMode;
		image.tileModeY = PaintImage::kDecal_TileMode;
		image.filterMode = filter;
		image.mipmapMode = mipmap;
		return get_sampler(&image);
	}

	VkTexture* VulkanRenderResource::newTexture(Vec2 size, ColorType type, uint32_t mipLevels,
		uint8_t flags, VkImageLayout initialLayout)
	{
		VkFormat format = vk_pixelFormat(type);
		if (format == VK_FORMAT_UNDEFINED || size.x() <= 0 || size.y() <= 0)
			return nullptr;
		if (type >= kPVRTCI_2BPP_RGB_ColorType && type <= kPVRTCII_4BPP_ColorType && !_pvrtcSupport)
			return nullptr;

		VkFormatProperties formatProperties{};
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

		VkImageCreateInfo imageInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
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

		VkCommandBuffer cmd = VK_NULL_HANDLE;
		auto tex = new VkTexture();
		tex->extent = { uint32_t(size.x()), uint32_t(size.y()) };
		tex->format = format;
		tex->mipLevels = mipLevels;
		tex->usage = usage;
		tex->layouts = Array<VkImageLayout>(mipLevels);
		for (auto &layout: tex->layouts)
			layout = VK_IMAGE_LAYOUT_UNDEFINED;
		auto fail = [&]() {
			if (cmd)
				vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
			tex->unref();
			return nullptr;
		};
		VkResult result;
		vk_call(vkCreateImage, _device, &imageInfo, nullptr, &tex->image);

		uint32_t memoryType;
		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(_device, tex->image, &requirements);
		vk_call(vk_findMemoryType, _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, _device, &memoryInfo, nullptr, &tex->memory);
		vk_call(vkBindImageMemory, _device, tex->image, tex->memory, 0);

		VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		viewInfo.image = tex->image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.layerCount = 1;
		vk_call(vkCreateImageView, _device, &viewInfo, nullptr, &tex->view);

		if (initialLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			ScopeLock lock(_mutex);
			vk_call(vk_beginCommandBuffer, _device, _commandPool, &cmd);
			tex->transitionLayout(cmd, initialLayout, 0, mipLevels);
			vk_call(vkEndCommandBuffer, cmd);
			vk_call(vk_submitCommand, &cmd, Cb([this,cmd](auto e) {
				ScopeLock lock(_mutex);
				vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
			}));
		}
		return tex;
	}

	TexStat VulkanRenderResource::createTextureStat(Vec2 size, ColorType type, uint8_t flags) {
		if (size.x() <= 0 || size.y() <= 0)
			return TexStat();
		uint32_t levels = flags & kMipmap_TextureFlags ? vk_mipLevelCount(size) : 1;
		return TexStat(newTexture(size, type, levels, flags));
	}

	bool VulkanRenderResource::uploadTexture(Pixel *pix, int levels, TexStat *out, bool mipmap) {
		Qk_ASSERT_GT(levels, 0, "Levels must be greater than 0");
		if (!pix || !pix->length())
			return false;

		VkFormat format = vk_pixelFormat(pix->type());
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
		auto imageLevels = generateMipmaps ? vk_mipLevelCount(pix->size()) : uint32_t(levels);
		auto vkTex = newTexture(pix->size(), pix->type(), imageLevels, 0);
		if (!vkTex)
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
			vkTex->unref();
			return false;
		};

		VkResult result;
		VkBufferCreateInfo bufferInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = uploadSize,
			.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		};
		vk_call(vkCreateBuffer, _device, &bufferInfo, nullptr, &stagingBuffer);

		uint32_t memoryType;
		VkMemoryRequirements requirements = {};
		vkGetBufferMemoryRequirements(_device, stagingBuffer, &requirements);
		vk_call(vk_findMemoryType, _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryType
		);
		void *mapped;
		VkMemoryAllocateInfo memoryInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, _device, &memoryInfo, nullptr, &stagingMemory);
		vk_call(vkBindBufferMemory, _device, stagingBuffer, stagingMemory, 0);
		vk_call(vkMapMemory, _device, stagingMemory, 0, uploadSize, 0, &mapped);

		VkDeviceSize offset = 0;
		for (int i = 0; i < levels; i++) {
			offset = (offset + 15) & ~VkDeviceSize(15);
			memcpy(static_cast<uint8_t*>(mapped) + offset, pix[i].val(), pix[i].length());
			offset += pix[i].length();
		}
		vkUnmapMemory(_device, stagingMemory);

		ScopeLock lock(_mutex); // Lock the mutex to ensure thread safety during texture upload

		vk_call(vk_beginCommandBuffer, _device, _commandPool, &cmd);
		vkTex->transitionLayout(cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, generateMipmaps ? 1: imageLevels);

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
				stagingBuffer, vkTex->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);
			offset += pix[i].length();
		}

		if (generateMipmaps) {
			vkTex->generateMipmaps(cmd);
		} else {
			vkTex->transitionLayout(cmd,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0, imageLevels);
		}

		vk_call(vkEndCommandBuffer, cmd);
		vk_call(vk_submitCommand, &cmd, Cb([this,stagingBuffer,stagingMemory,cmd](auto e) {
			ScopeLock lock(_mutex);
			vkDestroyBuffer(_device, stagingBuffer, nullptr);
			vkFreeMemory(_device, stagingMemory, nullptr);
			vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
		}));

		VulkanRenderResource::unloadTexture(out);
		out->set_ptr(vkTex);
		return true;
	}

	void VulkanRenderResource::unloadTexture(TexStat *tex) {
		auto *vk_tex = vk_cast_texture(tex);
		if (!vk_tex)
			return;
		vk_tex->unref();
		tex->set_ptr(nullptr);
	}

	VkVertexBuffer* VulkanRenderResource::newVertexBuffer(uint32_t size) {
		auto vertex = new VkVertexBuffer();
		vertex->size = VkDeviceSize(size);
		auto fail = [&]() {
			vertex->unref();
			return nullptr;
		};
		VkResult result;
		VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = vertex->size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vk_call(vkCreateBuffer, _device, &bufferInfo, nullptr, &vertex->buffer);

		uint32_t memoryType;
		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(_device, vertex->buffer, &requirements);
		vk_call(vk_findMemoryType, _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, _device, &memoryInfo, nullptr, &vertex->memory);
		vk_call(vkBindBufferMemory, _device, vertex->buffer, vertex->memory, 0);

		return vertex;
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
			vertex->unref();
			return false;
		};

		VkResult result;
		VkBufferCreateInfo bufferInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
		bufferInfo.size = vertex->size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vk_call(vkCreateBuffer, _device, &bufferInfo, nullptr, &stagingBuffer);

		uint32_t memoryType;
		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(_device, stagingBuffer, &requirements);
		vk_call(vk_findMemoryType, _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, _device, &memoryInfo, nullptr, &stagingMemory);
		vk_call(vkBindBufferMemory, _device, stagingBuffer, stagingMemory, 0);

		void *mapped = nullptr;
		vk_call(vkMapMemory, _device, stagingMemory, 0, vertex->size, 0, &mapped);
		memcpy(mapped, data.val(), data.size());
		vkUnmapMemory(_device, stagingMemory);

		ScopeLock lock(_mutex);

		vk_call(vk_beginCommandBuffer, _device, _commandPool, &cmd);

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

		vk_call(vkEndCommandBuffer, cmd);
		vk_call(vk_submitCommand, &cmd,
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

	void VulkanRenderResource::unloadVertexData(VertexData::ID *id) {
		auto vertex = static_cast<VkVertexBuffer*>(id->ptr);
		if (!vertex)
			return;
		vertex->unref();
		id->ptr = nullptr;
	}

} // namespace qk
