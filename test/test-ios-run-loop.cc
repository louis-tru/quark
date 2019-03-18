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

#include <qgr/env.h>

#if XX_IOS

#include <qgr/utils/util.h>
#include <CoreFoundation/CoreFoundation.h>
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <thread>

static void _perform(void *info __unused) {
	printf("hello2\n");
}

static void _timer(CFRunLoopTimerRef timer __unused, void *info) {
	printf("hello1\n");
	CFRunLoopSourceRef source = (CFRunLoopSourceRef)info;
	CFRunLoopSourceSignal(source);
}

int test_ios_run_loop(int argc, char **argv) {
	
	CFRunLoopSourceRef source;
	CFRunLoopSourceContext source_context;
	CFRunLoopTimerRef timer;
	CFRunLoopTimerContext timer_context;
	
	bzero(&source_context, sizeof(source_context));
	source_context.perform = _perform;
	source = CFRunLoopSourceCreate(NULL, 0, &source_context);
	//
	bzero(&timer_context, sizeof(timer_context));
	timer_context.info = source;
	timer = CFRunLoopTimerCreate(NULL, CFAbsoluteTimeGetCurrent(), 1, 0, 0, _timer, &timer_context);
	
	std::thread([&]() {
		CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopCommonModes);
		CFRunLoopRun();
	}).detach();
	
	
	CFRunLoopContainsSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
	CFRunLoopRun();

	return 0;
}

#endif
