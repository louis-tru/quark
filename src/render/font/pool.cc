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

#include "../../util/fs.h"
#include "../../util/hash.h"
#include "./pool.h"

namespace qk {

	// ---------------------- F o n t . P o o l --------------------------

	FontPool::FontPool(): _last_65533(0) {}

	void FontPool::initialize() {
		FontStyle style; // default style

		// Find english character set
		auto tf = match(String(), style);
		_second.push(tf->getFamilyName());
		Qk_DEBUG(_second[0]);

		// Find chinese character set, 楚(26970)
		auto tf1 = matchCharacter(String(), style, 26970);
		if (tf1) {
			_second.push(tf1->getFamilyName());
			Qk_DEBUG(_second[1]);
		}

		// find last �(65533)
		_last = matchCharacter(String(), style, 65533);
		Qk_ASSERT(_last);
		Qk_DEBUG(_last->getFamilyName());
		_last_65533 = _last->unicharToGlyph(65533);
	}

	FFID FontPool::getFFID(const Array<String>& familys) {
		Array<String> newFamilys;
		Hash5381 hash;

		for (auto& i: familys) {
			String s = i.trim();
			if ( !s.is_empty() ) {
				newFamilys.push(s);
				hash.update(s);
			}
		}

		auto it = _FFIDs.find(hash.hash_code());
		if (it != _FFIDs.end()) {
			return it->value.value();
		}

		FFID id = new FontFamilys(this, newFamilys);
		_FFIDs[ hash.hash_code() ] = id;
		return id;
	}

	FFID FontPool::getFFID(cString& familys) {
		if ( familys.is_empty() )
			return getFFID(Array<String>());
		else
			return getFFID(familys.split(","));
	}

	void FontPool::addFromData(cBuffer& buff) {
		for (int i = 0; ;i++) {
			auto tf = onMakeFromData(buff, i);
			if (!tf)
				break;
			String familyName = tf->getFamilyName();
			auto& family = _extFamilies[familyName];
			if ( !family.has(tf->fontStyle()) ) {
				family.set(tf->fontStyle(), tf);
			}
		}
	}

	const Array<String>& FontPool::second() const {
		return _second;
	}

	int FontPool::countFamilies() const {
		return onCountFamilies();
	}

	String FontPool::getFamilyName(int index) const {
		return onGetFamilyName(index);
	}
	
	Sp<Typeface> FontPool::match(cString& familyName, FontStyle style) const {
		if (familyName.is_empty()) {
			return onMatchFamilyStyle(nullptr, style);
		}
		// find register font family
		auto it0 = _extFamilies.find(familyName);
		if (it0 != _extFamilies.end()) {
			auto it = it0->value.find(style);
			if (it != it0->value.end())
				return it->value.value();
			return it0->value.begin()->value.value();
		}
		return onMatchFamilyStyle(familyName.c_str(), style);
	}

	Sp<Typeface> FontPool::matchCharacter(cString& familyName, FontStyle style,
																		 Unichar character) const {
		cChar* c_familyName = familyName.is_empty() ? nullptr: familyName.c_str();
		return onMatchFamilyStyleCharacter(c_familyName, style, character);
	}

}
