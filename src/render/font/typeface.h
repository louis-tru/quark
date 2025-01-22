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

#ifndef __quark__font__typeface__
#define __quark__font__typeface__

#include "../../util/string.h"
#include "../../math.h"
#include "../path.h"
#include "./style.h"
#include "./metrics.h"
#include "../source.h"

namespace qk {
	class RenderBackend;
	class SharedMutex;

	/**
	 * @class Typeface
	 * @safe Rt
	*/
	class Qk_Export Typeface: public Reference {
		Qk_HIDDEN_ALL_COPY(Typeface);
	public:
		struct ImageOut {
			Sp<ImageSource> image; // The shared pointer of image 
			float top; // Distance of baseline origin to the top side of image, positive number
			float right; // Distance of baseline origin to the right side of image, positive number
			float fontSize; // Font size of image
			float needToScale; // This image needs to be scaled when used
		};
		Qk_DEFINE_P_GET(FontStyle, fontStyle, ProtectedConst);
		bool isBold() const { return _fontStyle.weight() >= TextWeight::Semibold; }
		bool isItalic() const { return _fontStyle.slant() >= TextSlant::Italic; }
		int countGlyphs() const;
		int countTables() const;
		int getTableTags(FontTableTag tags[]) const;
		Buffer getTableData(FontTableTag tag) const;
		size_t getTableSize(FontTableTag) const;
		int getUnitsPerEm() const;
		String getFamilyName() const;
		bool getPostScriptName(String *name) const;
		void unicharsToGlyphs(const Unichar unichar[], uint32_t count, GlyphID glyphs[]) const;
		Array<GlyphID> unicharsToGlyphs(cArray<Unichar>& unichar) const;
		GlyphID unicharToGlyph(Unichar unichar) const;
		// non const methods
		const Path& getPath(GlyphID glyph); // returns the path of glyph in 64 px
		const FontGlyphMetrics& getGlyphMetrics(GlyphID glyph); // returns the font glyph metrics in 64 px
		Array<FontGlyphMetrics> getGlyphsMetrics(cArray<GlyphID>& glyphs); // returns the glyphs metrics
		float getMetrics(FontMetrics *metrics, float fontSize);
		float getMetrics(FontMetricsBase *metrics, float fontSize);
		/**
		* get image source object from out param and return top to baseline value for image text
		* @method getImage
		* @param offset {cArray<Vec2>*} offset.length = glyphs.length + 1
		* @param offsetScale {float} offset scale
		*/
		ImageOut getImage(cArray<GlyphID> &glyphs,
			float fontSize, cArray<Vec2> *offset, float offsetScale, RenderBackend *render = nullptr);
	protected:
		Typeface(FontStyle style);
		virtual int onCountGlyphs() const = 0;
		virtual int onGetUPEM() const = 0;
		virtual String onGetFamilyName() const = 0;
		virtual bool onGetPostScriptName(String*) const = 0;
		virtual int onGetTableTags(FontTableTag tags[]) const = 0;
		virtual size_t onGetTableData(FontTableTag, size_t offset, size_t length, void* data) const = 0;
		virtual void onCharsToGlyphs(const Unichar* chars, int count, GlyphID glyphs[]) const = 0;
		virtual void onGetMetrics(FontMetrics* metrics) = 0;
		virtual void onGetGlyphMetrics(GlyphID glyph, FontGlyphMetrics* metrics) = 0;
		virtual bool onGetPath(GlyphID glyph, Path *path) = 0;
		virtual ImageOut onGetImage(cArray<GlyphID> &glyphs,
			float fontSize, cArray<Vec2> *offset, float offsetScale, RenderBackend *render) = 0;
		SharedMutex& mutex() const { return **_Mutex; }
	private:
		// props field:
		FontMetrics  _metrics;
		Dict<GlyphID, FontGlyphMetrics> _glyphsCache;
		Dict<GlyphID, Path> _pathsCache;
		mutable int _unitsPerEm;
		mutable Sp<SharedMutex> _Mutex;
	};

}
#endif
