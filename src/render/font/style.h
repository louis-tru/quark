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

#ifndef __quark__font__style__
#define __quark__font__style__

#include "../../util/numbers.h"
#include "../../util/dict.h"

namespace qk {

	enum class TextWeight: uint16_t {
		Inherit      = 0,
		Thin         = 100,
		Ultralight   = 200,
		Light        = 300,
		Regular      = 400,
		Medium       = 500,
		Semibold     = 600,
		Bold         = 700,
		Heavy        = 800,
		Black        = 900,
		ExtraBlack   = 1000,
		Default      = Regular, // default
	};

	enum class TextWidth: uint8_t {
		Inherit          = 0, // inherit
		UltraCondensed   = 1,
		ExtraCondensed   = 2,
		Condensed        = 3,
		SemiCondensed    = 4,
		Normal           = 5,
		SemiExpanded     = 6,
		Expanded         = 7,
		ExtraExpanded    = 8,
		UltraExpanded    = 9,
		Default          = Normal,
	};

	enum class TextSlant: uint8_t {
		Inherit, // inherit
		Normal, // 正常
		Italic, // 斜体
		Oblique,  // 倾斜
		Default = Normal,
	};

	class FontFamilys;
	typedef FontFamilys* FFID;

	class Qk_Export FontStyle {
	public:
		Qk_DEFINE_PGET(uint32_t, value, Const);

		FontStyle(TextWeight weight, TextWidth width, TextSlant slant) : _value(
			(Uint32::clamp(uint32_t(weight), uint32_t(TextWeight::Inherit), uint32_t(TextWeight::ExtraBlack))) +
			(Uint32::clamp(uint32_t(width), uint32_t(TextWidth::UltraCondensed), uint32_t(TextWidth::UltraExpanded)) << 16) +
			(Uint32::clamp(uint32_t(slant) - 1u, 0u, 2u) << 24)
		) {}

		FontStyle(): FontStyle{TextWeight::Default, TextWidth::Default, TextSlant::Normal} {}

		bool operator==(const FontStyle& rhs) const {
			return _value == rhs._value;
		}

		TextWeight weight() const { return TextWeight(_value & 0xFFFF); }
		TextWidth width() const { return TextWidth((_value >> 16) & 0xFF); }
		TextSlant slant() const { return TextSlant(((_value >> 24) & 0xFF) + 1); }

		static FontStyle Normal() {
			return FontStyle(TextWeight::Default, TextWidth::Default, TextSlant::Default);
		}
		static FontStyle Bold() {
			return FontStyle(TextWeight::Bold, TextWidth::Default, TextSlant::Default);
		}
		static FontStyle Italic() {
			return FontStyle(TextWeight::Default, TextWidth::Default, TextSlant::Italic );
		}
		static FontStyle BoldItalic() {
			return FontStyle(TextWeight::Bold, TextWidth::Default, TextSlant::Italic );
		}
	};

	template<> Qk_Export uint64_t Compare<FontStyle>::hashCode(const FontStyle& key);

}
#endif
