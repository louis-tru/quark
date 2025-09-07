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

#include "./spine.h"
#include "../../util/fs.h"
#include "../painter.h"
#include "../app.h"
#include "../window.h"
#include <spine/spine.h>

using namespace spine;

namespace qk {

// C Variable length array
#ifdef _MSC_VER
// VLA not supported, use _malloca
#define VLA(type, arr, count) \
	type *arr = static_cast<type *>(_malloca(sizeof(type) * count))
#define VLA_FREE(arr) \
	do { _freea(arr); } while (false)
#else
#define VLA(type, arr, count) \
	type arr[count]
#define VLA_FREE(arr)
#endif

	typedef Canvas::V3F_T2F_C4B_C4B V3F_T2F_C4B_C4B;
	typedef Canvas::Triangles Triangles;

	class AttachmentVertices {
	public:
		AttachmentVertices(AtlasPage* page, uint16_t *triangles, int vertCount, int indexCount)
			: _source(static_cast<ImageSource*>(page->getRendererObject()))
			, _width(page->width), _height(page->height)
			, _pma(page->pma)
		{
			if (_source)
				_source->retain();
			_triangles.verts = Allocator::shared()->alloc<V3F_T2F_C4B_C4B>(vertCount);
			_triangles.vertCount = vertCount;
			_triangles.indices = triangles;
			_triangles.indexCount = indexCount;
			_paint.image = _source;
			_paint.filterMode = page->magFilter == TextureFilter_Linear ?
				ImagePaint::kLinear_FilterMode: ImagePaint::kNearest_FilterMode;

			switch (page->minFilter) {
				case TextureFilter_MipMapLinearNearest:
					_paint.mipmapMode = ImagePaint::kLinearNearest_MipmapMode;
					break;
				case TextureFilter_MipMapNearestLinear:
					_paint.mipmapMode = ImagePaint::kNearestLinear_MipmapMode;
					break;
				case TextureFilter_MipMapLinearLinear:
				_paint.mipmapMode = ImagePaint::kLinear_MipmapMode;
					break;
				default:
					_paint.mipmapMode = ImagePaint::kNone_MipmapMode;
					break;
			}

			switch (page->uWrap) {
				case TextureWrap_MirroredRepeat:
					_paint.tileModeX = ImagePaint::kMirror_TileMode;
					break;
				case TextureWrap_ClampToEdge:
					_paint.tileModeX = ImagePaint::kClamp_TileMode;
					break;
				case TextureWrap_Repeat:
					_paint.tileModeX = ImagePaint::kRepeat_TileMode;
					break;
			}

			switch (page->vWrap) {
				case TextureWrap_MirroredRepeat:
					_paint.tileModeY = ImagePaint::kMirror_TileMode;
					break;
				case TextureWrap_ClampToEdge:
					_paint.tileModeY = ImagePaint::kClamp_TileMode;
					break;
				case TextureWrap_Repeat:
					_paint.tileModeY = ImagePaint::kRepeat_TileMode;
					break;
			}
		}

		~AttachmentVertices() {
			Allocator::shared()->free(_triangles.verts);
			if (_source)
				_source->release();
		}

		Triangles _triangles;
		ImageSource *_source;
		ImagePaint _paint;
		int _width, _height;
		bool _pma;
	};

	static uint16_t quadTriangles[6] = {0, 1, 2, 2, 3, 0};

	class QkAtlasAttachmentLoader: public AtlasAttachmentLoader {
	public:
		QkAtlasAttachmentLoader(Atlas *atlas): AtlasAttachmentLoader(atlas) {}

