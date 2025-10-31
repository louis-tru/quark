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
#include "../../util/fs.h"
#include "../../errno.h"

SpineExtension *spine::getDefaultExtension() {
	return new DefaultSpineExtension();
}

namespace qk {
	static QkTextureLoader *textureLoader = new QkTextureLoader();
	static Dict<String, Sp<SkeletonData>> _skeletonCache;

	void QkTextureLoader::load(AtlasPage &page, const spine::String &path) {
		auto source = shared_app()->imgPool()->get(path.buffer());
		Qk_ASSERT(source, "Invalid image");
		Qk_ASSERT_NE(page.width, 0, "Invalid image width");
		Qk_ASSERT_NE(page.height, 0, "Invalid image height");
		source->retain();
		page.texture = source;
	}

	void QkTextureLoader::unload(void *texture) {
		((ImageSource *) texture)->release();
	}

	template<class T>
	T* SkeletonData::QkAtlasAttachmentLoader::newAttachmentEx(Skin &skin,
		cSPString &name, cSPString &path, Sequence *sequence) {
		auto attachment = new T(name);
		if (sequence) {
			if (!loadSequence(_atlas, path, sequence)) return nullptr;
		} else {
			AtlasRegion *region = findRegion(path);
			if (!region) return nullptr;
			attachment->setRegion(region);
		}
		return attachment;
	}

	RegionAttachment *SkeletonData::QkAtlasAttachmentLoader::newRegionAttachment(Skin &skin,
		cSPString &name, cSPString &path, Sequence *sequence) {
		return newAttachmentEx<RegionAttachmentEx>(skin, name, path, sequence);
	}

	MeshAttachment *SkeletonData::QkAtlasAttachmentLoader::newMeshAttachment(Skin &skin,
		cSPString &name, cSPString &path, Sequence *sequence) {
		return newAttachmentEx<MeshAttachmentEx>(skin, name, path, sequence);
	}

