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
#include <src/util/loop.h>
#include "../test.h"

using namespace qk;

volatile bool run_flag = false;
Mutex mtx;
RecursiveMutex rmtx;

void test_mutex0()
{
	int cnt = 0;
	while(run_flag)
	{
		ScopeLock scope(mtx);
		cnt++;
	}
	Qk_Log("std::mutex,%d", cnt);
}

void test_recursive_mutex()
{
	int cnt = 0;
	while(run_flag)
	{
		std::lock_guard<RecursiveMutex> scope(rmtx);
		cnt++;
	}
	Qk_Log("std::recursive_mutex,%d", cnt);
}

void test_idle()
{
	volatile int j = 0;
	int cnt = 0;
	while(run_flag)
	{
		j++;
		cnt++;
	}
	Qk_Log("idle,%d", cnt);
}

Qk_TEST_Func(mutex) {
	{
		run_flag = true;
		std::thread thr(test_mutex0);
		thread_sleep(1e6);
		run_flag = false;
		thr.join();
	}
	{
		run_flag = true;
		std::thread thr(test_recursive_mutex);
		thread_sleep(1e6);
		run_flag = false;
		thr.join();
	}
	{
		run_flag = true;
		std::thread thr(test_idle);
		thread_sleep(1e6);
		run_flag = false;
		thr.join();
	}
}
