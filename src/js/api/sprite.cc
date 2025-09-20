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

#include "./ui.h"
#include "../../ui/view/sprite.h"

namespace qk { namespace js {

	class MixSprite: public MixViewObject {
	public:
		typedef Sprite Type;
		virtual MatrixView* asMatrixView() {
			return self<Sprite>();
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Define_Class(Sprite, View, {
				Js_NewView(Sprite);
			});
			inheritMatrixView(cls, worker);

			Js_MixObject_Accessor(Type, String, src, src);
			Js_MixObject_Accessor(Type, float, width, width);
			Js_MixObject_Accessor(Type, float, height, height);
			Js_MixObject_Accessor(Type, uint32_t, frame, frame);
			Js_MixObject_Accessor(Type, uint32_t, frames, frames);
			Js_MixObject_Accessor(Type, uint32_t, item, item);
			Js_MixObject_Accessor(Type, uint32_t, items, items);
			Js_MixObject_Accessor(Type, uint32_t, gap, gap);
			Js_MixObject_Accessor(Type, uint32_t, fsp, fsp);
			Js_MixObject_Accessor(Type, Direction, direction, direction);
			Js_MixObject_Accessor(Type, bool, playing, playing);

			Js_Class_Method(play, {
				self->play();
			});

			Js_Class_Method(stop, {
				self->stop();
			});

			cls->exports("Sprite", exports);
		}
	};

	void binding_sprite(JSObject* exports, Worker* worker) {
		MixSprite::binding(exports, worker);
	}
} }
