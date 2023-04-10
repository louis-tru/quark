// @private head
/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef __quark_render_gl_glcanvas__
#define __quark_render_gl_glcanvas__

#include "../canvas.h"
#include "./glsl_shader.h"

namespace qk {

	class GLRender; // gl render backend
	class GLCanvas: public Canvas {
	public:
		GLCanvas(GLRender *backend);
		virtual ~GLCanvas();
		virtual int  save() override;
		virtual void restore(uint32_t count) override;
		virtual int  getSaveCount() const override;
		virtual const Mat& getMatrix() const override;
		virtual void setMatrix(const Mat& mat) override;
		virtual void translate(float x, float y) override;
		virtual void scale(float x, float y) override;
		virtual void rotate(float z) override;
		virtual bool readPixels(Pixel* dst, uint32_t srcX, uint32_t srcY) override;
		virtual void clipPath(const Path& path, ClipOp op, bool antiAlias) override;
		virtual void clearColor(const Color4f& color) override;
		virtual void drawColor(const Color4f& color, BlendMode mode) override;
		virtual void drawPath(const Path& path, const Paint& paint) override;
		virtual float drawGlyphs(const FontGlyphs &glyphs,
			Vec2 origin, const Array<Vec2> *offset, const Paint &paint) override;
		virtual void drawTextBlob(TextBlob *blob, Vec2 origin,
			float fontSize, const Paint &paint) override;
	protected:
		void drawColor(const Array<float>& vertex_vec2, const Paint& paint);
		void drawGradient(const Array<float>& vertex_vec2, const Paint& paint);
		void drawImage(const Array<float>& vertex_vec2, const Paint& paint);
		void drawImageMask(const Array<float>& vertex_vec2, const Paint& paint);
		float drawTextImage(ImageSource *textImg,
			float imgTop, float scale, Vec2 origin, const Paint &paint);

		void setBlendMode(BlendMode blendMode);
		void setMatrixBuffer(const Mat& mat);
		void setRootMatrixBuffer(Mat4& root);
		bool isStencilRefDefaultValue();
		// props
		struct Clip {
			Array<float> vertex; // vec2[]
			ClipOp op;
			bool antiAlias;
		};
		bool drawClip(Clip *clip);
		struct State {
			Mat         matrix;
			Array<Clip> clips;
		};
		GLRender *_backend;
		bool      _IsDeviceMsaa; // device anti alias, msaa
		GLuint    _stencil_ref, _stencil_ref_decr;
		BlendMode _blendMode;
		GLuint    _ubo, _texTmp[3]; // ubo => root,view matrix
		State    *_curState;
		Array<State> _state;
		// define buffers
		GLuint _frame_buffer,_msaa_frame_buffer;
		GLuint _render_buffer,_msaa_render_buffer,
					 _stencil_buffer,_depth_buffer;
		GLuint _aa_tex;
		Vec2   _surfaceScale;
		float  _surfaceScalef1
			, _transfromScale
			, _Scale; // surface scale * transfrom scale
	};

}
#endif
