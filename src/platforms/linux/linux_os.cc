/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "../../os/os.h"
#include "../../util/string.h"
#include "../../util/fs.h"
#include <unistd.h>
#include <sys/utsname.h>

namespace qk {

	// struct utsname {
	// 	char sysname[];   // Linux
	// 	char nodename[];  // hostname
	// 	char release[];   // 6.6.12-arch1-1
	// 	char version[];   // #1 SMP PREEMPT_DYNAMIC Thu, 30 Nov 2023 10:12:34 +0000
	// 	char machine[];   // x86_64 / aarch64
	// };
	static utsname* _device_uname() {
		static utsname* utsn = nullptr;
		if (!utsn) {
			utsn = new utsname;
			uname(utsn);
		}
		return utsn;
	}

	String os_name() {
		return _device_uname()->sysname;
	}

	String os_version() {
		return _device_uname()->release;
	}

	String os_brand() {
		static String brand([]() {
			String str(fs_read_file_sync("/etc/os-release", 16));
			auto idx = str.indexOf('\n');
			if (idx == -1)
				return String();
			return str.substring(6, idx - 1);
		}());
		return brand;
	}

	String os_model() {
		// return _device_uname()->version;
		return String(); // not supported
	}

	int os_network_interface() {
		return 1;
	}

	bool os_is_ac_power() {
		return true;
	}

	bool os_is_battery() {
		return false;
	}

	float os_battery_level() {
		return false;
	}

	struct memory_info_t {
		size_t MemTotal;
		size_t MemFree;
		size_t MemAvailable;
	};

	memory_info_t device_get_memory_info() {
		memory_info_t r = {0,0,0};

		String s = fs_read_file_sync("/proc/meminfo", 127).collapseString();
		Qk_DLog("/proc/meminfo, %s", *s);

		if (!s.isEmpty()) {
			int i, j;

			i = s.indexOf("MemTotal:");
			if (i == -1) return r;
			j = s.indexOf("kB", i);
			if (j == -1) return r;

			r.MemTotal = s.substring(i + 9, j).trim().toNumber<uint64_t>() * 1024;
			Qk_DLog("MemTotal, %lu", r.MemTotal);

			i = s.indexOf("MemFree:", j);
			if (i == -1) return r;
			j = s.indexOf("kB", i);
			if (j == -1) return r;

			r.MemFree = s.substring(i + 8, j).trim().toNumber<uint64_t>() * 1024;
			Qk_DLog("MemFree, %lu", r.MemFree);

			i = s.indexOf("MemAvailable:", j);
			if (i == -1) return r;
			j = s.indexOf("kB", i);
			if (j == -1) return r;

			r.MemAvailable = s.substring(i + 13, j).trim().toNumber<uint64_t>() * 1024;
			Qk_DLog("MemAvailable, %lu", r.MemAvailable);
		}
		return r;
	}

	uint64_t os_memory() {
		return device_get_memory_info().MemTotal;
	}

	uint64_t os_used_memory() {
		memory_info_t info = device_get_memory_info();
		return int64_t(info.MemTotal) - info.MemAvailable;
	}

	uint64_t os_available_memory() {
		return device_get_memory_info().MemAvailable;
	}
}
