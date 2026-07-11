/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_vulkan_vk_render__
#define __quark_render_vulkan_vk_render__

#if Qk_ANDROID && !defined(VK_USE_PLATFORM_ANDROID_KHR)
# define VK_USE_PLATFORM_ANDROID_KHR 1
#endif

#include <vulkan/vulkan.h>
#include <vector>
#include "../render.h"
#include "../plotforms.h"
#include "./vk_canvas.h"

namespace qk {

	class VulkanRender final: public RenderBackend, public RenderSurface {
	public:
		~VulkanRender() override;
		void release() override;
		void reload() override;
		Canvas* createCanvas(Options opts) override;
		RenderSurface* surface() override { return this; }
		void post_message(Cb cb) override;
		TexStat createTextureStat(Vec2 size, ColorType type, uint8_t flags) override;
		bool uploadTexture(cPixel *pix, int levels, TexStat *out, bool mipmap) override;
		void unloadTexture(TexStat *tex) override;
		bool uploadVertexData(VertexData::ID *id) override;
		void unloadVertexData(VertexData::ID *id) override;
		void lock();
		void unlock();

		void makeSurface(EGLNativeWindowType win) override;
		void deleteSurface() override;
		void renderDisplay() override;
		void renderLoopRun() override;
		void renderLoopStop() override;
		Vec2 getSurfaceSize() override;

		VkDevice device() const { return _device; }
		VkPhysicalDevice physicalDevice() const { return _physicalDevice; }
		VkQueue queue() const { return _queue; }
		uint32_t queueFamily() const { return _queueFamily; }
	private:
		explicit VulkanRender(Options opts);
		bool createDevice();
		bool createSwapchain();
		void destroySwapchain();
		void destroyDevice();
		void drawFrame(const Color4f &clearColor);
		VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) const;
		VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> &modes) const;
		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR &caps) const;
	private:
		VkInstance _instance;
		VkPhysicalDevice _physicalDevice;
		VkDevice _device;
		VkQueue _queue;
		uint32_t _queueFamily;
		VkSurfaceKHR _surface;
		VkSwapchainKHR _swapchain;
		VkFormat _swapchainFormat;
		VkExtent2D _swapchainExtent;
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainViews;
		std::vector<VkFramebuffer> _framebuffers;
		VkRenderPass _renderPass;
		VkCommandPool _commandPool;
		std::vector<VkCommandBuffer> _commandBuffers;
		VkSemaphore _imageAvailable[2];
		VkSemaphore _renderFinished[2];
		VkFence _inFlight[2];
		uint32_t _frameIndex;
		EGLNativeWindowType _window;
		VulkanCanvas *_vkCanvas;
		RecursiveMutex _mutex;
		ThreadID _threadId;
		ThreadID _loopThreadId;
		friend Render* make_vulkan_render(Render::Options opts);
	};

}

#endif
