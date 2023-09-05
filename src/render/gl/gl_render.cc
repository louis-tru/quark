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

	String GL_MaxTextureImageUnits_Str;
	uint32_t GL_MaxTextureImageUnits;

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

	void gl_set_texture(GLuint id, uint32_t slot, const ImagePaintBase* paint) {
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
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
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
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
		, _stencilRef(0), _stencilRefDecr(0)
		, _frameBuffer(0), _msaaFrameBuffer(0)
		, _renderBuffer(0), _msaaRenderBuffer(0), _stencilBuffer(0), _depthBuffer(0)
		, _clipAAAlphaBuffer(0)
		, _texBuffer{0,0,0}
		, _zDepth(0)
		, _glCanvas(this, opts.isMultiThreading)
		, _shaders{
			&_clear, &_clip, &_color, &_color1, &_image, &_imageMask, &_linear, &_radial, &_imageYuv
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
			_opts.msaa = 0;
		}
		_canvas = &_glCanvas; // set default canvas

		// Create the framebuffer and bind it so that future OpenGL ES framebuffer commands are directed to it.
		glGenFramebuffers(2, &_frameBuffer); // _frame_buffer,_msaa_frame_buffer
		// Create a color renderbuffer, allocate storage for it, and attach it to the framebuffer.
		glGenRenderbuffers(4, &_renderBuffer); // _render_buffer,_msaa_render_buffer,_stencil_buffer,_depth_buffer
		// Create aa texture buffer
		glGenTextures(1, &_clipAAAlphaBuffer); // _clipAlphaBuffer

		glGenBuffers(3, &_rootMatrixBlock); // _matrixBlock, _viewMatrixBlock, _optsBlock
		glBindBuffer(GL_UNIFORM_BUFFER, _rootMatrixBlock);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, _rootMatrixBlock);
		// _viewMatrixBlock
		glBindBuffer(GL_UNIFORM_BUFFER, _viewMatrixBlock);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, _viewMatrixBlock);
		// _optsBlock
		glBindBuffer(GL_UNIFORM_BUFFER, _optsBlock);
		glBindBufferBase(GL_UNIFORM_BUFFER, 2, _optsBlock);

		// get consts
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
		glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &_maxTextureBufferSize);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &_maxTextureImageUnits);
		_maxTextureImageUnits -= 1; // aaalpha
		GL_MaxTextureImageUnits = _maxTextureImageUnits;
		GL_MaxTextureImageUnits_Str = _maxTextureImageUnits;

		// compile the shader
		for (auto shader: _shaders) {
			shader->build();
		}
		glUseProgram(_image.shader);
		glUniform1i(_image.image, 0); // set texture slot

		glUseProgram(_imageYuv.shader);
		glUniform1i(_imageYuv.image, 0); // set texture slot
		glUniform1i(_imageYuv.image_u, 1);
		glUniform1i(_imageYuv.image_v, 2);

		glUseProgram(_imageMask.shader);
		glUniform1i(_imageMask.image, 0); // set texture slot

		glUseProgram(_clip.shader);
		glUniform1i(_clip.aaalpha, _maxTextureImageUnits); // set texture slot

		glUseProgram(_color1.shader);
		GLuint optsBlock = glGetUniformBlockIndex(_color1.shader, "optsBlock");
		glUniformBlockBinding(_color1.shader, optsBlock, 2); // binding = 2

		glEnable(GL_BLEND); // enable color blend
		setBlendMode(kSrcOver_BlendMode); // set default color blend mode
		// enable and disable test function
		glClearStencil(127);
		glStencilMask(0xFFFFFFFF);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE); // enable color
		glDisable(GL_STENCIL_TEST); // disable stencil test
		// set depth test
		glEnable(GL_DEPTH_TEST); // enable depth test
		// glDisable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER); // passes if depth is greater than the stored depth.
		glClearDepth(0.0f); // set depth clear value to -1.0
	}

	GLRender::~GLRender() {
		glDeleteFramebuffers(2, &_frameBuffer); // _frameBuffer,_msaaFrameBuffer
		glDeleteRenderbuffers(4, &_renderBuffer); // _renderBuffer,_msaaRenderBuffer,_stencilBuffer,_depthBuffer
		glDeleteTextures(4, &_clipAAAlphaBuffer); // _clipAAAlphaBuffer, _texBuffer
		glDeleteBuffers(3, &_rootMatrixBlock); // _rootMatrixBlock, _viewMatrixBlock, _optsBlock
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

		glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer); // bind frame buffer
		setMainRenderBuffer(w, h);

		if (_opts.msaa > 1 && !_IsDeviceMsaa) {
			glBindFramebuffer(GL_FRAMEBUFFER, _msaaFrameBuffer);

			do { // enable multisampling
				setMSAARenderBuffer(w, h, _opts.msaa);
				// Test the framebuffer for completeness.
				if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE ) {
					if ( _opts.msaa > 1 )
						_IsDeviceMsaa = true;
					break;
				}
				_opts.msaa >>= 1;
			} while (_opts.msaa > 1);
		}

		if (!_IsDeviceMsaa) { // no device msaa
			glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
		}
		setDepthStencilBuffer(w, h, _opts.msaa);
		setClipAABuffer(w, h, _opts.msaa);

		const GLenum buffers[]{ GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, buffers);

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

		_glCanvas.onSurfaceReload(surfaceScale);
		setRootMatrix(mat);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void GLRender::setRootMatrix(const Mat4& root) {
		// update shader root matrix and clear all save state
		auto m4x4 = root.transpose(); // transpose matrix
		glBindBuffer(GL_UNIFORM_BUFFER, _rootMatrixBlock);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(float) * 16, m4x4.val, GL_STREAM_DRAW);
		glClear(GL_STENCIL_BUFFER_BIT); // clear stencil buffer
		glDisable(GL_STENCIL_TEST); // disable stencil test

		if (!_IsDeviceMsaa) { // clear clip aa buffer
			const float color[] = {0.0f,0.0f,0.0f,0.0f};
			glClearBufferfv(GL_COLOR, 1, color); // clear GL_COLOR_ATTACHMENT1
		}
	}

	void GLRender::setMainRenderBuffer(int width, int height) {
		glBindRenderbuffer(GL_RENDERBUFFER, _renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, gl_pixel_internal_format(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _renderBuffer);
	}

	void GLRender::setMSAARenderBuffer(int width, int height, int msaaSample) {
		glBindRenderbuffer(GL_RENDERBUFFER, _msaaRenderBuffer); // render buffer
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaSample, gl_pixel_internal_format(_opts.colorType), width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _msaaRenderBuffer);
	}

	void GLRender::setDepthStencilBuffer(int width, int height, int msaaSample) { // set buffers
		// depth buffer
		glBindRenderbuffer(GL_RENDERBUFFER, _depthBuffer); // set depth buffer
		msaaSample > 1 ?
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaSample, GL_DEPTH_COMPONENT24, width, height):
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthBuffer);
		// clip stencil buffer
		glBindRenderbuffer(GL_RENDERBUFFER, _stencilBuffer);
		msaaSample > 1?
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, msaaSample, GL_STENCIL_INDEX8, width, height):
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _stencilBuffer);
	}

	void GLRender::setClipAABuffer(int width, int height, int msaaSample) {
		// clip anti alias buffer
		if (msaaSample <= 1) {
			glActiveTexture(GL_TEXTURE15); // 15 only use on aa alpha
			glBindTexture(GL_TEXTURE_2D, _clipAAAlphaBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0/*level*/, GL_ALPHA/*internalformat*/,
									width, height, 0/*border*/, GL_ALPHA/*format*/, GL_UNSIGNED_BYTE/*type*/, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, _clipAAAlphaBuffer, 0);
		}
	}

	uint32_t GLRender::makeTexture(cPixel *src, uint32_t id) {
		return gl_gen_texture(src, id, true);
	}

	void GLRender::deleteTextures(const uint32_t *ids, uint32_t count) {
		glDeleteTextures(count, ids);
	}

	void GLRender::makeVertexData(VertexData::ID *id) {
		if (!id->vao) {
			auto &vertex = id->ref->vertex;
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

	void GLRender::setTexture(cPixel *pixel, int slot, const ImagePaintBase *paint) {
		auto id = pixel->texture();
		if (!id) {
			id = gl_gen_texture(pixel, _texBuffer[slot], true);
			if (!id) {
				Qk_DEBUG("setTexturePixel() fail"); return;
			}
			_texBuffer[slot] = id;
		}
		gl_set_texture(id, slot, paint);
	}

	void GLRender::setBlendMode(BlendMode mode) {
		_blendMode = mode;
		gl_set_blend_mode(mode);
	}

}
