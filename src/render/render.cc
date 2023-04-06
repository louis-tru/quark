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

#include <math.h>
#include "../util/loop.h"
#include "../util/codec.h"
#include "../app.h"
#include "../display.h"
#include "./render.h"
#include "./gl/gl_render.h"

// layout
#include "../layout/root.h"

namespace qk {

	static uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		return Qk_MIN(n, 8);
	}

	RenderBackend::RenderBackend(Options opts)
		: _opts(opts)
		, _canvas(nullptr)
		, _delegate(nullptr)
		, _default_scale(1)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kColor_Type_RGBA_8888;//kColor_Type_BGRA_8888;
		_opts.msaaSampleCnt = massSample(_opts.msaaSampleCnt);
	}

	RenderBackend::~RenderBackend() {}

	void RenderBackend::activate(bool isActive) {
	}

	Array<Vec2>& RenderBackend::getPathPolygonsCache(const Path &path) {
		auto hash = path.hashCode();
		auto it = _PathPolygonsCache.find(hash);
		if (it != _PathPolygonsCache.end()) {
			return it->value;
		}
		if (_PathPolygonsCache.length() >= 1024)
			_PathPolygonsCache.clear();
		return _PathPolygonsCache.set(hash, path.getPolygons(3));
	}

	Array<Vec2>& RenderBackend::getPathStrokesCache(
		const Path &path, float width, Paint::Join join, float offset)
	{
		auto hash = path.hashCode();
		auto hash_part = ((*(int64_t*)&width) << 32) | *(int32_t*)&offset;
		hash += (hash << 5) + hash_part + join;
		auto it = _PathStrokesCache.find(hash);
		if (it != _PathStrokesCache.end()) {
			return it->value;
		}
		if (_PathStrokesCache.length() >= 1024)
			_PathStrokesCache.clear();
		return _PathStrokesCache
			.set(hash, path.strokePath(width, join, offset).getPolygons(3));
	}

	void RenderBackend::visitView(View* v) {
		// TODO ...
	}

	void RenderBackend::visitBox(Box* box) {
		// TODO ...
	}

	void RenderBackend::visitImage(Image* image) {
		// TODO ...
	}

	void RenderBackend::visitVideo(Video* video) {
		// TODO ...
	}

	void RenderBackend::visitScroll(Scroll* scroll) {
		// TODO ...
	}

	void RenderBackend::visitInput(Input* input) {
		// TODO ...
	}

	void RenderBackend::visitTextarea(Textarea* textarea) {
		// TODO ...
	}

	void RenderBackend::visitButton(Button* btn) {
		// TODO ...
	}

	void RenderBackend::visitTextLayout(TextLayout* text) {
		// TODO ...
	}

	void RenderBackend::visitLabel(Label* label) {
		// TODO ...
	}

	void RenderBackend::visitRoot(Root* root) {
		_canvas->clearColor(Color4f(1,1,1));
		//_canvas->drawColor(Color4f(1,0,0));
		Paint paint0, paint;

		auto size = shared_app()->display()->size();
		
		GradientColor g{{Color4f(1,0,1), Color4f(0,1,0), Color4f(0,0,1)}, {0,0.5,1}};
		Rect rect{ size*0.2*0.5, size*0.8 };
		//paint0.setLinearGradient(&g, rect.origin, rect.origin+rect.size);
		paint0.setRadialGradient(&g, rect.origin + rect.size*0.5, rect.size*0.5);
		
		_canvas->save(); 
		_canvas->setMatrix(_canvas->getMatrix() * Mat(Vec2(100,-50), Vec2(0.8, 0.8), -0.2, Vec2(0.3,0)));
		_canvas->drawRect(rect, paint0);
		_canvas->restore();

		// -------- clip ------
		_canvas->save();
		//_canvas->clipRect({ size*0.3*0.5, size*0.7 }, Canvas::kIntersect_ClipOp, 0);

		paint.color = Color4f(1, 0, 1, 0.5);

		paint.color = Color4f(0, 0, 1, 0.5);
		_canvas->drawPath(Path::Circle(Vec2(300), 100), paint);

		paint.color = Color4f(1, 0, 0, 0.8);
		_canvas->drawPath(Path::Oval({Vec2(200, 100), Vec2(100, 200)}), paint);

		// -------- clip ------
		_canvas->save();
		//_canvas->clipPath(Path::Circle(size*0.5, 100), Canvas::kDifference_ClipOp, 0);


		paint.color = Color4f(1, 1, 0, 0.5);

		Path path(   Vec2(0, size.y()) );
		path.lineTo( size );
		path.lineTo( Vec2(size.x()*0.5, 0) );

		path.moveTo( Vec2(100, 100) );
		path.lineTo( Vec2(100, 200) );
		path.lineTo( Vec2(200, 200) );
		path.close();
		_canvas->drawPath(path, paint);

		paint.color = Color4f(0, 1, 0, 0.8);
		_canvas->drawPath(Path::Arc({Vec2(400, 100), Vec2(200, 100)}, 0, 4.5, 1), paint);

		paint.color = Color4f(1, 0, 1, 0.8);
		_canvas->drawPath(Path::Arc({Vec2(450, 250), Vec2(200, 100)}, 4.5, 4, 0), paint);

		paint.color = Color4f(0, 0, 0, 0.8);
		_canvas->drawPath(Path::Arc({Vec2(450, 300), Vec2(100, 200)}, 3, 4, 1), paint);

		_canvas->restore(2);

		paint.color = Color4f(0,0,0);

		auto stype = FontStyle(TextWeight::BOLD, TextWidth::DEFAULT, TextSlant::NORMAL);
		auto pool = root->pre_render()->host()->font_pool();
		auto unicode = codec_decode_to_uint32(kUTF8_Encoding, "A 你好 HgKr向日葵pjAH");
		auto fgs = pool->getFFID()->makeFontGlyphs(unicode, stype, 64);

		Vec2 offset(0,60);

		for (auto &fg: fgs) {
			offset[0] += ceilf(_canvas->drawGlyphs(fg, offset, NULL, paint));
		}
	}

	void RenderBackend::visitFloatLayout(FloatLayout* flow) {
		// TODO ...
	}

	void RenderBackend::visitFlowLayout(FlowLayout* flow) {
		// TODO ...
	}

	void RenderBackend::visitFlexLayout(FlexLayout* flex) {
		// TODO ...
	}

}
