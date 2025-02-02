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

#include "./ft_typeface.h"

#include <utility>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#ifdef FT_COLOR_H
#  include FT_COLOR_H
#endif
#include FT_IMAGE_H
#include FT_OUTLINE_H
#include FT_SIZES_H
// In the past, FT_GlyphSlot_Own_Bitmap was defined in this header file.
#include FT_SYNTHESIS_H

// FT_LOAD_COLOR and the corresponding FT_Pixel_Mode::FT_PIXEL_MODE_BGRA
// were introduced in FreeType 2.5.0.
// The following may be removed once FreeType 2.5.0 is required to build.
#ifndef FT_LOAD_COLOR
#  define FT_LOAD_COLOR ( 1L << 20 )
#  define FT_PIXEL_MODE_BGRA 7
#endif

#undef FT_COLOR_H

#if Qk_DEBUG
const char* QkTraceFtrGetError(int e) {
	switch ((FT_Error)e) {
		#undef FTERRORS_H_
		#define FT_ERRORDEF( e, v, s ) case v: return s;
		#define FT_ERROR_START_LIST
		#define FT_ERROR_END_LIST
		#include FT_ERRORS_H
		#undef FT_ERRORDEF
		#undef FT_ERROR_START_LIST
		#undef FT_ERROR_END_LIST
		default: return "";
	}
}
#endif  // Qk_DEBUG

static void copyFTBitmap(const FT_Bitmap& ftsrc, FT_Bitmap &ftdst) {
	Qk_ASSERT_EQ(ftdst.width, ftsrc.width,
		"ftdst.width = %d\n"
		"ftsrc.width = %d",
		ftdst.width,
		ftsrc.width
	);
	Qk_ASSERT_EQ(ftdst.rows, ftsrc.rows,
		"ftdst.height = %d\n"
		"ftsrc.rows = %d",
		ftdst.rows,
		ftsrc.rows
	);

	const uint8_t* src = ftsrc.buffer;
	const FT_Pixel_Mode srcFormat = static_cast<FT_Pixel_Mode>(ftsrc.pixel_mode);
	const int srcPitch = ftsrc.pitch; // FT_Bitmap::pitch is an int and allowed to be negative.
	uint8_t* dst = ftdst.buffer;
	const uint32_t dstPitch = ftdst.pitch;
	const uint32_t width = ftsrc.width;
	const uint32_t height = ftsrc.rows;

	switch (srcFormat) {
		case FT_PIXEL_MODE_GRAY: {
			for (uint32_t y = height; y --> 0;) {
				memcpy(dst, src, width);
				src += srcPitch;
				dst += dstPitch;
			}
		}
		break;
		case FT_PIXEL_MODE_LCD: {
			for (uint32_t y = height; y-- > 0;) {
				const uint8_t* triple = src;
				for (int x = 0; x < width; x++) {
					*dst++ = *triple++;
					*dst++ = *triple++;
					*dst++ = *triple++;
					*dst++ = 255;
				}
				src += srcPitch;
			}
		} break;
		case FT_PIXEL_MODE_LCD_V: {
			for (int y = height; y-- > 0;) {
				const uint8_t* srcR = src;
				const uint8_t* srcG = srcR + srcPitch;
				const uint8_t* srcB = srcG + srcPitch;
				for (int x = 0; x < width; x++) {
					*(dst++) = *srcR++;
					*(dst++) = *srcG++;
					*(dst++) = *srcB++;
					*dst++ = 255;
				}
				src += 3 * srcPitch;
			}
		} break;
		case FT_PIXEL_MODE_MONO: {
			for (uint32_t y = height; y --> 0;) {
				uint8_t byte = 0;
				int bits = 0;
				const uint8_t* src_row = src;
				uint8_t* dst_row = dst;
				for (uint32_t x = width; x --> 0;) {
					if (0 == bits) {
						byte = *src_row++;
						bits = 8;
					}
					*dst_row++ = byte & 0x80 ? 0xff : 0x00;
					bits--;
					byte <<= 1;
				}
				src += srcPitch;
				dst += dstPitch;
			}
		} break;
		case FT_PIXEL_MODE_BGRA: {
			// FT_PIXEL_MODE_BGRA is pre-multiplied.
			for (uint32_t y = height; y --> 0;) {
				const uint8_t* src_row = src;
				Color* dst_row = reinterpret_cast<Color*>(dst);
				for (uint32_t x = 0; x < width; ++x) {
					uint8_t b = *src_row++;
					uint8_t g = *src_row++;
					uint8_t r = *src_row++;
					uint8_t a = *src_row++;
					*dst_row++ = Color(r, g, b, a);
	#ifdef Qk_SHOW_TEXT_BLIT_COVERAGE
					*(dst_row-1) = (dst_row-1)->blendSrcOver(Color(0x40,0x40,0x40,0x40));
	#endif
				}
				src += srcPitch;
				dst += dstPitch;
			}
		} break;
		default:
			Qk_DLog("FT_Pixel_Mode %d\n", srcFormat);
			Qk_ASSERT(0, "unsupported combination of FT_Pixel_Mode and MaskFormat");
			break;
	}
}

