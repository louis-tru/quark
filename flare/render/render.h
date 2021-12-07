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
#include "../math.h"

#define SK_GL 1

#if F_IOS || F_OSX
# define SK_METAL 1
#elif F_ANDROID
# define SK_VULKAN 1
#elif F_WIN
# define SK_DIRECT3D 1
#endif

#include <skia/core/SkCanvas.h>
#include <skia/core/SkSurface.h>
#include <skia/gpu/GrDirectContext.h>

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

		/** @enum ColorType
			 Describes how pixel bits encode color. A pixel may be an alpha mask, a grayscale, RGB, or ARGB.
		*/
		enum ColorType : int {
			kAlpha_8_ColorType = 1,  //!< pixel with alpha in 8-bit byte
			kRGB_565_ColorType,      //!< pixel with 5 bits red, 6 bits green, 5 bits blue, in 16-bit word
			kARGB_4444_ColorType,    //!< pixel with 4 bits for alpha, red, green, blue; in 16-bit word
			kRGBA_8888_ColorType,    //!< pixel with 8 bits for red, green, blue, alpha; in 32-bit word
			kRGB_888x_ColorType,     //!< pixel with 8 bits each for red, green, blue; in 32-bit word
			kBGRA_8888_ColorType,    //!< pixel with 8 bits for blue, green, red, alpha; in 32-bit word
			kRGBA_1010102_ColorType, //!< 10 bits for red, green, blue; 2 bits for alpha; in 32-bit word
			kBGRA_1010102_ColorType, //!< 10 bits for blue, green, red; 2 bits for alpha; in 32-bit word
			kRGB_101010x_ColorType,  //!< pixel with 10 bits each for red, green, blue; in 32-bit word
			kBGR_101010x_ColorType,  //!< pixel with 10 bits each for blue, green, red; in 32-bit word
			kGray_8_ColorType,       //!< pixel with grayscale level in 8-bit byte
		};

		enum Flags {
			kUseDeviceIndependentFonts_Flag = 1 << 0,
			// Use internal MSAA to render to non-MSAA GPU surfaces.
			kDynamicMSAA_Flag               = 1 << 1
		};

		struct Options {
			ColorType           colorType = kRGBA_8888_ColorType;
			uint32_t            flags = 0;
			int                 MSAASampleCount;
			bool                disableVsync;
			bool                delayDrawableAcquisition;
			bool                enableBinaryArchive;
			bool                enableGpu;
			bool                enableMetal;
		};

		static Options parseOptions(cJSON& opts);

		static Render* create(Application* host, const Options& opts);

		virtual ~Render();

		Canvas* canvas();
		virtual SkSurface* surface() = 0;
		virtual void reload() = 0;
		virtual void commit() = 0;
		virtual void activate(bool isActive);
		virtual bool is_gpu() { return false; }
		inline Application* host() { return _host; }
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) override;

	 protected:
		Render(Application* host, const Options& params);

		Application*  _host;
		Options       _opts;
		sk_sp<GrDirectContext> _direct;
		int _sample_count, _stencil_bits;
	};

}

#endif
