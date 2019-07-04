/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "langou/gl/gl.h"
#include "langou/texture.h"
#include "langou/font/font-1.h"
#include "native-glsl.h"

#define gl_  glshaders(this)

#ifndef xx_use_depth_test
#define xx_use_depth_test 0
#endif

XX_NS(langou)

Array<GLShader*>* GLDraw::m_shaders = nullptr;

XX_DEFINE_INLINE_MEMBERS(GLDraw, Inl) {
public:
	
	/**
	 * @func initializ_shader 初始化着色器程序
	 */
	void initializ_shader() {
		XX_CHECK(m_shaders);
		
		for (auto& i : *m_shaders) {
			GLShader* shader = i.value();
			GLuint handle = 0;
			XX_CHECK(shader->shader == 0);
			
			if (m_library == DRAW_LIBRARY_GLES2) {
				handle = compile_link_shader(shader->name,
										WeakBuffer((cchar*)shader->es2_source_vp, (uint)shader->es2_source_vp_len),
										WeakBuffer((cchar*)shader->es2_source_fp, (uint)shader->es2_source_fp_len),
										String(shader->shader_attributes).split(','));
			} else if (m_library == DRAW_LIBRARY_GLES3) {
				handle = compile_link_shader(shader->name,
										WeakBuffer((cchar*)shader->source_vp, (uint)shader->source_vp_len),
										WeakBuffer((cchar*)shader->source_fp, (uint)shader->source_fp_len),
										String(shader->shader_attributes).split(','));
			} else { // opengl
				// TODO ...
			}
			XX_ASSERT(handle);
			shader->shader = handle;
			
			if ( *shader->shader_uniforms != '\0' ) {
				Array<String> uniforms = String(shader->shader_uniforms).split(',');
				for ( uint i = 0; i < uniforms.length(); i++ ) {
					int index = glGetUniformLocation( handle, *uniforms[i] );
					*(((int*)(shader + 1)) + i) = index;
				}
			}
		}
		
		// set uniform tex
		glUseProgram(shader::box_yuv420p_image.shader);
		glUniform1i(shader::box_yuv420p_image.s_tex_uv, 1);
		glUseProgram(shader::box_yuv420sp_image.shader);
		glUniform1i(shader::box_yuv420sp_image.s_tex_uv, 1);
	}
	
	void initializ_indexd_vbo() {
		
		ArrayBuffer<float> buffer(65536);
		for ( int i = 0; i < 65536; i++ ) {
			buffer[i] = i;
		}
		glGenBuffers(1, &m_indexd_vbo_data);
		glBindBuffer(GL_ARRAY_BUFFER, m_indexd_vbo_data);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 65536, *buffer, GL_STATIC_DRAW);
		
		/*
		 * 为了兼容 es3.0 语法的自动索引id
		 * 设置0属性槽,做为opengles2.0 的 gl_VertexID 自动化索引数据
		 * 所以调用这个方法后0,1两个属性槽被占用不能使用
		 */
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	/**
	 * @func es2_initializ_indexd_vbo_and_ext_support
	 */
	void es2_initializ_indexd_vbo_and_ext_support() {
		String info = (cchar*)glGetString(GL_EXTENSIONS);
		XX_DEBUG("OGL Info: %s", glGetString(GL_VENDOR));
		XX_DEBUG("OGL Info: %s", glGetString(GL_RENDERER));
		XX_DEBUG("OGL Info: %s", glGetString(GL_VERSION));
		XX_DEBUG("OGL Info: %s", *info);
		
		m_is_support_vao = info.index_of( "GL_OES_vertex_array_object" ) != -1;
		m_is_support_instanced = info.index_of( "GL_EXT_draw_instanced" ) != -1;
		// GL_EXT_disjoint_timer_query
		// GL_EXT_occlusion_query_boolean
		m_is_support_query = info.index_of( "occlusion" ) != -1;
		// GL_EXT_multisampled_render_to_texture
		// GL_IMG_multisampled_render_to_texture
		// GL_APPLE_framebuffer_multisample
		m_is_support_multisampled = info.index_of( "multisample" ) != -1;
		m_is_support_compressed_ETC1 = info.index_of( "GL_OES_compressed_ETC1_RGB8_texture" ) != -1;
		m_is_support_packed_depth_stencil = info.index_of( "packed_depth_stencil" ) != -1;
	}
	
};

