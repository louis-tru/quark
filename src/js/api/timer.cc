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

#include "./cb.h"

namespace qk { namespace js {

	class WrapTimer {
	public:
		static void timer_(FunctionArgs args, int64_t repeat) {
			Js_Worker(args);
			if (args.length() < 2 || !args[0]->isFunction() || !args[1]->isUint32()) {
				Js_Throw(
					"* @method setTimer(cb,time[,repeat])\n"
					"* @param cb {Function}\n"
					"* @param time {Number}\n"
					"* @param [repeat] {Number}\n"
					"* @return {Number}\n"
				);
			}
			uint64_t timeout = args[1]->toUint32Value(worker) * 1e3;
			if (repeat == -1) {
				repeat = 0;
				if (args.length() > 2 && args[2]->isUint32()) {
					repeat = args[2]->toUint32Value(worker);
				}
			}
			auto id = first_loop()->timer(get_callback_for_none(worker, args[0]), timeout, repeat);
			Js_Return(id);
		}

		static void clear_timer(FunctionArgs args) {
			Js_Worker(args);
			if (!args.length() || !args[0]->isUint32()) {
				Js_Throw(
					"* @method clearTimer(id)\n"
					"* @param id {Number}\n"
				);
			}
			first_loop()->timer_stop(args[0]->toUint32Value(worker));
		}

		static void next_tick(FunctionArgs args) {
			Js_Worker(args);
			if (!args.length() || !args[0]->isFunction()) {
				Js_Throw(
					"* @method nextTick(cb,time[,repeat])\n"
					"* @param cb {Function}\n"
				);
			}
			first_loop()->next_tick(get_callback_for_none(worker, args[0]));
		}

		static void set_timer(FunctionArgs args) {
			timer_(args, -1);
		}

		static void setTimeout(FunctionArgs args) {
			timer_(args, 0);
		}

		static void setInterval(FunctionArgs args) {
			timer_(args, 0xffffffffffffffu);
		}

		static void binding(JSObject* exports, Worker* worker) {
			Js_Set_Method(setTimer, set_timer);
			Js_Set_Method(nextTick, next_tick);
			Js_Set_Method(clearTimer, clear_timer);
			Js_Set_Method(setTimeout, setTimeout);
			Js_Set_Method(setInterval, setInterval);
			Js_Set_Method(clearTimeout, clear_timer);
			Js_Set_Method(clearInterval, clear_timer);
		}
	};

	Js_Set_Module(_timer, WrapTimer)
} }
