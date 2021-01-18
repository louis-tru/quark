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

#include "gl.h"
#include "ftr/texture.h"
#include "ftr/font/font-1.h"
#include "ftr/box.h"
#include "ftr/sprite.h"
#include "ftr/box-shadow-1.h"
#include "ftr/image.h"
#include "ftr/video.h"
#include "ftr/text-node.h"
#include "ftr/label.h"
#include "ftr/hybrid.h"
#include "ftr/text.h"
#include "ftr/root.h"
#include "ftr/app.h"
#include "ftr/display-port.h"
#include "ftr/scroll.h"
#include "ftr/input.h"
#include "ftr/textarea.h"
#include "ftr/background.h"
#include "native-glsl.h"

#define SIZEOF(T) sizeof(T) / sizeof(float)
#define nx_ctx_data(view, T)       static_cast<CtxDataWrap<T>*>(view->_ctx_data)->value()
#define nx_ctx_data_float_p(view)  static_cast<CtxDataWrap<CtxData>*>(view->_ctx_data)->float_p()

namespace ftr {

template<typename T>
class CtxDataWrap: public DrawData {
 public:
	inline CtxDataWrap() { memset(&_value, 0, sizeof(T)); }
	inline float* float_p() { return _value.value(); }
	inline T* value() { return &_value; }
 private:
	T _value;
};

struct CtxData {
	inline float* value() { return (float*)this; }
};

/**
 * @class GLDraw::Inl2
 */
class GLDraw::Inl2: public GLDraw {
public:
#define _inl(self) static_cast<GLDraw::Inl2*>(self)
	
	template<class T>
	inline void new_ctx_data(View* v) {
		if ( !v->_ctx_data ) {
			v->_ctx_data = new CtxDataWrap<T>();
		}
	}
	