/**
 * @constructor
 */
GLDraw::GLDraw(GUIApplication* host, cJSON& options): Draw(host, options)
, m_begin_screen_occlusion_query_status(false)
, m_SCREEN_RANGE_OCCLUSION_QUERY_HANDLE(0)
, m_current_frame_buffer(0)
, m_render_buffer(0)
, m_frame_buffer(0)
, m_msaa_render_buffer(0)
, m_msaa_frame_buffer(0)
, m_depth_buffer(0)
, m_stencil_buffer(0)
, m_stencil_ref_value(0)
, m_root_stencil_ref_value(0)
, m_indexd_vbo_data(0)
, m_is_support_vao(true)
, m_is_support_instanced(true)
, m_is_support_query(true)
, m_is_support_multisampled(true)
, m_is_support_compressed_ETC1(true)
{
}

GLDraw::~GLDraw() {
	if (m_shaders) {
		for (auto& i : (*m_shaders)) {
			if (i.value()->shader) {
				glDeleteProgram(i.value()->shader);
				i.value()->shader = 0;
			}
		}
	}
	
	if ( m_indexd_vbo_data ) {
		glDeleteBuffers(1, &m_indexd_vbo_data);
	}
	glDeleteRenderbuffers(1, &m_render_buffer);
	glDeleteFramebuffers(1, &m_msaa_render_buffer);
	glDeleteRenderbuffers(1, &m_depth_buffer);
	glDeleteRenderbuffers(1, &m_stencil_buffer);
	glDeleteFramebuffers(1, &m_frame_buffer);
	glDeleteRenderbuffers(1, &m_msaa_frame_buffer);
	
	if ( m_SCREEN_RANGE_OCCLUSION_QUERY_HANDLE ) {
		glDeleteQueries(1, &m_SCREEN_RANGE_OCCLUSION_QUERY_HANDLE);
	}
	
	Release(m_empty_texture); m_empty_texture = nullptr;
	Release(m_font_pool); m_font_pool = nullptr;
	Release(m_tex_pool); m_tex_pool = nullptr;
}

/**
 * 初始化上下文
 */
void GLDraw::initialize() {
	Inl_GLDraw(this)->initializ_shader();
	if (m_library == DRAW_LIBRARY_GLES2) {
		Inl_GLDraw(this)->es2_initializ_indexd_vbo_and_ext_support();
	}
	Inl_GLDraw(this)->initializ_indexd_vbo();
	initializ_gl_buffers();
	initializ_gl_status();
}

