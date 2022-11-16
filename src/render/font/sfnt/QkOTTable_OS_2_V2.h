/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef QkOTTable_OS_2_V2_DEFINED
#define QkOTTable_OS_2_V2_DEFINED

#include "../util.h"
#include "QkIBMFamilyClass.h"
#include "QkPanose.h"

#pragma pack(push, 1)

struct QkOTTableOS2_V2 {
	Qk_OT_USHORT version;
	static const Qk_OT_USHORT VERSION = QkEndian_SwapBE16(2);

	Qk_OT_SHORT xAvgCharWidth;
	struct WeightClass {
		enum Value : Qk_OT_USHORT {
			Thin = QkEndian_SwapBE16(100),
			ExtraLight = QkEndian_SwapBE16(200),
			Light = QkEndian_SwapBE16(300),
			Normal = QkEndian_SwapBE16(400),
			Medium = QkEndian_SwapBE16(500),
			SemiBold = QkEndian_SwapBE16(600),
			Bold = QkEndian_SwapBE16(700),
			ExtraBold = QkEndian_SwapBE16(800),
			Black = QkEndian_SwapBE16(900),
		};
		Qk_OT_USHORT value;
	} usWeightClass;
	struct WidthClass {
		enum Value : Qk_OT_USHORT {
			UltraCondensed = QkEndian_SwapBE16(1),
			ExtraCondensed = QkEndian_SwapBE16(2),
			Condensed = QkEndian_SwapBE16(3),
			SemiCondensed = QkEndian_SwapBE16(4),
			Medium = QkEndian_SwapBE16(5),
			SemiExpanded = QkEndian_SwapBE16(6),
			Expanded = QkEndian_SwapBE16(7),
			ExtraExpanded = QkEndian_SwapBE16(8),
			UltraExpanded = QkEndian_SwapBE16(9),
		} value;
	} usWidthClass;
	union Type {
		struct Field {
			//8-15
			Qk_OT_BYTE_BITFIELD(
				NoSubsetting,
				Bitmap,
				Reserved10,
				Reserved11,
				Reserved12,
				Reserved13,
				Reserved14,
				Reserved15)
			//0-7
			Qk_OT_BYTE_BITFIELD(
				Reserved00,
				Restricted,
				PreviewPrint,
				Editable,
				Reserved04,
				Reserved05,
				Reserved06,
				Reserved07)
		} field;
		struct Raw {
			static const Qk_OT_USHORT Installable = 0;
			static const Qk_OT_USHORT RestrictedMask = QkOTSetUSHORTBit<1>::value;
			static const Qk_OT_USHORT PreviewPrintMask = QkOTSetUSHORTBit<2>::value;
			static const Qk_OT_USHORT EditableMask = QkOTSetUSHORTBit<3>::value;
			static const Qk_OT_USHORT NoSubsettingMask = QkOTSetUSHORTBit<8>::value;
			static const Qk_OT_USHORT BitmapMask = QkOTSetUSHORTBit<9>::value;
			Qk_OT_USHORT value;
		} raw;
	} fsType;
	Qk_OT_SHORT ySubscriptXSize;
	Qk_OT_SHORT ySubscriptYSize;
	Qk_OT_SHORT ySubscriptXOffset;
	Qk_OT_SHORT ySubscriptYOffset;
	Qk_OT_SHORT ySuperscriptXSize;
	Qk_OT_SHORT ySuperscriptYSize;
	Qk_OT_SHORT ySuperscriptXOffset;
	Qk_OT_SHORT ySuperscriptYOffset;
	Qk_OT_SHORT yStrikeoutSize;
	Qk_OT_SHORT yStrikeoutPosition;
	QkIBMFamilyClass sFamilyClass;
	QkPanose panose;
	union UnicodeRange {
		struct Field {
			//l0 24-31
			Qk_OT_BYTE_BITFIELD(
				Thai,
				Lao,
				Georgian,
				Reserved027,
				HangulJamo,
				LatinExtendedAdditional,
				GreekExtended,
				GeneralPunctuation)
			//l0 16-23
			Qk_OT_BYTE_BITFIELD(
				Bengali,
				Gurmukhi,
				Gujarati,
				Oriya,
				Tamil,
				Telugu,
				Kannada,
				Malayalam)
			//l0 8-15
			Qk_OT_BYTE_BITFIELD(
				Reserved008,
				Cyrillic,
				Armenian,
				Hebrew,
				Reserved012,
				Arabic,
				Reserved014,
				Devanagari)
			//l0 0-7
			Qk_OT_BYTE_BITFIELD(
				BasicLatin,
				Latin1Supplement,
				LatinExtendedA,
				LatinExtendedB,
				IPAExtensions,
				SpacingModifierLetters,
				CombiningDiacriticalMarks,
				Greek)

