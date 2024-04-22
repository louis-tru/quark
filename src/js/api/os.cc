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
 * @ns qk::js
 */

Js_BEGIN

class NativeOs {
	public:

	static void time(FunctionCall args) {
		Js_Worker(args);
		Js_Return( uint64(sys::time() / 1000) );
	}
	static void time_monotonic(FunctionCall args) {
		Js_Worker(args);
		Js_Return( uint64(sys::time_monotonic() / 1000) );
	}
	static void name(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::name() );
	}
	static void info(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::info() );
	}
	static void version(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::version() );
	}
	static void brand(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::brand() );
	}
	static void subsystem(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::subsystem() );
	}
	static void language(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::language() );
	}
	static void is_wifi(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::is_wifi() );
	}
	static void is_mobile(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::is_mobile() );
	}
	static void network_interface(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::network_status() );
	}
	static void is_ac_power(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::is_ac_power() );
	}
	static void is_battery(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::is_battery() );
	}
	static void battery_level(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::battery_level() );
	}
	static void memory(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::memory() );
	}
	static void used_memory(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::used_memory() );
	}
	static void available_memory(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::available_memory() );
	}
	static void cpu_usage(FunctionCall args) {
		Js_Worker(args);
		Js_Return( sys::cpu_usage() );
	}
	
	static void binding(Local<JSObject> exports, Worker* worker) {
		Js_Set_Method(time, time);
		Js_Set_Method(timeMonotonic, time_monotonic);
		Js_Set_Method(name, name);
		Js_Set_Method(info, info);
		Js_Set_Method(version, version);
		Js_Set_Method(brand, brand);
		Js_Set_Method(subsystem, subsystem);
		Js_Set_Method(language, language);
		Js_Set_Method(isWifi, is_wifi);
		Js_Set_Method(isMobile, is_mobile);
		Js_Set_Method(networkInterface, network_interface);
		Js_Set_Method(isACPower, is_ac_power);
		Js_Set_Method(isBattery, is_battery);
		Js_Set_Method(batteryLevel, battery_level);
		Js_Set_Method(memory, memory);
		Js_Set_Method(usedMemory, used_memory);
		Js_Set_Method(availableMemory, available_memory);
		Js_Set_Method(cpuUsage, cpu_usage);
	}
};

Js_REG_MODULE(_os, NativeOs)
Js_END
