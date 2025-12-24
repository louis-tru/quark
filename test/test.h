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

#ifndef __qk__test__
#define __qk__test__

#include <string.h>
#include <stdlib.h>

typedef void (*TestAssert)(const char* tag, bool cond, const char* msg, ...);
typedef void (*TestFunc)(int argc, char **argv, const char* func, TestAssert assert);

void __test_fail(const char* expr, const char* file, int line);
void __test_pass(const char* expr);

#define Qk_TEST_Func(n) \
	void test_##n(int argc, char **argv, const char* func, TestAssert assert)

#define Qk_TEST_EXPECT(expr) \
	do { if (!(expr)) __test_fail(#expr, __FILE__, __LINE__); \
				else __test_pass(#expr); } while(0)

#define Qk_TEST_EQ(a, b) \
	do { auto _A = (a); auto _B = (b); \
				if (_A != _B) __test_fail(#a " == " #b, __FILE__, __LINE__); \
				else __test_pass(#a " == " #b); } while(0)

#define Qk_TEST_STREQ(a,b) \
	do { if (strcmp((a),(b)) != 0) __test_fail(#a " == " #b, __FILE__, __LINE__); \
				else __test_pass(#a " == " #b); } while(0)

#endif
