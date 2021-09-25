/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright © 2015-2016, xuewen.chu
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

#include "./render.h"
#include <math.h>

namespace flare {

	static inline uint32_t integerExp(uint32_t n) {
		return (uint32_t) powf(2, floor(log2(n)));
	}

	static inline uint32_t massSample(uint32_t n) {
		n = integerExp(n);
		return FX_MIN(n, 8);
	}

	Render::Render(Application* host, const DisplayParams& params)
		: _host(host)
		, _DisplayParams(params)
		, _SampleCount(1)
		, _StencilBits(0)
	{
		_DisplayParams.fMSAASampleCount = massSample(_DisplayParams.fMSAASampleCount);
	}

	Render::~Render() {
	}

	SkCanvas* Render::canvas() {
		return getSurface()->getCanvas();
	}

	void Render::activate(bool isActive) {}

	void Render::setDisplayParams(const DisplayParams& params) {
		_DisplayParams = params;
		_DisplayParams.fMSAASampleCount = massSample(_DisplayParams.fMSAASampleCount);
		reload();
	}

	Render::DisplayParams Render::parseDisplayParams(cJSON& options) {
		// parse options to render params
		return DisplayParams();
	}

}
