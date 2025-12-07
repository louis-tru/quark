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

#include <src/util/util.h>
#include <src/util/util.h>
#include <src/ui/geometry.h>
#include <math.h>
#include "./test.h"

using namespace qk;

static Array<Vec2> mk(const std::initializer_list<Vec2> &pts) {
	return Array<Vec2>(pts);
}

static void check(bool cond, const char* info) {
	if (!cond) {
		printf("❌ FAIL: %s\n", info);
		Qk_ASSERT(false);
	} else {
		printf("✅ PASS: %s\n", info);
	}
}

Qk_TEST_Func(mtv) {
	// 0.
	{
		auto A = mk({{218.641693, 1747.281006},{212.783829, 1761.423096},{198.641693, 1767.281006},{184.499557, 1761.423096},
			{178.641693, 1747.281006},{184.499557, 1733.138916},{198.641693, 1727.281006},{212.783829, 1733.138916}});
		auto B = mk({{895, 120},{1025, 120},{1025, 300},{895, 300}});
		MTV mt;
		bool hit = test_polygon_vs_polygon(A,B,&mt,true);
		check(hit == false, "far: no collision");
		check(mt.overlap > 100, "far: min distance > 100");
	}

// 1. 远离测试
	{
		auto A = mk({{0,0},{2,0},{2,2},{0,2}});
		auto B = mk({{10,0},{12,0},{12,2},{10,2}});
		MTV mt;
		bool hit = test_polygon_vs_polygon(A,B,&mt,true);
		check(hit == false, "far: no collision");
		check(fabs(mt.overlap - 8.0f) < 1e-4, "far: min distance = 8");
	}

	// 2. 重叠测试
	{
		auto A = mk({{0,0},{4,0},{4,4},{0,4}});
		auto B = mk({{2,2},{6,2},{6,6},{2,6}});
		MTV mt;
		bool hit = test_polygon_vs_polygon(A,B,&mt,true);
		check(hit == true, "overlap: collision");
		check(fabs(mt.overlap - 2.0f) < 1e-4, "overlap: penetration = 2");
	}

	// 3. 接触测试（touching）
	{
		auto A = mk({{0,0},{2,0},{2,2},{0,2}});
		auto B = mk({{2,0},{4,0},{4,2},{2,2}});
		MTV mt;
		bool hit = test_polygon_vs_polygon(A,B,&mt,true);
		check(hit == true, "touch: treated as collision");
		check(fabs(mt.overlap - 0.0f) < 1e-4, "touch: depth=0");
	}

	// 4. 三角形 vs 正方形（不相交）
	{
		auto A = mk({{0,0},{3,0},{0,3}});
		auto B = mk({{5,1},{7,1},{7,3},{5,3}});
		MTV mt;
		bool hit = test_polygon_vs_polygon(A,B,&mt,true);
		check(hit == false, "triangle vs square: no hit");
		const float expected = sqrtf(5.0f);  // ≈ 2.2360679
		check(fabs(mt.overlap - expected) < 1e-4, "triangle vs square: distance ≈ sqrt(5)");
	}

	// 6. requestSeparationMTV=false
	{
		auto A = mk({{0,0},{2,0},{2,2},{0,2}});
		auto B = mk({{5,0},{7,0},{7,2},{5,2}});
		bool hit = test_polygon_vs_polygon(A,B,nullptr,false);
		check(hit == false, "requestSeparationMTV=false no collision");
	}

	printf("\n🎉 ALL TESTS DONE!\n");
}
