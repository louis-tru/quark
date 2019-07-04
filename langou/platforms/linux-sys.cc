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

#include "langou/sys.h"
#include "langou/utils/string.h"
#include "langou/utils/array.h"
#include "langou/utils/fs.h"
#include "langou/utils/buffer.h"
#include <unistd.h>

XX_NS(langou)
XX_NS(sys)

int network_status() {
	return 1;
}

bool is_ac_power() {
	return 1;
}

bool is_battery() {
	return 0;
}

float battery_level() {
	return 0;
}

struct memory_info_t {
	size_t MemTotal;
	size_t MemFree;
	size_t MemAvailable;
};

memory_info_t get_memory_info() {
	memory_info_t r = {0,0,0};

	String s = FileHelper::read_file_sync("/proc/meminfo", 127).collapse_string();
	DLOG("/proc/meminfo, %s", *s);

	if (!s.is_empty()) {
		int i, j;

		i = s.index_of("MemTotal:");
		if (i == -1) return r;
		j = s.index_of("kB", i);
		if (j == -1) return r;

		r.MemTotal = s.substring(i + 9, j).trim().to_uint64() * 1024;
		DLOG("MemTotal, %lu", r.MemTotal);

		i = s.index_of("MemFree:", j);
		if (i == -1) return r;
		j = s.index_of("kB", i);
		if (j == -1) return r;

		r.MemFree = s.substring(i + 8, j).trim().to_uint64() * 1024;
		DLOG("MemFree, %lu", r.MemFree);

		i = s.index_of("MemAvailable:", j);
		if (i == -1) return r;
		j = s.index_of("kB", i);
		if (j == -1) return r;

		r.MemAvailable = s.substring(i + 13, j).trim().to_uint64() * 1024;
		DLOG("MemAvailable, %lu", r.MemAvailable);
	}
	return r;
}

uint64 memory() {
	return get_memory_info().MemTotal;
}

uint64 used_memory() {
	memory_info_t info = get_memory_info();
	return int64(info.MemTotal) - info.MemAvailable;
}

uint64 available_memory() {
	return get_memory_info().MemAvailable;
}

XX_END XX_END
