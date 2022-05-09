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

namespace noug {

	#define SkMgr(impl) static_cast<SkFontMgr*>(impl)

	FontFamilysID::FontFamilysID(Array<String>& familys, uint64_t code)
		: _code(code), _familys(std::move(familys))
	{}

	FFID FontFamilysID::Make(const Array<String>& familys) {
		static Dict<uint64_t, FFID> FFIDs; // global groups

		Array<String> newFamilys;
		SimpleHash hash;

		for (auto& i: familys) {
			String s = i.trim();
			if ( !s.is_empty() ) {
				newFamilys.push(s);
				hash.update(s);
			}
		}

		auto it = FFIDs.find(hash.hash_code());
		if (it != FFIDs.end()) {
			return it->value;
		}

		FFID id = new FontFamilysID(newFamilys, hash.hash_code());
		FFIDs[ hash.hash_code() ] = id;
		return id;
	}

	FFID FontFamilysID::Make(cString familys) {
		if ( familys.is_empty() )
			return Make(Array<String>());
		else
			return Make(familys.split(','));
	}

	// ---------------------- F o n t . P o o l --------------------------

	FontPool::FontPool(Application* host)
		: _host(host)
		, _impl(SkFontMgr::RefDefault().get())
	{
		N_ASSERT(_impl);
		SkMgr(_impl)->ref();
		set_default_typeface();
	}

	FontPool::~FontPool() {
		SkMgr(_impl)->unref();
		_impl = nullptr;
	}

	Array<String> FontPool::family_names() const {
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

	Typeface FontPool::typeface(cString& familyName, const FontStyle& style) {
		SkFontStyle skStyle = *reinterpret_cast<const SkFontStyle*>(&style);
		
		if (!familyName.is_empty()) {
			auto set = SkMgr(_impl)->matchFamily(familyName.c_str());
			if (set->count()) {
				auto tf = set->matchStyle(skStyle);
				if (tf)
					return Typeface(this, tf);

				// find register font family
				auto it0 = _register_tf.find(familyName);
				if (it0 != _register_tf.end()) {
					auto it = it0->value.find(style.value());
					if (it != it0->value.end())
						return it->value;
				}
				// use default style
				set->getStyle(0, &skStyle, nullptr);
				auto tf2 = set->matchStyle(skStyle);
				N_ASSERT(tf2);
				return Typeface(this, tf2);
			}
			set->unref();
		
			// find register font family
			auto it0 = _register_tf.find(familyName);
			if (it0 != _register_tf.end()) {
				auto it = it0->value.find(style.value());
				if (it != it0->value.end())
					return it->value;
				return it0->value.begin()->value;
			}
		}

		// find default family
		auto tf = SkMgr(_impl)->matchFamilyStyle(nullptr, skStyle);
		return Typeface(this, tf);
	}

	void FontPool::register_from_data(cBuffer& buff) {
		auto data = SkData::MakeWithoutCopy(*buff, buff.length());
		for (int i = 0; ;i++) {
			auto tf = SkMgr(_impl)->makeFromData(data, i);
			if (!tf) break;
			Typeface tf2(this, tf.get());
			tf->ref();
			String familyName = tf2.getFamilyName();
			int32_t style = tf2.fontStyle().value();
			auto& family = _register_tf[familyName];
			if (!family.has(style)) {
				family.set(style, tf2);
			}
		}
	}

	void FontPool::register_from_file(cString& path) {
		register_from_data(fs_read_file_sync(path));
	}

	const Array<Typeface>& FontPool::default_typeface() {
		return _default_tf;
	}

	void FontPool::set_default_typeface() {
		SkFontStyle skStyle;
		
		// Find english character set
		auto tf = SkMgr(_impl)->matchFamilyStyle(nullptr, skStyle);
		_default_tf.push(Typeface(this, tf));
		N_DEBUG(_default_tf[0].getFamilyName());

		// Find chinese character set, 楚(26970)
		auto tf1 = SkMgr(_impl)->matchFamilyStyleCharacter(nullptr, skStyle, nullptr, 0, 26970);
		if (tf1) {
			_default_tf.push(Typeface(this, tf1));
			N_DEBUG(_default_tf[1].getFamilyName());
		}
		
		if (!_default_tf.back().unicharToGlyph(65533)) { // find �
			auto tf = SkMgr(_impl)->matchFamilyStyleCharacter(nullptr, skStyle, nullptr, 0, 65533);
			if (tf) {
				_default_tf.push(Typeface(this, tf));
				N_DEBUG(_default_tf.back().getFamilyName());
			}
		}
	}

}
