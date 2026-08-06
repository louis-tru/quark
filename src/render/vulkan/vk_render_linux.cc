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
#include "../plotforms.h"
#include "../../util/thread.h"

#if Qk_ANDROID
# include <android/native_window.h>
#elif Qk_LINUX
# include "../platforms.h"
# undef Status
# undef Bool
# undef None
#endif

#define EGL_NO_NATIVE_WINDOW 0

namespace qk {
	typedef Render::Options Options;

	struct VulkanSwapchainImage {
		void setTexture(VkTexture* tex) {
			if (texture) {
				texture->image = VK_NULL_HANDLE;
				texture->unref();
			}
			if (tex)
				tex->ref();
			texture = tex;
		}
		VkTexture* texture = nullptr;
		VkCommandBuffer presentCommand = VK_NULL_HANDLE;
		VkSemaphore renderFinished = VK_NULL_HANDLE;
	};

	class LinuxVulkanRender final: public VulkanRender, public RenderSurface {
		public:
			explicit LinuxVulkanRender(Options opts)
				: VulkanRender(opts)
				, _surface(VK_NULL_HANDLE)
				, _swapchain(VK_NULL_HANDLE)
				, _presentCommandPool(VK_NULL_HANDLE)
				, _imageAvailableIndex(0)
				, _window(EGL_NO_NATIVE_WINDOW)
				, _imageIndex(U32::limit_max)
				, _isRun(true)
		{
			VkCommandPoolCreateInfo info{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
			info.queueFamilyIndex = _resource->queueFamily();
			Qk_CHECK(vkCreateCommandPool(
				_device, &info, nullptr, &_presentCommandPool) == VK_SUCCESS,
				"Failed to create Vulkan present command pool");
		}

		void release() override {
			stopDisplay();
			destroySwapchain(true);
			vkDestroyCommandPool(_device, _presentCommandPool, nullptr);
			_presentCommandPool = VK_NULL_HANDLE;

			if (_surface)
				vkDestroySurfaceKHR(_resource->instance(), _surface, nullptr);
			_surface = VK_NULL_HANDLE;

			VulkanRender::release();
			Object::release();
		}

		RenderSurface* surface() override {
			return this;
		}

		void reload() override {
			ScopeLock lock(_mutex);
			Qk_ASSERT(_surface, "Vulkan surface must be created before reload");
			_surfaceSize = getSurfaceSize();
			destroySwapchain(true);
			createSwapchain();
			_delegate->onRenderBackendReload(_surfaceSize);
		}

		Vec2 getSurfaceSize() override {
			if (!_window) return {};
#if Qk_ANDROID
			auto window = reinterpret_cast<ANativeWindow*>(_window);
			return Vec2(ANativeWindow_getWidth(window), ANativeWindow_getHeight(window));
#else
			XWindowAttributes attrs{};
			auto dpy = openXDisplay();
			auto xwin = static_cast<XWindow>(_window);
			Qk_ASSERT_EQ(1, XGetWindowAttributes(dpy, xwin, &attrs), "Failed to get X window attributes");
			return Vec2(attrs.width, attrs.height);
#endif
		}

		void makeSurface(EGLNativeWindowType window) override {
			if (_surface)
				return;
			VkResult result;
			VkSurfaceKHR surface = VK_NULL_HANDLE;
#if Qk_ANDROID
			VkAndroidSurfaceCreateInfoKHR info{
				VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR
			};
			info.window = reinterpret_cast<ANativeWindow*>(window);
			result = vkCreateAndroidSurfaceKHR(
				_resource->instance(), &info, nullptr, &surface);
#else
			VkXlibSurfaceCreateInfoKHR info{
				VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR
			};
			info.dpy = openXDisplay();
			info.window = static_cast<XWindow>(window);
			result = vkCreateXlibSurfaceKHR(
				_resource->instance(), &info, nullptr, &surface);
#endif
			Qk_CHECK(result == VK_SUCCESS, "Failed to create Vulkan platform surface");

			VkBool32 supported = VK_FALSE;
			result = vkGetPhysicalDeviceSurfaceSupportKHR(
				_resource->physicalDevice(), _resource->queueFamily(), surface, &supported);
			Qk_CHECK(result == VK_SUCCESS, "Failed to query Vulkan surface support");
			Qk_CHECK(supported, "Selected Vulkan queue cannot present to this surface");

			_surface = surface;
			_window = window;
		}

		void deleteSurface() override {
			if (!_surface)
				return;
			destroySwapchain(true);
			vkDestroySurfaceKHR(_resource->instance(), _surface, nullptr);
			_surface = VK_NULL_HANDLE;
			_window = EGL_NO_NATIVE_WINDOW;
		}

		bool acquireNextImage() {
			if (_swapchainImages.isNull())
				return false;
			if (_imageIndex == U32::limit_max) {
				uint32_t imageIndex = 0;
				auto acquire = vkAcquireNextImageKHR(
					_device, _swapchain, 0,
					_imageAvailable[_imageAvailableIndex], VK_NULL_HANDLE, &imageIndex);
				if (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR) {
					if (acquire != VK_NOT_READY &&
						acquire != VK_TIMEOUT &&
						acquire != VK_ERROR_OUT_OF_DATE_KHR)
						Qk_DLog("vkAcquireNextImageKHR failed: %d", int(acquire));
					return false;
				}
				_imageIndex = imageIndex;
				Qk_ASSERT_LT(_imageIndex, _swapchainImages.length(),
					"Invalid Vulkan swapchain image index");
				_vkCanvas->setDefaultTarget(_swapchainImages[_imageIndex].texture);
			}
			return true;
		}

		void renderDisplay() override {
			ScopeLock lock(_mutex);
			if (!_isRun)
				return;
			Qk_ASSERT(_surface, "Vulkan surface must be created before renderDisplay");

			if (!acquireNextImage())
				return;
			if (!_delegate->onRenderBackendDisplay())
				return;
			auto commands = _vkCanvas->flushBuffer();
			if (commands.isNull())
				return;

			auto &image = _swapchainImages[_imageIndex];
			commands.push(image.presentCommand);
			image.texture->levels[0].layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkPipelineStageFlags waitStage =
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
			VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO};
			submit.waitSemaphoreCount = 1;
			submit.pWaitSemaphores = _imageAvailable + _imageAvailableIndex;
			submit.pWaitDstStageMask = &waitStage;
			submit.commandBufferCount = commands.length();
			submit.pCommandBuffers = commands.val();
			submit.signalSemaphoreCount = 1;
			submit.pSignalSemaphores = &image.renderFinished;

			VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
			present.waitSemaphoreCount = 1;
			present.pWaitSemaphores = &image.renderFinished;
			present.swapchainCount = 1;
			present.pSwapchains = &_swapchain;
			present.pImageIndices = &_imageIndex;

			auto result = _resource->submitCommand(&submit, &present, _vkCanvas->cmdPackFront());
			_imageAvailableIndex = (_imageAvailableIndex + 1) & 1;
			if (result == VK_SUCCESS ||
				result == VK_SUBOPTIMAL_KHR ||
				result == VK_ERROR_OUT_OF_DATE_KHR ||
				result == VK_ERROR_SURFACE_LOST_KHR) {
				_imageIndex = U32::limit_max;
			} else {
				Qk_DLog("vkQueuePresentKHR failed: %d", int(result));
			}
		}

