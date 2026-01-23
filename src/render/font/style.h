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

#ifndef __quark__font__style__
#define __quark__font__style__

#include "../../util/numbers.h"
#include "../../util/dict.h"

namespace qk {

	enum class FontWeight: uint16_t {
		Inherit      = 0, // inherit
		Default      = 1, // user application default value
		Thin         = 100,
		Ultralight   = 200,
		Light        = 300,
		Regular      = 400,
		Normal       = 400, // alias of Regular
		Medium       = 500,
		Semibold     = 600,
		Bold         = 700,
		Heavy        = 800,
		Black        = 900,
		ExtraBlack   = 1000,
	};

	enum class FontWidth: uint8_t {
		Inherit = 0, // inherit
		Default, // user application default value
		UltraCondensed,
		ExtraCondensed,
		Condensed,
		SemiCondensed,
		Normal,
		SemiExpanded,
		Expanded,
		ExtraExpanded,
		UltraExpanded,
	};

	enum class FontSlant: uint8_t {
		Inherit = 0, // inherit
		Default, // use application default value
		Normal, // 正常
		Italic, // 斜体
		Oblique,  // 倾斜
	};

	class FontFamilies;
	typedef FontFamilies* FFID;

	class Qk_EXPORT FontStyle {
	public:
		Qk_DEFINE_PROP_GET(uint32_t, value, Const);

		FontStyle(FontWeight weight, FontWidth width, FontSlant slant) : _value(
			(Uint32::clamp(uint32_t(weight), uint32_t(FontWeight::Inherit), uint32_t(FontWeight::ExtraBlack))) +
			(Uint32::clamp(uint32_t(width), uint32_t(FontWidth::UltraCondensed), uint32_t(FontWidth::UltraExpanded)) << 16) +
			(Uint32::clamp(uint32_t(slant) - 1u, 0u, 2u) << 24)
		) {}

		FontStyle(): FontStyle{FontWeight::Regular, FontWidth::Normal, FontSlant::Normal} {}

		bool operator==(const FontStyle& rhs) const {
			return _value == rhs._value;
		}

		FontWeight weight() const { return FontWeight(_value & 0xFFFF); }
		FontWidth width() const { return FontWidth((_value >> 16) & 0xFF); }
		FontSlant slant() const { return FontSlant(((_value >> 24) & 0xFF) + 1); }

		static FontStyle Normal() {
			return FontStyle(FontWeight::Regular, FontWidth::Normal, FontSlant::Normal);
		}
		static FontStyle Bold() {
			return FontStyle(FontWeight::Bold, FontWidth::Normal, FontSlant::Normal);
		}
		static FontStyle Italic() {
			return FontStyle(FontWeight::Regular, FontWidth::Normal, FontSlant::Italic );
		}
		static FontStyle BoldItalic() {
			return FontStyle(FontWeight::Bold, FontWidth::Normal, FontSlant::Italic );
		}
	};

	template<> Qk_EXPORT uint64_t Compare<FontStyle>::hashCode(const FontStyle& key);

}
#endif
