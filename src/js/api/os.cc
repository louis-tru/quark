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

	struct NativeOs {
		static void binding(JSObject* exports, Worker* worker) {
			Js_Set_Method(version, {
				Js_Return( os_version() );
			});
			Js_Set_Method(brand, {
				Js_Return( os_brand() );
			});
			Js_Set_Method(model, {
				Js_Return( os_model() );
			});
			Js_Set_Method(info, {
				Js_Return( os_info() );
			});
			Js_Set_Method(languages, {
				Js_Return( os_languages() );
			});
			Js_Set_Method(isWifi, {
				Js_Return( os_is_wifi() );
			});
			Js_Set_Method(isMobile, {
				Js_Return( os_is_mobile() );
			});
			Js_Set_Method(networkInterface, {
				Js_Return( os_network_interface() );
			});
			Js_Set_Method(isAcPower, {
				Js_Return( os_is_ac_power() );
			});
			Js_Set_Method(isBattery, {
				Js_Return( os_is_battery() );
			});
			Js_Set_Method(batteryLevel, {
				Js_Return( os_battery_level() );
			});
			Js_Set_Method(memory, {
				Js_Return( os_memory() );
			});
			Js_Set_Method(usedMemory, {
				Js_Return( os_used_memory() );
			});
			Js_Set_Method(availableMemory, {
				Js_Return( os_available_memory() );
			});
			Js_Set_Method(cpuUsage, {
				Js_Return( os_cpu_usage() );
			});
		}
	};

	Js_Set_Module(_os, NativeOs)
}}