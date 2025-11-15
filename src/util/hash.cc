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

#include "./hash.h"

namespace qk {

	static cChar* I64BIT_TABLE =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

	void Hash5381::update(cVoid* data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + reinterpret_cast<const uint8_t*>(data)[len];
	}

	void Hash5381::updateu16v(const uint16_t *data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + data[len];
	}

	void Hash5381::updateu32v(const uint32_t *data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + data[len];
	}

	void Hash5381::updateu64v(const uint64_t *data, uint32_t len) {
		while (len--)
			_hash += (_hash << 5) + data[len]; // update u64
	}

	void Hash5381::updateu64(const uint64_t data) {
		_hash += (_hash << 5) + data; // update u64
	}

	void Hash5381::updateu32(const uint32_t data) {
		_hash += (_hash << 5) + data; // update u32
	}

	void Hash5381::updatef(float data) {
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(&data); // update
	}

	void Hash5381::updatefv2(const float data[2]) {
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(data); // update
	}

	void Hash5381::updatefv4(const float data[4]) {
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(data); // update u64
		_hash += (_hash << 5) + *reinterpret_cast<const uint64_t*>(data+2); // update u64
	}

	void Hash5381::updatestr(cString& str) {
		update(str.c_str(), str.length());
	}

	String Hash5381::digest() {
		uint64_t hash = _hash;
		_hash = 5381;
		return hash_str(hash);
	}

	///////////////////////////////////////////////////////////////////////////

	uint32_t mix32(uint32_t x) {
		x ^= x >> 16;
		x *= 0x7feb352du;
		x ^= x >> 15;
		x *= 0x846ca68bu;
		x ^= x >> 16;
		return x;
	}

	uint32_t mix32_fast(uint32_t x) {
		x ^= x >> 17;
		x *= 0xed5ad4bbu;
		x ^= x >> 11;
		return x;
	}
	
	uint32_t mix32_fastest(uint32_t x) {
		x ^= x >> 16;
		x ^= x << 9;
		x ^= x >> 5;
		return x;
	}

	// 64-bit mixer
	uint64_t mix64(uint64_t x) {
		x ^= x >> 30;
		x *= 0xbf58476d1ce4e5b9ULL;
		x ^= x >> 27;
		x *= 0x94d049bb133111ebULL;
		x ^= x >> 31;
		return x;
	}

	uint64_t mix64_xx(uint64_t x) {
		x ^= x >> 33;
		x *= 0xff51afd7ed558ccdULL;
		x ^= x >> 33;
		x *= 0xc4ceb9fe1a85ec53ULL;
		x ^= x >> 33;
		return x;
	}

	// Fast mixer, corresponding to mix32_fast
	uint64_t mix64_fast(uint64_t x) {
		x ^= x >> 33;
		// x *= 0xed5ad4bbcb3e515dULL;   // 对应 ed5ad4bb 的 64-bit 扩展
		x *= 0xd6e8feb86659fd93ULL; // wyhash 的核心常量，强度极高
		x ^= x >> 28;
		return x;
	}

	// simplest xor-shift mixer (对应 mix32_fastest)
	uint64_t mix64_fastest(uint64_t x) {
		x ^= x >> 32;
		x ^= x << 13;
		x ^= x >> 7;
		return x;
	}

	// uint32_t hash_number(double v) {
	// 	uint64_t x;
	// 	memcpy(&x, &v, sizeof(v));  // 按 IEEE754 位模式处理
	// 	return mix32((uint32_t)(x ^ (x >> 32)));
	// }

	uint64_t hash_code(cVoid* data, uint32_t len) {
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);

		uint64_t h = 0xcbf29ce484222325ULL;   // FNV-1a 64-bit offset
		const uint64_t FNV_PRIME = 0x100000001b3ULL;

		uint32_t i = 0;
		// ---- 每轮处理 8 字节（64-bit） ----
		for (; i + 8 <= len; i += 8) {
			uint64_t v;
			memcpy(&v, p + i, 8);             // safe bit copy
			h ^= v;
			h *= FNV_PRIME;
		}

		// ---- 处理最后的 0~7 字节 ----
		uint64_t tail = 0;
		uint32_t rem = len - i;

		if (rem) {
			memcpy(&tail, p + i, rem);        // 按原始字节拼成 64bit
			h ^= tail;
			h *= FNV_PRIME;
		}

		return mix64(h);
	}

	String hash_digest(uint64_t hash) {
		String rev;
		do {
			rev.append(I64BIT_TABLE[hash & 0x3F]);
		} while (hash >>= 6);
		return rev;
	}

	String hash_str(cVoid* data, uint32_t len) {
		uint64_t hash = hash_code(data, len);
		return hash_digest(hash);
	}

	String hash_str(cString& str) {
		return hash_str(str.c_str(), str.length());
	}

}
