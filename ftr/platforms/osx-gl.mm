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

// ******************** U n r e a l i z e d ********************

#import "osx-gl-1.h"
#import "ftr/app.h"
#import "ftr/display-port.h"
#import "ftr/sys.h"
#import <OpenGL/gl.h>

#define UIScreen NSScreen

namespace ftr {

	/**
	* @class MyGLDraw
	*/
	template<class Basic> class MyGLDraw: public Basic {
		public:
			MyGLDraw(GUIApplication* host,
							DrawLibrary library,
							cJSON& options): Basic(host, options), proxy_(this) {
				this->_library = library;
			}
			virtual void begin_render() {
				proxy_.commit_render();
			}
			virtual void commit_render() {
				proxy_.commit_render();
			}
			virtual GLint get_gl_texture_pixel_format(PixelData::Format pixel_format) {
				return proxy_.get_gl_texture_pixel_format(pixel_format);
			}
			inline GLDrawProxy* proxy() { return &proxy_; }
			
		private:
			GLDrawProxy proxy_;
	};

	GLDrawProxy* GLDrawProxy::create(GUIApplication* host, cJSON& options) {
		GLDrawProxy* rv = nullptr;
		rv = (new MyGLDraw<GLDraw>(host, DRAW_LIBRARY_GL3, options))->proxy();
		return rv;
	}

	GLDrawProxy::GLDrawProxy(GLDraw* host): _host(host), _context(nil) {
	}

	GLDrawProxy::~GLDrawProxy() {
		[NSOpenGLContext clearCurrentContext];
	}

	void GLDrawProxy::initialize(UIView* view, NSOpenGLContext* ctx) {
		[ctx makeCurrentContext];
		
		GLint major, minor;
		NSOpenGLGetVersion(&major, &minor);
		LOG("NSOpenGLGetVersion: %d, %d", major, minor);
		
		const GLubyte * name = glGetString(GL_VENDOR);
		const GLubyte * biaoshifu = glGetString(GL_RENDERER);
		const GLubyte * OpenGLVersion = glGetString(GL_VERSION);
		LOG("%s, %s, %s", name, biaoshifu, OpenGLVersion);
		
		_surface_view = view;
		_context = ctx;
		_host->initialize();
		_host->set_best_display_scale(UIScreen.mainScreen.backingScaleFactor);
	}

	void GLDrawProxy::begin_render() {
		// Add your drawing codes here
		[_context makeCurrentContext];
		// must lock GL context because display link is threaded
		CGLLockContext(_context.CGLContextObj);
		_host->GLDraw::begin_render();
	}

	void GLDrawProxy::commit_render() {
		_host->GLDraw::commit_render();
		[_context flushBuffer];
		CGLUnlockContext(_context.CGLContextObj);
	}

	/**
	* @func get_gl_texture_pixel_format 获取当前环境对应的OpenGL纹理像素格式,如果返回0表示不支持纹理格式
	*/
	GLint GLDrawProxy::get_gl_texture_pixel_format(PixelData::Format pixel_format) {
		switch (pixel_format) {
			case PixelData::RGBA4444:
			case PixelData::RGBX4444:
			case PixelData::RGBA5551:
			case PixelData::RGBA8888:
			case PixelData::RGBX8888: return GL_RGBA;
			case PixelData::RGB565:
			case PixelData::RGB888: return GL_RGB;
			case PixelData::ALPHA8: return GL_ALPHA;
			case PixelData::LUMINANCE8: return GL_LUMINANCE;
			case PixelData::LUMINANCE_ALPHA88: return GL_LUMINANCE_ALPHA;
			// compressd texture
	//		case PixelData::PVRTCI_2BPP_RGB: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
	//		case PixelData::PVRTCI_2BPP_RGBA:
	//		case PixelData::PVRTCII_2BPP: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
	//		case PixelData::PVRTCI_4BPP_RGB: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
	//		case PixelData::PVRTCI_4BPP_RGBA:
	//		case PixelData::PVRTCII_4BPP: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
	//		case PixelData::ETC1:
	//		case PixelData::ETC2_RGB: return GL_COMPRESSED_RGB8_ETC2;
	//		case PixelData::ETC2_RGB_A1:
	//		case PixelData::ETC2_RGBA: return GL_COMPRESSED_RGBA8_ETC2_EAC;
			default: return 0;
		}
	}

	bool GLDrawProxy::refresh_surface_size(::CGRect rect) {
		float scale = UIScreen.mainScreen.backingScaleFactor;
		Vec2 size(rect.size.width * scale, rect.size.height * scale);
		if ( !size.is_zero() ) {
			return _host->set_surface_size(size);
		}
		return false;
	}

}
