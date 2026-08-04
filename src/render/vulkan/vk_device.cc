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
#include "../arguments.h"

#if Qk_LINUX
# include "../plotforms.h"
# undef Status
# undef Bool
# undef None
#endif

namespace qk {

	static const char* vk_deviceTypeName(VkPhysicalDeviceType type) {
		switch (type) {
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				return "INTEGRATED_GPU";
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				return "DISCRETE_GPU";
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				return "VIRTUAL_GPU";
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				return "CPU";
			default:
				return "OTHER";
		}
	}

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
#elif Qk_LINUX
		auto display = openXDisplay();
		auto visual = DefaultVisual(display, DefaultScreen(display));
		return vkGetPhysicalDeviceXlibPresentationSupportKHR(
			device, family, display, XVisualIDFromVisual(visual));
#else
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

	static bool vk_supportsDeviceExtension(VkPhysicalDevice device, const char *requiredExtension) {
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

	void vk_logDeviceInfo(VkPhysicalDevice device, const VkPhysicalDeviceProperties &properties) {
		Qk_DLog("VK_DEVICE_NAME: %s", properties.deviceName);
		Qk_DLog("VK_DEVICE_TYPE: %s", vk_deviceTypeName(properties.deviceType));
		Qk_DLog("VK_VENDOR_ID: 0x%04x", properties.vendorID);
		Qk_DLog("VK_DEVICE_ID: 0x%08x", properties.deviceID);
		Qk_DLog("VK_API_VERSION: %u.%u.%u",
			VK_VERSION_MAJOR(properties.apiVersion),
			VK_VERSION_MINOR(properties.apiVersion),
			VK_VERSION_PATCH(properties.apiVersion));
		Qk_DLog("VK_DRIVER_VERSION: %u (0x%08x)",
			properties.driverVersion, properties.driverVersion);

		uint32_t count = 0;
		VkResult result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
		if (result != VK_SUCCESS) {
			Qk_DLog("VK_DEVICE_EXTENSIONS: query failed: %d", int(result));
			return;
		}

		Array<VkExtensionProperties> extensions(count);
		if (count) {
			result = vkEnumerateDeviceExtensionProperties(
				device, nullptr, &count, extensions.val());
			if (result != VK_SUCCESS) {
				Qk_DLog("VK_DEVICE_EXTENSIONS: query failed: %d", int(result));
				return;
			}
		}

		Qk_DLog("VK_DEVICE_EXTENSIONS: %u", count);
		for (uint32_t i = 0; i < count; i++) {
			Qk_DLog("  %s (specVersion=%u)",
				extensions[i].extensionName, extensions[i].specVersion);
		}
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
		app.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pApplicationInfo = &app;
		info.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);
		info.ppEnabledExtensionNames = extensions;
		VkResult result = vkCreateInstance(&info, nullptr, instance);
		return result == VK_SUCCESS;
	}

	static uint32_t capaPreferredImageCount(const VkPhysicalDeviceProperties &properties) {
		switch (properties.deviceType) {
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				return 512;
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				return 256;
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				return 128;
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			default:
				return 64;
		}
	}

	static bool vk_capaSupport(VkPhysicalDevice physicalDevice,
		const VkPhysicalDeviceProperties &properties,
		const VkPhysicalDeviceDescriptorIndexingFeaturesEXT &supportedDescriptorFeatures,
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT *enabledDescriptorFeatures,
		uint32_t *maxImageCount
	) {
		*maxImageCount = 1;
		if (runArguments && runArguments->options.has("aaside")) {
			return false; // Disable CAPA, if argument --aaside is specified.
		}
		if (!vk_supportsDeviceExtension(
			physicalDevice, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME))
			return false;

		auto &limits = properties.limits;
		// CAPA keeps sampled images and samplers in two variable-length descriptor arrays.
		// Clamp their shared length against every descriptor limit involved. The
		// fixed clip sampler consumes one sampled-image and one sampler slot first.
		auto sampledImages = limits.maxPerStageDescriptorSampledImages > 1 ?
			limits.maxPerStageDescriptorSampledImages - 1: 0;
		auto setSampledImages = limits.maxDescriptorSetSampledImages > 1 ?
			limits.maxDescriptorSetSampledImages - 1: 0;
		auto samplers = limits.maxPerStageDescriptorSamplers > 1 ?
			limits.maxPerStageDescriptorSamplers - 1: 0;
		auto setSamplers = limits.maxDescriptorSetSamplers > 1 ?
			limits.maxDescriptorSetSamplers - 1: 0;
		*maxImageCount = U32::min(capaPreferredImageCount(properties), sampledImages);
		*maxImageCount = U32::min(*maxImageCount, setSampledImages);
		*maxImageCount = U32::min(*maxImageCount, samplers);
		*maxImageCount = U32::min(*maxImageCount, setSamplers);
		// capa_composite currently has 12 fixed resources: 10 storage buffers,
		// one sampled clip and one destination storage image. Reserve 16 slots so
		// a few future fixed bindings do not silently make the runtime image arrays
		// exceed maxPerStageResources.
		// Each CAPA image consumes two more resources: one image and one sampler.
		constexpr uint32_t fixedResourceReserve = 16;
		auto arrayResources = limits.maxPerStageResources > fixedResourceReserve ?
			(limits.maxPerStageResources - fixedResourceReserve) >> 1: 0;
		*maxImageCount = U32::min(*maxImageCount, arrayResources);

		VkFormatProperties rgba8{};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, VK_FORMAT_R8G8B8A8_UNORM, &rgba8);
		// CAPA needs non-uniform indexing into runtime image/sampler arrays. Both
		// arrays use a variable descriptor count chosen from maxImageCount above.
		auto supported =
			supportedDescriptorFeatures.runtimeDescriptorArray &&
			supportedDescriptorFeatures.shaderSampledImageArrayNonUniformIndexing &&
			supportedDescriptorFeatures.descriptorBindingVariableDescriptorCount &&
			// R8 sampling is mandatory; only verify the RGBA8 storage-image output.
			(rgba8.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT) &&
			// capa_composite uses descriptor sets 0..3, ten storage buffers and one
			// storage image in the compute stage.
			limits.maxBoundDescriptorSets >= 4 &&
			limits.maxPerStageDescriptorStorageBuffers >= 10 &&
			limits.maxDescriptorSetStorageBuffers >= 10 &&
			limits.maxPerStageDescriptorStorageImages >= 1 &&
			limits.maxDescriptorSetStorageImages >= 1 &&
			// Current CAPA kernels require at most 32 threads on X, 8 on Y, and
			// 64 total invocations for the 8x8 composite kernel.
			limits.maxComputeWorkGroupInvocations >= 64 &&
			limits.maxComputeWorkGroupSize[0] >= 32 &&
			limits.maxComputeWorkGroupSize[1] >= 8 &&
			*maxImageCount;
		if (!supported)
			return false;

