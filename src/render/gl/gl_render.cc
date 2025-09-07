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
	String gl_Global_GLSL_Macros;
	int    gl_MaxTextureImageUnits = 0;

	void gl_CheckFramebufferStatus(GLenum target) {
		auto status = glCheckFramebufferStatus(target);
		const char* errorReason = "Unknown";
		switch (glCheckFramebufferStatus(target)) {
			case GL_FRAMEBUFFER_COMPLETE: 
				// errorReason = "Success"; 
				return;
			case GL_FRAMEBUFFER_UNDEFINED: 
				errorReason = "Framebuffer undefined"; 
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: 
				errorReason = "Incomplete attachment"; 
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: 
				errorReason = "No valid attachments"; 
				break;
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
			case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
				errorReason = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS"; 
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: 
				errorReason = "Draw buffer incomplete"; 
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				errorReason = "Read buffer incomplete"; 
				break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
			case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
				errorReason = "Layer targets incomplete"; 
				break;
#endif
			case GL_FRAMEBUFFER_UNSUPPORTED: 
				errorReason = "Unsupported format combination"; 
				break;
			case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: 
				errorReason = "Mismatched multisample settings"; 
				break;
			default: 
				errorReason = "Unknown error";
		}
		Qk_Fatal("failed to make complete framebuffer object, %s, %x", errorReason, status);
	}

	void gl_texture_barrier() {
#if defined(GL_ARB_texture_barrier)
		glTextureBarrier();
#elif defined(GL_NV_texture_barrier)
		glTextureBarrierNV();
#elif Qk_MacOS
		glFlushRenderAPPLE();
#else
		glFlush();
#endif
	}

	GLint gl_get_texture_pixel_format(ColorType type) {
 #ifndef GL_LUMINANCE
 # define GL_LUMINANCE       GL_RED
 # define GL_LUMINANCE_ALPHA GL_RG
 #endif
		switch (type) {
			case kAlpha_8_ColorType: return GL_ALPHA;
			case kRGB_565_ColorType: return GL_RGB;
			case kRGBA_4444_ColorType: return GL_RGBA;
			case kRGB_444X_ColorType: return GL_RGBA;
			case kRGBA_8888_ColorType: return GL_RGBA;
			case kRGB_888X_ColorType: return GL_RGBA;
			case kRGBA_1010102_ColorType: return GL_RGBA;
			case kRGB_101010X_ColorType: return GL_RGBA;
			case kRGB_888_ColorType: return GL_RGB;
			case kRGBA_5551_ColorType: return GL_RGBA;
			// TODO: Grayscale images may not display properly for macos
			case kLuminance_8_ColorType: return GL_LUMINANCE;
			case kLuminance_Alpha_88_ColorType: return GL_LUMINANCE_ALPHA;
			// case kSDF_Float_ColorType: return GL_RGBA;
#if Qk_LINUX || Qk_ANDROID
			case kYUV420P_Y_8_ColorType: return GL_LUMINANCE;
			case kYUV420P_U_8_ColorType: return GL_LUMINANCE;
			case kYUV420SP_UV_88_ColorType: return GL_LUMINANCE_ALPHA;
#else
			case kYUV420P_Y_8_ColorType: return GL_RED;
			case kYUV420P_U_8_ColorType: return GL_RED;
			case kYUV420SP_UV_88_ColorType: return GL_RG;
#endif
#if Qk_iOS // ios
			// compressd texture
			case kPVRTCI_2BPP_RGB_ColorType: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			case kPVRTCI_2BPP_RGBA_ColorType:
			case kPVRTCII_2BPP_ColorType: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
			case kPVRTCI_4BPP_RGB_ColorType: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			case kPVRTCI_4BPP_RGBA_ColorType:
			case kPVRTCII_4BPP_ColorType: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
#endif
#if !Qk_MacOS
			case kETC1_ColorType:
			case kETC2_RGB_ColorType: return GL_COMPRESSED_RGB8_ETC2;
			case kETC2_RGBA_ColorType: return GL_COMPRESSED_RGBA8_ETC2_EAC;
			case kETC2_RGB_A1_ColorType: return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
#endif
			default: return 0;
		}
	}

	GLint gl_get_texture_data_type(ColorType format) {
#ifndef GL_UNSIGNED_INT_10_10_10_2
# define GL_UNSIGNED_INT_10_10_10_2 0x8036
#endif
		switch (format) {
			case kAlpha_8_ColorType:      return GL_UNSIGNED_BYTE;
			case kRGB_565_ColorType:      return GL_UNSIGNED_SHORT_5_6_5;
			case kRGBA_4444_ColorType:    return GL_UNSIGNED_SHORT_4_4_4_4;
			case kRGB_444X_ColorType:     return GL_UNSIGNED_SHORT_4_4_4_4;
			case kRGBA_8888_ColorType:    return GL_UNSIGNED_BYTE;
			case kRGB_888X_ColorType:     return GL_UNSIGNED_BYTE;
			case kRGBA_1010102_ColorType: return GL_UNSIGNED_INT_10_10_10_2;
			case kRGB_101010X_ColorType:  return GL_UNSIGNED_INT_10_10_10_2;
			case kRGB_888_ColorType:      return GL_UNSIGNED_BYTE;
			case kRGBA_5551_ColorType:    return GL_UNSIGNED_SHORT_5_5_5_1;
			default:                      return GL_UNSIGNED_BYTE;
		}
	}

	void gl_set_texture_no_repeat(GLenum wrapdir) {
#ifdef GL_CLAMP_TO_BORDER
		glTexParameteri(GL_TEXTURE_2D, wrapdir, GL_CLAMP_TO_BORDER);
		//constexpr float black[4] = {0,0,0,0}; // system default value the Zero
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, black);
#else
		glTexParameteri(GL_TEXTURE_2D, wrapdir, GL_CLAMP_TO_EDGE);
