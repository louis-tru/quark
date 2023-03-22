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

#ifndef __quark_render_gl_glrender__
#define __quark_render_gl_glrender__

#include "../render.h"
#include "./gl_canvas.h"

namespace qk {

	class GLRender: public GLCanvas, public Render {
	public:
		virtual ~GLRender();
		virtual Object* asObject() override;
		virtual void reload(int w, int h, Mat4& root) override;
		virtual GLuint setTexture(cPixel *src, GLuint id) override;
		virtual void deleteTextures(const GLuint *IDs, GLuint count) override;
	protected:
		GLRender(Application* host);
		virtual void setRenderBuffer(int width, int height);
		virtual void setStencilBuffer(int width, int height, int MSAASample);
		virtual void setMSAABuffer(int width, int height, int MSAASample);
		virtual void setAntiAlias(int width, int height);
		virtual void setDepthBuffer(int width, int height);
		void setRootMatrix(Mat4& root);
		GLuint _frame_buffer,_msaa_frame_buffer;
		GLuint _render_buffer,_msaa_render_buffer,_stencil_buffer,_depth_buffer;
		GLuint _aa_tex;
		bool _is_support_multisampled, _raster;
	};
}
#endif
