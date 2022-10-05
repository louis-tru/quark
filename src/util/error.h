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

#ifndef __quark__util__error__
#define __quark__util__error__

#include "./string.h"
#include "./errno.h"

#if !Qk_EXCEPTIONS_SUPPORT
	#error Exceptions must be turned on
#endif

#define Qk_THROW(code, ...) throw quark::Error(code, __VA_ARGS__)
#define Qk_CHECK(cond, ...) if(!(cond)) throw quark::Error(__VA_ARGS__)

#define Qk_ERROR_IGNORE(block) try block catch (quark::Error& err) {    \
	Qk_DEBUG("%s,%s", "The exception is ignored", err.message().c_str());     \
}((void)0)

namespace quark {

	/**
	* @class Error
	*/
	class Qk_EXPORT Error: public Object {
	public:
		Error(const Error& err);
		Error(cChar* msg, ...);
		Error(int code, cChar* msg, ...);
		Error(int code = ERR_UNKNOWN_ERROR, cString& msg = "Unknown exception");
		virtual ~Error();
		Error& operator=(const Error& e);
		String message() const throw();
		int    code() const throw();
	private:
		int     _code;
		String _message;
	};

	typedef const Error cError;
}

#endif
