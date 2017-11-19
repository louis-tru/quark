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

#include "ogl-es2-1.h"
#include "glsl-shader.h"
#include "glsl-es2-shader.h"
#include "../font/font-1.h"

XX_NS(ngui)

#define gl_ glshaders(this)

XX_DEFINE_INLINE_MEMBERS(GLES2Draw, Inl) {
public:
  /**
   * @func initializ_shader
   */
  void initializ_shader() {
    // ----------- 初始化着色器程序 -----------

    // el、es2的着色器程序，变量与统一值必须完全相同
    static_assert(sizeof(ES2GLShaders) == sizeof(GLShaders), "Err");
    
    m_shaders = new ES2GLShaders();
    int size = sizeof(shader::ES2natives) / sizeof(shader::NativeGLSL);
    
    for ( int i = 0; i < size; i++ ) {
      const shader::NativeGLSL& native = shader::ES2natives[i];
      
      GLuint handle = compile_link_shader(native.name,
                                          WeakBuffer((cchar*)native.source_vp, (uint)native.source_vp_len),
                                          WeakBuffer((cchar*)native.source_fp, (uint)native.source_fp_len),
                                          String(native.shader_attributes).split(',')
                                          );
      GLShader* shader = (GLShader*)( ((int*)m_shaders) + native.address );
      shader->m_is_query = (String(native.name).index_of("query") != -1);
      shader->handle(handle);
      m_shaders_vector.push(*shader);
      
      int j = 0;
      
      if ( *native.shader_uniforms != '\0' ) {
        Array<String> uniforms = String(native.shader_uniforms).split(',');
        for ( int i = 0; i < uniforms.length(); i++, j++ ) {
          int index = glGetUniformLocation( handle, *uniforms[i] );
          *(((uint*)m_shaders) + native.address + j + 2) = index;
        }
      }
    }
    
    // set uniform tex
    gl_->box_yuv420p_image.use();
    glUniform1i(gl_->box_yuv420p_image_uniform_s_tex_y, 5);
    glUniform1i(gl_->box_yuv420p_image_uniform_s_tex_uv, 6);
    //
    gl_->box_yuv420sp_image.use();
    glUniform1i(gl_->box_yuv420sp_image_uniform_s_tex_y, 5);
    glUniform1i(gl_->box_yuv420sp_image_uniform_s_tex_uv, 6);
    //
    gl_->text_texture.use();
    glUniform1i(gl_->text_texture_uniform_sampler_tex_1, 7);
    //
    // --------------------------
  }
  
  /**
   * @func initializ_indexd_vbo_and_ext_support
   */
  void initializ_indexd_vbo_and_ext_support() {
    { //
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
    
    { //
      ArrayBuffer<float> buffer(65536);
      for ( int i = 0; i < 65536; i++ ) {
        buffer[i] = i;
      }
      glGenBuffers(1, &m_gl_index_data_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, m_gl_index_data_vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 65536, *buffer, GL_STATIC_DRAW);
      
      /*
       * 为了兼容 es3.0 语法的自动索引id
       * 设置0,1属性槽,做为opengles2.0 的 gl_InstanceID, gl_VertexID 自动化索引数据
       * 所以调用这个方法后0,1两个属性槽被占用不能使用
       */
      glEnableVertexAttribArray(0);
      glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
      
      if ( m_is_support_instanced ) {
        glEnableVertexAttribArray(1);
        glVertexAttribDivisor(1, 1);   // 1 Instanced
        glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
      }
      
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }
  
};

GLES2Draw::GLES2Draw(GUIApplication* host, const Map<String, int>& option): GLDraw(host, option)
, m_gl_index_data_vbo(0)
, m_is_support_vao(false)
, m_is_support_instanced(false)
, m_is_support_query(false)
, m_is_support_multisampled(false)
, m_is_support_compressed_ETC1(false) {
  
}

GLES2Draw::~GLES2Draw() {
  if ( m_gl_index_data_vbo ) {
    glDeleteBuffers(1, &m_gl_index_data_vbo);
  }
  if ( m_shaders ) {
    delete (ES2GLShaders*)m_shaders;
    m_shaders = nullptr;
  }
}

void GLES2Draw::initialize() {
  
  Inl_GLES2Draw(this)->initializ_shader();
  Inl_GLES2Draw(this)->initializ_indexd_vbo_and_ext_support();
  
  initializ_ogl_buffers();
  initializ_ogl_status();
}

void GLES2Draw::refresh_status_for_font_pool(FontPool* pool) {
  gl_->text_texture.use();
  glUniform1f(gl_->text_texture_uniform_display_port_scale,
              pool->m_display_port_scale);
}

XX_END
