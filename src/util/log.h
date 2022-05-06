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

#ifndef __noug__util__log__
#define __noug__util__log__

#include "./macros.h"
#include "./string.h"

namespace noug {

	class Error;

	/**
	* @class Console # util log
	*/
	class F_EXPORT Console {
	public:
		typedef NonObjectTraits Traits;
		virtual ~Console() = default;
		virtual void log(cString& log, cChar* feed = nullptr);
		virtual void warn(cString& log, cChar* feed = nullptr);
		virtual void error(cString& log, cChar* feed = nullptr);
		virtual void clear();
		void set_as_default();
		static Console* instance();
	};

	namespace console {
		F_EXPORT void log(int8_t s);
		F_EXPORT void log(uint8_t s);
		F_EXPORT void log(int16_t s);
		F_EXPORT void log(uint16_t s);
		F_EXPORT void log(int32_t s);
		F_EXPORT void log(uint32_t s);
		F_EXPORT void log(float s);
		F_EXPORT void log(double);
		F_EXPORT void log(int64_t);
		F_EXPORT void log(uint64_t);
		F_EXPORT void log(size_t);
		F_EXPORT void log(bool);
		F_EXPORT void log(cString&);
		F_EXPORT void log(cString2&);
		F_EXPORT void log(cChar*, ...);
		F_EXPORT void warn(cChar*, ...);
		F_EXPORT void error(cChar*, ...);
		F_EXPORT void error(const Error&);
	}

}
#endif
