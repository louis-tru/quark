/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

// ------------------- Metal ------------------
#if Qk_ENABLE_METAL
#import "./appel_render.h"

using namespace qk;

@interface MTView: UIView
@end

@implementation MTView
+ (Class)layerClass {
	if (@available(iOS 13.0, *))
		return CAMetalLayer.class;
	return nil;
}
@end

namespace qk {

	class AppleMetalRender: public RenderBackend, public RenderSurface {
	public:
		AppleMetalRender(Options opts)
			: Render(opts)
		{}
		// void release() override;
		// void reload() override;
		// Canvas* newCanvas(Options opts) override;
		// bool newTexture(cPixel *pix, int levels, TexStat *&out, bool genMipmap) override;
		// void newVertexData(VertexData::ID *id) override
		// void deleteTexture(TexStat *tex) override;
		// void deleteVertexData(VertexData::ID *id) override;
		// void post_message(Cb cb) override;
		// Vec2 getSurfaceSize() override;
		UIView* surfaceView() override {
			if (_view) {
				return _view;
			}
			_view = [[MTKView alloc] initWithFrame:CGRectZero device:nil];
			_view.layer.opaque = YES;
			return _view;
		}
		RenderSurface* surface() override {
			return this;
		}
	};

	Render* make_metal_render(Render::Options opts) {
		Render* r = nullptr;
			//if (@available(macOS 10.11, iOS 13.0, *))
			//	r = new AppleMetalRender(opts);
		return r;
	}
}
#endif
