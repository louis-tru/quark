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

#ifndef __quark_render_gl_glrender__
#define __quark_render_gl_glrender__

#include "../render.h"
#include "./gl_canvas.h"

namespace qk {
	/**
	 * Global render resource, not thread safe, called in the post message callback of thread
	 */
	class GLRenderResource: public RenderResource {
	public:
		void post_message(Cb cb) override;
		bool uploadTexture(cPixel *pix, int levels, TexStat *tex, bool mipmap) override;
		void unloadTexture(TexStat *tex) override;
		TexStat createTextureStat(Vec2 size, ColorType type, uint8_t flags) override;
	protected:
		explicit GLRenderResource(RunLoop *loop): _loop(loop) {}
	private:
		RunLoop *_loop;
	};

	// Not thread safe, called in the rendering thread
	class GLRender: public RenderBackend {
	public:
		~GLRender() override;
		void release() override;
		void reload() override;
		Canvas* createCanvas(Options opts) override;
		bool uploadTexture(cPixel *pix, int levels, TexStat *tex, bool mipmap) override;
		bool uploadVertexData(VertexData::ID *id) override;
		void unloadTexture(TexStat *tex) override;
		void unloadVertexData(VertexData::ID *id) override;
		virtual void lock(); // lock render thread
		virtual void unlock(); // unlock render
		// set gl state
		void set_blend_mode(BlendMode mode);
		void set_viewport(Vec2 size);
		bool use_texture(ImageSource *src, int srcSlot, int dstSlot, const PaintImage *paint); // temp tex
		void set_texture_param(GLuint tex, int dstSlot, const PaintImage* paint);
		GLuint get_tex_sampler(const PaintImage* paint);
		TexStat createTextureStat(Vec2 size, ColorType type, uint8_t flags) override;
	protected:
		explicit GLRender(Options opts);
		// define props
		GLuint _uboRMat,_ubovMat,_uboClip; // ubo: rootMatrixBlock,viewportBlock,clipBlock
		GLuint _ubo0,_ubo1; // temp ubo for draw call
		GLuint _ebo; // temp ebo, GL_ELEMENT_ARRAY_BUFFER
		GLCanvas* _glcanvas; // main canvas
		GLSLShaders _shaders; // glsl shaders
		BlendMode _blendMode; // last setting status
		Dict<uint32_t, GLuint> _texSamplers; // PaintImage => Sampler
		Vec2 _vportSize; // last viewport size
		friend class GLCanvas;
		friend class GLC_CmdPack;
	};

}
#endif