	/**
	 * @func draw_scroll_bar
	 */
	void draw_scroll_bar(Box* v1, BasicScroll* v) {
		
		if ( (v->_h_scrollbar || v->_v_scrollbar) && v->_scrollbar_color.a() ) {
			
			float scrollbar_width = v->scrollbar_width();
			float scrollbar_margin = v->scrollbar_margin();
			
			struct view_matrix {
				Mat view_matrix;
				float opacity;
			} vm = { v1->_final_matrix, v1->_final_opacity * v->_scrollbar_opacity };
			auto color = v->_scrollbar_color.to_float_color();
			float r = scrollbar_width / 2.0f;
			
			glUseProgram(shader::box_color.shader);
			glUniform1fv(shader::box_color.view_matrix, 7, vm.view_matrix.value());
			glUniform4fv(shader::box_color.background_color, 1, color.value());
			glUniform4f(shader::box_color.border_width, 0, 0, 0, 0);
			glUniform4f(shader::box_color.radius_size, r, r, r, r);
			glUniform1f(shader::box_color.sample_x2, 6); // sample 3*2
			glUniform1i(shader::box_color.is_radius, 1);
			
			if ( v->_h_scrollbar ) { // 绘制水平滚动条
				Vec2 a(v->_h_scrollbar_position[0] - v1->_origin.x(),
							 v1->_final_height - v1->_origin.y() - scrollbar_width - scrollbar_margin);
				Vec2 c(a.x() + v->_h_scrollbar_position[1],
							 a.y() + scrollbar_width);
				glUniform4f(shader::box_color.vertex_ac, a[0], a[1], c[0], c[1]);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 16);
			}
			
			if ( v->_v_scrollbar ) { // 绘制垂直滚动条
				
				Vec2 a(v1->_final_width - v1->_origin.x() - scrollbar_width - scrollbar_margin,
							 v->_v_scrollbar_position[0] - v1->_origin.y());
				Vec2 c(a.x() + scrollbar_width,
							 a.y() + v->_v_scrollbar_position[1]);
				glUniform4f(shader::box_color.vertex_ac, a[0], a[1], c[0], c[1]);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 16);
			}
		}
	}
	
	template<typename T>
	void set_box_uniform_value(T& shader, Box* v) {
		glUniform1fv(shader.view_matrix, 7, v->_final_matrix.value());
		glUniform4f(shader.vertex_ac,
								-v->_origin.x(), -v->_origin.y(),
								v->_final_width - v->_origin.x(), v->_final_height - v->_origin.y());
		glUniform4fv(shader.border_width, 1, &v->_border_top_width);
		glUniform4fv(shader.radius_size, 1, &v->_final_border_radius_left_top);
	}
	
	/**
	 * @func draw_border_radius 绘制圆角边框
	 */
	void draw_border_radius(Box* v) {
		
		glUseProgram(shader::box_border_radius.shader);

		set_box_uniform_value(shader::box_border_radius, v);
		
		if ( v->_border_top_width != 0) { // top
			FloatColor color = v->_border_top_color.to_float_color();
			glUniform1i(shader::box_border_radius.direction, 0);
			glUniform4fv(shader::box_border_radius.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);
		}
		if ( v->_border_right_width != 0) { // right
			FloatColor color = v->_border_right_color.to_float_color();
			glUniform1i(shader::box_border_radius.direction, 1);
			glUniform4fv(shader::box_border_radius.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);
		}
		if ( v->_border_bottom_width != 0) { // bottom
			FloatColor color = v->_border_bottom_color.to_float_color();
			glUniform1i(shader::box_border_radius.direction, 2);
			glUniform4fv(shader::box_border_radius.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);
		}
		if ( v->_border_left_width != 0) { // left
			FloatColor color = v->_border_left_color.to_float_color();
			glUniform1i(shader::box_border_radius.direction, 3);
			glUniform4fv(shader::box_border_radius.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);
		}
	}
	
	/**
	 * @func draw_border 绘制普通边框
	 */
	void draw_border(Box* v) {
		glUseProgram(shader::box_border.shader);
		
		set_box_uniform_value(shader::box_border, v);
		
		if ( v->_border_top_width != 0) { // top
			// LOG("_border_top_width, %f", v->_border_top_width);
			FloatColor color = v->_border_top_color.to_float_color();
			glUniform1i(shader::box_border.direction, 0);
			glUniform4fv(shader::box_border.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		if ( v->_border_right_width != 0) { // right
			// LOG("_border_right_width, %f", v->_border_right_width);
			FloatColor color = v->_border_right_color.to_float_color();
			glUniform1i(shader::box_border.direction, 1);
			glUniform4fv(shader::box_border.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		if ( v->_border_bottom_width != 0) { // bottom
			// LOG("_border_bottom_width, %f", v->_border_bottom_width);
			FloatColor color = v->_border_bottom_color.to_float_color();
			glUniform1i(shader::box_border.direction, 2);
			glUniform4fv(shader::box_border.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		if ( v->_border_left_width != 0) { // left
			// LOG("_border_left_width, %f", v->_border_left_width);
			FloatColor color = v->_border_left_color.to_float_color();
			glUniform1i(shader::box_border.direction, 3);
			glUniform4fv(shader::box_border.border_color, 1, color.value());
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
	}
	
	void draw_box_border(Box* v) {
		if ( v->_is_draw_border ) { // 绘制边框
			if ( v->_is_draw_border_radius ) {
				// TODO resolved
				// 在iOS系统中使用OpenGLES2.0方式运行Examples程序时，
				// 从主界面切换到`Examplex source`时会出现绘图命令提交异常导致程序奔溃
				// 初步判定是在绘制圆角边框时参数错误导致,所以在iOS上暂时只运行OpenGLES3.0模式
				draw_border_radius(v);
			} else {
				draw_border(v);
			}
		}
	}
	
	void draw_box_background_color(Box* v) {
		auto color = v->_background_color.to_float_color();
		glUseProgram(shader::box_color.shader);
		set_box_uniform_value(shader::box_color, v);
		glUniform4fv(shader::box_color.background_color, 1, color.value());
		if ( v->_is_draw_border_radius ) { // 圆角
			glUniform1f(shader::box_color.sample_x2, 30); // sample 15*2
			glUniform1f(shader::box_color.is_radius, 1);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 64);
		} else {
			glUniform1f(shader::box_color.is_radius, 0);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
	}
	
	void draw_box_background_image(Box* v, BackgroundImage* image, bool clip) {
		Vec2 size, position;
		int level;
		if (image->get_background_image_data(v, size, position, level)) {
			if ( image->_texture->use(0, Texture::Level(level), image->_repeat) ) {
				Vec4 tex_coord(-position.x() / size.x(), -position.y() / size.y(),  // start
											 v->_final_width / size.x(), v->_final_height / size.y());  // scale
				glUseProgram(shader::box_background_image.shader); // 使用矩形纹理着色器
				set_box_uniform_value(shader::box_background_image, v);
				glUniform4fv(shader::box_background_image.tex_coord, 1, tex_coord.value());
				
				switch (image->_repeat) {
					case Repeat::NONE:
						glUniform2i(shader::box_background_image.repeat, 0, 0);
						break;
					case Repeat::REPEAT:
					case Repeat::MIRRORED_REPEAT:
						glUniform2i(shader::box_background_image.repeat, 1, 1);
						break;
					case Repeat::REPEAT_X:
					case Repeat::MIRRORED_REPEAT_X:
						glUniform2i(shader::box_background_image.repeat, 1, 0);
						break;
					case Repeat::REPEAT_Y:
					case Repeat::MIRRORED_REPEAT_Y:
						glUniform2i(shader::box_background_image.repeat, 0, 1);
						break;
				}
				if (v->_is_draw_border_radius && !clip) { // 绘制圆角
					glUniform1f(shader::box_background_image.sample_x2, 30); // sample 15*2
					glUniform1i(shader::box_background_image.is_radius, 1);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 64);
				} else {
					glUniform1i(shader::box_background_image.is_radius, 0);
					glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				}
			}
		}
	}
	
	void draw_box_background_gradient(Box* v, BackgroundGradient* gradient, bool clip) {
		// TODO ...
		// draw background
	}
	
	void draw_box_background(Box* v, bool clip) {
		auto bg = v->_background;
		while (bg) {
			switch (bg->type()) {
				case Background::M_IMAGE:
					draw_box_background_image(v, static_cast<BackgroundImage*>(bg), clip);
					break;
				case Background::M_GRADIENT:
					draw_box_background_gradient(v, static_cast<BackgroundGradient*>(bg), clip);
					break;
				default:
					break;
			}
			bg = bg->next();
		}
	}
	
	void draw_begin_clip(Box* v) {
		// build stencil test value and background color draw
		
		if ( _stencil_ref_value == _root_stencil_ref_value ) {
			glEnable(GL_STENCIL_TEST); // 启用模板测试
			glStencilFunc(GL_ALWAYS, _stencil_ref_value + 1, 0xFF); // 总是通过模板测试
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE); // Test成功将参考值设置为模板值
		} else {
			glStencilOp(GL_KEEP, GL_INCR, GL_INCR); // Test成功增加模板值
		}
		
		draw_box_background_color(v);
		
		glStencilFunc(GL_LEQUAL, ++_stencil_ref_value, 0xFF); // 设置新参考值
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		
		// 限制子视图区域绘图
		display_port()->push_draw_region(v->get_screen_region());
	}
	
	void draw_end_clip(Box* v) {
		display_port()->pop_draw_region(); // 弹出区域绘图
		
		if ( _root_stencil_ref_value == _stencil_ref_value - 1 ) {
			_root_stencil_ref_value = _stencil_ref_value;
			glDisable(GL_STENCIL_TEST); // 禁止模板测试
		} else {
			// 恢复原模板值
			glStencilFunc(GL_LEQUAL, --_stencil_ref_value, 0xFF); // 新参考值
			glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE); // Test成功设置模板为参考值
			glBlendFunc(GL_ZERO, GL_ONE); // 禁止颜色输出
			
			draw_box_background_color(v);
			
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 打开颜色输出
		}
	}
	
	/**
	 * @func draw_text_background
	 */
	void draw_text_background(View* v, TextFont::Data& data,
														Color color, uint begin, uint end, Vec2 offset) 
	{
		glUseProgram(shader::text_box_color.shader);
		
		glUniform1fv(shader::text_box_color.view_matrix, 7, &v->_final_matrix[0]);
		glUniform4f(shader::text_box_color.background_color,
								color.r() / 255.0f, color.g() / 255.0f, color.b() / 255.0f, color.a() / 255.0f );
		glUniform2f( shader::text_box_color.origin, -v->_origin.x(), -v->_origin.y() );
		
		while ( begin < end ) {
			TextFont::Cell& cell = data.cells[begin];
			
			if ( cell.chars.length() ) {
				float y = cell.baseline - data.text_hori_bearing + offset.y();
				float offset_start = cell.offset_start + offset.x();
				float* offset_table = &cell.offset[0];
				
				if ( cell.reverse ) {
					glUniform4f(shader::text_box_color.vertex_ac,
											offset_start - offset_table[cell.chars.length()], y,
											offset_start - offset_table[0], y + data.text_height);
				} else {
					glUniform4f(shader::text_box_color.vertex_ac,
											offset_start + offset_table[0], y,
											offset_start + offset_table[cell.chars.length()], y + data.text_height);
				}
				
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // 绘图背景
			}
			begin++;
		}
	}
	
	/**
	 * @func draw_vector_text
	 */
	void draw_vector_text(View* v, TextFont* f, TextFont::Data& data, Color color, Vec2 offset) {
		
		FontGlyphTable* table = font_pool()->get_table(f->_text_family.value, f->_text_style.value);
		
		glBindVertexArray(0);
		glUseProgram(shader::text_vertex.shader);
		
		glUniform1fv(shader::text_vertex.view_matrix, 7, &v->_final_matrix[0]);
		glUniform1f(shader::text_vertex.text_size, f->_text_size.value);
		glUniform4f(shader::text_vertex.color,
								color.r() / 255.0f, color.g() / 255.0f, color.b() / 255.0f, color.a() / 255.0f );
		glEnableVertexAttribArray(shader::text_vertex.vertex);
		
		for (int i = data.cell_draw_begin, e = data.cell_draw_end; i < e; i++) {
			
			TextFont::Cell& cell = data.cells[i];
			uint count = cell.chars.length();
			if ( count ) {
				const uint16* chars = &cell.chars[0];
				float offset_start = cell.offset_start + offset.x();
				float* offset_table = &cell.offset[0];
				
				glUniform1f(shader::text_vertex.hori_baseline, cell.baseline + offset.y());
				
				for (uint j = 0; j < count; j++) {
					FontGlyph* glyph = table->use_vector_glyph(chars[j]);
					/* 水平偏移 */
					float offset_x = offset_start + (cell.reverse ? -offset_table[j + 1] : offset_table[j]);
					
					glUniform1f(shader::text_vertex.offset_x, offset_x);
					glBindBuffer(GL_ARRAY_BUFFER, glyph->vertex_data());
					glVertexAttribPointer(shader::text_vertex.vertex, 2, GL_SHORT, GL_FALSE, 0, 0);
					glDrawArrays(GL_TRIANGLES, 0, glyph->vertex_count()); // 绘图
				}
			}
		}
	}
	
	/**
	 * @func draw_texture_text
	 */
	void draw_texture_text(View* v, TextFont* f, TextFont::Data& data, Color color, Vec2 offset) {
		
		FontGlyphTable* table = font_pool()->get_table(f->_text_family.value, f->_text_style.value);
		FGTexureLevel level = data.texture_level;
		
		glUseProgram(shader::text_texture.shader);
		
		glUniform1fv(shader::text_texture.view_matrix, 7, v->_final_matrix.value());
		glUniform1f(shader::text_texture.texture_scale, data.texture_scale);
		glUniform4f(shader::text_texture.color,
								color.r() / 255.0f, color.g() / 255.0f, color.b() / 255.0f, color.a() / 255.0f );
		glActiveTexture(GL_TEXTURE0);
		
		for (int i = data.cell_draw_begin, e = data.cell_draw_end; i < e; i++) {
			
			TextFont::Cell& cell = data.cells[i];
			uint count = cell.chars.length();
			if ( count ) {
				const uint16* chars = &cell.chars[0];
				float offset_start = cell.offset_start + offset.x();
				float* offset_table = &cell.offset[0];
				
				glUniform1f(shader::text_texture.hori_baseline, cell.baseline + offset.y());
				
				for (uint j = 0; j < count; j++) {
					
					uint16_t unicode = chars[j];
					FontGlyph* glyph = table->use_texture_glyph(unicode, level); /* 使用纹理 */
					FontGlyph::TexSize s = glyph->texture_size(level);
					glUniform4f(shader::text_texture.tex_size, s.width, s.height, s.left, s.top );
					
					/* 水平偏移 */
					float offset_x = offset_start + (cell.reverse ? -offset_table[j + 1] : offset_table[j]);
					
					glUniform1f(shader::text_texture.offset_x, offset_x);
					glBindTexture(GL_TEXTURE_2D, glyph->texture_id(level));
					
					/*
					LOG(
							"view_matrix: %f,%f,%f,%f,%f,%f,%f\n"
							"texture_scale: %f\n"
							"hori_baseline: %f\n"
							"tex_size: %d,%d,%d,%d\n"
							"offset_x: %f\n",
							v->_final_matrix[0],v->_final_matrix[1],v->_final_matrix[2],
							v->_final_matrix[3],v->_final_matrix[4],v->_final_matrix[5],v->_final_opacity,
							data.texture_scale,
							cell.baseline + offset.y(),
							s.width, s.height, s.left, s.top,
							offset_x
					);*/
					
					glDrawArrays(GL_TRIANGLE_FAN, 0, 4);  // 绘制文字纹理
				}
			}
		}
	}
	
	/**
	 * @func draw_text
	 */
	void draw_text(View* v, TextFont* f, TextFont::Data& data, Color color, Vec2 offset) {
		if ( data.texture_level == FontGlyph::LEVEL_NONE ) { //  没有纹理等级,使用矢量顶点
			draw_vector_text(v, f, data, color, offset - v->_origin);
		} else {
			draw_texture_text(v, f, data, color, offset - v->_origin);
		}
	}
	
	/**
	 * @func draw_input_cursor
	 */
	void draw_input_cursor(Input* v, int linenum, Vec2 offset) {
		
		glUseProgram(shader::text_box_color.shader);
		
		float y = v->_rows[linenum].baseline - v->_data.text_hori_bearing + offset.y();
		float x = v->cursor_x_ + offset.x();
		
		Color color = v->_text_color.value;
		
		glUniform1fv(shader::text_box_color.view_matrix, 7, &v->_final_matrix[0]);
		glUniform4f(shader::text_box_color.background_color,
								color.r() / 255.0f, color.g() / 255.0f, color.b() / 255.0f, color.a() / 255.0f );
		
		glUniform2f(shader::text_box_color.origin, -v->_origin.x(), -v->_origin.y());
		glUniform4f(shader::text_box_color.vertex_ac, x - 1, y, x + 1, y + v->_data.text_height);
		
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4); // 绘图背景
	}
	
	/**
	 * @func draw_input
	 */
	void draw_input(Input* v) {
		Vec2 offset = v->input_text_offset();
		uint begin = v->_data.cell_draw_begin;
		uint end = v->_data.cell_draw_end;
		
		if ( begin != end ) {
			
			Color color = v->_text_background_color.value;
			if ( color.a() ) {
				draw_text_background(v, v->_data, color, begin, end, offset);
			}
			
			// draw marked background
			begin = v->marked_cell_begin_; end = v->marked_cell_end_;
			if ( begin != end ) {
				draw_text_background(v, v->_data, v->marked_color_, begin, end, offset);
			}
			
			color = v->length() ? v->_text_color.value : v->placeholder_color_;
			
			draw_text(v, v, v->_data, color, offset);
			
			// draw cursor
			if ( v->editing_ && v->cursor_twinkle_status_ ) {
				draw_input_cursor(v, v->cursor_linenum_, offset);
			}
			
		} else {
			// draw cursor
			if ( v->editing_ && v->cursor_twinkle_status_ ) {
				draw_input_cursor(v, v->cursor_linenum_, offset);
			}
		}
	}
	
};

void GLDraw::clear_color(Color color) {
	glClearColor(color.r() / 255, color.g() / 255, color.b() / 255, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GLDraw::draw(Box* v) {
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v); // draw border
		if ( v->_clip ) {
			_inl(this)->draw_begin_clip(v);
			_inl(this)->draw_box_background(v, 1);
			v->visit(this);
			_inl(this)->draw_end_clip(v);
		} else {
			if (v->_background_color.a()) {
				_inl(this)->draw_box_background_color(v);
			}
			_inl(this)->draw_box_background(v, 0);
			v->visit(this);
		}
	} else {
		v->visit(this);
	}
}

void GLDraw::draw(Image* v) {
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v);

		bool clip = v->_clip;
		if ( clip ) 
			_inl(this)->draw_begin_clip(v);

		if (v->_final_width != 0 && v->_final_height != 0) {
			if ( v->_texture->use(0, Texture::Level(v->_tex_level)) ) {
				glUseProgram(shader::box_image.shader); // 使用矩形纹理着色器
				_inl(this)->set_box_uniform_value(shader::box_image, v);
				if (v->_is_draw_border_radius) { // 绘制圆角
					glUniform1f(shader::box_image.sample_x2, 30); // sample 15*2
					glUniform1i(shader::box_image.is_radius, 1);
					glDrawArrays(GL_TRIANGLE_STRIP, 0, 64);
				} else {
					glUniform1i(shader::box_image.is_radius, 0);
					glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
				}
			} else {
				if ( !clip && v->_background_color.a() ) { // 绘制背景
					_inl(this)->draw_box_background_color(v);
				}
				_inl(this)->draw_box_background(v, clip);
			}
		}

		v->visit(this); // draw child view

		if ( clip ) 
			_inl(this)->draw_end_clip(v);

	} else {// end draw
		v->visit(this);
	}
}

