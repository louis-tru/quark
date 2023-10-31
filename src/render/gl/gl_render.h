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

// @private head

#ifndef __quark_render_gl_glrender__
#define __quark_render_gl_glrender__

#include "../render.h"
#include "./gl_canvas.h"

namespace qk {

	// Not thread safe, called in the rendering thread
	class GLRender: public RenderBackend {
	public:
		virtual      ~GLRender();
		virtual void reload() override;
		virtual void makeTexture(cPixel *pix, TexStat *&out, bool isMipmap) override;
		virtual void deleteTexture(TexStat *tex) override;
		virtual void makeVertexData(VertexData::ID *id) override;
		virtual void deleteVertexData(VertexData::ID *id) override;
		virtual void lock(); // lock render
		virtual void unlock(); // unlock render
		virtual Canvas* newCanvas(Options opts) override;
		virtual void release() override;
		// set gl state
		void gl_set_blend_mode(BlendMode mode);
		bool gl_set_texture(ImageSource *src, int slot, const ImagePaint *paint);
		void gl_set_texture_param(TexStat *tex, uint32_t slot, const ImagePaint* paint);

	protected:
		GLRender(Options opts);
		// define props
		GLuint _fbo; // temp fbo
		TexStat **_texStat; // temp tbo
		GLuint _rootMatrixBlock,_viewMatrixBlock; // ubo, matrixBlock => root view matrix
		GLuint _optsBlock; // ubo, generic optsBlock
		GLCanvas* _glcanvas; // main canvas
		GLSLShaders _shaders; // glsl shaders
		BlendMode _blendMode; // last setting status

		friend class GLCanvas;
		friend class GLC_CmdPack;
	};
}
#endif
