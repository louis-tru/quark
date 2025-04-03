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

#include "./os.h"
#include <quark/util/fs.h>
#include <quark/util/loop.h>
#include <quark/util/dict.h>
#include <string.h>
#include <atomic>
#include <unistd.h>

#if Qk_POSIX
# include <sys/utsname.h>
#endif

#if Qk_ANDROID
# include "../platforms/android/android.h"
#endif

namespace qk {
	static Mutex mutex;
#if Qk_POSIX
	static String* info_str = nullptr;
	String os_info() {
		if (!info_str) {
			ScopeLock lock(mutex);
			if (!info_str) {
				auto str = new String();
				static struct utsname uts;
				static Char name[256];
				gethostname(name, 255);
				uname(&uts);
				*str = String::format(
					"host: %s\nsys: %s\nmachine: %s\n"
					"nodename: %s\nversion: %s\nrelease: %s",
					name,
					uts.sysname,
					uts.machine,
					uts.nodename, uts.version, uts.release
				);
				//  getlogin(), getuid(), getgid(),
				info_str = str;
			}
		}
		return *info_str;
	}
#endif

#if Qk_APPLE
	void os_get_languages_mac(Array<String>& langs);
#endif

	struct language_t {
		Array<String> langs;
	};
	static language_t* language = nullptr;
	static language_t* get_languages() {
		if (!language) {
			ScopeLock lock(mutex);
			if (!language) {
				auto langs = new language_t;
#if Qk_iOS
				os_get_languages_mac(langs->langs);
#elif Qk_ANDROID
				langs->langs.push(Android_language());
#elif Qk_LINUX
				cChar* lang = getenv("LANG") ? getenv("LANG"): getenv("LC_ALL");
				if ( lang ) {
					langs->langs.push(String(lang).split('.')[0]);
				} else {
					langs->langs.push("en_US");
				}
#endif
				language = langs;
			}
		}
		return language;
	}

	cArray<String>& os_languages() {
		return get_languages()->langs;
	}

	bool os_is_wifi() {
		return os_network_interface() == 2;
	}

	bool os_is_mobile() {
		return os_network_interface() >= 3;
	}

#if Qk_LINUX

	static std::atomic_int priv_cpu_total_count(0);
	static std::atomic_int priv_cpu_usage_count(0);

	float os_cpu_usage() {
		Char bf[512] = {0};
		Array<String> cpus;
		String prev_str;
		int size;
		int fd = fs_open_sync("/proc/stat");
		if (fd <= 0) return 0;

		while((size = fs_read_sync(fd, bf, 511))) {
			Char* s = bf;
			Char* ch;
			while ((ch = strchr(s, '\n'))) {
				prev_str.append(s, ch - s);
				cpus.push(prev_str);
				prev_str = String();
				size -= (ch - s + 1);
				s = ch + 1;
				if (size >= 3) {
					if (s[0] != 'c' || s[1] != 'p' || s[2] != 'u') {
						goto close;
					}
				}
			}
			prev_str = String(s, size);
			memset(bf, 0, 512);
		}

		close:

		fs_close_sync(fd);

		/*
		cpu  13338472 1558806 14443730 67158069 309781 8802 449488 0 0 0
		cpu0 5973903 511252 6880315 56648901 292831 5742 229508 0 0 0
		cpu1 3601621 560275 3626209 3411119 7260 1362 175918 0 0 0
		cpu2 3197300 455895 3207150 3442833 5097 1186 32364 0 0 0
		cpu3 565647 31382 730054 3655214 4591 510 11697 0 0 0
		*/

		if (cpus.length() < 2) {
			return 0;
		}

		auto ls = cpus[0].substr(3).trim().split(' ');
		int total = ls[0].toNumber<int>() +
						ls[1].toNumber<int>() +
						ls[2].toNumber<int>() +
						ls[3].toNumber<int>() +
						ls[4].toNumber<int>() +
						ls[5].toNumber<int>() + ls[6].toNumber<int>();

		fd = fs_open_sync(String::format("/proc/%d/stat", getpid()));
		if (fd <= 0) return 0;
		memset(bf, 0, 512);
		size = fs_read_sync(fd, bf, 511);
		ls = String(bf, size).split(' ');
		fs_close_sync(fd);
		if (ls.length() < 17) return 0;

		int usage = ls[13].toNumber<int>() +
						ls[14].toNumber<int>() +
						ls[15].toNumber<int>() + ls[16].toNumber<int>();

		int priv_total = priv_cpu_total_count;
		int priv_usage = priv_cpu_usage_count;

		float cpu_usage = float(usage - priv_usage) / float(total - priv_total);

		priv_cpu_total_count = total;
		priv_cpu_usage_count = usage;

		return cpu_usage * (cpus.length() - 1);
	}

#endif

}
