/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
// @private head

#ifndef QkOTTable_OS_2_V0_DEFINED
#define QkOTTable_OS_2_V0_DEFINED

#include "QkIBMFamilyClass.h"
#include "QkPanose.h"

#pragma pack(push, 1)

struct QkOTTableOS2_V0 {
	Qk_OT_USHORT version;
	//QkOTTableOS2_VA::VERSION and QkOTTableOS2_V0::VERSION are both 0.
	//The only way to differentiate these two versions is by the size of the table.
	static const Qk_OT_USHORT VERSION = QkEndian_SwapBE16(0);

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
	Qk_OT_ULONG ulCharRange[4];
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
};

#pragma pack(pop)


static_assert(sizeof(QkOTTableOS2_V0) == 78, "sizeof_QkOTTableOS2_V0_not_78");

#endif
