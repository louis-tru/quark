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

#ifndef __quark__util__numbers__
#define __quark__util__numbers__

#include "./object.h"

namespace qk {

	template <typename T> class Number: public Object {
	public:
		inline Number(T v): value(v) { }
		inline T operator*() { return value; }
		inline Number& operator++() { value++; return *this; } // ++i
		inline Number& operator--() { value--; return *this; } // --i
		inline Number  operator++(int) { T v = value; value++; return v; } // i++
		inline Number  operator--(int) { T v = value; value--; return v; } // i--
		template <typename T2> inline T operator=(T2 v) { value = v.value; return value; }
		template <typename T2> inline bool operator==(T2 v) { return value == v.value; }
		template <typename T2> inline bool operator!=(T2 v) { return value != v.value; }
		template <typename T2> inline bool operator<(T2 v) { return value < v.value; }
		template <typename T2> inline bool operator>(T2 v) { return value > v.value; }
		template <typename T2> inline bool operator<=(T2 v) { return value <= v.value; }
		template <typename T2> inline bool operator>=(T2 v) { return value >= v.value; }
		T value;
		static const T limit_min;
		static const T limit_max;

		static inline T max(T a, T b) {
			return a > b ? a: b;
		}
		static inline T min(T a, T b) {
			return a < b ? a: b;
		}
		static inline T clamp(T x, T min, T max) {
			return x < min ? min: x > max ? max: x;
		}
	};

	#define define_number(N, T) \
		typedef Number<T> N; template<> \
		Qk_EXPORT const T N::limit_min; template<> const T N::limit_max

	define_number(Int8, int8_t);   define_number(Uint8 , uint8_t);
	define_number(Int16, int16_t); define_number(Uint16, uint16_t);
	define_number(Int32, int32_t); define_number(Uint32, uint32_t);
	define_number(Int64, int64_t); define_number(Uint64, uint64_t);
	define_number(Float32, float); define_number(Float64, double);
	define_number(Bool, bool);

	#undef define_number
}
#endif
