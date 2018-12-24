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

#include "error.h"
#include "string.h"

XX_NS(ngui)

#if XX_EXCEPTIONS_SUPPORT

Error::Error()
: m_code(ERR_UNKNOWN_ERROR)
, m_message(new String("unknown exception")) {

}

Error::Error(int code, cString& msg)
: m_code(code)
, m_message(new String(msg)) {

}

Error::Error(int code, cchar* msg, ...): m_code(code) {
	XX_STRING_FORMAT(msg, m);
	m_message = new String(m);
}

Error::Error(cString& msg): m_code(ERR_UNKNOWN_ERROR), m_message(new String(msg)) {

}

Error::Error(cchar* msg, ...): m_code(ERR_UNKNOWN_ERROR) {
	XX_STRING_FORMAT(msg, m);
	m_message = new String(m);
}

Error::Error(cError& e)
: m_code(e.code())
, m_message(new String(*e.m_message)) {

}

Error& Error::operator=(const Error& e) {
	m_code = e.m_code;
	*m_message = *e.m_message;
	return *this;
}

Error::~Error() {
	Release(m_message);
}

cString& Error::message() const throw() {
	return *m_message;
}

int Error::code() const throw() {
	return m_code;
}

void Error::set_message(cString& value) {
	*m_message = value;
}

void Error::set_code(int value) {
	m_code = value;
}

#endif
XX_END
