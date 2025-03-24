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

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <quark/util/string.h>
#include <quark/util/codec.h>

using namespace std;
using namespace qk;

class Str {
 private:
	char* _value;
	int _length;
 public:
 
	Str (const Str& str) {
		_length = str._length;
		_value = (char*)malloc(_length + 1);
		memcpy(_value, str._value, _length + 1);
		Qk_Log("%s\n", "copy constructor");
	}
	Str (Str&& str) {
		_length = str._length;
		_value = str._value;
		str._value = NULL;
		Qk_Log("%s\n", "move constructor");
	}
	Str (const char* str) {
		_length = strlen(str);
		_value = (char*)malloc(_length + 1);
		memcpy(_value, str, _length + 1);
		Qk_Log("%s\n", "new constructor");
	}
	
	~ Str () {
		if (_value) {
			free(_value);
			_value = NULL;
		}
	}
	
	const char* c_str () const {
		return _value;
	}
	const int length () const {
		return _length;
	}
};

void test (const Str& str) {
	printf("str:%s, len:%d, add:%s, test ok\n", str.c_str(), str.length(), str.c_str());
	Qk_Log("");
}

void test2 (const string& str) {
	printf("str:%s, len:%lu, add:%s, test ok\n", str.c_str(), str.length(), str.c_str());
}

// const static Str s = "op";

void test_string (int argc, char **argv) {
	
	// utf8 / ucs2 / ucs4
	String2 ucs2 = codec_decode_to_ucs2(kUTF8_Encoding, String("哎呀").array().buffer());
	String4 ucs4 = codec_decode_to_unicode(kUTF8_Encoding, String("咔咔").array().buffer());
	String utf8_1 = codec_encode(kUTF8_Encoding, ucs2.array().buffer());
	String utf8_2 = codec_encode(kUTF8_Encoding, ucs4.array().buffer());
	
	Qk_Log(ucs2.hashCode() % 10);
	Qk_Log(ucs4.hashCode() % 10);
	Qk_Log(utf8_1.hashCode() % 10);
	Qk_Log(utf8_2.hashCode() % 10);
	
	Qk_Log(ucs2[0]);
	Qk_Log(ucs2[1]);
	Qk_Log(ucs2[2]);
	Qk_Log(ucs2[3]);
	Qk_Log(ucs4[0]);
	Qk_Log(ucs4[1]);
	Qk_Log(ucs4[2]);
	Qk_Log(ucs4[3]);
	Qk_Log(utf8_1);
	Qk_Log(utf8_2);
	
	Qk_Log("%d", __cplusplus);
	
	const char* c = "op";
	
	Qk_Log(c);
	
	// Str s = "op";
	
	for (int i = 0; i < 10; i++) {
		// test(s);
		// test("op");
		test2(c);
	}
	
	const char* str = "ABCD";
	
	Qk_Log(str);
	
	string str0 = str;
	
	for (int i = 0; i < 10; i++) {
		str0 += "u";
	}
	
	const string&& str2 = std::move(str0);
	
	string str3(str0.c_str());
	
	Qk_Log("%u,%u\n", str0.c_str(), str3.c_str());
	
	Qk_Log("capacity:%d,%d\n", str0.capacity(), str3.capacity());
	
	const_cast<char*>(str0.c_str())[0] = 'K';
	
	str0 = "ABCD-KKKKK";
	
	Qk_Log("%u,%u\n", str0.c_str(), str3.c_str());
	
	Qk_Log("capacity:%d,%d\n", str0.capacity(), str3.capacity());
	
	Qk_Log("%s,%s\n", str0.c_str(), str3.c_str());
}
