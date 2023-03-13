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

#include <quark/util/loop.h>

using namespace qk;

static void message_cb(Cb::Data& ev, RunLoop* loop) {
	static int i = 0;
	Qk_LOG("message_cb, %d", i++);
}

void test_loop(int argc, char **argv) {
	RunLoop* loop = RunLoop::current();
	KeepLoop* keep = loop->keep_alive("test_loop");
	thread_fork([&](Thread *e) {
		for ( int i = 0; i < 5; i++) {
			thread_sleep(1e6);
			loop->post(Cb(message_cb, loop));
		}
		delete keep;
		return 0;
	}, "test");
	
	loop->run(10e6);
	
	int id = loop->work(Cb([&](Cb::Data& e){
		for (int i = 0; i < 5; i++) {
			thread_sleep(1e6);
			Qk_LOG("Exec work");
			loop->post(Cb(message_cb, loop));
		}
	}), Cb([](Cb::Data& e){
		Qk_LOG("Done");
	}));
	
	loop->run();
	
	Qk_LOG("Loop ok");
}
