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

#include "./os.h"
#include "./fs.h"
#include "./dict.h"
#include <string.h>
#include <atomic>
#include <unistd.h>

#if FX_UNIX
# include <sys/utsname.h>
# include <unistd.h>
#endif

#if FX_ANDROID
# include "../../android.h"
#endif

#if FX_APPLE
# include <mach/mach_time.h>
# include <mach/mach.h>
# include <mach/clock.h>

# define clock_gettime clock_gettime2

static clock_serv_t get_clock_port(clock_id_t clock_id) {
	clock_serv_t clock_r;
	host_get_clock_service(mach_host_self(), clock_id, &clock_r);
	return clock_r;
}

static clock_serv_t clock_realtime = get_clock_port(CALENDAR_CLOCK);
static mach_port_t clock_monotonic = get_clock_port(SYSTEM_CLOCK);

int clock_gettime2(clockid_t id, struct timespec *tspec) {
	mach_timespec_t mts;
	int retval = 0;
	if (id == CLOCK_MONOTONIC) {
		retval = clock_get_time(clock_monotonic, &mts);
		if (retval != 0) {
			return retval;
		}
	} else if (id == CLOCK_REALTIME) {
		retval = clock_get_time(clock_realtime, &mts);
		if (retval != 0) {
			return retval;
		}
	} else {
		/* only CLOCK_MONOTOIC and CLOCK_REALTIME clocks supported */
		return -1;
	}
	tspec->tv_sec = mts.tv_sec;
	tspec->tv_nsec = mts.tv_nsec;
	return 0;
}

#endif

namespace flare {
	namespace os {

		String name() {

			#if  FX_IOS
				static String _name("iOS");
			#elif  FX_OSX
				static String _name("MacOSX");
			#elif  FX_ANDROID
				static String _name("Android");
			#elif  FX_WIN
				static String _name("Windows");
			#elif  FX_LINUX
				static String _name("Linux");
			#else
				# error no support
			#endif
			return _name;
		}

		#if FX_UNIX
			static String* info_str = nullptr;

			String info() {
				
				if (!info_str) {
					info_str = new String();
					static struct utsname uts;
					static Char name[256];
					gethostname(name, 255);
					uname(&uts);
					*info_str = String::format(
						"host: %s\nsys: %s\nmachine: %s\n"
						"nodename: %s\nversion: %s\nrelease: %s",
						name,
						uts.sysname,
						uts.machine,
						uts.nodename, uts.version, uts.release
					);
					//  getlogin(), getuid(), getgid(),
				}
				return *info_str;
			}
		#endif

			int64_t time_second() {
				return ::time(nullptr);
			}

			int64_t time() {
				timespec now;
				clock_gettime(CLOCK_REALTIME, &now);
				int64_t r = now.tv_sec * 1000000LL + now.tv_nsec / 1000LL;
				return r;
			}

			int64_t time_monotonic() {
				timespec now;
				clock_gettime(CLOCK_MONOTONIC, &now);
				int64_t r = now.tv_sec * 1000000LL + now.tv_nsec / 1000LL;
				return r;
			}

			struct language_t {
				String langs;
				String lang;
			};

		#if FX_APPLE
			void get_languages_(String& langs, String& lang);
		#endif

		static language_t* langs_ = nullptr;
		static language_t* get_languages() {
			if (!langs_) {
				langs_ = new language_t();
			#if FX_IOS
				langs_ = new language_t();
				get_languages_(langs_->langs, langs_->lang);
			#elif FX_ANDROID
				langs_->langs = Android::language();
				langs_->lang = langs_->langs;
			#elif FX_LINUX
				cChar* lang = getenv("LANG") ? getenv("LANG"): getenv("LC_ALL");
				if ( lang ) {
					langs_->langs = String(lang).split('.')[0];
				} else {
					langs_->langs = "en_US";
				}
				langs_->lang = langs_->langs;
			#endif
			}
			return langs_;
		}

		String languages() {
			return get_languages()->langs;
		}

		String language() {
			return get_languages()->lang;
		}

		// advanced

		bool is_wifi() {
			return network_status() == 2;
		}

		bool is_mobile() {
			return network_status() >= 3;
		}

		#if FX_LINUX || FX_ANDROID

			static std::atomic_int priv_cpu_total_count(0);
			static std::atomic_int priv_cpu_usage_count(0);

			float cpu_usage() {
				Char bf[512] = {0};
				Array<String> cpus;
				String prev_str;
				int size;
				int fd = FileHelper::open_sync("/proc/stat");
				if (fd <= 0) return 0;

				while((size = FileHelper::read_sync(fd, bf, 511))) {
					Char* s = bf;
					Char* ch;
					while ((ch = strchr(s, '\n'))) {
						prev_str.push(s, ch - s);
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

				FileHelper::close_sync(fd);

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
				int total = ls[0].to_int() +
								ls[1].to_int() +
								ls[2].to_int() +
								ls[3].to_int() +
								ls[4].to_int() +
								ls[5].to_int() + ls[6].to_int();

				fd = FileHelper::open_sync(String::format("/proc/%d/stat", getpid()));
				if (fd <= 0) return 0;
				memset(bf, 0, 512);
				size = FileHelper::read_sync(fd, bf, 511);
				ls = String(bf, size).split(' ');
				FileHelper::close_sync(fd);
				if (ls.length() < 17) return 0;

				int usage = ls[13].to_int() +
								ls[14].to_int() +
								ls[15].to_int() + ls[16].to_int();

				int priv_total = priv_cpu_total_count;
				int priv_usage = priv_cpu_usage_count;

				float cpu_usage = float(usage - priv_usage) / float(total - priv_total);

				priv_cpu_total_count = total;
				priv_cpu_usage_count = usage;

				return cpu_usage * (cpus.length() - 1);
			}

		#endif

	}
}
