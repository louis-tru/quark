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

#ifndef __noug__font__style__
#define __noug__font__style__

#include "../../util/numbers.h"
#include "../../util/dict.h"
#include "../../value.h"

namespace noug {

	class N_EXPORT FontStyle {
	public:

		constexpr FontStyle(TextWeight weight, TextWidth width, TextSlant slant) : _value(
			(Int32::limit(int(weight), int(TextWeight::INHERIT), int(TextWeight::ExtraBlack))) +
			(Int32::limit(int(width), int(TextWidth::UltraCondensed), int(TextWidth::UltraExpanded)) << 16) +
			(Int32::limit(int(slant) - 1, 0, 2) << 24)
		) {}

		constexpr FontStyle(): FontStyle{TextWeight::DEFAULT, TextWidth::DEFAULT, TextSlant::NORMAL} {}

		bool operator==(const FontStyle& rhs) const {
			return _value == rhs._value;
		}

		int weight() const { return _value & 0xFFFF; }
		int width() const { return (_value >> 16) & 0xFF; }
		TextSlant slant() const { return TextSlant(((_value >> 24) & 0xFF) + 1); }
		inline int32_t value() const { return _value; }

		static constexpr FontStyle Normal() {
			return FontStyle(TextWeight::DEFAULT, TextWidth::DEFAULT, TextSlant::DEFAULT);
		}
		static constexpr FontStyle Bold() {
			return FontStyle(TextWeight::BOLD, TextWidth::DEFAULT, TextSlant::DEFAULT);
		}
		static constexpr FontStyle Italic() {
			return FontStyle(TextWeight::DEFAULT, TextWidth::DEFAULT, TextSlant::ITALIC );
		}
		static constexpr FontStyle BoldItalic() {
			return FontStyle(TextWeight::BOLD, TextWidth::DEFAULT, TextSlant::ITALIC );
		}

	private:
		int32_t _value;
	};

	template<> N_EXPORT uint64_t Compare<FontStyle>::hash_code(const FontStyle& key);

}
#endif
