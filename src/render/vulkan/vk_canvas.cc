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

namespace qk {
	void clear_PathvCache(PathvCache *cache, int flags);
	void clearExec_PathvCache(PathvCache *cache);

	VulkanCanvas::VulkanCanvas(VulkanRender *render, Render::Options opts)
		: GPUCanvas(render, opts)
		, _vkRender(render)
		, _clearColor(0, 0, 0, 0)
		, _frontClearColor(0, 0, 0, 0)
		, _recorded(false)
		, _frontReady(false)
	{
	}

	VulkanCanvas::~VulkanCanvas() {
	}

	bool VulkanCanvas::swapBuffer() {
		ScopeLock lock(_mutex);
		if (_frontReady)
			return false;
		_frontClearColor = _clearColor;
		_frontReady = _recorded;
		_recorded = false;
		clear_PathvCache(_cache, 0);
		return true;
	}

	Color4f VulkanCanvas::consumeClearColor() {
		ScopeLock lock(_mutex);
		auto color = _frontReady ? _frontClearColor : _clearColor;
		_frontReady = false;
		clearExec_PathvCache(_cache);
		return color;
	}

	void VulkanCanvas::setSurfaceCmd(bool changeSize) {
		_recorded = true;
	}

	void VulkanCanvas::setMatrixCmd() {}
	void VulkanCanvas::setBlendModeCmd() {}

	void VulkanCanvas::drawClipCmd(const VertexData&, GC_State::Clip*, GC_State::Clip*, ClipOp) {
		_recorded = true;
	}

	void VulkanCanvas::restoreClipCmd(GC_State::Clip*) {}

	void VulkanCanvas::clearColorCmd(const Color4f &color, GC_ClearFlags) {
		_clearColor = color;
		_recorded = true;
	}

	void VulkanCanvas::drawImageCmd(const VertexData&, const GC_ImageDrawInfo&) { _recorded = true; }
	void VulkanCanvas::drawGradientCmd(const VertexData&, const PaintGradient*, const Color4f&) { _recorded = true; }
	void VulkanCanvas::drawColorCmd(const VertexData&, const Color4f&) { _recorded = true; }
	bool VulkanCanvas::drawCAPACmd(CAPADrawData&) { return false; }
	void VulkanCanvas::drawRRectBlurColorCmd(const Rect&, const float*, float, const Color4f&) { _recorded = true; }
	void VulkanCanvas::blurFilterBeginCmd(Range, Mat4&, ImageSource*) {}
	void VulkanCanvas::blurFilterEndCmd(Range, Mat4&, float, float, int, int, ImageSource*, ImageSource*) {}
	void VulkanCanvas::drawTrianglesCmd(const Triangles&, const PaintImage*, const Color4f&, bool) { _recorded = true; }
	void VulkanCanvas::readImageCmd(const Rect&, ImageSource*, ImageSource*) {}
	void VulkanCanvas::outputImageBeginCmd(ImageSource*) {}
	void VulkanCanvas::outputImageEndCmd(ImageSource*) {}
	void VulkanCanvas::flushSubcanvasCmd(GPUCanvas*) {}

}
