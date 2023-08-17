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

	GLint gl_get_texture_pixel_format(ColorType type) {
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

	GLint gl_get_texture_data_format(ColorType format) {
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

	uint32_t gl_pixel_internal_format(ColorType type) {
		switch (type) {
			case kColor_Type_RGB_565: return GL_RGB565;
			case kColor_Type_RGBA_8888: return GL_RGBA8;
			case kColor_Type_RGB_888X: return GL_RGBA8;
			case kColor_Type_RGBA_1010102: return GL_RGB10_A2;
			case kColor_Type_RGB_101010X: return GL_RGB10_A2;
			default: return 0;
		}
	}

	uint32_t gl_gen_texture(cPixel* src, GLuint id, bool genMipmap) {
		if ( src->body().length() == 0 )
			return 0;

		ColorType type = src->type();
		GLint internalformat = gl_get_texture_pixel_format(type);
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
									gl_get_texture_data_format(type)/*type*/, *src->body());
			if (genMipmap) {
				glGenerateMipmap(GL_TEXTURE_2D);
			}
		}

		//constexpr float block[4] = {0,0,0,0};
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, block);

		return id;
	}

	void gl_set_texture(GLuint id, uint32_t slot, const Paint& paint) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, id);

		switch (paint.tileModeX) {
			case Paint::kClamp_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				break;
			case Paint::kRepeat_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				break;
			case Paint::kMirror_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				break;
			case Paint::kDecal_TileMode: // no repeat
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				break;
		}

		switch (paint.tileModeY) {
			case Paint::kClamp_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				break;
			case Paint::kRepeat_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				break;
			case Paint::kMirror_TileMode:
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				break;
			case Paint::kDecal_TileMode: // no repeat
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
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

	void gl_set_blend_mode(BlendMode blendMode) {
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
	}

	bool gl_read_pixels(Pixel* dst, uint32_t srcX, uint32_t srcY) {
		GLenum format = gl_get_texture_pixel_format(dst->type());
		GLenum type = gl_get_texture_data_format(dst->type());
		if (format && dst->bytes() != dst->body().size())
			return false;
		glReadPixels(srcX, srcY, dst->width(), dst->height(), format, type, *dst->body());
		return true;
	}

	bool gl_is_support_multisampled() {
		String VENDOR = (const char*)glGetString(GL_VENDOR);
		String RENDERER = (const char*)glGetString(GL_RENDERER);
		String version = (const char*)glGetString(GL_VERSION);
		String extensions = (const char*)glGetString(GL_EXTENSIONS);

		Qk_DEBUG("OGL VENDOR: %s", *VENDOR);
		Qk_DEBUG("OGL RENDERER: %s", *RENDERER);
		Qk_DEBUG("OGL VERSION: %s", *version);
		Qk_DEBUG("OGL EXTENSIONS: %s", *extensions);

		auto str = String::format("%s %s %s %s", *VENDOR, *RENDERER, *version, *extensions);

		for (auto s : {"OpenGL ES", "OpenGL", "OpenGL Entity"}) {
			int idx = str.indexOf(s);
			if (idx >= 0) {
				int num = str.substr(idx + strlen(s)).trim().substr(0,1).toNumber<int>();
				if (num > 2)
					return true;
				else if (extensions.indexOf( "multisample" ) >= 0)
					return false;
			}
		}

		return version.indexOf("Metal") >= 0;
	}

	GLRender::GLRender(Options opts)
		: Render(opts)
		, _IsSupportMultisampled(gl_is_support_multisampled())
		, _IsDeviceMsaa(false)
		, _blendMode(kClear_BlendMode)
		, _frame_buffer(0), _msaa_frame_buffer(0)
		, _render_buffer(0), _msaa_render_buffer(0), _stencil_buffer(0), _depth_buffer(0)
		, _tex_buffer{0,0,0}
		, _mainCanvas(this)
		, _shaders{
			&_clear, &_clip, &_color, &_image, &_colorMask, &_yuv420p,
			&_yuv420sp, &_linear, &_radial,
			&_colorSdf, &_colorMaskSdf, &_linearSdf, &_radialSdf,&_imageSdf,
		}
	{
		switch(_opts.colorType) {
			case kColor_Type_BGRA_8888:
				_opts.colorType = kColor_Type_RGBA_8888; break;
			case kColor_Type_BGRA_1010102:
				_opts.colorType = kColor_Type_RGBA_1010102; break;
			case kColor_Type_BGR_101010X:
				_opts.colorType = kColor_Type_RGB_101010X; break;
			default: break;
		}

		if (!_IsSupportMultisampled) {
			_opts.msaaSampleCnt = 0;
		}
		_canvas = &_mainCanvas; // set default canvas

		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(2, &_frame_buffer); // _frame_buffer,_msaa_frame_buffer
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(4, &_render_buffer); // _render_buffer,_msaa_render_buffer,_stencil_buffer,_depth_buffer

		setBlendMode(kSrcOver_BlendMode); // set default color blend mode

		// compile the shader
		for (auto shader: _shaders) {
			shader->build();
		}

		glUseProgram(_image.shader);
		glUniform1i(_image.image, 0); // set texture slot

		glUseProgram(_colorMask.shader);
		glUniform1i(_colorMask.image, 0); // set texture slot

		glUseProgram(_yuv420p.shader);
		glUniform1i(_yuv420p.image, 0); // set texture slot
		glUniform1i(_yuv420p.image_u, 1);
		glUniform1i(_yuv420p.image_v, 2);

		glUseProgram(_yuv420sp.shader);
		glUniform1i(_yuv420sp.image, 0); // set texture slot
		glUniform1i(_yuv420sp.image_uv, 1);

		glUseProgram(_colorMaskSdf.shader);
		glUniform1i(_colorMaskSdf.image, 0); // set texture slot

		glUseProgram(0);

		glEnable(GL_BLEND); // enable color blend
		// enable and disable test function
		glClearStencil(127);
		glStencilMask(0xFFFFFFFF);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); // enable color
		glDisable(GL_STENCIL_TEST); // disable stencil test
		// set depth test
		//glDisable(GL_DEPTH_TEST); // disable depth test
		glEnable(GL_DEPTH_TEST); // enable depth test
		glDepthFunc(GL_GREATER); // passes if depth is greater than the stored depth.
		glClearDepth(0.0f); // set depth clear value
	}

	GLRender::~GLRender() {
		glDeleteFramebuffers(2, &_frame_buffer); // _frame_buffer,_msaa_frame_buffer
		glDeleteRenderbuffers(4, &_render_buffer); // _render_buffer,_msaa_render_buffer,_stencil_buffer,_depth_buffer
		glDeleteTextures(3, _tex_buffer);
	}

	void GLRender::reload() {
		auto size = getSurfaceSize();
		Mat4 mat;
		Vec2 surfaceScale;
		if (!_delegate->onRenderBackendReload({Vec2{0,0},size}, size, getDefaultScale(), &mat, &surfaceScale))
			return;

		auto w = size.x(), h = size.y();

		Qk_ASSERT(w, "Invalid viewport size width");
		Qk_ASSERT(h, "Invalid viewport size height");

		glViewport(0, 0, w, h);

		glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer); // bind frame buffer
		setRenderBuffer(w, h);

		if (_opts.msaaSampleCnt > 1 && !_IsDeviceMsaa) {
			glBindFramebuffer(GL_FRAMEBUFFER, _msaa_frame_buffer);

			do { // enable multisampling
				setMSAABuffer(w, h, _opts.msaaSampleCnt);
				// Test the framebuffer for completeness.
				if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ) {
					if ( _opts.msaaSampleCnt > 1 )
						_IsDeviceMsaa = true;
					break;
				}
				_opts.msaaSampleCnt >>= 1;
			} while (_opts.msaaSampleCnt > 1);
		}

		if (!_IsDeviceMsaa) { // no device msaa
			glBindFramebuffer(GL_FRAMEBUFFER, _frame_buffer);
		}
		setDepthBuffer(w, h, _opts.msaaSampleCnt);
		setStencilBuffer(w, h, _opts.msaaSampleCnt);

		const GLenum buffers[]{ GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, buffers);

		if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE ) {
			Qk_FATAL("failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		}

#if DEBUG
		int width, height;
		// Retrieve the height and width of the color renderbuffer.
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
		glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
		Qk_DEBUG("GL_RENDERBUFFER_WIDTH: %d, GL_RENDERBUFFER_HEIGHT: %d", width, height);
#endif

		_mainCanvas.setRootMatrix(mat, surfaceScale);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void GLRender::setRenderBuffer(int width, int height) {
		glBindRenderbuffer(GL_RENDERBUFFER, _render_buffer);
		glRenderbufferStorage(GL_RENDERBUFFER, gl_pixel_internal_format(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _render_buffer);
	}

	void GLRender::setMSAABuffer(int width, int height, int MSAASample) {
		glBindRenderbuffer(GL_RENDERBUFFER, _msaa_render_buffer); // render buffer
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASample, gl_pixel_internal_format(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaa_render_buffer);
	}

	void GLRender::setStencilBuffer(int width, int height, int MSAASample) { // set clip stencil buffer
		glBindRenderbuffer(GL_RENDERBUFFER, _stencil_buffer);
		MSAASample > 1?
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASample, GL_STENCIL_INDEX8, width, height):
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _stencil_buffer);
	}

	void GLRender::setDepthBuffer(int width, int height, int MSAASample) {
		glBindRenderbuffer(GL_RENDERBUFFER, _depth_buffer); // set depth buffer
		MSAASample > 1 ?
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAASample, GL_DEPTH_COMPONENT24, width, height):
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_buffer);
	}

	uint32_t GLRender::makeTexture(cPixel *src, uint32_t id) {
		return gl_gen_texture(src, id, true);
	}

	void GLRender::deleteTextures(const uint32_t *IDs, uint32_t count) {
		glDeleteTextures(count, IDs);
	}

	static void setVertexAttribPointer(VertexData *data) {
		glBindVertexArray(data->vao);
		if (data->items == 2) { // vec2 vertex
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*2, (const GLvoid*)0); // xy
			glEnableVertexAttribArray(0);
		} else { // vec3 sdf vertex
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float)*3, (const GLvoid*)0); // xy
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float)*3, (const GLvoid*)(sizeof(float)*2)); // sdf
			glEnableVertexAttribArray(1);
		}
		data->vertex.clear(); // clear cpu memory data
		glBindVertexArray(0);
	}

	void GLRender::makeVertexData(VertexData *data) {
		if (!data->vao && data->vertex.length()) {
			glGenVertexArrays(1, &data->vao);
			glGenBuffers(1, &data->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
			glBufferData(GL_ARRAY_BUFFER, data->vertex.size(), *data->vertex, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
			setVertexAttribPointer(data);
		}
	}

	void GLRender::deleteVertexData(const VertexData &data) {
		if (data.vao)
			glDeleteVertexArrays(1, &data.vao);
		if (data.vbo)
			glDeleteBuffers(1, &data.vbo);
	}

	void GLRender::copyVertexData(const VertexData &src, VertexData *dest) {
		if (src.vao && src.count) {
			dest->count = src.count;
			dest->items = src.items;
			auto size = src.count * src.items * sizeof(float);
			glGenVertexArrays(1, &dest->vao);
			glGenBuffers(1, &dest->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, dest->vbo);
			glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW
			glBindBuffer(GL_COPY_READ_BUFFER, src.vbo);
			glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, size);
			setVertexAttribPointer(dest);
		}
	}

	void GLRender::setBlendMode(BlendMode blendMode) {
		if (_blendMode != blendMode) {
			gl_set_blend_mode(blendMode);
			_blendMode = blendMode;
		}
	}

	void GLRender::setTexture(cPixel *pixel, int slot, const Paint &paint) {
		auto id = pixel->texture();
		if (!id) {
			id = gl_gen_texture(pixel, _tex_buffer[slot], true);
			if (!id) {
				Qk_DEBUG("setTexturePixel() fail"); return;
			}
			_tex_buffer[slot] = id;
		}
		gl_set_texture(id, slot, paint);
	}

}