		enabledDescriptorFeatures->runtimeDescriptorArray = VK_TRUE;
		enabledDescriptorFeatures->shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
		enabledDescriptorFeatures->descriptorBindingVariableDescriptorCount = VK_TRUE;
		return true;
	}

	bool vk_createDevice(VkPhysicalDevice physicalDevice,
		const VkPhysicalDeviceProperties &properties,
		uint32_t queueFamily, VkDevice *device,
		bool *pvrtcSupport, bool *capaSupport, uint32_t *capaMaxImageCount) {
		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);
		Array<VkQueueFamilyProperties> families(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, families.val());
		Qk_ASSERT(queueFamily < familyCount, "Invalid graphics queue family index");
		if (queueFamily >= familyCount)
			return false;

		uint32_t queueCount = std::min(families[queueFamily].queueCount, 1u);
		Qk_ASSERT(queueCount > 0, "No queues available in graphics queue family");
		if (queueCount == 0)
			return false;

		float priorities[] = { 1.0f, 1.0f };
		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamily;
		queueInfo.queueCount = queueCount;
		queueInfo.pQueuePriorities = priorities;

		VkPhysicalDeviceDescriptorIndexingFeaturesEXT supportedDescriptorFeatures{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT};
		VkPhysicalDeviceFeatures2 supportedFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
		supportedFeatures.pNext = &supportedDescriptorFeatures;
		vkGetPhysicalDeviceFeatures2(physicalDevice, &supportedFeatures);

		// Add extensions to the device if supported.
		Array<const char*> extensions;
		extensions.push(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		*pvrtcSupport = vk_supportsDeviceExtension(physicalDevice, VK_IMG_FORMAT_PVRTC_EXTENSION_NAME);
		if (*pvrtcSupport)
			extensions.push(VK_IMG_FORMAT_PVRTC_EXTENSION_NAME);
		// check for CAPA support and enable the extension if supported.
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorFeatures{
			VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT};
		*capaSupport = vk_capaSupport(
			physicalDevice, properties, supportedDescriptorFeatures,
			&descriptorFeatures, capaMaxImageCount);
		if (*capaSupport)
			extensions.push(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

		// create the device with the requested extensions and features.
		VkPhysicalDeviceFeatures enabledFeatures = {};
		enabledFeatures.textureCompressionETC2 = supportedFeatures.features.textureCompressionETC2;
		enabledFeatures.textureCompressionBC = supportedFeatures.features.textureCompressionBC;
		// enabledFeatures.shaderStorageImageExtendedFormats =
		// 	supportedFeatures.features.shaderStorageImageExtendedFormats;
		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = *capaSupport ? &descriptorFeatures: nullptr;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.enabledExtensionCount = extensions.length();
		deviceInfo.ppEnabledExtensionNames = extensions.val();
		deviceInfo.pEnabledFeatures = &enabledFeatures;
		auto result = vkCreateDevice(physicalDevice, &deviceInfo, nullptr, device);
		if (result != VK_SUCCESS)
			Qk_DLog("vkCreateDevice failed: %d", int(result));
		if (result == VK_SUCCESS) {
			Qk_DLog("Vulkan CAPA: %s, max images=%u",
				*capaSupport ? "supported": "unsupported", *capaMaxImageCount);
		}
		return result == VK_SUCCESS;
	}
}
