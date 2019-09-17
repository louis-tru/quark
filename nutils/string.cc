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

#include "nutils/string.h"

XX_NS(ngui)

template<>
BasicString<char, Container<char>>::BasicString(const Object& o) {
	String str = o.to_string();
	m_core = str.m_core;
	m_core->retain();
}

template<> BasicString<uint16, Container<uint16>>::BasicString(const Object& o) {
	String str = o.to_string();
	ArrayBuffer<uint16> data = Coder::decoding_to_uint16(Encoding::utf8, str);
	uint len = data.length();
	Char* s = data.collapse();
	if (s) {
		m_core = new StringCore(len, s);
	} else {
		m_core = StringCore::empty();
	}
}

template<> BasicString<uint32, Container<uint32>>::BasicString(const Object& o) {
	String str = o.to_string();
	ArrayBuffer<uint32> data = Coder::decoding_to_uint32(Encoding::utf8, str);
	uint len = data.length();
	Char* s = data.collapse();
	if (s) {
		m_core = new StringCore(len, s);
	} else {
		m_core = StringCore::empty();
	}
}

template<>
String BasicString<char, Container<char>>::to_string() const {
	return *this;
}

template<>
String BasicString<uint16, Container<uint16>>::to_string() const {
	return Coder::encoding(Encoding::utf8, *this);
}

template<>
String BasicString<uint32, Container<uint32>>::to_string() const {
	return Coder::encoding(Encoding::utf8, *this);
}

XX_END
