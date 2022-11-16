/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef QkSFNTHeader_DEFINED
#define QkSFNTHeader_DEFINED

#include "../util.h"

//All Qk_SFNT_ prefixed types should be considered as big endian.
typedef uint16_t Qk_SFNT_USHORT;
typedef uint32_t Qk_SFNT_ULONG;

#pragma pack(push, 1)

struct QkSFNTHeader {
	Qk_SFNT_ULONG fontType;
	struct fontType_WindowsTrueType {
		static const Qk_OT_CHAR TAG0 = 0;
		static const Qk_OT_CHAR TAG1 = 1;
		static const Qk_OT_CHAR TAG2 = 0;
		static const Qk_OT_CHAR TAG3 = 0;
		static const Qk_OT_ULONG TAG = QkOTTableTAG<fontType_WindowsTrueType>::value;
	};
	struct fontType_MacTrueType {
		static const Qk_OT_CHAR TAG0 = 't';
		static const Qk_OT_CHAR TAG1 = 'r';
		static const Qk_OT_CHAR TAG2 = 'u';
		static const Qk_OT_CHAR TAG3 = 'e';
		static const Qk_OT_ULONG TAG = QkOTTableTAG<fontType_MacTrueType>::value;
	};
	struct fontType_PostScript {
		static const Qk_OT_CHAR TAG0 = 't';
		static const Qk_OT_CHAR TAG1 = 'y';
		static const Qk_OT_CHAR TAG2 = 'p';
		static const Qk_OT_CHAR TAG3 = '1';
		static const Qk_OT_ULONG TAG = QkOTTableTAG<fontType_PostScript>::value;
	};
	struct fontType_OpenTypeCFF {
		static const Qk_OT_CHAR TAG0 = 'O';
		static const Qk_OT_CHAR TAG1 = 'T';
		static const Qk_OT_CHAR TAG2 = 'T';
		static const Qk_OT_CHAR TAG3 = 'O';
		static const Qk_OT_ULONG TAG = QkOTTableTAG<fontType_OpenTypeCFF>::value;
	};

	Qk_SFNT_USHORT numTables;
	Qk_SFNT_USHORT searchRange;
	Qk_SFNT_USHORT entrySelector;
	Qk_SFNT_USHORT rangeShift;

	struct TableDirectoryEntry {
		Qk_SFNT_ULONG tag;
		Qk_SFNT_ULONG checksum;
		Qk_SFNT_ULONG offset; //From beginning of header.
		Qk_SFNT_ULONG logicalLength;
	}; //tableDirectoryEntries[numTables]
};

#pragma pack(pop)


static_assert(sizeof(QkSFNTHeader) == 12, "sizeof_SkSFNTHeader_not_12");
static_assert(sizeof(QkSFNTHeader::TableDirectoryEntry) == 16, "sizeof_SkSFNTHeader_TableDirectoryEntry_not_16");

#endif
