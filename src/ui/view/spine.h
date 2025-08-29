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

// @private

#ifndef __quark__view__spine__
#define __quark__view__spine__

#include "./sprite.h"

namespace spine {
	class Atlas;
	class AttachmentLoader;
	class Skeleton;
	class SkeletonClipping;
	class VertexEffect;
} // namespace spine

namespace qk {

	class Qk_EXPORT Spine: public SpriteView {
	public:
		Qk_DEFINE_VIEW_PROPERTY(String, src, Const); // spine data file path
		Spine();
		void destroy() override;
		//void play(bool all = false); // Play the sprite frames, play action of view together if the all equals true
		//void stop(bool all = false); // Stop the sprite frames, stop action of view together if the all equals true
		ViewType viewType() const override;
		Vec2 client_size() override;
		void draw(UIDraw *render) override;
	private:
		spine::Atlas *_atlas;
		spine::AttachmentLoader *_attachmentLoader;
		spine::Skeleton *_skeleton;
		spine::SkeletonClipping *_clipper;
		spine::VertexEffect *_effect;
	};

} // namespace qk
#endif // __quark__view__spine__
