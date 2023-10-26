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

#include "../../app.h"
#include "../../display.h"
#include "./gl_render.h"
#include "./gl_canvas.h"

namespace qk {

	String gl_MaxTextureImageUnits_GLSL_Macros;
	int    gl_MaxTextureImageUnits = 0;

	void gl_textureBarrier() {
#if defined(GL_ARB_texture_barrier)
		glTextureBarrier();
#elif defined(GL_NV_texture_barrier)
		glTextureBarrierNV();
#elif Qk_OSX
		glFlushRenderAPPLE();
#else
		glFlush();
#endif
	}

	GLint gl_get_texture_pixel_format(ColorType type) {
#if Qk_MAC
#if Qk_OSX
#define GL_LUMINANCE                      GL_RED
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
			// case kColor_Type_YUV420SP_Y_8:
			case kColor_Type_YUV420P_Y_8: return GL_LUMINANCE;
			// case kColor_Type_YUV420P_V_8:
			case kColor_Type_YUV420P_U_8: return GL_LUMINANCE;
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

	GLint gl_get_texture_data_type(ColorType format) {
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

	uint32_t gl_gen_texture(cPixel* src, GLuint id, bool genMipmap) {
		if ( src->body().length() == 0 )
			return 0;

		ColorType type = src->type();
		GLint iformat = gl_get_texture_pixel_format(type);
		Qk_ASSERT(iformat);

		if (!iformat)
			return 0;

		if (!id) {
			glGenTextures(1, &id);
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, id);

#if defined(GL_EXT_texture_filter_anisotropic)
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
			glCompressedTexImage2D(GL_TEXTURE_2D, 0/*level*/, iformat,
														src->width(),
														src->height(), 0/*border*/, src->body().length(), *src->body());
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0/*level*/, iformat,
									src->width(),
									src->height(), 0/*border*/, iformat/*format*/,
									gl_get_texture_data_type(type)/*type*/, *src->body());
			if (genMipmap) {
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		//constexpr float black[4] = {0,0,0,0};
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);

		return id;
	}

	void gl_set_texture_no_repeat(GLenum pname) {
#if Qk_OSX
		glTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_BORDER);
#else
		glTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_EDGE);
#endif
	}

	void gl_set_texture_param(GLuint id, uint32_t slot, const ImagePaint* paint) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);

		switch (paint->tileModeX) {
			case ImagePaint::kClamp_TileMode: // border repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				break;
			case ImagePaint::kRepeat_TileMode: // repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				break;
			case ImagePaint::kMirror_TileMode: // mirror repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				break;
			case ImagePaint::kDecal_TileMode: // no repeat
				gl_set_texture_no_repeat(GL_TEXTURE_WRAP_S);
				break;
		}

		switch (paint->tileModeY) {
			case ImagePaint::kClamp_TileMode: // border repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
			case ImagePaint::kRepeat_TileMode: // repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				break;
			case ImagePaint::kMirror_TileMode: // mirror repeat
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				break;
			case ImagePaint::kDecal_TileMode: // no repeat
				gl_set_texture_no_repeat(GL_TEXTURE_WRAP_T);
				break;
		}

		switch (paint->filterMode) {
			case ImagePaint::kNearest_FilterMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			case ImagePaint::kLinear_FilterMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
		}

		switch (paint->mipmapMode) {
			case ImagePaint::kNone_MipmapMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				break;
			case ImagePaint::kNearest_MipmapMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;
			case ImagePaint::kLinear_MipmapMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;
		}
	}

	void gl_set_color_blend_mode(BlendMode blendMode) {
		switch (blendMode) {
			case kClear_BlendMode:         //!< r = 0 + (1-sa)*d
				// glBlendFunc (sfactor, dfactor)
				glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kSrc_BlendMode:           //!< r = s
				glBlendFunc(GL_ONE, GL_ZERO);
				// glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ZERO, GL_ZERO);
				break;
			case kDst_BlendMode:           //!< r = d
				glBlendFunc(GL_ZERO, GL_ONE);
				break;
			case kSrcOver_BlendMode:       //!< r = sa*s + (1-sa)*d
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				//glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kDstOver_BlendMode:       //!< r = (1-da)*s + da*d
				glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
				//glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
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
	}

	void gl_tex_image2D(GLuint tex, Vec2 size, GLint iformat, GLenum type, GLuint slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, tex);
		// glBindBuffer(GL_PIXEL_UNPACK_BUFFER, readBuffer);
		// glTexStorage2D(GL_TEXTURE_2D, 1, iformat, size[0], size[1]);
		glTexImage2D(GL_TEXTURE_2D, 0/*level*/, iformat, size[0], size[1], 0, iformat, type, nullptr);
	}

	void gl_set_framebuffer_renderbuffer(GLuint buff, Vec2 size, GLenum iformat, GLenum attachment) {
		glBindRenderbuffer(GL_RENDERBUFFER, buff);
		// msaa > 1 ?
		// glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, iformat, size[0], size[1]):
		glRenderbufferStorage(GL_RENDERBUFFER, iformat, size[0], size[1]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buff);
	}

	void gl_set_color_renderbuffer(GLuint buff, ColorType type, Vec2 size, bool texRBO) {
		if (texRBO) {
			// use texture render buffer
			gl_tex_image2D(buff, size, gl_get_texture_pixel_format(type), gl_get_texture_data_type(type), 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buff, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 64);
		} else {
			GLenum ifo;
			switch (type) {
				case kColor_Type_RGB_565: ifo = GL_RGB565; break;
				case kColor_Type_RGBA_8888: ifo = GL_RGBA8; break;
				case kColor_Type_RGB_888X: ifo = GL_RGBA8; break;
				case kColor_Type_RGBA_1010102: ifo = GL_RGB10_A2; break;
				case kColor_Type_RGB_101010X: ifo = GL_RGB10_A2; break;
				default: ifo = GL_RGBA8; break;
			}
			gl_set_framebuffer_renderbuffer(buff, size, ifo, GL_COLOR_ATTACHMENT0);
		}
	}

	void gl_set_aaclip_buffer(GLuint tex, Vec2 size) {
		// clip anti alias buffer
		gl_tex_image2D(tex, size, GL_LUMINANCE, GL_UNSIGNED_BYTE, gl_MaxTextureImageUnits - 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// range: 0 - 1, no repeat
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex, 0);
	}

	void gl_set_blur_renderbuffer(GLuint tex, Vec2 size) {
		gl_tex_image2D(tex, size,
			gl_get_texture_pixel_format(kColor_Type_RGBA_8888),
			gl_get_texture_data_type(kColor_Type_RGBA_8888), 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
#if DEBUG
		int texDims;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 4, GL_TEXTURE_WIDTH, &texDims);
		Qk_DEBUG("glGetTexLevelParameteriv: %d", texDims);
#endif
	}

	GLRender::GLRender(Options opts)
		: Render(opts)
		, _texBuffer{0,0,0}, _glcanvas(nullptr)
	{
		_glcanvas = new GLCanvas(this, _opts);
		_canvas = _glcanvas; // set default canvas
		_glcanvas->retain(); // retain

		glGenFramebuffers(1, &_fbo);
		glGenBuffers(3, &_rootMatrixBlock); // _matrixBlock, _viewMatrixBlock, _optsBlock
		glBindBuffer(GL_UNIFORM_BUFFER, _rootMatrixBlock);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _rootMatrixBlock);
		// _viewMatrixBlock
		glBindBuffer(GL_UNIFORM_BUFFER, _viewMatrixBlock);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, _viewMatrixBlock);
		// _optsBlock
		glBindBuffer(GL_UNIFORM_BUFFER, _optsBlock);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, _optsBlock);
		// Create texture buffer
		glGenTextures(3, _texBuffer); // _texBuffer

