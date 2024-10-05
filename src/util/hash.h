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

#ifndef __quark__util__hash__
#define __quark__util__hash__

#include "./object.h"
#include "./string.h"

namespace qk {

	class Qk_Export Hash5381 {
		uint64_t _hash;
	public:
		inline Hash5381(): _hash(5381) {}
		inline Hash5381(uint64_t hash): _hash(hash) {}
		uint64_t hashCode() const { return _hash; }
		void   update(const void* data, uint32_t len);
		void   updatestr(cString& str);
		void   updateu16v(const uint16_t *data, uint32_t len);
		void   updateu32v(const uint32_t *data, uint32_t len);
		void   updateu64v(const uint64_t *data, uint32_t len);
		void   updateu64(const uint64_t data);
		void   updatef(float data);
		void   updatefv2(const float data[2]);
		void   updatefv4(const float data[4]);
		String digest();
	};

	Qk_Export uint64_t hashCode(const void* data, uint32_t len);
	Qk_Export String hash(const void* data, uint32_t len);
	Qk_Export String hash(cString& str);
}
#endif