void GLDraw::draw(Video* v) {
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v);

		bool clip = v->_clip;
		if ( clip ) 
			_inl(this)->draw_begin_clip(v);

		Texture* tex = v->_texture;

		if (v->_status != PLAYER_STATUS_STOP &&
				v->_status != PLAYER_STATUS_START &&
				tex->use(0, Texture::LEVEL_0) && tex->use(1, Texture::LEVEL_1) ) {
			// Video暂时忽略圆角的绘制，可开启clip间接开启圆角。

			if ( tex->format() == PixelData::YUV420P ) {
				glUseProgram(shader::box_yuv420p_image.shader);
				_inl(this)->set_box_uniform_value(shader::box_yuv420p_image, v);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}
			else if (tex->format() == PixelData::YUV420SP) {
				glUseProgram(shader::box_yuv420sp_image.shader);
				_inl(this)->set_box_uniform_value(shader::box_yuv420sp_image, v);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			} else {
				// TODO 暂不支持
			}
		} else {
			if ( !clip && v->_background_color.a() ) { // 绘制背景
				_inl(this)->draw_box_background_color(v);
			}
			_inl(this)->draw_box_background(v, clip);
		}

		v->visit(this); // draw child view

		if ( clip ) 
			_inl(this)->draw_end_clip(v);

	} else {
		v->visit(this);
	}
}

