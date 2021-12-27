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

#include "./log.h"
#include "./error.h"
#include "./codec.h"
#include <stdio.h>
#include <algorithm>

#if defined(__GLIBC__) || defined(__GNU_LIBRARY__)
# define F_VLIBC_GLIBC 1
#endif

#if F_VLIBC_GLIBC || F_BSD
# include <cxxabi.h>
# include <dlfcn.h>
# include <execinfo.h>
#elif F_QNX
# include <backtrace.h>
#endif  // F_VLIBC_GLIBC || F_BSD

#if F_GNUC && !F_ANDROID
# define IMMEDIATE_CRASH() __builtin_trap()
#else
# define IMMEDIATE_CRASH() ((void(*)())0)()
#endif

#if F_ANDROID
#include <android/log.h>
#endif

#ifndef f_stderr
# define f_stderr stdout
#endif

#define F_STRING_FORMAT(format, str) \
	va_list __arg; \
	va_start(__arg, format); \
	String str = string_format(format, __arg); \
	va_end(__arg)

namespace flare {

	String string_format(cChar* f, va_list arg);

	static Console* _default_console = nullptr;

	void Console::log(cString& str, cChar* feed) {
		#if F_ANDROID
			__android_log_print(ANDROID_LOG_INFO, "LOG ", "%s%s", str.c_str(), feed ? feed: "");
		#else
			printf("%s%s", str.c_str(), feed ? feed: "");
		#endif
	}

	void Console::warn(cString& str, cChar* feed) {
		#if F_ANDROID
			__android_log_print(ANDROID_LOG_WARN, "WARN", "%s%s", str.c_str(), feed ? feed: "");
		#else
			printf("%s%s", str.c_str(), feed ? feed: "");
		#endif
	}

	void Console::error(cString& str, cChar* feed) {
		#if F_ANDROID
			__android_log_print(ANDROID_LOG_ERROR, "ERR ", "%s%s", str.c_str(), feed ? feed: "");
		#else
			fprintf(f_stderr, "%s%s", str.c_str(), feed ? feed: "");
		#endif
	}

	void Console::clear() {
		fflush(stdout);
		fflush(f_stderr);
	}

	void Console::set_as_default() {
		if (_default_console != this) {
			delete _default_console;
			_default_console = this;
		}
	}

	Console* Console::instance() {
		if (!_default_console) {
			New<Console>()->set_as_default();
		}
		return _default_console;
	}

	namespace console {

		void log(int8_t msg) {
			Console::instance()->log( String::format("%u\n", msg) );
		}
		
		void log(uint8_t msg) {
			Console::instance()->log( String::format("%u\n", msg) );
		}

		void log(int16_t msg) {
			Console::instance()->log( String::format("%d\n", msg) );
		}

		void log(uint16_t  msg) {
			Console::instance()->log( String::format("%u\n", msg) );
		}

		void log(int32_t msg) {
			Console::instance()->log( String::format("%d\n", msg) );
		}
		
		void log(uint32_t msg) {
			Console::instance()->log( String::format("%u\n", msg) );
		}

		void log(float msg) {
			Console::instance()->log( String::format("%f\n", msg) );
		}

		void log(double msg) {
			Console::instance()->log( String::format("%lf\n", msg) );
		}

		void log(int64_t msg) {
			#if F_ARCH_64BIT
				Console::instance()->log( String::format("%ld\n", msg) );
			#else
				Console::instance()->log( String::format("%lld\n", msg) );
			#endif
		}

		void log(uint64_t msg) {
			#if F_ARCH_64BIT
				Console::instance()->log( String::format("%lu\n", msg) );
			#else
				Console::instance()->log( String::format("%llu\n", msg) );
			#endif
		}

		void log(size_t msg) {
			#if F_ARCH_64BIT
				Console::instance()->log( String::format("%lu\n", msg) );
			#else
				Console::instance()->log( String::format("%llu\n", msg) );
			#endif
		}

		void log(bool msg) {
			Console::instance()->log( msg ? "true\n": "false\n" );
		}

		void log(cString& msg) {
			Console::instance()->log(msg, "\n");
		}
		
		void log(cString2& msg) {
			Console::instance()->log(Coder::encode(Encoding::utf8, msg), "\n");
		}

		void log(cChar* format, ...) {
			F_STRING_FORMAT(format, str);
			Console::instance()->log(str, "\n");
		}
		
		void warn(cChar* format, ...) {
			F_STRING_FORMAT(format, str);
			Console::instance()->warn(str, "\n");
		}
		
		void error(cChar* format, ...) {
			F_STRING_FORMAT(format, str);
			Console::instance()->error(str, "\n");
		}
		
		void error(const Error& err) {
			auto str = String::format("Error: %d \n message:\n\t%s\n", err.code(), err.message().c_str());
			Console::instance()->error(str);
		}

	}

	static void report_error(cChar* format, ...) {
		F_STRING_FORMAT(format, str);
		printf("%s", str.c_str());
	}

	// Attempts to dump a backtrace (if supported).
	static void dump_backtrace() {
		#if F_VLIBC_GLIBC || F_BSD
			void* trace[100];
			int size = backtrace(trace, 100);
			report_error("\n==== C stack trace ===============================\n\n");
			if (size == 0) {
				report_error("(empty)\n");
			} else {
				for (int i = 1; i < size; ++i) {
					report_error("%2d: ", i);
					Dl_info info;
					Char* demangled = NULL;
					if (!dladdr(trace[i], &info) || !info.dli_sname) {
						report_error("%p\n", trace[i]);
					} else if ((demangled = abi::__cxa_demangle(info.dli_sname, 0, 0, 0))) {
						report_error("%s\n", demangled);
						free(demangled);
					} else {
						report_error("%s\n", info.dli_sname);
					}
				}
			}
		#elif F_QNX
			Char out[1024];
			bt_accessor_t acc;
			bt_memmap_t memmap;
			bt_init_accessor(&acc, BT_SELF);
			bt_load_memmap(&acc, &memmap);
			bt_sprn_memmap(&memmap, out, sizeof(out));
			error(out);
			bt_addr_t trace[100];
			int size = bt_get_backtrace(&acc, trace, 100);
			report_error("\n==== C stack trace ===============================\n\n");
			if (size == 0) {
				report_error("(empty)\n");
			} else {
				bt_sprnf_addrs(&memmap, trace, size, const_cast<Char*>("%a\n"),
											out, sizeof(out), NULL);
				report_error(out);
			}
			bt_unload_memmap(&memmap);
			bt_release_accessor(&acc);
		#endif  // F_VLIBC_GLIBC || F_BSD
	}

	void fatal(cChar* file, uint32_t line, cChar* func, cChar* msg, ...) {
		Console::instance()->clear();
		if (msg) {
			F_STRING_FORMAT(msg, str);
			Console::instance()->error("\n\n\n");
			Console::instance()->error(str, "\n");
		}
		report_error("#\n# Fatal error in %s, line %d, func %s\n# \n\n", file, line, func);
		dump_backtrace();
		Console::instance()->clear();
		IMMEDIATE_CRASH();
	}

}
