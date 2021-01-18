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


#ifndef __ftr__util__http__cookie__
#define __ftr__util__http__cookie__

#include <ftr/util/util.h>
#include <string>
#include <unordered_map>

namespace ftr {

	FX_EXPORT String http_cookie_get(const String& domain,
																	const String& name,
																	const String& path = String(), bool ssl = 0);

	FX_EXPORT String http_cookie_get_all_string(const String& domain,
																							const String& path = String(), bool ssl = 0);

	FX_EXPORT std::unordered_map<String, String> http_cookie_get_all(const String& domain,
																																					 const String& path = String(), bool ssl = 0);

	FX_EXPORT void http_cookie_set_with_expression(const String& domain, const String& expression);

	FX_EXPORT void http_cookie_set(const String& domain,
																const String& name,
																const String& value,
																int64_t expires = -1, const String& path = String(), bool ssl = 0);

	FX_EXPORT void http_cookie_delete(const String& domain, 
																		const String& name, const String& path = String(), bool ssl = 0);

	FX_EXPORT void http_cookie_delete_all(const String& domain, bool ssl = 0);

	FX_EXPORT void http_cookie_clear();

}
#endif
