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

#include "./log.h"
#include "./error.h"
#include "./codec.h"
#include "./array.h"
#include <stdio.h>
#include <algorithm>

#if defined(__GLIBC__) || defined(__GNU_LIBRARY__)
# define Qk_VLIBC_GLIBC 1
#endif

#if Qk_VLIBC_GLIBC || Qk_BSD
# include <cxxabi.h>
# include <dlfcn.h>
# include <execinfo.h>
#elif Qk_QNX
# include <backtrace.h>
#endif  // Qk_VLIBC_GLIBC || Qk_BSD

#if defined(__GNUC__) && !Qk_ANDROID
# define IMMEDIATE_CRASH() __builtin_trap()
#else
# define IMMEDIATE_CRASH() ((void(*)())0)()
#endif

#if Qk_ANDROID
#include <android/log.h>
#endif

#ifndef f_stderr
# define f_stderr stdout
#endif

#define Qk_STRING_FORMAT(f, str) \
	va_list __arg; \
	va_start(__arg, f); \
	String str = _Str::printfv(f, __arg); \
	va_end(__arg)

namespace qk {

	static Log* _shared_log = nullptr;

	Log::~Log() {}

	void Log::log(cChar* log, cChar* end) {
#if Qk_ANDROID
			__android_log_print(ANDROID_LOG_INFO, "LOG ", "%s%s", log.c_str(), end ? end: "");
#else
			printf("%s%s", log, end ? end: "");
#endif
	}

	void Log::warn(cChar* log, cChar* end) {
#if Qk_ANDROID
			__android_log_print(ANDROID_LOG_WARN, "WARN", "%s%s", log.c_str(), end ? end: "");
#else
			printf("%s%s", log, end ? end: "");
#endif
	}

	void Log::error(cChar* log, cChar* end) {
#if Qk_ANDROID
			__android_log_print(ANDROID_LOG_ERROR, "ERR ", "%s%s", log.c_str(), end ? end: "");
#else
			fprintf(f_stderr, "%s%s", log, end ? end: "");
#endif
	}

	void Log::fflush() {
		::fflush(stdout);
		::fflush(f_stderr);
	}

	void Log::print(Type t, cChar *msg, ...) {
		Qk_STRING_FORMAT(msg, str);
		switch(t) {
			case kLog: log(str.c_str()); break;
			case kWarn: warn(str.c_str()); break;
			default: error(str.c_str()); break;
		}
	}

	void Log::println(Type t, cChar *msg, ...) {
		Qk_STRING_FORMAT(msg, str);
		switch(t) {
			case kLog: log(str.c_str(), "\n"); break;
			case kWarn: warn(str.c_str(), "\n"); break;
			default: error(str.c_str(), "\n"); break;
		}
	}

	void Log::set_shared(Log *log) {
		if (_shared_log != log) {
			delete _shared_log;
			_shared_log = log;
		}
	}

	Log* Log::shared() {
		if (!_shared_log) {
			set_shared(New<Log>());
		}
		return _shared_log;
	}

	// -------------------------------------------------------------------------------------

	void log_print(cChar *format, ...) {
		Qk_STRING_FORMAT(format, str);
		Log::shared()->log(str.c_str());
	}

	void log_println(cChar* format, ...) {
		Qk_STRING_FORMAT(format, str);
		Log::shared()->log(str.c_str(), "\n");
	}

	void log_println(int8_t msg) {
		Log::shared()->println(Log::kLog, "%u", msg);
	}
	
	void log_println(uint8_t msg) {
		Log::shared()->println(Log::kLog, "%u", msg );
	}

	void log_println(int16_t msg) {
		Log::shared()->println(Log::kLog, "%d", msg );
	}

	void log_println(uint16_t  msg) {
		Log::shared()->println(Log::kLog, "%u", msg );
	}

	void log_println(int32_t msg) {
		Log::shared()->println(Log::kLog, "%d", msg );
	}
	
	void log_println(uint32_t msg) {
		Log::shared()->println(Log::kLog, "%u", msg );
	}

	void log_println(float msg) {
		Log::shared()->println(Log::kLog, "%f", msg );
	}

	void log_println(double msg) {
		Log::shared()->println(Log::kLog, "%lf", msg );
	}

	void log_println(int64_t msg) {
		Log::shared()->println(Log::kLog, Qk_ARCH_64BIT ? "%ld": "%lld", msg );
	}

	void log_println(uint64_t msg) {
		Log::shared()->println(Log::kLog, Qk_ARCH_64BIT ? "%lu": "%llu", msg );
	}

	void log_println(size_t msg) {
		Log::shared()->println(Log::kLog, Qk_ARCH_64BIT ? "%lu": "%llu", msg );
	}

	void log_println(bool msg) {
		Log::shared()->log( msg ? "true\n": "false\n" );
	}

	void log_println(cString& msg) {
		Log::shared()->log(msg.c_str(), "\n");
	}

	void log_println_warn(cString& msg) {
		Log::shared()->warn(msg.c_str(), "\n");
	}

	void log_println_error(cString& msg) {
		Log::shared()->error(msg.c_str(), "\n");
	}

	void log_println(cBuffer& buf) {
		Log::shared()->log(*buf, "\n");
	}
	
	void log_println(cString2& msg) {
		Log::shared()->log(*codec_encode(kUTF8_Encoding, msg.array().buffer()), "\n");
	}

	void log_println_warn(cChar* format, ...) {
		Qk_STRING_FORMAT(format, str);
		Log::shared()->warn(str.c_str(), "\n");
	}

	void log_println_error(cChar* format, ...) {
		Qk_STRING_FORMAT(format, str);
		Log::shared()->error(str.c_str(), "\n");
	}
	
	void log_println_error(const Error& err) {
		Log::shared()->print(Log::kError, "Error: %d \n message:\n\t%s\n", err.code(), err.message().c_str() );
	}

	void log_fflush() {
		Log::shared()->fflush();
	}

	static void report_error(cChar* format, ...) {
		Qk_STRING_FORMAT(format, str);
		Log::shared()->log(str.c_str());
	}

	// Attempts to dump a backtrace (if supported).
	static void dump_backtrace() {
#if Qk_VLIBC_GLIBC || Qk_BSD
			void* trace[100];
			int size = backtrace(trace, 100);
			report_error("\n==== C stack trace ===============================\n\n");
			if (size == 0) {
				report_error("(empty)\n");
			} else {
				for (int i = 1; i < size; ++i) {
					report_error("%2d: ", i);
					Dl_info info;
					char* demangled = NULL;
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
#elif Qk_QNX
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
#endif  // Qk_VLIBC_GLIBC || Qk_BSD
	}

	void Fatal(cChar* file, uint32_t line, cChar* func, cChar* msg, ...) {
		Log::shared()->fflush();
		if (msg) {
			Qk_STRING_FORMAT(msg, str);
			_shared_log->error("\n\n\n");
			_shared_log->error(str.c_str(), "\n");
		}
		report_error("#\n# Fatal error in %s, line %d, func %s\n# \n\n", file, line, func);
		dump_backtrace();
		_shared_log->fflush();

		IMMEDIATE_CRASH();
	}

}
