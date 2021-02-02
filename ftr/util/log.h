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

#ifndef __ftr__util__log__
#define __ftr__util__log__

#include "./macros.h"
#include "./string.h"

namespace ftr {

	class Error;

	/**
	* @class Console # util log
	*/
	class FX_EXPORT Console {
		public:
			// typedef NonObjectTraits Traits;
			virtual ~Console() = default;
			virtual void log(cString& str);
			virtual void warn(cString& str);
			virtual void error(cString& str);
			virtual void print(cString& str);
			virtual void print_err(cString& str);
			virtual void clear();
			void set_as_default();
	};

	namespace console {
		FX_EXPORT void log(Char s);
		FX_EXPORT void log(uint8_t s);
		FX_EXPORT void log(int16_t s);
		FX_EXPORT void log(uint32_t s);
		FX_EXPORT void log(int s);
	
		FX_EXPORT void log(uint32_t s);
		FX_EXPORT void log(float s);
		FX_EXPORT void log(double);
		FX_EXPORT void log(int64_t);
		FX_EXPORT void log(uint64_t);
		FX_EXPORT void log(bool);
		FX_EXPORT void log(cChar*, ...);
		FX_EXPORT void log(cString&);
		FX_EXPORT void log(cString16&);
		FX_EXPORT void warn(cChar*, ...);
		FX_EXPORT void warn(cString&);
		FX_EXPORT void error(cChar*, ...);
		FX_EXPORT void error(cString&);
		FX_EXPORT void error(const Error&);
		FX_EXPORT void tag(cChar*, cChar*, ...);
		FX_EXPORT void print(cChar*, ...);
		FX_EXPORT void print(cString&);
		FX_EXPORT void print_err(cChar*, ...);
		FX_EXPORT void print_err(cString&);
		FX_EXPORT void clear();
		#if FX_ARCH_32BIT
			FX_EXPORT void log(long);
			FX_EXPORT void log(unsigned long);
		#endif
	}

}
#endif
