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
#ifndef __quark__view__spine_inl__
#define __quark__view__spine_inl__

#include "./spine.h"
#include "../painter.h"
#include "../app.h"
#include "../window.h"
#include <spine/spine.h>

using namespace spine;

namespace qk {
	extern const float zDepthNextUnit;
	typedef Canvas::V3F_T2F_C4B_C4B V3F_T2F_C4B_C4B;
	typedef Canvas::Triangles Triangles;
	typedef SpineEvent::Type SEType;
	class AttachmentVertices;

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

	struct Spine::SpineOther {
		SkeletonClipping _clipper;
		Sp<VertexEffect> _effect;
		QkMutex _mutex; // protects _self and SkeletonSelf members
		SpineOther(): _clipper(), _effect(nullptr) {}
	};

	class AttachmentVertices {
	public:
		AttachmentVertices(AtlasPage* page, uint16_t *triangles,
			uint32_t vertCount, uint32_t indexCount);
		~AttachmentVertices();
		Triangles _triangles;
		ImageSource *_source;
		ImagePaint _paint;
		uint32_t _width, _height;
		bool _pma;
	};

	class QkTextureLoader: public TextureLoader {
	public:
		void load(AtlasPage &page, const spine::String &path) override;
		void unload(void *source) override;
	};

	class SkeletonData::QkAtlasAttachmentLoader: public AtlasAttachmentLoader {
	public:
		QkAtlasAttachmentLoader(Atlas *atlas): AtlasAttachmentLoader(atlas) {}
		void configureAttachment(Attachment *attachment) override;
		static void deleteAttachmentVertices(void *vertices);
		void setAttachmentVertices(RegionAttachment *attachment);
		void setAttachmentVertices(MeshAttachment *attachment);
	};

}
#endif