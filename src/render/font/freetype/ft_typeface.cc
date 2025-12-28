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

#include "./ft_typeface.h"
#include "../../render.h"

#include <memory>
#include <tuple>

#include <ft2build.h>
#include <freetype/ftadvanc.h>
#include <freetype/ftbitmap.h>
#ifdef FT_COLOR_H  // 2.10.0
# include <freetype/ftcolor.h>
#endif
#include <freetype/freetype.h>
#include <freetype/ftlcdfil.h>
#include <freetype/ftmodapi.h>
#include <freetype/ftmm.h>
#include <freetype/ftoutln.h>
#include <freetype/ftsizes.h>
#include <freetype/ftsystem.h>
#include <freetype/tttables.h>
#include <freetype/t1tables.h>
#include <freetype/ftfntfmt.h>

#undef FT_COLOR_H

#if 1
	#define LOG_INFO(...)
#else
	#define LOG_INFO Qk_DLog
#endif

#define Qk_GLOBAL_LOCK 0

// hand-tuned value to reduce outline embolden strength
#ifndef Qk_OUTLINE_EMBOLDEN_DIVISOR
	#ifdef Qk_BUILD_FOR_ANDROID_FRAMEWORK
			#define Qk_OUTLINE_EMBOLDEN_DIVISOR   34
	#else
			#define Qk_OUTLINE_EMBOLDEN_DIVISOR   24
	#endif
#endif

typedef QkTypeface_FreeType::Scanner Scanner;
typedef QkTypeface_FreeType::FaceRec FaceRec;

template <>
void object_traits<std::remove_pointer_t<FT_Face>>::Release(FT_Face obj) {
	FT_Done_Face(obj);
}

template <>
void object_traits<std::remove_pointer_t<FT_Size>>::Release(FT_Size obj) {
	FT_Done_Size(obj);
}

using SpFTFace = Sp<std::remove_pointer_t<FT_Face>>;

using FT_Alloc_size_t = QkCallableTraits<FT_Alloc_Func>::argument<1>::type;

static_assert(std::is_same<FT_Alloc_size_t, long>::value ||
			std::is_same<FT_Alloc_size_t, size_t>::value, "");

using QkAutoFree = Sp<void>;

static void* qk_ft_alloc(FT_Memory mem, FT_Alloc_size_t size) {
	return ::malloc(size); // qk_malloc_throw(size);
}

static void qk_ft_free(FT_Memory mem, void* block) {
	::free(block); //qk_free(block);
}

static void* qk_ft_realloc(FT_Memory mem, FT_Alloc_size_t cur_size,
										FT_Alloc_size_t new_size, void* block) {
	return ::realloc(block, new_size); // qk_realloc_throw(block, new_size);
}

static unsigned long qk_ft_stream_io(FT_Stream ftStream,
										unsigned long offset,
										unsigned char* buffer,
										unsigned long count)
{
	auto stream = static_cast<QkStream*>(ftStream->descriptor.pointer);

	if (count) {
		auto r = stream->read(buffer, count, offset);
		if (r >= 0)
			return r;
	}
	return 0;
}

static void qk_ft_stream_close(FT_Stream) {}

static bool isSubpixel(uint16_t flags) {
	return flags & kSubpixelPositioning_Flag;
}

enum class FontHinting {
	kNone,      //!< glyph outlines unchanged
	kSlight,    //!< minimal modification to improve constrast
	kNormal,    //!< glyph outlines modified to improve constrast
	kFull,      //!< modifies glyph outlines for maximum constrast
};

static FontHinting getHinting(uint16_t flags) {
	return static_cast<FontHinting>((flags & kHinting_Mask) >> kHinting_Shift);
}

/** Returns the bitmap strike equal to or just larger than the requested size. */
static FT_Int chooseBitmapStrike(FT_Face face, FT_F26Dot6 scaleY) {
	if (face == nullptr) {
		LOG_INFO("chooseBitmapStrike aborted due to nullptr face.\n");
		return -1;
	}
	FT_Pos requestedPPEM = scaleY;  // FT_Bitmap_Size::y_ppem is in 26.6 format.
	FT_Int chosenStrikeIndex = -1;
	FT_Pos chosenPPEM = 0;
	for (FT_Int strikeIndex = 0; strikeIndex < face->num_fixed_sizes; ++strikeIndex) {
		FT_Pos strikePPEM = face->available_sizes[strikeIndex].y_ppem;
		if (strikePPEM == requestedPPEM) {
			// exact match - our search stops here
			return strikeIndex;
		} else if (chosenPPEM < requestedPPEM) {
			// attempt to increase chosenPPEM
			if (chosenPPEM < strikePPEM) {
				chosenPPEM = strikePPEM;
				chosenStrikeIndex = strikeIndex;
			}
		} else {
			// attempt to decrease chosenPPEM, but not below requestedPPEM
			if (requestedPPEM < strikePPEM && strikePPEM < chosenPPEM) {
				chosenPPEM = strikePPEM;
				chosenStrikeIndex = strikeIndex;
			}
		}
	}
	return chosenStrikeIndex;
}

FT_MemoryRec_ gFTMemory = { nullptr, qk_ft_alloc, qk_ft_free, qk_ft_realloc };

class FreeTypeLibrary;
static FreeTypeLibrary* gFTLibrary = nullptr;
static FT_Int gMajor, gMinor, gPatch;
bool   gIsFT_version_2_13 = false; // 2.13.2

class FreeTypeLibrary {
	Qk_DISABLE_COPY(FreeTypeLibrary);
public:
	FreeTypeLibrary() : fLibrary(nullptr) {
		if (FT_New_Library(&gFTMemory, &fLibrary)) {
			return;
		}
		FT_Add_Default_Modules(fLibrary);
		FT_Set_Default_Properties(fLibrary);
	}
	~FreeTypeLibrary() {
		if (fLibrary) {
			FT_Done_Library(fLibrary);
		}
	}

	FT_Library library() { return fLibrary; }

private:
	FT_Library fLibrary;

