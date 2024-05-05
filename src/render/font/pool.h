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

	class Qk_EXPORT FontPool: public Object {
		Qk_HIDDEN_ALL_COPY(FontPool);
	public:
		static FontPool* Make();
		// define ptops
		Qk_DEFINE_PROP_ACC_GET(int32_t, countFamilies, Const);
		Qk_DEFINE_PROP_ACC_GET(const Array<String>&, second, Const);
		Qk_DEFINE_PROP_GET(Sp<Typeface>, last);
		Qk_DEFINE_PROP_GET(GlyphID, last_65533, Const);
		// define methods
		FFID defaultFFID();
		FFID getFFID(cString& familys = String());
		FFID getFFID(const Array<String>& familys);
		void addFromData(cBuffer& buff);
		String getFamilyName(int index) const;
		Sp<Typeface> match(cString& familyName, FontStyle style) const;
		Sp<Typeface> matchCharacter(cString& familyName, FontStyle, Unichar character) const;
		float getMaxMetrics(FontMetricsBase* metrics, float fontSize);
	protected:
		FontPool();
		void initialize();
		virtual int onCountFamilies() const = 0;
		virtual String onGetFamilyName(int index) const = 0;

		virtual Typeface* onMatchFamilyStyle(const char familyName[], FontStyle) const = 0;
		virtual Typeface* onMatchFamilyStyleCharacter(const char familyName[],
																									FontStyle, Unichar character) const = 0;
		virtual Typeface* onMakeFromData(cBuffer& data, int ttcIndex) const = 0;

		Array<String> _second; // default family names
		Dict<String, Dict<FontStyle, Sp<Typeface>>> _extFamilies;
		Dict<uint64_t, Sp<FontFamilys>> _FFIDs;
		FFID _Default;
		FontMetrics _MaxMetrics64;
	};

}
#endif
