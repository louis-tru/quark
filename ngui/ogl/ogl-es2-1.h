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

#ifndef __ngui__ogl_es2__ogl__
#define __ngui__ogl_es2__ogl__

#include "ogl.h"

XX_NS(ngui)

/**
 * @class GLES2Draw
 */
class GLES2Draw: public GLDraw {
public:
  GLES2Draw(GUIApplication* host, const Map<String, int>& option);
  virtual ~GLES2Draw();
  virtual void initialize();
  virtual void refresh_status_for_font_pool(FontPool* pool);
  virtual bool is_support_query() { return m_is_support_query; }
  virtual bool is_support_vao() { return m_is_support_vao; }
  virtual bool is_support_instanced() { return m_is_support_instanced; }
  virtual bool is_support_multisampled() { return m_is_support_multisampled; }
  virtual bool is_support_compressed_ETC1() { return m_is_support_compressed_ETC1; }
  virtual bool is_support_packed_depth_stencil() { return m_is_support_packed_depth_stencil; }
  
private:
  
  XX_DEFINE_INLINE_CLASS(Inl);
  
  GLuint m_gl_index_data_vbo;
  bool m_is_support_vao;
  bool m_is_support_instanced;
  bool m_is_support_query;
  bool m_is_support_multisampled;
  bool m_is_support_compressed_ETC1;
  bool m_is_support_packed_depth_stencil;
};

XX_END
#endif
