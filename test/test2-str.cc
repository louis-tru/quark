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
#include "../src/include/ftr/util/macros.h"
//#include <string.h>
//#include <stdio.h>
//#include <ctype.h>

const char test_big_char[] = { 1, 0, 0, 0 };
const int* test_big_int = (const int*)test_big_char;
const bool has_big_data = *test_big_int != 1;

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

int test2_str(int argc, char *argv[]) {

	std::string s = std::string("-hello").substr(1,5) + "-";
	
	char* a = const_cast<char*>(s.c_str());

	
	
	// std::free(a - 2);
	
	int i = 3/4;
	
	a[0] = 'U';
	
	std::cout
	<< "sizeof(s):" << sizeof(s) << std::endl
	<< "capacity:" << s.capacity() << std::endl
	<< "size:" << s.size() << std::endl
	<< "length:" << s.length() << std::endl
	<< "str:" << s << std::endl
	<< "ptr:" << s.c_str() << std::endl
	<< "uint32_t:" << std::numeric_limits<uint32_t>::max() << std::endl
	<< "has_big_data:" << has_big_data << std::endl
	<< "i:" << i << std::endl
	<< "memcmp:" << memcmp("A", "A", 1) << std::endl
	<< std::endl;

	return 0;
}