	// FT_Library_SetLcdFilterWeights 2.4.0
	// FT_LOAD_COLOR 2.5.0
	// FT_Pixel_Mode::FT_PIXEL_MODE_BGRA 2.5.0
	// Thread safety in 2.6.0
	// freetype/ftfntfmt.h (rename) 2.6.0
	// Direct header inclusion 2.6.1
	// FT_Get_Var_Design_Coordinates 2.7.1
	// FT_LOAD_BITMAP_METRICS_ONLY 2.7.1
	// FT_Set_Default_Properties 2.7.2
	// The 'light' hinting is vertical only from 2.8.0
	// FT_Get_Var_Axis_Flags 2.8.1
	// FT_VAR_AXIS_FLAG_HIDDEN was introduced in FreeType 2.8.1
	// --------------------
	// FT_Done_MM_Var 2.9.0 (Currenty setting ft_free to a known allocator.)
	// freetype/ftcolor.h 2.10.0 (Currently assuming if compiled with FT_COLOR_H runtime available.)

	// Ubuntu 18.04       2.8.1
	// Debian 10          2.9.1
	// openSUSE Leap 15.2 2.10.1
	// Fedora 32          2.10.4
	// RHEL 8             2.9.1
};

static QkMutex& f_t_mutex() {
	static QkMutex& mutex = *(new QkMutex);
	return mutex;
}

constexpr float FixedUnitsScale = 64.0f; // Fixed size

// Just made up, so we don't end up storing 1000s of entries
constexpr int kMaxC2GCacheCount = 512;

// See http://freetype.sourceforge.net/freetype2/docs/reference/ft2-bitmap_handling.html#FT_Bitmap_Embolden
// This value was chosen by eyeballing the result in Firefox and trying to match it.
constexpr FT_Pos kBitmapEmboldenStrength = 1 << 6;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Scanner::Scanner() : fLibrary(nullptr) {
	if (FT_New_Library(&gFTMemory, &fLibrary)) {
		return;
	}
	FT_Add_Default_Modules(fLibrary);
	FT_Set_Default_Properties(fLibrary);
}

Scanner::~Scanner() {
	if (fLibrary) {
		FT_Done_Library(fLibrary);
	}
}

FT_Face Scanner::openFace(QkStream* stream, int ttcIndex, FT_Stream ftStream) const
{
	if (fLibrary == nullptr || stream == nullptr) {
		return nullptr;
	}

	FT_Open_Args args;
	memset(&args, 0, sizeof(args));

	const void* memoryBase = stream->getMemoryBase();

	if (memoryBase) {
		args.flags = FT_OPEN_MEMORY;
		args.memory_base = (const FT_Byte*)memoryBase;
		args.memory_size = stream->getLength();
	} else {
		memset(ftStream, 0, sizeof(*ftStream));
		ftStream->size = stream->getLength();
		ftStream->descriptor.pointer = stream;
		ftStream->read  = qk_ft_stream_io;
		ftStream->close = qk_ft_stream_close;

		args.flags = FT_OPEN_STREAM;
		args.stream = ftStream;
	}

	FT_Face face;
	if (FT_Open_Face(fLibrary, &args, ttcIndex, &face)) {
		return nullptr;
	}
	return face;
}

bool Scanner::recognizedFont(QkStream* stream, int* numFaces) const {
	QkAutoMutexExclusive libraryLock(fLibraryMutex);

	FT_StreamRec streamRec;
	SpFTFace face(this->openFace(stream, -1, &streamRec));
	if (!face) {
		return false;
	}

	*numFaces = face->num_faces;
	return true;
}

bool Scanner::scanFont(
	QkStream* stream, int ttcIndex,
	String* name, FontStyle* style, bool* isFixedPitch, AxisDefinitions* axes) const
{
	QkAutoMutexExclusive libraryLock(fLibraryMutex);

	FT_StreamRec streamRec;
	SpFTFace face(this->openFace(stream, ttcIndex, &streamRec));
	if (!face) {
		return false;
	}

	TextWeight weight = TextWeight::Regular;
	TextWidth width = TextWidth::Normal;
	TextSlant slant = TextSlant::Normal;
	if (face->style_flags & FT_STYLE_FLAG_BOLD) {
		weight = TextWeight::Bold;
	}
	if (face->style_flags & FT_STYLE_FLAG_ITALIC) {
		slant = TextSlant::Italic;
	}

	PS_FontInfoRec psFontInfo;
	TT_OS2* os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face.get(), ft_sfnt_os2));
	if (os2 && os2->version != 0xffff) {
		weight = TextWeight(os2->usWeightClass);
		width = TextWidth(os2->usWidthClass+1); // +1 because usWidthClass is 1-9

		// OS/2::fsSelection bit 9 indicates oblique.
		if (QkToBool(os2->fsSelection & (1u << 9))) {
			slant = TextSlant::Oblique;
		}
	} else if (0 == FT_Get_PS_Font_Info(face.get(), &psFontInfo) && psFontInfo.weight) {
		static Dict<String, TextWeight> commonWeights({
			// There are probably more common names, but these are known to exist.
			{ String("all"), TextWeight::Regular }, // Multiple Masters usually default to normal.
			{ String("black"), TextWeight::Black },
			{ String("bold"), TextWeight::Bold },
			{ String("book"), TextWeight((int(TextWeight::Regular) + int(TextWeight::Light)) / 2) },
			{ String("demi"), TextWeight::Semibold },
			{ String("demibold"), TextWeight::Semibold },
			{ String("extra"), TextWeight::Heavy },
			{ String("extrabold"), TextWeight::Heavy },
			{ String("extralight"), TextWeight::Ultralight },
			{ String("hairline"), TextWeight::Thin },
			{ String("heavy"), TextWeight::Black },
			{ String("light"), TextWeight::Light },
			{ String("medium"), TextWeight::Medium },
			{ String("normal"), TextWeight::Regular },
			{ String("plain"), TextWeight::Regular },
			{ String("regular"), TextWeight::Regular },
			{ String("roman"), TextWeight::Regular },
			{ String("semibold"), TextWeight::Semibold },
			{ String("standard"), TextWeight::Regular },
			{ String("thin"), TextWeight::Thin },
			{ String("ultra"), TextWeight::Heavy },
			{ String("ultrablack"), TextWeight::ExtraBlack },
			{ String("ultrabold"), TextWeight::Heavy },
			{ String("ultraheavy"), TextWeight::ExtraBlack },
			{ String("ultralight"), TextWeight::Ultralight },
		});
		TextWeight out;
		if (commonWeights.get(String(psFontInfo.weight).lowerCase(), out)) {
			weight = out;
		} else {
			LOG_INFO("Do not know weight for: %s (%s) \n", face->family_name, psFontInfo.weight);
		}
	}

	if (name != nullptr) {
		*name = face->family_name;
	}
	if (style != nullptr) {
		*style = FontStyle(weight, width, slant);
	}
	if (isFixedPitch != nullptr) {
		*isFixedPitch = FT_IS_FIXED_WIDTH(face);
	}

	if (axes != nullptr && !GetAxes(face.get(), axes)) {
		return false;
	}
	return true;
}

