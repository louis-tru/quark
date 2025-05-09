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

#ifndef __quark__os__os__
#define __quark__os__os__

#include "../util/util.h"
#include "../util/array.h"

namespace qk {
	Qk_EXPORT String os_arch();
	Qk_EXPORT String os_name();
	Qk_EXPORT String os_version();
	Qk_EXPORT String os_brand();
	Qk_EXPORT String os_model();
	Qk_EXPORT String os_info();
	Qk_EXPORT cArray<String>& os_languages();
	Qk_EXPORT bool   os_is_wifi();
	Qk_EXPORT bool   os_is_mobile();
	Qk_EXPORT int    os_network_interface();
	Qk_EXPORT bool   os_is_ac_power();
	Qk_EXPORT bool   os_is_battery();
	Qk_EXPORT float  os_battery_level();
	Qk_EXPORT uint64_t os_memory();
	Qk_EXPORT uint64_t os_used_memory();
	Qk_EXPORT uint64_t os_available_memory();
	Qk_EXPORT float os_cpu_usage();
}
#endif
