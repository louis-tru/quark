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

#ifndef __quark__font__freetype__ft_typeface__
#define __quark__font__freetype__ft_typeface__

#include "../priv/to.h"
#include "../priv/fontdata.h"
#include "../priv/arguments.h"
#include "./ft_glyph_cache.h"
#include "../typeface.h"

using namespace qk;

// These are forward declared to avoid pimpl but also hide the FreeType implementation.
typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;
typedef struct FT_StreamRec_* FT_Stream;
typedef struct FT_SizeRec_* FT_Size;

#ifdef DEBUG
const char* QkTraceFtrGetError(int);
#define Qk_TRACEFTR(ERR, MSG, ...) \
	Qk_DLog("%s:%d:1: error: 0x%x '%s' " MSG "\n", __FILE__, __LINE__, ERR, \
			QkTraceFtrGetError((int)(ERR)), __VA_ARGS__)
#else
#define Qk_TRACEFTR(ERR, ...) do { qk_ignore_unused_variable(ERR); } while (false)
#endif

enum Flags {
	kFrameAndFill_Flag        = 0x0001,
	kUnused                   = 0x0002,
	kEmbeddedBitmapText_Flag  = 0x0004,
	kEmbolden_Flag            = 0x0008,
	kSubpixelPositioning_Flag = 0x0010,
	kForceAutohinting_Flag    = 0x0020,  // Use auto instead of bytcode hinting if hinting.

	// together, these two flags resulting in a two bit value which matches
	// up with the Paint::Hinting enum.
	kHinting_Shift            = 7, // to shift into the other flags above
	kHintingBit1_Flag         = 0x0080,
	kHintingBit2_Flag         = 0x0100,

	kLinearMetrics_Flag       = 0x1000,
};

// computed values
enum {
	kHinting_Mask   = kHintingBit1_Flag | kHintingBit2_Flag,
};

class QkTypeface_FreeType : public Typeface {
public:
	/** For QkFontMgrs to make use of our ability to extract
	 *  name and style from a stream, using FreeType's API.
	 */
	class Scanner {
		Qk_DISABLE_COPY(Scanner);
	public:
		Scanner();
		~Scanner();
		struct AxisDefinition {
			FontByteTag fTag;
			QkFixed fMinimum;
			QkFixed fDefault;
			QkFixed fMaximum;
		};
		// using AxisDefinitions = QkSTArray<4, AxisDefinition, true>;
		using AxisDefinitions = Array<AxisDefinition>;
		bool recognizedFont(QkStream* stream, int* numFonts) const;
		bool scanFont(QkStream* stream, int ttcIndex,
									String* name, FontStyle* style, bool* isFixedPitch, AxisDefinitions* axes) const;
		static void computeAxisValues(
			AxisDefinitions axisDefinitions,
			const FontArguments::VariationPosition position,
			QkFixed* axisValues,
			cString& name,
			const FontArguments::VariationPosition::Coordinate* currentPosition = nullptr);
		static bool GetAxes(FT_Face face, AxisDefinitions* axes);

	private:
		FT_Face openFace(QkStream* stream, int ttcIndex, FT_Stream ftStream) const;
		FT_Library fLibrary;
		mutable QkMutex fLibraryMutex;
	};

	/** Fetch units/EM from "head" table if needed (ie for bitmap fonts) */
	static int GetUnitsPerEm(FT_Face face);

	/**
	 *  Return the font data, or nullptr on failure.
	 */
	Sp<QkFontData> makeFontData() const;

	class FaceRec;
	inline FaceRec* getFaceRec() const { return fFaceRec.get(); }

	inline SharedMutex& ft_mutex() const { return mutex(); }

protected:

	QkTypeface_FreeType(const FontStyle& style, uint16_t flags);
	~QkTypeface_FreeType() override;
	void initFreeType();
	bool onGetPostScriptName(String*) const override;
	int onGetUPEM() const override;
	void onCharsToGlyphs(const Unichar uni[], int count, GlyphID glyphs[]) const override;
	int onCountGlyphs() const override;
	int onGetTableTags(FontTableTag tags[]) const override;
	size_t onGetTableData(FontTableTag, size_t offset, size_t length, void* data) const override;
	void onGetGlyphMetrics(GlyphID glyph, FontGlyphMetrics* metrics) override;
	void onGetMetrics(FontMetrics* metrics) override;
	bool onGetPath(GlyphID glyph, Path *path) override;
	TextImage onGetImage(cArray<GlyphID>& glyphs, float fontSize,
		cArray<Vec2> *offset, float padding, bool antiAlias, RenderBackend *render) override;

	virtual Sp<QkFontData> onMakeFontData() const = 0;

private:
	void generateGlyphImage(cFontGlyphMetrics &glyph, Pixel &pixel, FT_Pixel_Mode mode, Vec2 imgBaseline);
	bool generateFacePath(Path* path);

	uint16_t          fFlags;
	FT_Size           fFTSize;  // The size to apply to the fFace.
	mutable FT_Face   fFace;  // Borrowed face from fFaceRec.
	int               fStrikeIndex; // The bitmap strike for the fFace (or -1 if none).
	uint32_t          fLoadGlyphFlags;
	bool              fDoLinearMetrics;

	mutable Sp<FaceRec> fFaceRec;
	mutable QkCharToGlyphCache fC2GCache;

	Qk_DEFINE_INLINE_CLASS(Inl);
};

#endif // __quark__font__ft__ft_typeface__
