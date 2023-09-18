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

	class GLRender: public RenderBackend {
	public:
		virtual          ~GLRender();
		virtual void     reload() override;
		virtual uint32_t makeTexture(cPixel *src, uint32_t id) override;
		virtual void     deleteTextures(const uint32_t *ids, uint32_t count) override;
		virtual void     makeVertexData(VertexData::ID *id) override;
		virtual void     deleteVertexData(VertexData::ID *id) override;
		void             setTexture(cPixel *pixel, int slot, const ImagePaint *paint);
		void             setBlendMode(BlendMode mode);
	protected:
		GLRender(Options opts);
		friend class GLCanvas;
		friend class GLC_CmdPack;
		// --------------- define props ---------------
		bool _IsSupportMultisampled;
		BlendMode _blendMode;
		GLuint _texBuffer[3]; // temp texture buffers
		GLuint _rootMatrixBlock,_viewMatrixBlock; // matrixBlock => root view matrix
		GLuint _optsBlock; // generic optsBlock
		GLint  _maxTextureSize;
		GLint  _maxTextureBufferSize;
		GLint  _maxTextureImageUnits;
		GLCanvas _glCanvas; // main canvas
		GLSLShaders _shaders;
		// --------------------------------------------
	};
}
#endif
