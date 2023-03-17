// @private head
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


#ifndef __ftr__render_render__
#define __ftr__render_render__

#include "../util/json.h"
#include "../math.h"
#include "../layout/view.h"
#include "./source.h"
#include "./canvas.h"

#ifndef Qk_USE_DEFAULT_THREAD_RENDER
#define Qk_USE_DEFAULT_THREAD_RENDER 1
#endif

namespace qk {
	class Application;

	class BackendDevice: public PostMessage {
	public:
		virtual uint32_t setTexture(cPixel *src, uint32_t id) = 0;
		virtual void     deleteTextures(const uint32_t *IDs, uint32_t count) = 0;
	};

	/**
	 * @class Render drawing management
	 */
	class Qk_EXPORT Render: public BackendDevice, public ViewVisitor {
	public:
		struct Options {
			ColorType   colorType;
			int         msaaSampleCnt; // gpu msaa
			int         stencilBits;   // gpu stencil
			bool        independentThread; // independent render thread
		};
		static  Render* Make(Application* host);
		virtual        ~Render();
		virtual void    reload(Vec2 size, Mat4& root) = 0;
		virtual void    begin() = 0;
		virtual void    submit() = 0;
		virtual void    activate(bool isActive);
		virtual Object* asObject() = 0;
		inline  Canvas* getCanvas() { return _canvas; }
		inline  Application* host() { return _host; }
		// @overwrite class PostMessage
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) override;
		// @overwrite class ViewVisitor
		virtual void    visitView(View* v) override;
		virtual void    visitBox(Box* box) override;
		virtual void    visitImage(Image* image) override;
		virtual void    visitVideo(Video* video) override;
		virtual void    visitScroll(Scroll* scroll) override;
		virtual void    visitInput(Input* input) override;
		virtual void    visitTextarea(Textarea* textarea) override;
		virtual void    visitButton(Button* btn) override;
		virtual void    visitTextLayout(TextLayout* text) override;
		virtual void    visitLabel(Label* label) override;
		virtual void    visitRoot(Root* root) override;
		virtual void    visitFloatLayout(FloatLayout* flow) override;
		virtual void    visitFlowLayout(FlowLayout* flow) override;
		virtual void    visitFlexLayout(FlexLayout* flex) override;
	protected:
		Render(Application *host, bool independentThread);
		Options       _opts;
		Application  *_host;
		Canvas       *_canvas;
		KeepLoop     *_renderLoop;
	};

}
#endif