void GLDraw::initializ_gl_status() {
	
	glClearDepthf(0);
	glClearStencil(0);
	
	/*
	 * @开启颜色混合
	 *
	 * 如果设置了glBlendFunc(GL_ONE, GL_ZERO);
	 * 则表示完全使用源颜色，完全不使用目标颜色，因此画面效果和不使用混合的时候一致（当然效率可能会低一点点）。
	 * 如果没有设置源因子和目标因子，则默认情况就是这样的设置。
	 *
	 * 如果设置了glBlendFunc(GL_ZERO, GL_ONE);
	 * 则表示完全不使用源颜色，因此无论你想画什么，最后都不会被画上去了。
	 *（但这并不是说这样设置就没有用，有些时候可能有特殊用途）
	 *
	 * 如果设置了glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	 * 则表示源颜色乘以自身的alpha 值，目标颜色乘以1.0减去源颜色的alpha值，这样一来，源颜色的alpha值越大，
	 * 则产生的新颜色中源颜色所占比例就越大，而目标颜色所占比例则减 小。这种情况下，我们可以简单的将源颜色的alpha值
	 * 理解为“不透明度”。这也是混合时最常用的方式。
	 *
	 * 如果设置了glBlendFunc(GL_ONE, GL_ONE);
	 * 则表示完全使用源颜色和目标颜色，最终的颜色实际上就是两种颜色的简单相加。
	 * 例如红色(1, 0, 0)和绿色(0, 1, 0)相加得到(1, 1, 0)，结果为黄色。
	 */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// glEnable(GL_CULL_FACE); // 背面剔除
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	//glDepthFunc(GL_GEQUAL);
	glStencilFunc(GL_LEQUAL, 0, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

/**
 * @func initializ_gl_buffers
 */
void GLDraw::initializ_gl_buffers() {
	if ( ! m_frame_buffer ) {
		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(1, &m_frame_buffer);
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(1, &m_render_buffer);
		// Perform similar steps to create and attach a depth renderbuffer.
		if ( xx_use_depth_test ) {
			glGenRenderbuffers(1, &m_depth_buffer);
		}
		// stencil buffer
		glGenRenderbuffers(1, &m_stencil_buffer);
		// Create multisample buffers
		glGenFramebuffers(1, &m_msaa_frame_buffer);
		glGenRenderbuffers(1, &m_msaa_render_buffer);
		
		if ( is_support_query() ) { // 屏幕遮挡查询对像
			glGenQueries(1, &m_SCREEN_RANGE_OCCLUSION_QUERY_HANDLE);
		}
	}
}

/**
 * @func gl_main_render_buffer_storage
 */
void GLDraw::gl_main_render_buffer_storage() {
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, surface_size()[0], surface_size()[1]);
}

void GLDraw::refresh_buffer() {

	if ( surface_size() == Vec2() )
		return;

	int width = surface_size()[0];
	int height = surface_size()[1];
	
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer);
	gl_main_render_buffer_storage();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_render_buffer);
	
	if ( multisample() > 1 && is_support_multisampled() ) { // 启用多重采样
		glBindFramebuffer(GL_FRAMEBUFFER, m_msaa_frame_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_msaa_render_buffer); // render
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample(), GL_RGBA8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_msaa_render_buffer);

		if ( xx_use_depth_test ) {
			if ( is_support_packed_depth_stencil() ) {
				glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer); // depth
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample(), GL_DEPTH24_STENCIL8, width, height);
			} else {
				glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer); // depth
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample(), GL_DEPTH_COMPONENT16, width, height);
				glBindRenderbuffer(GL_RENDERBUFFER, m_stencil_buffer); // stencil
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample(), GL_STENCIL_INDEX8, width, height);
			}
		} else {
			glBindRenderbuffer(GL_RENDERBUFFER, m_stencil_buffer); // stencil
			glRenderbufferStorageMultisample(GL_RENDERBUFFER, multisample(), GL_STENCIL_INDEX8, width, height);
		}
	} else {
		if ( xx_use_depth_test ) {
			if ( is_support_packed_depth_stencil() ) {
				glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer); // depth
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			} else {
				glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer); // depth
				glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
				glBindRenderbuffer(GL_RENDERBUFFER, m_stencil_buffer); // stencil
				glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
			}
		} else {
			glBindRenderbuffer(GL_RENDERBUFFER, m_stencil_buffer); // stencil
			glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		}
	}

	if ( xx_use_depth_test ) {
		 glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_buffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
															is_support_packed_depth_stencil() ? m_depth_buffer : m_stencil_buffer);
	} else {
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_stencil_buffer);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, m_frame_buffer);
	
	// Test the framebuffer for completeness.
	if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
		XX_ERR("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER) );
	}
	
	// Retrieve the height and width of the color renderbuffer.
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
	XX_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);
}

void GLDraw::refresh_root_matrix(const Mat4& root, const Mat4& query) {
	
	if (!m_shaders) return;
	
	Mat4 root_(root); root_   .transpose();
	Mat4 query_(query); query_.transpose();
	
	const Array<GLShader*>& shaders = *m_shaders; // 更新2D视图变换矩阵
	
	for ( auto it : shaders ) {
		GLShader* shader = it.value();
		int handle = glGetUniformLocation(shader->shader, "root_matrix");
		if ( handle != -1 ) {
			glUseProgram(shader->shader);
			if ( shader->is_test ) {
				glUniformMatrix4fv( handle, 1, GL_FALSE, query_.value() );
			} else {
				glUniformMatrix4fv( handle, 1, GL_FALSE, root_.value() );
			}
		}
	}
}

void GLDraw::refresh_font_pool(FontPool* pool) {
	glUseProgram(shader::text_texture.shader);
	glUniform1f(shader::text_texture.display_port_scale, pool->m_display_port_scale);
}

