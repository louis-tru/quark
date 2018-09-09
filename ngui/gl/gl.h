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
 
#ifndef __ngui__gl__gl__
#define __ngui__gl__gl__

#include "../draw.h"

#if XX_IOS
# include <OpenGLES/ES3/gl.h>
# include <OpenGLES/ES3/glext.h>
#elif XX_ANDROID
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#elif XX_OSX
# include <OpenGL/OpenGL.h>
# include <OpenGL/gl3.h>
# include <OpenGL/gl3ext.h>
#elif XX_LINUX
# define GL_GLEXT_PROTOTYPES
# include <GLES3/gl3.h>
# include <GLES3/gl3ext.h>
#elif XX_WIN
#else
# error "The operating system does not support"
#endif

/**
 * @ns ngui
 */

XX_NS(ngui)

class  Font;
class  FontGlyph;

struct GLShader {
#pragma pack(push,4)
	const char* name;
	const unsigned char* source_vp;
	const  unsigned long source_vp_len;
	const unsigned char* source_fp;
	const  unsigned long source_fp_len;
	const unsigned char* es2_source_vp;
	const  unsigned long es2_source_vp_len;
	const unsigned char* es2_source_fp;
	const  unsigned long es2_source_fp_len;
	const char* shader_uniforms;
	const char* shader_uniform_blocks;
	const char* shader_attributes;
	unsigned int shader;
	const int is_test;
#pragma pack(pop)
};

/**
 * @class GLDraw
 */
class GLDraw: public Draw {
public:
	
	/**
	 * @constructor
	 */
	GLDraw(GUIApplication* host, const Map<String, int>& option);
	
	/**
	 * @destructor
	 */
	virtual ~GLDraw();
	
	/**
	 * 初始化上下文
	 */
	virtual void initialize();
	
	/**
	 * @func gl_main_render_buffer_storage()
	 */
	virtual void gl_main_render_buffer_storage();

	/**
	 * @overwrite
	 */
	virtual void refresh_buffer();
	virtual void refresh_root_matrix(const Mat4& root, const Mat4& query);
	virtual void refresh_font_pool(FontPool* pool);
	virtual void begin_render();
	virtual void commit_render();
	virtual void begin_screen_occlusion_query();
	virtual void end_screen_occlusion_query();
	virtual uint set_texture(const Array<PixelData>& data);
	virtual void del_texture(uint id);
	virtual void del_buffer(uint id);
	virtual bool set_yuv_texture(TextureYUV* yuv_tex, cPixelData& data);
	virtual uint gen_texture(uint origin_texture, uint width, uint height);
	virtual void use_texture(uint id, Repeat repeat, uint slot);
	virtual void use_texture(uint id, uint slot);
	virtual bool set_font_glyph_vertex_data(Font* font, FontGlyph* glyph);
	virtual bool set_font_glyph_texture_data(Font* font, FontGlyph* glyph, int level);
	virtual void clear_color(Color color);
	
	/**
	 * @overwrite
	 */
	virtual void draw(Root* v);
	virtual void draw(Video* v);
	virtual void draw(Image* v);
	virtual void draw(BoxShadow* v);
	virtual void draw(Box* v);
	virtual void draw(TextNode* v);
	virtual void draw(Label* v);
	virtual void draw(Text* v);
	virtual void draw(Sprite* v);
	virtual void draw(Scroll* v);
	virtual void draw(Input* v);
	virtual void draw(Textarea* v);
	
	/**
	 * @func get_gl_texture_pixel_format 获取当前环境对应的OpenGL纹理像素格式,如果返回0表示不支持纹理格式
	 */
	virtual GLint get_gl_texture_pixel_format(PixelData::Format pixel_format) = 0;
	
	/**
	 * @func compile_shader       # 编译着色器程序
	 * @arg code {cData&}         #     代码
	 * @arg shader_type {GLenum}  #     程序类型
	 * @ret {GLuint}
	 */
	GLuint compile_shader(cString& name, cBuffer& code, GLenum shader_type);
	
	/**
	 * @func compile_link_shader # 编译着色器程序
	 * @arg vertex {cBuffer&}                 #             顶点程序代码
	 * @arg fragment {cData&}                 #             片段程序代码
	 * @arg attrs {const Array<String>&}      #             要编号的属性列表
	 * @ret {GLuint}
	 */
	GLuint compile_link_shader(cString& name, cBuffer& vertex,
														 cBuffer& fragment,
														 const Array<String>& attrs = Array<String>());
	
	inline bool is_support_query() { return m_is_support_query; }
	inline bool is_support_vao() { return m_is_support_vao; }
	inline bool is_support_instanced() { return m_is_support_instanced; }
	inline bool is_support_multisampled() { return m_is_support_multisampled; }
	inline bool is_support_compressed_ETC1() { return m_is_support_compressed_ETC1; }
	inline bool is_support_packed_depth_stencil() { return m_is_support_packed_depth_stencil; }
	
	/**
	 * @func register_gl_shader()
	 */
	static void register_gl_shader(GLShader* shader);

 protected:

	/**
	 * @func initializ_gl_buffers
	 */
	virtual void initializ_gl_buffers();

	/**
	 * @func initializ_gl_status
	 */
	void initializ_gl_status();
	
	bool    m_begin_screen_occlusion_query_status; // 屏幕遮挡test状态
	GLuint  m_SCREEN_RANGE_OCCLUSION_QUERY_HANDLE; // 屏幕遮挡查询对像句柄
	GLuint  m_current_frame_buffer;
	GLuint  m_render_buffer;
	GLuint  m_frame_buffer;
	GLuint  m_msaa_render_buffer;
	GLuint  m_msaa_frame_buffer;
	GLuint  m_depth_buffer;
	GLuint  m_stencil_buffer;
	GLuint  m_stencil_ref_value;
	GLuint  m_root_stencil_ref_value;
	
	GLuint m_indexd_vbo_data;
	bool m_is_support_vao;
	bool m_is_support_instanced;
	bool m_is_support_query;
	bool m_is_support_multisampled;
	bool m_is_support_compressed_ETC1;
	bool m_is_support_packed_depth_stencil;
	
	static Array<GLShader*>* m_shaders;
	
	XX_DEFINE_INLINE_CLASS(Inl);
	XX_DEFINE_INLINE_CLASS(Inl2);
	
	friend class GLShader;
	friend class Texture;
	friend class IOSGLDrawCore;
	friend class LinuxGLDrawCore;
};

XX_END

#endif
