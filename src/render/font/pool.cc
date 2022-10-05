/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include "../../util/fs.h"
#include "../../util/hash.h"
#include "./pool.h"
#include <skia/core/SkFontMgr.h>
#include <skia/core/SkTypeface.h>
#include <skia/core/SkString.h>
#include <skia/core/SkData.h>

namespace quark {

	#define SkMgr(impl) static_cast<SkFontMgr*>(impl)

	// ---------------------- F o n t . P o o l --------------------------

	FontPool::FontPool(Application* host)
		: _host(host)
		, _last_65533(0)
		, _impl(SkFontMgr::RefDefault().get())
	{
		Qk_Assert(_impl);
		SkMgr(_impl)->ref();
		initialize();
	}

	FontPool::~FontPool() {
		SkMgr(_impl)->unref();
		_impl = nullptr;
	}

	int32_t FontPool::count_families() const {
		return SkMgr(_impl)->countFamilies();
	}

	Array<String> FontPool::familys() const {
		Array<String> familyNames;
		auto mgr = SkMgr(_impl);
		auto count = mgr->countFamilies();
		SkString familyName;
		for (int i = 0; i < count; i++) {
			mgr->getFamilyName(i, &familyName);
			familyNames.push(String(familyName.c_str(), (uint32_t)familyName.size()));
		}
		return familyNames;
	}

	FFID FontPool::getFFID(const Array<String>& familys) {

		Array<String> newFamilys;
		SimpleHash hash;

		for (auto& i: familys) {
			String s = i.trim();
			if ( !s.is_empty() ) {
				newFamilys.push(s);
				hash.update(s);
			}
		}

		auto it = _FFIDs.find(hash.hash_code());
		if (it != _FFIDs.end()) {
			return it->value;
		}

		FFID id = new FontFamilys(this, newFamilys);
		_FFIDs[ hash.hash_code() ] = id;
		return id;
	}

	FFID FontPool::getFFID(cString familys) {
		if ( familys.is_empty() )
			return getFFID(Array<String>());
		else
			return getFFID(familys.split(","));
	}

	Typeface FontPool::match(cString& familyName, const FontStyle& style, bool useDefault) {
		auto skStyle = *reinterpret_cast<const SkFontStyle*>(&style);

		if (!familyName.is_empty()) {
			
			// find register font family
			auto it0 = _rtf.find(familyName);
			if (it0 != _rtf.end()) {
				auto it = it0->value.find(style);
				if (it != it0->value.end())
					return it->value;
				return it0->value.begin()->value;
			}

			// find system font family
			sk_sp<SkFontStyleSet> styleSet(SkMgr(_impl)->matchFamily(familyName.c_str()));
			if (styleSet->count()) {
				auto tf = styleSet->matchStyle(skStyle);
				if (tf)
					return Typeface(tf);
				// use default style
				styleSet->getStyle(0, &skStyle, nullptr);
				auto tf2 = styleSet->matchStyle(skStyle);
				Qk_Assert(tf2);
				return Typeface(tf2);
			}
		}
		
		if (useDefault) {
			auto Default = SkMgr(_impl)->matchFamilyStyle(nullptr, skStyle);
			return Typeface(Default);
		}
		return Typeface();
	}

	void FontPool::register_from_data(cBuffer& buff) {
		auto data = SkData::MakeWithoutCopy(*buff, buff.length());
		for (int i = 0; ;i++) {
			auto tf = SkMgr(_impl)->makeFromData(data, i);
			if (!tf) break;
			Typeface tf2(tf.get());
			tf->ref();
			String familyName = tf2.getFamilyName();
			auto& family = _rtf[familyName];
			if (!family.has(tf2.fontStyle())) {
				family.set(tf2.fontStyle(), tf2);
			}
		}
	}

	void FontPool::register_from_file(cString& path) {
		register_from_data(fs_read_file_sync(path));
	}

	const Array<Typeface>& FontPool::second() const {
		return _second;
	}

	const Typeface& FontPool::last() const {
		return _last;
	}

	void FontPool::initialize() {
		SkFontStyle skStyle; // default style
		
		// Find english character set
		auto tf = SkMgr(_impl)->matchFamilyStyle(nullptr, skStyle);
		_second.push(Typeface(tf));
		Qk_DEBUG(_second[0].getFamilyName());

		// Find chinese character set, 楚(26970)
		auto tf1 = SkMgr(_impl)->matchFamilyStyleCharacter(nullptr, skStyle, nullptr, 0, 26970);
		if (tf1) {
			_second.push(Typeface(tf1));
			Qk_DEBUG(_second[1].getFamilyName());
		}

		// find last �(65533)
		auto tf2 = SkMgr(_impl)->matchFamilyStyleCharacter(nullptr, skStyle, nullptr, 0, 65533);
		if (tf2) {
			_last = Typeface(tf2);
			_last_65533 = _last.unicharToGlyph(65533);
			Qk_DEBUG(_last.getFamilyName());
		}
	}

}
