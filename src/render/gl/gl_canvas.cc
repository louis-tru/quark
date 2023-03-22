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

#include "./gl_canvas.h"

namespace qk {

	static void gl_use_texture(GLuint id, const Paint& paint, uint32_t slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);

		switch (paint.tileMode) {
			case Paint::kClamp_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
			case Paint::kRepeat_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				break;
			case Paint::kRepeat_X_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
			case Paint::kRepeat_Y_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				break;
			case Paint::kMirror_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				break;
			case Paint::kMirror_X_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
			case Paint::kMirror_Y_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				break;
			case Paint::kDecal_TileMode: // no repeat
				// GL_CLAMP_TO_BORDER
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
		}

		switch (paint.filterMode) {
			case Paint::kNearest_FilterMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			case Paint::kLinear_FilterMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
		}

		switch (paint.mipmapMode) {
			case Paint::kNone_MipmapMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				break;
			case Paint::kNearest_MipmapMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;
			case Paint::kLinear_MipmapMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;
		}
	}

	GLCanvas::GLCanvas()
		: _blendMode(kClear_BlendMode)
		, _IsDeviceMsaa(false)
		, _texTmp{0,0,0}
		, _linear(Paint::kLinear_GradientType)
		, _radial(Paint::kRadial_GradientType)
		, _shaders{&_color, &_image, &_yuv420p, &_yuv420sp, &_linear, &_radial}
	{
		glGenBuffers(1, &_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 32, NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _ubo);

		for (auto shader: _shaders) {
			shader->build();
		}

		glUseProgram(_image.shader());
		glUniform1i(_image.image(), 0);

		glUseProgram(_yuv420p.shader());
		glUniform1i(_yuv420p.image(), 0);
		glUniform1i(_yuv420p.image_u(), 1);
		glUniform1i(_yuv420p.image_v(), 2);

		glUseProgram(_yuv420sp.shader());
		glUniform1i(_yuv420sp.image(), 0);
		glUniform1i(_yuv420sp.image_uv(), 1);
		
		glUseProgram(0);

		setMatrix(Mat()); // init matrix
	}

	void GLCanvas::setMatrix(const Mat& mat) {
		float mat4[16] = {
			mat[0], mat[3], 0.0, 0.0,
			mat[1], mat[4], 0.0, 0.0,
			0.0,    0.0,    1.0, 0.0,
			mat[2], mat[5], 0.0, 1.0
		};
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 16, mat4);
	}

	int  GLCanvas::save() {
		// TODO ...
	}

	void GLCanvas::restore() {
		// TODO ...
	}

	int  GLCanvas::getSaveCount() const {
		// TODO ...
	}

	void GLCanvas::restoreToCount(int saveCount) {
		// TODO ...
	}

	bool GLCanvas::readPixels(Pixel* dstPixels, int srcX, int srcY) {
		// TODO ...
	}

	void GLCanvas::clipRect(const Rect& rect, ClipOp op, bool antiAlias) {
		// TODO ...
	}

	void GLCanvas::clipPath(const Path& path, ClipOp op, bool antiAlias) {
		Array<Vec2> triangles = path.getPolygons(3);

		glUseProgram(_color.shader());
		// glUniform4fv(_color.color(), 1, paint.color.val);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, triangles.val());
		glDrawArrays(GL_TRIANGLES, 0, triangles.length());
	}

	void GLCanvas::drawPaint(const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::drawPath(const Path& path, const Paint& paint) {

		bool antiAlias = paint.antiAlias && !_IsDeviceMsaa; // Anti-aliasing using software

		Array<Vec2> polygons;

		if (_blendMode != paint.blendMode) {
			setBlendMode(paint.blendMode); // switch blend mode
		}

		// gen stroke path and fill path and polygons
		switch (paint.style) {
			case Paint::kFill_Style:
				polygons = path.getPolygons(3);
				break;
			case Paint::kStroke_Style:
				polygons = path.strokePath(paint.width, paint.join).getPolygons(3);
				break;
			case Paint::kStrokeAndFill_Style:
				polygons = path.extendPath(paint.width * 0.5, paint.join).getPolygons(3);
				break;
		}

		// fill polygons
		switch (paint.type) {
			case Paint::kColor_Type:
				drawColor(polygons, paint); break;
			case Paint::kGradient_Type:
				drawGradient(polygons, paint); break;
			case Paint::kBitmap_Type:
				drawImage(polygons, paint); break;
		}
	}

	void GLCanvas::drawColor(const Array<Vec2>& triangles, const Paint& paint) {
		_color.use(sizeof(Vec2) * triangles.length(), *triangles);
		glUniform4fv(_color.color(), 1, paint.color.val);
		glDrawArrays(GL_TRIANGLES, 0, triangles.length());
	}

	void GLCanvas::drawGradient(const Array<Vec2>& triangles, const Paint& paint) {
		const GradientColor *g = paint.gradientColor();
		auto shader = paint.gradientType == Paint::kLinear_GradientType ? &_linear: &_radial;
		glUseProgram(shader->shader());
		glUniform4fv(shader->range(), 1, paint.color.val);
		glUniform1i(shader->count(), g->colors.length());
		glUniform4fv(shader->colors(), g->colors.length(), (const GLfloat*)g->colors.val());
		glUniform1fv(shader->positions(), g->colors.length(), (const GLfloat*)g->positions.val());
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, triangles.val());
		glDrawArrays(GL_TRIANGLES, 0, triangles.length());
	}

	void GLCanvas::drawImage(const Array<Vec2>& triangles, const Paint& paint) {
		auto pixel = paint.bitmapPixel();
		auto type = pixel->type();
		auto shader = &_image;
		auto texCount = 1;

		if (type == kColor_Type_YUV420P_Y_8) {
			shader = &_yuv420p;
			texCount = 3;
		} else if (type == kColor_Type_YUV420SP_Y_8) {
			shader = &_yuv420sp;
			texCount = 2;
		}

		glUseProgram(shader->shader());
		glUniform1f(shader->opacity(), paint.opacity);
		glUniform4fv(shader->coord(), 1, paint.color.val);

		for (int i = 0; i < texCount; i++) {
			auto id = pixel[i].texture();
			if (!id || (id = setTexture(pixel+i, _texTmp[i], true)))
				_texTmp[i] = id;
			gl_use_texture(id, paint, i);
		}

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, triangles.val());
		glDrawArrays(GL_TRIANGLES, 0, triangles.length());
	}

	void GLCanvas::drawGlyphs(const Array<GlyphID>& glyphs, const Array<Vec2>& positions,
		Vec2 origin, float fontSize, Typeface* typeface, const Paint& paint) 
	{
		// TODO ...
	}

	void GLCanvas::drawTextBlob(TextBlob* blob, Vec2 origin, float floatSize, const Paint& paint) {
		// TODO ...
	}

	void GLCanvas::setBlendMode(BlendMode blendMode) {

		switch (blendMode) {
			case kClear_BlendMode:         //!< r = 0
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kSrc_BlendMode:           //!< r = s
				glBlendFunc(GL_ONE, GL_ZERO);
				break;
			case kDst_BlendMode:           //!< r = d
				glBlendFunc(GL_ZERO, GL_ONE);
				break;
			case kSrcOver_BlendMode:       //!< r = s + (1-sa)*d
				/** [Sa + (1 - Sa)*Da, Rc = Sc + (1 - Sa)*Dc] */
				// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kDstOver_BlendMode:       //!< r = (1-da)*s + d
				/** [Sa + (1 - Sa)*Da, Rc = Dc + (1 - Da)*Sc] */
				// glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
				glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
				break;
			case kSrcIn_BlendMode:         //!< r = da*s
				glBlendFunc(GL_DST_ALPHA, GL_ZERO);
				break;
			case kDstIn_BlendMode:         //!< r = sa*d
				glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
				break;
			case kSrcOut_BlendMode:        //!< r = (1-da)*s
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ZERO);
				break;
			case kDstOut_BlendMode:        //!< r = (1-sa)*d
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kSrcATop_BlendMode:       //!< r = da*s + (1-sa)*d
				glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kDstATop_BlendMode:       //!< r = (1-da)*s + sa*d
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_SRC_ALPHA);
				break;
			case kXor_BlendMode:           //!< r = (1-da)*s + (1-sa)*d
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kPlus_BlendMode:          //!< r = min(s + d, 1)
				glBlendFunc(GL_ONE, GL_ONE);
				break;
			case kModulate_BlendMode:      //!< r = s*d
				glBlendFunc(GL_ZERO, GL_SRC_COLOR);
				break;
			case kScreen_BlendMode:        //!< r = s + d - s*d
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
				break;
		}

		_blendMode = blendMode;
	}

	static GLint get_gl_texture_data_format(ColorType format) {
		switch (format) {
			case kColor_Type_Alpha_8: return GL_UNSIGNED_BYTE;
			case kColor_Type_RGB_565: return GL_UNSIGNED_SHORT_5_6_5;
			case kColor_Type_RGBA_4444: return GL_UNSIGNED_SHORT_4_4_4_4;
			case kColor_Type_RGB_444X: return GL_UNSIGNED_SHORT_4_4_4_4;
#if Qk_OSX
			case kColor_Type_RGBA_8888: return GL_UNSIGNED_INT_8_8_8_8;
			case kColor_Type_RGB_888X: return GL_UNSIGNED_INT_8_8_8_8;
			case kColor_Type_BGRA_8888: return GL_UNSIGNED_INT_8_8_8_8;
			case kColor_Type_RGBA_1010102: return GL_UNSIGNED_INT_10_10_10_2;
			case kColor_Type_BGRA_1010102: return GL_UNSIGNED_INT_10_10_10_2;
			case kColor_Type_RGB_101010X: return GL_UNSIGNED_INT_10_10_10_2;
			case kColor_Type_BGR_101010X: return GL_UNSIGNED_INT_10_10_10_2;
#else
			case kColor_Type_RGBA_8888: return GL_UNSIGNED_BYTE;
			case kColor_Type_RGB_888X: return GL_UNSIGNED_BYTE;
			case kColor_Type_BGRA_8888: return GL_UNSIGNED_BYTE;
			case kColor_Type_RGBA_1010102: return GL_UNSIGNED_INT_2_10_10_10_REV;
			case kColor_Type_BGRA_1010102: return GL_UNSIGNED_INT_2_10_10_10_REV;
			case kColor_Type_RGB_101010X: return GL_UNSIGNED_INT_2_10_10_10_REV;
			case kColor_Type_BGR_101010X: return GL_UNSIGNED_INT_2_10_10_10_REV;
#endif
			case kColor_Type_RGB_888: return GL_UNSIGNED_BYTE;
			case kColor_Type_RGBA_5551: return GL_UNSIGNED_SHORT_5_5_5_1;
			default: return GL_UNSIGNED_BYTE;
		}
	}

	static GLint get_gl_texture_pixel_format(ColorType type) {
#if Qk_APPLE
#if Qk_OSX
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A
#endif
	switch (type) {
		case kColor_Type_Alpha_8: return GL_ALPHA;
		case kColor_Type_RGB_565: return GL_RGB;
		case kColor_Type_RGBA_4444: return GL_RGBA;
		case kColor_Type_RGB_444X: return GL_RGBA;//GL_RGB;
		case kColor_Type_RGBA_8888: return GL_RGBA;
		case kColor_Type_RGB_888X: return GL_RGBA;//GL_RGB;
		case kColor_Type_BGRA_8888: return GL_BGRA;
		case kColor_Type_RGBA_1010102: return GL_RGBA;
		case kColor_Type_BGRA_1010102: return GL_BGRA;
		case kColor_Type_RGB_101010X: return GL_RGBA; // GL_RGB
		case kColor_Type_BGR_101010X: return GL_BGRA; // GL_BGR;
		case kColor_Type_RGB_888: return GL_RGB;
		case kColor_Type_RGBA_5551: return GL_RGBA;
		case kColor_Type_Luminance_8: return GL_LUMINANCE;
		case kColor_Type_Luminance_Alpha_88: return GL_LUMINANCE_ALPHA;
		// case kColor_Type_SDF_Float: return GL_RGBA;
		case kColor_Type_YUV420P_Y_8: return GL_LUMINANCE;
		// case kColor_Type_YUV420P_V_8:
		case kColor_Type_YUV420P_U_8: return GL_LUMINANCE;
		case kColor_Type_YUV420SP_Y_8: return GL_LUMINANCE;
		case kColor_Type_YUV420SP_UV_88: return GL_LUMINANCE_ALPHA;
#if Qk_iOS // ios
			// compressd texture
		case kColor_Type_PVRTCI_2BPP_RGB: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
		case kColor_Type_PVRTCI_2BPP_RGBA:
		case kColor_Type_PVRTCII_2BPP: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
		case kColor_Type_PVRTCI_4BPP_RGB: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
		case kColor_Type_PVRTCI_4BPP_RGBA:
		case kColor_Type_PVRTCII_4BPP: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
		case kColor_Type_ETC1:
		case kColor_Type_ETC2_RGB: return GL_COMPRESSED_RGB8_ETC2;
		case kColor_Type_ETC2_RGB_A1:
		case kColor_Type_ETC2_RGBA: return GL_COMPRESSED_RGBA8_ETC2_EAC;
#endif
		default: return 0;
	}
#endif

#if Qk_LINUX
		return 0;
#endif
	}

	uint32_t GLCanvas::setTexture(cPixel* src, GLuint id, bool isGenerateMipmap) {
		if ( src->body().length() == 0 )
			return 0;

		ColorType type = src->type();
		GLint internalformat = get_gl_texture_pixel_format(type);
		Qk_ASSERT(internalformat);

		if (!internalformat)
			return 0;

		if (!id) {
			glGenTextures(1, &id);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id);

#if defined(GL_EXT_texture_filter_anisotropic) && GL_EXT_texture_filter_anisotropic == 1
		//  GLfloat largest;
		//  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest);
		//  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest);
#endif

		glPixelStorei(GL_UNPACK_ALIGNMENT, Pixel::bytes_per_pixel(type));
		// GL_REPEAT / GL_CLAMP_TO_EDGE / GL_MIRRORED_REPEAT 
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		if ( type >= kColor_Type_PVRTCI_2BPP_RGB ) {
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_level - 1);
			glCompressedTexImage2D(GL_TEXTURE_2D, 0/*level*/, internalformat,
														src->width(),
														src->height(), 0/*border*/, src->body().length(), *src->body());
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0/*level*/, internalformat,
									src->width(),
									src->height(), 0/*border*/, internalformat/*format*/,
									get_gl_texture_data_format(type)/*type*/, *src->body());
			if (isGenerateMipmap) {
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		return id;
	}

	void GLCanvas::deleteTextures(const GLuint *IDs, uint32_t count) {
		glDeleteTextures(count, IDs);
	}

}