void GLDraw::begin_render() {
	m_stencil_ref_value = 0;
	m_root_stencil_ref_value = 0;
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	if ( multisample() > 1 && is_support_multisampled() ) {
		glBindFramebuffer(GL_FRAMEBUFFER, m_msaa_frame_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_msaa_frame_buffer);
		m_current_frame_buffer = m_msaa_frame_buffer;
	} else {
		glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_frame_buffer);
		m_current_frame_buffer = m_frame_buffer;
	}
}

void GLDraw::commit_render() {
	if ( is_support_vao() ) {
		glBindVertexArray(0);
	}
	if ( multisample() && is_support_multisampled() ) {
		Vec2 ssize = surface_size();
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_msaa_frame_buffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_frame_buffer);
		glBlitFramebuffer(0, 0, ssize.width(), ssize.height(),
											0, 0, ssize.width(), ssize.height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_frame_buffer);
	}
}

void GLDraw::begin_screen_occlusion_query() {
	if ( ! m_begin_screen_occlusion_query_status ) {
		m_begin_screen_occlusion_query_status = true;
		glViewport(0, 0, surface_size()[0] / 10, surface_size()[1] / 10);
	}
}

void GLDraw::end_screen_occlusion_query() {
	if ( m_begin_screen_occlusion_query_status ) {
		m_begin_screen_occlusion_query_status = false;
		glViewport(0, 0, surface_size()[0], surface_size()[1]);
	}
}

/**
 * @func compile_shader       # 编译着色器程序
 * @arg code {cData&}         #     代码
 * @arg shader_type {GLenum}  #     程序类型
 * @ret {GLuint}
 */
GLuint GLDraw::compile_shader(cString& name, cBuffer& code, GLenum shader_type) {
	GLuint shader_handle = glCreateShader(shader_type);
	GLint code_len = code.length();
	cchar* c = code.value();
	glShaderSource(shader_handle, 1, &c, &code_len);
	glCompileShader(shader_handle);
	GLint ok;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &ok);
	if ( ok != GL_TRUE ) {
		char log[255] = { 0 };
		cchar* c_name = *name;
		glGetShaderInfoLog(shader_handle, 254, &ok, log);
		XX_FATAL("Compile shader error. name: %s, \n%s", c_name, log);
	}
	return shader_handle;
}

/**
 * @func compile_link_shader                #       编译着色器程序
 * @arg vertex {cData&}                     #       顶点程序代码
 * @arg fragment {cData&}                   #       片段程序代码
 * @arg attributes {const Array<String>&}   #       要编号的属性列表
 * @ret {GLuint}
 * @private
 */
GLuint GLDraw::compile_link_shader(cString& name,
																	 cBuffer& vertex, cBuffer& fragment,
																	 const Array<String>& attrs) {
	
	GLuint vertex_handle = compile_shader(name, vertex, GL_VERTEX_SHADER);
	GLuint fragment_handle = compile_shader(name, fragment, GL_FRAGMENT_SHADER);
	GLuint program_handle = glCreateProgram();
	glAttachShader(program_handle, vertex_handle);
	glAttachShader(program_handle, fragment_handle);
	
	int i = 0;

	for (auto it : attrs) {
		cString& attr = it.value();
		if ( !attr.is_empty() ) {
			glBindAttribLocation(program_handle, i, *attr);
		}
		i++;
	}
	
	glLinkProgram(program_handle);
	glDeleteShader(vertex_handle);
	glDeleteShader(fragment_handle);
	GLint ok;
	glGetProgramiv(program_handle, GL_LINK_STATUS, &ok);
	if (ok != GL_TRUE) {
		char log[255] = { 0 };
		cchar* c_name = *name;
		glGetProgramInfoLog(program_handle, 254, &ok, log);
		XX_FATAL("Link shader error, name: %s, \n%s", c_name, log);
	}
	return program_handle;
}

void GLDraw::del_buffer(uint id) {
	glDeleteBuffers(1, &id); // delete gl buffer data
}

void GLDraw::register_gl_shader(GLShader* shader) {
	static Array<GLShader*> shaders;
	if (!m_shaders) {
		m_shaders = &shaders;
	}
	shaders.push(shader);
}

XX_END
