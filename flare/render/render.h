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

#include "../util/loop.h"
#include "../util/json.h"

#define SK_GL 1

#if F_IOS || F_OSX
# define SK_METAL 1
#elif F_ANDROID
# define SK_VULKAN 1
#elif F_WIN
# define SK_DIRECT3D 1
#endif

#include "../math.h"
#include "skia/core/SkCanvas.h"
#include "skia/core/SkRefCnt.h"
#include "skia/core/SkSurfaceProps.h"
#include "skia/core/SkImageInfo.h"
#include "skia/core/SkSurface.h"
#include "skia/gpu/GrTypes.h"
#include "skia/gpu/GrDirectContext.h"
#include "skia/gpu/GrContextOptions.h"

namespace flare {

	class Application;

	class Canvas: public SkCanvas {
	 public:
		void setMatrix(const flare::Mat& mat);
	};

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
		Canvas* canvas();

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

}

#endif
