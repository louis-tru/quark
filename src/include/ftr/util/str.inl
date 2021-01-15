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

namespace internal {

	struct str {
		static const char ws[8];
		static bool to_number(const char* i, int32_t* o, int len);
		static bool to_number(const char* i, int64_t* o, int len);
		static bool to_number(const char* i, uint32_t* o, int len);
		static bool to_number(const char* i, uint64_t* o, int len);
		static bool to_number(const char* i, float* o, int len);
		static bool to_number(const char* i, double* o, int len);
		static void strcp(char* o, int size_o, const char* i, int size_i, uint32_t len);
		template <typename Output, typename Input>
		static void strcp(Output* o, const Input* i, uint32_t len) {
			internal::str::strcp(o, sizeof(Output), i, sizeof(Input), len);
		}
		static uint32_t strlen(const char* s, int size_of);
		static int memcmp(const char* s1, const char* s2, uint32_t len, int size_of);
		// 1 > , -1 <, 0 ==
		template <typename T>
		static int memcmp(const T* s1, const T* s2, uint32_t len) {
			return internal::str::memcmp(s1, s2, len, sizeof(T));
		}
		static int32_t sprintf(char** o, uint32_t* capacity, const char* f, ...);
		static int32_t index_of     (const char* s1, uint32_t s1_len, const char* s2, uint32_t s2_len, uint32_t start, int size_of);
		static int32_t last_index_of(const char* s1, uint32_t s1_len, const char* s2, uint32_t s2_len, uint32_t start, int size_of);
		char* replace(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len,
			const char* rep, uint32_t rep_len,
			int size_of, uint32_t* out_len, uint32_t* capacity_out, bool all
		);
	};
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(const T* s)
  : BasicString<T, M, A>(s, internal::str::strlen(s, sizeof(T))) {
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(const T* s, uint32_t len) {
  this->_length = len;
  if (len) {
    this->realloc_(this->_length + 1);
    internal::str::strcp(this->_val, s, this->_length);
  }
}

template <>
BasicString<char>::BasicString(const char* s, uint32_t len);

template <>
BasicString<uint16_t>::BasicString(const uint16_t* s, uint32_t len);

template <>
BasicString<uint32_t>::BasicString(const uint32_t* s, uint32_t len);

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(const T* a, uint32_t a_len, const T* b, uint32_t b_len) {
  static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	this->_length = a_len + b_len;
  this->realloc_(this->_length + 1);
	internal::str::strcp(this->_val, a, a_len);
	internal::str::strcp(this->_val + a_len, b, b_len);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(char i): ArrayBuffer<T, M, A>(1) {
	this->_val[0] = i; this->_val[1] = '\0';
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(int i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%d", i);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(uint32_t i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%u", i);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(int64_t i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	#if FX_ARCH_64BIT
		this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%ld", i);
	#else
		this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%lld", i);
	#endif
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(uint64_t i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	#if FX_ARCH_64BIT
		this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%lu", i);
	#else
		this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%llu", i);
	#endif
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(float i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%f", i);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>::BasicString(double i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	this->_length = internal::str::sprintf(&this->_val, &this->_capacity, "%g", i);
}

template <>
MutableString BasicString<>::format(const char* format, ...);

template <typename T, HolderMode M, typename A>
BasicString<T, M, A> ArrayBuffer<T, M, A>::collapse_string() {
	return BasicString<T, M, A>(*this);
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substr(uint32_t start, uint32_t length) const {
	return BasicString<T, HolderMode::kWeak, A>(this->slice(start, start + length));
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substr(uint32_t start) const {
	return BasicString<T, HolderMode::kWeak, A>(this->slice(start));
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substring(uint32_t start, uint32_t end) const {
	return BasicString<T, HolderMode::kWeak, A>(this->slice(start, end));
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::substring(uint32_t start) const {
	return BasicString<T, HolderMode::kWeak, A>(this->slice(start));
}

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
std::vector<BasicString<T, HolderMode::kWeak, A>> BasicString<T, M, A>::split(const BasicString<T, M2, A2>& sp) const {
	std::vector<BasicString<T, HolderMode::kWeak, A>> r;
	int splen = sp.length();
	int prev = 0;
	int index = 0;
	while ((index = index_of(sp, prev)) != -1) {
		r.push_back(substring(prev, index));
		prev = index + splen;
	}
	r.push_back( substring(prev) );
	return r;
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::trim() const {
	uint32_t start = 0;
	uint32_t end = this->_length;
	for ( ; start < this->_length; start++) {
		if (strchr(internal::str::ws, this->_val[start]) == nullptr) {
			break;
		}
	}
	if (start == this->_length) {
		return BasicString<T, HolderMode::kWeak, A>(); // empty string
	} else {
		for ( ; end > 0; end--) {
			if (strchr(internal::str::ws, this->_val[end - 1]) == nullptr) {
				break;
			}
		}
	}
	if (start == 0 && end == this->_length) {
		return BasicString<T, HolderMode::kWeak, A>(*this);
	}
	return substring(start, end);
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::trim_left() const {
	for (uint32_t start = 0; start < this->_length; start++) {
		if (strchr(internal::str::ws, this->_val[start]) == nullptr) {
			if (start == 0) {
				return BasicString<T, HolderMode::kWeak, A>(*this);
			} else {
				return substring(start);
			}
		}
	}
	return BasicString<T, HolderMode::kWeak, A>();
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kWeak, A> BasicString<T, M, A>::trim_right() const {
	for (uint32_t end = this->_length; end > 0; end--) {
		if (strchr(internal::str::ws, this->_val[end - 1]) == nullptr) {
			if (end == this->_length) {
				return BasicString<T, HolderMode::kWeak, A>(*this);
			} else {
				return substring(0, end);
			}
		}
	}
	return BasicString<T, HolderMode::kWeak, A>();
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>&  BasicString<T, M, A>::upper_case() {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	T* s = this->_val;
	for (uint32_t i = 0; i < this->_length; i++, s++) {
		*s = ::toupper(*s);
	}
	return *this;
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>&  BasicString<T, M, A>::lower_case() {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	T* s = this->_val;
	for (uint32_t i = 0; i < this->_length; i++, s++) {
		*s = ::tolower(*s);
	}
	return *this;
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::to_upper_case() const {
	return std::move(this->copy().upper_case());
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::to_lower_case() const {
	return std::move(this->copy().lower_case());
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
int BasicString<T, M, A>::index_of(const BasicString<T, M2, A2>& s, uint32_t start) const {
	return internal::str::index_of(this->_val, this->_length, s.val(), s.length(), start, sizeof(T));
}

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
int BasicString<T, M, A>::last_index_of(const BasicString<T, M2, A2>& s, int start) const {
	return internal::str::last_index_of(this->_val, this->_length, s.val(), s.length(), start, sizeof(T));
}

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
int BasicString<T, M, A>::last_index_of(const BasicString<T, M2, A2>& s) const {
	return internal::str::last_index_of(this->_val, this->_length, s.val(), s.length(), this->_length, sizeof(T));
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
//template<HolderMode M2, typename A2, HolderMode M3, typename A3>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::replace(
	const BasicString& s, const BasicString& rep
) const {
	uint32_t len, capacity;
	void* val = internal::str::replace(this->_val, this->_length,
																		 s._val, s._length,
																		 rep._val, rep._length,
																		 sizeof(T), &len, &capacity, false
	);
	return ArrayBuffer<T, HolderMode::kStrong, A>(val, len, capacity).collapse_string();
}

template <typename T, HolderMode M, typename A>
//template<HolderMode M2, typename A2, HolderMode M3, typename A3>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::replace_all(
	const BasicString& s, const BasicString& rep
) const {
	uint32_t len, capacity;
	void* val = internal::str::replace(this->_val, this->_length, s._val, s._length, rep._val, rep._length, sizeof(T), &len, &capacity, true);
	return ArrayBuffer<T, HolderMode::kStrong, A>(val, len, capacity).collapse_string();
}

// --------------------------------------------------------------------------------

template<typename T, HolderMode M, typename A>
BasicString<T, M, A>& BasicString<T, M, A>::operator=(const BasicString& s) { // Only weak types can be copied assign value
  ArrayBuffer<T, M, A>::operator=(s);
	return *this;
}

template<typename T, HolderMode M, typename A>
BasicString<T, M, A>& BasicString<T, M, A>::operator=(BasicString& s) { // assign ref
	ArrayBuffer<T, M, A>::operator=(std::move(s));
	return *this;
}

template<typename T, HolderMode M, typename A>
BasicString<T, M, A>& BasicString<T, M, A>::operator=(BasicString&& s) { // assign right ref
	ArrayBuffer<T, M, A>::operator=(std::move(s));
	return *this;
}

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
BasicString<T, M, A>& BasicString<T, M, A>::operator+=(const BasicString<T, M2, A2>& s) { // write, Only strong types can be call
	this->write(s);
	return *this;
}

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::operator+(const BasicString<T, M2, A2>& s) const { // concat new
	BasicString<T, HolderMode::kStrong, A> s1(this->copy());
	s1.write(s);
	return std::move(s1);
}

template <typename T, HolderMode M, typename A>
BasicString<T, M, A>& BasicString<T, M, A>::operator+=(const BasicString& s) { // write, Only strong types can be call
	this->write(s);
	return *this;
}

template <typename T, HolderMode M, typename A>
BasicString<T, HolderMode::kStrong, A> BasicString<T, M, A>::operator+(const BasicString& s) const { // concat new
	BasicString<T, HolderMode::kStrong, A> s1(this->copy());
	s1.write(s);
	return std::move(s1);
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator==(const BasicString<T>& s) const {
	return internal::str::memcmp(this->_val, s.val(), this->_length) == 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator!=(const BasicString<T>& s) const {
	return internal::str::memcmp(this->_val, s.val(), this->_length) != 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator>(const BasicString<T>& s) const {
	return internal::str::memcmp(this->_val, s.val(), this->_length) > 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator<(const BasicString<T>& s) const {
	return internal::str::memcmp(this->_val, s.val(), this->_length) < 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator>=(const BasicString<T>& s) const {
	return internal::str::memcmp(this->_val, s.val(), this->_length) >= 0;
}

template <typename T, HolderMode M, typename A>
bool BasicString<T, M, A>::operator<=(const BasicString<T>& s) const {
	return internal::str::memcmp(this->_val,s.val(), this->_length) <= 0;
}

// --------------------------------------------------------------------------------

template<typename T, HolderMode M, typename A>
template<typename T2>
T2 BasicString<T, M, A>::to_number() const {
	T2 o;
	internal::str::to_number(this->_val, &o, this->_length);
	return o;
}

template<typename T, HolderMode M, typename A>
template<typename T2>
bool BasicString<T, M, A>::to_number(T2* o) const {
	return internal::str::to_number(this->_val, o, this->_length);
}
