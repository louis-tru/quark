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

#include "./gl_render.h"
#include "./gl_cmd.h"

namespace qk {

	String gl_MaxTextureImageUnits_GLSL_Macros;
	int    gl_MaxTextureImageUnits = 0;

	void gl_texture_barrier() {
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
		switch (type) {
			case kAlpha_8_ColorType: return GL_ALPHA;
			case kRGB_565_ColorType: return GL_RGB;
			case kRGBA_4444_ColorType: return GL_RGBA;
			case kRGB_444X_ColorType: return GL_RGBA;//GL_RGB;
			case kRGBA_8888_ColorType: return GL_RGBA;
			case kRGB_888X_ColorType: return GL_RGBA;//GL_RGB;
			case kBGRA_8888_ColorType: return GL_BGRA;
			case kRGBA_1010102_ColorType: return GL_RGBA;
			case kBGRA_1010102_ColorType: return GL_BGRA;
			case kRGB_101010X_ColorType: return GL_RGBA; // GL_RGB
			case kBGR_101010X_ColorType: return GL_BGRA; // GL_BGR;
			case kRGB_888_ColorType: return GL_RGB;
			case kRGBA_5551_ColorType: return GL_RGBA;
#if Qk_OSX
# define GL_LUMINANCE       GL_RED
# define GL_LUMINANCE_ALPHA GL_RG
#endif
			// TODO Grayscale images may not display properly for macos
			case kLuminance_8_ColorType: return GL_LUMINANCE;
			case kLuminance_Alpha_88_ColorType: return GL_LUMINANCE_ALPHA;
			// case kSDF_Float_ColorType: return GL_RGBA;
			// case kYUV420SP_Y_8_ColorType:
			case kYUV420P_Y_8_ColorType: return GL_LUMINANCE;
			// case kYUV420P_V_8_ColorType:
			case kYUV420P_U_8_ColorType: return GL_LUMINANCE;
			case kYUV420SP_UV_88_ColorType: return GL_RG;
#if Qk_iOS // ios
				// compressd texture
			case kPVRTCI_2BPP_RGB_ColorType: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			case kPVRTCI_2BPP_RGBA_ColorType:
			case kPVRTCII_2BPP_ColorType: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			case kPVRTCI_4BPP_RGB_ColorType: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			case kPVRTCI_4BPP_RGBA_ColorType:
			case kPVRTCII_4BPP_ColorType: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
			case kETC1_ColorType:
			case kETC2_RGB_ColorType: return GL_COMPRESSED_RGB8_ETC2;
			case kETC2_RGB_A1_ColorType:
			case kETC2_RGBA_ColorType: return GL_COMPRESSED_RGBA8_ETC2_EAC;
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
			case kAlpha_8_ColorType: return GL_UNSIGNED_BYTE;
			case kRGB_565_ColorType: return GL_UNSIGNED_SHORT_5_6_5;
			case kRGBA_4444_ColorType: return GL_UNSIGNED_SHORT_4_4_4_4;
			case kRGB_444X_ColorType: return GL_UNSIGNED_SHORT_4_4_4_4;
#if Qk_OSX
			case kRGBA_8888_ColorType: return GL_UNSIGNED_INT_8_8_8_8;
			case kRGB_888X_ColorType: return GL_UNSIGNED_INT_8_8_8_8;
			case kBGRA_8888_ColorType: return GL_UNSIGNED_INT_8_8_8_8;
			case kRGBA_1010102_ColorType: return GL_UNSIGNED_INT_10_10_10_2;
			case kBGRA_1010102_ColorType: return GL_UNSIGNED_INT_10_10_10_2;
			case kRGB_101010X_ColorType: return GL_UNSIGNED_INT_10_10_10_2;
			case kBGR_101010X_ColorType: return GL_UNSIGNED_INT_10_10_10_2;
#else
			case kRGBA_8888_ColorType: return GL_UNSIGNED_BYTE;
			case kRGB_888X_ColorType: return GL_UNSIGNED_BYTE;
			case kBGRA_8888_ColorType: return GL_UNSIGNED_BYTE;
			case kRGBA_1010102_ColorType: return GL_UNSIGNED_INT_2_10_10_10_REV;
			case kBGRA_1010102_ColorType: return GL_UNSIGNED_INT_2_10_10_10_REV;
			case kRGB_101010X_ColorType: return GL_UNSIGNED_INT_2_10_10_10_REV;
			case kBGR_101010X_ColorType: return GL_UNSIGNED_INT_2_10_10_10_REV;
#endif
			case kRGB_888_ColorType: return GL_UNSIGNED_BYTE;
			case kRGBA_5551_ColorType: return GL_UNSIGNED_SHORT_5_5_5_1;
			default: return GL_UNSIGNED_BYTE;
		}
	}

