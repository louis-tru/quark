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

#ifndef __flare__utils__os__
#define __flare__utils__os__

#include "./util.h"
#include "./object.h"

namespace flare {
	namespace os {

		// util
		FX_EXPORT String name();
		FX_EXPORT String version();
		FX_EXPORT String brand();
		FX_EXPORT String subsystem();
		FX_EXPORT String info();
		FX_EXPORT String languages();
		FX_EXPORT String language();
		FX_EXPORT int64_t time();
		FX_EXPORT int64_t time_second();
		FX_EXPORT int64_t time_monotonic();

		// advanced
		FX_EXPORT bool  is_wifi();
		FX_EXPORT bool  is_mobile();
		FX_EXPORT int   network_status();
		FX_EXPORT bool  is_ac_power();
		FX_EXPORT bool  is_battery();
		FX_EXPORT float battery_level();
		FX_EXPORT uint64_t memory();
		FX_EXPORT uint64_t used_memory();
		FX_EXPORT uint64_t available_memory();
		FX_EXPORT float cpu_usage();

	}
}
#endif