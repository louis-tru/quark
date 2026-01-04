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

#include "../../util/fs.h"
#include "../../util/hash.h"
#include "./pool.h"
#include "./priv/mutex.h"
#include "../../../out/native-font.h"

namespace qk {

	// ---------------------- F o n t . P o o l --------------------------

	static std::mutex _sf_mutex;
	static FontPool* _shared_fontPool = nullptr;

	FontPool* FontPool::shared() {
		if (!_shared_fontPool) {
			ScopeLock scope(_sf_mutex);
			if (!_shared_fontPool) {
				_shared_fontPool = FontPool::Make();
			}
		}
		return _shared_fontPool;
	}

	FontPool::FontPool(): _tf65533GlyphID(0), _Mutex(new SharedMutex) {}
	
	FontPool::~FontPool() {
		Releasep(_Mutex);
	}

	void FontPool::initFontPool() {
		FontStyle style; // default style

		// Find english character set
		auto tf = match(String(), style);
		_defaultFamilyNames.push(tf->getFamilyName());

		Qk_DLog("FontPool::init _defaultFamilyNames, %s", *_defaultFamilyNames[0]);

		// Find chinese character set, 楚(26970)
		auto tf1 = matchCharacter(String(), style, 26970);
		if (tf1) {
			_defaultFamilyNames.push(tf1->getFamilyName());
			tf1->getMetrics(&_UnitMetrics64, 64);
			Qk_DLog("FontPool::init _defaultFamilyNames, %s", *_defaultFamilyNames[1]);
		} else {
			tf->getMetrics(&_UnitMetrics64, 64);
		}
		// find �(65533) character tf
		_tf65533 = matchCharacter(String(), style, 65533);
		//Qk_ASSERT(_tf65533);
		//Qk_DLog("FontPool::init _tf65533, %s", *_tf65533->getFamilyName());

		if (_tf65533)
			_tf65533GlyphID = _tf65533->unicharToGlyph(65533);
		_defaultFontFamilies = getFontFamilies(Array<String>());

		WeakBuffer buff((cChar*)native_fonts_[0].data, native_fonts_[0].count);
		addFontFamily(buff.buffer());
	}

	FFID FontPool::getFontFamilies(cString& families) {
		return families.is_empty() ? _defaultFontFamilies: getFontFamilies(families.split(","));
	}

	FFID FontPool::getFontFamilies(cArray<String>& families) {
		AutoSharedMutexShared ama(*_Mutex);
		Hash5381 hash;
		for (auto& i: families) {
			hash.updatestr(i.trim());
		}
		auto it = _fontFamilies.find(hash.hashCode());
		if (it != _fontFamilies.end()) {
			return *it->second;
		}
		return *_fontFamilies.set(hash.hashCode(), new FontFamilies(this, families));
	}

	String FontPool::addFontFamily(cBuffer& buff, cString& alias) {
		String familyName;
		AutoSharedMutexExclusive asme(*_Mutex);
		for (int i = 0; ;i++) {
			auto tf = onAddFontFamily(buff, i);
			if (!tf)
				break;
			familyName = tf->getFamilyName();
			_ext.get(familyName).set(tf->fontStyle(), tf);
			if (!alias.is_empty()) {
				_ext.get(alias).set(tf->fontStyle(), tf);
			}
		}
		return familyName;
	}

	cArray<String>& FontPool::defaultFamilyNames() const {
		return _defaultFamilyNames;
	}

	uint32_t FontPool::countFamilies() const {
		return onCountFamilies();
	}

	String FontPool::getFamilyName(int index) const {
		return onGetFamilyName(index);
	}

	Sp<Typeface> FontPool::match(cString& familyName, FontStyle style) const {
		if (familyName.is_empty()) {
			return onMatchFamilyStyle(nullptr, style);
		}
		// find extend font families
		if (_ext.length()) {
			AutoSharedMutexShared ama(*_Mutex);
			auto it0 = _ext.find(familyName);
			if (it0 != _ext.end()) {
				auto it = it0->second.find(style);
				if (it != it0->second.end())
					return const_cast<Typeface*>(it->second.get());
				return const_cast<Typeface*>(it0->second.begin()->second.get());
			}
		}
		return onMatchFamilyStyle(familyName.c_str(), style);
	}

	Sp<Typeface> FontPool::matchCharacter(cString& familyName, FontStyle style,
																		 Unichar character) const {
		cChar* c_familyName = familyName.is_empty() ? nullptr: familyName.c_str();
		return onMatchFamilyStyleCharacter(c_familyName, style, nullptr, 0, character);
	}

}
