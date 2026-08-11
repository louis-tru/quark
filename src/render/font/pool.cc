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

#include "../../util/hash.h"
#include "../../../out/native-font.h"
#include "./pool.h"

namespace qk {

	// ---------------------- F o n t . P o o l --------------------------

	FontPool* FontPool::shared() {
		static FontPool* _shared_fontPool = FontPool::Make();
		return _shared_fontPool;
	}

	FontPool::FontPool(): _tf65533GlyphID(0), _Mutex(new SharedMutex) {}
	
	FontPool::~FontPool() {
		Releasep(_Mutex);
	}

	void FontPool::initFontPool() {
		FontStyle style; // default style

		List<String> familyNames;
		// Start with the platform's default font family.
		auto tf0 = match(String(), style);
		Qk_CHECK(tf0, "Cannot find default system font");
		familyNames.pushBack(tf0->getFamilyName());
		Qk_DLog("Init default font family, %s", *tf0->getFamilyName());

		// Add a CJK-capable fallback, using 楚 (U+695A) as the probe character.
		auto tf1 = matchCharacter(String(), style, 26970);
		if (tf1) {
			familyNames.pushBack(tf1->getFamilyName());
			tf1->getMetrics(&_strutMetrics64, 64);
			Qk_DLog("Init chinese font family, %s", *tf1->getFamilyName());
		} else {
			tf0->getMetrics(&_strutMetrics64, 64);
		}

#if Qk_LINUX
		// Fontconfig may not include the color emoji face in the default family
		// fallback, so request Fontconfig's generic emoji family explicitly.
		auto tf2 = matchCharacter("emoji", style, 0x1F600); // 😀😂😊🚀
#else
		// Native font matching on Apple and Android resolves emoji from an empty
		// family name.
		auto tf2 = matchCharacter(String(), style, 0x1F600);
	#endif
		if (tf2) {
			// Do not globally prioritize an Emoji family that also contains ordinary
			// printable ASCII, since it could replace digits and punctuation.
			bool normalFontHasEmoji = tf0->unicharToGlyph(0x1F600) ||
				(tf1 && tf1->unicharToGlyph(0x1F600));
			bool hasOrdinaryChars = false;
			if (normalFontHasEmoji) {
				hasOrdinaryChars =
					tf2->unicharToGlyph('A') ||
					tf2->unicharToGlyph('a') ||
					tf2->unicharToGlyph('0') ||
					tf2->unicharToGlyph('#') ||
					tf2->unicharToGlyph('*');
			}
			if (normalFontHasEmoji && !hasOrdinaryChars) {
				familyNames.pushFront(tf2->getFamilyName());
			} else {
				familyNames.pushBack(tf2->getFamilyName());
			}
			Qk_DLog("Init emoji font family, %s", *tf2->getFamilyName());
		}

		// Remove duplicate families while preserving the priority established above.
		Set<String> familyNamesSet;
		for (auto& i: familyNames)
			familyNamesSet.add(i);
		_defaultFamilyNames = familyNamesSet.keys();

		// Find the guaranteed fallback face and glyph for � (U+FFFD).
		_tf65533 = matchCharacter(String(), style, 65533);
		Qk_CHECK(_tf65533, "Cannot find a font containing U+FFFD");

		_tf65533GlyphID = _tf65533->unicharToGlyph(65533);
		Qk_CHECK(_tf65533GlyphID, "Cannot find glyph for U+FFFD");

		_defaultFontFamilies = getFontFamilies(Array<String>());

#if 0
		// Disabled: register the embedded native font as an additional family.
		WeakBuffer buff((cChar*)native_fonts_[0].data, native_fonts_[0].count);
		addFontFamily(buff.buffer());
#endif
	}

	FFID FontPool::getFontFamilies(cString& familieNames) {
		return familieNames.isEmpty() ? _defaultFontFamilies:
			getFontFamilies(familieNames.split(","));
	}

	FFID FontPool::getFontFamilies(cArray<String>& familieNames) {
		AutoSharedMutexShared ama(*_Mutex);
		Hash hash;
		for (auto& i: familieNames) {
			hash.updatestr(i.trim());
		}
		auto it = _fontFamilies.find(hash.hashCode());
		if (it != _fontFamilies.end()) {
			return *it->second;
		}
		return *_fontFamilies.set(hash.hashCode(), new FontFamilies(this, familieNames));
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
			if (!alias.isEmpty()) {
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
		if (familyName.isEmpty()) {
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
		cChar* c_familyName = familyName.isEmpty() ? nullptr: familyName.c_str();
		return onMatchFamilyStyleCharacter(c_familyName, style, nullptr, 0, character);
	}

}
