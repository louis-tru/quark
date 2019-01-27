/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <shark/utils/util.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>

using namespace shark;

class Foo {
 public:
	Foo()
	: flag_(0)
	, thread1_(std::bind(&Foo::threadFunc1, this))
	, thread2_(std::bind(&Foo::threadFunc2, this))
	{
	}
	
	~Foo()
	{
		thread1_.join();
		thread2_.join();
	}
	
 private:
	void threadFunc1()
	{
		std::unique_lock<std::mutex> ul(mutex_);
		while (0 == flag_) {
			cond_.wait(ul);
		}
		LOG("%d", flag_);
	}
	
	void threadFunc2()
	{
		// 为了测试，等待3秒
		std::this_thread::sleep_for((std::chrono::milliseconds(3000)));
		std::unique_lock<std::mutex> ul(mutex_);
		flag_ = 100;
		cond_.notify_one();
	}
	
	int flag_;
	std::mutex mutex_;
	std::condition_variable cond_;
	std::thread thread1_;
	std::thread thread2_;
};

std::mutex m1;
int i = 0;
std::atomic_int atomic_lock(false);

void test_for() {
	
	int j = 0;
	
	while (1) {
		
		std::lock_guard<std::mutex> lock(m1);
		
		if (i < 10000000) {
			i++;
			j++;
		} else {
			break;
		}
	
	}
	
	m1.lock();
	LOG("result: %d", j);
	m1.unlock();
	
}

void test_thread(int argc, char **argv) {
	
	std::thread::id id = std::this_thread::get_id();
	{
		Foo f;
	}
	
	std::thread g_a(test_for);
	std::thread g_b(test_for);
	
	g_a.get_id();
	
	g_a.join();
	g_b.join();
	
	LOG("done");
	
}
