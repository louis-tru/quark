/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
	Qk_EXPORT uint32_t mix32(uint32_t x);
	Qk_EXPORT uint32_t mix32_fast(uint32_t x);
	Qk_EXPORT uint64_t mix64(uint64_t x);
	Qk_EXPORT uint64_t mix64_fast(uint64_t x);
	Qk_EXPORT uint32_t mix32_combine(uint32_t a, uint32_t b);
	Qk_EXPORT uint32_t mix32_combine_fast(uint32_t a, uint32_t b);
	Qk_EXPORT uint64_t mix64_combine(uint64_t a, uint64_t b);
	Qk_EXPORT uint64_t mix64_combine_fast(uint64_t a, uint64_t b);
	Qk_EXPORT uint64_t hash_code(cVoid* data, uint32_t len);
	Qk_EXPORT String   hash_str(cVoid* data, uint32_t len);
	Qk_EXPORT String   hash_str(cString& str);

	class Qk_EXPORT Hash {
		uint64_t _value;
	public:
		Hash();
		uint64_t value() const { return _value; }
		uint64_t hashCode() const { return mix64_fast(_value); }
		uint32_t hashCode32() const;
		String hashStr();
		void update(cVoid* data, uint32_t len);
		void updatestr(cString& str);
		void updateu16v(const uint16_t *data, uint32_t len);
		void updateu32v(const uint32_t *data, uint32_t len);
		void updateu64v(const uint64_t *data, uint32_t len);
		void updateu64(const uint64_t data);
		void updatef64(const double data);
		void updateu32(const uint32_t data);
		void update1f(const float data);
		void update2f(const float data[2]);
		void update4f(const float data[4]);
	};
}
#endif
