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

#include "./dict.h"

template<> uint64_t Compare<char>::hash_code(const char& key) { return *reinterpret_cast<uint8_t*>(key); }
template<> uint64_t Compare<uint8_t>::hash_code(const uint8_t& key) { return key; }
template<> uint64_t Compare<int16_t>::hash_code(const int16_t& key) { return *reinterpret_cast<uint16_t*>(key); }
template<> uint64_t Compare<uint16_t>::hash_code(const uint16_t& key) { return key; }
template<> uint64_t Compare<int>::hash_code(const int& key) { return *reinterpret_cast<uint32_t*>(&key); }
template<> uint64_t Compare<uint32_t>::hash_code(const uint32_t& key) { return key; }
template<> uint64_t Compare<int64_t>::hash_code(const int64_t& key) { return key; }
template<> uint64_t Compare<uint64_t>::hash_code(const uint64_t& key) { return key; }
template<> uint64_t Compare<float>::hash_code(const float& key) { return *reinterpret_cast<uint32_t*>(&key); }
template<> uint64_t Compare<double>::hash_code(const double& key) { return *reinterpret_cast<uint64_t*>(&key); }
template<> uint64_t Compare<bool>::hash_code(const bool& key) { return key; }