	void gl_set_texture_no_repeat(GLenum wrapdir) {
#if Qk_OSX
		glTexParameteri(GL_TEXTURE_2D, wrapdir, GL_CLAMP_TO_BORDER);
		//constexpr float black[4] = {0,0,0,0};
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
#else
		glTexParameteri(GL_TEXTURE_2D, wrapdir, GL_CLAMP_TO_EDGE);
#endif
	}

	void gl_set_texture_wrap(GLenum wrapdir, ImagePaint::TileMode param) {
		switch (param) {
			case ImagePaint::kClamp_TileMode: // border repeat
				glTexParameteri(GL_TEXTURE_2D, wrapdir, GL_CLAMP_TO_EDGE);
				break;
			case ImagePaint::kRepeat_TileMode: // repeat
				glTexParameteri(GL_TEXTURE_2D, wrapdir, GL_REPEAT);
				break;
			case ImagePaint::kMirror_TileMode: // mirror repeat
				glTexParameteri(GL_TEXTURE_2D, wrapdir, GL_MIRRORED_REPEAT);
				break;
			case ImagePaint::kDecal_TileMode: // no repeat
				gl_set_texture_no_repeat(wrapdir);
				break;
		}
	}

	void gl_set_texture_mag_filter(ImagePaint::FilterMode filter) {
		switch (filter) {
			case ImagePaint::kNearest_FilterMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			case ImagePaint::kLinear_FilterMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
		}
	}

	void gl_set_texture_min_filter(ImagePaint::MipmapMode filter) {
		switch (filter) {
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

	TexStat* gl_new_texture() {
		auto tex = new TexStat{
			0, ImagePaint::kClamp_TileMode, ImagePaint::kClamp_TileMode,
			ImagePaint::kNearest_FilterMode, ImagePaint::kNone_MipmapMode,
		};
		glActiveTexture(GL_TEXTURE0);
		glGenTextures(1, &tex->id);
		glBindTexture(GL_TEXTURE_2D, tex->id);
		gl_set_texture_wrap(GL_TEXTURE_WRAP_S, tex->tileModeX);
		gl_set_texture_wrap(GL_TEXTURE_WRAP_T, tex->tileModeY);
		gl_set_texture_mag_filter(tex->filterMode);
		gl_set_texture_min_filter(tex->mipmapMode);
		return tex;
	}

	void gl_tex_image2D_null(GLuint tex, Vec2 size, GLint iformat, GLenum type, GLuint slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, tex);
		// glBindBuffer(GL_PIXEL_UNPACK_BUFFER, readBuffer);
		// glTexStorage2D(GL_TEXTURE_2D, 1, iformat, size[0], size[1]);
		glTexImage2D(GL_TEXTURE_2D, 0/*level*/, iformat, size[0], size[1], 0, iformat, type, nullptr);
	}

	void gl_set_framebuffer_renderbuffer(GLuint buff, Vec2 size, GLenum iformat, GLenum attachment) {
		glBindRenderbuffer(GL_RENDERBUFFER, buff);
		// glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaa, iformat, size[0], size[1]):
		glRenderbufferStorage(GL_RENDERBUFFER, iformat, size[0], size[1]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buff);
	}

	void gl_set_color_renderbuffer(GLuint rbo, TexStat *t_rbo, ColorType type, Vec2 size) {
		if (t_rbo) {
			// use texture render buffer
			gl_tex_image2D_null(t_rbo->id, size, gl_get_texture_pixel_format(type), gl_get_texture_data_type(type), 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t_rbo->id, 0);
			//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, t_rbo->id, 0);
		} else {
			GLenum ifo;
			switch (type) {
				case kRGB_565_ColorType: ifo = GL_RGB565; break;
				case kRGBA_8888_ColorType: ifo = GL_RGBA8; break;
				case kRGB_888X_ColorType: ifo = GL_RGBA8; break;
				case kRGBA_1010102_ColorType: ifo = GL_RGB10_A2; break;
				case kRGB_101010X_ColorType: ifo = GL_RGB10_A2; break;
				default: ifo = GL_RGBA8; break;
			}
			gl_set_framebuffer_renderbuffer(rbo, size, ifo, GL_COLOR_ATTACHMENT0);
		}
	}

	void gl_set_blur_renderbuffer(GLuint tex, Vec2 size) {
		gl_tex_image2D_null(tex, size,
			gl_get_texture_pixel_format(kRGBA_8888_ColorType),
			gl_get_texture_data_type(kRGBA_8888_ColorType), 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);
#if DEBUG
		int texDims;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 4, GL_TEXTURE_WIDTH, &texDims);
		Qk_DLog("glGetTexLevelParameteriv: %d", texDims);
#endif
	}

