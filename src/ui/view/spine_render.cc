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

#include "./spine.inl"

namespace qk {

	AttachmentEx::AttachmentEx(
		AtlasPage* page, uint16_t *triangles, uint32_t vertCount, uint32_t indexCount
	)
		: _source(static_cast<ImageSource*>(page->texture))
		, _width(page->width), _height(page->height)
	{
		if (_source)
			_source->retain();
		_triangles.verts = shared_allocator()->alloc<V3F_T2F_C4B_C4B>(vertCount);
		_triangles.vertCount = vertCount;
		_triangles.indices = triangles;
		_triangles.indexCount = indexCount;
		_paint.image = _source;
		_paint.filterMode = page->magFilter == TextureFilter_Linear ?
			PaintImage::kLinear_FilterMode: PaintImage::kNearest_FilterMode;
		if (page->pma)
			_source->set_premulFlags(ImageSource::kOnlyMark_PremulFlags);

		switch (page->minFilter) {
			case TextureFilter_MipMapLinearNearest:
				_paint.mipmapMode = PaintImage::kLinearNearest_MipmapMode;
				break;
			case TextureFilter_MipMapNearestLinear:
				_paint.mipmapMode = PaintImage::kNearestLinear_MipmapMode;
				break;
			case TextureFilter_Linear:
			case TextureFilter_MipMapLinearLinear:
				_paint.mipmapMode = PaintImage::kLinear_MipmapMode;
				break;
			default:
				_paint.mipmapMode = PaintImage::kNone_MipmapMode;
				break;
		}

		switch (page->uWrap) {
			case TextureWrap_MirroredRepeat:
				_paint.tileModeX = PaintImage::kMirror_TileMode;
				break;
			case TextureWrap_ClampToEdge:
				_paint.tileModeX = PaintImage::kClamp_TileMode;
				break;
			case TextureWrap_Repeat:
				_paint.tileModeX = PaintImage::kRepeat_TileMode;
				break;
		}

		switch (page->vWrap) {
			case TextureWrap_MirroredRepeat:
				_paint.tileModeY = PaintImage::kMirror_TileMode;
				break;
			case TextureWrap_ClampToEdge:
				_paint.tileModeY = PaintImage::kClamp_TileMode;
				break;
			case TextureWrap_Repeat:
				_paint.tileModeY = PaintImage::kRepeat_TileMode;
				break;
		}

		Hash5381 hash;
		hash.updateu64(uintptr_t(_source));
		hash.updateu32(_paint.bitfields);
		_hashCode = hash.hashCode();
	}

	AttachmentEx::~AttachmentEx() {
		shared_allocator()->free(_triangles.verts);
		if (_source)
			_source->release();
	}

	////////////////////////////////////////////////////////////////////////////////////

