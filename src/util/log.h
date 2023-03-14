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

#ifndef __quark__util__log__
#define __quark__util__log__

#include "./macros.h"
#include "./string.h"

namespace qk {
	class Error;

	class Qk_EXPORT Log {
	public:
		enum Type {
			kLog,kWarn,kError
		};
		typedef NonObjectTraits Traits;
		virtual ~Log();
		virtual void log(cChar* log, cChar* end = nullptr);
		virtual void warn(cChar* log, cChar* end = nullptr);
		virtual void error(cChar* log, cChar* end = nullptr);
		virtual void fflush();
		static  void set_shared(Log *c);
		static  Log* shared();
		void         print(Type t, cChar*, ...);
		void         println(Type t, cChar*, ...);
	};

	Qk_EXPORT void log_print(cChar*, ...);
	Qk_EXPORT void log_println(cChar*, ...);
	Qk_EXPORT void log_println(int8_t s);
	Qk_EXPORT void log_println(uint8_t s);
	Qk_EXPORT void log_println(int16_t s);
	Qk_EXPORT void log_println(uint16_t s);
	Qk_EXPORT void log_println(int32_t s);
	Qk_EXPORT void log_println(uint32_t s);
	Qk_EXPORT void log_println(float s);
	Qk_EXPORT void log_println(double);
	Qk_EXPORT void log_println(int64_t);
	Qk_EXPORT void log_println(uint64_t);
	Qk_EXPORT void log_println(size_t);
	Qk_EXPORT void log_println(bool);
	Qk_EXPORT void log_println(cString&);
	Qk_EXPORT void log_println(cBuffer&);
	Qk_EXPORT void log_println(cString2&);
	Qk_EXPORT void log_println_warn(cChar*, ...);
	Qk_EXPORT void log_println_error(cChar*, ...);
	Qk_EXPORT void log_println_error(const Error&);
	Qk_EXPORT void log_fflush();
}
#endif
