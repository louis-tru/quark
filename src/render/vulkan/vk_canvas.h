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

// @private head

#ifndef __quark_render_vulkan_vk_canvas__
#define __quark_render_vulkan_vk_canvas__

#include "../gpu_canvas.h"
#include "./vk_shaders.h"
#include "./vk_util.h"

namespace qk {
	class VulkanCanvas;
	class VulkanRender;
	class VulkanRenderResource;

	struct VkFramebufferData {
		VkFramebuffer framebuffer = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
		bool holdView = false; // is hold view
		void clearSafe(VkDevice device);
		inline ~VkFramebufferData() {
			Qk_ASSERT(!framebuffer && !view, "Vulkan framebuffer should be released before destruction");
		}
	};

	struct VkDescriptorPools {
		List<VkDescriptorPool> pools;
		List<VkDescriptorPool>::Iterator iter;
		VkDescriptorPool createDescriptorPool(VkDevice device);
		VkDescriptorSet allocateDescriptorSet(VkDevice device, VkDescriptorSetLayout setLayout);
		inline ~VkDescriptorPools() {
			Qk_ASSERT(pools.isNull(), "Vulkan descriptor pools should be released before destruction");
		}
		void reset(VkDevice device);
		void destroy(VkDevice device);
	};

	struct VkCmdPack {
		inline bool isRecorded() const {
			return recorded || commands.length();
		}
		void initialize(VkDevice device);
		Sp<VkMemBufferAllocator> allocator[2]; // 0: host visible, 1: device local
		Array<VkCommandBuffer> commands; // recorded command buffers for this pack
		VkCommandBuffer current; // current command buffer for recording
		VkDescriptorPools descriptorPools; // descriptor pools for this pack
		VkDescriptorSet set0; // common set, 0: image sampler, binding=1: root uniform buffer
		VkDescriptorBufferInfo buffers[3]; // root, view, clip;
		VkRenderPass renderPass; // current render pass for this pack
		VkPipeline pipeline; // current pipeline for this pack
		VkTexture *target; // current render target for this pack
		uint32_t level; // current mip level for this pack
		VkClearColorValue clearColor; // current clear color for this pack
		VkAttachmentLoadOp loadOp;
		VkAttachmentStoreOp storeOp;
		bool beginPass; // is begin pass for this pack
		bool recorded; // is recorded command buffer for this pack
		bool commonSetDirty; // is common descriptor set dirty for this pack
	};

	class VulkanCanvas: public GPUCanvas {
	public:
		VulkanCanvas(VulkanRender *render, Render::Options opts);
		~VulkanCanvas() override;
		bool swapBuffer() override;
		Array<VkCommandBuffer> flushBuffer();
		bool isRecorded() const { return _cmdPackFront.isRecorded(); }
		void setDefaultTarget(VkTexture *target);
	private:
		void beginRenderPassReady();
		void beginRenderPass();
		void beginPass(int level = 0, bool loadColor = true, Color4f *clearColor = nullptr);
		void endPass();
		void updateRootMatrixSet();
		void updateViewMatrixSet();
		void bindCommonDescriptorSet(VkShader& shader);
		void updateCommonDescriptorSet(bool allocBuff);
		void updateDescriptorSet(VkDescriptorSet set, uint32_t binding, VkTexture tex, VkSampler sampler);
		void updateDescriptorSet(VkDescriptorSet set, uint32_t binding, cVkMemBlock& buffer);
		void bindDescriptorSet(VkDescriptorSet set, VkShader& shader, uint32_t bindSet = 0,
				VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS);
		inline VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout setLayout) {
			return _cmdPack.descriptorPools.allocateDescriptorSet(_device, setLayout);
		}
		// void setPipeline(VkCommandBuffer cmd, VkPipeline pipeline);
		VkCommandBuffer usePipeline(VkShader &shader);
		VkCommandBuffer usePipeline(VkShader &shader, const VertexData &vertex);
		VkSampler get_sampler(const PaintImage* paint);
		VkSampler get_sampler(PaintImage::FilterMode filter, PaintImage::MipmapMode mipmap);
		VkRenderPass getRenderPass(
			VkFormat format, VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			VkImageLayout finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		void resetCmdPack(VkCmdPack &pack);
		void setSurface(const Mat4 &root, Vec2 surfaceSize, Vec2 scale) override;
		void setSurfaceCmd(bool changeSize) override;
		void setMatrixCmd() override;
		void setBlendModeCmd() override;
		void drawClipCmd(const VertexData &vertex, GC_State::Clip *lastClip,
			GC_State::Clip *clip, ClipOp rawOp) override;
		void restoreClipCmd(GC_State::Clip *clip) override;
		void clearColorCmd(const Color4f &color, GC_ClearFlags flags) override;
		void drawImageCmd(const VertexData &vertex, const GC_ImageDrawInfo &info) override;
		void drawGradientCmd(const VertexData &vertex, const PaintGradient *paint,
			const Color4f &color) override;
		void drawColorCmd(const VertexData &vertex, const Color4f &color) override;
		bool drawCAPACmd(CAPADrawData &data) override;
		void drawRRectBlurColorCmd(const Rect &rect, const float *radius, float blur,
			const Color4f &color) override;
		void blurFilterBeginCmd(Range bounds, Mat4 &rootMat, ImageSource *tmpA) override;
		void blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius,
			float clearPad, int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) override;
		void drawTrianglesCmd(const Triangles &triangles, const PaintImage *paint,
			const Color4f &color, bool copyData) override;
		void readImageCmd(const Rect &srcRect, ImageSource *src, ImageSource *dst) override;
		void outputImageBeginCmd(ImageSource *dst) override;
		void outputImageEndCmd(ImageSource *exit) override;
		void flushSubcanvasCmd(GPUCanvas *canvas) override;
	private:
		// fields:
		VulkanRender *_vkRender; // render backend
		VulkanRenderResource *_resource; // shared Vulkan device resource
		VkDevice _device;
		VkCommandPool _commandPool; // owned by this canvas; all local command buffers come from it
		VkFramebufferData _framebuffer;
		VkTexture *_target, *_outTex; // current / default render target
		VkTexture *_emptyTex; // empty texture for fallback
		VkCmdPack _cmdPack, _cmdPackFront; // current and front command pack for render
		VkShaders _shaders; // canvas-local reflection and pipeline handle cache
		Dict<uint32_t, VkSampler> _texSamplers;
		Dict<uint64_t, VkRenderPass> _renderPasss;
		friend class VkCmdPack;
	};

}

#endif
