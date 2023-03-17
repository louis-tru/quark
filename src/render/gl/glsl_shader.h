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

#ifndef __quark_render_gl_glsl_shader__
#define __quark_render_gl_glsl_shader__

#include "../../util/util.h"

#if Qk_iOS
# include <OpenGLES/ES3/gl.h>
# include <OpenGLES/ES3/glext.h>
#elif Qk_OSX
# include <OpenGL/gl.h>
# include <OpenGL/gl3.h>
# include <OpenGL/gl3ext.h>
#elif Qk_ANDROID || Qk_LINUX
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#else
# error "The operating system does not support"
#endif

#include "../paint.h"

namespace qk {

	class GLSLShader: public Object {
	public:
		Qk_DEFINE_PROP_GET(GLuint, shader);
		Qk_DEFINE_PROP_GET(GLuint, root_matrix);
		Qk_DEFINE_PROP_GET(GLuint, vertex_in); // glEnableVertexAttribArray
		virtual void build() = 0;
	protected:
		GLSLShader();
		void compile(
			cChar* name,
			cChar* vertexShader, cChar* fragmentShader,
			cChar* attributes, cChar* uniforms, GLuint *storeLocation
		);
	};

	class GLSLColor: public GLSLShader {
	public:
		Qk_DEFINE_PROP_GET(GLuint, color);
		virtual void build() override;
	};

	class GLSLImage: public GLSLShader {
	public:
		Qk_DEFINE_PROP_GET(GLuint, opacity);
		Qk_DEFINE_PROP_GET(GLuint, coord);
		Qk_DEFINE_PROP_GET(GLuint, image);
		virtual void build() override;
		friend class GLSLImageYUV420P;
		friend class GLSLImageYUV420SP;
	};

	class GLSLImageYUV420P: public GLSLImage {
	public:
		Qk_DEFINE_PROP_GET(GLuint, image_u);
		Qk_DEFINE_PROP_GET(GLuint, image_v);
		virtual void build() override;
	};

	class GLSLImageYUV420SP: public GLSLImage {
	public:
		Qk_DEFINE_PROP_GET(GLuint, image_uv);
		virtual void build() override;
	};

	class GLSLGradient: public GLSLShader {
	public:
		typedef Paint::GradientType GradientType;
		GLSLGradient(GradientType type): _type(type) {}
		Qk_DEFINE_PROP_GET(GLuint, range); // vertex uniform value
		Qk_DEFINE_PROP_GET(GLuint, count); // fragment uniform value
		Qk_DEFINE_PROP_GET(GLuint, colors);
		Qk_DEFINE_PROP_GET(GLuint, positions);
		Qk_DEFINE_PROP_GET(GradientType, type);
		virtual void build() override;
	};

}

#endif
