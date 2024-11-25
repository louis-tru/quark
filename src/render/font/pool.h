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

#ifndef __quark__font__pool__
#define __quark__font__pool__

#include "./font.h"

namespace qk {

	class Qk_Export FontPool: public Object {
		Qk_HIDDEN_ALL_COPY(FontPool);
	public:
		static FontPool* Make();
		// define ptops
		Qk_DEFINE_A_GET(uint32_t, countFamilies, Const);
		Qk_DEFINE_A_GET(cArray<String>&, defaultFamilyNames, Const);
		Qk_DEFINE_P_GET(Sp<Typeface>, tf65533);
		Qk_DEFINE_P_GET(GlyphID, tf65533GlyphID, Const);
		Qk_DEFINE_P_GET(FFID, defaultFontFamilys);
		// define methods
		FFID getFontFamilys(cString& familys = String());
		FFID getFontFamilys(cArray<String>& familys);
		void addFontFamily(cBuffer& buff, cString& alias = String());
		String getFamilyName(int index) const;
		Sp<Typeface> match(cString& familyName, FontStyle style);
		Sp<Typeface> matchCharacter(cString& familyName, FontStyle, Unichar character) const;
		float getMaxMetrics(FontMetricsBase* metrics, float fontSize);
	protected:
		FontPool();
		void init();
		virtual uint32_t onCountFamilies() const = 0;
		virtual String onGetFamilyName(int index) const = 0;
		virtual Typeface* onMatch(cChar familyName[], FontStyle) const = 0;
		virtual Typeface* onMatchCharacter(cChar familyName[], FontStyle, Unichar character) const = 0;
		virtual Typeface* onAddFontFamily(cBuffer& data, int ttcIndex) const = 0;

		Array<String> _defaultFamilyNames; // default family names
		Dict<String, Dict<FontStyle, Sp<Typeface>>> _ext; // familyname => (style=>tf)
		Dict<uint64_t, Sp<FontFamilys>> _fontFamilys;
		FontMetrics _MaxMetrics64;
		Mutex _Mutex;
	};

}
#endif