	void gl_set_aaclip_buffer(GLuint tex, Vec2 size) {
		// clip anti alias buffer
		uint32_t slot = gl_MaxTextureImageUnits - 1;
		gl_tex_image2D_null(tex, size, GL_LUMINANCE, GL_UNSIGNED_BYTE, slot);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex, 0);
	}

	GLRender::GLRender(Options opts)
		: Render(opts)
		, _texStat(new TexStat*[8]{0}), _glcanvas(nullptr)
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

#if DEBUG
		GLint maxTextureSize,maxTextureBufferSize,maxTextureImageUnits;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTextureBufferSize);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
		Qk_DLog("GL_MAX_TEXTURE_SIZE: %d", maxTextureSize);
		Qk_DLog("GL_MAX_TEXTURE_BUFFER_SIZE: %d", maxTextureBufferSize);
		Qk_DLog("GL_MAX_TEXTURE_IMAGE_UNITS: %d", maxTextureImageUnits);
#endif

		if (!gl_MaxTextureImageUnits) {
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_MaxTextureImageUnits);
			gl_MaxTextureImageUnits_GLSL_Macros = 
				String("#define Qk_GL_MAX_TEXTURE_IMAGE_UNITS ") + gl_MaxTextureImageUnits + "\n";
		}

#if DEBUG || QK_MoreLog
		int64_t st = time_micro();
		_shaders.buildAll(); // compile all shaders
		Qk_DLog("shaders.buildAll time: %ld (micro s)", time_micro() - st);
#else
		_shaders.buildAll();
