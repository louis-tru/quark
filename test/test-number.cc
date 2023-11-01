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

#include <quark/util/util.h>
#include <quark/util/loop.h>
#include <quark/util/array.h>
#include <quark/os/info.h>

using namespace qk;

#define Operation_DEF(name, symbol, T) {\
	T a(random(0, 99999999)); \
	T b(random(0, 99999999));\
	T c(random(0, 99999999));\
	T d(random(0, 99999999));\
	T e(random(0, 99999999));\
	T f(random(0, 99999999));\
	T g(random(0, 99999999));\
	T h(random(0, 99999999));\
	T i(random(0, 99999999));\
	T j(random(0, 99999999));\
	uint64_t s1 = time_monotonic(); \
	for ( int o = 0; o < 100000; o++ ) { \
		T k = 0; \
		k symbol##= a; \
		k symbol##= b; \
		k symbol##= c; \
		k symbol##= d; \
		k symbol##= e; \
		k symbol##= f; \
		k symbol##= g; \
		k symbol##= h; \
		k symbol##= i; \
		k symbol##= j; \
		k symbol##= a; \
		k symbol##= b; \
		k symbol##= c; \
		k symbol##= d; \
		k symbol##= e; \
		k symbol##= f; \
		k symbol##= g; \
		k symbol##= h; \
		k symbol##= i; \
		k symbol##= j; \
		k symbol##= a; \
		k symbol##= b; \
		k symbol##= c; \
		k symbol##= d; \
		k symbol##= e; \
		k symbol##= f; \
		k symbol##= g; \
		k symbol##= h; \
		k symbol##= i; \
		k symbol##= j; \
		k symbol##= a; \
		k symbol##= b; \
		k symbol##= c; \
		k symbol##= d; \
		k symbol##= e; \
		k symbol##= f; \
		k symbol##= g; \
		k symbol##= h; \
		k symbol##= i; \
		k symbol##= j; \
		k symbol##= a; \
		k symbol##= b; \
		k symbol##= c; \
		k symbol##= d; \
		k symbol##= e; \
		k symbol##= f; \
		k symbol##= g; \
		k symbol##= h; \
		k symbol##= i; \
		k symbol##= j; \
	} \
	uint64_t s2 = time_monotonic(); \
	Qk_LOG("%s: %llu", name #symbol, s2 - s1 - s0); \
}

uint64_t test_operation_assign() {
	int a(random(0, 99999999));
	int b(random(0, 99999999));
	int c(random(0, 99999999));
	int d(random(0, 99999999));
	int e(random(0, 99999999));
	int f(random(0, 99999999));
	int g(random(0, 99999999));
	int h(random(0, 99999999));
	int i(random(0, 99999999));
	int j(random(0, 99999999));

	uint64_t s1 = time_monotonic();

	for ( int o = 0; o < 100000; o++ ) {
		int k = 0;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
		k = a;
		b = k;
		c = b;
		d = c;
		c = e;
		f = e;
		g = f;
		f = h;
		i = f;
		j = k;
	}

	uint64_t s2 = time_monotonic();

	Qk_LOG("assign: %lld, %d", s2 - s1, j);

	return s2 - s1;
}

void test_number(int argc, char **argv) {
	double d = -500;

	char c = d;
	uint8_t b = d;

	Qk_LOG("%d", c);
	Qk_LOG("%d", *(char*)&b);

	uint64_t s0 = test_operation_assign();

	Operation_DEF("int16", +, int16_t);
	Operation_DEF("int32", +, int32_t);
	Operation_DEF("int64", +, int64_t);
	Operation_DEF("float", +, float);
	Operation_DEF("double", +, double);
	// Operation_DEF("AtomicInt", +, std::atomic_int);

	Operation_DEF("int16", *, int16_t);
	Operation_DEF("int32", *, int32_t);
	Operation_DEF("int64", *, int64_t);
	Operation_DEF("float", *, float);
	Operation_DEF("double", *, double);

	Operation_DEF("int16", /, int16_t);
	Operation_DEF("int32", /, int32_t);
	Operation_DEF("int64", /, int64_t);
	Operation_DEF("float", /, float);
	Operation_DEF("double", /, double);

	Operation_DEF("int16", %, int16_t);
	Operation_DEF("int32", %, int32_t);
	Operation_DEF("int64", %, int64_t);
	// Operation_DEF("float", /, float);
	// Operation_DEF("double", /, double);
}
