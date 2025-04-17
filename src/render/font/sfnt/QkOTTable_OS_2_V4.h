/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// @private head

#ifndef QkOTTable_OS_2_V4_DEFINED
#define QkOTTable_OS_2_V4_DEFINED

#include "QkIBMFamilyClass.h"
#include "QkPanose.h"

#pragma pack(push, 1)

struct QkOTTableOS2_V4 {
	Qk_OT_USHORT version;
	static const Qk_OT_USHORT VERSION = QkEndian_SwapBE16(4);

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
				Balinese,
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
				Coptic,
				Cyrillic,
				Armenian,
				Hebrew,
				Vai,
				Arabic,
				NKo,
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
				GreekAndCoptic)

			//l1 24-31
			Qk_OT_BYTE_BITFIELD(
				Hangul,
				NonPlane0,
				Phoenician,
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
				PhagsPa,
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
				MusicalSymbols,
				MathematicalAlphanumericSymbols,
				PrivateUse,
				VariationSelectors,
				Tags,
				Limbu,
				TaiLe,
				NewTaiLue)
			//l2 16-23
			Qk_OT_BYTE_BITFIELD(
				Khmer,
				Mongolian,
				Braille,
				Yi,
				Tagalog_Hanunoo_Buhid_Tagbanwa,
				OldItalic,
				Gothic,
				Deseret)
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
				PhaistosDisc,
				Carian_Lycian_Lydian,
				DominoTiles_MahjongTiles,
				Reserved123,
				Reserved124,
				Reserved125,
				Reserved126,
				Reserved127)
			//l3 16-23
			Qk_OT_BYTE_BITFIELD(
				Sundanese,
				Lepcha,
				OlChiki,
				Saurashtra,
				KayahLi,
				Rejang,
				Cham,
				AncientSymbols)
			//l3 8-15
			Qk_OT_BYTE_BITFIELD(
				OldPersian,
				Shavian,
				Osmanya,
				CypriotSyllabary,
				Kharoshthi,
				TaiXuanJingSymbols,
				Cuneiform,
				CountingRodNumerals)
			//l3 0-7
			Qk_OT_BYTE_BITFIELD(
				Buginese,
				Glagolitic,
				Tifinagh,
				YijingHexagramSymbols,
				SylotiNagri,
				LinearB_AegeanNumbers,
				AncientGreekNumbers,
				Ugaritic)
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
				static const Qk_OT_ULONG GreekAndCopticMask = QkOTSetULONGBit<7>::value;
				static const Qk_OT_ULONG CopticMask = QkOTSetULONGBit<8>::value;
				static const Qk_OT_ULONG CyrillicMask = QkOTSetULONGBit<9>::value;
				static const Qk_OT_ULONG ArmenianMask = QkOTSetULONGBit<10>::value;
				static const Qk_OT_ULONG HebrewMask = QkOTSetULONGBit<11>::value;
				static const Qk_OT_ULONG VaiMask = QkOTSetULONGBit<12>::value;
				static const Qk_OT_ULONG ArabicMask = QkOTSetULONGBit<13>::value;
				static const Qk_OT_ULONG NKoMask = QkOTSetULONGBit<14>::value;
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
				static const Qk_OT_ULONG BalineseMask = QkOTSetULONGBit<27>::value;
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
				static const Qk_OT_ULONG PhagsPaMask = QkOTSetULONGBit<53 - 32>::value;
				static const Qk_OT_ULONG EnclosedCJKLettersAndMonthsMask = QkOTSetULONGBit<54 - 32>::value;
				static const Qk_OT_ULONG CJKCompatibilityMask = QkOTSetULONGBit<55 - 32>::value;
				static const Qk_OT_ULONG HangulMask = QkOTSetULONGBit<56 - 32>::value;
				static const Qk_OT_ULONG NonPlane0Mask = QkOTSetULONGBit<57 - 32>::value;
				static const Qk_OT_ULONG PhoenicianMask = QkOTSetULONGBit<58 - 32>::value;
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
				static const Qk_OT_ULONG Tagalog_Hanunoo_Buhid_TagbanwaMask = QkOTSetULONGBit<84 - 64>::value;
				static const Qk_OT_ULONG OldItalicMask = QkOTSetULONGBit<85 - 64>::value;
				static const Qk_OT_ULONG GothicMask = QkOTSetULONGBit<86 - 64>::value;
				static const Qk_OT_ULONG DeseretMask = QkOTSetULONGBit<87 - 64>::value;
				static const Qk_OT_ULONG MusicalSymbolsMask = QkOTSetULONGBit<88 - 64>::value;
				static const Qk_OT_ULONG MathematicalAlphanumericSymbolsMask = QkOTSetULONGBit<89 - 64>::value;
				static const Qk_OT_ULONG PrivateUseMask = QkOTSetULONGBit<90 - 64>::value;
				static const Qk_OT_ULONG VariationSelectorsMask = QkOTSetULONGBit<91 - 64>::value;
				static const Qk_OT_ULONG TagsMask = QkOTSetULONGBit<92 - 64>::value;
				static const Qk_OT_ULONG LimbuMask = QkOTSetULONGBit<93 - 64>::value;
				static const Qk_OT_ULONG TaiLeMask = QkOTSetULONGBit<94 - 64>::value;
				static const Qk_OT_ULONG NewTaiLueMask = QkOTSetULONGBit<95 - 64>::value;
			};
			struct l3 {
				static const Qk_OT_ULONG BugineseMask = QkOTSetULONGBit<96 - 96>::value;
				static const Qk_OT_ULONG GlagoliticMask = QkOTSetULONGBit<97 - 96>::value;
				static const Qk_OT_ULONG TifinaghMask = QkOTSetULONGBit<98 - 96>::value;
				static const Qk_OT_ULONG YijingHexagramSymbolsMask = QkOTSetULONGBit<99 - 96>::value;
				static const Qk_OT_ULONG SylotiNagriMask = QkOTSetULONGBit<100 - 96>::value;
				static const Qk_OT_ULONG LinearB_AegeanNumbersMask = QkOTSetULONGBit<101 - 96>::value;
				static const Qk_OT_ULONG AncientGreekNumbersMask = QkOTSetULONGBit<102 - 96>::value;
				static const Qk_OT_ULONG UgariticMask = QkOTSetULONGBit<103 - 96>::value;
				static const Qk_OT_ULONG OldPersianMask = QkOTSetULONGBit<104 - 96>::value;
				static const Qk_OT_ULONG ShavianMask = QkOTSetULONGBit<105 - 96>::value;
				static const Qk_OT_ULONG OsmanyaMask = QkOTSetULONGBit<106 - 96>::value;
				static const Qk_OT_ULONG CypriotSyllabaryMask = QkOTSetULONGBit<107 - 96>::value;
				static const Qk_OT_ULONG KharoshthiMask = QkOTSetULONGBit<108 - 96>::value;
				static const Qk_OT_ULONG TaiXuanJingSymbolsMask = QkOTSetULONGBit<109 - 96>::value;
				static const Qk_OT_ULONG CuneiformMask = QkOTSetULONGBit<110 - 96>::value;
				static const Qk_OT_ULONG CountingRodNumeralsMask = QkOTSetULONGBit<111 - 96>::value;
				static const Qk_OT_ULONG SundaneseMask = QkOTSetULONGBit<112 - 96>::value;
				static const Qk_OT_ULONG LepchaMask = QkOTSetULONGBit<113 - 96>::value;
				static const Qk_OT_ULONG OlChikiMask = QkOTSetULONGBit<114 - 96>::value;
				static const Qk_OT_ULONG SaurashtraMask = QkOTSetULONGBit<115 - 96>::value;
				static const Qk_OT_ULONG KayahLiMask = QkOTSetULONGBit<116 - 96>::value;
				static const Qk_OT_ULONG RejangMask = QkOTSetULONGBit<117 - 96>::value;
				static const Qk_OT_ULONG ChamMask = QkOTSetULONGBit<118 - 96>::value;
				static const Qk_OT_ULONG AncientSymbolsMask = QkOTSetULONGBit<119 - 96>::value;
				static const Qk_OT_ULONG PhaistosDiscMask = QkOTSetULONGBit<120 - 96>::value;
				static const Qk_OT_ULONG Carian_Lycian_LydianMask = QkOTSetULONGBit<121 - 96>::value;
				static const Qk_OT_ULONG DominoTiles_MahjongTilesMask = QkOTSetULONGBit<122 - 96>::value;
			};
			Qk_OT_ULONG value[4];
		} raw;
	} ulUnicodeRange;
	Qk_OT_CHAR achVendID[4];
	union Selection {
		struct Field {
			//8-15
			Qk_OT_BYTE_BITFIELD(
				WWS,
				Oblique,
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
				UseTypoMetrics)
		} field;
		struct Raw {
			static const Qk_OT_USHORT ItalicMask = QkOTSetUSHORTBit<0>::value;
			static const Qk_OT_USHORT UnderscoreMask = QkOTSetUSHORTBit<1>::value;
			static const Qk_OT_USHORT NegativeMask = QkOTSetUSHORTBit<2>::value;
			static const Qk_OT_USHORT OutlinedMask = QkOTSetUSHORTBit<3>::value;
			static const Qk_OT_USHORT StrikeoutMask = QkOTSetUSHORTBit<4>::value;
			static const Qk_OT_USHORT BoldMask = QkOTSetUSHORTBit<5>::value;
			static const Qk_OT_USHORT RegularMask = QkOTSetUSHORTBit<6>::value;
			static const Qk_OT_USHORT UseTypoMetricsMask = QkOTSetUSHORTBit<7>::value;
			static const Qk_OT_USHORT WWSMask = QkOTSetUSHORTBit<8>::value;
			static const Qk_OT_USHORT ObliqueMask = QkOTSetUSHORTBit<9>::value;
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


static_assert(sizeof(QkOTTableOS2_V4) == 96, "sizeof_QkOTTableOS2_V4_not_96");

#endif
