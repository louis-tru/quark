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

//@private head

#ifndef __quark__font__android__android_font__
#define __quark__font__android__android_font__

#include "../ft/ft_typeface.h"

#include <limits>

using namespace qk;

struct QkAndroid_CustomFonts {
	/** When specifying custom fonts, indicates how to use system fonts. */
	enum SystemFontUse {
		kOnlyCustom, /** Use only custom fonts. NDK compliant. */
		kPreferCustom, /** Use custom fonts before system fonts. */
		kPreferSystem /** Use system fonts before custom fonts. */
	};
	/** Whether or not to use system fonts. */
	SystemFontUse fSystemFontUse;

	/** Base path to resolve relative font file names. If a directory, should end with '/'. */
	cChar* fBasePath;

	/** Optional custom configuration file to use. */
	cChar* fFontsXml;

	/** Optional custom configuration file for fonts which provide fallback.
	 *  In the new style (version > 21) fontsXml format is used, this should be NULL.
	 */
	cChar* fFallbackFontsXml;

	/** Optional custom flag. If set to true the QkFontMgr will acquire all requisite
	 *  system IO resources on initialization.
	 */
	bool fIsolated;
};

/** \class QkLanguage

	The QkLanguage class represents a human written language, and is used by
	text draw operations to determine which glyph to draw when drawing
	characters with variants (ie Han-derived characters).
*/
class QkLanguage {
public:
	QkLanguage() { }
	QkLanguage(cString& tag) : fTag(tag) { }
	QkLanguage(cChar* tag) : fTag(tag) { }
	QkLanguage(cChar* tag, size_t len) : fTag(tag, len) { }
	QkLanguage(const QkLanguage& b) : fTag(b.fTag) { }

	/** Gets a BCP 47 language identifier for this QkLanguage.
		@return a BCP 47 language identifier representing this language
	*/
	cString& getTag() const { return fTag; }

	/** Performs BCP 47 fallback to return an QkLanguage one step more general.
		@return an QkLanguage one step more general
	*/
	QkLanguage getParent() const;

	bool operator==(const QkLanguage& b) const {
		return fTag == b.fTag;
	}
	bool operator!=(const QkLanguage& b) const {
		return fTag != b.fTag;
	}
	QkLanguage& operator=(const QkLanguage& b) {
		fTag = b.fTag;
		return *this;
	}

private:
	//! BCP 47 language identifier
	String fTag;
};

enum FontVariants {
	kDefault_FontVariant = 0x01,
	kCompact_FontVariant = 0x02,
	kElegant_FontVariant = 0x04,
	kLast_FontVariant = kElegant_FontVariant,
};
typedef uint32_t FontVariant;

// Must remain trivially movable (can be memmoved).
struct FontFileInfo {
	FontFileInfo() : fIndex(0), fWeight(0), fStyle(Style::kAuto) { }
	String fFileName;
	int fIndex;
	int fWeight;
	enum class Style { kAuto, kNormal, kItalic } fStyle;
	Array<FontArguments::VariationPosition::Coordinate> fVariationDesignPosition;
};

/**
 * A font families provides one or more names for a collection of fonts, each of
 * which has a different style (normal, italic) or weight (thin, light, bold,
 * etc).
 * Some fonts may occur in compact variants for use in the user interface.
 * Android distinguishes "fallback" fonts to support non-ASCII character sets.
 */
struct FontFamily {
	FontFamily(cString& basePath, bool isFallbackFont)
		: fVariant(kDefault_FontVariant)
		, fOrder(-1)
		, fIsFallbackFont(isFallbackFont)
		, fBasePath(basePath)
	{}
	Array<String> fNames;
	Array<FontFileInfo> fFonts;
	Array<QkLanguage> fLanguages;
	Dict<String, Sp<FontFamily>> fallbackFamilies;
	FontVariant fVariant;
	int fOrder; // internal to the parser, not useful to users.
	bool fIsFallbackFont;
	String fFallbackFor;
	cString fBasePath;
};

/** Parses system font configuration files and appends result to fontFamilies. */
void GetSystemFontFamilies(Array<FontFamily*>& fontFamilies);

/** Parses font configuration files and appends result to fontFamilies. */
void GetCustomFontFamilies(Array<FontFamily*>& fontFamilies,
													cString& basePath,
													cChar* fontsXml,
													cChar* fallbackFontsXml,
													cChar* langFallbackFontsDir = nullptr);

/** Parses a null terminated string into an integer type, checking for overflow.
 *  http://www.w3.org/TR/html-markup/datatypes.html#common.data.integer.non-negative-def
 *
 *  If the string cannot be parsed into 'value', returns false and does not change 'value'.
 */
template <typename T> static bool parse_non_negative_integer(cChar* s, T* value) {
	static_assert(std::numeric_limits<T>::is_integer, "T_must_be_integer");

	if (*s == '\0') {
		return false;
	}

	const T nMax = std::numeric_limits<T>::max() / 10;
	const T dMax = std::numeric_limits<T>::max() - (nMax * 10);
	T n = 0;
	for (; *s; ++s) {
		// Check if digit
		if (*s < '0' || '9' < *s) {
			return false;
		}
		T d = *s - '0';
		// Check for overflow
		if (n > nMax || (n == nMax && d > dMax)) {
			return false;
		}
		n = (n * 10) + d;
	}
	*value = n;
	return true;
}

/** Parses a null terminated string into a signed fixed point value with bias N.
 *
 *  Like http://www.w3.org/TR/html-markup/datatypes.html#common.data.float-def ,
 *  but may start with '.' and does not support 'e'. '-?((:digit:+(.:digit:+)?)|(.:digit:+))'
 *
 *  Checks for overflow.
 *  Low bit rounding is not defined (is currently truncate).
 *  Bias (N) required to allow for the sign bit and 4 bits of integer.
 *
 *  If the string cannot be parsed into 'value', returns false and does not change 'value'.
 */
template <int N, typename T> static bool parse_fixed(cChar* s, T* value) {
	static_assert(std::numeric_limits<T>::is_integer, "T_must_be_integer");
	static_assert(std::numeric_limits<T>::is_signed, "T_must_be_signed");
	static_assert(sizeof(T) * CHAR_BIT - N >= 5, "N_must_leave_four_bits_plus_sign");

	bool negate = false;
	if (*s == '-') {
		++s;
		negate = true;
	}
	if (*s == '\0') {
		return false;
	}

	const T nMax = (std::numeric_limits<T>::max() >> N) / 10;
	const T dMax = (std::numeric_limits<T>::max() >> N) - (nMax * 10);
	T n = 0;
	T frac = 0;
	for (; *s; ++s) {
		// Check if digit
		if (*s < '0' || '9' < *s) {
			// If it wasn't a digit, check if it is a '.' followed by something.
			if (*s != '.' || s[1] == '\0') {
				return false;
			}
			// Find the end, verify digits.
			for (++s; *s; ++s) {
				if (*s < '0' || '9' < *s) {
					return false;
				}
			}
			// Read back toward the '.'.
			for (--s; *s != '.'; --s) {
				T d = *s - '0';
				frac = (frac + (d << N)) / 10; // This requires four bits overhead.
			}
			break;
		}
		T d = *s - '0';
		// Check for overflow
		if (n > nMax || (n == nMax && d > dMax)) {
			return false;
		}
		n = (n * 10) + d;
	}
	if (negate) {
		n = -n;
		frac = -frac;
	}
	*value = QkLeftShift(n, N) + frac;
	return true;
}

#endif /* __quark__font__android__android_font__ */
