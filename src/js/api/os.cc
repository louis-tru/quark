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

#include "../../util/os.h"
#include "../_js.h"

/**
 * @ns quark::js
 */

JS_BEGIN

class NativeOs {
	public:

	static void time(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( uint64(sys::time() / 1000) );
	}
	static void time_monotonic(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( uint64(sys::time_monotonic() / 1000) );
	}
	static void name(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::name() );
	}
	static void info(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::info() );
	}
	static void version(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::version() );
	}
	static void brand(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::brand() );
	}
	static void subsystem(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::subsystem() );
	}
	static void language(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::language() );
	}
	static void is_wifi(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::is_wifi() );
	}
	static void is_mobile(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::is_mobile() );
	}
	static void network_interface(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::network_status() );
	}
	static void is_ac_power(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::is_ac_power() );
	}
	static void is_battery(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::is_battery() );
	}
	static void battery_level(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::battery_level() );
	}
	static void memory(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::memory() );
	}
	static void used_memory(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::used_memory() );
	}
	static void available_memory(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::available_memory() );
	}
	static void cpu_usage(FunctionCall args) {
		JS_WORKER(args);
		JS_RETURN( sys::cpu_usage() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		JS_SET_METHOD(time, time);
		JS_SET_METHOD(timeMonotonic, time_monotonic);
		JS_SET_METHOD(name, name);
		JS_SET_METHOD(info, info);
		JS_SET_METHOD(version, version);
		JS_SET_METHOD(brand, brand);
		JS_SET_METHOD(subsystem, subsystem);
		JS_SET_METHOD(language, language);
		JS_SET_METHOD(isWifi, is_wifi);
		JS_SET_METHOD(isMobile, is_mobile);
		JS_SET_METHOD(networkInterface, network_interface);
		JS_SET_METHOD(isACPower, is_ac_power);
		JS_SET_METHOD(isBattery, is_battery);
		JS_SET_METHOD(batteryLevel, battery_level);
		JS_SET_METHOD(memory, memory);
		JS_SET_METHOD(usedMemory, used_memory);
		JS_SET_METHOD(availableMemory, available_memory);
		JS_SET_METHOD(cpuUsage, cpu_usage);
	}
};

JS_REG_MODULE(_os, NativeOs)
JS_END
