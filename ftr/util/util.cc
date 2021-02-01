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

#include "./util.h"
#include "../version.h"

namespace ftr {

	int random(uint32_t start, uint32_t end) {
		static uint32_t id;
		srand(uint32_t(time(NULL) + id));
		id = rand();
		return (id % (end - start + 1)) + start;
	}

	int fix_random(uint32_t a, ...) {
		int i = 0;
		int total = a;
		va_list ap;
		va_start(ap, a);
		while (1) {
			uint32_t e = va_arg(ap, uint32_t);
			if (e < 1) {
				break;
			}
			total += e;
		}
		//
		va_start(ap, a);
		int r = random(0, total - 1);
		total = a;
		if (r >= total) {
			while (1) {
				i++;
				uint32_t e = va_arg(ap, uint32_t);
				if (e < 1) {
					break;
				}
				total += e;
				if (r < total) {
					break;
				}
			}
		}
		va_end(ap);
		return i;
	}

	static std::atomic<uint64_t> id(10);

	uint64_t getId() {
		return id++;
	}

	uint32_t getId32() {
		return id++ % Uint32::max;
	}

	String version() {
    static String ver(FTR_VERSION);
		return ver;
	}

	String platform() {
		#if  FX_IOS || FX_OSX
			static String _name("darwin");
		#elif FX_LINUX
			static String _name("linux");
		#elif  FX_ANDROID
      static String _name("android");
		#elif  FX_WIN
			static String _name("win32");
		#else
			# error no support
		#endif
		return _name;
	}

}
