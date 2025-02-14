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

#include "./gl_shader.h"
#include "./glsl_shaders.h"
#include "../path.h"

namespace qk {

	extern String gl_MaxTextureImageUnits_GLSL_Macros;
	extern int    gl_MaxTextureImageUnits;

	static GLuint compile_shader(cChar* name, cString &code, GLenum shader_type) {
		GLuint shader_handle = glCreateShader(shader_type);
		GLint code_len = (GLint)code.length();
		const GLchar *code_p = code.c_str();
		glShaderSource(shader_handle, 1, &code_p, &code_len);
		glCompileShader(shader_handle);
		GLint ok;
		glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &ok);
		if ( ok != GL_TRUE ) {
			char log[255] = { 0 };
			glGetShaderInfoLog(shader_handle, 254, &ok, log);
			//Qk_ASSERT_EQ(GL_TRUE, ok);
			Qk_Fatal("Compile shader error, %s\n\n%s\n\n////////////\n\n%s\n", name, log, *code);
		}
		return shader_handle;
	}

	void gl_compile_link_shader(
		GLSLShader *s,
		cChar *name, cChar* macros,
		cString& vertexShader, cString& fragmentShader,
		const Array<GLShaderAttr> &attributes, cChar *uniforms)
	{
		String vertexCode("#version " Qk_GL_Version "\n#define Qk_SHADER_VERT\n");
		String fragmentCode("#version " Qk_GL_Version "\n#define Qk_SHADER_FRAG\n");

		vertexCode += gl_MaxTextureImageUnits_GLSL_Macros;
		fragmentCode += gl_MaxTextureImageUnits_GLSL_Macros;
		vertexCode += macros;
		fragmentCode += macros;
		vertexCode += vertexShader;
		fragmentCode += fragmentShader;

		GLuint vertex_handle = compile_shader(name, vertexCode, GL_VERTEX_SHADER);
		GLuint fragment_handle = compile_shader(name, fragmentCode, GL_FRAGMENT_SHADER);
		GLuint program = glCreateProgram();
		glAttachShader(program, vertex_handle);
		glAttachShader(program, fragment_handle);

		GLint status; // query status

		//Qk_DLog("sizeof(GLSLShader) %d,%d,%d", sizeof(GLSLShader), sizeof(GLSLColor), sizeof(GLSLImage));

		GLuint *storeLocation = &s->vbo + 1;

		// bind attrib Location
		GLuint attrIdx = 0;
		GLuint *store = storeLocation;
		for (auto &i: attributes) {
			glBindAttribLocation(program, *store++ = attrIdx++, i.name);
		}

		glLinkProgram(program);
		glDeleteShader(vertex_handle);
		glDeleteShader(fragment_handle);

		if ((glGetProgramiv(program, GL_LINK_STATUS, &status), status) != GL_TRUE) {
			char log[256] = { 0 };
			glGetProgramInfoLog(program, 255, &status, log);
			Qk_Fatal("Link shader error, %s\n\n%s", name, log);
		}

		// binding = 0 uniform block index as 0
		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "rootMatrixBlock"), 0);
		// binding = 0 uniform block index as 1
		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "viewMatrixBlock"), 1);
#if DEBUG
		// Get uniform block and bind index
		GLuint ubo = glGetUniformBlockIndex(program, "rootMatrixBlock");
		GLint bufferSize;
		glGetActiveUniformBlockiv(program, ubo, GL_UNIFORM_BLOCK_DATA_SIZE, &bufferSize);
		Qk_ASSERT(bufferSize == 64);
#endif

		// Get Uniform Location index value
		for (auto &i: String(uniforms).split(",")) {
			if (!i.isEmpty()) {
				*store++ = glGetUniformLocation(program, i.c_str());
			}
		}

		glUseProgram(program);

		glGenVertexArrays(1, &s->vao);
		glGenBuffers(1, &s->vbo);
		glBindVertexArray(s->vao);
		glBindBuffer(GL_ARRAY_BUFFER, s->vbo);

		GLsizei stride = 0, pointer = 0;

		for (auto &i: attributes) {
			stride += i.stride;
		}

		for (auto &i: attributes) {
			GLuint local = *storeLocation++;
			glVertexAttribPointer(local, i.size, i.type, GL_FALSE, stride, (const GLvoid*)pointer);
			glEnableVertexAttribArray(local);
			pointer += i.stride;
		}
		glUniform1i(glGetUniformLocation(program, "aaclip"), gl_MaxTextureImageUnits-1); // set aa texture slot
		glBindVertexArray(0);

		s->shader = program;
	}

	void GLSLShader::use(GLsizeiptr size, const GLvoid* data) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
		glBindVertexArray(vao);
		glUseProgram(shader);
	}

}
