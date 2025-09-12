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
	static uint16_t quadTriangles[6] = {0, 1, 2, 2, 3, 0};
	static Rect computeBoundingRect(const float *coords, int vertexCount);
	static bool nothingToDraw(Slot &slot);
	static BlendMode getBlendMode(spine::BlendMode blendMode, bool premultipliedAlpha);
	static Color ColorToColor4B(const spine::Color &color);
	static void premultipliedAlpha(spine::Color &color);
	static void drawTriangles(Triangles &triangles,
		AttachmentVertices *attachment, Painter *painter, BlendMode blend);

	AttachmentVertices::AttachmentVertices(AtlasPage* page, uint16_t *triangles, uint32_t vertCount, uint32_t indexCount)
		: _source(static_cast<ImageSource*>(page->getRendererObject()))
		, _width(page->width), _height(page->height)
		, _pma(page->pma)
	{
		if (_source)
			_source->retain();
		_triangles.verts = shared_allocator()->alloc<V3F_T2F_C4B_C4B>(vertCount);
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
			case TextureFilter_Linear:
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

	AttachmentVertices::~AttachmentVertices() {
		shared_allocator()->free(_triangles.verts);
		if (_source)
			_source->release();
	}

	void QkTextureLoader::load(AtlasPage &page, const spine::String &path) {
		auto source = shared_app()->imgPool()->get(path.buffer());
		Qk_ASSERT(source, "Invalid image");
		Qk_ASSERT_NE(page.width, 0, "Invalid image width");
		Qk_ASSERT_NE(page.height, 0, "Invalid image height");
		source->retain();
		page.setRendererObject(source);
	}

	void QkTextureLoader::unload(void *source) {
		((ImageSource *) source)->release();
	}

	void SkeletonData::QkAtlasAttachmentLoader::configureAttachment(Attachment *attachment) {
		if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
			setAttachmentVertices((RegionAttachment *) attachment);
		} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
			setAttachmentVertices((MeshAttachment *) attachment);
		}
	}

	void SkeletonData::QkAtlasAttachmentLoader::deleteAttachmentVertices(void *vertices) {
		delete (AttachmentVertices *) vertices;
	}

	void SkeletonData::QkAtlasAttachmentLoader::setAttachmentVertices(RegionAttachment *attachment) {
		auto region = (AtlasRegion *) attachment->getRendererObject();
		auto attachmentVertices = new AttachmentVertices(region->page, quadTriangles, 4, 6);
		V3F_T2F_C4B_C4B *vertices = attachmentVertices->_triangles.verts;
		for (int i = 0, ii = 0; i < 4; ++i, ii += 2) {
			vertices[i].texCoords[0] = attachment->getUVs()[ii];
			vertices[i].texCoords[1] = attachment->getUVs()[ii + 1];
		}
		attachment->setRendererObject(attachmentVertices, deleteAttachmentVertices);
	}

	void SkeletonData::QkAtlasAttachmentLoader::setAttachmentVertices(MeshAttachment *attachment) {
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

	void Spine::draw(Painter *painter) {
		auto skel = _skel.load(std::memory_order_acquire);
		// Early exit if the skeleton is invisible.
		if (!skel || skel->_skeleton.getColor().a == 0){
			return painter->visitView(this, &matrix());
		}
		_other->_mutex.lock();

		auto lastMatrix = painter->matrix();
		auto mat = matrix();
		mat.translate({_skel_origin.x() - _origin_value.x(), _skel_origin.y() - _origin_value.y()});
		mat.scale_y(-1); // Flip Y axis for Spine
		painter->set_matrix(&mat);

		auto skeleton = &skel->_skeleton;
		auto clipper = &_other->_clipper;
		auto effect = _other->_effect;
		if (effect)
			effect->begin(*skeleton);

		spine::Color color, darkColor;
		AttachmentVertices *attachmentV, *lastAttachmentV = nullptr;
		BlendMode lastBlendFunc;
		Triangles cmdTriangles;
		float zDepth = 0;

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
				auto region = (RegionAttachment *)attachment;
				attachmentV = (AttachmentVertices *) region->getRendererObject();
				color = region->getColor();
				Vec3 dstPtr[4] = {{0,0,zDepth}, {0,0,zDepth}, {0,0,zDepth}, {0,0,zDepth}};
				region->computeWorldVertices(slot->getBone(), dstPtr->val, 0, 3);
				auto verts = attachmentV->_triangles.verts;
				for (int i = 0; i < 4; ++i, verts++) {
					verts->vertices = dstPtr[i];
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
					verts->vertices[2] = zDepth;
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
					vertex->vertices[2] = zDepth;
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

			if (lastAttachmentV) {
				if (lastAttachmentV->_source != attachmentV->_source ||
						lastAttachmentV->_paint.bitfields != attachmentV->_paint.bitfields ||
						lastBlendFunc != blendFunc) {
					cmdTriangles.zDepthTotal = zDepth;
					drawTriangles(cmdTriangles, lastAttachmentV, painter, lastBlendFunc);
					cmdTriangles = Triangles(); // reset
					zDepth = 0;
				}
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
			zDepth += zDepthNextUnit;
		}
		clipper->clipEnd();

		if (effect)
			effect->end();

		_other->_mutex.unlock();

		// commit last triangles
		if (lastAttachmentV) {
			cmdTriangles.zDepthTotal = zDepth;
			drawTriangles(cmdTriangles, lastAttachmentV, painter, lastBlendFunc);
		}
		painter->set_matrix(&matrix());
		painter->visitView(this);
		painter->set_matrix(lastMatrix); // restore previous matrix
	}

	//////////////////////////////////////////////////////////////////////////////

	static Rect computeBoundingRect(const float *coords, int vertexCount) {
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

	static BlendMode getBlendMode(spine::BlendMode blendMode, bool premultipliedAlpha) {
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

	static Color ColorToColor4B(const spine::Color &color) {
		return Color(color.r * 255.f, color.g * 255.f, color.b * 255.f, color.a * 255.f);
	}

	static void premultipliedAlpha(spine::Color &color) {
		color.r *= color.a;
		color.g *= color.a;
		color.b *= color.a;
	}

	static void drawTriangles(Triangles &triangles, AttachmentVertices *attachment,
		Painter *painter, BlendMode blend
	) {
		Qk_ASSERT_NE(triangles.indexCount, 0);
		Qk_ASSERT_EQ(triangles.indexCount % 3, 0);
		auto src = attachment->_source;
		if (src->load()) {
			painter->canvas()->drawTriangles(triangles, attachment->_paint, blend);
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
}
