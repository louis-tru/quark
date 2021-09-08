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
#include "skia/core/SkRefCnt.h"
#include "skia/core/SkSurfaceProps.h"
#include "skia/core/SkImageInfo.h"
#include "skia/core/SkCanvas.h"
#include "skia/gpu/GrTypes.h"
#include "skia/gpu/GrDirectContext.h"
#include "skia/gpu/GrContextOptions.h"

namespace flare {

	/**
	* @class Render
	*/
	class FX_EXPORT Render: public Object {
		FX_HIDDEN_ALL_COPY(Render);
	public:

		struct DisplayParams {
			SkColorType         fColorType;
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
		SkCanvas* canvas();

		/**
		 * @func begin_render()
		 */
		virtual void begin_render();

		/**
		 * @func getBackbufferSurface()
		 */
		virtual sk_sp<SkSurface> getBackbufferSurface() = 0;

		virtual void swapBuffers() = 0;

		virtual bool isValid() = 0;

		virtual void resize(Vec2 size, Rect surface_region) = 0;

		virtual void activate(bool isActive);

		inline const DisplayParams& getDisplayParams() { return fDisplayParams; }

		virtual void setDisplayParams(const DisplayParams& params) = 0;

		inline GrDirectContext* directContext() const { return fContext.get(); }

		inline int width() const { return fWidth; }
		inline int height() const { return fHeight; }
		inline int sampleCount() const { return fSampleCount; }
		inline int stencilBits() const { return fStencilBits; }

		/**
		 * @func create(host, options)
		 */
		static Render* create(Application* host, cJSON& options);

		static DisplayParams parseDisplayParams(cJSON& options);
		
	protected:
		Render(Application* host, const DisplayParams& params);

		virtual bool isGpuContext() { return false; }

		Application*  _host;

		sk_sp<GrDirectContext> fContext;

		int               fWidth;
		int               fHeight;
		DisplayParams     fDisplayParams;

		// parameters obtained from the native window
		// Note that the platform .cpp file is responsible for
		// initializing fSampleCount and fStencilBits!
		int               fSampleCount;
		int               fStencilBits;
	};

	inline Render* render() { return app()->render(); }

}

#endif
