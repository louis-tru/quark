#include <android/log.h>
#include <android_native_app_glue.h>

#define VK_USE_PLATFORM_ANDROID_KHR 1
#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>
#include <vector>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "VkMinimal", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "VkMinimal", __VA_ARGS__)

namespace {

class VulkanApp {
public:
	bool start(ANativeWindow *window) {
		_window = window;
		return createInstance() &&
			createSurface() &&
			selectDevice() &&
			createDevice() &&
			createCommandPool() &&
			createSync() &&
			createSwapchain();
	}

	void stop() {
		if (_device)
			vkDeviceWaitIdle(_device);
		destroySwapchain();
		if (_device) {
			if (_imageAvailable)
				vkDestroySemaphore(_device, _imageAvailable, nullptr);
			if (_renderFinished)
				vkDestroySemaphore(_device, _renderFinished, nullptr);
			if (_commandPool)
				vkDestroyCommandPool(_device, _commandPool, nullptr);
			vkDestroyDevice(_device, nullptr);
		}
		if (_surface)
			vkDestroySurfaceKHR(_instance, _surface, nullptr);
		if (_instance)
			vkDestroyInstance(_instance, nullptr);
		*this = VulkanApp();
	}

	bool draw() {
		if (!_swapchain && !createSwapchain())
			return false;

		uint32_t imageIndex = 0;
		VkResult result = vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX,
			_imageAvailable, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			return recreateSwapchain();
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			return fail("vkAcquireNextImageKHR", result);

		if (!recordCommands(imageIndex))
			return false;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &_imageAvailable;
		submit.pWaitDstStageMask = &waitStage;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &_commands[imageIndex];
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &_renderFinished;
		if ((result = vkQueueSubmit(_queue, 1, &submit, VK_NULL_HANDLE)) != VK_SUCCESS)
			return fail("vkQueueSubmit", result);

		VkPresentInfoKHR present = {};
		present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = &_renderFinished;
		present.swapchainCount = 1;
		present.pSwapchains = &_swapchain;
		present.pImageIndices = &imageIndex;
		result = vkQueuePresentKHR(_queue, &present);
		vkQueueWaitIdle(_queue); // Deliberately simple: formal backend should use fences.

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			return recreateSwapchain();
		return result == VK_SUCCESS || fail("vkQueuePresentKHR", result);
	}

private:
	bool fail(const char *call, VkResult result) const {
		LOGE("%s failed: %d", call, int(result));
		return false;
	}

	bool createInstance() {
		const char *extensions[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
		};
		VkApplicationInfo app = {};
		app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app.pApplicationName = "Quark Vulkan Minimal";
		app.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pApplicationInfo = &app;
		info.enabledExtensionCount = 2;
		info.ppEnabledExtensionNames = extensions;
		VkResult result = vkCreateInstance(&info, nullptr, &_instance);
		return result == VK_SUCCESS || fail("vkCreateInstance", result);
	}

	bool createSurface() {
		VkAndroidSurfaceCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		info.window = _window;
		VkResult result = vkCreateAndroidSurfaceKHR(_instance, &info, nullptr, &_surface);
		return result == VK_SUCCESS || fail("vkCreateAndroidSurfaceKHR", result);
	}

