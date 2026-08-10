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

#include <ft2build.h>
#include <freetype/ftsizes.h>
#include <freetype/ftsystem.h>

#include <freetype/ftmodapi.h>
#include <freetype/ftmm.h>
#include <freetype/tttables.h>
#include <freetype/t1tables.h>

typedef QkTypeface_FreeType::Scanner Scanner;

using FT_Alloc_size_t_check = QkCallableTraits<FT_Alloc_Func>::argument<1>::type;

static_assert(std::is_same<FT_Alloc_size_t_check, long>::value ||
			std::is_same<FT_Alloc_size_t_check, size_t>::value, "");

void QkFT_Done_Size(FT_Size obj) {
	FT_Done_Size(obj);
}

void QkFT_Done_Face(FT_Face obj) {
	FT_Done_Face(obj);
}

void* qk_ft_alloc(FT_Memory mem, FT_Alloc_size_t size) {
	return ::malloc(size); // qk_malloc_throw(size);
}

void qk_ft_free(FT_Memory mem, void* block) {
	::free(block); //qk_free(block);
}

void* qk_ft_realloc(FT_Memory mem, FT_Alloc_size_t cur_size,
										FT_Alloc_size_t new_size, void* block) {
	return ::realloc(block, new_size); // qk_realloc_throw(block, new_size);
}

unsigned long qk_ft_stream_io(FT_Stream ftStream,
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

static FT_MemoryRec_ gFTMemory = { nullptr, qk_ft_alloc, qk_ft_free, qk_ft_realloc };

void SpFT_Done_MM_Var(FT_MM_Var* obj) {
	// FT_Done_MM_Var(gFTLibrary->library(), obj);
	gFTMemory.free(&gFTMemory, obj);
}

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
	SpFT_Face face(this->openFace(stream, -1, &streamRec));
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
	SpFT_Face face(this->openFace(stream, ttcIndex, &streamRec));
	if (!face) {
		return false;
	}

	FontWeight weight = FontWeight::Regular;
	FontWidth width = FontWidth::Normal;
	FontSlant slant = FontSlant::Normal;
	if (face->style_flags & FT_STYLE_FLAG_BOLD) {
		weight = FontWeight::Bold;
	}
	if (face->style_flags & FT_STYLE_FLAG_ITALIC) {
		slant = FontSlant::Italic;
	}

	PS_FontInfoRec psFontInfo;
	TT_OS2* os2 = static_cast<TT_OS2*>(FT_Get_Sfnt_Table(face.get(), ft_sfnt_os2));
	if (os2 && os2->version != 0xffff) {
		weight = FontWeight(os2->usWeightClass);
		width = FontWidth(os2->usWidthClass+1); // +1 because usWidthClass is 1-9

		// OS/2::fsSelection bit 9 indicates oblique.
		if (QkToBool(os2->fsSelection & (1u << 9))) {
			slant = FontSlant::Oblique;
		}
	} else if (0 == FT_Get_PS_Font_Info(face.get(), &psFontInfo) && psFontInfo.weight) {
		static Dict<String, FontWeight> commonWeights({
			// There are probably more common names, but these are known to exist.
			{ String("all"), FontWeight::Regular }, // Multiple Masters usually default to normal.
			{ String("black"), FontWeight::Black },
			{ String("bold"), FontWeight::Bold },
			{ String("book"), FontWeight((int(FontWeight::Regular) + int(FontWeight::Light)) / 2) },
			{ String("demi"), FontWeight::Semibold },
			{ String("demibold"), FontWeight::Semibold },
			{ String("extra"), FontWeight::Heavy },
			{ String("extrabold"), FontWeight::Heavy },
			{ String("extralight"), FontWeight::Ultralight },
			{ String("hairline"), FontWeight::Thin },
			{ String("heavy"), FontWeight::Black },
			{ String("light"), FontWeight::Light },
			{ String("medium"), FontWeight::Medium },
			{ String("normal"), FontWeight::Regular },
			{ String("plain"), FontWeight::Regular },
			{ String("regular"), FontWeight::Regular },
			{ String("roman"), FontWeight::Regular },
			{ String("semibold"), FontWeight::Semibold },
			{ String("standard"), FontWeight::Regular },
			{ String("thin"), FontWeight::Thin },
			{ String("ultra"), FontWeight::Heavy },
			{ String("ultrablack"), FontWeight::ExtraBlack },
			{ String("ultrabold"), FontWeight::Heavy },
			{ String("ultraheavy"), FontWeight::ExtraBlack },
			{ String("ultralight"), FontWeight::Ultralight },
		});
		FontWeight out;
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
	Qk_ASSERT(face && axes, "face and axes must be non-null");
	if (face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS) {
		FT_MM_Var* variations = nullptr;
		FT_Error err = FT_Get_MM_Var(face, &variations);
		if (err) {
			LOG_INFO("INFO: font %s claims to have variations, but none found.\n",
					 face->family_name);
			return false;
		}
		SpFT_MM_Var autoFreeVariations(variations);

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

void Scanner::computeAxisValues(
	AxisDefinitions axisDefinitions,
	const FontArguments::VariationPosition position,
	QkFixed* axisValues,
	const String& name,
	const FontArguments::VariationPosition::Coordinate* current)
{
	for (uint32_t i = 0; i < axisDefinitions.length(); ++i) {
		const Scanner::AxisDefinition& axisDefinition = axisDefinitions[i];
		const QkScalar axisMin = QkFixedToScalar(axisDefinition.fMinimum);
		const QkScalar axisMax = QkFixedToScalar(axisDefinition.fMaximum);

		// Start with the default value.
		axisValues[i] = axisDefinition.fDefault;

		// Then the current value.
		if (current) {
			for (uint32_t j = 0; j < axisDefinitions.length(); ++j) {
				const auto& coordinate = current[j];
				if (axisDefinition.fTag == coordinate.axis) {
					const QkScalar axisValue = qk::F32::clamp(coordinate.value, axisMin, axisMax);
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
				const QkScalar axisValue = qk::F32::clamp(coordinate.value, axisMin, axisMax);
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
			for (uint32_t j = 0; j < axisDefinitions.length(); ++j) {
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
