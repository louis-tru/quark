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
#include "../../types.h"

namespace qk {

	class Qk_EXPORT FontStyle {
	public:
		/*constexpr */FontStyle(TextWeight weight, TextWidth width, TextSlant slant) : _value(
			(Int32::clamp(int(weight), int(TextWeight::kInherit), int(TextWeight::kExtraBlack))) +
			(Int32::clamp(int(width), int(TextWidth::kUltraCondensed), int(TextWidth::kUltraExpanded)) << 16) +
			(Int32::clamp(int(slant) - 1, 0, 2) << 24)
		) {}

    FontStyle(): FontStyle{TextWeight::kDefault, TextWidth::kDefault, TextSlant::kNormal} {}

		bool operator==(const FontStyle& rhs) const {
			return _value == rhs._value;
		}

		TextWeight weight() const { return TextWeight(_value & 0xFFFF); }
		TextWidth width() const { return TextWidth((_value >> 16) & 0xFF); }
		TextSlant slant() const { return TextSlant(((_value >> 24) & 0xFF) + 1); }
		inline int32_t value() const { return _value; }

		static FontStyle Normal() {
			return FontStyle(TextWeight::kDefault, TextWidth::kDefault, TextSlant::kDefault);
		}
		static FontStyle Bold() {
			return FontStyle(TextWeight::kBold, TextWidth::kDefault, TextSlant::kDefault);
		}
		static FontStyle Italic() {
			return FontStyle(TextWeight::kDefault, TextWidth::kDefault, TextSlant::kItalic );
		}
		static FontStyle BoldItalic() {
			return FontStyle(TextWeight::kBold, TextWidth::kDefault, TextSlant::kItalic );
		}

	private:
		int32_t _value;
	};

	template<> Qk_EXPORT uint64_t Compare<FontStyle>::hashCode(const FontStyle& key);

}
#endif
