/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, blue.chu
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

#include "../util/loop.h"
#include "./render.h"
#include "../app.h"
#include <math.h>
#include "./gl/gl_render.h"
#include "../display.h"

namespace qk {

	static uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		return Qk_MIN(n, 8);
	}

	Render::Render(Application* host)
		: _host(host)
		, _opts(host->options())
		, _canvas(nullptr)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kColor_Type_RGBA_8888;//kColor_Type_BGRA_8888;
		_opts.msaaSampleCnt = massSample(_opts.msaaSampleCnt);
		//_opts.stencilBits = integerExp(Qk_MIN(Qk_MAX(_opts.stencilBits, 8), 16));
	}

	Render::~Render() {
	}

	void Render::activate(bool isActive) {
	}

	void Render::visitView(View* v) {
		// TODO ...
	}

	void Render::visitBox(Box* box) {
		// TODO ...
	}

	void Render::visitImage(Image* image) {
		// TODO ...
	}

	void Render::visitVideo(Video* video) {
		// TODO ...
	}

	void Render::visitScroll(Scroll* scroll) {
		// TODO ...
	}

	void Render::visitInput(Input* input) {
		// TODO ...
	}

	void Render::visitTextarea(Textarea* textarea) {
		// TODO ...
	}

	void Render::visitButton(Button* btn) {
		// TODO ...
	}

	void Render::visitTextLayout(TextLayout* text) {
		// TODO ...
	}

	void Render::visitLabel(Label* label) {
		// TODO ...
	}

	void Render::visitRoot(Root* root) {
		//glClearColor(0, 0, 0, 1);
		//glClear(GL_COLOR_BUFFER_BIT);

		auto size = _host->display()->size();

		Paint paint;
		paint.color = Color4f(1, 0, 1, 0.5);

		_canvas->drawRect(Rect{ Vec2(0,0), size*0.5 }, paint);
		
		paint.color = Color4f(1, 1, 0, 0.5);
		
		Path path(   Vec2(0, size.y()) );
		path.lineTo( size );
		path.lineTo( Vec2(size.x()*0.5, 0) );

		path.moveTo( Vec2(100, 100) );
		path.lineTo( Vec2(100, 200) );
		path.lineTo( Vec2(200, 200) );
		path.close();

		_canvas->drawPath(path, paint);

		paint.color = Color4f(0, 0, 1, 0.5);

		_canvas->drawPath(Path::Circle(Vec2(300), 100), paint);
	}

	void Render::visitFloatLayout(FloatLayout* flow) {
		// TODO ...
	}

	void Render::visitFlowLayout(FlowLayout* flow) {
		// TODO ...
	}

	void Render::visitFlexLayout(FlexLayout* flex) {
		// TODO ...
	}

}
