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

#include "./util.h"
#include "../version.h"
#include <vector>

#if Qk_WIN
# include <windows.h>
#endif

#if Qk_POSIX
# include <sys/utsname.h>
# include <unistd.h>
# include <time.h>
# include <stdlib.h>
#endif

#if Qk_APPLE
# include <mach/mach_time.h>
# include <mach/mach.h>
# include <mach/clock.h>
# include <dlfcn.h>

# define clock_gettime clock_gettime2

static clock_serv_t get_clock_port(clock_id_t clock_id) {
	clock_serv_t clock_r;
	host_get_clock_service(mach_host_self(), clock_id, &clock_r);
	return clock_r;
}

int clock_gettime2(clockid_t id, struct timespec *tspec) {
	typedef int (*clock_gettime_fn)(clockid_t, struct timespec*);
	static clock_gettime_fn real_clock_gettime = NULL;
	if (!real_clock_gettime) {
		real_clock_gettime = (clock_gettime_fn)dlsym(RTLD_DEFAULT, "clock_gettime");
	}

	if (real_clock_gettime) {
		return real_clock_gettime(id, tspec);
	} else {
		static clock_serv_t clock_realtime = get_clock_port(REALTIME_CLOCK);
		static clock_serv_t clock_monotonic = get_clock_port(SYSTEM_CLOCK);
		mach_timespec_t mts;
		int retval = 0;
		switch (id) {
			case CLOCK_MONOTONIC:
				retval = clock_get_time(clock_monotonic, &mts);
				break;
			case CLOCK_REALTIME:
				retval = clock_get_time(clock_realtime, &mts);
				break;
			default:
				/* only CLOCK_MONOTOIC and CLOCK_REALTIME clocks supported */
				return -1;
		}
		tspec->tv_sec = mts.tv_sec;
		tspec->tv_nsec = mts.tv_nsec;
		return retval;
	}
}

#endif

namespace qk {

	int random(uint32_t start, uint32_t end) {
		static uint32_t id;
		srand(uint32_t(time(NULL) + id));
		id = rand();
		return (id % (end - start + 1)) + start;
	}

	int fix_random(uint32_t a, ...) {
		int i = 0;
		int total = a;
		va_list ap;
		va_start(ap, a);
		while (1) {
			uint32_t e = va_arg(ap, uint32_t);
			if (e < 1) {
				break;
			}
			total += e;
		}
		//
		va_start(ap, a);
		int r = random(0, total - 1);
		total = a;
		if (r >= total) {
			while (1) {
				i++;
				uint32_t e = va_arg(ap, uint32_t);
				if (e < 1) {
					break;
				}
				total += e;
				if (r < total) {
					break;
				}
			}
		}
		va_end(ap);
		return i;
	}

	static std::atomic<uint64_t> id(10);

	uint64_t getId() {
		return id++;
	}

	uint32_t getId32() {
		return (id++) % Uint32::limit_max;
	}

	String version() {
		static String ver(Qk_VERSION);
		return ver;
	}

	String platform() {
		#if  Qk_iOS || Qk_MacOS
			static String _name("darwin");
		#elif  Qk_ANDROID
			static String _name("android");
		#elif  Qk_LINUX
			static String _name("linux");
		#elif  Qk_WIN
			static String _name("win32");
		#else
			# error no support
		#endif
		return _name;
	}

	int64_t time_second() {
		return ::time(nullptr);
	}

	int64_t time_micro() {
#if Qk_WIN // Windows
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		int64_t t = ((int64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
		// Windows epoch is 1601-01-01, Unix epoch is 1970-01-01
		return (t - 116444736000000000LL) / 10; // to microseconds
#else // Linux / Unix
		timespec now;
		int rc = clock_gettime(CLOCK_REALTIME, &now);
		if (rc != 0)
			return -1;
		int64_t r = int64_t(now.tv_sec) * 1e6 + now.tv_nsec / 1000;
		return r;
#endif
	}

	int64_t time_monotonic() {
#if Qk_WIN // Windows
		static LARGE_INTEGER freq;
		static int init = 0;
		if (!init) {
			QueryPerformanceFrequency(&freq);
			init = 1;
		}
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return (int64_t)(counter.QuadPart * 1000 / freq.QuadPart);
#elif defined(Qk_APPLE) // Apple
		static mach_timebase_info_data_t info;
		if (info.denom == 0) {
			mach_timebase_info(&info);
		}
		int64_t t = mach_absolute_time();
		int64_t ns = t * info.numer / info.denom;
		return ns / 1e3;
#else // Linux / Unix
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);
		return int64_t(ts.tv_sec) * 1e6 + ts.tv_nsec / 1e3;
#endif
	}
}
