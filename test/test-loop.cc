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

#include <ngui/utils/loop.h>

using namespace ngui;

static void message_cb(Se& ev, RunLoop* loop) {
	static int i = 0;
	LOG("message_cb, %d", i++);
}

void test_loop(int argc, char **argv) {
	RunLoop* loop = RunLoop::current();
	KeepLoop* keep = loop->keep_alive();
	SimpleThread::detach([&](SimpleThread& e) {
		for ( int i = 0; i < 5; i++) {
			e.sleep_for(1e6);
			loop->post(Cb(message_cb, loop));
		}
		delete keep;
	}, "test");
	
	loop->run(10e6);
	
	int id = loop->work(Cb([&](Se& e){
		for (int i = 0; i < 5; i++) {
			SimpleThread::sleep_for(1e6);
			LOG("Exec work");
			loop->post(Cb(message_cb, loop));
		}
	}), Cb([](Se& e){
		LOG("Done");
	}));
	
	loop->run();
	
	LOG("Loop ok");
}
