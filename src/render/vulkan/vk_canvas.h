/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * ***** END LICENSE BLOCK ***** */

// @private head

#ifndef __quark_render_vulkan_vk_canvas__
#define __quark_render_vulkan_vk_canvas__

#include "../gpu_canvas.h"

namespace qk {

	class VulkanRender;

	class VulkanCanvas: public GPUCanvas {
	public:
		VulkanCanvas(VulkanRender *render, Render::Options opts);
		~VulkanCanvas() override;
		bool swapBuffer() override;
		Color4f consumeClearColor();
	private:
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
		VulkanRender *_vkRender;
		Color4f _clearColor, _frontClearColor;
		bool _recorded, _frontReady;
	};

}

#endif