#endif

		// settings shader
		for (auto s = &_shaders.color1, e = s + 2; s < e; s++) {
			glUniformBlockBinding(s->shader, glGetUniformBlockIndex(s->shader, "optsBlock"), 2); // binding = 2
		}
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
		for (auto s = &_shaders.blur, e = s + 5; s < e; s++) {
			glUseProgram(s->shader);
			glUniform1i(s->image, 0);
		}
		glUseProgram(_shaders.vportCp.shader);
		glUniform1i(_shaders.vportCp.image, 0);

		glEnable(GL_BLEND); // enable color blend
		gl_set_blend_mode(kSrcOver_BlendMode); // set default color blend mode
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
		Qk_Fatal_Assert(_glcanvas == nullptr);
	}

	void GLRender::lock() {}
	void GLRender::unlock() {}

	void GLRender::release() {
		GLuint fbo = _fbo, ubo[] = {_rootMatrixBlock,_viewMatrixBlock,_optsBlock};
		auto texStat = _texStat;
		post_message(Cb([fbo,ubo,texStat](auto &e){
			glDeleteFramebuffers(1, &fbo);
			glDeleteBuffers(3, ubo);
			for (int i = 0; i < 8; i++) {
				if (texStat[i])
					glDeleteTextures(1, &texStat[i]->id);
			}
			delete[] texStat;
		}));
		Qk_Assert(_glcanvas->refCount() == 1);
		_glcanvas->release(); _glcanvas = nullptr;
		_canvas = nullptr;
	}

	void GLRender::reload() {
		lock();
		_surfaceSize = getSurfaceSize(&_defaultScale);
		_delegate->onRenderBackendReload({Vec2{0,0},_surfaceSize}, _surfaceSize, _defaultScale);
		unlock();
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

	void GLRender::makeTexture(cPixel *pix, TexStat *&out, bool isMipmap) {
		if ( pix->body().length() == 0 )
			return;

		ColorType type = pix->type();
		GLint iformat = gl_get_texture_pixel_format(type);
		Qk_Assert(iformat);

		if (!iformat)
			return;

		if (!out) {
			out = gl_new_texture();
		} else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, out->id);
		}

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

		if ( type >= kPVRTCI_2BPP_RGB_ColorType ) {
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_level - 1);
			glCompressedTexImage2D(GL_TEXTURE_2D, 0/*level*/, iformat,
														pix->width(),
														pix->height(), 0/*border*/, pix->body().length(), *pix->body());
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0/*level*/, iformat,
									pix->width(),
									pix->height(), 0/*border*/, iformat/*format*/,
									gl_get_texture_data_type(type)/*type*/, *pix->body());
			if (isMipmap) {
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}
	}

	void GLRender::deleteTexture(TexStat *tex) {
		glDeleteTextures(1, &tex->id);
		delete tex;
	}

	void GLRender::gl_set_blend_mode(BlendMode mode) {
		switch (mode) {
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
		_blendMode = mode;
	}

	bool GLRender::gl_set_texture(ImageSource *src, int slot, const ImagePaint *paint) {
		auto index = paint->srcIndex + slot;
		auto pixel = &src->pixels()[index];
		auto tex = const_cast<TexStat *>(src->texture_Rt(index));
		if (!tex) {
			Qk_Assert(slot < 8);
			GLRender::makeTexture(pixel, _texStat[slot], true);
			tex = _texStat[slot];
			if (!tex) {
				Qk_DLog("setTexturePixel() Fail");
				return false;
			}
		} else if (src->render() != this) {
			// Spanning multiple backends is not supported for the time being.
			Qk_DLog("setTexturePixel() Spanning multiple backends is not supported for the time being.");
			return false;
		}
		gl_set_texture_param(tex, slot, paint);
		return true;
	}

	void GLRender::gl_set_texture_param(TexStat *tex, uint32_t slot, const ImagePaint* paint) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, tex->id);
		if (tex->tileModeX != paint->tileModeX) {
			gl_set_texture_wrap(GL_TEXTURE_WRAP_S, paint->tileModeX);
			tex->tileModeX = paint->tileModeX;
		}
		if (tex->tileModeY != paint->tileModeY) {
			gl_set_texture_wrap(GL_TEXTURE_WRAP_T, paint->tileModeY);
			tex->tileModeY = paint->tileModeY;
		}
		if (tex->filterMode != paint->filterMode) {
			gl_set_texture_mag_filter(paint->filterMode);
			tex->filterMode = paint->filterMode;
		}
		if (tex->mipmapMode != paint->mipmapMode) {
			gl_set_texture_min_filter(paint->mipmapMode);
			tex->mipmapMode = paint->mipmapMode;
		}
	}

	Canvas* GLRender::newCanvas(Options opts) {
#if Qk_USE_GLC_CMD_QUEUE
		opts.colorType = opts.colorType ? opts.colorType: kRGBA_8888_ColorType;
		return new GLCanvas(this, opts);
#else
		return nullptr;
#endif
	}

}
