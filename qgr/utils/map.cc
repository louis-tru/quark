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
 * (INCLUDING NEGLIhashCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

#include "qgr/utils/map.h"

XX_NS(qgr)

//  /*
//   * java hashCode
//   * 如果该域是boolean类型，则计算(f?1:0)
//   * 如果该域是byte、char、short或者int类型，则计算(int)f
//   * 如果该域是long类型，则计算(int)(f^(f>>>32))
//   * 如果该域是float类型，则计算Float.floatToIntBits(f)
//   * 如果该域是double类型，则计算Double.doubleToLongBits(f)，然后重复第三个步骤。
//   * 如果该域是一个对象引用，并且该类的equals方法通过递归调用equals方法来比较这个域，
//   同样为这个域递归的调用hashCode，如果这个域为null，则返回0。
//   * 如果该域是数组，则要把每一个元素当作单独的域来处理，递归的运用上述规则，
//   如果数组域中的每个元素都很重要，那么可以使用Arrays.hashCode方法。
//   */

// hash

template<> uint Compare<char>::hash(const char& key) {
	return key;
}

template<> uint Compare<byte>::hash(const byte& key) {
	return key;
}

template<> uint Compare<int16>::hash(const int16& key) {
	return key;
}

template<> uint Compare<uint16>::hash(const uint16& key) {
	return key;
}

template<> uint Compare<int>::hash(const int& key) {
	return key;
}

template<> uint Compare<uint>::hash(const uint& key) {
	return key;
}

template<> uint Compare<int64>::hash(const int64& key) {
	return (uint)(key ^ (key >> 32));
}

template<> uint Compare<uint64>::hash(const uint64& key) {
	return (uint)(key ^ (key >> 32));
}

template<> uint Compare<float>::hash(const float& key) {
	return *reinterpret_cast<const uint*>(&key);
}

template<> uint Compare<double>::hash(const double& key) {
	const uint64* f = reinterpret_cast<const uint64*>(&key);
	return (uint)((*f) ^ ((*f) >> 32));
}

template<> uint Compare<bool>::hash(const bool& key) {
	return key;
}

// equals

template<> bool Compare<String>::equals(const String& a, const String& b, uint ha, uint hb) {
	// 字符串做一个模糊比较,只要hash_code相同就认为字符串是相同的.(完全比较太慢)
	return ha == hb;
}

template<> bool Compare<Ucs2String>::equals(const Ucs2String& a,
																						const Ucs2String& b, uint ha, uint hb) {
	return ha == hb;
}

template<> bool Compare<char>::equals(const char& a, const char& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<byte>::equals(const byte& a, const byte& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<int16>::equals(const int16& a, const int16& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<uint16>::equals(const uint16& a, const uint16& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<int>::equals(const int& a, const int& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<uint>::equals(const uint& a, const uint& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<int64>::equals(const int64& a, const int64& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<uint64>::equals(const uint64& a, const uint64& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<float>::equals(const float& a, const float& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<double>::equals(const double& a, const double& b, uint ha, uint hb) {
	return a == b;
}

template<> bool Compare<bool>::equals(const bool& a, const bool& b, uint ha, uint hb) {
	return a == b;
}

XX_END
