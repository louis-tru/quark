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

//@private head

#ifndef __quark__font__priv__arguments__
#define __quark__font__priv__arguments__

typedef uint32_t FontByteTag;

/** Represents a set of actual arguments for a font. */
struct FontArguments {
	struct VariationPosition {
		struct Coordinate {
			FontByteTag axis;
			float value;
		};
		const Coordinate* coordinates;
		int coordinateCount;
	};

	FontArguments() : fCollectionIndex(0), fVariationDesignPosition{nullptr, 0} {}

	/** Specify the index of the desired font.
	 *
	 *  Font formats like ttc, dfont, cff, cid, pfr, t42, t1, and fon may actually be indexed
	 *  collections of fonts.
	 */
	FontArguments& setCollectionIndex(int collectionIndex) {
		fCollectionIndex = collectionIndex;
		return *this;
	}

	/** Specify a position in the variation design space.
	 *
	 *  Any axis not specified will use the default value.
	 *  Any specified axis not actually present in the font will be ignored.
	 *
	 *  @param position not copied. The value must remain valid for life of FontArguments.
	 */
	FontArguments& setVariationDesignPosition(VariationPosition position) {
		fVariationDesignPosition.coordinates = position.coordinates;
		fVariationDesignPosition.coordinateCount = position.coordinateCount;
		return *this;
	}

	int getCollectionIndex() const {
		return fCollectionIndex;
	}

	VariationPosition getVariationDesignPosition() const {
		return fVariationDesignPosition;
	}
private:
	int fCollectionIndex;
	VariationPosition fVariationDesignPosition;
};

#endif
