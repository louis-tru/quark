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

// @private
#ifndef __quark__view__spine_inl__
#define __quark__view__spine_inl__

#include <spine/spine.h>
#include "./spine.h"
#include "../painter.h"
#include "../app.h"
#include "../window.h"

using namespace spine;

namespace spine {
	bool loadSequence(Atlas *atlas, const String &basePath, Sequence *sequence);
}

namespace qk {
	extern const float zDepthNextUnit;
	typedef Canvas::V3F_T2F_C4B_C4B V3F_T2F_C4B_C4B;
	typedef Canvas::Triangles Triangles;
	typedef SpineEvent::Type SEType;
	typedef const spine::String cSPString;

	struct TrianglesEx: Triangles {
		BlendMode blendMode;
	};

	struct Spine::SkeletonWrapper: public Object {
		Spine *_host;
		Sp<SkeletonData> _wrapData;
		spine::SkeletonData* _data;
		Skeleton _skeleton;
		AnimationStateData _stateData;
		AnimationState _state;
		SkeletonWrapper(Spine *host, SkeletonData *data);
		void destroy() override;
		void update(float deltaTime);
	};

	class AttachmentEx {
	public:
		AttachmentEx(AtlasPage* page, uint16_t *triangles,
			uint32_t vertCount, uint32_t indexCount);
		~AttachmentEx();
		Triangles _triangles;
		ImageSource *_source;
		PaintImage _paint;
		uint32_t _width, _height;
		uint64_t _hashCode;
	};

	template<class T>
	class AttachmentWrap: public T {
	public:
		AttachmentWrap(cSPString &name): T(name), ex(nullptr) {}
		~AttachmentWrap() {
			Releasep(ex);
		}
		AttachmentEx *ex;
	};

	typedef AttachmentWrap<RegionAttachment> RegionAttachmentEx;
	typedef AttachmentWrap<MeshAttachment> MeshAttachmentEx;

	class QkTextureLoader: public TextureLoader {
	public:
		void load(AtlasPage &page, cSPString &path) override;
		void unload(void *source) override;
	};

	class SkeletonData::QkAtlasAttachmentLoader: public AtlasAttachmentLoader {
	public:
		QkAtlasAttachmentLoader(Atlas *atlas): AtlasAttachmentLoader(atlas), _atlas(atlas) {}
		void configureAttachment(Attachment *attachment) override;
		RegionAttachment *newRegionAttachment(Skin &skin, cSPString &name, cSPString &path, Sequence *sequence) override;
		MeshAttachment *newMeshAttachment(Skin &skin, cSPString &name, cSPString &path, Sequence *sequence) override;
		void setAttachmentEx(RegionAttachmentEx *attachment);
		void setAttachmentEx(MeshAttachmentEx *attachment);
	private:
		template<class T>
		T* newAttachmentEx(Skin &skin, cSPString &name, cSPString &path, Sequence *sequence);
		Atlas *_atlas;
	};

}
#endif
