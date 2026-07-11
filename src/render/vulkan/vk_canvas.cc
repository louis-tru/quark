/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
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
