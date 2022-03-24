// @private head
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


#ifndef __ftr__render_skia_render__
#define __ftr__render_skia_render__

#ifdef __OBJC__
#include "../metal.h"
#endif

#include "../gl.h"

#define SK_GL 1

#if F_IOS || F_OSX
# define SK_METAL 1
#elif F_ANDROID
# define SK_VULKAN 1
#elif F_WIN
# define SK_DIRECT3D 1
#endif

#include "skia/core/SkCanvas.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrBackendSurface.h"
#include "skia/gpu/GrDirectContext.h"

F_NAMESPACE_START

/**
* @class SkiaGLRender
*/
class SkiaGLRender: public GLRender {
public:
	virtual Canvas* canvas() override;
protected:
	virtual void onReload() override;
	virtual void onSubmit() override;
	SkiaGLRender(Application* host, const Options& opts, bool raster);
	sk_sp<GrDirectContext> _direct;
	sk_sp<SkSurface> _surface;
	sk_sp<SkSurface> _rasterSurface;
	bool _raster; // software raster
};

#ifdef __OBJC__

/**
* @class SkiaMetalRender
*/
class SkiaMetalRender: public MetalRender {
public:
	virtual Canvas* canvas() override;
protected:
	virtual void onReload() override;
	virtual void onBegin() override;
	virtual void onSubmit() override;
	SkiaMetalRender(Application* host, const Options& opts, bool raster);
	sk_sp<GrDirectContext> _direct;
	sk_sp<SkSurface> _surface;
	sk_sp<SkSurface> _rasterSurface;
	bool _raster; // software raster
};

#endif

F_NAMESPACE_END
#endif
