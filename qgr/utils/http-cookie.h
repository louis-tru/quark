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


#ifndef __qgr__utils__http__cookie__
#define __qgr__utils__http__cookie__

#include "qgr/utils/util.h"
#include "qgr/utils/string.h"
#include "qgr/utils/map.h"

XX_NS(qgr)

XX_EXPORT String http_cookie_get(cString& domain,
																 cString& name,
																 cString& path = String(), bool secure = 0);

XX_EXPORT String http_cookie_get_all_string(cString& domain,
																						cString& path = String(), bool secure = 0);

XX_EXPORT Map<String, String> http_cookie_get_all(cString& domain,
																									cString& path = String(), bool secure = 0);

XX_EXPORT void http_cookie_set_with_expression(cString& domain, cString& expression);

XX_EXPORT void http_cookie_set(cString& domain,
															 cString& name,
															 cString& value,
															 int64 expires = -1, cString& path = String(), bool secure = 0);

XX_EXPORT void http_cookie_delete(cString& domain, cString& name, cString& path = String());

XX_EXPORT void http_cookie_delete_all(cString& domain);

XX_EXPORT void http_cookie_clear();

XX_END
#endif