void GLDraw::draw(BoxShadow* v) {
	if (v->_draw_visible) {
		_inl(this)->draw_box_border(v);

		bool clip = v->_clip;
		if ( clip ) {
			_inl(this)->draw_begin_clip(v);
		} else if (v->_background_color.a()) { // 绘制背景颜色
			_inl(this)->draw_box_background_color(v);
		}
		_inl(this)->draw_box_background(v, clip);

		// TODO draw shadow ..
		// ..
		
		v->visit(this); // draw child view

		if ( clip ) 
			_inl(this)->draw_end_clip(v);

	} else {
		v->visit(this);
	}
}

void GLDraw::draw(Sprite* v) {
	if ( v->_draw_visible ) { // 为false时不需要绘制
		if ( v->_texture->use(0, Texture::Level(v->_tex_level), v->_repeat) ) {
			glUseProgram(shader::sprite.shader);
			glUniform1fv(shader::sprite.view_matrix, 7, v->_final_matrix.value());
			glUniform4f(shader::sprite.vertex_ac, 
									-v->_origin.x(), -v->_origin.y(),
									v->_size.width() - v->_origin.x(),
									v->_size.height() - v->_origin.y());
			glUniform4f(shader::sprite.tex_start_size,
									v->_ratio.x(), v->_ratio.y(),
									v->_texture->width(), v->_texture->height());
			glUniform2f(shader::sprite.tex_ratio, v->_ratio.x(), v->_ratio.y());
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
	}
	v->visit(this);
}

void GLDraw::draw(TextNode* v) {
	if ( v->_draw_visible ) {
		uint begin = v->_data.cell_draw_begin;
		uint end = v->_data.cell_draw_end;
		
		if ( begin != end ) {
			Vec2 offset = Vec2(-v->_offset_start.x(), -v->_offset_start.y());
			Color color = v->_text_background_color.value;
			if ( color.a() ) {
				_inl(this)->draw_text_background(v, v->_data, color, begin, end, offset);
			}
			_inl(this)->draw_text(v, v, v->_data, v->_text_color.value, offset);
		}
	}
}

void GLDraw::draw(Label* v) {
	if ( v->_draw_visible ) {
		uint begin = v->_data.cell_draw_begin;
		uint end = v->_data.cell_draw_end;
		
		if ( begin != end ) {
			Color color = v->_text_background_color.value;
			if ( color.a() ) {
				_inl(this)->draw_text_background(v, v->_data, color, begin, end, Vec2());
			}
			_inl(this)->draw_text(v, v, v->_data, v->_text_color.value, Vec2());
		}
	}
}

void GLDraw::draw(Text* v) {
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v);
		
		bool clip = v->_clip;
		if ( clip ) {
			_inl(this)->draw_begin_clip(v);
		} else if (v->_background_color.a()) { // 绘制背景颜色
			_inl(this)->draw_box_background_color(v);
		}
		_inl(this)->draw_box_background(v, clip);

		uint begin = v->_data.cell_draw_begin;
		uint end = v->_data.cell_draw_end;
		
		if ( begin != end ) {
			Color color = v->_text_background_color.value;
			if ( color.a() ) {
				_inl(this)->draw_text_background(v, v->_data, color, begin, end, Vec2());
			}
			_inl(this)->draw_text(v, v, v->_data, v->_text_color.value, Vec2());
		}

		if ( clip ) 
			_inl(this)->draw_end_clip(v);
	}
}

