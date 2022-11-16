/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef QkOTTable_OS_2_VA_DEFINED
#define QkOTTable_OS_2_VA_DEFINED

#include "../util.h"
#include "QkIBMFamilyClass.h"
#include "QkPanose.h"

#pragma pack(push, 1)

//Original V0 TT
struct QkOTTableOS2_VA {
	Qk_OT_USHORT version;
	//QkOTTableOS2_VA::VERSION and QkOTTableOS2_V0::VERSION are both 0.
	//The only way to differentiate these two versions is by the size of the table.
	static const Qk_OT_USHORT VERSION = QkEndian_SwapBE16(0);

	Qk_OT_SHORT xAvgCharWidth;
	struct WeightClass {
		enum Value : Qk_OT_USHORT {
			UltraLight = QkEndian_SwapBE16(1),
			ExtraLight = QkEndian_SwapBE16(2),
			Light = QkEndian_SwapBE16(3),
			SemiLight = QkEndian_SwapBE16(4),
			Medium = QkEndian_SwapBE16(5),
			SemiBold = QkEndian_SwapBE16(6),
			Bold = QkEndian_SwapBE16(7),
			ExtraBold = QkEndian_SwapBE16(8),
			UltraBold = QkEndian_SwapBE16(9),
			Qk_SEQ_END,
		} value;
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
			Qk_SEQ_END,
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
				Reserved06,
				Reserved07)
		} field;
		struct Raw {
			static const Qk_OT_USHORT ItalicMask = QkOTSetUSHORTBit<0>::value;
			static const Qk_OT_USHORT UnderscoreMask = QkOTSetUSHORTBit<1>::value;
			static const Qk_OT_USHORT NegativeMask = QkOTSetUSHORTBit<2>::value;
			static const Qk_OT_USHORT OutlinedMask = QkOTSetUSHORTBit<3>::value;
			static const Qk_OT_USHORT StrikeoutMask = QkOTSetUSHORTBit<4>::value;
			static const Qk_OT_USHORT BoldMask = QkOTSetUSHORTBit<5>::value;
			Qk_OT_USHORT value;
		} raw;
	} fsSelection;
	Qk_OT_USHORT usFirstCharIndex;
	Qk_OT_USHORT usLastCharIndex;
};

#pragma pack(pop)


static_assert(sizeof(QkOTTableOS2_VA) == 68, "sizeof_QkOTTableOS2_VA_not_68");

#endif
