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
	class SkeletonData;
	class Atlas;
	class AtlasAttachmentLoader;
}

namespace qk {

	class Qk_EXPORT SkeletonData: public Reference {
	public:
		static Sp<SkeletonData> Make(cString &skeletonPath, cString &atlasPath = String(), float scale = 1.0f) throw(Error);
		static Sp<SkeletonData> Make(cBuffer &skeletonBuff, cString &atlasPath, float scale = 1.0f) throw(Error);
		static Sp<SkeletonData> Make(cBuffer &skeletonBuff, cBuffer &atlasBuff, cString &dir, float scale = 1.0f);
		~SkeletonData();
	private:
		SkeletonData(spine::SkeletonData* data, spine::Atlas* atlas, spine::AtlasAttachmentLoader* loader);
		spine::SkeletonData* _data;
		spine::Atlas* _atlas;
		spine::AtlasAttachmentLoader* _atlasLoader;
		friend class Spine;
	};

	class Qk_EXPORT Spine: public SpriteView, public PreRender::Task {
	public:
		Qk_DEFINE_ACCESSOR(SkeletonData*, skeleton); // spine skeleton data
		Spine();
		void destroy() override;
		// Play the sprite frames, play action of view together if the all equals true
		//void play(bool all = false);
		// Stop the sprite frames, stop action of view together if the all equals true
		//void stop(bool all = false);
		ViewType viewType() const override;
		Vec2 client_size() override;
		void draw(Painter *painter) override;
		void onActivate() override;
		bool run_task(int64_t time, int64_t delta) override;
	private:
		Qk_DEFINE_INLINE_CLASS(SkeletonWraper);
		std::atomic<SkeletonWraper*> _wraper;
		int _startSlotIndex;
		int _endSlotIndex;
	};

} // namespace qk
#endif // __quark__view__spine__
