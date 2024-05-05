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

	class NativeConsole {
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

		static void binding(JSObject* exports, Worker* worker) {
			Js_Set_Method(log, {
				print_to(args, log_println);
			});

			Js_Set_Method(warn, {
				print_to(args, log_println_warn);
			});

			Js_Set_Method(error, {
				print_to(args, log_println_error);
			});

			Js_Set_Method(clear, {
				log_fflush();
			});

			Js_Set_Method(debug, {
				print_to(args, log_println);
			});

			Js_Set_Method(info, {
				print_to(args, log_println);
			});

			Js_Set_Method(dir, {});
			Js_Set_Method(dirxml, {});
			Js_Set_Method(table, {});
			Js_Set_Method(trace, {});
			Js_Set_Method(group, {});
			Js_Set_Method(groupCollapsed, {});
			Js_Set_Method(groupEnd, {});
			Js_Set_Method(count, {});
			Js_Set_Method(Assert, {});
			Js_Set_Method(markTimeline, {});
			Js_Set_Method(profile, {});
			Js_Set_Method(profileEnd, {});
			Js_Set_Method(timeline, {});
			Js_Set_Method(timelineEnd, {});
			Js_Set_Method(time, {});
			Js_Set_Method(timeEnd, {});
			Js_Set_Method(timeStamp, {});

			Js_Set_Accessor_Get(memory, {
				Js_Return( args.worker()->newNull() );
			});
		}
	};

	Js_Set_Module(_console, NativeConsole)
} }