			//l1 24-31
			Qk_OT_BYTE_BITFIELD(
				Hangul,
				Surrogates,
				Reserved058,
				CJKUnifiedIdeographs,
				PrivateUseArea,
				CJKCompatibilityIdeographs,
				AlphabeticPresentationForms,
				ArabicPresentationFormsA)
			//l1 16-23
			Qk_OT_BYTE_BITFIELD(
				CJKSymbolsAndPunctuation,
				Hiragana,
				Katakana,
				Bopomofo,
				HangulCompatibilityJamo,
				CJKMiscellaneous,
				EnclosedCJKLettersAndMonths,
				CJKCompatibility)
			//l1 8-15
			Qk_OT_BYTE_BITFIELD(
				ControlPictures,
				OpticalCharacterRecognition,
				EnclosedAlphanumerics,
				BoxDrawing,
				BlockElements,
				GeometricShapes,
				MiscellaneousSymbols,
				Dingbats)
			//l1 0-7
			Qk_OT_BYTE_BITFIELD(
				SuperscriptsAndSubscripts,
				CurrencySymbols,
				CombiningDiacriticalMarksForSymbols,
				LetterlikeSymbols,
				NumberForms,
				Arrows,
				MathematicalOperators,
				MiscellaneousTechnical)

			//l2 24-31
			Qk_OT_BYTE_BITFIELD(
				Reserved088,
				Reserved089,
				Reserved090,
				Reserved091,
				Reserved092,
				Reserved093,
				Reserved094,
				Reserved095)
			//l2 16-23
			Qk_OT_BYTE_BITFIELD(
				Khmer,
				Mongolian,
				Braille,
				Yi,
				Reserved084,
				Reserved085,
				Reserved086,
				Reserved087)
			//l2 8-15
			Qk_OT_BYTE_BITFIELD(
				Thaana,
				Sinhala,
				Myanmar,
				Ethiopic,
				Cherokee,
				UnifiedCanadianSyllabics,
				Ogham,
				Runic)
			//l2 0-7
			Qk_OT_BYTE_BITFIELD(
				CombiningHalfMarks,
				CJKCompatibilityForms,
				SmallFormVariants,
				ArabicPresentationFormsB,
				HalfwidthAndFullwidthForms,
				Specials,
				Tibetan,
				Syriac)

