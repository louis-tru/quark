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

namespace noug {

	class N_EXPORT FontStyle {
	public:
		enum Weight {
			kInvisible_Weight   =    0,
			kThin_Weight        =  100,
			kExtraLight_Weight  =  200,
			kLight_Weight       =  300,
			kNormal_Weight      =  400,
			kMedium_Weight      =  500,
			kSemiBold_Weight    =  600,
			kBold_Weight        =  700,
			kExtraBold_Weight   =  800,
			kBlack_Weight       =  900,
			kExtraBlack_Weight  = 1000,
		};

		enum Width {
			kUltraCondensed_Width   = 1,
			kExtraCondensed_Width   = 2,
			kCondensed_Width        = 3,
			kSemiCondensed_Width    = 4,
			kNormal_Width           = 5,
			kSemiExpanded_Width     = 6,
			kExpanded_Width         = 7,
			kExtraExpanded_Width    = 8,
			kUltraExpanded_Width    = 9,
		};

		enum Slant {
			kUpright_Slant,
			kItalic_Slant,
			kOblique_Slant,
		};

		constexpr FontStyle(int weight, int width, Slant slant) : _value(
			(Int32::limit(weight, kInvisible_Weight, kExtraBlack_Weight)) +
			(Int32::limit(width, kUltraCondensed_Width, kUltraExpanded_Width) << 16) +
			(Int32::limit(slant, kUpright_Slant, kOblique_Slant) << 24)
		) {}

		constexpr FontStyle() : FontStyle{kNormal_Weight, kNormal_Width, kUpright_Slant} {}

		bool operator==(const FontStyle& rhs) const {
			return _value == rhs._value;
		}

		int weight() const { return _value & 0xFFFF; }
		int width() const { return (_value >> 16) & 0xFF; }
		Slant slant() const { return (Slant)((_value >> 24) & 0xFF); }

		static constexpr FontStyle Normal() {
			return FontStyle(kNormal_Weight, kNormal_Width, kUpright_Slant);
		}
		static constexpr FontStyle Bold() {
			return FontStyle(kBold_Weight, kNormal_Width, kUpright_Slant);
		}
		static constexpr FontStyle Italic() {
			return FontStyle(kNormal_Weight, kNormal_Width, kItalic_Slant );
		}
		static constexpr FontStyle BoldItalic() {
			return FontStyle(kBold_Weight, kNormal_Width, kItalic_Slant );
		}

	private:
		int32_t _value;
	};


}
#endif