		void configureAttachment(Attachment *attachment) override {
			if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
				setAttachmentVertices((RegionAttachment *) attachment);
			} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
				setAttachmentVertices((MeshAttachment *) attachment);
			}
		}

		static void deleteAttachmentVertices(void *vertices) {
			delete (AttachmentVertices *) vertices;
		}

		void setAttachmentVertices(RegionAttachment *attachment) {
			auto region = (AtlasRegion *) attachment->getRendererObject();
			auto attachmentVertices = new AttachmentVertices(region->page, quadTriangles, 4, 6);
			V3F_T2F_C4B_C4B *vertices = attachmentVertices->_triangles.verts;
			for (int i = 0, ii = 0; i < 4; ++i, ii += 2) {
				vertices[i].texCoords[0] = attachment->getUVs()[ii];
				vertices[i].texCoords[1] = attachment->getUVs()[ii + 1];
			}
			attachment->setRendererObject(attachmentVertices, deleteAttachmentVertices);
		}

		void setAttachmentVertices(MeshAttachment *attachment) {
			auto region = (AtlasRegion *) attachment->getRendererObject();
			auto attachmentVertices = new AttachmentVertices(
				region->page,
				attachment->getTriangles().buffer(),
				attachment->getWorldVerticesLength() >> 1,
				attachment->getTriangles().size()
			);
			V3F_T2F_C4B_C4B *vertices = attachmentVertices->_triangles.verts;
			for (int i = 0, ii = 0, nn = attachment->getWorldVerticesLength(); ii < nn; ++i, ii += 2) {
				vertices[i].texCoords[0] = attachment->getUVs()[ii];
				vertices[i].texCoords[1] = attachment->getUVs()[ii + 1];
			}
			attachment->setRendererObject(attachmentVertices, deleteAttachmentVertices);
		}
	};

	class QkTextureLoader: public TextureLoader {
	public:
		void load(AtlasPage &page, const spine::String &path) override {
			auto source = shared_app()->imgPool()->get(path.buffer());
			Qk_ASSERT(source, "Invalid image");
			Qk_ASSERT_NE(page.width, 0, "Invalid image width");
			Qk_ASSERT_NE(page.height, 0, "Invalid image height");
			source->retain();
			page.setRendererObject(source);
		}
		void unload(void *source) override {
			((ImageSource *) source)->release();
		}
	};

	static QkTextureLoader textureLoader;

	Sp<SkeletonData> SkeletonData::Make(cString &skeletonPath, cString &atlasPath, float scale) throw(Error) {
		if (skeletonPath.isEmpty())
			return nullptr;
		auto sk_buff = fs_reader()->read_file_sync(skeletonPath);
		auto atlasP(atlasPath);
		if (atlasP.isEmpty()) {
			auto dir = fs_dirname(skeletonPath);
			auto base = fs_basename(skeletonPath);
			if (base.lastIndexOf("-ess.skel")) {
				atlasP = fs_format(*dir, *base.substring(0, base.lastIndexOf("-ess.skel")), ".atlas");
			} else if (base.lastIndexOf("-pro.skel")) {
				atlasP = fs_format(*dir, *base.substring(0, base.lastIndexOf("-pro.skel")), ".atlas");
			} else if (base.lastIndexOf(".skel")) {
				atlasP = fs_format(*dir, *base.substring(0, base.lastIndexOf(".skel")), ".atlas");
			} else {
				return nullptr;
			}
		}
		auto atlas_buff = fs_reader()->read_file_sync(atlasP);
		return Make(sk_buff, atlas_buff, fs_dirname(atlasP), scale);
	}

	Sp<SkeletonData> SkeletonData::Make(cBuffer &skeletonBuff, cString &atlasPath, float scale) throw(Error) {
		auto atlas_buff = fs_reader()->read_file_sync(atlasPath);
		return Make(skeletonBuff, atlas_buff, fs_dirname(atlasPath), scale);
	}

	Sp<SkeletonData> SkeletonData::Make(cBuffer &skeletonBuff, cBuffer &atlasBuff, cString &dir, float scale) {
		Sp<Atlas> atlas = new Atlas(*atlasBuff, atlasBuff.length(), dir.c_str(), &textureLoader, true);
		Sp<AtlasAttachmentLoader> loader = new QkAtlasAttachmentLoader(*atlas);

		SkeletonBinary binary(*loader);
		binary.setScale(scale);
		auto data = binary.readSkeletonData((const unsigned char*)skeletonBuff.val(), skeletonBuff.length());

		Qk_ASSERT(data, (!binary.getError().isEmpty() ? binary.getError().buffer() : "Error reading skeleton data."));

		if (!data)
			return nullptr;
		return new SkeletonData(data, atlas.collapse(), loader.collapse());
	}

	SkeletonData::SkeletonData(spine::SkeletonData* data, Atlas* atlas, AtlasAttachmentLoader* loader)
		: _data(data), _atlas(atlas), _atlasLoader(loader) {
	}

	SkeletonData::~SkeletonData() {
		Releasep(_data);
		Releasep(_atlas);
		Releasep(_atlasLoader);
	}

	class Spine::SkeletonWraper: public Object {
	public:
		SkeletonWraper(Spine *host, SkeletonData *data)
			: _host(host), _wrapData(data), _data(_wrapData->_data)
			, _skeleton(_data)
			, _stateData(_data)
			, _state(&_stateData)
			, _clipper(), _effect(nullptr)
		{}
		~SkeletonWraper() {
			Releasep(_skeleton);
			Releasep(_clipper);
			Releasep(_effect);
		}
		void destroy() {
			_host->preRender().async_call([](auto self, auto arg) {
				self->Object::destroy(); // safe destroy on render thread
			}, this, 0);
		}
		Spine *_host;
		Sp<SkeletonData> _wrapData;
		spine::SkeletonData* _data;
		spine::Skeleton _skeleton;
		spine::AnimationStateData _stateData;
		spine::AnimationState _state;
		spine::SkeletonClipping _clipper;
		spine::VertexEffect *_effect;
	};

	#define _IfWraper(...) if (_wraper) return __VA_ARGS__; auto self = _wraper.load()

	Spine::Spine(): SpriteView()
		, _wraper(nullptr)
		, _start_slot(0), _end_slot(0xffffffffu) // all slots
		, _speed(1.0f)
	{}

	void Spine::destroy() {
		Release(_wraper.load());
		_wraper.store(nullptr);
		View::destroy(); // Call parent destroy
	}

	void Spine::set_skeleton(SkeletonData *data) {
		if (_wraper) {
			if (_wraper.load()->_wrapData == data)
				return;
			Release(_wraper.load());
			_wraper.store(nullptr);
		}
		if (!data)
			return;
		auto self = NewRetain<SkeletonWraper>(this, data);
		_wraper.store(self);
		self->_skeleton.setToSetupPose();
		self->_skeleton.updateWorldTransform();
		self->_state.setRendererObject(this);
		self->_state.setListener([](AnimationState *state, spine::EventType type, TrackEntry *entry, spine::Event *event) {
			// ((Spine *) state->getRendererObject())->onAnimationStateEvent(entry, type, event);
		});
		// Pre update once
		run_task(0, 0);
	}

	SkeletonData* Spine::skeleton() {
		_IfWraper(nullptr);
		return self->_wrapData.get();
	}

	void Spine::set_start_slot(uint32_t val) {
		_start_slot = val;
	}

	void Spine::set_end_slot(uint32_t val) {
		_end_slot = val;
	}

	void Spine::set_to_setup_pose() {
		_IfWraper();
		self->_skeleton.setToSetupPose();
	}

	void Spine::set_bones_to_setup_pose() {
		_IfWraper();
		self->_skeleton.setBonesToSetupPose();
	}

	void Spine::set_slots_to_setup_pose() {
		_IfWraper();
		self->_skeleton.setSlotsToSetupPose();
	}

	String Spine::skin() const {
		_IfWraper(String());
		auto &name = self->_skeleton.getSkin()->getName();
		return String(name.buffer(), name.length());
	}

	void Spine::set_skin(String val) {
		_IfWraper();
		self->_skeleton.setSkin(val.isEmpty() ? nullptr: val.c_str());
	}

	void Spine::set_speed(float val) {
		_speed = Qk_Min(1e2, Qk_Max(val, 0.01));
	}

	void Spine::set_attachment(cString &slotName, cString &attachmentName) {
		_IfWraper();
		self->_skeleton.setAttachment(slotName.c_str(),
			attachmentName.isEmpty() ? nullptr: attachmentName.c_str()
		);
	}

	float Spine::default_mix() const {
		if (_wraper) {
			return _wraper.load()->_stateData.getDefaultMix();
		}
		return 0.f;
	}

	void Spine::set_default_mix(float val) {
		_IfWraper();
		self->_stateData.setDefaultMix(val);
	}

	void Spine::set_mix(cString &fromAnimation, cString &toAnimation, float duration) {
		_IfWraper();
		self->_stateData.setMix(fromAnimation.c_str(), toAnimation.c_str(), duration);
	}

	TrackEntry* Spine::set_animation(int trackIndex, cString &name, bool loop) {
		_IfWraper(nullptr);
		Animation *animation = self->_data->findAnimation(name.c_str());
		if (!animation) {
			Qk_Log("Spine: Animation not found: %s", name.c_str());
			return nullptr;
		}
		return self->_state.setAnimation(trackIndex, animation, loop);
	}

	TrackEntry *Spine::add_animation(int trackIndex, cString &name, bool loop, float delay) {
		_IfWraper(nullptr);
		Animation *animation = self->_data->findAnimation(name.c_str());
		if (!animation) {
			Qk_Log("Spine: Animation not found: %s", name.c_str());
			return nullptr;
		}
		return self->_state.addAnimation(trackIndex, animation, loop, delay);
	}

	TrackEntry *Spine::set_empty_animation(int trackIndex, float mixDuration) {
		_IfWraper(nullptr);
		return self->_state.setEmptyAnimation(trackIndex, mixDuration);
	}

	void Spine::set_empty_animations(float mixDuration) {
		_IfWraper();
		self->_state.setEmptyAnimations(mixDuration);
	}

	TrackEntry *Spine::add_empty_animation(int trackIndex, float mixDuration, float delay) {
		_IfWraper(nullptr);
		return self->_state.addEmptyAnimation(trackIndex, mixDuration, delay);
	}

	Animation*  Spine::find_animation(cString &name) const {
		_IfWraper(nullptr);
		return self->_data->findAnimation(name.c_str());
	}

	TrackEntry* Spine::get_current(int trackIndex) {
		_IfWraper(nullptr);
		return self->_state.getCurrent(trackIndex);
	}

	void Spine::clear_tracks() {
		_IfWraper();
		self->_state.clearTracks();
	}

	void Spine::clear_track(int trackIndex) {
		_IfWraper();
		self->_state.clearTrack(trackIndex);
	}

	ViewType Spine::viewType() const {
		return kSpine_ViewType;
	}

	Vec2 Spine::client_size() {
		auto inl = _wraper.load();
		if (inl) {
			return { inl->_data->getWidth(), inl->_data->getHeight() };
		}
		return Vec2();
	}

	void Spine::onActivate() {
		if (level() == 0) {
			preRender().untask(this);
		} else {
			preRender().addtask(this);
		}
	}

	bool Spine::run_task(int64_t time, int64_t delta) {
		auto wraper = _wraper.load();
		if (wraper) {
			auto deltaTime = _speed * 0.000001f * delta; /* delta in seconds */
			wraper->_skeleton.update(deltaTime);

			// if (_preUpdateListener) _preUpdateListener(this);
			wraper->_state.update(deltaTime);
			wraper->_state.apply(wraper->_skeleton);

			wraper->_skeleton.updateWorldTransform();
			// if (_postUpdateListener) _postUpdateListener(this);

			return wraper->_skeleton.getColor().a != 0;
		}
		return false;
	}

	// Draw skeleton
	namespace {
		Rect computeBoundingRect(const float *coords, int vertexCount) {
			Qk_ASSERT(coords);
			Qk_ASSERT(vertexCount > 0);
			const float *v = coords;
			float minX = v[0];
			float minY = v[1];
			float maxX = minX;
			float maxY = minY;
			for (int i = 1; i < vertexCount; ++i) {
				v += 2;
				float x = v[0];
				float y = v[1];
				minX = std::min(minX, x);
				minY = std::min(minY, y);
				maxX = std::max(maxX, x);
				maxY = std::max(maxY, y);
			}
			return {{minX, minY}, {maxX - minX, maxY - minY}};
		}

		bool slotIsOutRange(Slot &slot, int startSlotIndex, int endSlotIndex) {
			const int index = slot.getData().getIndex();
			return startSlotIndex > index || endSlotIndex < index;
		}

		bool nothingToDraw(Slot &slot, int startSlotIndex, int endSlotIndex) {
			Attachment *attachment = slot.getAttachment();
			if (!attachment ||
				slotIsOutRange(slot, startSlotIndex, endSlotIndex) ||
				!slot.getBone().isActive())
				return true;
			const auto &attachmentRTTI = attachment->getRTTI();
			if (attachmentRTTI.isExactly(ClippingAttachment::rtti))
				return false;
			if (slot.getColor().a == 0)
				return true;
			if (attachmentRTTI.isExactly(RegionAttachment::rtti)) {
				if (static_cast<RegionAttachment *>(attachment)->getColor().a == 0)
					return true;
			} else if (attachmentRTTI.isExactly(MeshAttachment::rtti)) {
				if (static_cast<MeshAttachment *>(attachment)->getColor().a == 0)
					return true;
			}
			return false;
		}

		BlendMode getBlendMode(spine::BlendMode blendMode, bool premultipliedAlpha) {
			switch (blendMode) {
				case BlendMode_Additive:
					return premultipliedAlpha ? kPlus_BlendMode: kAdditive_BlendMode;
				case BlendMode_Multiply:
					return kMultiply_BlendMode;
				case BlendMode_Screen:
					return kScreen_BlendMode;
				default:
					return premultipliedAlpha ? kSrcOverExt_BlendMode: kSrcOver_BlendMode;
			}
		}

		Color ColorToColor4B(const spine::Color &color) {
			return Color(color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f);
		}

		void premultipliedAlpha(spine::Color &color) {
			color.r *= color.a;
			color.g *= color.a;
			color.b *= color.a;
		}

		void drawTriangles(Triangles &triangles,
			AttachmentVertices *attachment, Painter *painter, BlendMode blend
		) {
			Qk_ASSERT_NE(triangles.indexCount, 0);
			Qk_ASSERT_EQ(triangles.indexCount % 3, 0);
			auto src = attachment->_source;
			if (src->load()) {
				painter->canvas()->drawTriangles(triangles, attachment->_paint, blend);
			} else {
				src->onState().once<Window>([](auto e, auto ctx) {
					if (e.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
						if (ctx->root())
							ctx->preRender().mark_render(); // mark rerender
					}
				}, painter->window());
			}
		}
	} // namespace {

	void Spine::draw(Painter *painter) {
		auto self = _wraper.load();
		// Early exit if the skeleton is invisible.
		if (!self || self->_skeleton.getColor().a == 0) {
			return painter->visitView(this, matrix());
		}

		// Rect bb = computeBoundingRect(worldCoords, coordCount / 2);
		// if (cullRectangle(renderer, transform, bb)) {
		// 	VLA_FREE(worldCoords);
		// 	return;
		// }

		auto _matrix = painter->_matrix;
		painter->_matrix = &matrix();
		painter->canvas()->setMatrix(matrix());

		auto skeleton = &self->_skeleton;
		auto clipper = &self->_clipper;
		auto effect = self->_effect;
		if (effect)
			effect->begin(*skeleton);

		spine::Color color, darkColor;
		AttachmentVertices *attachmentV, *lastAttachmentV = nullptr;
		BlendMode lastBlendFunc;
		Triangles cmdTriangles;

		auto tmpBuff = &painter->_tempBuff; // Temp memory buffer allocation
		auto allocator = painter->_tempAllocator;

		for (int i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
			Slot *slot = skeleton->getDrawOrder()[i];

			if (nothingToDraw(*slot, _start_slot, _end_slot)) {
				clipper->clipEnd(*slot);
				continue;
			}

			auto attachment = slot->getAttachment();
			if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
				auto region = (RegionAttachment *)attachment;
				attachmentV = (AttachmentVertices *) region->getRendererObject();
				color = region->getColor();
				Vec2 dstPtr[4];
				region->computeWorldVertices(slot->getBone(), dstPtr->val, 0, 2);
				auto verts = attachmentV->_triangles.verts;
				for (int i = 0; i < 4; ++i, verts++) {
					verts->vertices[0] = dstPtr[i][0];
					verts->vertices[1] = dstPtr[i][1];
				}
			} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
				auto mesh = (MeshAttachment *)attachment;
				attachmentV = (AttachmentVertices *) mesh->getRendererObject();
				color = mesh->getColor();
				tmpBuff->reset(mesh->getWorldVerticesLength() * sizeof(float));
				float *dstPtr = (float *) tmpBuff->val();
				mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), dstPtr, 0, 2);
				auto verts = attachmentV->_triangles.verts;
				auto vertCount = attachmentV->_triangles.vertCount;
				for (int i = 0; i < vertCount; ++i, verts++, dstPtr+=2) {
					verts->vertices[0] = dstPtr[0];
					verts->vertices[1] = dstPtr[1];
				}
			} else if (attachment->getRTTI().isExactly(ClippingAttachment::rtti)) {
				clipper->clipStart(*slot, (ClippingAttachment *) attachment);
				continue;
			} else {
				clipper->clipEnd(*slot);
				continue;
			}

			color.a *= skeleton->getColor().a * slot->getColor().a * painter->opacity();
			if (color.a == 0) {
				clipper->clipEnd(*slot);
				continue;
			}
			color.r *= skeleton->getColor().r * slot->getColor().r;
			color.g *= skeleton->getColor().g * slot->getColor().g;
			color.b *= skeleton->getColor().b * slot->getColor().b;
			darkColor = slot->hasDarkColor() ? slot->getDarkColor(): spine::Color();

			// (r0*r1)*(a0*a1) == (r0*a0)*(r1*a1)
			if (attachmentV->_pma) { // premultiplied alpha
				if (slot->hasDarkColor())
					premultipliedAlpha(darkColor);
				premultipliedAlpha(color);
			}

			auto blendFunc = getBlendMode(slot->getData().getBlendMode(), attachmentV->_pma);
			auto triangles = attachmentV->_triangles;
			if (clipper->isClipping()) {
				clipper->clipTriangles(
					(float*)&triangles.verts[0].vertices,
					triangles.indices, triangles.indexCount,
					(float*)&triangles.verts[0].texCoords,
					sizeof(V3F_T2F_C4B_C4B) / 4
				);

				if (clipper->getClippedTriangles().size() == 0) {
					clipper->clipEnd(*slot);
					continue;
				}

				triangles.vertCount = clipper->getClippedVertices().size() / 2;
				triangles.indexCount = clipper->getClippedTriangles().size();
				triangles.indices = clipper->getClippedTriangles().buffer();
				tmpBuff->reset(triangles.vertCount * sizeof(V3F_T2F_C4B_C4B));
				triangles.verts = reinterpret_cast<V3F_T2F_C4B_C4B*>(tmpBuff->val());

				auto verts = clipper->getClippedVertices().buffer();
				auto uvs = clipper->getClippedUVs().buffer();
				auto vertex = triangles.verts;
				for (int v = 0, vv = 0; v < triangles.vertCount; ++v, vv+=2, ++vertex) {
					vertex->vertices[0] = verts[vv];
					vertex->vertices[1] = verts[vv + 1];
					vertex->texCoords[0] = uvs[vv];
					vertex->texCoords[1] = uvs[vv + 1];
				}
			}

			auto vertex = triangles.verts;
			if (effect) {
				for (int v = 0; v < triangles.vertCount; ++v, ++vertex) {
					auto colorCopy = color;
					auto darkColorCopy = darkColor;
					effect->transform(vertex->vertices[0], vertex->vertices[1],
						vertex->texCoords[0], vertex->texCoords[1], colorCopy, darkColorCopy);
					vertex->color = ColorToColor4B(colorCopy);
					vertex->color2 = ColorToColor4B(darkColorCopy);
				}
			} else {
				auto color4B = ColorToColor4B(color);
				auto darkColor4B = ColorToColor4B(darkColor);
				for (int v = 0; v < triangles.vertCount; ++v, ++vertex) {
					vertex->color = color4B;
					vertex->color2 = darkColor4B;
				}
			}

			Qk_ASSERT_EQ(triangles.indexCount % 3, 0); // must be triangles
			// global vertices and indices buffer
			// merge the same texture attachment to reduce drawcall

			if (lastAttachmentV && lastAttachmentV->_source != attachmentV->_source ||
					lastAttachmentV->_paint.bitfields != attachmentV->_paint.bitfields ||
					lastBlendFunc != blendFunc
			) {
				drawTriangles(cmdTriangles, lastAttachmentV, painter, lastBlendFunc);
				cmdTriangles = Triangles(); // reset
			}
			auto lastVertCount = cmdTriangles.vertCount;
			auto lastIndexCount = cmdTriangles.indexCount;
			cmdTriangles.verts = allocator[0].realloc(cmdTriangles.verts, lastVertCount + triangles.vertCount);
			cmdTriangles.indices = allocator[1].realloc(cmdTriangles.indices, lastIndexCount + triangles.indexCount);
			memcpy(cmdTriangles.verts + lastVertCount, triangles.verts, triangles.vertCount * sizeof(V3F_T2F_C4B_C4B));

			if (lastVertCount) {
				// The indeices is to be copied and rerejusted to the new vertices array
				for (int i = 0, ii = lastIndexCount; i < triangles.indexCount; ++i, ++ii)
					cmdTriangles.indices[ii] = triangles.indices[i] + lastVertCount;
			} else {
				memcpy(cmdTriangles.indices, triangles.indices, triangles.indexCount * sizeof(uint16_t));
			}
			cmdTriangles.vertCount += triangles.vertCount;
			cmdTriangles.indexCount += triangles.indexCount;
			lastAttachmentV = attachmentV;
			lastBlendFunc = blendFunc;
			clipper->clipEnd(*slot);
		}
		clipper->clipEnd();

		if (effect)
			effect->end();

		// commit last triangles
		if (lastAttachmentV) {
			drawTriangles(cmdTriangles, lastAttachmentV, painter, lastBlendFunc);
		}
		painter->visitView(this);
		painter->_matrix = _matrix;
		painter->canvas()->setMatrix(*_matrix); // restore previous matrix
	}

}