			//l3 24-31
			Qk_OT_BYTE_BITFIELD(
				Reserved120,
				Reserved121,
				Reserved122,
				Reserved123,
				Reserved124,
				Reserved125,
				Reserved126,
				Reserved127)
			//l3 16-23
			Qk_OT_BYTE_BITFIELD(
				Reserved112,
				Reserved113,
				Reserved114,
				Reserved115,
				Reserved116,
				Reserved117,
				Reserved118,
				Reserved119)
			//l3 8-15
			Qk_OT_BYTE_BITFIELD(
				Reserved104,
				Reserved105,
				Reserved106,
				Reserved107,
				Reserved108,
				Reserved109,
				Reserved110,
				Reserved111)
			//l3 0-7
			Qk_OT_BYTE_BITFIELD(
				Reserved096,
				Reserved097,
				Reserved098,
				Reserved099,
				Reserved100,
				Reserved101,
				Reserved102,
				Reserved103)
		} field;
		struct Raw {
			struct l0 {
				static const Qk_OT_ULONG BasicLatinMask = QkOTSetULONGBit<0>::value;
				static const Qk_OT_ULONG Latin1SupplementMask = QkOTSetULONGBit<1>::value;
				static const Qk_OT_ULONG LatinExtendedAMask = QkOTSetULONGBit<2>::value;
				static const Qk_OT_ULONG LatinExtendedBMask = QkOTSetULONGBit<3>::value;
				static const Qk_OT_ULONG IPAExtensionsMask = QkOTSetULONGBit<4>::value;
				static const Qk_OT_ULONG SpacingModifierLettersMask = QkOTSetULONGBit<5>::value;
				static const Qk_OT_ULONG CombiningDiacriticalMarksMask = QkOTSetULONGBit<6>::value;
				static const Qk_OT_ULONG GreekMask = QkOTSetULONGBit<7>::value;
				//Reserved
				static const Qk_OT_ULONG CyrillicMask = QkOTSetULONGBit<9>::value;
				static const Qk_OT_ULONG ArmenianMask = QkOTSetULONGBit<10>::value;
				static const Qk_OT_ULONG HebrewMask = QkOTSetULONGBit<11>::value;
				//Reserved
				static const Qk_OT_ULONG ArabicMask = QkOTSetULONGBit<13>::value;
				//Reserved
				static const Qk_OT_ULONG DevanagariMask = QkOTSetULONGBit<15>::value;
				static const Qk_OT_ULONG BengaliMask = QkOTSetULONGBit<16>::value;
				static const Qk_OT_ULONG GurmukhiMask = QkOTSetULONGBit<17>::value;
				static const Qk_OT_ULONG GujaratiMask = QkOTSetULONGBit<18>::value;
				static const Qk_OT_ULONG OriyaMask = QkOTSetULONGBit<19>::value;
				static const Qk_OT_ULONG TamilMask = QkOTSetULONGBit<20>::value;
				static const Qk_OT_ULONG TeluguMask = QkOTSetULONGBit<21>::value;
				static const Qk_OT_ULONG KannadaMask = QkOTSetULONGBit<22>::value;
				static const Qk_OT_ULONG MalayalamMask = QkOTSetULONGBit<23>::value;
				static const Qk_OT_ULONG ThaiMask = QkOTSetULONGBit<24>::value;
				static const Qk_OT_ULONG LaoMask = QkOTSetULONGBit<25>::value;
				static const Qk_OT_ULONG GeorgianMask = QkOTSetULONGBit<26>::value;
				//Reserved
				static const Qk_OT_ULONG HangulJamoMask = QkOTSetULONGBit<28>::value;
				static const Qk_OT_ULONG LatinExtendedAdditionalMask = QkOTSetULONGBit<29>::value;
				static const Qk_OT_ULONG GreekExtendedMask = QkOTSetULONGBit<30>::value;
				static const Qk_OT_ULONG GeneralPunctuationMask = QkOTSetULONGBit<31>::value;
			};
			struct l1 {
				static const Qk_OT_ULONG SuperscriptsAndSubscriptsMask = QkOTSetULONGBit<32 - 32>::value;
				static const Qk_OT_ULONG CurrencySymbolsMask = QkOTSetULONGBit<33 - 32>::value;
				static const Qk_OT_ULONG CombiningDiacriticalMarksForSymbolsMask = QkOTSetULONGBit<34 - 32>::value;
				static const Qk_OT_ULONG LetterlikeSymbolsMask = QkOTSetULONGBit<35 - 32>::value;
				static const Qk_OT_ULONG NumberFormsMask = QkOTSetULONGBit<36 - 32>::value;
				static const Qk_OT_ULONG ArrowsMask = QkOTSetULONGBit<37 - 32>::value;
				static const Qk_OT_ULONG MathematicalOperatorsMask = QkOTSetULONGBit<38 - 32>::value;
				static const Qk_OT_ULONG MiscellaneousTechnicalMask = QkOTSetULONGBit<39 - 32>::value;
				static const Qk_OT_ULONG ControlPicturesMask = QkOTSetULONGBit<40 - 32>::value;
				static const Qk_OT_ULONG OpticalCharacterRecognitionMask = QkOTSetULONGBit<41 - 32>::value;
				static const Qk_OT_ULONG EnclosedAlphanumericsMask = QkOTSetULONGBit<42 - 32>::value;
				static const Qk_OT_ULONG BoxDrawingMask = QkOTSetULONGBit<43 - 32>::value;
				static const Qk_OT_ULONG BlockElementsMask = QkOTSetULONGBit<44 - 32>::value;
				static const Qk_OT_ULONG GeometricShapesMask = QkOTSetULONGBit<45 - 32>::value;
				static const Qk_OT_ULONG MiscellaneousSymbolsMask = QkOTSetULONGBit<46 - 32>::value;
				static const Qk_OT_ULONG DingbatsMask = QkOTSetULONGBit<47 - 32>::value;
				static const Qk_OT_ULONG CJKSymbolsAndPunctuationMask = QkOTSetULONGBit<48 - 32>::value;
				static const Qk_OT_ULONG HiraganaMask = QkOTSetULONGBit<49 - 32>::value;
				static const Qk_OT_ULONG KatakanaMask = QkOTSetULONGBit<50 - 32>::value;
				static const Qk_OT_ULONG BopomofoMask = QkOTSetULONGBit<51 - 32>::value;
				static const Qk_OT_ULONG HangulCompatibilityJamoMask = QkOTSetULONGBit<52 - 32>::value;
				static const Qk_OT_ULONG CJKMiscellaneousMask = QkOTSetULONGBit<53 - 32>::value;
				static const Qk_OT_ULONG EnclosedCJKLettersAndMonthsMask = QkOTSetULONGBit<54 - 32>::value;
				static const Qk_OT_ULONG CJKCompatibilityMask = QkOTSetULONGBit<55 - 32>::value;
				static const Qk_OT_ULONG HangulMask = QkOTSetULONGBit<56 - 32>::value;
				static const Qk_OT_ULONG SurrogatesMask = QkOTSetULONGBit<57 - 32>::value;
				//Reserved
				static const Qk_OT_ULONG CJKUnifiedIdeographsMask = QkOTSetULONGBit<59 - 32>::value;
				static const Qk_OT_ULONG PrivateUseAreaMask = QkOTSetULONGBit<60 - 32>::value;
				static const Qk_OT_ULONG CJKCompatibilityIdeographsMask = QkOTSetULONGBit<61 - 32>::value;
				static const Qk_OT_ULONG AlphabeticPresentationFormsMask = QkOTSetULONGBit<62 - 32>::value;
				static const Qk_OT_ULONG ArabicPresentationFormsAMask = QkOTSetULONGBit<63 - 32>::value;
			};
			struct l2 {
				static const Qk_OT_ULONG CombiningHalfMarksMask = QkOTSetULONGBit<64 - 64>::value;
				static const Qk_OT_ULONG CJKCompatibilityFormsMask = QkOTSetULONGBit<65 - 64>::value;
				static const Qk_OT_ULONG SmallFormVariantsMask = QkOTSetULONGBit<66 - 64>::value;
				static const Qk_OT_ULONG ArabicPresentationFormsBMask = QkOTSetULONGBit<67 - 64>::value;
				static const Qk_OT_ULONG HalfwidthAndFullwidthFormsMask = QkOTSetULONGBit<68 - 64>::value;
				static const Qk_OT_ULONG SpecialsMask = QkOTSetULONGBit<69 - 64>::value;
				static const Qk_OT_ULONG TibetanMask = QkOTSetULONGBit<70 - 64>::value;
				static const Qk_OT_ULONG SyriacMask = QkOTSetULONGBit<71 - 64>::value;
				static const Qk_OT_ULONG ThaanaMask = QkOTSetULONGBit<72 - 64>::value;
				static const Qk_OT_ULONG SinhalaMask = QkOTSetULONGBit<73 - 64>::value;
				static const Qk_OT_ULONG MyanmarMask = QkOTSetULONGBit<74 - 64>::value;
				static const Qk_OT_ULONG EthiopicMask = QkOTSetULONGBit<75 - 64>::value;
				static const Qk_OT_ULONG CherokeeMask = QkOTSetULONGBit<76 - 64>::value;
				static const Qk_OT_ULONG UnifiedCanadianSyllabicsMask = QkOTSetULONGBit<77 - 64>::value;
				static const Qk_OT_ULONG OghamMask = QkOTSetULONGBit<78 - 64>::value;
				static const Qk_OT_ULONG RunicMask = QkOTSetULONGBit<79 - 64>::value;
				static const Qk_OT_ULONG KhmerMask = QkOTSetULONGBit<80 - 64>::value;
				static const Qk_OT_ULONG MongolianMask = QkOTSetULONGBit<81 - 64>::value;
				static const Qk_OT_ULONG BrailleMask = QkOTSetULONGBit<82 - 64>::value;
				static const Qk_OT_ULONG YiMask = QkOTSetULONGBit<83 - 64>::value;
			};
			Qk_OT_ULONG value[4];
		} raw;
	} ulUnicodeRange;
	Qk_OT_CHAR achVendID[4];
	union Selection {
		struct Field {
			//8-15
			Qk_OT_BYTE_BITFIELD(
				Reserved08,
				Reserved09,
				Reserved10,
				Reserved11,
				Reserved12,
				Reserved13,
				Reserved14,
				Reserved15)
			//0-7
			Qk_OT_BYTE_BITFIELD(
				Italic,
				Underscore,
				Negative,
				Outlined,
				Strikeout,
				Bold,
				Regular,
				Reserved07)
		} field;
		struct Raw {
			static const Qk_OT_USHORT ItalicMask = QkOTSetUSHORTBit<0>::value;
			static const Qk_OT_USHORT UnderscoreMask = QkOTSetUSHORTBit<1>::value;
			static const Qk_OT_USHORT NegativeMask = QkOTSetUSHORTBit<2>::value;
			static const Qk_OT_USHORT OutlinedMask = QkOTSetUSHORTBit<3>::value;
			static const Qk_OT_USHORT StrikeoutMask = QkOTSetUSHORTBit<4>::value;
			static const Qk_OT_USHORT BoldMask = QkOTSetUSHORTBit<5>::value;
			static const Qk_OT_USHORT RegularMask = QkOTSetUSHORTBit<6>::value;
			Qk_OT_USHORT value;
		} raw;
	} fsSelection;
	Qk_OT_USHORT usFirstCharIndex;
	Qk_OT_USHORT usLastCharIndex;
	//version0
	Qk_OT_SHORT sTypoAscender;
	Qk_OT_SHORT sTypoDescender;
	Qk_OT_SHORT sTypoLineGap;
	Qk_OT_USHORT usWinAscent;
	Qk_OT_USHORT usWinDescent;
	//version1
	union CodePageRange {
		struct Field {
			//l0 24-31
			Qk_OT_BYTE_BITFIELD(
				Reserved24,
				Reserved25,
				Reserved26,
				Reserved27,
				Reserved28,
				MacintoshCharacterSet,
				OEMCharacterSet,
				SymbolCharacterSet)
			//l0 16-23
			Qk_OT_BYTE_BITFIELD(
				Thai_874,
				JISJapan_932,
				ChineseSimplified_936,
				KoreanWansung_949,
				ChineseTraditional_950,
				KoreanJohab_1361,
				Reserved22,
				Reserved23)
			//l0 8-15
			Qk_OT_BYTE_BITFIELD(
				Vietnamese,
				Reserved09,
				Reserved10,
				Reserved11,
				Reserved12,
				Reserved13,
				Reserved14,
				Reserved15)
			//l0 0-7
			Qk_OT_BYTE_BITFIELD(
				Latin1_1252,
				Latin2EasternEurope_1250,
				Cyrillic_1251,
				Greek_1253,
				Turkish_1254,
				Hebrew_1255,
				Arabic_1256,
				WindowsBaltic_1257)

