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

#include <ftr/utils/str.h>
#include <algorithm>

namespace ftr {

	namespace str {

		String& to_lower(String& str) {
			std::transform(str.begin(), str.end(), str.begin(), tolower);
			return str;
		}

		String& to_upper(String& str) {
			std::transform(str.begin(), str.end(), str.begin(), toupper);
			return str;
		}
		
		
	#if FX_GNUC
		 #define FX_STRING_FORMAT(format, str) \
		 String str; \
		 va_list __arg;  \
		 va_start(__arg, format);  \
		 char* __buf = nullptr;  \
		 int __len = vasprintf(&__buf, format, __arg); \
		 if (__buf) {  \
			 str = String(Buffer(__buf,__len));  \
		 } \
		 va_end(__arg)
	 #else
		 #define FX_STRING_FORMAT(format, str) \
		 String str; \
		 va_list __arg;  \
		 va_start(__arg, format);  \
		 uint32_t __len = _vscprintf(format, __arg); \
		 if (__len) {  \
			 char* buf = (char*)malloc(__len+1); \
			 buf[__len] = '\0';  \
			 va_start(__arg, format);  \
			 _vsnprintf_s(buf, __len + 1, format, __arg);  \
			 str = String(Buffer(buf, __len)); \
		 } \
		 va_end(__arg)
	 #endif

		String format(const char* format, ...) {
			// FX_STRING_FORMAT(format, str);
			
			String str;
			va_list __arg;
			va_start(__arg, format);
			char* __buf = nullptr;
			int __len = vasprintf(&__buf, format, __arg);
			if (__buf) {
//				str = String(Buffer(__buf,__len));
			}
			va_end(__arg);
			
			std::string_view v = "A";
			
//			std::basic_string<T>(std::move(*this));
			
			String s(v);
			
//			s.c_str()
			
			s.capacity();
			
			return String("A");
		}

	}

}