bool Scanner::GetAxes(FT_Face face, AxisDefinitions* axes) {
	Qk_ASSERT(face && axes);
	if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS) {
		FT_MM_Var* variations = nullptr;
		FT_Error err = FT_Get_MM_Var(face, &variations);
		if (err) {
			LOG_INFO("INFO: font %s claims to have variations, but none found.\n",
					 face->family_name);
			return false;
		}
		QkAutoFree autoFreeVariations(variations);

		axes->reset(variations->num_axis);
		for (FT_UInt i = 0; i < variations->num_axis; ++i) {
			const FT_Var_Axis& ftAxis = variations->axis[i];
			(*axes)[i].fTag = ftAxis.tag;
			(*axes)[i].fMinimum = ftAxis.minimum;
			(*axes)[i].fDefault = ftAxis.def;
			(*axes)[i].fMaximum = ftAxis.maximum;
		}
	}
	return true;
}

/*static*/ void Scanner::computeAxisValues(
	AxisDefinitions axisDefinitions,
	const FontArguments::VariationPosition position,
	QkFixed* axisValues,
	const String& name,
	const FontArguments::VariationPosition::Coordinate* current)
{
	for (int i = 0; i < axisDefinitions.length(); ++i) {
		const Scanner::AxisDefinition& axisDefinition = axisDefinitions[i];
		const QkScalar axisMin = QkFixedToScalar(axisDefinition.fMinimum);
		const QkScalar axisMax = QkFixedToScalar(axisDefinition.fMaximum);

		// Start with the default value.
		axisValues[i] = axisDefinition.fDefault;

		// Then the current value.
		if (current) {
			for (int j = 0; j < axisDefinitions.length(); ++j) {
				const auto& coordinate = current[j];
				if (axisDefinition.fTag == coordinate.axis) {
					const QkScalar axisValue = qk::Float32::clamp(coordinate.value, axisMin, axisMax);
					axisValues[i] = QkScalarToFixed(axisValue);
					break;
				}
			}
		}

		// Then the requested value.
		// The position may be over specified. If there are multiple values for a given axis,
		// use the last one since that's what css-fonts-4 requires.
		for (int j = position.coordinateCount; j --> 0;) {
			const auto& coordinate = position.coordinates[j];
			if (axisDefinition.fTag == coordinate.axis) {
				const QkScalar axisValue = qk::Float32::clamp(coordinate.value, axisMin, axisMax);
				if (coordinate.value != axisValue) {
					LOG_INFO("Requested font axis value out of range: "
							 "%s '%c%c%c%c' %f; pinned to %f.\n",
							 name.c_str(),
							 (axisDefinition.fTag >> 24) & 0xFF,
							 (axisDefinition.fTag >> 16) & 0xFF,
							 (axisDefinition.fTag >>  8) & 0xFF,
							 (axisDefinition.fTag      ) & 0xFF,
							 QkScalarToDouble(coordinate.value),
							 QkScalarToDouble(axisValue));
				}
				axisValues[i] = QkScalarToFixed(axisValue);
				break;
			}
		}
		// TODO: warn on defaulted axis?
	}

	Qk_DEBUGCODE(
		// Check for axis specified, but not matched in font.
		for (int i = 0; i < position.coordinateCount; ++i) {
			FontByteTag skTag = position.coordinates[i].axis;
			bool found = false;
			for (int j = 0; j < axisDefinitions.length(); ++j) {
				if (skTag == axisDefinitions[j].fTag) {
					found = true;
					break;
				}
			}
			if (!found) {
				LOG_INFO("Requested font axis not found: %s '%c%c%c%c'\n",
						name.c_str(),
						(skTag >> 24) & 0xFF,
						(skTag >> 16) & 0xFF,
						(skTag >>  8) & 0xFF,
						(skTag)       & 0xFF);
			}
		}
	)
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class QkTypeface_FreeType::FaceRec {
public:
	SpFTFace fFace;
	FT_StreamRec fFTStream;
	Sp<QkStream> fStream;

	// Will return nullptr on failure
	// Caller must lock f_t_mutex() before calling this function.
	static Sp<FaceRec> Make(const QkTypeface_FreeType* typeface) {
		f_t_mutex().assertHeld();

		Sp<QkFontData> data = typeface->makeFontData();
		if (data == nullptr || !data->hasStream()) {
			return nullptr;
		}

		Sp<FaceRec> rec(new FaceRec(data->detachStream()));

		FT_Open_Args args;
		memset(&args, 0, sizeof(args));
		const void* memoryBase = rec->fStream->getMemoryBase();
		if (memoryBase) {
			args.flags = FT_OPEN_MEMORY;
			args.memory_base = (const FT_Byte*)memoryBase;
			args.memory_size = rec->fStream->getLength();
		} else {
			args.flags = FT_OPEN_STREAM;
			args.stream = &rec->fFTStream;
		}

		{
			FT_Face rawFace;
			FT_Error err = FT_Open_Face(gFTLibrary->library(), &args, data->getIndex(), &rawFace);
			if (err) {
				Qk_TRACEFTR(err, "unable to open font '%x'", typeface);
				return nullptr;
			}
			rec->fFace = rawFace;
		}
		Qk_ASSERT(rec->fFace);

		rec->setupAxes(**data);

		// FreeType will set the charmap to the "most unicode" cmap if it exists.
		// If there are no unicode cmaps, the charmap is set to nullptr.
		// However, "symbol" cmaps should also be considered "fallback unicode" cmaps
		// because they are effectively private use area only (even if they aren't).
		// This is the last on the fallback list at
		// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6cmap.html
		if (!rec->fFace->charmap) {
			FT_Select_Charmap(rec->fFace.get(), FT_ENCODING_MS_SYMBOL);
		}

		return rec;
	}

	~FaceRec() {
		f_t_mutex().assertHeld();
		fFace.release(); // Must release face before the library, the library frees existing faces.
		unref_ft_library();
	}

private:
	FaceRec(Sp<QkStream> stream): fStream(std::move(stream)) {
		qk_bzero(&fFTStream, sizeof(fFTStream));
		fFTStream.size = fStream->getLength();
		fFTStream.descriptor.pointer = fStream.get();
		fFTStream.read  = qk_ft_stream_io;
		fFTStream.close = qk_ft_stream_close;

		f_t_mutex().assertHeld();
		ref_ft_library();
	}

	void setupAxes(const QkFontData& data) {
		if (!(fFace->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS)) {
			return;
		}

		// If a named variation is requested, don't overwrite the named variation's position.
		if (data.getIndex() > 0xFFFF) {
			return;
		}

		Qk_DEBUGCODE(
			FT_MM_Var* variations = nullptr;
			if (FT_Get_MM_Var(fFace.get(), &variations)) {
				LOG_INFO("INFO: font %s claims variations, but none found.\n",
						rec->fFace->family_name);
				return;
			}
			QkAutoFree autoFreeVariations(variations);

			if (static_cast<FT_UInt>(data.getAxisCount()) != variations->num_axis) {
				LOG_INFO("INFO: font %s has %d variations, but %d were specified.\n",
						rec->fFace->family_name, variations->num_axis, data.getAxisCount());
				return;
			}
		)

		Array<FT_Fixed> coords(data.getAxisCount());
		for (int i = 0; i < data.getAxisCount(); ++i) {
			coords[i] = data.getAxis()[i];
		}
		if (FT_Set_Var_Design_Coordinates(fFace.get(), data.getAxisCount(), coords.val())) {
			LOG_INFO("INFO: font %s has variations, but specified variations could not be set.\n",
					rec->fFace->family_name);
			return;
		}
	}

	// Private to ref_ft_library and unref_ft_library
	static int gFTCount;

	// Caller must lock f_t_mutex() before calling this function.
	static bool ref_ft_library() {
		f_t_mutex().assertHeld();
		Qk_ASSERT(gFTCount >= 0);

		if (0 == gFTCount) {
			Qk_ASSERT(nullptr == gFTLibrary);
			gFTLibrary = new FreeTypeLibrary;

			FT_Library_Version(gFTLibrary->library(), &gMajor, &gMinor, &gPatch);
			gIsFT_version_2_13 = (gMajor >= 2 && gMinor >= 13);

			Qk_DLog("FT_Library_Version, v%d.%d.%d", gMajor, gMinor, gPatch);
		}
		++gFTCount;
		return gFTLibrary->library();
	}

	// Caller must lock f_t_mutex() before calling this function.
	static void unref_ft_library() {
		f_t_mutex().assertHeld();
		Qk_ASSERT(gFTCount > 0);

		--gFTCount;
		if (0 == gFTCount) {
			Qk_ASSERT(nullptr != gFTLibrary);
			delete gFTLibrary;
			Qk_DEBUGCODE(gFTLibrary = nullptr;)
		}
	}
};

int FaceRec::gFTCount;

class AutoFTAccess {
public:
	AutoFTAccess(const QkTypeface_FreeType* tf) : _ft(tf) {
#if Qk_GLOBAL_LOCK
		f_t_mutex().lock();
#else
		_ft->ft_mutex().lock();
#endif
	}
	~AutoFTAccess() {
#if Qk_GLOBAL_LOCK
		f_t_mutex().unlock();
#else
		_ft->ft_mutex().unlock();
#endif
	}
private:
	const QkTypeface_FreeType *_ft;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

QkTypeface_FreeType::QkTypeface_FreeType(const FontStyle& style, uint16_t flags)
	: Typeface(style)
	, fFlags(flags)
	, fFace(nullptr)
	, fFTSize(nullptr)
	, fStrikeIndex(-1)
{
}

void QkTypeface_FreeType::initFreeType() {
	QkAutoMutexExclusive ac(f_t_mutex());
	fFaceRec = FaceRec::Make(this);

	// load the font file
	if (fFaceRec == nullptr) {
		LOG_INFO("Could not create FT_Face.\n");
		return;
	}

	// compute the flags we send to Load_Glyph
	bool linearMetrics = fFlags & kLinearMetrics_Flag;
	auto hinting = getHinting(fFlags);
	FT_Int32 loadFlags = FT_LOAD_DEFAULT;

	switch (hinting) {
	case FontHinting::kNone:
		loadFlags = FT_LOAD_NO_HINTING;
		linearMetrics = true;
		break;
	case FontHinting::kSlight:
		loadFlags = FT_LOAD_TARGET_LIGHT;  // This implies FORCE_AUTOHINT
		linearMetrics = true;
		break;
	case FontHinting::kNormal:
		loadFlags = FT_LOAD_TARGET_NORMAL;
		break;
	case FontHinting::kFull:
		loadFlags = FT_LOAD_TARGET_NORMAL;
		break;
	default:
		LOG_INFO("---------- UNKNOWN hinting %d\n", hinting);
		break;
	}

	if (fFlags & kForceAutohinting_Flag) {
		loadFlags |= FT_LOAD_FORCE_AUTOHINT;
#ifdef Qk_BUILD_FOR_ANDROID_FRAMEWORK
	} else {
		loadFlags |= FT_LOAD_NO_AUTOHINT;
#endif
	}

	if ((fFlags & kEmbeddedBitmapText_Flag) == 0) {
		loadFlags |= FT_LOAD_NO_BITMAP;
	}

	// Always using FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH to get correct
	// advances, as fontconfig and cairo do.
	// See http://code.google.com/p/skia/issues/detail?id=222.
	loadFlags |= FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH;

	// Use vertical layout if requested.
	// loadFlags |= FT_LOAD_VERTICAL_LAYOUT;

	loadFlags |= FT_LOAD_COLOR;

	FT_Error err = FT_New_Size(fFaceRec->fFace.get(), &fFTSize);
	if (err != 0) {
		Qk_TRACEFTR(err, "FT_New_Size(%s) failed.", fFaceRec->fFace->family_name);
		return;
	}

	err = FT_Activate_Size(fFTSize);
	if (err != 0) {
		Qk_TRACEFTR(err, "FT_Activate_Size(%s) failed.", fFaceRec->fFace->family_name);
		return;
	}

	if ( FT_IS_SCALABLE(fFaceRec->fFace) ) {
		//
	} else if (FT_HAS_FIXED_SIZES(fFaceRec->fFace)) {
		// FreeType does not provide linear metrics for bitmap fonts.
		linearMetrics = false;

		// FreeType documentation says:
		// FT_LOAD_NO_BITMAP -- Ignore bitmap strikes when loading.
		// Bitmap-only fonts ignore this flag.
		//
		// However, in FreeType 2.5.1 color bitmap only fonts do not ignore this flag.
		// Force this flag off for bitmap only fonts.
		loadFlags &= ~FT_LOAD_NO_BITMAP;
	} else {
		LOG_INFO("Unknown kind of font \"%s\" size %f.\n", fFaceRec->fFace->family_name, 64.0);
		return;
	}

#ifdef FT_COLOR_H
	FT_Palette_Select(fFaceRec->fFace.value(), 0, nullptr);
#endif

	fFace = fFaceRec->fFace.get();
	fDoLinearMetrics = linearMetrics;
	fLoadGlyphFlags = loadFlags;
}

QkTypeface_FreeType::~QkTypeface_FreeType() {
	if (fFaceRec || fFTSize) {
		QkAutoMutexExclusive ac(f_t_mutex());
		if (fFTSize)
			FT_Done_Size(fFTSize);
		fFTSize = nullptr;
		fFaceRec = nullptr;
	}
}

bool QkTypeface_FreeType::onGetPostScriptName(String* skPostScriptName) const {
	AutoFTAccess fta(this);
	if (!fFace) {
		return false;
	}

	const char* ftPostScriptName = FT_Get_Postscript_Name(fFace);
	if (!ftPostScriptName) {
		return false;
	}
	if (skPostScriptName) {
		*skPostScriptName = ftPostScriptName;
	}
	return true;
}

int QkTypeface_FreeType::GetUnitsPerEm(FT_Face face) {
	Qk_ASSERT(face);

	auto upem = face->units_per_EM;
	// At least some versions of FreeType set face->units_per_EM to 0 for bitmap only fonts.
	if (upem == 0) {
		TT_Header* ttHeader = (TT_Header*)FT_Get_Sfnt_Table(face, ft_sfnt_head);
		if (ttHeader) {
			upem = ttHeader->Units_Per_EM;
		}
	}
	return upem;
}

int QkTypeface_FreeType::onGetUPEM() const {
	AutoFTAccess fta(this);
	if (!fFace) {
		return 0;
	}
	return GetUnitsPerEm(fFace);
}

void QkTypeface_FreeType::onCharsToGlyphs(const Unichar uni[], int count, GlyphID glyphs[]) const {
	// Try the cache first, *before* accessing freetype lib/face, as that
	// can be very slow. If we do need to compute a new glyphID, then
	// access those freetype objects and continue the loop.

	int i;
	{
		// Optimistically use a shared lock.
		QkAutoSharedMutexShared ama(mutex());
		for (i = 0; i < count; ++i) {
			int index = fC2GCache.findGlyphIndex(uni[i]);
			if (index < 0) {
				break;
			}
			glyphs[i] = QkToU16(index);
		}
		if (i == count) {
			// we're done, no need to access the freetype objects
			return;
		}
	}

	// Need to add more so grab an exclusive lock.
#if Qk_GLOBAL_LOCK
	QkAutoSharedMutexExclusive ama(mutex());
#endif
	AutoFTAccess fta(this);
	if (!fFace) {
		qk_bzero(glyphs, count * sizeof(glyphs[0]));
		return;
	}

	for (; i < count; ++i) {
		Unichar c = uni[i];
		int index = fC2GCache.findGlyphIndex(c);
		if (index >= 0) {
			glyphs[i] = QkToU16(index);
		} else {
			glyphs[i] = QkToU16(FT_Get_Char_Index(fFace, c));
			fC2GCache.insertCharAndGlyph(~index, c, glyphs[i]);
		}
	}

	if (fC2GCache.count() > kMaxC2GCacheCount) {
		fC2GCache.reset();
	}
}

int QkTypeface_FreeType::onCountGlyphs() const {
	return fFace ? fFace->num_glyphs : 0;
}

int QkTypeface_FreeType::onGetTableTags(FontTableTag tags[]) const {
	AutoFTAccess fta(this);
	if (!fFace)
		return 0;

	FT_ULong tableCount = 0;
	FT_Error error;

	// When 'tag' is nullptr, returns number of tables in 'length'.
	error = FT_Sfnt_Table_Info(fFace, 0, nullptr, &tableCount);
	if (error) {
		return 0;
	}

	if (tags) {
		for (FT_ULong tableIndex = 0; tableIndex < tableCount; ++tableIndex) {
			FT_ULong tableTag;
			FT_ULong tablelength;
			error = FT_Sfnt_Table_Info(fFace, tableIndex, &tableTag, &tablelength);
			if (error) {
				return 0;
			}
			tags[tableIndex] = static_cast<FontTableTag>(tableTag);
		}
	}
	return tableCount;
}

size_t QkTypeface_FreeType::onGetTableData(
		FontTableTag tag, size_t offset, size_t length, void* data) const
{
	AutoFTAccess fta(this);
	if (!fFace)
		return 0;

	FT_ULong tableLength = 0;
	FT_Error error;

	// When 'length' is 0 it is overwritten with the full table length; 'offset' is ignored.
	error = FT_Load_Sfnt_Table(fFace, tag, 0, nullptr, &tableLength);
	if (error) {
		return 0;
	}

	if (offset > tableLength) {
		return 0;
	}
	FT_ULong size = std::min((FT_ULong)length, tableLength - (FT_ULong)offset);
	if (data) {
		error = FT_Load_Sfnt_Table(fFace, tag, offset, reinterpret_cast<FT_Byte*>(data), &size);
		if (error) {
			return 0;
		}
	}

	return size;
}

Qk_DEFINE_INLINE_MEMBERS(QkTypeface_FreeType, Inl) {
	#define _this static_cast<QkTypeface_FreeType::Inl*>(this)
public:

/*  We call this before each use of the fFace, since we may be sharing
		this face with other context (at different sizes).
*/
FT_Error setupSize(float fontSize, float *scaleOut) {
#if Qk_GLOBAL_LOCK
	f_t_mutex().assertHeld();
#else
	ft_mutex().assertHeld();
#endif

	if (!fFace)
		return -1;

	FT_Error err;
	FT_F26Dot6 scaleY = QkScalarToFDot6(fontSize);

	if (FT_IS_SCALABLE(fFace)) {
		err = FT_Set_Char_Size(fFace, 0, scaleY, 72, 72);
		if (err != 0) {
			Qk_TRACEFTR(err, "FT_Set_CharSize(%s, %f, %f) failed.",
									fFace->family_name, fontSize, fontSize);
			return err;
		}
		*scaleOut = 1;
	} else {
		Qk_ASSERT(FT_HAS_FIXED_SIZES(fFace));

		fStrikeIndex = chooseBitmapStrike(fFace, scaleY);
		if (fStrikeIndex == -1) {
			LOG_INFO("No glyphs for font \"%s\" size %f.\n", fFace->family_name, fontSize);
			return -1;
		}

		err = FT_Select_Size(fFace, fStrikeIndex);
		if (err != 0) {
			Qk_TRACEFTR(err, "FT_Select_Size(%s, %d) failed.",
									fFace->family_name, fStrikeIndex);
			fStrikeIndex = -1;
			return err;
		}

		*scaleOut = fFace->size->metrics.y_ppem / fontSize;
	}

	return 0;
}

void emboldenIfNeeded(FT_Face face, FT_GlyphSlot glyph, GlyphID gid) {
	// check to see if the embolden bit is set
	if (0 == (fFlags & kEmbolden_Flag)) {
		return;
	}

	switch (glyph->format) {
		case FT_GLYPH_FORMAT_OUTLINE:
			FT_Pos strength;
			strength = FT_MulFix(face->units_per_EM, face->size->metrics.y_scale)
									/ Qk_OUTLINE_EMBOLDEN_DIVISOR;
			FT_Outline_Embolden(&glyph->outline, strength);
			break;
		case FT_GLYPH_FORMAT_BITMAP:
			if (!fFace->glyph->bitmap.buffer) {
				FT_Load_Glyph(fFace, gid, fLoadGlyphFlags);
			}
			FT_GlyphSlot_Own_Bitmap(glyph);
			FT_Bitmap_Embolden(glyph->library, &glyph->bitmap, kBitmapEmboldenStrength, 0);
			break;
		default:
			Qk_ASSERT(0, "unknown glyph format");
	}
}

bool getCBoxForLetter(char letter, FT_BBox* bbox) {
	const FT_UInt glyph_id = FT_Get_Char_Index(fFace, letter);
	if (!glyph_id) {
		return false;
	}
	if (FT_Load_Glyph(fFace, glyph_id, fLoadGlyphFlags) != 0) {
		return false;
	}
	emboldenIfNeeded(fFace, fFace->glyph, QkTo<GlyphID>(glyph_id));
	FT_Outline_Get_CBox(&fFace->glyph->outline, bbox);
	return true;
}

void getBBoxForCurrentGlyph(FT_BBox* bbox, bool snapToPixelBoundary) {

	FT_Outline_Get_CBox(&fFace->glyph->outline, bbox);

	// The final result here is an offset of 0.25 to 0.75 pixels, temporarily disable this feature.
	// if (isSubpixel(fFlags)) {
	// 	int dx = QkFixedToFDot6(glyph->getSubXFixed());
	// 	int dy = QkFixedToFDot6(glyph->getSubYFixed());
	// 	// negate dy since freetype-y-goes-up and skia-y-goes-down
	// 	bbox->xMin += dx;
	// 	bbox->yMin -= dy;
	// 	bbox->xMax += dx;
	// 	bbox->yMax -= dy;
	// }

	// outset the box to integral boundaries
	if (snapToPixelBoundary) {
		bbox->xMin &= ~63;
		bbox->yMin &= ~63;
		bbox->xMax  = (bbox->xMax + 63) & ~63;
		bbox->yMax  = (bbox->yMax + 63) & ~63;
	}
}

};

void QkTypeface_FreeType::onGetGlyphMetrics(GlyphID id, FontGlyphMetrics* glyph) {
	Qk_ASSERT(glyph);

#if Qk_GLOBAL_LOCK
	AutoFTAccess fta(this);
#endif
	#define Qk_zeroMetrics() qk_bzero(glyph, sizeof(*glyph))

	float scale;
	if (_this->setupSize(FixedUnitsScale, &scale)) {
		Qk_zeroMetrics();
		return;
	}

	FT_Error err;
	err = FT_Load_Glyph( fFace, id, fLoadGlyphFlags | FT_LOAD_BITMAP_METRICS_ONLY );
	if (err != 0) {
		Qk_zeroMetrics();
		return;
	}
	_this->emboldenIfNeeded(fFace, fFace->glyph, id);

	glyph->id = id;

	if (fFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
		using FT_PosLimits = std::numeric_limits<FT_Pos>;
		FT_BBox bounds = { FT_PosLimits::max(), FT_PosLimits::max(),
												FT_PosLimits::min(), FT_PosLimits::min() };
		if (0 < fFace->glyph->outline.n_contours) {
			_this->getBBoxForCurrentGlyph(&bounds, false);
		} else {
			bounds = { 0, 0, 0, 0 };
		}
		FT_Pos width  =  bounds.xMax - bounds.xMin;
		FT_Pos height =  bounds.yMax - bounds.yMin;
		FT_Pos top    = -bounds.yMax;  // Freetype y-up, Qkia y-down.
		FT_Pos left   =  bounds.xMin;

		glyph->fWidth  = QkFDot6ToFloat(width );
		glyph->fHeight = QkFDot6ToFloat(height);
		glyph->fTop    = QkFDot6ToFloat(top   );
		glyph->fLeft   = QkFDot6ToFloat(left  );

	} else if (fFace->glyph->format == FT_GLYPH_FORMAT_BITMAP) {

		glyph->fWidth   = QkIntToScalar(fFace->glyph->bitmap.width * scale);
		glyph->fHeight  = QkIntToScalar(fFace->glyph->bitmap.rows  * scale);
		glyph->fTop     = -QkIntToScalar(fFace->glyph->bitmap_top  * scale);
		glyph->fLeft    = QkIntToScalar(fFace->glyph->bitmap_left  * scale);
	} else {
		Qk_ASSERT(0, "unknown glyph format");
		Qk_zeroMetrics();
		return;
	}

	if (fDoLinearMetrics) {
		auto advanceScalar = QkFixedToScalar(fFace->glyph->linearHoriAdvance);
		glyph->fAdvanceX = glyph->fAdvanceY = advanceScalar * scale;
	} else {
		glyph->fAdvanceX = QkFDot6ToFloat(fFace->glyph->advance.x * scale);
		glyph->fAdvanceY = -QkFDot6ToFloat(fFace->glyph->advance.y * scale);
	}

	LOG_INFO("Metrics(glyph:%d flags:0x%x) w:%f\n", id, fLoadGlyphFlags, glyph->fWidth);
}

void QkTypeface_FreeType::onGetMetrics(FontMetrics* metrics) {
	Qk_ASSERT(metrics);

#if Qk_GLOBAL_LOCK
	AutoFTAccess fta(this);
#endif

	float scale;
	if (_this->setupSize(FixedUnitsScale, &scale)) {
		qk_bzero(metrics, sizeof(*metrics));
		return;
	}

	FT_Face face = fFaceRec->fFace.get();
	metrics->fFlags = 0;

	QkScalar upem = QkIntToScalar(QkTypeface_FreeType::GetUnitsPerEm(face)) / scale;

	const float fScale_y = FixedUnitsScale;

	// use the os/2 table as a source of reasonable defaults.
	QkScalar x_height = 0.0f;
	QkScalar avgCharWidth = 0.0f;
	QkScalar cap_height = 0.0f;
	QkScalar strikeoutThickness = 0.0f, strikeoutPosition = 0.0f;
	TT_OS2* os2 = (TT_OS2*) FT_Get_Sfnt_Table(face, ft_sfnt_os2);
	if (os2) {
		x_height = QkIntToScalar(os2->sxHeight) / upem * fScale_y;
		avgCharWidth = QkIntToScalar(os2->xAvgCharWidth) / upem;
		strikeoutThickness = QkIntToScalar(os2->yStrikeoutSize) / upem;
		strikeoutPosition = -QkIntToScalar(os2->yStrikeoutPosition) / upem;
		metrics->fFlags |= FontMetrics::kStrikeoutThicknessIsValid_Flag;
		metrics->fFlags |= FontMetrics::kStrikeoutPositionIsValid_Flag;
		if (os2->version != 0xFFFF && os2->version >= 2) {
			cap_height = QkIntToScalar(os2->sCapHeight) / upem * fScale_y;
		}
	}

	// pull from format-specific metrics as needed
	QkScalar ascent, descent, leading, xmin, xmax, ymin, ymax;
	QkScalar underlineThickness, underlinePosition;
	if (face->face_flags & FT_FACE_FLAG_SCALABLE) { // scalable outline font
		// FreeType will always use HHEA metrics if they're not zero.
		// It completely ignores the OS/2 fsSelection::UseTypoMetrics bit.
		// It also ignores the VDMX tables, which are also of interest here
		// (and override everything else when they apply).
		static const int kUseTypoMetricsMask = (1 << 7);
		if (os2 && os2->version != 0xFFFF && (os2->fsSelection & kUseTypoMetricsMask)) {
			ascent = -QkIntToScalar(os2->sTypoAscender) / upem;
			descent = -QkIntToScalar(os2->sTypoDescender) / upem;
			leading = QkIntToScalar(os2->sTypoLineGap) / upem;
		} else {
			ascent = -QkIntToScalar(face->ascender) / upem;
			descent = -QkIntToScalar(face->descender) / upem;
			leading = QkIntToScalar(face->height + (face->descender - face->ascender)) / upem;
		}
		xmin = QkIntToScalar(face->bbox.xMin) / upem;
		xmax = QkIntToScalar(face->bbox.xMax) / upem;
		ymin = -QkIntToScalar(face->bbox.yMin) / upem;
		ymax = -QkIntToScalar(face->bbox.yMax) / upem;
		underlineThickness = QkIntToScalar(face->underline_thickness) / upem;
		underlinePosition = -QkIntToScalar(face->underline_position + face->underline_thickness / 2) / upem;

		metrics->fFlags |= FontMetrics::kUnderlineThicknessIsValid_Flag;
		metrics->fFlags |= FontMetrics::kUnderlinePositionIsValid_Flag;

		// we may be able to synthesize x_height and cap_height from outline
		if (!x_height) {
			FT_BBox bbox;
			if (_this->getCBoxForLetter('x', &bbox)) {
				x_height = QkIntToScalar(bbox.yMax) / 64.0f;
			}
		}
		if (!cap_height) {
			FT_BBox bbox;
			if (_this->getCBoxForLetter('H', &bbox)) {
				cap_height = QkIntToScalar(bbox.yMax) / 64.0f;
			}
		}
	} else if (fStrikeIndex != -1) { // bitmap strike metrics
		QkScalar xppem = QkIntToScalar(face->size->metrics.x_ppem) * scale;
		QkScalar yppem = QkIntToScalar(face->size->metrics.y_ppem) * scale;
		ascent = -QkIntToScalar(face->size->metrics.ascender) / (yppem * 64.0f);
		descent = -QkIntToScalar(face->size->metrics.descender) / (yppem * 64.0f);
		leading = (QkIntToScalar(face->size->metrics.height) / (yppem * 64.0f)) + ascent - descent;

		xmin = 0.0f;
		xmax = QkIntToScalar(face->available_sizes[fStrikeIndex].width) / xppem;
		ymin = descent;
		ymax = ascent;
		// The actual bitmaps may be any size and placed at any offset.
		metrics->fFlags |= FontMetrics::kBoundsInvalid_Flag;

		underlineThickness = 0;
		underlinePosition = 0;
		metrics->fFlags &= ~FontMetrics::kUnderlineThicknessIsValid_Flag;
		metrics->fFlags &= ~FontMetrics::kUnderlinePositionIsValid_Flag;

		TT_Postscript* post = (TT_Postscript*) FT_Get_Sfnt_Table(face, ft_sfnt_post);
		if (post) {
			underlineThickness = QkIntToScalar(post->underlineThickness) / upem;
			underlinePosition = -QkIntToScalar(post->underlinePosition) / upem;
			metrics->fFlags |= FontMetrics::kUnderlineThicknessIsValid_Flag;
			metrics->fFlags |= FontMetrics::kUnderlinePositionIsValid_Flag;
		}
	} else {
		qk_bzero(metrics, sizeof(*metrics));
		return;
	}

	// synthesize elements that were not provided by the os/2 table or format-specific metrics
	if (!x_height) {
		x_height = -ascent * fScale_y;
	}
	if (!avgCharWidth) {
		avgCharWidth = xmax - xmin;
	}
	if (!cap_height) {
		cap_height = -ascent * fScale_y;
	}

	// disallow negative linespacing
	if (leading < 0.0f) {
		leading = 0.0f;
	}

	metrics->fTop = ymax * fScale_y;
	metrics->fAscent = ascent * fScale_y;
	metrics->fDescent = descent * fScale_y;
	metrics->fBottom = ymin * fScale_y;
	metrics->fLeading = leading * fScale_y;
	metrics->fAvgCharWidth = avgCharWidth * fScale_y;
	metrics->fXMin = xmin * fScale_y;
	metrics->fXMax = xmax * fScale_y;
	metrics->fMaxCharWidth = metrics->fXMax - metrics->fXMin;
	metrics->fXHeight = x_height;
	metrics->fCapHeight = cap_height;
	metrics->fUnderlineThickness = underlineThickness * fScale_y;
	metrics->fUnderlinePosition = underlinePosition * fScale_y;
	metrics->fStrikeoutThickness = strikeoutThickness * fScale_y;
	metrics->fStrikeoutPosition = strikeoutPosition * fScale_y;

	if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS) {
		// The bounds are only valid for the default variation.
		metrics->fFlags |= FontMetrics::kBoundsInvalid_Flag;
	}
}

bool QkTypeface_FreeType::onGetPath(GlyphID glyphID, Path *path) {
	Qk_ASSERT(path);

#if Qk_GLOBAL_LOCK
	AutoFTAccess fta(this);
#endif

	float scale;
	// FT_IS_SCALABLE is documented to mean the face contains outline glyphs.
	if (_this->setupSize(FixedUnitsScale, &scale) || !FT_IS_SCALABLE(fFace)) {
		return false;
	}

	uint32_t flags = fLoadGlyphFlags;
	flags |= FT_LOAD_NO_BITMAP; // ignore embedded bitmaps so we're sure to get the outline
	flags &= ~FT_LOAD_RENDER;   // don't scan convert (we just want the outline)

	FT_Error err = FT_Load_Glyph(fFace, glyphID, flags);
	if (err != 0 || fFace->glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
		return false;
	}
	_this->emboldenIfNeeded(fFace, fFace->glyph, glyphID);

	return generateFacePath(path);
}

Typeface::TextImage QkTypeface_FreeType::onGetImage(cArray<GlyphID>& glyphs, float fontSize,
	cArray<Vec2> *offset, float padding, bool antiAlias, RenderBackend *render)
{
	Array<FontGlyphMetrics> gms = getGlyphsMetrics(glyphs);

#if !Qk_GLOBAL_LOCK
	QkAutoMutexExclusive ac(f_t_mutex());
#endif
	AutoFTAccess fta(this);

	#define Return() return { ImageSource::Make(PixelInfo()) }

	float needToScale;
	if (_this->setupSize(fontSize, &needToScale)) {
		Return();
	}

	const Vec2* off = offset ? offset->val(): nullptr;
	const float scale = fontSize * needToScale / FixedUnitsScale;
	float top = 0, bottom = 0;
	float right = offset ? off->x() * needToScale: 0;

	for (auto &gm: gms) {
		gm.fLeft *= scale;
		gm.fTop *= scale;
		gm.fWidth *= scale;
		gm.fHeight *= scale;
		if (offset) {
			gm.fTop += off->y() * needToScale;
			// Note: Not very useful at the moment, used as offset value x from the pixel
			gm.fAdvanceY = (off++)->x() * needToScale;
			right = fmax(right,  gm.fAdvanceX * scale + gm.fAdvanceY);
		} else {
			// Note: Used as offset value x from the pixel
			gm.fAdvanceY = right;
			right += gm.fAdvanceX * scale;
		}
		top = qk::Float32::max(top, -gm.fTop);
		bottom = qk::Float32::max(bottom, gm.fHeight + gm.fTop);
	}

	FT_Glyph_Format ft_format = fFace->glyph->format;
	uint32_t w = ceilf(right);
	uint32_t h = ceilf(top + bottom);
	int paddInt = Qk_Min(h, w) * padding;
	w += paddInt * 2;
	h += paddInt * 2;
	top += paddInt;

	if (!h || !w) {
		Return();
	}

	ColorType type;
	if (ft_format == FT_GLYPH_FORMAT_OUTLINE) {
		type = kAlpha_8_ColorType;
	}
	else if ( ft_format == FT_GLYPH_FORMAT_BITMAP) {
		switch (fFace->glyph->bitmap.pixel_mode) {
		case FT_PIXEL_MODE_MONO:
		case FT_PIXEL_MODE_GRAY: type = kAlpha_8_ColorType; break;
		case FT_PIXEL_MODE_BGRA: type = kRGBA_8888_ColorType; break;
		case FT_PIXEL_MODE_LCD:
		case FT_PIXEL_MODE_LCD_V: type = kRGB_888X_ColorType; break;
		default:
			Qk_DLog("Unknown pixel mode %d", fFace->glyph->bitmap.pixel_mode);
			Return();
		}
	} else {
		Qk_DLog("Unknown glyph format %d", ft_format);
		Return();
	}

	PixelInfo info(w, h, type, kUnknown_AlphaType);
	Pixel pixel(info, Buffer(info.bytes()));
	memset(pixel.val(), 0, pixel.length());

	FT_Pixel_Mode mode = antiAlias ? FT_PIXEL_MODE_GRAY: FT_PIXEL_MODE_MONO;
	Vec2 imgBaseline{float(paddInt), top};

	for (auto &gm: gms) {
		if (FT_Load_Glyph(fFace, gm.id, fLoadGlyphFlags) != 0)
			Return();
		_this->emboldenIfNeeded(fFace, fFace->glyph, gm.id);
		generateGlyphImage(gm, pixel, mode, imgBaseline);
	}

	return {
		.image = ImageSource::Make(std::move(pixel), render),
		.left = float(paddInt),
		.top = top,
		.width = right,
		.fontSize = fontSize,
		.scale = needToScale,
	};
}

Sp<QkFontData> QkTypeface_FreeType::makeFontData() const {
	return this->onMakeFontData();
}
