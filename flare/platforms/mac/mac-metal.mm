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

#import <Metal/Metal.h>
#import "./mac-render.h"
#import "../../render/metal.h"

namespace flare {

	class MetalRenderMAC : public MetalRender, public RenderMAC {
	public:
		MetalRenderMAC(GUIApplication* host, const DisplayParams& params): MetalRender(host, params) {}
		void setView(UIView* view) {
			ASSERT(!_view);
			_view = view;
			_layer = view.layer;
			_host->display()->set_best_display_scale(UIScreen.mainScreen.scale);
		}
		Class layerClass() { return [CAMetalLayer class]; }
		Render* render() { return this; }
	private:
		UIView* _view;
	};

	RenderMAC* MakeMetalRender(GUIApplication* host, const Render::DisplayParams& parems) {
#if GR_METAL_SDK_VERSION >= 230
		if (@available(macOS 11.0, iOS 14.0, *)) {
			return new MetalRenderMAC(host, parems);
		}
#endif
		return nullptr;
	}

}