#endif
	}

	void gl_set_sampler_wrap(GLuint sampler, GLenum wrapdir, ImagePaint::TileMode param) {
		switch (param) {
			case ImagePaint::kClamp_TileMode: // border repeat
				glSamplerParameteri(sampler, wrapdir, GL_CLAMP_TO_EDGE);
				break;
			case ImagePaint::kRepeat_TileMode: // repeat
				glSamplerParameteri(sampler, wrapdir, GL_REPEAT);
				break;
			case ImagePaint::kMirror_TileMode: // mirror repeat
				glSamplerParameteri(sampler, wrapdir, GL_MIRRORED_REPEAT);
				break;
			case ImagePaint::kDecal_TileMode: // no repeat
#ifdef GL_CLAMP_TO_BORDER
				glSamplerParameteri(sampler, wrapdir, GL_CLAMP_TO_BORDER);
				//constexpr float black[4] = {0,0,0,0}; // system default value the Zero
				//glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, black);
#else
				glSamplerParameteri(sampler, wrapdir, GL_CLAMP_TO_EDGE);
#endif
				break;
		}
	}

	void gl_set_sampler_mag_filter(GLuint sampler, ImagePaint::FilterMode filter) {
		switch (filter) {
			case ImagePaint::kNearest_FilterMode:
				glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				break;
			case ImagePaint::kLinear_FilterMode:
				glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				break;
		}
	}

	void gl_set_sampler_min_filter(GLuint sampler, ImagePaint::MipmapMode filter) {
		switch (filter) {
			case ImagePaint::kNone_MipmapMode:
				glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
				break;
			case ImagePaint::kLinearNearest_MipmapMode:
				glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
				break;
			case ImagePaint::kNearestLinear_MipmapMode:
				glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				break;
			case ImagePaint::kLinear_MipmapMode:
				glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				break;
		}
	}

	TexStat* gl_new_tex_stat() {
		auto tex = new TexStat{
			.id=0,
		};
		glGenTextures(1, &tex->id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		return tex;
	}

	bool gl_new_texture(cPixel *pix, int levels, TexStat *&out, bool genMipmap) {
		if ( pix->length() == 0 )
			return false;

		ColorType type = pix->type();
		GLint iformat = gl_get_texture_pixel_format(type);

		if (!iformat)
			return false;

		if (!out) {
			out = gl_new_tex_stat();
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, out->id);

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

		// Qk_DLog("gl_new_texture(), len: %d, p: %p", pix->length(), pix);

		if ( type >= kPVRTCI_2BPP_RGB_ColorType ) {
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap_level - 1);
			for (int i = 0; i < levels; i++) {
				auto it = pix + i;
				glCompressedTexImage2D(GL_TEXTURE_2D, i/*level*/, iformat,
															it->width(),
															it->height(), 0/*border*/, it->body().length(), *it->body());
			}
		} else {
			for (int i = 0; i < levels; i++) {
				auto it = pix + i;
				glTexImage2D(GL_TEXTURE_2D, i/*level*/, iformat,
										it->width(),
										it->height(), 0/*border*/, iformat/*format*/,
										gl_get_texture_data_type(type)/*type*/, *it->body());
			}
			if (levels == 1 && genMipmap) {
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		return true;
	}

	void gl_tex_image2D_null(GLuint tex, Vec2 size, GLint iformat, GLenum type, GLuint slot) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, tex);
		glBindSampler(slot, 0);
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

	void gl_set_color_renderbuffer(GLuint rbo, TexStat *rboTex, ColorType type, Vec2 size) {
		if (rboTex) {
			// use texture render buffer
			gl_tex_image2D_null(rboTex->id, size, gl_get_texture_pixel_format(type), gl_get_texture_data_type(type), 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, rboTex->id, 0);
			//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, rboTex->id, 0);
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

	void gl_set_aaclip_buffer(GLuint tex, Vec2 size) {
		// clip anti alias buffer
		GLuint slot = gl_MaxTextureImageUnits - 1; // Binding go to the last channel
#if Qk_iOS || Qk_LINUX || Qk_ANDROID
		ColorType type = kRGBA_8888_ColorType;
#else
		ColorType type = kLuminance_8_ColorType;
#endif
		gl_tex_image2D_null(tex, size,
			gl_get_texture_pixel_format(type), gl_get_texture_data_type(type), slot
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	}

	void gl_set_tex_renderbuffer(GLuint tex, Vec2 size) {
		gl_tex_image2D_null(tex, size,
			gl_get_texture_pixel_format(kRGBA_8888_ColorType),
			gl_get_texture_data_type(kRGBA_8888_ColorType), 0
		);
		gl_set_texture_no_repeat(GL_TEXTURE_WRAP_S);
		gl_set_texture_no_repeat(GL_TEXTURE_WRAP_T);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glGenerateMipmap(GL_TEXTURE_2D);
#if DEBUG && defined(GL_TEXTURE_WIDTH)
		int texDims;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 4, GL_TEXTURE_WIDTH, &texDims);
		Qk_DLog("glGetTexLevelParameteriv: %d", texDims);
#endif
	}

	GLRender::GLRender(Options opts)
		: Render(opts)
		, _texStat(new TexStat*[8]{0}), _glcanvas(nullptr)
	{
		_glcanvas = new GLCanvas(this, _opts);
		_canvas = _glcanvas; // set default canvas
		_glcanvas->retain(); // retain

		// glGenFramebuffers(1, &_fbo);
		glGenBuffers(3, &_rootMatrixBlock); // _rootMatrixBlock, _viewMatrixBlock, _optsBlock
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
#ifdef GL_MAX_TEXTURE_BUFFER_SIZE
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &maxTextureBufferSize);
#endif
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);

		Qk_DLog("GL_MAX_TEXTURE_SIZE: %d", maxTextureSize);
		Qk_DLog("GL_MAX_TEXTURE_BUFFER_SIZE: %d", maxTextureBufferSize);
		Qk_DLog("GL_MAX_TEXTURE_IMAGE_UNITS: %d", maxTextureImageUnits);
#endif

		if (!gl_MaxTextureImageUnits) {
			glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gl_MaxTextureImageUnits);
			gl_Global_GLSL_Macros = 
				String::format("#define Qk_GL_MAX_TEXTURE_IMAGE_UNITS %d\n", gl_MaxTextureImageUnits);
#if Qk_ANDROID
			gl_Global_GLSL_Macros += "#define Qk_ANDROID\n";
#elif Qk_LINUX
			gl_Global_GLSL_Macros += "#define Qk_LINUX\n";
#endif
		}

		_extensions = (cChar*)glGetString(GL_EXTENSIONS);

		Qk_DLog("GL_VENDOR: %s", glGetString(GL_VENDOR));
		Qk_DLog("GL_RENDERER: %s", glGetString(GL_RENDERER));
		Qk_DLog("GL_VERSION: %s", glGetString(GL_VERSION));
		Qk_DLog("GL_EXTENSIONS:\n%s", *_extensions);

#if DEBUG
		int64_t st = time_micro();
		_shaders.buildAll(); // compile all shaders
		Qk_DLog("shaders.buildAll time: %ld (micro s)", time_micro() - st);
#else
		_shaders.buildAll();
#endif

		// settings shader
		for (auto s = &_shaders.colors, e = s + 2; s < e; s++) {
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
			glUniform1i(s->image, 0); // set texture slot
		}
		for (auto s = &_shaders.triangles, e = s + 2; s < e; s++) {
			glUseProgram(s->shader);
			glUniform1i(s->image, 0); // set texture slot
		}
		glUseProgram(_shaders.vportCp.shader);
		glUniform1i(_shaders.vportCp.image, 0); // set texture slot

		glEnable(GL_BLEND); // enable color blend
		gl_set_blend_mode(kSrcOver_BlendMode); // set default color blend mode
		// enable and disable test function
		glClearStencil(127);
		glStencilMask(0xFFFFFFFF);
		glStencilFunc(GL_LEQUAL, 127, 0xFFFFFFFF); // Equality passes the test
		glDisable(GL_STENCIL_TEST); // disable stencil test
		// glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); // enable color
		// set depth test
		glEnable(GL_DEPTH_TEST); // enable depth test
		glDepthFunc(GL_GREATER); // passes if depth is greater than the stored depth.
		glClearDepthf(0.0f); // set depth clear value to -1.0
	}

	GLRender::~GLRender() {
		Qk_ASSERT_RAW(_glcanvas == nullptr);
	}

	void GLRender::lock() {}
	void GLRender::unlock() {}

	void GLRender::release() {
		GLuint ubo[] = {_rootMatrixBlock,_viewMatrixBlock,_optsBlock};
		auto texStat = _texStat;
		auto texSamplers = std::move(_texSamplers);
		post_message(Cb([ubo,texStat,texSamplers](auto &e) {
			for (int i = 0; i < 8; i++) {
				if (texStat[i]) glDeleteTextures(1, &texStat[i]->id);
			}
			for (auto &i: texSamplers) {
				glDeleteSamplers(1, &i.value);
			}
			delete[] texStat;
			glDeleteBuffers(3, ubo);
		}));
		Qk_ASSERT_EQ(_glcanvas->refCount(), 1);
		_glcanvas->release(); _glcanvas = nullptr;
		_canvas = nullptr;
	}

	void GLRender::reload() {
		lock();
		_surfaceSize = getSurfaceSize();
		_delegate->onRenderBackendReload(_surfaceSize);
		unlock();
	}

	void GLRender::newVertexData(VertexData::ID *id) {
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

	bool GLRender::newTexture(cPixel *pix, int levels, TexStat *&out, bool genMipmap) {
		return gl_new_texture(pix, levels, out, genMipmap);
	}

	void GLRender::deleteTexture(TexStat *tex) {
		glDeleteTextures(1, &tex->id);
		delete tex;
	}

	void GLRender::gl_set_blend_mode(BlendMode mode) {
		switch (mode) {
			case kClear_BlendMode:         //!< r = 0 + (1-sa)*d
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
				// glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
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
			case kMultiply_BlendMode:      //!< r = d*s + (1-sa)*d
				glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
				break;
			case kSrcOverExt_BlendMode: // r = s + (1-sa)*d
				glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				break;
		}
		_blendMode = mode;
	}

	bool GLRender::gl_set_texture(ImageSource *src, int slot, const ImagePaint *paint) {
		Qk_ASSERT(slot < 8);
		src->onState().assertHeld(); // Check mutex lock
		auto index = paint->srcIndex + slot;
		auto tex = const_cast<TexStat *>(src->texture(index));
		if (!tex) {
			auto pixel = src->pixel(index);
			if (!pixel) {
				Qk_DLog("gl_set_texture() Fail %p, reason: src->pixel(%d) == nullptr", src, index);
				return false;
			}
			if (!gl_new_texture(pixel, 1, _texStat[slot], true)) {
				Qk_DLog("gl_set_texture() Fail %p, reason: _texStat[%d] == nullptr", src, slot);
				return false;
			}
			tex = _texStat[slot];
		}
		gl_set_texture_param(tex, slot, paint);
		return true;
	}

	void GLRender::gl_set_texture_param(TexStat *tex, uint32_t slot, const ImagePaint* paint) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, tex->id);
		glBindSampler(slot, gl_get_tex_sampler(paint));
	}

	GLuint GLRender::gl_get_tex_sampler(const ImagePaint* paint) {
		constexpr uint32_t bitfields = (
			// 0 | // src index default zero
			(0b11 << 8)  | // 2 bits
			(0b11 << 10) | // 2 bits
			(0b11 << 12) | // 2 bits
			(0b1  << 14) | // 1 bits
			0
		);
		uint32_t key = bitfields & (*(uint32_t*)(paint));
		GLuint sampler;
		if (!_texSamplers.get(key, sampler)) {
			glGenSamplers(1, &sampler);
			_texSamplers.set(key, sampler);
			gl_set_sampler_wrap(sampler, GL_TEXTURE_WRAP_S, paint->tileModeX);
			gl_set_sampler_wrap(sampler, GL_TEXTURE_WRAP_T, paint->tileModeY);
			gl_set_sampler_mag_filter(sampler, paint->filterMode);
			gl_set_sampler_min_filter(sampler, paint->mipmapMode);
		}
		return sampler;
	}

	Canvas* GLRender::newCanvas(Options opts) {
#if Qk_USE_GLC_CMD_QUEUE
		opts.colorType = opts.colorType ? opts.colorType: kRGBA_8888_ColorType;
		return new GLCanvas(this, opts);
#else
		return nullptr;
#endif
	}

	// ------------------------------------------------

	void GLRenderResource::post_message(Cb cb) {
		_loop->post(cb);
	}

	bool GLRenderResource::newTexture(cPixel *pix, int levels, TexStat *&out, bool genMipmap) {
		if (gl_new_texture(pix, levels, out, genMipmap))
			return glFlush(), true;
		return false;
	}

	void GLRenderResource::deleteTexture(TexStat *tex) {
		glDeleteTextures(1, &tex->id);
		glFlush();
		delete tex;
	}

}
