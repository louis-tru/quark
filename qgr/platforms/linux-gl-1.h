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

#ifndef __qgr__linux_gl_1__
#define __qgr__linux_gl_1__

#include "qgr/utils/macros.h"

#if XX_LINUX || XX_ANDROID

#include "qgr/gl/gl.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

XX_NS(qgr)

class LinuxGLDrawProxy {
 public:
	LinuxGLDrawProxy(GLDraw* host, EGLDisplay display, EGLConfig cfg, EGLContext ctx);
	~LinuxGLDrawProxy();
	void initialize();
	bool create_surface(EGLNativeWindowType window);
	void destroy_surface(EGLNativeWindowType window);
	void refresh_surface_size(CGRect* rect);
	void refresh_virtual_keyboard_rect();
	void refresh_buffer();
	void begin_render();
	void commit_render();
	void initializ_gl_buffers();
	GLint get_gl_texture_pixel_format(PixelData::Format pixel_format);
	inline GLDraw* host() { return m_host; }
	static LinuxGLDrawProxy* create(GUIApplication* host, cJSON& options);
 protected:
	EGLDisplay m_display;
	EGLConfig m_config;
	EGLContext m_context;
	EGLSurface m_surface;
	EGLNativeWindowType m_window;
	Vec2 m_raw_surface_size;
	CGRect m_virtual_keys_rect;
	GLDraw* m_host;
};

XX_END

#endif
#endif