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

#include "./ft_glyph_cache.h"

QkCharToGlyphCache::QkCharToGlyphCache() {
	this->reset();
}

QkCharToGlyphCache::~QkCharToGlyphCache() {}

void QkCharToGlyphCache::reset() {
	fK32.clear();
	fV16.clear();

	// Add sentinels so we can always rely on these to stop linear searches (in either direction)
	// Neither is a legal unichar, so we don't care what glyphID we use.
	//
	fK32.push_back(0x80000000);    fV16.push_back(0);
	fK32.push_back(0x7FFFFFFF);    fV16.push_back(0);

	fDenom = 0;
}

// Determined experimentally. For N much larger, the slope technique is faster.
// For N much smaller, a simple search is faster.
//
constexpr int kSmallCountLimit = 16;

// To use slope technique we need at least 2 real entries (+2 sentinels) hence the min of 4
//
constexpr int kMinCountForSlope = 4;

static int find_simple(const Unichar base[], int count, Unichar value) {
	int index;
	for (index = 0;; ++index) {
		if (value <= base[index]) {
			if (value < base[index]) {
				index = ~index; // not found
			}
			break;
		}
	}
	return index;
}

static int find_with_slope(const Unichar base[], int count, Unichar value, double denom) {
	Qk_ASSERT(count >= kMinCountForSlope);

	int index;
	if (value <= base[1]) {
		index = 1;
		if (value < base[index]) {
			index = ~index;
		}
	} else if (value >= base[count - 2]) {
		index = count - 2;
		if (value > base[index]) {
			index = ~(index + 1);
		}
	} else {
		// make our guess based on the "slope" of the current values
		//        index = 1 + (int64_t)(count - 2) * (value - base[1]) / (base[count - 2] - base[1]);
		index = 1 + (int)(denom * (count - 2) * (value - base[1]));
		Qk_ASSERT(index >= 1 && index <= count - 2);

		if (value >= base[index]) {
			for (;; ++index) {
				if (value <= base[index]) {
					if (value < base[index]) {
						index = ~index; // not found
					}
					break;
				}
			}
		} else {
			for (--index;; --index) {
				Qk_ASSERT(index >= 0);
				if (value >= base[index]) {
					if (value > base[index]) {
						index = ~(index + 1);
					}
					break;
				}
			}
		}
	}
	return index;
}

int QkCharToGlyphCache::findGlyphIndex(Unichar unichar) const {
	const int count = fK32.size();
	int index;
	if (count <= kSmallCountLimit) {
		index = find_simple(fK32.data(), count, unichar);
	} else {
		index = find_with_slope(fK32.data(), count, unichar, fDenom);
	}
	if (index >= 0) {
		return fV16[index];
	}
	return index;
}

void QkCharToGlyphCache::insertCharAndGlyph(int index, Unichar unichar, GlyphID glyph) {
	Qk_ASSERT(fK32.size() == fV16.size());
	Qk_ASSERT((unsigned)index < fK32.size());
	Qk_ASSERT(unichar < fK32[index]);

	fK32.insert(fK32.begin() + index, unichar);
	fV16.insert(fV16.begin() + index, glyph);

	// if we've changed the first [1] or last [count-2] entry, recompute our slope
	const int count = fK32.size();
	if (count >= kMinCountForSlope && (index == 1 || index == count - 2)) {
		Qk_ASSERT(index >= 1 && index <= count - 2);
		fDenom = 1.0 / ((double)fK32[count - 2] - fK32[1]);
	}

#ifdef SK_DEBUG
	for (int i = 1; i < fK32.size(); ++i) {
		Qk_ASSERT(fK32[i-1] < fK32[i]);
	}
#endif
}
