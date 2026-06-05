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

#include "./hash.h"

namespace qk {

	static inline uint64_t byte_swap64(uint64_t v) {
	#if defined(__GNUC__) || defined(__clang__)
		return __builtin_bswap64(v);
	#else
		return ((v & 0x00000000000000ffULL) << 56) |
			((v & 0x000000000000ff00ULL) << 40) |
			((v & 0x0000000000ff0000ULL) << 24) |
			((v & 0x00000000ff000000ULL) << 8) |
			((v & 0x000000ff00000000ULL) >> 8) |
			((v & 0x0000ff0000000000ULL) >> 24) |
			((v & 0x00ff000000000000ULL) >> 40) |
			((v & 0xff00000000000000ULL) >> 56);
	#endif
	}

	static inline uint64_t load64_le(const void *p) {
		uint64_t v;
		memcpy(&v, p, 8);
#if Qk_BIG_ENDIAN
		v = byte_swap64(v);
#endif
		return v;
	}

	static inline uint64_t load_tail_le(const void *p, uint32_t len) {
		uint64_t v = 0;
		memcpy(&v, p, len);
#if Qk_BIG_ENDIAN
		v = byte_swap64(v);
#endif
		return v;
	}

	///////////////////////////////////////////////////////////////////////////

	// 32 位 avalanche 收尾混合器。
	// 用来把已经收集好的整数/hash 状态进一步打散；它本身不是
	// streaming/buffer hash。
	uint32_t mix32(uint32_t x) {
		x ^= x >> 16;
		x *= 0x7feb352du;
		x ^= x >> 15;
		x *= 0x846ca68bu;
		x ^= x >> 16;
		return x;
	}

	// 更便宜的 32 位混合器，比 mix32 快，但 avalanche 质量更弱。
	uint32_t mix32_fast(uint32_t x) {
		x ^= x >> 17;
		x *= 0xed5ad4bbu;
		x ^= x >> 11;
		return x;
	}
	
	// 最轻量的 xor-shift 混合器。适合简单扰动；如果输入模式很规则，
	// 或需要更强的哈希表 key 分布，不建议用这个。
	uint32_t mix32_fastest(uint32_t x) {
		x ^= x >> 16;
		x ^= x << 9;
		x ^= x >> 5;
		return x;
	}

	// SplitMix64 收尾混合器。
	// 适合整数 key、随机种子、最终 hash 状态的 64 位 avalanche 扩散。
	// 它本身不是完整的 buffer hash。
	uint64_t mix64(uint64_t x) {
		x ^= x >> 30;
		x *= 0xbf58476d1ce4e5b9ULL;
		x ^= x >> 27;
		x *= 0x94d049bb133111ebULL;
		x ^= x >> 31;
		return x;
	}

	// MurmurHash3 fmix64 风格的收尾混合器。
	// 历史上的 "xx" 名称只是表示 xxhash/hash finalizer 这一类用途；
	// 这个函数只是 64 位 avalanche mixer，不是完整的 xxHash64 streaming 算法。
	uint64_t mix64_xx(uint64_t x) {
		x ^= x >> 33;
		x *= 0xff51afd7ed558ccdULL;
		x ^= x >> 33;
		x *= 0xc4ceb9fe1a85ec53ULL;
		x ^= x >> 33;
		return x;
	}

	// 快速 64 位混合器，对应 mix32_fast。
	// 使用一个 wyhash 来源的强乘法常量和几次 xor shift；适合速度比最高
	// avalanche 质量更重要的场景。
	uint64_t mix64_fast(uint64_t x) {
		x ^= x >> 33;
		// x *= 0xed5ad4bbcb3e515dULL;   // 对应 ed5ad4bb 的 64-bit 扩展
		x *= 0xd6e8feb86659fd93ULL; // wyhash 的核心常量，强度极高
		x ^= x >> 28;
		return x;
	}

	// 最简单的 64 位 xor-shift 混合器，对应 mix32_fastest。
	// 这只是一个轻量 bit scrambler。
	uint64_t mix64_fastest(uint64_t x) {
		x ^= x >> 32;
		x ^= x << 13;
		x ^= x >> 7;
		return x;
	}

	// 把两个 32 位整数混合成一个 32 位 hash。
	// 使用 32x32 => 64 乘法做交叉扩散，适合滚动 hash/combine 的默认选择。
	uint32_t mix32_combine(uint32_t a, uint32_t b) {
		uint32_t x = a ^ 0x9e3779b9u;
		uint32_t y = b ^ 0x85ebca6bu;
		uint64_t r = uint64_t(x) * uint64_t(y);
		return uint32_t(r) ^ uint32_t(r >> 32);
	}

	// 快速 32 位 combine。
	// 比 mix32_combine 少一次 64 位乘法，适合质量要求较低的高频 combine。
	uint32_t mix32_combine_fast(uint32_t a, uint32_t b) {
		return a ^ (b + 0x9e3779b9u + (a << 6) + (a >> 2));
	}

