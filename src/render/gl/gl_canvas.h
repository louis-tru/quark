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

#ifndef __quark_render_gl_glcanvas__
#define __quark_render_gl_glcanvas__

#include "../gpu_canvas.h"
#include "./glsl_shaders.h"

namespace qk {
	class GLRender;
	class GLC_CmdPack;

	class GLCanvas: public GPUCanvas {
	public:
		GLCanvas(GLRender *render, Render::Options opts);
		~GLCanvas() override;
		bool swapBuffer() override; // swap gl double cmd pkg, success return true
		void flushBuffer(); // commit gl cmd, only can rendering thread call
		void vportCopy(GLuint dstFBO);
		bool readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst);
		GLuint fbo() { return _fbo; }
	private:
		void checkMatrix();
		void setBuffers(Vec2 surfaceSize);
		void setSurfaceCmd(bool changeSize) override;
		void setMatrixCmd() override;
		void setBlendModeCmd() override;
		void drawClipCmd(const VertexData &vertex, GC_State::Clip *lastClip,
				GC_State::Clip *clip, ClipOp rawOp) override;
		void clearColorCmd(const Color4f &color, GC_ClearFlags flags) override;
		void drawImageCmd(const VertexData &vertex, const GC_ImageDrawInfo &info) override;
		void drawGradientCmd(const VertexData &vertex, const PaintGradient *paint, const Color4f &color) override;
		void drawColorCmd(const VertexData &vertex, const Color4f &color) override;
		void drawRRectBlurColorCmd(const Rect& rect, const float *radius, float blur, const Color4f &color) override;
		void blurFilterBeginCmd(Range bounds, Mat4 &rootMat, ImageSource *tmpA) override;
		void blurFilterEndCmd(Range bounds, Mat4 &recoverRootMat, float radius, float clearPad,
				int sample, int imageLod, ImageSource *tmpA, ImageSource *tmpB) override;
		void drawTrianglesCmd(const Triangles& triangles, const PaintImage *paint, const Color4f &color, bool copyData) override;
		void readImageCmd(const Rect &srcRect, ImageSource* src, ImageSource* dest) override;
		void outputImageBeginCmd(ImageSource* img) override;
		void outputImageEndCmd(ImageSource* exit) override;
		void restoreClipCmd(GC_State::Clip* clip) override;
		void flushSubcanvasCmd(GPUCanvas* canvas) override;
		bool drawCAPACmd(CAPADrawData &data) override;
	private:
	// fields:
		GLRender *_render; // render backend
		GLC_CmdPack *_cmdPack;
		GLC_CmdPack *_cmdPackFront;
		GLuint _fbo; // frame buffer object
		GLuint _outTex; // Color render buffer object of texture
		bool _matrixFlag; // change matrix flag

		friend class GLC_CmdPack;
	};

}
#endif