#if DEBUG
		GLint maxTextureSize,maxTextureBufferSize,maxTextureImageUnits;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTextureBufferSize);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
		Qk_DEBUG("GL_MAX_TEXTURE_SIZE: %d", maxTextureSize);
		Qk_DEBUG("GL_MAX_TEXTURE_BUFFER_SIZE: %d", maxTextureBufferSize);
		Qk_DEBUG("GL_MAX_TEXTURE_IMAGE_UNITS: %d", maxTextureImageUnits);
#endif

		if (!gl_MaxTextureImageUnits) {
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_MaxTextureImageUnits);
			gl_MaxTextureImageUnits_GLSL_Macros = 
				String("#define Qk_GL_MAX_TEXTURE_IMAGE_UNITS ") + gl_MaxTextureImageUnits + "\n";
		}

#if DEBUG || QK_MoreLog
		int64_t st = time_micro();
		_shaders.buildAll(); // compile all shaders
		Qk_DEBUG("shaders.buildAll time: %ld (micro s)", time_micro() - st);
#else
		_shaders.buildAll();
#endif

		// settings shader
		for (auto s = &_shaders.image, e = s + 4; s < e; s++) {
			glUseProgram(s->shader);
			glUniform1i(s->image, 0); // set texture slot
		}
		for (auto s = &_shaders.imageMask, e = s + 4; s < e; s++) {
			glUseProgram(s->shader);
			glUniform1i(s->image, 0); // set texture slot
		}
		for (auto s = &_shaders.imageYuv, e = s + 4; s < e; s++) {
			glUseProgram(s->shader);
			glUniform1i(s->image, 0); // set texture slot
			glUniform1i(s->image_u, 1);
			glUniform1i(s->image_v, 2);
		}
		for (auto s = &_shaders.color1, e = s + 2; s < e; s++) {
			glUniformBlockBinding(s->shader, glGetUniformBlockIndex(s->shader, "optsBlock"), 2); // binding = 2
		}
		glUseProgram(_shaders.imageCp.shader);
		glUniform1i(_shaders.imageCp.image, 0);
		glUseProgram(_shaders.blur.shader);
		glUniform1i(_shaders.blur.image, 0); // GL_MaxTextureImageUnits - 2

		glEnable(GL_BLEND); // enable color blend
		setBlendMode(kSrcOver_BlendMode); // set default color blend mode
		// enable and disable test function
		glClearStencil(127);
		glStencilMask(0xFFFFFFFF);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); // enable color
		glDisable(GL_STENCIL_TEST); // disable stencil test
		// set depth test
		glEnable(GL_DEPTH_TEST); // enable depth test
		glDepthFunc(GL_GREATER); // passes if depth is greater than the stored depth.
		glClearDepth(0.0f); // set depth clear value to -1.0
	}

	GLRender::~GLRender() {
		Qk_STRICT_ASSERT(_glcanvas == nullptr);
	}

	void GLRender::release() {
		GLuint fbo = _fbo,
					tbo[] = {_texBuffer[0],_texBuffer[1],_texBuffer[2]},
					ubo[] = {_rootMatrixBlock,_viewMatrixBlock,_optsBlock};
		post_message(Cb([fbo,tbo,ubo](auto &e){
			glDeleteFramebuffers(1, &fbo);
			glDeleteTextures(3, tbo);
			glDeleteBuffers(3, ubo);
		}));
		Qk_ASSERT(_glcanvas->refCount() == 1);
		_glcanvas->release(); _glcanvas = nullptr;
		_canvas = nullptr;
	}

	void GLRender::reload() {
		lock();
		_surfaceSize = getSurfaceSize(&_defaultScale);
		_delegate->onRenderBackendReload({Vec2{0,0},_surfaceSize}, _surfaceSize, _defaultScale);
		unlock();
	}

	uint32_t GLRender::makeTexture(cPixel *src, uint32_t id) {
		return gl_gen_texture(src, id, true);
	}

	void GLRender::deleteTextures(const uint32_t *ids, uint32_t count) {
		glDeleteTextures(count, ids);
	}

	void GLRender::makeVertexData(VertexData::ID *id) {
		if (!id->vao) {
			auto &vertex = id->self->vertex;
			glGenVertexArrays(1, &id->vao);
			glGenBuffers(1, &id->vbo);
			glBindVertexArray(id->vao);
			glBindBuffer(GL_ARRAY_BUFFER, id->vbo);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec3), (const GLvoid*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vec3), (const GLvoid*)sizeof(Vec2));
			glEnableVertexAttribArray(1);
			glBufferData(GL_ARRAY_BUFFER, vertex.size(), vertex.val(), GL_STREAM_DRAW);
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			vertex.clear(); // clear memory data
		}
	}

	void GLRender::deleteVertexData(VertexData::ID *id) {
		if (id->vao) {
			glDeleteVertexArrays(1, &id->vao);
			glDeleteBuffers(1, &id->vbo);
			id->vao = 0;
			id->vbo = 0;
		}
	}

	void GLRender::setTexture(cPixel *pixel, int slot, const ImagePaint *paint) {
		auto id = pixel->texture();
		if (!id) {
			id = gl_gen_texture(pixel, _texBuffer[slot], true);
			if (!id) {
				Qk_DEBUG("setTexturePixel() fail"); return;
			}
			_texBuffer[slot] = id;
		}
		gl_set_texture_param(id, slot, paint);
	}

	void GLRender::setBlendMode(BlendMode mode) {
		_blendMode = mode;
		gl_set_color_blend_mode(mode);
	}

	void GLRender::lock() {}
	void GLRender::unlock() {}

	Canvas* GLRender::newCanvas(Options opts) {
		opts.colorType = opts.colorType ? opts.colorType: kColor_Type_RGBA_8888;
		return new GLCanvas(this, opts);
	}

}
