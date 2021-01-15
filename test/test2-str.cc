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

#include <stdio.h>
#include <sys/utsname.h>
#include <string>
#include <iostream>
#include <limits>
#include <stdarg.h>
#include <ftr/util/macros.h>
#include <ftr/util/str.h>
#include <functional>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>

const char test_big_char[] = { 1, 0, 0, 0 };
const int* test_big_int = (const int*)test_big_char;
const bool has_big_data = *test_big_int != 1;

using namespace std;

namespace ftr {

	int32_t vasprintf(char** o, const char* f, va_list arg) {
		#if FX_GNUC
			int32_t len = ::vasprintf(o, f, arg);
		#else
			int32_t len = ::vsprintf(o, f, arg);
			if (len) {
				o = (char*)::malloc(len + 1);
				o[len] = '\0';
				::_vsnprintf_s(o, len + 1, f, arg);
			}
		#endif
		return len;
	}

	int32_t sprintf(char*& o, uint32_t& capacity, const char* f, ...) {
		va_list arg;
		va_start(arg, f);
		int32_t len = vasprintf(&o, f, arg);
		va_end(arg);
		if (o) {
			capacity = len + 1;
		}
		return len;
	}

}

class Str {
public:
	Str(const char* v): _val(v) {
	}
	uint64_t hash_code() const {
		return 100;
	}
	private:
	const char* _val;
};

namespace std {
			
	template<>
	struct std::hash<Str> {
		size_t operator()(const Str& val) const {
			return val.hash_code();
		}
	};

	template<typename T, ftr::HolderMode M, typename A>
	 struct hash<ftr::BasicString<T, M, A>> {
		 size_t operator()(const ftr::BasicString<T, M, A>& val) const {
			 return 101;
		 }
	 };
}

void test_str2() {
	
	ftr::WeakBuffer wb(const_cast<char*>("ABCD"), 4);
	ftr::Buffer     sb(wb.copy());
	ftr::MutableString ms("ABCD");
	ftr::String s(ms);
	ftr::String s_(s);
	ftr::MutableString s2(ms.copy());
	
	cout
	<< endl
	<< "-------------" << endl
	<< "s.operator==(ms)" << s.operator==(ms) << endl
	<< "s.operator==(s):" << s.operator==(s) << endl
	<< "s.operator==('B'):" << s.operator==("B") << endl
	<< "s.operator==(wb):" << s.operator==(wb) << endl
	<< "s.operator==(sb):" << s.operator==(sb) << endl
	<< "s2.operator==(ms)" << s2.operator==(ms) << endl
	<< "s2.operator==(s):" << s2.operator==(s) << endl
	<< "s2.operator==('B'):" << s2.operator==("B") << endl
	<< "s2.operator==(wb):" << s2.operator==(wb) << endl
	<< "s2.operator==(sb):" << s2.operator==(sb) << endl
	
	<< endl
	
	<< "-------------" << endl
	<< "s.operator!=(ms)" << s.operator!=(ms) << endl
	<< "s.operator!=(s):" << s.operator!=(s) << endl
	<< "s.operator!=('B'):" << s.operator!=("B") << endl
	<< "s.operator!=(wb):" << s.operator!=(wb) << endl
	<< "s.operator!=(sb):" << s.operator!=(sb) << endl
	<< "s2.operator!=(ms)" << s2.operator!=(ms) << endl
	<< "s2.operator!=(s):" << s2.operator!=(s) << endl
	<< "s2.operator!=('B'):" << s2.operator!=("B") << endl
	<< "s2.operator!=(wb):" << s2.operator!=(wb) << endl
	<< "s2.operator!=(sb):" << s2.operator!=(sb) << endl
	
	<< endl;

	//	s2.split("A");
	
	s.operator=(s_);
//	auto A =
	ms.replace("A", "BBB");
	s.operator=(ms);
	s.operator=("A");
	
	s2.operator=(s_.copy());
	s2.operator=(ms);
	s2.operator=("A");
	
	s + s_;
	s + ms;
	s + "A";
	
	s2 + s_;
	s2 + ms;
	s2 + "A";
	
//	s += s_;
//	s += ms;
//	s += "A";
	
	s2 += s_;
	ms + "B";
	s2 += "C";
	s2 += "A";
	ms+= "100";
}

int test2_str(int argc, char *argv[]) {

	std::string s = std::string("-hello").substr(1,5) + "-";
	
	char* a = const_cast<char*>(s.c_str());
	a[0] = 'U';
	
	test_str2();
	
	std::cout
	<< "sizeof(s):" << sizeof(s) << endl
	<< "capacity:" << s.capacity() << endl
	<< "size:" << s.size() << endl
	<< "length:" << s.length() << endl
	<< "str:" << s << endl
	<< "ptr:" << s.c_str() << endl
	<< "uint32_t:" << std::numeric_limits<uint32_t>::max() << endl
	<< "has_big_data:" << has_big_data << endl
	<< "i:" << 3 / 4 << endl
	<< "memcmp:" << memcmp("A", "A", 1) << endl
	<< "std::hash<std::string>():" << std::hash<std::string>()("ABCD") << endl
	<< "std::hash<Str>():" << std::hash<Str>()("ABCD") << endl
	<< "std::hash<String>():" << std::hash<ftr::String>()("ABCD") << endl
	<< endl;

	return 0;
}
