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

#include "util.h"
#include "v8.h"
#include <string>

#if defined(__GLIBC__) || defined(__GNU_LIBRARY__)
# define v8_vlibc_glibc 1
#endif

#if v8_vlibc_glibc || V8_OS_BSD
# include <cxxabi.h>
# include <dlfcn.h>
# include <execinfo.h>
#elif V8_OS_QNX
# include <backtrace.h>
#endif  // v8_vlibc_glibc || V8_OS_BSD

#if V8_CC_GNU && !V8_OS_ANDROID
# define IMMEDIATE_CRASH() __builtin_trap()
#else
# define IMMEDIATE_CRASH() ((void(*)())0)()
#endif

namespace v8 {
	
	void log(const std::string& msg) {
		fprintf(stdout, "%s\n", msg.c_str());
	}
	
	void log(const char* msg, ...) {
		va_list args;
		va_start(args, msg);
		vfprintf(stdout, msg, args);
		fprintf(stdout, "%s", "\n");
	}
	
	void report_error(const char* msg, ...) {
		va_list args;
		va_start(args, msg);
		vfprintf(stderr, msg, args);
	}
	
	void dump_backtrace() {
#if v8_vlibc_glibc || V8_OS_BSD
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
#elif V8_OS_QNX
		char out[1024];
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
			bt_sprnf_addrs(&memmap, trace, size, const_cast<char*>("%a\n"),
										 out, sizeof(out), NULL);
			report_error(out);
		}
		bt_unload_memmap(&memmap);
		bt_release_accessor(&acc);
#endif  // v8_vlibc_glibc || V8_OS_BSD
	}
	
	extern void fatal(const char* msg, ...) {
		fflush(stdout);
		fflush(stderr);
		if (msg) {
			va_list args;
			va_start(args, msg);
			vfprintf(stderr, msg, args);
		}
		dump_backtrace();
		fflush(stdout);
		fflush(stderr);
		IMMEDIATE_CRASH();
	}
	
	void fatal(const char* file, int line, const char* func, const char* msg, ...) {
		fflush(stdout);
		fflush(stderr);
		if (msg) {
			va_list args;
			va_start(args, msg);
			vfprintf(stderr, msg, args);
		}
		fprintf(stderr, "#\n# Fatal error in %s, line %d, func %s\n# \n\n", file, line, func);
		dump_backtrace();
		fflush(stdout);
		fflush(stderr);
		IMMEDIATE_CRASH();
	}
}
