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
#include "../../os/os.h"

namespace qk { namespace js {

	class WrapOs {
	public:
		static void version(FunctionArgs args) {
			Js_Return( os_version() );
		}
		static void brand(FunctionArgs args) {
			Js_Return( os_brand() );
		}
		static void model(FunctionArgs args) {
			Js_Return( os_model() );
		}
		static void info(FunctionArgs args) {
			Js_Return( os_info() );
		}
		static void languages(FunctionArgs args) {
			Js_Return( os_languages() );
		}
		static void is_wifi(FunctionArgs args) {
			Js_Return( os_is_wifi() );
		}
		static void is_mobile(FunctionArgs args) {
			Js_Return( os_is_mobile() );
		}
		static void network_interface(FunctionArgs args) {
			Js_Return( os_network_status() );
		}
		static void is_ac_power(FunctionArgs args) {
			Js_Return( os_is_ac_power() );
		}
		static void is_battery(FunctionArgs args) {
			Js_Return( os_is_battery() );
		}
		static void battery_level(FunctionArgs args) {
			Js_Return( os_battery_level() );
		}
		static void memory(FunctionArgs args) {
			Js_Return( os_memory() );
		}
		static void used_memory(FunctionArgs args) {
			Js_Return( os_used_memory() );
		}
		static void available_memory(FunctionArgs args) {
			Js_Return( os_available_memory() );
		}
		static void cpu_usage(FunctionArgs args) {
			Js_Return( os_cpu_usage() );
		}
		static void binding(JSObject* exports, Worker* worker) {
			Js_Set_Method(version, version);
			Js_Set_Method(brand, brand);
			Js_Set_Method(model, model);
			Js_Set_Method(info, info);
			Js_Set_Method(languages, languages);
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

	Js_Set_Module(_os, WrapOs)
}}