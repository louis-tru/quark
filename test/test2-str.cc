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

#include <stdio.h>
#include <sys/utsname.h>
#include <string>
#include <iostream>
#include <limits>
#include <stdarg.h>
#include <quark/util/macros.h>
#include <quark/util/string.h>
#include <functional>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <unordered_map>

const char test_big_char[] = { 1, 0, 0, 0 };
const int* test_big_int = (const int*)test_big_char;
const bool has_big_data = *test_big_int != 1;

using namespace std;
using namespace qk;

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
	struct hash<Str> {
		size_t operator()(const Str& val) const {
			return val.hash_code();
		}
	};
}

void test_str2() {

	String strv("ABCDEFG");
	
	{
		String sv = strv;
		String s(sv.copy());
		cout
		<< endl
		<< "s2.operator==('B'):" << s.operator==("B") << endl
		<< "s2.operator==(ms)"   << s.operator==(s) << endl
		<< "s2.operator==(s):"   << s.operator==(sv) << endl
		<< endl;
	}

	{
		String sv = strv;
		String s(sv.copy());
		cout
		<< endl
		<< "s.operator==('B'):"  << sv.operator==("B") << endl
		<< "s.operator==(ms)"    << sv.operator==(s) << endl
		<< "s.operator==(s):"    << sv.operator==(sv) << endl
		<< endl;
	}

	{
		String sv = strv;
		String s(sv.copy());
		s.operator=("A");
		s.operator=(s);
		s.operator=(sv.copy());
	}
	
	{
		String sv = strv;
		String     s(sv.copy());
		sv.operator=("A");
		sv.operator=(s);
		sv.operator=(sv);
	}
	
	{
		String sv = strv;
		String s(sv.copy());
		s + "A";
		s + s;
		s + sv;
	}
	
	{
		String sv = strv;
		String s(sv.copy());
		sv + "A";
		sv + s;
		sv + sv;
	}

	{
		String sv = strv;
		String s(sv.copy());
		s += "A";
		s += s;
		s += sv;
    //sv += s;
	}
	

	{
		String sv = strv;
		String s(sv.copy());
		s.split("A");
		s.replace("A", "K");
		s.replace_all("A", "V");
		cout
		<< endl
		<< "length:" << sv.length() << endl
		<< sv.index_of("B") << endl
		<< sv.last_index_of("B") << endl
		<< endl;
	}

}

int test2_str(int argc, char *argv[]) {

	std::string s = std::string("-hello").substr(1,5) + "-";
	
	s.data();
	
	char* a = const_cast<char*>(s.c_str());
	a[0] = 'U';
	
	vector<String> A{"a", "b"};
	
	std::for_each(++A.begin(), A.end(), [](String a) {
		return a;
	});
	
	Array<String> b{ "A", "B" };
	
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
	<< "std::hash<String>():" << std::hash<String>()("ABCD") << endl
	<< endl;

	return 0;
}
