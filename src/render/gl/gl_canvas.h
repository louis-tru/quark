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

#include "../render.h"
#include "../canvas.h"
#include "./glsl_shaders.h"

namespace qk {

	struct GLC_State { // gl canvas state
		struct Clip { // gl canvas clip
			Mat             matrix;
			VertexData      vertex,aafuzz;
			Canvas::ClipOp  op;
			bool            aaclip; // is aaclip
		};
		Mat         matrix;
		uint32_t    aaclip; // Is there a aa clip area and aaclip count
		Array<Clip> clips; // clip queue
		Sp<ImageSource> output; // output dest
	};

	class GLRender; // gl render backend
	class GLC_CmdPack;
	class GLCFilter;
	class GLCBlurFilter;

	class GLCanvas: public Canvas, public PathvCache::ClearSync {
	public:
		GLCanvas(GLRender *render, Render::Options opts);
		virtual ~GLCanvas();
		virtual int  save() override;
		virtual void restore(uint32_t count) override;
		virtual int  getSaveCount() const override;
		virtual const Mat& getMatrix() const override;
		virtual void setMatrix(const Mat& mat) override;
		virtual void setTranslate(Vec2 val) override;
		virtual void translate(Vec2 val) override;
		virtual void scale(Vec2 val) override;
		virtual void rotate(float z) override;
		virtual bool readPixels(uint32_t srcX, uint32_t srcY, Pixel* dst) override;
		virtual void clipPath(const Path& path, ClipOp op, bool antiAlias) override;
		virtual void clipPathv(const Pathv& path, ClipOp op, bool antiAlias) override;
		virtual void clipRect(const Rect& rect, ClipOp op, bool antiAlias) override;
		virtual void clearColor(const Color4f& color) override;
		virtual void drawColor(const Color4f& color, BlendMode mode) override;
		virtual void drawPath(const Path& path, const Paint& paint) override;
		virtual void drawPathv(const Pathv& path, const Paint& paint) override;
		virtual void drawPathvColor(const Pathv &path, const Color4f &color, BlendMode mode, bool antiAlias) override;
		virtual void drawPathvColors(const Pathv* path[], int count, const Color4f &color, BlendMode mode, bool antiAlias) override;
		virtual void drawRRectBlurColor(const Rect& rect,
			const float radius[4], float blur, const Color4f &color, BlendMode mode) override;
		virtual void drawRect(const Rect& rect, const Paint& paint) override;
		virtual void drawRRect(const Rect& rect, const Path::BorderRadius &radius, const Paint& paint) override;
		virtual float drawGlyphs(const FontGlyphs &glyphs, Vec2 origin, const Array<Vec2> *offset, const Paint &paint) override;
		virtual void drawTextBlob(TextBlob *blob, Vec2 origin, float fontSize, const Paint &paint) override;
		virtual void drawTriangles(const Triangles& triangles, const Paint &paint) override;
		virtual Sp<ImageSource> readImage(const Rect &src, Vec2 dest, ColorType type, BlendMode mode, bool isMipmap) override;
		virtual Sp<ImageSource> outputImage(ImageSource* dest, bool isMipmap) override;
		virtual void swapBuffer() override; // swap gl double cmd pkg
		void         flushBuffer(); // commit gl cmd, only can rendering thread call
		virtual PathvCache* getPathvCache() override;
		virtual void setSurface(const Mat4& root, Vec2 surfaceSize, Vec2 scale) override;
		virtual Vec2 size() override;
		inline uint8_t isDeviceMsaa() { return _DeviceMsaa; }
		inline GLuint fbo() { return _fbo; }
		virtual bool isGpu() override;
		inline Vec2 surfaceSize() { return _surfaceSize; }
		virtual void lock() override; // clear sync lock
		virtual void unlock() override; // clear sync unlock
		void vportFullCopy(GLuint dstFBO);
	private:
		virtual void setBuffers(Vec2 size);
		// define props
		Array<GLC_State> _stateStack; // state
		GLC_State    *_state; // state
		GLC_CmdPack  *_cmdPack;
		GLC_CmdPack  *_cmdPackFront;
		GLRender     *_render;
		PathvCache   *_cache;
		GLuint _fbo; // frame buffer object
		TexStat *_outTex; // Color render buffer object of texture
		GLuint _outDepth; // Depth and stencil buffer object
		GLuint _outAAClipTex; // Output texture AA clip buffer object
		GLuint _outA, _outB; // Output texture buffer object A and B
		GLuint _stencilRef, _stencilRefDrop; // stencil clip state
		float  _zDepth;
		float  _surfaceScale, _scale;
		float  _allScale, _phy2Pixel; // surface scale * transfrom scale, _phy2Pixel = 2 / _scale
		Vec2   _size, _surfaceSize; // canvas size and surface size
		Mat4   _rootMatrix;
		BlendMode _blendMode; // blend mode state
		uint8_t  _DeviceMsaa; // device anti alias, msaa
		bool   _isClipState; // clip state
		bool   _matrixFlag; // set matrix flag
		Render::Options _opts;
		CondMutex _mutex; // submit swap mutex

		friend class GLCFilter;
		friend class GLC_CmdPack;
		friend class GLCBlurFilter;
		friend class LinuxGLRender;
		Qk_DEFINE_INLINE_CLASS(Inl);
	};

}
#endif
