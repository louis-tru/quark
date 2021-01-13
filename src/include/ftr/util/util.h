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

#ifndef __ftr__util__util__
#define __ftr__util__util__

#if !defined(__cplusplus)
# error "Please use the c++ compiler"
#endif

#if __cplusplus < 201103L
# error "The compiler does not support c++ 11"
#endif

#include <ftr/util/object.h>
#include <ftr/util/hash.h>
#include <ftr/util/log.h>
#include <ftr/util/numbers.h>

namespace ftr {

	FX_EXPORT int random(uint32_t start = 0, uint32_t end = 0x7fffffff);
	FX_EXPORT int fix_random(uint32_t a, ...);
	FX_EXPORT uint64_t getId();
	FX_EXPORT uint32_t getId32();
	FX_EXPORT int64_t  parse_time(const String& str);
	FX_EXPORT String gmt_time_string(int64_t second);
	FX_EXPORT String platform();
	FX_EXPORT String version();
	FX_EXPORT void exit(int rc);
	FX_EXPORT bool is_exited();
}
#endif
