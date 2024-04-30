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

#include "../js_.h"

namespace qk { namespace js {

	class WrapConsole {
	public:
		static void print_to(FunctionArgs args, void(*print)(cString&)) {
			Js_Worker(args);
			Array<String> rv;
			bool isSpace = false;

			for (int i = 0; i < args.length(); i++) {
				if (isSpace) {
					rv.push(String(' '));
				}
				if (args[i]->isObject()) {
					if (!stringifyConsoleStyled(worker, args[i], &rv)) {
						return;
					}
				} else {
					rv.push( args[i]->toStringValue(worker) );
				}
				isSpace = true;
			}
			print(rv.join(String()));
		}

		static void log(FunctionArgs args) {
			print_to(args, log_println);
		}

		static void warn(FunctionArgs args) {
			print_to(args, log_println_warn);
		}

		static void error(FunctionArgs args) {
			print_to(args, log_println_error);
		}

		static void clear(FunctionArgs args) {
			log_fflush();
		}

		static void debug(FunctionArgs args) {
			print_to(args, log_println);
		}

		static void info(FunctionArgs args) {
			print_to(args, log_println);
		}
		
		static void dir(FunctionArgs args) {}
		
		static void dirxml(FunctionArgs args) {}
		
		static void table(FunctionArgs args) {}
		
		static void trace(FunctionArgs args) {}
		
		static void group(FunctionArgs args) {}
		
		static void groupCollapsed(FunctionArgs args) {}
		
		static void groupEnd(FunctionArgs args) {}
		
		static void count(FunctionArgs args) {}
		
		static void Assert(FunctionArgs args) {}
		
		static void markTimeline(FunctionArgs args) {}
		
		static void profile(FunctionArgs args) {}
		
		static void profileEnd(FunctionArgs args) {}
		
		static void timeline(FunctionArgs args) {}
		
		static void timelineEnd(FunctionArgs args) {}
		
		static void time(FunctionArgs args) {}
		
		static void timeEnd(FunctionArgs args) {}
		
		static void timeStamp(FunctionArgs args) {}
		
		static void memory(JSValue* name, PropertyArgs args) {
			Js_Return( args.worker()->newNull() );
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Set_Method(log, log);
			Js_Set_Method(warn, warn);
			Js_Set_Method(error, error);
			Js_Set_Method(clear, clear);
			Js_Set_Method(debug, debug);
			Js_Set_Method(info, info);
			Js_Set_Method(dir, dir);
			Js_Set_Method(dirxml, dirxml);
			Js_Set_Method(table, table);
			Js_Set_Method(trace, trace);
			Js_Set_Method(group, group);
			Js_Set_Method(groupCollapsed, groupCollapsed);
			Js_Set_Method(groupEnd, groupEnd);
			Js_Set_Method(count, count);
			Js_Set_Method(assert, Assert);
			Js_Set_Method(markTimeline, markTimeline);
			Js_Set_Method(profile, profile);
			Js_Set_Method(profileEnd, profileEnd);
			Js_Set_Method(timeline, timeline);
			Js_Set_Method(timelineEnd, timelineEnd);
			Js_Set_Method(time, time);
			Js_Set_Method(timeEnd, timeEnd);
			Js_Set_Method(timeStamp, timeStamp);
			Js_Set_Accessor(memory, memory);
		}
	};

	Js_Set_Module(_console, WrapConsole)
} }
