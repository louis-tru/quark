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
#include "./source.h"
#include "../layout/view.h"

#ifndef N_USE_DEFAULT_THREAD_RENDER
#define N_USE_DEFAULT_THREAD_RENDER 0
#endif

namespace noug {

	class Application;

	/**
	* @class Render
	*/
	class N_EXPORT Render: public Object, public PostMessage {
		N_HIDDEN_ALL_COPY(Render);
	public:
		struct Options {
			ColorType colorType;
			int  msaaSampleCnt; // gpu msaa
			int  stencilBits;   // gpu stencil
		};
		static Options parseOptions(cJSON& json);
		static Render* Make(Application* host, const Options& opts);

		virtual ~Render();
		virtual void reload() = 0;
		virtual void begin() = 0;
		virtual void submit() = 0;
		virtual void activate(bool isActive);
		virtual ViewVisitor* visitor() = 0;
		inline  Application* host() { return _host; }
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) override;

	protected:
		Render(Application* host, const Options& opts);
		Application*  _host;
		Options       _opts;
	};

}
#endif
