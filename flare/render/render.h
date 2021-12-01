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


#ifndef __ftr__render_render__
#define __ftr__render_render__

#include "../app.h"

#define SK_GL 1

#if F_IOS || F_OSX
# define SK_METAL 1
#elif F_ANDROID
# define SK_VULKAN 1
#elif F_WIN
# define SK_DIRECT3D 1
#endif

// -DSK_HAS_ANDROID_CODEC 
// -DSK_ENABLE_DUMP_GPU 
// -DSK_DISABLE_AAA 
// -DSK_PARAGRAPH_LIBTXT_SPACES_RESOLUTION 
// -DSK_LEGACY_INNER_JOINS 
// -DSK_DISABLE_LEGACY_SHADERCONTEXT 
// -DSK_DISABLE_LOWP_RASTER_PIPELINE 
// -DSK_FORCE_RASTER_PIPELINE_BLITTER 
// -DSK_DISABLE_EFFECT_DESERIALIZATION 
// -DSK_ENABLE_SKSL 
// -DSK_ASSUME_GL_ES=1 
// -DSK_ENABLE_API_AVAILABLE 
// -DSK_GAMMA_APPLY_TO_A8 
// -DSKIA_IMPLEMENTATION=1 
// -DSK_GL 
// -DSK_METAL 
// -DSK_SUPPORT_PDF 
// -DSK_CODEC_DECODES_JPEG 
// -DSK_ENCODE_JPEG 
// -DSK_USE_LIBGIFCODEC 
// -DSK_CODEC_DECODES_PNG 
// -DSK_ENCODE_PNG 
// -DSK_CODEC_DECODES_RAW 
// -DSK_CODEC_DECODES_WEBP 
// -DSK_ENCODE_WEBP 
// -DSK_XML 

#include "flare/render/canvas.h"
#include "skia/core/SkRefCnt.h"
#include "skia/core/SkSurfaceProps.h"
#include "skia/core/SkImageInfo.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrTypes.h"
#include "skia/gpu/GrDirectContext.h"
#include "skia/gpu/GrContextOptions.h"

namespace flare {

	/**
	* @class Render
	*/
	class F_EXPORT Render: public Object, public PostMessage {
		F_HIDDEN_ALL_COPY(Render);
	 public:

		struct DisplayParams {
			SkColorType         fColorType = kRGBA_8888_SkColorType;
			sk_sp<SkColorSpace> fColorSpace;
			int                 fMSAASampleCount;
			GrContextOptions    fGrContextOptions;
			SkSurfaceProps      fSurfaceProps;
			bool                fDisableVsync;
			bool                fDelayDrawableAcquisition;
			bool                fEnableBinaryArchive;
		};

		virtual ~Render();

		/**
		 * @func canvas()
		 */
		SkCanvasLink* canvas();

		/**
		 * @func getSurface()
		 */
		virtual SkSurface* getSurface() = 0;

		/**
		 * @func reload()
		 */
		virtual void reload() = 0;
		virtual void commit() = 0;
		virtual void activate(bool isActive);
		virtual bool isGpu() { return false; }
		virtual void setDisplayParams(const DisplayParams& params);

		inline const DisplayParams& displayParams() { return _DisplayParams; }
		inline GrDirectContext* directContext() { return _Context.get(); }
		inline Application* host() { return _host; }
		inline int sampleCount() const { return _SampleCount; }
		inline int stencilBits() const { return _StencilBits; }

		/**
		 * @override
		 */
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) override;

		/**
		 * @func create(host, options)
		 */
		static Render* create(Application* host, cJSON& options);

		static DisplayParams parseDisplayParams(cJSON& options);

	 protected:
		Render(Application* host, const DisplayParams& params);

		Application*  _host;

		sk_sp<GrDirectContext> _Context;
		DisplayParams     _DisplayParams;
		int _SampleCount, _StencilBits;
	};

	inline Render* render() { return app()->render(); }

}

#endif
