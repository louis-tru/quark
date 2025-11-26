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
#include "../util/thread.h"
#include "../util/codec.h"
#include "./render.h"
#include "./gl/gl_render.h"

namespace qk {

	static uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		n = Qk_Min(n, 9);
		return n > 1 ? n: 0;
	}

	RenderBackend::RenderBackend(Options opts)
		: _opts(opts)
		, _canvas(nullptr)
		, _delegate(nullptr)
		, _isActive(true)
	{
		_opts.colorType = _opts.colorType ? _opts.colorType: kRGBA_8888_ColorType;
	}

	void RenderBackend::activate(bool isActive) {
		_isActive = isActive;
	}

	Render* make_metal_render(Render::Options opts);
	Render* make_vulkan_render(Render::Options opts);
	Render* make_gl_render(Render::Options opts);

	Render* Render::Make(Options opts, Delegate *delegate) {
		Render* r = nullptr;

		opts.msaaSample = massSample(opts.msaaSample);

#if Qk_ENABLE_VULKAN
		if (!r) r = make_vulkan_render();
#endif
#if Qk_ENABLE_METAL
		if (!r) r = make_metal_render();
#endif
#if Qk_ENABLE_GL
		if (!r) r = make_gl_render(opts);
#endif
		Qk_CHECK(r, "Create render object fail");

		r->_delegate = delegate;
		return r;
	}
}