void GLDraw::draw(Scroll* v) {
	uint inherit_mark = v->mark_value & View::M_INHERIT;
	if ( v->mark_value & Scroll::M_SCROLL ) {
		inherit_mark |= View::M_MATRIX;
	}
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v);
		_inl(this)->draw_begin_clip(v);
		_inl(this)->draw_box_background(v, true);
		v->visit(this, inherit_mark);
		_inl(this)->draw_end_clip(v);
		_inl(this)->draw_scroll_bar(v, v); // 绘制scrollbar
	} else {
		v->visit(this, inherit_mark);
	}
}

void GLDraw::draw(Input* v) {
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v);
		_inl(this)->draw_begin_clip(v);
		_inl(this)->draw_box_background(v, true);
		_inl(this)->draw_input(v);
		_inl(this)->draw_end_clip(v);
	}
}

void GLDraw::draw(Textarea* v) {
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v);
		_inl(this)->draw_begin_clip(v);
		_inl(this)->draw_box_background(v, true);
		_inl(this)->draw_input(v);
		_inl(this)->draw_end_clip(v);
		_inl(this)->draw_scroll_bar(v, v); // 绘制scrollbar
	}
}

void GLDraw::draw(Root* v) {
	if ( v->_draw_visible ) {
		_inl(this)->draw_box_border(v);
		_inl(this)->draw_box_background(v, true);
	}
	v->visit(this);
}

}
