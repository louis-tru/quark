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

#include "./glsl_shader.h"

namespace quark {

	GLuint compile_shader(cChar* name, const GLchar* code, GLenum shader_type) {
		GLuint shader_handle = glCreateShader(shader_type);
		GLint code_len = strlen(code);
		glShaderSource(shader_handle, 1, &code, &code_len);
		glCompileShader(shader_handle);
		GLint ok;
		glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &ok);
		if ( ok != GL_TRUE ) {
			char log[255] = { 0 };
			glGetShaderInfoLog(shader_handle, 254, &ok, log);
			Qk_FATAL("Compile shader error, %s\n\n%s", name, log);
		}
		return shader_handle;
	}

	static const String vertexHeader("\n\
	#version 300 es\n\
	uniform mat4  root_matrix;\n\
	uniform float view_matrix[6];\n\
	#define matrix root_matrix * v_matrix()\n\
	mat4 v_matrix() {\n\
		return mat4(view_matrix[0], view_matrix[3], 0.0, 0.0,\n\
								view_matrix[1], view_matrix[4], 0.0, 0.0,\n\
								0.0,            0.0,            1.0, 0.0,\n\
								view_matrix[2], view_matrix[5], 0.0, 1.0);\n\
	}");

	static const String fragmentHeader("\n\
	#version 300 es\n\
	out lowp vec4 color_o;\n\
	");

	GLSLShader::GLSLShader(): _shader(0), _root_matrix(0), _view_matrix(0) {
	}

	void GLSLShader::compile(
		cChar* name, cChar* vertexShader, cChar* fragmentShader,
		cChar* _attrs, cChar* _uniforms)
	{
		Qk_ASSERT(!_shader, "#GLSLShader::compile");
		GLuint vertex_handle =
			compile_shader(name, (vertexHeader + vertexShader).c_str(), GL_VERTEX_SHADER);
		GLuint fragment_handle =
			compile_shader(name, (fragmentHeader + fragmentShader).c_str(), GL_FRAGMENT_SHADER);
		GLuint program_handle = glCreateProgram();
		glAttachShader(program_handle, vertex_handle);
		glAttachShader(program_handle, fragment_handle);

		auto attrs = String(_attrs).split(',');
		auto uniforms = String(_uniforms).split(',');
		
		GLuint *store = static_cast<GLuint*>(((void*)(this)) + sizeof(GLSLShader));
		GLuint index = 0;
		for (auto& i: attrs) {
			glBindAttribLocation(program_handle, *store = index++, i.c_str());
			store++;
		}

		glLinkProgram(program_handle);
		glDeleteShader(vertex_handle);
		glDeleteShader(fragment_handle);

		GLint ok;
		glGetProgramiv(program_handle, GL_LINK_STATUS, &ok);

		if (ok != GL_TRUE) {
			char log[256] = { 0 };
			glGetProgramInfoLog(program_handle, 255, &ok, log);
			Qk_FATAL("Link shader error, %s\n\n%s", name, log);
		}

		_root_matrix = glGetUniformLocation(program_handle, "root_matrix");
		_view_matrix = glGetUniformLocation(program_handle, "view_matrix");

		for (auto &i: uniforms) {
			*store = glGetUniformLocation(program_handle, i.c_str());
			store++;
		}

		_shader = program_handle;
	}

	GLSLColor::GLSLColor() {
		compile("color shader",
		"\n\
		in      vec3  vertex_in;\n\
		uniform vec4  color;\n\
		void main() {\n\
			color_f = color * vec4(1.0, 1.0, 1.0, vertex_in.z);\n\
			gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);\n\
		}\n\
		out     vec4 color_f;\n\
		",
		"\n\
		in lowp vec4 color_f;\n\
		void main() {\n\
			color_o = color_f;\n\
		}\n\
		",
		"vertex_in", "color");
	}

	GLSLImage::GLSLImage() {
		compile("image shader",
		"\n\
		in        vec3      vertex_in;\n\
		uniform   float     opacity;\n\
		uniform   vec4      coord;//offset,scale\n\
		\n\
		void main() {\n\
			opacity_f = opacity * vertex_in.z;\n\
			image_uv_f = (vertex_in.xy - coord.xy) * coord.zw;\n\
			gl_Position = matrix * vec4(vertex_in.xy, 0.0, 1.0);\n\
		}\n\
		out       float     opacity_f;\n\
		out       vec2      image_uv_f;\n\
		",
		"\n\
		in lowp   float     opacity_f;\n\
		in lowp   vec2      image_uv_f;\n\
		uniform   sampler2D image;\n\
		void main() {\n\
			color_o = texture(image, image_uv_f) * vec4(1.0, 1.0, 1.0, opacity_f);\n\
		}\n\
		",
		"vertex_in", "opacity,coord,image");
	}

	GLSLGradient::GLSLGradient(GradientPaint::GradientType type) {

		const char *v_shader = type == GradientPaint::kLinear ?
		// kLinear
		"\n\
		in        vec3      vertex_in;\n\
		uniform   vec4      range;//start/end range for rect\n\
		\n\
		void main() {\n\
			vec2 ao = range.zw     - range.xy;\n\
			vec2 bo = vertex_in.xy - range.xy;\n\
			position_f = clamp(dot(ao,bo) / dot2(ao), 0.0, 1.0);\n\
			opacity_f  = vertex_in.z;\n\
		}\n\
		out       float     opacity_f;\n\
		out       float     position_f;\n\
		":
		// kRadial
		"\n\
		in        vec3      vertex_in;\n\
		uniform   vec4      range;//center/radius for circle\n\
		\n\
		void main() {\n\
			opacity_f  = vertex_in.z;\n\
			position_f = min(1.0, length((vertex_in.xy-range.xy)/range.zw));\n\
		}\n\
		out       float     opacity_f;\n\
		out       float     position_f;\n\
		";

		compile("gradient shader", v_shader,
		"\n\
		in lowp   float     opacity_f;\n\
		in lowp   float     position_f;\n\
		uniform   float     count;\n\
		uniform   vec4      colors[256];//max 256 color points\n\
		uniform   vec4      positions[256];\n\
		void main() {\n\
			int s = 0;\n\
			int e = count-1;\n\
			while (s+1 < e) {// dichotomy search color value\n\
				int idx = (e - s) / 2 + s;\n\
				if (position_f > positions[idx]) {\n\
					s = idx;\n\
				} else if (position_f < positions[idx]) {\n\
					e = idx;\n\
				} esle { \n\
					s = idx; e = idx+1; break; \n\
				}\n\
			}\n\
			float w = (position_f - positions[s]) / (positions[e] - positions[s]);\n\
			color_o = mix(colors[s], colors[e], w) * vec4(1.0, 1.0, 1.0, opacity_f);\n\
		\
		}\n\
		",
		"vertex_in", "range,count,colors,positions");
	}

}