void QkTypeface_FreeType::generateGlyphImage(cFontGlyphMetrics &glyph, Pixel &pixel, uint32_t top) {
	FT_Bitmap dst;
	dst.width = ceilf(glyph.fWidth);
	dst.rows = ceilf(glyph.fHeight);
	dst.pitch = pixel.rowbytes();
	dst.buffer = pixel.val();
	dst.buffer += Int32::max(top + int32_t(glyph.fTop), 0) * dst.pitch;
	dst.buffer += int32_t(glyph.fLeft) * Pixel::bytes_per_pixel(pixel.type());

	if ( fFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE ) {
		Qk_ASSERT_EQ(pixel.type(), kAlpha_8_ColorType);

		FT_Outline* outline = &fFace->glyph->outline;

		dst.pixel_mode = FT_PIXEL_MODE_GRAY;
		dst.num_grays = 256;

		FT_Outline_Get_Bitmap(fFace->glyph->library, outline, &dst);
#ifdef Qk_SHOW_TEXT_BLIT_COVERAGE
		for (int y = 0; y < dst.rows; ++y) {
			for (int x = 0; x < dst.width; ++x) {
				uint8_t& a = (dst.buffer)[(dst.pitch * y) + x];
				a = std::max<uint8_t>(a, 0x20);
			}
		}
#endif
	} else {
		Qk_ASSERT_EQ(fFace->glyph->format, FT_GLYPH_FORMAT_BITMAP);

		copyFTBitmap(fFace->glyph->bitmap, dst);
	}
}

///////////////////////////////////////////////////////////////////////////////

class QkFTGeometrySink {
	Path* fPath;
	bool fStarted;
	FT_Vector fCurrent;

	void goingTo(const FT_Vector* pt) {
		if (!fStarted) {
			fStarted = true;
			fPath->moveTo({QkFDot6ToScalar(fCurrent.x), -QkFDot6ToScalar(fCurrent.y)});
		}
		fCurrent = *pt;
	}

	bool currentIsNot(const FT_Vector* pt) {
		return fCurrent.x != pt->x || fCurrent.y != pt->y;
	}

public:
	static int Move(const FT_Vector* pt, void* ctx) {
		QkFTGeometrySink& self = *(QkFTGeometrySink*)ctx;
		if (self.fStarted) {
			self.fPath->close();
			self.fStarted = false;
		}
		self.fCurrent = *pt;
		return 0;
	}

	static int Line(const FT_Vector* pt, void* ctx) {
		QkFTGeometrySink& self = *(QkFTGeometrySink*)ctx;
		if (self.currentIsNot(pt)) {
			self.goingTo(pt);
			self.fPath->lineTo({QkFDot6ToScalar(pt->x), -QkFDot6ToScalar(pt->y)});
		}
		return 0;
	}

	static int Quad(const FT_Vector* pt0, const FT_Vector* pt1, void* ctx) {
		QkFTGeometrySink& self = *(QkFTGeometrySink*)ctx;
		if (self.currentIsNot(pt0) || self.currentIsNot(pt1)) {
			self.goingTo(pt1);
			self.fPath->quadTo({QkFDot6ToScalar(pt0->x), -QkFDot6ToScalar(pt0->y)},
							{QkFDot6ToScalar(pt1->x), -QkFDot6ToScalar(pt1->y)});
		}
		return 0;
	}

	static int Cubic(const FT_Vector* pt0, const FT_Vector* pt1, const FT_Vector* pt2, void* ctx) {
		QkFTGeometrySink& self = *(QkFTGeometrySink*)ctx;
		if (self.currentIsNot(pt0) || self.currentIsNot(pt1) || self.currentIsNot(pt2)) {
			self.goingTo(pt2);
			self.fPath->cubicTo({QkFDot6ToScalar(pt0->x), -QkFDot6ToScalar(pt0->y)},
								{QkFDot6ToScalar(pt1->x), -QkFDot6ToScalar(pt1->y)},
								{QkFDot6ToScalar(pt2->x), -QkFDot6ToScalar(pt2->y)});
		}
		return 0;
	}

	QkFTGeometrySink(Path* path) : fPath{path}, fStarted{false}, fCurrent{0,0} {}
};

static constexpr const FT_Outline_Funcs Funcs{
	/*move_to =*/ QkFTGeometrySink::Move,
	/*line_to =*/ QkFTGeometrySink::Line,
	/*conic_to =*/ QkFTGeometrySink::Quad,
	/*cubic_to =*/ QkFTGeometrySink::Cubic,
	/*shift = */ 0,
	/*delta =*/ 0,
};

bool QkTypeface_FreeType::generateFacePath(Path* path) {
	QkFTGeometrySink sink{path};
	FT_Error err = FT_Outline_Decompose(&fFace->glyph->outline, &Funcs, &sink);

	if (err != 0) {
		*path = Path();
		return false;
	}

	path->close();
	return true;
}
