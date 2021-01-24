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
		static int32_t index_of(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len, uint32_t start, int size_of
		);
		static int32_t last_index_of(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len, uint32_t start, int size_of
		);
		static char* replace(
			const char* s1, uint32_t s1_len,
			const char* s2, uint32_t s2_len,
			const char* rep, uint32_t rep_len,
			int size_of, uint32_t* out_len, uint32_t* capacity_out, bool all
		);
	};
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(): _length(0), _capacity(0), _val(nullptr) {
}

template<typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
ArrayBuffer<T, M, A>::ArrayBuffer(const ArrayBuffer<T, M2, A2>& arr)
	: ArrayBuffer(arr._length, arr._capacity, const_cast<T*>(arr._val)) {
	static_assert(M == HolderMode::kWeak, "Only weak types can be copied ..");
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(const ArrayBuffer& arr)
	: ArrayBuffer(arr._length, arr._capacity, const_cast<T*>(arr._val)) { // Only weak types can be copied
	static_assert(M == HolderMode::kWeak, "Only weak types can be copied ...");
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(ArrayBuffer& arr): ArrayBuffer(std::move(arr))
{}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(uint32_t length, uint32_t capacity)
: _length(length), _capacity(0), _val(nullptr)
{
	realloc_(FX_MAX(length, capacity));
	if (_length) {
		T* begin = _val;
		T* end = begin + _length;
		while (begin < end) {
			new(begin) T(); // 调用默认构造
			begin++;
		}
	}
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(uint32_t length, uint32_t capacity, T* data)
: _length(length), _capacity(FX_MAX(capacity, length)), _val(data)
{
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(ArrayBuffer&& arr): _length(0), _capacity(0), _val(nullptr)
{
	operator=(std::move(arr));
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(const std::initializer_list<T>& list)
: _length((uint32_t)list.size()), _capacity(0), _val(nullptr)
{
	realloc_(_length);
	T* begin = _val;
	for (auto& i : list) {
		new(begin) T(std::move(i)); // 调用默认构造
		begin++;
	}
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(const T* s)
	: ArrayBuffer<T, M, A>(s, internal::str::strlen(s, sizeof(T))) {
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(const T* s, uint32_t len): _length(len), _capacity(0), _val(nullptr) {
	if (len) {
		realloc_(_length + 1);
		internal::str::strcp(_val, s, _length);
	}
}

template <>
ArrayBuffer<char, HolderMode::kWeak>::ArrayBuffer(const char* s, uint32_t len);

template <>
ArrayBuffer<uint16_t, HolderMode::kWeak>::ArrayBuffer(const uint16_t* s, uint32_t len);

template <>
ArrayBuffer<uint32_t, HolderMode::kWeak>::ArrayBuffer(const uint32_t* s, uint32_t len);

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(const T* a, uint32_t a_len, const T* b, uint32_t b_len) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	_length = a_len + b_len;
	realloc_(_length + 1);
	internal::str::strcp(_val, a, a_len);
	internal::str::strcp(_val + a_len, b, b_len);
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(char i): ArrayBuffer<T, M, A>(1) {
	_val[0] = i; _val[1] = '\0';
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(int i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	_length = internal::str::sprintf(&_val, &_capacity, "%d", i);
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(uint32_t i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	_length = internal::str::sprintf(&_val, &_capacity, "%u", i);
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(int64_t i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	#if FX_ARCH_64BIT
		_length = internal::str::sprintf(&_val, &_capacity, "%ld", i);
	#else
		_length = internal::str::sprintf(&_val, &_capacity, "%lld", i);
	#endif
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(uint64_t i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	#if FX_ARCH_64BIT
		_length = internal::str::sprintf(&_val, &_capacity, "%lu", i);
	#else
		_length = internal::str::sprintf(&_val, &_capacity, "%llu", i);
	#endif
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(float i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	_length = internal::str::sprintf(&_val, &_capacity, "%f", i);
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(double i) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	_length = internal::str::sprintf(&_val, &_capacity, "%g", i);
}

template <>
SString String::format(const char* format, ...);

// --------------------------------------------------------------------------------

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator=(ArrayBuffer& arr) {
	return operator=(std::move(arr));
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator=(ArrayBuffer&& arr) {
	if ( arr._val != _val ) {
		clear();
		_length = arr._length;
		_capacity = arr._capacity;
		_val = arr._val;
		if (M == HolderMode::kStrong) {
			arr._length = 0;
			arr._capacity = 0;
			arr._val = nullptr;
		}
	}
	return *this;
}

template<typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator=(const ArrayBuffer<T, M2, A2>& arr) {
	static_assert(M == HolderMode::kWeak, "operator=(), Only weak types can be copied assign value .");
	_length = arr._length;
	_capacity = arr._capacity;
  _val = const_cast<T*>(arr._val);
  return *this;
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator=(const ArrayBuffer& arr) {
	static_assert(M == HolderMode::kWeak, "operator=(), Only weak types can be copied assign value ..");
	_length = arr._length;
	_capacity = arr._capacity;
	_val = const_cast<T*>(arr._val);
	return *this;
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::operator+(const ArrayBuffer<T, HolderMode::kWeak, A>& s) const { // concat new
	ArrayBuffer<T, HolderMode::kStrong, A> s1(copy());
	s1.write(s);
	return std::move(s1);
}

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::operator+(const ArrayBuffer<T, M2, A2>& s) const { // concat new
	ArrayBuffer<T, HolderMode::kStrong, A> s1(copy());
	s1.write(s);
	return std::move(s1);
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator+=(const ArrayBuffer<T, HolderMode::kWeak, A>& s) { // write, Only strong types can be call
	write(s);
	return *this;
}

template <typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator+=(const ArrayBuffer<T, M2, A2>& s) { // write, Only strong types can be call
	write(s);
	return *this;
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
bool ArrayBuffer<T, M, A>::operator==(const T* s) const {
	return internal::str::memcmp(_val, s, _length) == 0;
}

template <typename T, HolderMode M, typename A>
bool ArrayBuffer<T, M, A>::operator!=(const T* s) const {
	return internal::str::memcmp(_val, s, _length) != 0;
}

template <typename T, HolderMode M, typename A>
bool ArrayBuffer<T, M, A>::operator>(const T* s) const {
	return internal::str::memcmp(_val, s, _length) > 0;
}

template <typename T, HolderMode M, typename A>
bool ArrayBuffer<T, M, A>::operator<(const T* s) const {
	return internal::str::memcmp(_val, s, _length) < 0;
}

template <typename T, HolderMode M, typename A>
bool ArrayBuffer<T, M, A>::operator>=(const T* s) const {
	return internal::str::memcmp(_val, s, _length) >= 0;
}

template <typename T, HolderMode M, typename A>
bool ArrayBuffer<T, M, A>::operator<=(const T* s) const {
	return internal::str::memcmp(_val, s, _length) <= 0;
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
std::vector<ArrayBuffer<T, HolderMode::kWeak, A>>
ArrayBuffer<T, M, A>::split(const ArrayBuffer<T, HolderMode::kWeak, A>& sp) const {
	std::vector<ArrayBuffer<T, HolderMode::kWeak, A>> r;
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
ArrayBuffer<T, HolderMode::kWeak, A> ArrayBuffer<T, M, A>::trim() const {
	uint32_t start = 0;
	uint32_t end = _length;
	for ( ; start < _length; start++) {
		if (strchr(internal::str::ws, _val[start]) == nullptr) {
			break;
		}
	}
	if (start == _length) {
		return ArrayBuffer<T, HolderMode::kWeak, A>(); // empty string
	} else {
		for ( ; end > 0; end--) {
			if (strchr(internal::str::ws, _val[end - 1]) == nullptr) {
				break;
			}
		}
	}
	if (start == 0 && end == _length) {
		return ArrayBuffer<T, HolderMode::kWeak, A>(*this);
	}
	return substring(start, end);
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kWeak, A> ArrayBuffer<T, M, A>::trim_left() const {
	for (uint32_t start = 0; start < _length; start++) {
		if (strchr(internal::str::ws, _val[start]) == nullptr) {
			if (start == 0) {
				return ArrayBuffer<T, HolderMode::kWeak, A>(*this);
			} else {
				return substring(start);
			}
		}
	}
	return ArrayBuffer<T, HolderMode::kWeak, A>();
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kWeak, A> ArrayBuffer<T, M, A>::trim_right() const {
	for (uint32_t end = _length; end > 0; end--) {
		if (strchr(internal::str::ws, _val[end - 1]) == nullptr) {
			if (end == _length) {
				return ArrayBuffer<T, HolderMode::kWeak, A>(*this);
			} else {
				return substring(0, end);
			}
		}
	}
	return ArrayBuffer<T, HolderMode::kWeak, A>();
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>&  ArrayBuffer<T, M, A>::upper_case() {
	static_assert(M == HolderMode::kStrong, "upper_case(), the weak holder cannot be changed");
	T* s = _val;
	for (uint32_t i = 0; i < _length; i++, s++) {
		*s = ::toupper(*s);
	}
	return *this;
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>&  ArrayBuffer<T, M, A>::lower_case() {
	static_assert(M == HolderMode::kStrong, "lower_case(), the weak holder cannot be changed");
	T* s = _val;
	for (uint32_t i = 0; i < _length; i++, s++) {
		*s = ::tolower(*s);
	}
	return *this;
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::to_upper_case() const {
	return std::move(copy().upper_case());
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::to_lower_case() const {
	return std::move(copy().lower_case());
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
int ArrayBuffer<T, M, A>::index_of(const Weak& s, uint32_t start) const {
	return internal::str::index_of(_val, _length, s.val(), s.length(), start, sizeof(T));
}

template <typename T, HolderMode M, typename A>
int ArrayBuffer<T, M, A>::last_index_of(const Weak& s, int start) const {
	return internal::str::last_index_of(_val, _length, s.val(), s.length(), start, sizeof(T));
}

template <typename T, HolderMode M, typename A>
int ArrayBuffer<T, M, A>::last_index_of(const Weak& s) const {
	return internal::str::last_index_of(_val, _length, s.val(), s.length(), _length, sizeof(T));
}

// --------------------------------------------------------------------------------

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::replace(const Weak& s, const Weak& rep) const {
	uint32_t len, capacity;
	T* val = internal::str::replace(
		_val, _length, s._val, s._length, 
		rep._val, rep._length, sizeof(T), &len, &capacity, false
	);
	return Strong(len, capacity, val);
}

template <typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::replace_all(const Weak& s, const Weak& rep) const {
	uint32_t len, capacity;
	T* val = internal::str::replace(_val, _length, s._val, s._length,
		rep._val, rep._length, sizeof(T), &len, &capacity, true
	);
	return Strong(len, capacity, val);
}

// --------------------------------------------------------------------------------

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::push(const T& item) {
	_length++;
	realloc_(_length);
	new(_val + _length - 1) T(item);
	//	return _length;
	return *this;
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::push(T&& item) {
	_length++;
	realloc_(_length);
	new(_val + _length - 1) T(std::move(item));
	return *this;
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::push(const Weak& src) {
	write(src);
	return *this;
}

uint64_t hash_code(const void* data, uint32_t len);

template<typename T, HolderMode M, typename A>
uint64_t ArrayBuffer<T, M, A>::hash_code() const {
	return ftr::hash_code(_val, size());
}

template<typename T, HolderMode M, typename A>
template<typename T2>
T2 ArrayBuffer<T, M, A>::to_number() const {
	T2 o;
	internal::str::to_number(_val, &o, _length);
	return o;
}

template<typename T, HolderMode M, typename A>
template<typename T2>
bool ArrayBuffer<T, M, A>::to_number(T2* o) const {
	return internal::str::to_number(_val, o, _length);
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::concat_(T* src, uint32_t src_length) {
	if (src_length) {
		_length += src_length;
		realloc_(_length);
		T* end = _val + _length;
		T* to = end - src_length;
		while (to < end) {
			new(to) T(std::move(*src)); // 调用移动构造
			src++; to++;
		}
	}
	return *this;
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kWeak, A> ArrayBuffer<T, M, A>::slice(uint32_t start, uint32_t end) const {
	end = FX_MIN(end, _length);
	if (start < end) {
		return ArrayBuffer<T, HolderMode::kWeak, A>(_val + start, end - start);
	} else {
		return ArrayBuffer<T, HolderMode::kWeak, A>();
	}
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, HolderMode::kStrong, A> ArrayBuffer<T, M, A>::slice_(uint32_t start, uint32_t end) const {
	end = FX_MIN(end, _length);
	if (start < end) {
		ArrayBuffer<T, HolderMode::kStrong, A> arr;
		arr._length = end - start;
		arr.realloc_(arr._length);
		T* to = arr._val;
		T* e = to + arr._length;
		const T* src = _val + start;
		while (to < e) {
			new(to) T(*src);
			to++; src++;
		}
		return std::move(arr);
	}
	return ArrayBuffer<T, HolderMode::kStrong, A>();
}

template<typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
uint32_t ArrayBuffer<T, M, A>::write(const ArrayBuffer<T, M2, A2>& arr, int to, int size_src, uint32_t form_src) {
	int s = FX_MIN(arr._length - form_src, size_src < 0 ? arr._length : size_src);
	if (s > 0) {
		return write(arr._val + form_src, to, s);
	}
	return 0;
}

/**
 * @func write
 */
template<typename T, HolderMode M, typename A>
uint32_t ArrayBuffer<T, M, A>::write(const T* src, int to, uint32_t size_src) {
	if (size_src) {
		if ( to == -1 ) to = _length;
		uint32_t old_len = _length;
		uint32_t end = to + size_src;
		_length = FX_MAX(end, _length);
		realloc_(_length);
		T* to_ = _val + to;
		
		for (int i = to; i < end; i++) {
			if (i < old_len) {
				reinterpret_cast<Sham*>(to_)->~Sham(); // 先释放原对像
			}
			new(to_) T(*src);
			to_++; src++;
		}
	}
	return size_src;
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::pop(uint32_t count) {
	int j = FX_MAX(_length - count, 0);
	if (_length > j) {
		do {
			_length--;
			reinterpret_cast<Sham*>(_val + _length)->~Sham(); // 释放
		} while (_length > j);
		realloc_(_length);
	}
	//	return _length;
	return *this;
}

template<typename T, HolderMode M, typename A>
T* ArrayBuffer<T, M, A>::collapse() {
	if (M == HolderMode::kWeak) {
		return nullptr;
	}
	T* r = _val;
	_capacity = 0;
	_length = 0;
	_val = nullptr;
	return r;
}

template<typename T, HolderMode M, typename A>
void ArrayBuffer<T, M, A>::clear() {
	if (_capacity) {
		if (M == HolderMode::kStrong) {
			T* i = _val;
			T* end = i + _length;
			while (i < end) {
				reinterpret_cast<Sham*>(i)->~Sham(); // 释放
				i++;
			}
			A::free(_val); /* free */
		}
		_length = 0;
		_capacity = 0;
		_val = nullptr;
	}
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>&& ArrayBuffer<T, M, A>::realloc(uint32_t capacity) {
	if (capacity < _length) { // clear Partial data
		T* i = _val + capacity;
		T* end = i + _length;
		while (i < end) {
			reinterpret_cast<Sham*>(i)->~Sham(); // 释放
		}
		_length = capacity;
	}
	realloc_(capacity);
	return std::move(*this);
}

template<typename T, HolderMode M, typename A>
void ArrayBuffer<T, M, A>::realloc_(uint32_t capacity) {
	static_assert(M == HolderMode::kStrong, "the weak holder cannot be changed");
	if ( capacity ) {
		capacity = FX_MAX(FX_MIN_CAPACITY, capacity);
		if ( capacity > _capacity || capacity < _capacity / 4.0 ) {
			capacity = powf(2, ceil(log2(capacity)));
			uint32_t size = sizeof(T) * capacity;
			_capacity = capacity;
			_val = static_cast<T*>(_val ? A::realloc(_val, size) : A::alloc(size));
		}
		ASSERT(_val);
	} else {
		A::free(_val);
		_capacity = 0;
		_val = nullptr;
	}
}

#define FX_DEF_ARRAY_SPECIAL(T, M, A) \
	template<>                          ArrayBuffer<T, M, A>::ArrayBuffer(uint32_t length, uint32_t capacity); /*Strong*/ \
	template<>                          ArrayBuffer<T, M, A>::ArrayBuffer(const std::initializer_list<T>& list); /*Strong*/ \
	template<> ArrayBuffer<T, M, A>&    ArrayBuffer<T, M, A>::concat_(T* src, uint32_t src_length); /*Strong*/ \
	template<> uint32_t                 ArrayBuffer<T, M, A>::write(const T* src, int to, uint32_t size); /*Strong*/ \
	template<> ArrayBuffer<T, M, A>&    ArrayBuffer<T, M, A>::pop(uint32_t count); /*Strong*/ \
	template<> void                     ArrayBuffer<T, M, A>::clear(); /*Strong/Weak*/ \
	template<> ArrayBuffer<T, M, A>&&   ArrayBuffer<T, M, A>::realloc(uint32_t capacity); /*Strong*/ \
	FX_DEF_ARRAY_SPECIAL_SLICE(T, M, A)

#define FX_DEF_ARRAY_SPECIAL_SLICE(T, M, A) \
	template<> ArrayBuffer<T, HolderMode::kStrong, A> \
																					ArrayBuffer<T, M, A>::slice_(uint32_t start, uint32_t end) const /*Strong/Weak*/
#define FX_DEF_ARRAY_SPECIAL_ALL(T) \
	FX_DEF_ARRAY_SPECIAL(T, HolderMode::kStrong, AllocatorDefault); \
	FX_DEF_ARRAY_SPECIAL_SLICE(T, HolderMode::kWeak, AllocatorDefault)

FX_DEF_ARRAY_SPECIAL_ALL(char);
FX_DEF_ARRAY_SPECIAL_ALL(unsigned char);
FX_DEF_ARRAY_SPECIAL_ALL(int16_t);
FX_DEF_ARRAY_SPECIAL_ALL(uint16_t );
FX_DEF_ARRAY_SPECIAL_ALL(int32_t);
FX_DEF_ARRAY_SPECIAL_ALL(uint32_t);
FX_DEF_ARRAY_SPECIAL_ALL(int64_t);
FX_DEF_ARRAY_SPECIAL_ALL(uint64_t);
FX_DEF_ARRAY_SPECIAL_ALL(float);
FX_DEF_ARRAY_SPECIAL_ALL(double);