		void runRenderLoop() override {
			if (_threadId != ThreadID())
				return;
			_threadId = thread_new([this](cThread *thread) {
				const int64_t intervalUs = 1000000 / 60;
				while (!thread->abort) {
					auto sleepUs = time_monotonic() + intervalUs;
					renderDisplay();
					sleepUs -= time_monotonic();
					if (sleepUs > 0)
						thread_sleep(sleepUs);
				}
				_threadId = ThreadID();
			}, "vulkan_render_thread");
		}

		void stopRenderLoop() override {
			if (_threadId != ThreadID()) {
				thread_try_abort(_threadId);
				thread_join_for(_threadId);
				_threadId = ThreadID();
			}
		}

		void stopDisplay() {
			{
				ScopeLock lock(_mutex);
				_isRun = false;
			}
			stopRenderLoop();
		}

	private:

		VkSurfaceFormatKHR chooseSurfaceFormat(cArray<VkSurfaceFormatKHR> &formats) const {
			auto preferred = vk_pixelFormat(_opts.colorType);
			if (formats.length() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
				return {preferred, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

			for (auto format: formats) {
				if (format.format == preferred &&
					format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return format;
			}
			for (auto format: formats) {
				if ((format.format == VK_FORMAT_R8G8B8A8_UNORM ||
					format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
					format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return format;
			}
			return formats[0];
		}

		bool createSwapchain() {
			if (!_surface)
				return false;

			auto fail = [&]() {
				destroySwapchain();
				return false;
			};

			Qk_ASSERT_EQ(_swapchain, VK_NULL_HANDLE, "Vulkan swapchain must be null before creation");
			Qk_ASSERT_EQ(_swapchainImages.length(), 0,
				"Vulkan swapchain images must be empty before creation");

			VkResult result;
			VkSurfaceCapabilitiesKHR capabilities{};
			vk_call(vkGetPhysicalDeviceSurfaceCapabilitiesKHR,
				_resource->physicalDevice(), _surface, &capabilities);

			const VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
				| VK_IMAGE_USAGE_SAMPLED_BIT
				| VK_IMAGE_USAGE_STORAGE_BIT
			;
			if ((capabilities.supportedUsageFlags & imageUsage) != imageUsage) {
				Qk_DLog("Vulkan swapchain does not support color attachment, sampled and storage images");
				return false;
			}

			uint32_t formatCount = 0;
			vk_call(vkGetPhysicalDeviceSurfaceFormatsKHR,
				_resource->physicalDevice(), _surface, &formatCount, nullptr);
			Qk_ASSERT_GE(formatCount, 1, "Vulkan surface has no supported formats");
			Array<VkSurfaceFormatKHR> formats(formatCount);
			vk_call(vkGetPhysicalDeviceSurfaceFormatsKHR,
				_resource->physicalDevice(), _surface, &formatCount, formats.val());
			auto format = chooseSurfaceFormat(formats);

			auto extent = capabilities.currentExtent;
			if (extent.width == U32::limit_max) {
				if (_surfaceSize.x() <= 0 || _surfaceSize.y() <= 0)
					return false;
				extent.width = U32::clamp(_surfaceSize.x(),
					capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent.height = U32::clamp(_surfaceSize.y(),
					capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			}
			if (!extent.width || !extent.height)
				return false;

			uint32_t imageCount = capabilities.minImageCount;

			VkCompositeAlphaFlagBitsKHR compositeAlpha =
				VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			const VkCompositeAlphaFlagBitsKHR alphaModes[] = {
				VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
				VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
				VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
			};
			for (auto mode: alphaModes) {
				if (capabilities.supportedCompositeAlpha & mode) {
					compositeAlpha = mode;
					break;
				}
			}

			Qk_DLog("currentTransform=%u, supportedTransforms=%u",
							capabilities.currentTransform,
							capabilities.supportedTransforms);

			VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
			info.surface = _surface;
			info.minImageCount = imageCount;
			info.imageFormat = format.format;
			info.imageColorSpace = format.colorSpace;
			info.imageExtent = extent;
			info.imageArrayLayers = 1;
			info.imageUsage = imageUsage;
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			// info.preTransform = capabilities.currentTransform;
			info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			info.compositeAlpha = compositeAlpha;
			info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			info.clipped = VK_TRUE;
			vk_call(vkCreateSwapchainKHR, _device, &info, nullptr, &_swapchain);

			uint32_t swapchainImageCount = 0;
			vk_call(vkGetSwapchainImagesKHR, _device, _swapchain, &swapchainImageCount, nullptr);
			Qk_ASSERT_GT(swapchainImageCount, 0, "Vulkan swapchain has no images");

			Array<VkImage> images(swapchainImageCount);
			vk_call(vkGetSwapchainImagesKHR,
				_device, _swapchain, &swapchainImageCount, images.val());

			_swapchainImages = Array<VulkanSwapchainImage>(swapchainImageCount);
			VkSemaphoreCreateInfo semaphoreInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
			for (uint32_t i = 0; i < swapchainImageCount; i++) {
				auto texture = new VkTexture();
				_swapchainImages[i].setTexture(texture);
				texture->image = images[i];
				texture->extent = extent;
				texture->format = format.format;
				texture->usage = imageUsage;
				texture->levels = {{VK_IMAGE_LAYOUT_UNDEFINED}}; // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				texture->view = vk_createLevelView(_device, texture, 0);
				vk_call_if(bool, texture->view);
				vk_call_if(createPresentCommand, _swapchainImages[i]);
				vk_call(vkCreateSemaphore,
					_device, &semaphoreInfo, nullptr, &_swapchainImages[i].renderFinished);
			}

			for (auto &semaphore: _imageAvailable) {
				vk_call(vkCreateSemaphore, _device, &semaphoreInfo, nullptr, &semaphore);
			}
			return true;
		}

		void destroySwapchain(bool waitIdle = false) {
			if (!_swapchain)
				return;
			if (waitIdle)
				_resource->queueWaitIdle();
			for (auto &image: _swapchainImages) {
				if (image.renderFinished)
					vkDestroySemaphore(_device, image.renderFinished, nullptr);
				if (image.presentCommand)
					vkFreeCommandBuffers(
						_device, _presentCommandPool, 1, &image.presentCommand);
				image.setTexture(nullptr);
			}
			_swapchainImages.clear();
			for (auto &semaphore: _imageAvailable) {
				if (semaphore)
					vkDestroySemaphore(_device, semaphore, nullptr);
				semaphore = VK_NULL_HANDLE;
			}
			_imageAvailableIndex = 0;
			if (_swapchain)
				vkDestroySwapchainKHR(_device, _swapchain, nullptr);
			_swapchain = VK_NULL_HANDLE;
			_imageIndex = U32::limit_max;
		}

		bool createPresentCommand(VulkanSwapchainImage &image) {
			Qk_ASSERT_EQ(image.presentCommand, VK_NULL_HANDLE,
				"Vulkan present command must be null before creation");
			if (vk_beginCommandBuffer(
				_device, _presentCommandPool, &image.presentCommand, 0) != VK_SUCCESS)
				return false;
			VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = image.texture->image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.layerCount = 1;
			vkCmdPipelineBarrier(image.presentCommand,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
			return vkEndCommandBuffer(image.presentCommand) == VK_SUCCESS;
		}

		VkSurfaceKHR _surface;
		VkSwapchainKHR _swapchain;
		VkCommandPool _presentCommandPool;
		VkSemaphore _imageAvailable[2] = {VK_NULL_HANDLE, VK_NULL_HANDLE};
		uint32_t _imageAvailableIndex;
		EGLNativeWindowType _window;
		Array<VulkanSwapchainImage> _swapchainImages;
		uint32_t _imageIndex;
		ThreadID _threadId;
		Mutex _mutex;
		bool _isRun;
	};

	void* acquireRenderBackendStorage(size_t typeHash, size_t size);

	Render* make_vulkan_render(Options opts) {
		if (!getSharedRenderVulkanResource())
			return nullptr;
		auto memory = acquireRenderBackendStorage(
			typeid(LinuxVulkanRender).hash_code(), sizeof(LinuxVulkanRender));
		return new (memory) LinuxVulkanRender(opts);
	}

}