	void SkeletonData::QkAtlasAttachmentLoader::configureAttachment(Attachment *attachment) {
		if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
			setAttachmentEx(static_cast<RegionAttachmentEx *>(attachment));
		} else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
			setAttachmentEx(static_cast<MeshAttachmentEx *>(attachment));
		}
	}

	static uint16_t quadTriangles[6] = {0, 1, 2, 2, 3, 0};

	void SkeletonData::QkAtlasAttachmentLoader::setAttachmentEx(RegionAttachmentEx *attachment) {
		auto region = static_cast<AtlasRegion*>(attachment->getRegion());
		auto ex = new AttachmentEx(region->page, quadTriangles, 4, 6);
		V3F_T2F_C4B_C4B *vertices = ex->_triangles.verts;
		for (int i = 0, ii = 0; i < 4; ++i, ii += 2) {
			vertices[i].texCoords[0] = attachment->getUVs()[ii];
			vertices[i].texCoords[1] = attachment->getUVs()[ii + 1];
		}
		attachment->ex = ex;
	}

	void SkeletonData::QkAtlasAttachmentLoader::setAttachmentEx(MeshAttachmentEx *attachment) {
		auto region = static_cast<AtlasRegion*>(attachment->getRegion());
		auto ex = new AttachmentEx(
			region->page,
			attachment->getTriangles().buffer(),
			uint32_t(attachment->getWorldVerticesLength() >> 1),
			uint32_t(attachment->getTriangles().size())
		);
		V3F_T2F_C4B_C4B *vertices = ex->_triangles.verts;
		for (size_t i = 0, ii = 0, nn = attachment->getWorldVerticesLength(); ii < nn; ++i, ii += 2) {
			vertices[i].texCoords[0] = attachment->getUVs()[ii];
			vertices[i].texCoords[1] = attachment->getUVs()[ii + 1];
		}
		attachment->ex = ex;
	}

	static String get_atlas_path(cString &path, bool json) {
		String atlasP;
		atlasP = path.replace(json ? "-ess.json": "-ess.skel", ".atlas");
		if (atlasP.length() != path.length()) {
			return atlasP;
		}
		atlasP = path.replace(json ? "-pro.json": "-pro.skel", ".atlas");
		if (atlasP.length() != path.length()) {
			return atlasP;
		}
		atlasP = path.replace(json ? ".json": ".skel", ".atlas");
		if (atlasP.length() != path.length()) {
			return atlasP;
		}
		return atlasP;
	}

	/////////////// SkeletonData ///////////////

	Sp<SkeletonData> SkeletonData::Make(cString &skelPath, cString &atlasPath, float scale) throw(Error)
	{
		Qk_IfThrow(!skelPath.isEmpty(), ERR_SPINE_SKELETON_PATH_CANNOT_EMPTY, "skeleton path cannot empty");
		auto json = skelPath.lastIndexOf(".json") != -1;
		auto atlasP = atlasPath.isEmpty() ? get_atlas_path(skelPath, json) : atlasPath;
		Qk_IfThrow(!atlasP.isEmpty(), ERR_SPINE_ATLAS_PATH_CANNOT_EMPTY, "atlas path cannot empty");
		auto key = fs_format("%s|%s|%f", *skelPath, *atlasP, scale);
		Sp<qk::SkeletonData> *out;
		if (_skeletonCache.get(key, out))
			return *out;
		auto sk_buff = fs_reader()->read_file_sync(skelPath);
		auto atlas_buff = fs_reader()->read_file_sync(atlasP);
		return _skeletonCache.set(key, _Make(sk_buff, atlas_buff, fs_dirname(skelPath), scale, json));
	}

	Sp<SkeletonData> SkeletonData::Make(cBuffer &skelBuff,cString &atlasPath, float scale) throw(Error)
	{
		auto key0 = hash(skelBuff.val(), skelBuff.length());
		auto key = fs_format("%s|%s|%f", *key0, *atlasPath, scale);
		Sp<qk::SkeletonData> *out;
		if (_skeletonCache.get(key, out))
			return *out;
		auto atlas_buff = fs_reader()->read_file_sync(atlasPath);
		return _skeletonCache.set(key, _Make(skelBuff, atlas_buff, fs_dirname(atlasPath), scale, false));
	}

	Sp<SkeletonData> SkeletonData::Make(cBuffer &skel,
		cBuffer &atlasBuf, cString &dir, float scale) throw(Error)
	{
		auto key0 = hash(skel.val(), skel.length());
		auto key1 = hash(atlasBuf.val(), atlasBuf.length());
		auto key = fs_format("%s|%s|%s|%f", *key0, *key1, *dir, scale);
		Sp<qk::SkeletonData> *out;
		if (_skeletonCache.get(key, out))
			return *out;
		return _skeletonCache.set(key, _Make(skel, atlasBuf, dir, scale, false));
	}

	SkeletonData* SkeletonData::_Make(cBuffer &skel,
		cBuffer &atlasBuf, cString &dir, float scale, bool json) throw(Error)
	{
		Sp<Atlas> atlas = new Atlas(*atlasBuf, atlasBuf.length(), dir.c_str(), textureLoader, true);
		Sp<QkAtlasAttachmentLoader> loader = new QkAtlasAttachmentLoader(*atlas);
		String err;
		spine::SkeletonData *data;
		if (json) {
			SkeletonJson reader(*loader);
			reader.setScale(scale);
			data = reader.readSkeletonData(skel.val());
			err = reader.getError().buffer();
		} else {
			SkeletonBinary reader(*loader);
			reader.setScale(scale);
			data = reader.readSkeletonData((const uint8_t*)skel.val(), skel.length());
			err = reader.getError().buffer();
		}
		Qk_IfThrow(data, ERR_SPINE_LOAD_SKELETON_DATA_FAIL, (err.isEmpty() ? "Error reading skeleton data.": err));

		return new SkeletonData(data, atlas.collapse(), loader.collapse());
	}

	SkeletonData::SkeletonData(spine::SkeletonData* data, Atlas* atlas, QkAtlasAttachmentLoader* loader)
		: _data(data), _atlas(atlas), _atlasLoader(loader) {
	}

	SkeletonData::~SkeletonData() {
		Releasep(_data);
		Releasep(_atlas);
		Releasep(_atlasLoader);
	}

	uint64_t SkeletonData::hashCode() const {
		return uintptr_t(this);
	}
}