	static bool nothingToDraw(Slot &slot) {
		Attachment *attachment = slot.getAttachment();
		if (!attachment || !slot.getBone().isActive())
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

	static BlendMode getBlendMode(Slot *slot, bool premultipliedAlpha) {
		switch (slot->getData().getBlendMode()) {
			case BlendMode_Additive:
				return premultipliedAlpha ? kPlus_BlendMode: kAdditive_BlendMode;
			case BlendMode_Multiply:
				return kMultiply_BlendMode;
			case BlendMode_Screen:
				return kScreen_BlendMode;
			default:
				return premultipliedAlpha ? kSrcOverPre_BlendMode: kSrcOver_BlendMode;
		}
	}

	static Color4f toColor4f(const spine::Color &color) {
		return (Color4f&)color.r;
	}

	static void drawTriangles(Painter *painter, TrianglesEx &triangles, AttachmentEx *ex) {
		Qk_ASSERT_NE(triangles.indexCount, 0);
		Qk_ASSERT_EQ(triangles.indexCount % 3, 0);
		auto src = ex->_source;
		if (src->load()) {
			Paint paint;
			paint.fill.color = Color4f(1,1,1,1); // white
			paint.fill.image = &ex->_paint;
			paint.blendMode = triangles.blendMode;
			painter->canvas()->drawTriangles(triangles, paint);
		} else {
			src->onState().on<Window>([](auto e, auto win) {
				if (e.data() & (ImageSource::kSTATE_LOAD_COMPLETE | ImageSource::kSTATE_LOAD_ERROR)) {
					if (e.data() & ImageSource::kSTATE_LOAD_COMPLETE) {
						if (win->root())
							win->preRender().mark_render(); // mark rerender
					}
					e.off(); // off current listeners
				}
			}, painter->window());
		}
	}

	static void tryDrawLastTriangles(
		Painter *painter,
		TrianglesEx &triangles,
		AttachmentEx *lastEx, AttachmentEx *ex, Slot *slot
	) {
		auto blendMode = getBlendMode(slot, ex->_source->premultipliedAlpha());
		if (lastEx) {
			if (lastEx->_hashCode != ex->_hashCode || triangles.blendMode != blendMode) {
				drawTriangles(painter, triangles, lastEx);
				triangles = {}; // reset
			}
		}
		triangles.blendMode = blendMode;
	}

	//////////////////////////////////////////////////////////////////////////////

	void Spine::draw(Painter *painter) {
		auto skel = _skel.load(std::memory_order_acquire);
		// Early exit if the skeleton is invisible.
		if (!skel || !skel->_skeleton.getColor().a || !painter->_color.a()) {
			return Entity::draw(painter);
		}
		auto skeleton = &skel->_skeleton;
		auto clipper = _clipper.get();
		Color4f color = toColor4f(skel->_skeleton.getColor()).premul_alpha().mul(painter->_color);
		Color4f light, dark;
		AttachmentEx *ex, *lastEx = nullptr;
		TrianglesEx cmdTriangles;

		auto lastMatrix = painter->matrix();
		auto mat = matrix();
		mat.translate({_skel_origin.x() - _origin_value.x(), _skel_origin.y() - _origin_value.y()});
		mat.scale_y(-1); // Flip Y axis for Spine
		painter->set_matrix(&mat);

		_mutex.lock();

		auto tmpBuff = &painter->_tempBuff; // Temp memory buffer allocation
		auto allocator = painter->_tempAllocator;

		for (size_t i = 0, n = skeleton->getSlots().size(); i < n; ++i) {
			Slot *slot = skeleton->getDrawOrder()[i];

			if (nothingToDraw(*slot)) {
				clipper->clipEnd(*slot);
				continue;
			}

			auto attachment = slot->getAttachment();
			if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
				auto region = static_cast<RegionAttachmentEx*>(attachment);
				ex = region->ex;
				light = toColor4f(region->getColor());
				tryDrawLastTriangles(painter, cmdTriangles, lastEx, ex, slot);
				auto z = cmdTriangles.zDepthTotal;
				Vec3 dstPtr[4] = {{0,0,z}, {0,0,z}, {0,0,z}, {0,0,z}};
				region->computeWorldVertices(*slot, dstPtr->val, 0, 3);
				auto verts = ex->_triangles.verts;
				for (int i = 0; i < 4; ++i, verts++) {
					verts->vertices = dstPtr[i];
				}
			} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
				auto mesh = static_cast<MeshAttachmentEx*>(attachment);
				ex = mesh->ex;
				light = toColor4f(mesh->getColor());
				tryDrawLastTriangles(painter, cmdTriangles, lastEx, ex, slot);
				tmpBuff->reset((uint32_t)mesh->getWorldVerticesLength() * sizeof(float));
				float *dstPtr = (float *) tmpBuff->val();
				mesh->computeWorldVertices(*slot, 0, mesh->getWorldVerticesLength(), dstPtr, 0, 2);
				auto verts = ex->_triangles.verts;
				auto vertCount = ex->_triangles.vertCount;
				for (int i = 0; i < vertCount; ++i, verts++, dstPtr+=2) {
					verts->vertices[0] = dstPtr[0];
					verts->vertices[1] = dstPtr[1];
					verts->vertices[2] = cmdTriangles.zDepthTotal;
				}
			} else if (attachment->getRTTI().isExactly(ClippingAttachment::rtti)) {
				clipper->clipStart(*slot, (ClippingAttachment *) attachment);
				continue;
			} else {
				clipper->clipEnd(*slot);
				continue;
			}

			if (light.a() == 0) {
				clipper->clipEnd(*slot);
				continue;
			}
			light *= toColor4f(slot->getColor());
			if (slot->hasDarkColor()) {
				dark = toColor4f(slot->getDarkColor());
				cmdTriangles.isDarkColor = true;
			} else {
				dark = Color4f();
			}
			if (ex->_source->premultipliedAlpha()) { // premultiplied alpha
				if (slot->hasDarkColor())
					dark = dark.premul_alpha();
				light = light.premul_alpha().mul(color);
			} else {
				// unpremultiplied alpha
				light = light.mul(color.recover_unpremul_alpha());
			}

			auto triangles = ex->_triangles;
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

				triangles.vertCount = (uint32_t)clipper->getClippedVertices().size() / 2;
				triangles.indexCount = (uint32_t)clipper->getClippedTriangles().size();
				triangles.indices = clipper->getClippedTriangles().buffer();
				tmpBuff->reset(triangles.vertCount * sizeof(V3F_T2F_C4B_C4B));
				triangles.verts = reinterpret_cast<V3F_T2F_C4B_C4B*>(tmpBuff->val());

				auto verts = clipper->getClippedVertices().buffer();
				auto uvs = clipper->getClippedUVs().buffer();
				auto vertex = triangles.verts;
				for (int v = 0, vv = 0; v < triangles.vertCount; ++v, vv+=2, ++vertex) {
					vertex->vertices[0] = verts[vv];
					vertex->vertices[1] = verts[vv + 1];
					vertex->vertices[2] = cmdTriangles.zDepthTotal;
					vertex->texCoords[0] = uvs[vv];
					vertex->texCoords[1] = uvs[vv + 1];
				}
			}

			auto vertex = triangles.verts;
			auto light4B = light.to_color();
			auto dark4B = dark.to_color();
			for (int v = 0; v < triangles.vertCount; ++v, ++vertex) {
				vertex->lightColor = light4B;
				vertex->darkColor = dark4B;
			}

			Qk_ASSERT_EQ(triangles.indexCount % 3, 0); // must be triangles
			// global vertices and indices buffer
			// merge the same texture attachment to reduce drawcall

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
			lastEx = ex;
			clipper->clipEnd(*slot);
			cmdTriangles.zDepthTotal += zDepthNextUnit;
		}
		clipper->clipEnd();
		_mutex.unlock();

		// commit last triangles
		if (lastEx) {
			drawTriangles(painter, cmdTriangles, lastEx);
		}
		debugDraw(painter); // draw debug bounds
		if (first()) {
			painter->set_matrix(&matrix());
			painter->visitView(this);
		}
		painter->set_matrix(lastMatrix); // restore previous matrix
	}

}
