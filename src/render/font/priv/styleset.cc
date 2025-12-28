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

#include "./styleset.h"

/**
* Width has the greatest priority.
* If the value of pattern.width is 6 (normal) or less,
*    narrower width values are checked first, then wider values.
* If the value of pattern.width is greater than 6 (normal),
*    wider values are checked first, followed by narrower values.
*
* Italic/Oblique has the next highest priority.
* If italic requested and there is some italic font, use it.
* If oblique requested and there is some oblique font, use it.
* If italic requested and there is some oblique font, use it.
* If oblique requested and there is some italic font, use it.
*
* Exact match.
* If pattern.weight < 400, weights below pattern.weight are checked
*   in descending order followed by weights above pattern.weight
*   in ascending order until a match is found.
* If pattern.weight > 500, weights above pattern.weight are checked
*   in ascending order followed by weights below pattern.weight
*   in descending order until a match is found.
* If pattern.weight is 400, 500 is checked first
*   and then the rule for pattern.weight < 400 is used.
* If pattern.weight is 500, 400 is checked first
*   and then the rule for pattern.weight < 400 is used.
*/
Typeface* QkFontStyleSet::matchStyleCSS3(FontStyle pattern) {
	int count = this->count();
	if (0 == count) {
		return nullptr;
	}

	struct Score {
		int score;
		int index;
		Score& operator +=(int rhs) { this->score += rhs; return *this; }
		Score& operator <<=(int rhs) { this->score <<= rhs; return *this; }
		bool operator <(const Score& that) { return this->score < that.score; }
	};

	int pattern_slant = int(pattern.slant());
	int pattern_weight = int(pattern.weight());
	int pattern_width = int(pattern.width());

	Score maxScore = { 0, 0 };
	for (int i = 0; i < count; ++i) {
		FontStyle current;
		this->getStyle(i, &current, nullptr);
		Score currentScore = { 0, i };

		int current_slant = int(current.slant());
		int current_weight = int(current.weight());
		int current_width = int(current.width());

		// CSS stretch / FontStyle::Width
		// Takes priority over everything else.
		if (pattern_width <= int(TextWidth::Normal)) {
			if (current_width <= pattern_width) {
				currentScore += 10 - pattern_width + current_width;
			} else {
				currentScore += 10 - current_width;
			}
		} else {
			if (current_width > pattern_width) {
				currentScore += 10 + pattern_width - current_width;
			} else {
				currentScore += current_width;
			}
		}
		currentScore <<= 8;

		// CSS style (normal, italic, oblique) / FontStyle::Slant (upright, italic, oblique)
		// Takes priority over all valid weights.
		static_assert(TextSlant::Normal == TextSlant(2) &&
					TextSlant::Italic  == TextSlant(3) &&
					TextSlant::Oblique == TextSlant(4),
					"FontStyle::Slant values not as required.");

		Qk_ASSERT(2 <= pattern_slant && pattern_slant <= 4 &&
				 2 <= current_slant && current_slant <= 4);
		static const int score[5][5] = {
			{0,0,0,0,0},
			{0,0,0,0,0},
			/*                    Normal Italic Oblique  [current]*/
			/*   Normal  */ { 0,0,   3   ,  1   ,   2   },
			/*   Italic  */ { 0,0,   1   ,  3   ,   2   },
			/*   Oblique */ { 0,0,   1   ,  2   ,   3   },
			/* [pattern] */
		};
		currentScore += score[pattern_slant][current_slant];
		currentScore <<= 8;

		// Synthetics (weight, style) [no stretch synthetic?]

		// CSS weight / FontStyle::Weight
		// The 'closer' to the target weight, the higher the score.
		// 1000 is the 'heaviest' recognized weight
		if (pattern_weight == current_weight) {
			currentScore += 1000;
		// less than 400 prefer lighter weights
		} else if (pattern_weight < 400) {
			if (current_weight <= pattern_weight) {
				currentScore += 1000 - pattern_weight + current_weight;
			} else {
				currentScore += 1000 - current_weight;
			}
		// between 400 and 500 prefer heavier up to 500, then lighter weights
		} else if (pattern_weight <= 500) {
			if (current_weight >= pattern_weight && current_weight <= 500) {
				currentScore += 1000 + pattern_weight - current_weight;
			} else if (current_weight <= pattern_weight) {
				currentScore += 500 + current_weight;
			} else {
				currentScore += 1000 - current_weight;
			}
		// greater than 500 prefer heavier weights
		} else if (pattern_weight > 500) {
			if (current_weight > pattern_weight) {
				currentScore += 1000 + pattern_weight - current_weight;
			} else {
				currentScore += current_weight;
			}
		}

		if (maxScore < currentScore) {
			maxScore = currentScore;
		}
	}

	return this->createTypeface(maxScore.index);
}
