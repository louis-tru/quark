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

// @private head

#ifndef __quark__render_render__
#define __quark__render_render__

#include "../util/json.h"
#include "./math.h"
#include "./source.h"
#include "./canvas.h"
#include "./font/pool.h"

namespace qk {

	/**
	 * @class RenderBackend drawing device backend
	 * @thread render
	 */
	class Qk_EXPORT RenderBackend: public Object, public PostMessage {
	public:
		struct Options {
			ColorType   colorType;
			uint16_t    msaa; // gpu msaa
			uint16_t    fps; // if fps is 0 then use vSync, else limit fps value
			bool        isMultiThreading; // multi threading support
		};
		class Delegate {
		public:
			virtual bool onRenderBackendReload(Region region, Vec2 size,
								float defaultScale, Mat4 *surfaceMat, Vec2* surfaceScale) = 0;
			virtual void onRenderBackendDisplay() = 0;
		};

		inline const Options& options() const { return _opts; }
		static  RenderBackend* Make(Options opts, Delegate *delegate);
		virtual        ~RenderBackend();
		virtual void    reload() = 0; // surface size and scale change
		virtual void    begin() = 0; // start render task
		virtual void    submit() = 0; // submit render task
		virtual void    activate(bool isActive);
		inline  Canvas* getCanvas() { return _canvas; } // default canvas object
		inline  Vec2    surfaceSize() { return _surfaceSize; }
		inline  float   defaultScale() { return _defaultScale; }
		inline  Delegate* delegate() { return _delegate; }
		virtual uint32_t makeTexture(cPixel *src, uint32_t id) = 0;
		virtual void    deleteTextures(const uint32_t *ids, uint32_t count) = 0;
		virtual void    makeVertexData(VertexData::ID *id) = 0;
		virtual void    deleteVertexData(VertexData::ID *id) = 0;

		// @overwrite class PostMessage
		virtual uint32_t post_message(Cb cb, uint64_t delay_us = 0) override;

	protected:
		RenderBackend(Options opts);
		virtual Vec2  getSurfaceSize() = 0;
		virtual float getDefaultScale() = 0;
		Options       _opts;
		Canvas       *_canvas; // default canvas
		Delegate     *_delegate;
		Vec2          _surfaceSize; // current surface size
		float         _defaultScale; // recommend default surface scale
	};

	typedef RenderBackend Render;
}
#endif
