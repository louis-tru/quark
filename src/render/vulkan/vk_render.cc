/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

#include "./vk_render.h"

#if Qk_ANDROID
# include <android/native_window.h>
#endif

#include <algorithm>
#include <limits>

namespace qk {

	#define Qk_VK_CHECK(expr) Qk_CHECK((expr) == VK_SUCCESS, "Vulkan call failed: %s", #expr)

	VulkanRender::VulkanRender(Options opts)
		: RenderBackend(opts)
		, _instance(VK_NULL_HANDLE)
		, _physicalDevice(VK_NULL_HANDLE)
		, _device(VK_NULL_HANDLE)
		, _queue(VK_NULL_HANDLE)
		, _queueFamily(std::numeric_limits<uint32_t>::max())
		, _surface(VK_NULL_HANDLE)
		, _swapchain(VK_NULL_HANDLE)
		, _swapchainFormat(VK_FORMAT_UNDEFINED)
		, _swapchainExtent{}
		, _renderPass(VK_NULL_HANDLE)
		, _commandPool(VK_NULL_HANDLE)
		, _imageAvailable{}
		, _renderFinished{}
		, _inFlight{}
		, _frameIndex(0)
		, _window(nullptr)
		, _vkCanvas(nullptr)
		, _threadId(thread_self_id())
		, _loopThreadId()
	{
		_opts.enableCAPA = false;
		_opts.colorType = kRGBA_8888_ColorType;

		const char *extensions[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
		};
		VkApplicationInfo appInfo = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Quark",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Quark",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_0,
		};
		VkInstanceCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = uint32_t(sizeof(extensions) / sizeof(extensions[0])),
			.ppEnabledExtensionNames = extensions,
		};
		Qk_VK_CHECK(vkCreateInstance(&info, nullptr, &_instance));

		_vkCanvas = NewRetain<VulkanCanvas>(this, _opts);
		_canvas = _vkCanvas;
	}

	VulkanRender::~VulkanRender() {
		Qk_CHECK(_vkCanvas == nullptr);
	}

	void VulkanRender::release() {
		renderLoopStop();
		lock();
		deleteSurface();
		if (_instance) {
			vkDestroyInstance(_instance, nullptr);
			_instance = VK_NULL_HANDLE;
		}
		Releasep(_vkCanvas);
		_canvas = nullptr;
		unlock();
	}

	void VulkanRender::lock() { _mutex.lock(); }
	void VulkanRender::unlock() { _mutex.unlock(); }

	void VulkanRender::post_message(Cb cb) {
		std::lock_guard<RecursiveMutex> lock(_mutex);
		cb->resolve();
	}

	Vec2 VulkanRender::getSurfaceSize() {
#if Qk_ANDROID
		return _window ? Vec2(ANativeWindow_getWidth((ANativeWindow*)_window),
			ANativeWindow_getHeight((ANativeWindow*)_window)) : Vec2();
#else
		return {};
#endif
	}

	void VulkanRender::reload() {
		std::lock_guard<RecursiveMutex> lock(_mutex);
		if (_surface && !createSwapchain())
			return;
		_surfaceSize = getSurfaceSize();
		_delegate->onRenderBackendReload(_surfaceSize);
	}

	Canvas* VulkanRender::createCanvas(Options opts) {
		opts.enableCAPA = false;
		return new VulkanCanvas(this, opts);
	}

	TexStat VulkanRender::createTextureStat(Vec2, ColorType, uint8_t) {
		return TexStat();
	}

	bool VulkanRender::uploadTexture(cPixel*, int, TexStat*, bool) {
		return false;
	}

	void VulkanRender::unloadTexture(TexStat *tex) {
		tex->set_ptr(nullptr);
	}

	bool VulkanRender::uploadVertexData(VertexData::ID*) {
		return false;
	}

	void VulkanRender::unloadVertexData(VertexData::ID *id) {
		id->a = 0;
		id->b = 0;
		id->ptr = nullptr;
	}

	bool VulkanRender::createDevice() {
		uint32_t physicalCount = 0;
		Qk_VK_CHECK(vkEnumeratePhysicalDevices(_instance, &physicalCount, nullptr));
		if (!physicalCount)
			return false;
		std::vector<VkPhysicalDevice> physicalDevices(physicalCount);
		Qk_VK_CHECK(vkEnumeratePhysicalDevices(_instance, &physicalCount, physicalDevices.data()));

		for (auto physical : physicalDevices) {
			uint32_t queueCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueCount, nullptr);
			std::vector<VkQueueFamilyProperties> queues(queueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physical, &queueCount, queues.data());
			for (uint32_t i = 0; i < queueCount; i++) {
				VkBool32 present = false;
				Qk_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, _surface, &present));
				if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
					_physicalDevice = physical;
					_queueFamily = i;
					break;
				}
			}
			if (_physicalDevice)
				break;
		}
		if (!_physicalDevice)
			return false;

		float priority = 1.0f;
		VkDeviceQueueCreateInfo queueInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = _queueFamily,
			.queueCount = 1,
			.pQueuePriorities = &priority,
		};
		const char *extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo deviceInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueInfo,
			.enabledExtensionCount = 1,
			.ppEnabledExtensionNames = extensions,
		};
		Qk_VK_CHECK(vkCreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device));
		vkGetDeviceQueue(_device, _queueFamily, 0, &_queue);

		VkCommandPoolCreateInfo poolInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = _queueFamily,
		};
		Qk_VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool));

		VkSemaphoreCreateInfo semaphoreInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};
		for (int i = 0; i < 2; i++) {
			Qk_VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailable[i]));
			Qk_VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinished[i]));
			Qk_VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_inFlight[i]));
		}
		return true;
	}

	VkSurfaceFormatKHR VulkanRender::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) const {
		for (auto format : formats) {
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
			if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}
		return formats[0];
	}

	VkPresentModeKHR VulkanRender::choosePresentMode(const std::vector<VkPresentModeKHR> &modes) const {
		for (auto mode : modes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				return mode;
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanRender::chooseExtent(const VkSurfaceCapabilitiesKHR &caps) const {
		if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return caps.currentExtent;
		auto size = getSurfaceSize();
		return {
			std::max(caps.minImageExtent.width, std::min(caps.maxImageExtent.width, uint32_t(size.x()))),
			std::max(caps.minImageExtent.height, std::min(caps.maxImageExtent.height, uint32_t(size.y()))),
		};
	}

	bool VulkanRender::createSwapchain() {
		if (!_device || !_surface)
			return false;
		vkDeviceWaitIdle(_device);
		destroySwapchain();

		VkSurfaceCapabilitiesKHR caps;
		Qk_VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &caps));
		uint32_t formatCount = 0, modeCount = 0;
		Qk_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr));
		Qk_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &modeCount, nullptr));
		if (!formatCount || !modeCount)
			return false;
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		std::vector<VkPresentModeKHR> modes(modeCount);
		Qk_VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, formats.data()));
		Qk_VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &modeCount, modes.data()));
		auto format = chooseSurfaceFormat(formats);
		auto extent = chooseExtent(caps);
		uint32_t imageCount = std::max(2u, caps.minImageCount + 1);
		if (caps.maxImageCount)
			imageCount = std::min(imageCount, caps.maxImageCount);
		VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		const VkCompositeAlphaFlagBitsKHR alphaModes[] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto mode : alphaModes) {
			if (caps.supportedCompositeAlpha & mode) {
				alpha = mode;
				break;
			}
		}
		VkSwapchainCreateInfoKHR info = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = _surface,
			.minImageCount = imageCount,
			.imageFormat = format.format,
			.imageColorSpace = format.colorSpace,
			.imageExtent = extent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = caps.currentTransform,
			.compositeAlpha = alpha,
			.presentMode = choosePresentMode(modes),
			.clipped = VK_TRUE,
		};
		Qk_VK_CHECK(vkCreateSwapchainKHR(_device, &info, nullptr, &_swapchain));
		_swapchainFormat = format.format;
		_swapchainExtent = extent;

		Qk_VK_CHECK(vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr));
		_swapchainImages.resize(imageCount);
		Qk_VK_CHECK(vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data()));
		_swapchainViews.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo viewInfo = {
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = _swapchainImages[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = _swapchainFormat,
				.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
					VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
				.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
			};
			Qk_VK_CHECK(vkCreateImageView(_device, &viewInfo, nullptr, &_swapchainViews[i]));
		}

		VkAttachmentDescription attachment = {
			.format = _swapchainFormat,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		};
		VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkSubpassDescription subpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorRef,
		};
		VkSubpassDependency dependency = {
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		};
		VkRenderPassCreateInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &attachment,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 1,
			.pDependencies = &dependency,
		};
		Qk_VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass));

		_framebuffers.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++) {
			VkFramebufferCreateInfo framebufferInfo = {
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = _renderPass,
				.attachmentCount = 1,
				.pAttachments = &_swapchainViews[i],
				.width = extent.width,
				.height = extent.height,
				.layers = 1,
			};
			Qk_VK_CHECK(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_framebuffers[i]));
		}

		_commandBuffers.resize(imageCount);
		VkCommandBufferAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = _commandPool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = imageCount,
		};
		Qk_VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()));
		return true;
	}

	void VulkanRender::destroySwapchain() {
		if (!_device)
			return;
		if (!_commandBuffers.empty()) {
			vkFreeCommandBuffers(_device, _commandPool, uint32_t(_commandBuffers.size()), _commandBuffers.data());
			_commandBuffers.clear();
		}
		for (auto framebuffer : _framebuffers)
			vkDestroyFramebuffer(_device, framebuffer, nullptr);
		_framebuffers.clear();
		if (_renderPass)
			vkDestroyRenderPass(_device, _renderPass, nullptr);
		_renderPass = VK_NULL_HANDLE;
		for (auto view : _swapchainViews)
			vkDestroyImageView(_device, view, nullptr);
		_swapchainViews.clear();
		_swapchainImages.clear();
		if (_swapchain)
			vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		_swapchain = VK_NULL_HANDLE;
	}

	void VulkanRender::destroyDevice() {
		if (!_device)
			return;
		vkDeviceWaitIdle(_device);
		destroySwapchain();
		for (int i = 0; i < 2; i++) {
			if (_imageAvailable[i]) vkDestroySemaphore(_device, _imageAvailable[i], nullptr);
			if (_renderFinished[i]) vkDestroySemaphore(_device, _renderFinished[i], nullptr);
			if (_inFlight[i]) vkDestroyFence(_device, _inFlight[i], nullptr);
			_imageAvailable[i] = VK_NULL_HANDLE;
			_renderFinished[i] = VK_NULL_HANDLE;
			_inFlight[i] = VK_NULL_HANDLE;
		}
		if (_commandPool)
			vkDestroyCommandPool(_device, _commandPool, nullptr);
		_commandPool = VK_NULL_HANDLE;
		vkDestroyDevice(_device, nullptr);
		_device = VK_NULL_HANDLE;
		_physicalDevice = VK_NULL_HANDLE;
		_queue = VK_NULL_HANDLE;
		_queueFamily = std::numeric_limits<uint32_t>::max();
	}

	void VulkanRender::makeSurface(EGLNativeWindowType win) {
		std::lock_guard<RecursiveMutex> lock(_mutex);
		if (_surface || !win)
			return;
#if Qk_ANDROID
		_window = win;
		ANativeWindow_acquire((ANativeWindow*)_window);
		VkAndroidSurfaceCreateInfoKHR info = {
			.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR,
			.window = (ANativeWindow*)_window,
		};
		Qk_VK_CHECK(vkCreateAndroidSurfaceKHR(_instance, &info, nullptr, &_surface));
		Qk_CHECK(createDevice(), "Unable to create Vulkan device");
#endif
	}

	void VulkanRender::deleteSurface() {
		std::lock_guard<RecursiveMutex> lock(_mutex);
		if (_device)
			destroyDevice();
		if (_surface) {
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
			_surface = VK_NULL_HANDLE;
		}
#if Qk_ANDROID
		if (_window)
			ANativeWindow_release((ANativeWindow*)_window);
#endif
		_window = nullptr;
	}

	void VulkanRender::drawFrame(const Color4f &color) {
		if (!_swapchain)
			return;
		auto frame = _frameIndex & 1;
		Qk_VK_CHECK(vkWaitForFences(_device, 1, &_inFlight[frame], VK_TRUE, UINT64_MAX));
		uint32_t imageIndex = 0;
		auto result = vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX,
			_imageAvailable[frame], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			createSwapchain();
			return;
		}
		Qk_CHECK(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
			"Unable to acquire Vulkan swapchain image: %d", result);
		Qk_VK_CHECK(vkResetFences(_device, 1, &_inFlight[frame]));
		auto cmd = _commandBuffers[imageIndex];
		Qk_VK_CHECK(vkResetCommandBuffer(cmd, 0));
		VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		Qk_VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));
		VkClearValue clear{};
		clear.color.float32[0] = color.r();
		clear.color.float32[1] = color.g();
		clear.color.float32[2] = color.b();
		clear.color.float32[3] = color.a();
		VkRenderPassBeginInfo passInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = _renderPass,
			.framebuffer = _framebuffers[imageIndex],
			.renderArea = { {0, 0}, _swapchainExtent },
			.clearValueCount = 1,
			.pClearValues = &clear,
		};
		vkCmdBeginRenderPass(cmd, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(cmd);
		Qk_VK_CHECK(vkEndCommandBuffer(cmd));

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &_imageAvailable[frame],
			.pWaitDstStageMask = &waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &cmd,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &_renderFinished[frame],
		};
		Qk_VK_CHECK(vkQueueSubmit(_queue, 1, &submit, _inFlight[frame]));
		VkPresentInfoKHR present = {
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &_renderFinished[frame],
			.swapchainCount = 1,
			.pSwapchains = &_swapchain,
			.pImageIndices = &imageIndex,
		};
		result = vkQueuePresentKHR(_queue, &present);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			createSwapchain();
		else
			Qk_CHECK(result == VK_SUCCESS, "Unable to present Vulkan swapchain: %d", result);
		_frameIndex++;
	}

	void VulkanRender::renderDisplay() {
		std::lock_guard<RecursiveMutex> lock(_mutex);
		if (!_swapchain)
			return;
		if (_delegate->onRenderBackendDisplay())
			drawFrame(_vkCanvas->consumeClearColor());
	}

	void VulkanRender::renderLoopRun() {
		if (_loopThreadId != ThreadID())
			return;
		_loopThreadId = thread_new([this](cThread *thread) {
			const int64_t intervalUs = 1000000 / 60;
			while (!thread->abort) {
				auto remain = time_monotonic() + intervalUs;
				renderDisplay();
				remain -= time_monotonic();
				if (remain > 0)
					thread_sleep(remain);
			}
		}, "vulkan_render_thread");
	}

	void VulkanRender::renderLoopStop() {
		if (_loopThreadId != ThreadID()) {
			thread_try_abort(_loopThreadId);
			thread_join_for(_loopThreadId);
			_loopThreadId = ThreadID();
		}
	}

	void* acquireRenderBackendStorage(size_t typeHash, size_t size);

	Render* make_vulkan_render(Render::Options opts) {
		auto mem = acquireRenderBackendStorage(typeid(VulkanRender).hash_code(), sizeof(VulkanRender));
		return new (mem) VulkanRender(opts);
	}

}