	// 快速 64 位 combine。
	// 不依赖 128 位乘法；比 mix64_combine 便宜，但交叉扩散质量更弱。
	uint64_t mix64_combine_fast(uint64_t a, uint64_t b) {
		return a ^ (b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2));
	}

	// 把两个 64 位整数混合成一个 64 位 hash。
	// 优先使用 64x64 => 128 乘法做交叉扩散，适合滚动 hash/combine 的默认选择。
	uint64_t mix64_combine(uint64_t a, uint64_t b) {
	#if defined(__SIZEOF_INT128__)
		uint64_t x = a ^ 0x9e3779b97f4a7c15ULL;
		uint64_t y = b ^ 0xd6e8feb86659fd93ULL;
		__uint128_t r = (__uint128_t)x * y;
		return (uint64_t)r ^ (uint64_t)(r >> 64);
	#else
		uint64_t x = (a ^ 0x9e3779b97f4a7c15ULL) * 0xbf58476d1ce4e5b9ULL;
		uint64_t y = (b ^ 0xd6e8feb86659fd93ULL) * 0x94d049bb133111ebULL;
		return x ^ (x >> 32) ^ y ^ (y >> 29);
	#endif
	}

	// 64 位 combine 的快速版本，对应 mix64_fast，适合质量要求较低的高频 combine。
	static inline uint64_t fnv1a_combine(uint64_t a, uint64_t b) {
		a ^= b;
		a *= 0x100000001b3ULL; // FNV-1a 64-bit prime
		return a;
	}

	uint64_t hash_code(cVoid* data, uint32_t len) {
		Hash hash;
		hash.update(data, len);
		return hash.hashCode();
	}

	String hash_str(cVoid* data, uint32_t len) {
		Hash hash;
		hash.update(data, len);
		return hash.hashStr();
	}

	String hash_str(cString& str) {
		return hash_str(str.c_str(), str.length());
	}

	///////////////////////////////////////////////////////////////////////////

	Hash::Hash()
		// 0x9e3779b97f4a7c15ULL // 黄金分割常数，2^64 / φ
		// 0xcbf29ce484222325ULL // FNV offset
		: _value(0x243f6a8885a308d3ULL) // π 的前64位16进制小数展开，常用作 hash 初始值
	{}

	// 原始字节 buffer hash。
	// 每 8 字节显式按 little-endian 拼成一个 block，再用 mix64_combine_fast()
	// 滚动 combine。输入是内存字节而不是已经 combine 好的整数值时，优先用它。
	void Hash::update(cVoid* data, uint32_t len) {
		const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
		uint32_t i = 0;
		// ---- 每轮处理 8 字节（64-bit） ----
		for (; i + 8 <= len; i += 8) {
			_value = mix64_combine_fast(_value, load64_le(p + i));
		}
		// ---- 处理最后的 0~7 字节 ----
		uint32_t rem = len - i;
		if (rem) {
			uint64_t tail = load_tail_le(p + i, rem);
			_value = mix64_combine_fast(_value, tail ^ (uint64_t(rem) << 56));
		}
	}

	void Hash::updateu16v(const uint16_t *data, uint32_t len) {
		update((const void*)data, len * 2);
	}

	void Hash::updateu32v(const uint32_t *data, uint32_t len) {
		update((const void*)data, len * 4);
	}

	void Hash::updateu64v(const uint64_t *data, uint32_t len) {
		for (uint32_t i = 0; i < len; i++)
			updateu64(data[i]);
	}

	void Hash::updateu64(const uint64_t data) {
		_value = mix64_combine_fast(_value, data);
	}

	void Hash::updatef64(const double data) {
		_value = mix64_combine_fast(_value, reinterpret_cast<const uint64_t&>(data));
	}

	void Hash::updateu32(const uint32_t data) {
		_value = mix64_combine_fast(_value, uint64_t(data)); // update u32
	}

	void Hash::update1f(const float data) {
		union { float f; uint64_t u; } x = { .u=0 };
		x.f = data;
		_value = mix64_combine_fast(_value, x.u); // update
	}

	void Hash::update2f(const float data[2]) {
		_value = mix64_combine_fast(_value, reinterpret_cast<const uint64_t*>(data)[0]); // update
	}

	void Hash::update4f(const float data[4]) {
		_value = mix64_combine_fast(_value, reinterpret_cast<const uint64_t*>(data)[0]); // update u64
		_value = mix64_combine_fast(_value, reinterpret_cast<const uint64_t*>(data)[1]); // update u64
	}

	void Hash::updatestr(cString& str) {
		update(str.c_str(), str.length());
	}

	String Hash::hashStr() {
		constexpr cChar* I64BIT_TABLE =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";
		// 把 64 位 hash 转成字符串。每 6 位用一个字符表示。
		uint64_t hash = hashCode();
		String str;
		do {
			str.append(I64BIT_TABLE[hash & 0x3F]);
		} while (hash >>= 6);
		return str;
	}

	uint32_t Hash::hashCode32() const {
		return mix32_combine_fast(uint32_t(_value >> 32), uint32_t(_value));
	}
}