	bool selectDevice() {
		uint32_t count = 0;
		VkResult result = vkEnumeratePhysicalDevices(_instance, &count, nullptr);
		if (result != VK_SUCCESS || !count)
			return fail("vkEnumeratePhysicalDevices", result);
		std::vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(_instance, &count, devices.data());

		for (auto device: devices) {
			uint32_t queueCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
			std::vector<VkQueueFamilyProperties> queues(queueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());
			for (uint32_t i = 0; i < queueCount; i++) {
				VkBool32 present = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &present);
				if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
					_physicalDevice = device;
					_queueFamily = i;
					return true;
				}
			}
		}
		LOGE("No Vulkan graphics+present queue found");
		return false;
	}

	bool createDevice() {
		float priority = 1.0f;
		VkDeviceQueueCreateInfo queue = {};
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.queueFamilyIndex = _queueFamily;
		queue.queueCount = 1;
		queue.pQueuePriorities = &priority;
		const char *extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkDeviceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		info.queueCreateInfoCount = 1;
		info.pQueueCreateInfos = &queue;
		info.enabledExtensionCount = 1;
		info.ppEnabledExtensionNames = extensions;
		VkResult result = vkCreateDevice(_physicalDevice, &info, nullptr, &_device);
		if (result != VK_SUCCESS)
			return fail("vkCreateDevice", result);
		vkGetDeviceQueue(_device, _queueFamily, 0, &_queue);
		return true;
	}

	bool createCommandPool() {
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		info.queueFamilyIndex = _queueFamily;
		VkResult result = vkCreateCommandPool(_device, &info, nullptr, &_commandPool);
		return result == VK_SUCCESS || fail("vkCreateCommandPool", result);
	}

	bool createSync() {
		VkSemaphoreCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkResult result = vkCreateSemaphore(_device, &info, nullptr, &_imageAvailable);
		if (result == VK_SUCCESS)
			result = vkCreateSemaphore(_device, &info, nullptr, &_renderFinished);
		return result == VK_SUCCESS || fail("vkCreateSemaphore", result);
	}

	VkSurfaceFormatKHR chooseFormat(const std::vector<VkSurfaceFormatKHR> &formats) const {
		if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
			return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		for (auto format: formats) {
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return format;
		}
		return formats.front();
	}

	bool createSwapchain() {
		VkSurfaceCapabilitiesKHR caps = {};
		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &caps);
		if (result != VK_SUCCESS)
			return fail("vkGetPhysicalDeviceSurfaceCapabilitiesKHR", result);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr);
		if (!formatCount) {
			LOGE("Surface has no Vulkan formats");
			return false;
		}
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, formats.data());
		auto format = chooseFormat(formats);

		_extent = caps.currentExtent;
		if (_extent.width == UINT32_MAX) {
			_extent.width = std::min(std::max(uint32_t(ANativeWindow_getWidth(_window)),
				caps.minImageExtent.width), caps.maxImageExtent.width);
			_extent.height = std::min(std::max(uint32_t(ANativeWindow_getHeight(_window)),
				caps.minImageExtent.height), caps.maxImageExtent.height);
		}

		uint32_t imageCount = caps.minImageCount + 1;
		if (caps.maxImageCount && imageCount > caps.maxImageCount)
			imageCount = caps.maxImageCount;

		VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		const VkCompositeAlphaFlagBitsKHR alphaModes[] = {
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};
		for (auto mode: alphaModes) {
			if (caps.supportedCompositeAlpha & mode) {
				alpha = mode;
				break;
			}
		}

		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = _surface;
		info.minImageCount = imageCount;
		info.imageFormat = format.format;
		info.imageColorSpace = format.colorSpace;
		info.imageExtent = _extent;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.preTransform = caps.currentTransform;
		info.compositeAlpha = alpha;
		info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		info.clipped = VK_TRUE;
		result = vkCreateSwapchainKHR(_device, &info, nullptr, &_swapchain);
		if (result != VK_SUCCESS)
			return fail("vkCreateSwapchainKHR", result);
		_format = format.format;
		return createSwapchainResources();
	}

	bool createSwapchainResources() {
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
		_images.resize(imageCount);
		vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _images.data());
		_views.resize(imageCount);

		for (uint32_t i = 0; i < imageCount; i++) {
			VkImageViewCreateInfo view = {};
			view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.image = _images[i];
			view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view.format = _format;
			view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view.subresourceRange.levelCount = 1;
			view.subresourceRange.layerCount = 1;
			VkResult result = vkCreateImageView(_device, &view, nullptr, &_views[i]);
			if (result != VK_SUCCESS)
				return fail("vkCreateImageView", result);
		}

		VkAttachmentDescription color = {};
		color.format = _format;
		color.samples = VK_SAMPLE_COUNT_1_BIT;
		color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		VkAttachmentReference colorRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		VkRenderPassCreateInfo pass = {};
		pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		pass.attachmentCount = 1;
		pass.pAttachments = &color;
		pass.subpassCount = 1;
		pass.pSubpasses = &subpass;
		VkResult result = vkCreateRenderPass(_device, &pass, nullptr, &_renderPass);
		if (result != VK_SUCCESS)
			return fail("vkCreateRenderPass", result);

		_framebuffers.resize(imageCount);
		for (uint32_t i = 0; i < imageCount; i++) {
			VkFramebufferCreateInfo framebuffer = {};
			framebuffer.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer.renderPass = _renderPass;
			framebuffer.attachmentCount = 1;
			framebuffer.pAttachments = &_views[i];
			framebuffer.width = _extent.width;
			framebuffer.height = _extent.height;
			framebuffer.layers = 1;
			result = vkCreateFramebuffer(_device, &framebuffer, nullptr, &_framebuffers[i]);
			if (result != VK_SUCCESS)
				return fail("vkCreateFramebuffer", result);
		}

		_commands.resize(imageCount);
		VkCommandBufferAllocateInfo commands = {};
		commands.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commands.commandPool = _commandPool;
		commands.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commands.commandBufferCount = imageCount;
		result = vkAllocateCommandBuffers(_device, &commands, _commands.data());
		return result == VK_SUCCESS || fail("vkAllocateCommandBuffers", result);
	}

	bool recordCommands(uint32_t imageIndex) {
		VkCommandBuffer command = _commands[imageIndex];
		vkResetCommandBuffer(command, 0);
		VkCommandBufferBeginInfo begin = {};
		begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VkResult result = vkBeginCommandBuffer(command, &begin);
		if (result != VK_SUCCESS)
			return fail("vkBeginCommandBuffer", result);

		_frame = (_frame + 1) % 240;
		float phase = float(_frame) / 240.0f;
		VkClearValue clear = {};
		clear.color.float32[0] = 0.05f + phase * 0.90f;
		clear.color.float32[1] = 0.08f;
		clear.color.float32[2] = 0.95f - phase * 0.90f;
		clear.color.float32[3] = 1.0f;

		VkRenderPassBeginInfo pass = {};
		pass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		pass.renderPass = _renderPass;
		pass.framebuffer = _framebuffers[imageIndex];
		pass.renderArea.extent = _extent;
		pass.clearValueCount = 1;
		pass.pClearValues = &clear;
		vkCmdBeginRenderPass(command, &pass, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(command);
		result = vkEndCommandBuffer(command);
		return result == VK_SUCCESS || fail("vkEndCommandBuffer", result);
	}

	bool recreateSwapchain() {
		vkDeviceWaitIdle(_device);
		destroySwapchain();
		return createSwapchain();
	}

	void destroySwapchain() {
		if (!_device)
			return;
		if (!_commands.empty())
			vkFreeCommandBuffers(_device, _commandPool, uint32_t(_commands.size()), _commands.data());
		for (auto framebuffer: _framebuffers)
			vkDestroyFramebuffer(_device, framebuffer, nullptr);
		if (_renderPass)
			vkDestroyRenderPass(_device, _renderPass, nullptr);
		for (auto view: _views)
			vkDestroyImageView(_device, view, nullptr);
		if (_swapchain)
			vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		_commands.clear();
		_framebuffers.clear();
		_views.clear();
		_images.clear();
		_renderPass = VK_NULL_HANDLE;
		_swapchain = VK_NULL_HANDLE;
	}

	ANativeWindow *_window = nullptr;
	VkInstance _instance = VK_NULL_HANDLE;
	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	uint32_t _queueFamily = 0;
	VkQueue _queue = VK_NULL_HANDLE;
	VkCommandPool _commandPool = VK_NULL_HANDLE;
	VkSemaphore _imageAvailable = VK_NULL_HANDLE;
	VkSemaphore _renderFinished = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	VkFormat _format = VK_FORMAT_UNDEFINED;
	VkExtent2D _extent = {};
	VkRenderPass _renderPass = VK_NULL_HANDLE;
	std::vector<VkImage> _images;
	std::vector<VkImageView> _views;
	std::vector<VkFramebuffer> _framebuffers;
	std::vector<VkCommandBuffer> _commands;
	uint32_t _frame = 0;
};

