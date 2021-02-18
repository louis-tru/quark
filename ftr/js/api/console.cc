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

#include "./_json.h"
#include "../js.h"
// #include "ftr/util/string-builder.h"

/**
 * @ns ftr::js
 */

JS_BEGIN

static cString Space(' ');

/**
 * @class NativeConsole
 */
class NativeConsole {
	public:

	static void print_to(FunctionCall args, void(*func)(cString&)) {
		JS_WORKER(args);
		StringBuilder rv;
		bool is_space = false;
		
		for (int i = 0; i < args.Length(); i++) {
			if (is_space) {
				rv.push(Space);
			}
			if (args[i]->IsObject(worker)) {
				if (!JSON::stringify_console_styled(worker, args[i], &rv)) {
					return;
				}
			} else {
				rv.push( args[i]->ToStringValue(worker) );
			}
			is_space = true;
		}
		func(rv.to_string());
	}
	
	static void log(FunctionCall args) {
		print_to(args, console::log);
	}
	
	static void warn(FunctionCall args) {
		print_to(args, console::warn);
	}
	
	static void error(FunctionCall args) {
		print_to(args, console::error);
	}
	
	static void clear(FunctionCall args) {
		console::clear();
	}
	
	static void debug(FunctionCall args) {
		print_to(args, console::log);
	}
	
	static void info(FunctionCall args) {
		print_to(args, console::log);
	}
	
	static void dir(FunctionCall args) {}
	
	static void dirxml(FunctionCall args) {}
	
	static void table(FunctionCall args) {}
	
	static void trace(FunctionCall args) {}
	
	static void group(FunctionCall args) {}
	
	static void groupCollapsed(FunctionCall args) {}
	
	static void groupEnd(FunctionCall args) {}
	
	static void count(FunctionCall args) {}
	
	static void Assert(FunctionCall args) {}
	
	static void markTimeline(FunctionCall args) {}
	
	static void profile(FunctionCall args) {}
	
	static void profileEnd(FunctionCall args) {}
	
	static void timeline(FunctionCall args) {}
	
	static void timelineEnd(FunctionCall args) {}
	
	static void time(FunctionCall args) {}
	
	static void timeEnd(FunctionCall args) {}
	
	static void timeStamp(FunctionCall args) {}
	
	static void memory(Local<JSString> name, PropertyCall args) {
		JS_WORKER(args);
		JS_RETURN( worker->NewNull() );
	}

	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_SET_METHOD(log, log);
		JS_SET_METHOD(warn, warn);
		JS_SET_METHOD(error, error);
		JS_SET_METHOD(clear, clear);
		JS_SET_METHOD(debug, debug);
		JS_SET_METHOD(info, info);
		JS_SET_METHOD(dir, dir);
		JS_SET_METHOD(dirxml, dirxml);
		JS_SET_METHOD(table, table);
		JS_SET_METHOD(trace, trace);
		JS_SET_METHOD(group, group);
		JS_SET_METHOD(groupCollapsed, groupCollapsed);
		JS_SET_METHOD(groupEnd, groupEnd);
		JS_SET_METHOD(count, count);
		JS_SET_METHOD(assert, Assert);
		JS_SET_METHOD(markTimeline, markTimeline);
		JS_SET_METHOD(profile, profile);
		JS_SET_METHOD(profileEnd, profileEnd);
		JS_SET_METHOD(timeline, timeline);
		JS_SET_METHOD(timelineEnd, timelineEnd);
		JS_SET_METHOD(time, time);
		JS_SET_METHOD(timeEnd, timeEnd);
		JS_SET_METHOD(timeStamp, timeStamp);
		JS_SET_ACCESSOR(memory, memory);
	}
};

JS_REG_MODULE(_console, NativeConsole)
JS_END
