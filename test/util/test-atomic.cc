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

#include <src/util/loop.h>

using namespace qk;

class Ptr: public SafeFlag {
private:
	int i = 0;
};

struct TestAtomic0 {
	char i[32];
};

class TestAtomic {
public:
	TestAtomic() {
		_ptr0 = new Ptr();
		_ptr1 = new Ptr();
		_ptr2 = new Ptr();
	}
	~TestAtomic() {
		delete _ptr0; _ptr0 = nullptr;
		delete _ptr1; _ptr1 = nullptr;
		delete _ptr2; _ptr2 = nullptr;
	}
	inline Ptr* getPtr0() {
		return _ptr0;
	}
	inline Ptr* getPtr1() {
		return _ptr1->isValid() ? _ptr1: nullptr;
	}
	inline Ptr* getPtr2() {
		return _ptr2.load();
		//return *reinterpret_cast<Ptr**>(&_ptr2);
	}
	mutable Ptr *_ptr0;
	mutable Ptr *_ptr1;
	mutable std::atomic<Ptr*> _ptr2;
};

void test_atomic(int argc, char **argv) {

	std::atomic<uint32_t> a;
	std::atomic<uint64_t> b;
	std::atomic<void*> c;
	std::atomic<TestAtomic0> d;

	Qk_DLog("atomic_uint32_t, %d: %i, %d: %i, %d: %i, %d: %i",
		sizeof(a), a.is_lock_free(),
		sizeof(b), b.is_lock_free(),
		sizeof(c), c.is_lock_free(),
		sizeof(d), d.is_lock_free()
	);

	TestAtomic test;

	int64_t s1 = time_monotonic();

	for (int i = 0; i < 100000; i++) {
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
		test.getPtr0();
	}

	int64_t s2 = time_monotonic();

	for (int i = 0; i < 100000; i++) {
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
		test.getPtr1();
	}

	int64_t s3 = time_monotonic();

	for (int i = 0; i < 100000; i++) {
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
		test.getPtr2();
	}

	int64_t s4 = time_monotonic();

	Qk_Log("assign: %ld ms", (s2 - s1));
	Qk_Log("assign: %ld ms", (s3 - s2));
	Qk_Log("assign: %ld ms", (s4 - s3));
}