			//l1 24-31
			Qk_OT_BYTE_BITFIELD(
				IBMTurkish_857,
				IBMCyrillic_855,
				Latin2_852,
				MSDOSBaltic_775,
				Greek_737,
				Arabic_708,
				WELatin1_850,
				US_437)
			//l1 16-23
			Qk_OT_BYTE_BITFIELD(
				IBMGreek_869,
				MSDOSRussian_866,
				MSDOSNordic_865,
				Arabic_864,
				MSDOSCanadianFrench_863,
				Hebrew_862,
				MSDOSIcelandic_861,
				MSDOSPortuguese_860)
			//l1 8-15
			Qk_OT_BYTE_BITFIELD(
				Reserved40,
				Reserved41,
				Reserved42,
				Reserved43,
				Reserved44,
				Reserved45,
				Reserved46,
				Reserved47)
			//l1 0-7
			Qk_OT_BYTE_BITFIELD(
				Reserved32,
				Reserved33,
				Reserved34,
				Reserved35,
				Reserved36,
				Reserved37,
				Reserved38,
				Reserved39)
		} field;
		struct Raw {
			struct l0 {
				static const Qk_OT_ULONG Latin1_1252Mask = QkOTSetULONGBit<0>::value;
				static const Qk_OT_ULONG Latin2EasternEurope_1250Mask = QkOTSetULONGBit<1>::value;
				static const Qk_OT_ULONG Cyrillic_1251Mask = QkOTSetULONGBit<2>::value;
				static const Qk_OT_ULONG Greek_1253Mask = QkOTSetULONGBit<3>::value;
				static const Qk_OT_ULONG Turkish_1254Mask = QkOTSetULONGBit<4>::value;
				static const Qk_OT_ULONG Hebrew_1255Mask = QkOTSetULONGBit<5>::value;
				static const Qk_OT_ULONG Arabic_1256Mask = QkOTSetULONGBit<6>::value;
				static const Qk_OT_ULONG WindowsBaltic_1257Mask = QkOTSetULONGBit<7>::value;
				static const Qk_OT_ULONG Vietnamese_1258Mask = QkOTSetULONGBit<8>::value;
				static const Qk_OT_ULONG Thai_874Mask = QkOTSetULONGBit<16>::value;
				static const Qk_OT_ULONG JISJapan_932Mask = QkOTSetULONGBit<17>::value;
				static const Qk_OT_ULONG ChineseSimplified_936Mask = QkOTSetULONGBit<18>::value;
				static const Qk_OT_ULONG KoreanWansung_949Mask = QkOTSetULONGBit<19>::value;
				static const Qk_OT_ULONG ChineseTraditional_950Mask = QkOTSetULONGBit<20>::value;
				static const Qk_OT_ULONG KoreanJohab_1361Mask = QkOTSetULONGBit<21>::value;
				static const Qk_OT_ULONG MacintoshCharacterSetMask = QkOTSetULONGBit<29>::value;
				static const Qk_OT_ULONG OEMCharacterSetMask = QkOTSetULONGBit<30>::value;
				static const Qk_OT_ULONG SymbolCharacterSetMask = QkOTSetULONGBit<31>::value;
			};
			struct l1 {
				static const Qk_OT_ULONG IBMGreek_869Mask = QkOTSetULONGBit<48 - 32>::value;
				static const Qk_OT_ULONG MSDOSRussian_866Mask = QkOTSetULONGBit<49 - 32>::value;
				static const Qk_OT_ULONG MSDOSNordic_865Mask = QkOTSetULONGBit<50 - 32>::value;
				static const Qk_OT_ULONG Arabic_864Mask = QkOTSetULONGBit<51 - 32>::value;
				static const Qk_OT_ULONG MSDOSCanadianFrench_863Mask = QkOTSetULONGBit<52 - 32>::value;
				static const Qk_OT_ULONG Hebrew_862Mask = QkOTSetULONGBit<53 - 32>::value;
				static const Qk_OT_ULONG MSDOSIcelandic_861Mask = QkOTSetULONGBit<54 - 32>::value;
				static const Qk_OT_ULONG MSDOSPortuguese_860Mask = QkOTSetULONGBit<55 - 32>::value;
				static const Qk_OT_ULONG IBMTurkish_857Mask = QkOTSetULONGBit<56 - 32>::value;
				static const Qk_OT_ULONG IBMCyrillic_855Mask = QkOTSetULONGBit<57 - 32>::value;
				static const Qk_OT_ULONG Latin2_852Mask = QkOTSetULONGBit<58 - 32>::value;
				static const Qk_OT_ULONG MSDOSBaltic_775Mask = QkOTSetULONGBit<59 - 32>::value;
				static const Qk_OT_ULONG Greek_737Mask = QkOTSetULONGBit<60 - 32>::value;
				static const Qk_OT_ULONG Arabic_708Mask = QkOTSetULONGBit<61 - 32>::value;
				static const Qk_OT_ULONG WELatin1_850Mask = QkOTSetULONGBit<62 - 32>::value;
				static const Qk_OT_ULONG US_437Mask = QkOTSetULONGBit<63 - 32>::value;
			};
			Qk_OT_ULONG value[2];
		} raw;
	} ulCodePageRange;
	//version2
	Qk_OT_SHORT sxHeight;
	Qk_OT_SHORT sCapHeight;
	Qk_OT_USHORT usDefaultChar;
	Qk_OT_USHORT usBreakChar;
	Qk_OT_USHORT usMaxContext;
};

#pragma pack(pop)


static_assert(sizeof(QkOTTableOS2_V2) == 96, "sizeof_QkOTTableOS2_V2_not_96");

#endif
