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

#include "ftr/util/error.h"
#include "ftr/util/util.h"

#if FX_EXCEPTIONS_SUPPORT

namespace ftr {

	Error::Error()
	: _code(ERR_UNKNOWN_ERROR)
	, _message("Unknown exception") {

	}

	Error::Error(int code, const String& msg)
	: _code(code)
	, _message(msg.copy()) {
	}

	Error::Error(const String& msg): _code(ERR_UNKNOWN_ERROR), _message(msg.copy()) {
	}

	Error::Error(cError& e)
	: _code(e.code())
	, _message(e._message.copy()) {
	}

	Error& Error::operator=(const Error& e) {
		_code = e._code;
		_message = e._message.copy();
		return *this;
	}

	Error::~Error() {
	}

	const String Error::message() const throw() {
		return _message;
	}

	int Error::code() const throw() {
		return _code;
	}

	void Error::set_message(const String& value) {
		_message = value.copy();
	}

	void Error::set_code(int value) {
		_code = value;
	}

	String Error::to_string() const {
		return String::format("message: %s, code: %d", *_message, _code);
	}
}

#endif
