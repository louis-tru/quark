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

	bool VulkanRenderResource::createEmptyTexture() {
		auto tex = newTexture(Vec2(1), kRGBA_8888_ColorType, 1, kNone_TextureFlags);
		if (!tex)
			return false;

		VkResult result;
		VkCommandBuffer cmd = VK_NULL_HANDLE;
		auto fail = [&]() {
			if (cmd)
				vkFreeCommandBuffers(_device, _commandPool, 1, &cmd);
			vk_deleteTexture(_device, tex);
			return false;
		};
		vk_call(vk_beginCommandBuffer, return fail(), _device, _commandPool, &cmd);

		VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = tex->image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkClearColorValue color{};
		vkCmdClearColorImage(cmd, tex->image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &barrier.subresourceRange);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		vk_call(vkEndCommandBuffer, return fail(), cmd);
		vk_call(vk_submitCommand, return fail(), &cmd, 0);

		_emptyTexture = tex;
		return true;
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

	VkTexture* VulkanRenderResource::newTexture(Vec2 size, ColorType type, uint32_t mipLevels, uint8_t flags) {
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

		VkImageCreateInfo imageInfo{};
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

		auto tex = new VkTexture{
			.format = format,
			.extent = { uint32_t(size.x()), uint32_t(size.y()) },
			.mipLevels = mipLevels,
			.usage = usage,
		};
		auto fail = [&]() {
			vk_deleteTexture(_device, tex);
			return nullptr;
		};
		VkResult result;
		vk_call(vkCreateImage, return fail(), _device, &imageInfo, nullptr, &tex->image);

		uint32_t memoryType;
		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(_device, tex->image, &requirements);
		vk_call(vk_findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo = {};
		memoryInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, return fail(), _device, &memoryInfo, nullptr, &tex->memory);
		vk_call(vkBindImageMemory, return fail(), _device, tex->image, tex->memory, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = tex->image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.layerCount = 1;
		vk_call(vkCreateImageView, return fail(), _device, &viewInfo, nullptr, &tex->view);

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
		vk_call(vk_findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
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

	void VulkanRenderResource::unloadTexture(TexStat *tex) {
		auto *vk_tex = static_cast<VkTexture*>(tex->ptr());
		if (!vk_tex)
			return;
		// We cannot delete the texture immediately, because it may be in use by the GPU. Instead,
		// we schedule a delayed task to delete the texture after the GPU has finished using it.
		vk_deleteTextureSafe(_device, vk_tex);
		tex->set_ptr(nullptr);
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
		vk_call(vk_findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryType);

		VkMemoryAllocateInfo memoryInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
		memoryInfo.allocationSize = requirements.size;
		memoryInfo.memoryTypeIndex = memoryType;
		vk_call(vkAllocateMemory, return fail(), _device, &memoryInfo, nullptr, &vertex->memory);
		vk_call(vkBindBufferMemory, return fail(), _device, vertex->buffer, vertex->memory, 0);

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
		vk_call(vk_findMemoryType, return fail(), _physicalDevice, requirements.memoryTypeBits,
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

	void VulkanRenderResource::unloadVertexData(VertexData::ID *id) {
		auto vertex = static_cast<VkVertexBuffer*>(id->ptr);
		if (!vertex)
			return;
		vk_deleteVertexBufferSafe(_device, vertex);
		id->ptr = nullptr;
	}

} // namespace qk
