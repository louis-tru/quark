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

	static uint16_t quadTriangles[6] = {0, 1, 2, 2, 3, 0};

	struct V3F_T2F_C4B_C4B {
		/// vertices (3F)
		Vec3     vertices;          // 12 bytes
		// tex coords (2F)
		Vec2     texCoords;         // 8 bytes
		/// color (4B)
		Color    color;             // 4 bytes
		/// color2 (4B)
		Color    color2;            // 4 bytes
	};

	struct Triangles {
		/**Vertex data pointer.*/
		V3F_T2F_C4B_C4B* verts = nullptr;
		/**Index data pointer.*/
		uint16_t* indices = nullptr;
		/**The number of vertices.*/
		uint32_t vertCount = 0;
		/**The number of indices.*/
		uint32_t indexCount = 0;
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
		Sp<AtlasAttachmentLoader> loader = new AtlasAttachmentLoader(*atlas);

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
			: _host(host), _data(data)
			, _skdata(data->_data), _skeleton(nullptr)
			, _clipper(nullptr), _effect(nullptr)
		{
			_skeleton = new Skeleton(data->_data);
			_clipper = new SkeletonClipping();
		}
		~SkeletonWraper() {
			Releasep(_skeleton);
			Releasep(_clipper);
			Releasep(_effect);
		}
		void destroy() {
			_host->preRender().async_call([](auto self, auto arg) {
				self->Object::destroy();
			}, this, 0);
		}
		Spine *_host;
		Sp<SkeletonData> _data;
		spine::SkeletonData* _skdata;
		spine::Skeleton *_skeleton;
		spine::SkeletonClipping *_clipper;
		spine::VertexEffect *_effect;
	};

	Spine::Spine(): SpriteView()
		, _wraper(nullptr)
		, _startSlotIndex(0), _endSlotIndex(std::numeric_limits<int>::max())
	{}

	void Spine::destroy() {
		Release(_wraper.load());
		_wraper.store(nullptr);
		View::destroy(); // Call parent destroy
	}

	void Spine::set_skeleton(SkeletonData *data) {
		if (!_wraper || _wraper.load()->_data.get() != data) {
			if (_wraper) {
				Release(_wraper.load());
				_wraper.store(nullptr);
			}
			if (data) {
				_wraper.store(NewRetain<SkeletonWraper>(this, data));
			}
		}
	}

	SkeletonData* Spine::skeleton() {
		if (_wraper) {
			return _wraper.load()->_data.get();
		}
		return nullptr;
	}

	ViewType Spine::viewType() const {
		return kSpine_ViewType;
	}

	Vec2 Spine::client_size() {
		auto inl = _wraper.load();
		if (inl) {
			return {
				inl->_skdata->getWidth(),
				inl->_skdata->getHeight(),
			};
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
		auto inl = _wraper.load();
		if (inl) {
			inl->_skeleton->update(delta * 0.000001f/* delta in seconds */);
			return inl->_skeleton->getColor().a != 0;
		}
		return false;
	}

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

		int computeTotalCoordCount(Skeleton &skeleton, int startSlotIndex, int endSlotIndex) {
			int coordCount = 0;
			for (size_t i = 0; i < skeleton.getSlots().size(); ++i) {
				Slot &slot = *skeleton.getSlots()[i];
				if (nothingToDraw(slot, startSlotIndex, endSlotIndex)) {
					continue;
				}
				Attachment *const attachment = slot.getAttachment();
				if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
					coordCount += 8;
				} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
					MeshAttachment *const mesh = static_cast<MeshAttachment *>(attachment);
					coordCount += mesh->getWorldVerticesLength();
				}
			}
			return coordCount;
		}

		void transformWorldVertices(float *dstCoord, int coordCount,
			Skeleton &skeleton, int startSlotIndex, int endSlotIndex)
		{
			float *dstPtr = dstCoord;
			float *const dstEnd = dstCoord + coordCount;

			for (size_t i = 0; i < skeleton.getSlots().size(); ++i) {
				Slot &slot = *skeleton.getDrawOrder()[i];// match the draw order of SkeletonRenderer::Draw
				if (nothingToDraw(slot, startSlotIndex, endSlotIndex)) {
					continue;
				}
				Attachment *const attachment = slot.getAttachment();
				if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
					RegionAttachment *const region = static_cast<RegionAttachment *>(attachment);
					Qk_ASSERT(dstPtr + 8 <= dstEnd);
					region->computeWorldVertices(slot.getBone(), dstPtr, 0, 2);
					dstPtr += 8;
				} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
					MeshAttachment *const mesh = static_cast<MeshAttachment *>(attachment);
					Qk_ASSERT(dstPtr + mesh->getWorldVerticesLength() <= dstEnd);
					mesh->computeWorldVertices(slot, 0, mesh->getWorldVerticesLength(), dstPtr, 0, 2);
					dstPtr += mesh->getWorldVerticesLength();
				}
			}
			Qk_ASSERT_EQ(dstPtr, dstEnd);
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
			return Color{color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f};
		}
	}

	void Spine::draw(Painter *painter) {
		auto wraper = _wraper.load();
		auto skeleton = wraper->_skeleton;
		// Early exit if the skeleton is invisible.
		if (skeleton->getColor().a == 0) {
			return painter->visitView(this, matrix());
		}

		int coordCount = computeTotalCoordCount(*skeleton, _startSlotIndex, _endSlotIndex);
		if (coordCount == 0) {
			return painter->visitView(this, matrix());
		}
		Qk_ASSERT_EQ(coordCount % 2, 0);

		VLA(float, worldCoords, coordCount);
		transformWorldVertices(worldCoords, coordCount, *skeleton, _startSlotIndex, _endSlotIndex);

#if CC_USE_CULLING
		// Rect bb = computeBoundingRect(worldCoords, coordCount / 2);
		// if (cullRectangle(renderer, transform, bb)) {
		// 	VLA_FREE(worldCoords);
		// 	return;
		// }
#endif

		float *worldCoordPtr = worldCoords;
		auto clipper = wraper->_clipper;
		auto effect = wraper->_effect;
		if (effect) {
			effect->begin(*skeleton);
		}

		spine::Color color, darkColor;
		AtlasRegion *region;

		auto allocateTriangles = [](
			Triangles &triangles,
			uint16_t *indices,
			float *uvs,
			float *worldCoord,
			int verticesCount, int indicesCount
		) {
			// triangles.verts = batch->allocateVertices(attachmentVertices->_triangles.vertCount);
			triangles.indices = indices;
			triangles.vertCount = verticesCount;
			triangles.indexCount = indicesCount;
			// Copy world vertices to triangle vertices.
			auto uvsv = (Vec2*)uvs;
			auto verts = triangles.verts;
			for (int i = 0; i < verticesCount; ++i, ++verts, worldCoord+=2) {
				verts->vertices[0] = worldCoord[0];
				verts->vertices[1] = worldCoord[1];
				verts->texCoords = uvsv[i]; // Copy coord uvs
			}
		};

		for (int i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
			Slot *slot = skeleton->getDrawOrder()[i];

			if (nothingToDraw(*slot, _startSlotIndex, _endSlotIndex)) {
				clipper->clipEnd(*slot);
				continue;
			}
			Triangles triangles;

			if (slot->getAttachment()->getRTTI().isExactly(RegionAttachment::rtti)) {
				auto attachment = (RegionAttachment *) slot->getAttachment();
				region = (AtlasRegion *) attachment->getRendererObject();
				color = attachment->getColor();
				allocateTriangles(
					triangles, // output triangles
					quadTriangles, // index buffer
					attachment->getUVs().buffer(), // UV buffer
					worldCoordPtr, // world coordinates
					4, // vertex count
					6  // index count
				);
				worldCoordPtr += 8;
			}
			else if (slot->getAttachment()->getRTTI().isExactly(MeshAttachment::rtti)) {
				auto attachment = (MeshAttachment *) slot->getAttachment();
				region = (AtlasRegion *) attachment->getRendererObject();
				color = attachment->getColor();
				allocateTriangles(
					triangles, // output triangles
					attachment->getTriangles().buffer(),
					attachment->getUVs().buffer(),
					worldCoordPtr,
					attachment->getWorldVerticesLength() >> 1, // vertex count
					attachment->getTriangles().size() // index count
				);
				worldCoordPtr += attachment->getWorldVerticesLength();
			}
			else if (slot->getAttachment()->getRTTI().isExactly(ClippingAttachment::rtti)) {
				clipper->clipStart(*slot, (ClippingAttachment *) slot->getAttachment());
				continue;
			}
			else {
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

			auto blendFunc = getBlendMode(slot->getData().getBlendMode(), region->page->pma);

			if (clipper->isClipping()) {
				clipper->clipTriangles(
					(float*)&triangles.verts[0].vertices,
					triangles.indices,
					triangles.indexCount,
					(float*)&triangles.verts[0].texCoords,
					sizeof(V3F_T2F_C4B_C4B) / 4
				);
				// batch->deallocateVertices(triangles.vertCount);

				if (clipper->getClippedTriangles().size() == 0) {
					clipper->clipEnd(*slot);
					continue;
				}
				triangles.vertCount = clipper->getClippedVertices().size() / 2;
				triangles.indexCount = clipper->getClippedTriangles().size();
				// triangles.verts = batch->allocateVertices(triangles.vertCount);
				// triangles.indices = batch->allocateIndices(triangles.indexCount);
				memcpy(triangles.indices, clipper->getClippedTriangles().buffer(),
					sizeof(uint16_t) * clipper->getClippedTriangles().size());

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
			clipper->clipEnd(*slot);
		}
		clipper->clipEnd();

		if (effect) {
			effect->end();
		}
		VLA_FREE(worldCoords);

		auto canvas = painter->canvas();
		auto _matrix = painter->_matrix;
		painter->_matrix = &matrix();
		canvas->setMatrix(matrix());
		// TODO ...
		painter->visitView(this);
		painter->_matrix = _matrix;
		canvas->setMatrix(*_matrix); // restore previous matrix
	}

}
