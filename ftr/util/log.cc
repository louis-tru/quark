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
# define FX_VLIBC_GLIBC 1
#endif

#if FX_VLIBC_GLIBC || FX_BSD
# include <cxxabi.h>
# include <dlfcn.h>
# include <execinfo.h>
#elif FX_QNX
# include <backtrace.h>
#endif  // FX_VLIBC_GLIBC || FX_BSD

#if FX_GNUC && !FX_ANDROID
# define IMMEDIATE_CRASH() __builtin_trap()
#else
# define IMMEDIATE_CRASH() ((void(*)())0)()
#endif

#ifndef nx_stderr
# define nx_stderr stdout
#endif

#define FX_STRING_FORMAT(format, str) \
	va_list __arg; \
	va_start(__arg, format); \
	String str = string_format(format, __arg); \
	va_end(__arg)

namespace ftr {

	void Console::log(cString& str) {
		printf("%s\n", str.str_c());
	}
	void Console::warn(cString& str) {
		printf("Warning: %s\n", str.str_c());
	}
	void Console::error(cString& str) {
		fprintf(nx_stderr, "%s\n", str.str_c());
	}
	void Console::print(cString& str) {
		printf("%s", str.str_c());
	}
	void Console::print_err(cString& str) {
		fprintf(nx_stderr, "%s", str.str_c());
	}
	void Console::clear() {
		// noop
	}

	Console* _default_console = nullptr;

	void Console::set_as_default() {
		if (_default_console != this) {
			delete _default_console;
			_default_console = this;
		}
	}

	static Console* default_console() {
		if (!_default_console) {
			New<Console>()->set_as_default();
		}
		return _default_console;
	}

	String string_format(cChar* f, va_list arg);

	namespace console {
		
		void report_error(cChar* format, ...) {
			FX_STRING_FORMAT(format, str);
			printf("%s", str.str_c());
		}
		
		// Attempts to dump a backtrace (if supported).
		void dump_backtrace() {
			#if FX_VLIBC_GLIBC || FX_BSD
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
			#elif FX_QNX
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
			#endif  // FX_VLIBC_GLIBC || FX_BSD
		}
		
		void log(Char msg) {
			default_console()->log( String::format("%u", msg) );
		}
		
		void log(uint8_t msg) {
			default_console()->log( String::format("%u", msg) );
		}

		void log(int16_t msg) {
			default_console()->log( String::format("%d", msg) );
		}

		void log(uint16_t  msg) {
			default_console()->log( String::format("%u", msg) );
		}

		void log(int msg) {
			default_console()->log( String::format("%d", msg) );
		}
		
		void log(uint32_t msg) {
			default_console()->log( String::format("%u", msg) );
		}

		void log(float msg) {
			default_console()->log( String::format("%f", msg) );
		}

		void log(double msg) {
			default_console()->log( String::format("%lf", msg) );
		}

		void log(int64_t msg) {
			#if FX_ARCH_64BIT
				default_console()->log( String::format("%ld", msg) );
			#else
				default_console()->log( String::format("%lld", msg) );
			#endif
		}
		
		#if FX_ARCH_32BIT
			void log(long msg) {
				default_console()->log( String::format("%ld", msg) );
			}
			void log(unsigned long msg) {
				default_console()->log( String::format("%lu", msg) );
			}
		#endif

		void log(uint64_t msg) {
			#if FX_ARCH_64BIT
				default_console()->log( String::format("%lu", msg) );
			#else
				default_console()->log( String::format("%llu", msg) );
			#endif
		}

		void log(bool msg) {
			default_console()->log( msg ? "true": "false" );
		}
		
		void log(cChar* format, ...) {
      FX_STRING_FORMAT(format, str);
			default_console()->log(str);
		}
		
		void log(cString& msg) {
			default_console()->log(msg);
		}
		
		void log(cString16& msg) {
			default_console()->log(Coder::encode(Encoding::utf8, msg));
		}

		void print(cChar* format, ...) {
			FX_STRING_FORMAT(format, str);
			default_console()->print(str);
		}

		void print(cString& str) {
			default_console()->print(str);
		}
		
		void print_err(cChar* format, ...) {
			FX_STRING_FORMAT(format, str);
			default_console()->print_err(str);
		}
		
		void print_err(cString& str) {
			default_console()->print_err(str);
		}
		
		void warn(cChar* format, ...) {
			FX_STRING_FORMAT(format, str);
			default_console()->warn(str);
		}
		
		void warn(cString& str) {
			default_console()->warn(str);
		}
		
		void error(cChar* format, ...) {
			FX_STRING_FORMAT(format, str);
			default_console()->error(str);
		}
		
		void error(cString& str) {
			default_console()->error(str);
		}

		void error(const Error& err) {
			auto str = String::format("Error: %d \n message:\n\t%s", err.code(), err.message().str_c());
			default_console()->error(str);
		}
		
		void tag(cChar* tag, cChar* format, ...) {
			FX_STRING_FORMAT(format, str);
			default_console()->print(String::format("%s ", tag));
			default_console()->log(str);
		}

		void clear() {
			default_console()->clear();
		}
		
	} // end namescape console {


	void fatal(cChar* file, uint32_t line, cChar* func, cChar* msg, ...) {
		fflush(stdout);
		fflush(nx_stderr);
		if (msg) {
			FX_STRING_FORMAT(msg, str);
			default_console()->print_err("\n\n\n");
			default_console()->error(str);
		}
		console::report_error("#\n# Fatal error in %s, line %d, func %s\n# \n\n", file, line, func);
		console::dump_backtrace();
		fflush(stdout);
		fflush(nx_stderr);
		IMMEDIATE_CRASH();
	}

}
