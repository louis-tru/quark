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

#include "./vk_canvas.h"
#include "./vk_render.h"
#include "../source.h"

namespace qk {
	void VulkanCanvas::setSurface(const Mat4 &root, Vec2 surfaceSize, Vec2 scale) {
		Mat4 matrix = root;
		matrix.scale_y(-1);
		matrix.translate_y(-surfaceSize.y() / scale.y());
		matrix.translate_z(0.5f);
		matrix.scale_z(0.5f);
		GPUCanvas::setSurface(matrix, surfaceSize, scale);
	}

	void VulkanCanvas::setDefaultTarget(VkTexture *target) {
		if(_outTex)
			vk_deleteTextureSafe(_device, _outTex);
		_outTex = target;
		Qk_ASSERT(_outTex, "Failed to create Vulkan canvas output texture");
		_framebuffer.clearSafe(_device);
		_framebuffer.framebuffer = vk_create_framebuffer(
			_device, getRenderPass(_outTex->format), _outTex->view, _outTex->extent);
		_framebuffer.view = _outTex->view;
		Qk_ASSERT(_framebuffer.framebuffer, "Failed to create Vulkan canvas framebuffer");
		_target = _outTex; // set current render target to default output texture
	}

	void VulkanCanvas::setSurfaceCmd(bool changeSize) {
		endPass(); // end old pass if exist

		// create new output texture if size changed
		if (changeSize) {
			setDefaultTarget(_resource->newTexture(_surfaceSize, _opts.colorType, 1, kNone_TextureFlags));
		}
		// clear buffer allocators for new frame
		_cmdPack.allocator[0]->clear();
		_cmdPack.allocator[1]->clear();
		_cmdPackFront.allocator[0]->clear(kSafeDeleteVkMemBlock_Flag);
		_cmdPackFront.allocator[1]->clear(kSafeDeleteVkMemBlock_Flag);
	}

	void VulkanCanvas::setMatrixCmd() {
		if (_cmdPack.renderPass)
			updateViewMatrixSet();
	}

	void VulkanCanvas::setBlendModeCmd() {}

	void VulkanCanvas::drawClipCmd(const VertexData&, GC_State::Clip*, GC_State::Clip*, ClipOp) {
	}

	void VulkanCanvas::restoreClipCmd(GC_State::Clip*) {}

	void VulkanCanvas::clearColorCmd(const Color4f &color, GC_ClearFlags) {
	}

	void VulkanCanvas::drawImageCmd(const VertexData&, const GC_ImageDrawInfo&) {}
	void VulkanCanvas::drawGradientCmd(const VertexData&, const PaintGradient*, const Color4f&) {}
	void VulkanCanvas::drawColorCmd(const VertexData &vertex, const Color4f &color) {
		auto &shader = _shaders.color;
		auto cmd = usePipeline(shader, vertex);
		SpvColor::PcArgs pc{premul_alpha(color), Vec4(0), _flags};
		vkCmdPushConstants(cmd, shader.layout(), shader.pc.stages, 0, sizeof(pc), &pc);
		vkCmdDraw(cmd, vertex.vCount, 1, 0, 0);
	}
	bool VulkanCanvas::drawCAPACmd(CAPADrawData&) { return false; }
	void VulkanCanvas::drawRRectBlurColorCmd(const Rect&, const float*, float, const Color4f&) { }
	void VulkanCanvas::blurFilterBeginCmd(Range, Mat4&, ImageSource*) {}
	void VulkanCanvas::blurFilterEndCmd(Range, Mat4&, float, float, int, int, ImageSource*, ImageSource*) {}
	void VulkanCanvas::drawTrianglesCmd(const Triangles&, const PaintImage*, const Color4f&, bool) { }
	void VulkanCanvas::readImageCmd(const Rect&, ImageSource*, ImageSource*) {}
	void VulkanCanvas::outputImageBeginCmd(ImageSource*) {}
	void VulkanCanvas::outputImageEndCmd(ImageSource*) {}
	void VulkanCanvas::flushSubcanvasCmd(GPUCanvas*) {}

}
