/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef QkOTTable_OS_2_DEFINED
#define QkOTTable_OS_2_DEFINED

#include "QkOTTable_OS_2_VA.h"
#include "QkOTTable_OS_2_V0.h"
#include "QkOTTable_OS_2_V1.h"
#include "QkOTTable_OS_2_V2.h"
#include "QkOTTable_OS_2_V3.h"
#include "QkOTTable_OS_2_V4.h"

#pragma pack(push, 1)

struct QkOTTableOS2 {
	static constexpr Qk_OT_CHAR TAG0 = 'O';
	static constexpr Qk_OT_CHAR TAG1 = 'S';
	static constexpr Qk_OT_CHAR TAG2 = '/';
	static constexpr Qk_OT_CHAR TAG3 = '2';
	static constexpr Qk_OT_ULONG TAG = QkOTTableTAG<QkOTTableOS2>::value;

	union Version {
		Qk_OT_USHORT version;
		//original V0 TT
		struct VA : QkOTTableOS2_VA { } vA;
		struct V0 : QkOTTableOS2_V0 { } v0;
		struct V1 : QkOTTableOS2_V1 { } v1;
		struct V2 : QkOTTableOS2_V2 {} v2;
		//makes fsType 0-3 exclusive
		struct V3 : QkOTTableOS2_V3 { } v3;
		// //defines fsSelection bits 7-9
		struct V4 : QkOTTableOS2_V4 { } v4;
	} version;
};

#pragma pack(pop)

static_assert(sizeof(QkOTTableOS2::Version::VA) == 68, "sizeof_QkOTTableOS2__VA_not_68");
static_assert(sizeof(QkOTTableOS2::Version::V0) == 78, "sizeof_QkOTTableOS2__V0_not_78");
static_assert(sizeof(QkOTTableOS2::Version::V1) == 86, "sizeof_QkOTTableOS2__V1_not_86");
static_assert(sizeof(QkOTTableOS2::Version::V2) == 96, "sizeof_QkOTTableOS2__V2_not_96");
static_assert(sizeof(QkOTTableOS2::Version::V3) == 96, "sizeof_QkOTTableOS2__V3_not_96");
static_assert(sizeof(QkOTTableOS2::Version::V4) == 96, "sizeof_QkOTTableOS2__V4_not_96");

#endif