struct AppState {
	VulkanApp vulkan;
	bool ready = false;
};

void handleCommand(android_app *app, int32_t command) {
	auto *state = static_cast<AppState*>(app->userData);
	if (command == APP_CMD_INIT_WINDOW && app->window && !state->ready) {
		state->ready = state->vulkan.start(app->window);
		LOGI("Vulkan start: %s", state->ready ? "ok" : "failed");
		if (!state->ready)
			state->vulkan.stop();
	} else if (command == APP_CMD_TERM_WINDOW && state->ready) {
		state->vulkan.stop();
		state->ready = false;
	}
}

} // namespace

void android_main(android_app *app) {
	app_dummy();
	AppState state;
	app->userData = &state;
	app->onAppCmd = handleCommand;

	while (!app->destroyRequested) {
		int events = 0;
		android_poll_source *source = nullptr;
		int timeout = state.ready ? 0 : -1;
		while (ALooper_pollOnce(timeout, nullptr, &events,
			reinterpret_cast<void**>(&source)) >= 0) {
			if (source)
				source->process(app, source);
			if (app->destroyRequested)
				break;
			timeout = 0;
		}
		if (state.ready && !state.vulkan.draw()) {
			state.vulkan.stop();
			state.ready = false;
		}
	}
	state.vulkan.stop();
}
