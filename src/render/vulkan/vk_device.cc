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

	static const char* vk_platformSurfaceExtension() {
#if Qk_ANDROID
		return "VK_KHR_android_surface";
#elif Qk_LINUX
		return "VK_KHR_xlib_surface";
#elif Qk_WIN
		return "VK_KHR_win32_surface";
#else
		return nullptr;
#endif
	}

	static bool platformPresentationSupport(VkPhysicalDevice device, uint32_t family) {
#if Qk_ANDROID
		return true; // Vulkan Android 规范保证
#elif Qk_WIN
		return vkGetPhysicalDeviceWin32PresentationSupportKHR(device, family);
#else
/*
#if Qk_X11
		return vkGetPhysicalDeviceXlibPresentationSupportKHR(
				device, family, display, visualId);
#elif Qk_WAYLAND
		return vkGetPhysicalDeviceWaylandPresentationSupportKHR(
				device, family, display);
*/
		// X11/Wayland require platform display/visual parameters. Add them to
		// vk_selectBestDevice when the corresponding Vulkan backend is enabled.
		return false;
#endif
	}

	struct VulkanDeviceCandidate {
		VkPhysicalDevice device = VK_NULL_HANDLE;
		uint32_t queueFamily = U32::limit_max;
		bool computeSupport = false;
		int64_t score = -1;
		bool valid() const {
			return device != VK_NULL_HANDLE &&
				queueFamily != std::numeric_limits<uint32_t>::max() &&
				score >= 0;
		}
	};

	bool vk_supportsDeviceExtension(VkPhysicalDevice device, const char *requiredExtension) {
		uint32_t count = 0;
		VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

		if (result != VK_SUCCESS)
			return false;

		Array<VkExtensionProperties> extensions(count);

		if (count) {
			result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.val());

			if (result != VK_SUCCESS)
				return false;
		}

		for (uint32_t i = 0; i < count; i++) {
			if (strcmp(extensions[i].extensionName, requiredExtension) == 0) {
				return true;
			}
		}

		return false;
	}

	static bool findGraphicsQueueFamily(
		VkPhysicalDevice device,
		uint32_t *family,
		bool *computeSupport
	) {
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

		if (!count)
			return false;

		Array<VkQueueFamilyProperties> queues(count);

		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queues.val());

		uint32_t bestFamily = std::numeric_limits<uint32_t>::max();
		int bestScore = -1;

		for (uint32_t i = 0; i < count; i++) {
			const VkQueueFamilyProperties &queue = queues[i];

			if (!queue.queueCount)
				continue;

			if (!(queue.queueFlags & VK_QUEUE_GRAPHICS_BIT))
				continue;

			if (!platformPresentationSupport(device, i))
				continue;

			int score = 0;
			bool compute = queue.queueFlags & VK_QUEUE_COMPUTE_BIT;

			// Prefer a universal graphics+compute family, but graphics-only is valid.
			if (compute)
				score += 1000;

			// 同一 family 中可用 Queue 越多，未来越容易拆分上传和渲染。
			score += int(std::min(queue.queueCount, 8u)) * 10;

			// Graphics/Compute Queue 已经可以执行 transfer，
			// 这里仅作为显式能力信息的微小加分。
			if (queue.queueFlags & VK_QUEUE_TRANSFER_BIT)
				score += 1;

			// Qk 暂时不需要 protected/sparse，避免优先选择特殊 family。
			if (queue.queueFlags & VK_QUEUE_PROTECTED_BIT)
				score -= 1;

			if (score > bestScore) {
				bestScore = score;
				bestFamily = i;
				*computeSupport = compute;
			}
		}

		if (bestFamily == U32::limit_max)
			return false;

		*family = bestFamily;
		return true;
	}

	static VkDeviceSize getDeviceLocalMemory(const VkPhysicalDeviceMemoryProperties &memory) {
		VkDeviceSize size = 0;

		for (uint32_t i = 0; i < memory.memoryHeapCount; i++) {
			const VkMemoryHeap &heap = memory.memoryHeaps[i];

			if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				size += heap.size;
		}

		return size;
	}

	static int64_t deviceTypeScore(VkPhysicalDeviceType type) {
		switch (type) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				return 1000000;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				return 500000;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				return 100000;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				return 10000;
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			default:
				return 1000;
		}
	}

	static VulkanDeviceCandidate scoreDevice(VkPhysicalDevice device) {
		VulkanDeviceCandidate candidate;
		candidate.device = device;

		// Swapchain 是 Qk Vulkan 显示后端的硬性要求。
		if (!vk_supportsDeviceExtension(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
			return VulkanDeviceCandidate();
		}

		if (!findGraphicsQueueFamily(
			device,
			&candidate.queueFamily,
			&candidate.computeSupport
		)) {
			return VulkanDeviceCandidate();
		}

		VkPhysicalDeviceProperties properties = {};
		VkPhysicalDeviceMemoryProperties memory = {};

		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceMemoryProperties(device, &memory);

		int64_t score = deviceTypeScore(properties.deviceType);

		// Compute is optional, but prefer it when otherwise choosing similar GPUs.
		if (candidate.computeSupport)
			score += 10000;

		/*
		 * 显存只作为同类型 GPU 的辅助判断。
		 *
		 * 每 256 MiB 加一分，最多加 4096 分。
		 * 限制上限是为了避免 UMA 报告的大块共享内存影响设备类型排序。
		 */
		const VkDeviceSize unit = 256ull * 1024ull * 1024ull;
		const VkDeviceSize localMemory = getDeviceLocalMemory(memory);
		const VkDeviceSize memoryUnits = localMemory / unit;

		score += U64::min(memoryUnits, 4096);

		/*
		 * Limits 表示能力上限，不等于真实性能，因此只能给少量分数。
		 */
		score += U32::min(properties.limits.maxImageDimension2D / 1024, 32);

		score += U32::min(properties.limits.maxPerStageDescriptorSampledImages, 256);

		if (candidate.computeSupport) {
			score += U32::min(properties.limits.maxComputeSharedMemorySize / 1024, 128);
		}

		candidate.score = score;
		return candidate;
	}

	bool vk_selectBestDevice(
		VkInstance instance,
		VkPhysicalDevice *selectedDevice,
		uint32_t *selectedQueueFamily,
		bool *selectedComputeSupport
	) {
		uint32_t count = 0;
		VkResult result = vkEnumeratePhysicalDevices(instance, &count, nullptr);

		if (result != VK_SUCCESS || !count) {
			Qk_DLog("vkEnumeratePhysicalDevices failed: %d", int(result));
			return false;
		}

		Array<VkPhysicalDevice> devices(count);

		result = vkEnumeratePhysicalDevices(instance, &count, devices.val());

		if (result != VK_SUCCESS) {
			Qk_DLog("vkEnumeratePhysicalDevices failed: %d", int(result));
			return false;
		}

		VulkanDeviceCandidate best;

		for (uint32_t i = 0; i < count; i++) {
			VulkanDeviceCandidate candidate = scoreDevice(devices[i]);

			VkPhysicalDeviceProperties properties = {};
			vkGetPhysicalDeviceProperties(devices[i], &properties);

			if (!candidate.valid()) {
				Qk_DLog("Skip Vulkan device: %s", properties.deviceName);
				continue;
			}

			Qk_DLog(
				"Vulkan device candidate: %s, score=%lld, family=%u, compute=%d",
				properties.deviceName,
				static_cast<long long>(candidate.score),
				candidate.queueFamily,
				candidate.computeSupport
			);

			if (!best.valid() || candidate.score > best.score)
				best = candidate;
		}

		if (!best.valid()) {
			Qk_DLog("No Vulkan device with graphics+present and swapchain support");
			return false;
		}

		VkPhysicalDeviceProperties properties = {};
		vkGetPhysicalDeviceProperties(
			best.device,
			&properties
		);

		Qk_DLog(
			"Selected Vulkan device: %s, score=%lld, family=%u, compute=%d",
			properties.deviceName,
			static_cast<long long>(best.score),
			best.queueFamily,
			best.computeSupport
		);

		*selectedDevice = best.device;
		*selectedQueueFamily = best.queueFamily;
		*selectedComputeSupport = best.computeSupport;
		return true;
	}

	bool vk_createInstance(VkInstance *instance) {
		const char *platformExtension = vk_platformSurfaceExtension();
		if (!platformExtension) {
			Qk_DLog("No Vulkan platform surface extension");
			return false;
		}

		const char *extensions[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			platformExtension,
		};
		VkApplicationInfo app = {};
		app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app.pApplicationName = "Quark Render Backend for Vulkan";
		app.pEngineName = "Quark";
		app.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pApplicationInfo = &app;
		info.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);
		info.ppEnabledExtensionNames = extensions;
		VkResult result = vkCreateInstance(&info, nullptr, instance);
		return result == VK_SUCCESS;
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
}
