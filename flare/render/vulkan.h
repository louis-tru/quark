/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __flare__render__vulkan__
#define __flare__render__vulkan__

#include "skia/core/SkTypes.h"

#ifdef SK_VULKAN

#include "skia/gpu/vk/GrVkVulkan.h"
#include "skia/gpu/vk/GrVkBackendContext.h"
#include "./render.h"

struct GrVkInterface;

namespace flare {

	class VulkanRender : public Render {
	public:
		~VulkanRender() override;

		sk_sp<SkSurface> getBackbufferSurface() override;
		void swapBuffers() override;

		bool isValid() override { return fDevice != VK_NULL_HANDLE; }

		void resize(int w, int h) override {
			this->createSwapchain(w, h, fDisplayParams);
		}

		void setDisplayParams(const DisplayParams& params) override {
			this->destroyContext();
			fDisplayParams = params;
			this->initializeContext();
		}

		/** Platform specific function that creates a VkSurfaceKHR for a window */
		using CreateVkSurfaceFn = std::function<VkSurfaceKHR(VkInstance)>;
		/** Platform specific function that determines whether presentation will succeed. */
		using CanPresentFn = sk_gpu_test::CanPresentFn;

		VulkanRender(Application* host, const DisplayParams&, CreateVkSurfaceFn, CanPresentFn,
							PFN_vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr);

		bool isGpuContext() override { return true; }

	private:
		void initializeContext();
		void destroyContext();

		struct BackbufferInfo {
			uint32_t        fImageIndex;          // image this is associated with
			VkSemaphore     fRenderSemaphore;     // we wait on this for rendering to be done
		};

		BackbufferInfo* getAvailableBackbuffer();
		bool createSwapchain(int width, int height, const DisplayParams& params);
		bool createBuffers(VkFormat format, VkImageUsageFlags, SkColorType colorType, VkSharingMode);
		void destroyBuffers();

		VkInstance fInstance = VK_NULL_HANDLE;
		VkPhysicalDevice fPhysicalDevice = VK_NULL_HANDLE;
		VkDevice fDevice = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT fDebugCallback = VK_NULL_HANDLE;

		// Create functions
		CreateVkSurfaceFn fCreateVkSurfaceFn;
		CanPresentFn      fCanPresentFn;

		// Vulkan GetProcAddr functions
		PFN_vkGetInstanceProcAddr fGetInstanceProcAddr = nullptr;
		PFN_vkGetDeviceProcAddr fGetDeviceProcAddr = nullptr;

		// WSI interface functions
		PFN_vkDestroySurfaceKHR fDestroySurfaceKHR = nullptr;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR fGetPhysicalDeviceSurfaceSupportKHR = nullptr;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fGetPhysicalDeviceSurfaceCapabilitiesKHR =nullptr;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fGetPhysicalDeviceSurfacePresentModesKHR =nullptr;

		PFN_vkCreateSwapchainKHR fCreateSwapchainKHR = nullptr;
		PFN_vkDestroySwapchainKHR fDestroySwapchainKHR = nullptr;
		PFN_vkGetSwapchainImagesKHR fGetSwapchainImagesKHR = nullptr;
		PFN_vkAcquireNextImageKHR fAcquireNextImageKHR = nullptr;
		PFN_vkQueuePresentKHR fQueuePresentKHR = nullptr;

		PFN_vkDestroyInstance fDestroyInstance = nullptr;
		PFN_vkDeviceWaitIdle fDeviceWaitIdle = nullptr;
		PFN_vkDestroyDebugReportCallbackEXT fDestroyDebugReportCallbackEXT = nullptr;
		PFN_vkQueueWaitIdle fQueueWaitIdle = nullptr;
		PFN_vkDestroyDevice fDestroyDevice = nullptr;
		PFN_vkGetDeviceQueue fGetDeviceQueue = nullptr;

		sk_sp<const GrVkInterface> fInterface;

		VkSurfaceKHR      fSurface;
		VkSwapchainKHR    fSwapchain;
		uint32_t          fGraphicsQueueIndex;
		VkQueue           fGraphicsQueue;
		uint32_t          fPresentQueueIndex;
		VkQueue           fPresentQueue;

		uint32_t               fImageCount;
		VkImage*               fImages;         // images in the swapchain
		VkImageLayout*         fImageLayouts;   // layouts of these images when not color attachment
		sk_sp<SkSurface>*      fSurfaces;       // surfaces client renders to (may not be based on rts)
		BackbufferInfo*        fBackbuffers;
		uint32_t               fCurrentBackbufferIndex;
	};

}   // namespace flare

#endif // SK_VULKAN

#endif
