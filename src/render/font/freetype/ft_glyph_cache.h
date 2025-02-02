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

#ifndef __quark__font__freetype__ft_glyph_cache__
#define __quark__font__freetype__ft_glyph_cache__

#include "../priv/util.h"
#include "../priv/to.h"
#include "../metrics.h"

using namespace qk;

class QkCharToGlyphCache {
public:
	QkCharToGlyphCache();
	~QkCharToGlyphCache();

	// return number of unichars cached
	int count() const {
		return fK32.size();
	}

	void reset();       // forget all cache entries (to save memory)

	/**
	 *  Given a unichar, return its glyphID (if the return value is positive), else return
	 *  ~index of where to insert the computed glyphID.
	 *
	 *  int result = cache.charToGlyph(unichar);
	 *  if (result >= 0) {
	 *      glyphID = result;
	 *  } else {
	 *      glyphID = compute_glyph_using_typeface(unichar);
	 *      cache.insertCharAndGlyph(~result, unichar, glyphID);
	 *  }
	 */
	int findGlyphIndex(Unichar c) const;

	/**
	 *  Insert a new char/glyph pair into the cache at the specified index.
	 *  See charToGlyph() for how to compute the bit-not of the index.
	 */
	void insertCharAndGlyph(int index, Unichar, GlyphID);

	// helper to pre-seed an entry in the cache
	void addCharAndGlyph(Unichar unichar, GlyphID glyph) {
		int index = this->findGlyphIndex(unichar);
		if (index >= 0) {
			Qk_ASSERT(QkToU16(index) == glyph);
		} else {
			this->insertCharAndGlyph(~index, unichar, glyph);
		}
	}

private:
	std::vector<uint32_t>  fK32;
	std::vector<uint16_t>  fV16;
	double                 fDenom;
};

#endif // __quark__font__ft__ft_glyph_cache__
