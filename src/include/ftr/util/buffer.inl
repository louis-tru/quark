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

void fatal(const char* file, uint32_t line, const char* func, const char* msg = 0, ...);

template<typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
ArrayBuffer<T, M, A>::ArrayBuffer(const ArrayBuffer<T, M2, A2>& arr)
	: ArrayBuffer(const_cast<T*>(arr.val(), arr.length(), arr.capacity())) {
	static_assert(M == HolderMode::kWeak, "Only weak types can be copied");
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(ArrayBuffer& arr): ArrayBuffer(std::move(arr))
{}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(T* data, uint32_t length, uint32_t capacity)
: _length(length), _capacity(FX_MAX(capacity, length)), _val(data)
{
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>::ArrayBuffer(ArrayBuffer&& arr): _length(0), _capacity(0), _val(nullptr)
{
	operator=(std::move(arr));
}

template<typename T, HolderMode M, typename A>
template<HolderMode M2, typename A2>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::operator=(const ArrayBuffer<T, M2, A2>& arr) {
	static_assert(M == HolderMode::kWeak, "Only weak types can be copied assign value");
	_length = arr._length;
	_capacity = arr._capacity;
	_val = arr._val;
}

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
T& ArrayBuffer<T, M, A>::operator[](uint32_t index) {
	ASSERT(index < _length, "ArrayBuffer access violation.");
	return _val[index];
}

template<typename T, HolderMode M, typename A>
const T& ArrayBuffer<T, M, A>::operator[](uint32_t index) const {
	ASSERT(index < _length, "ArrayBuffer access violation.");
	return _val[index];
}

template<typename T, HolderMode M, typename A>
T* ArrayBuffer<T, M, A>::val() {
	return _val;
}

template<typename T, HolderMode M, typename A>
uint32_t ArrayBuffer<T, M, A>::push(const T& item) {
	_length++;
	realloc_(_length);
	new(_val + _length - 1) T(item);
	return _length;
}

template<typename T, HolderMode M, typename A>
uint32_t ArrayBuffer<T, M, A>::push(T&& item) {
	_length++;
	realloc_(_length);
	new(_val + _length - 1) T(std::move(item));
	return _length;
}

uint64_t hash_code(const void* data, uint32_t len);

template<typename T, HolderMode M, typename A>
uint64_t ArrayBuffer<T, M, A>::hash_code() const {
	return ftr::hash_code(_val, size());
}

template<typename T, HolderMode M, typename A>
ArrayBuffer<T, M, A>& ArrayBuffer<T, M, A>::concat_(T* src, uint32_t src_length) {
	if (src_length) {
		_length += src_length;
		realloc_(_length);
		//T* src = src._val;
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
		return write(_val + form_src, to, s);
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
uint32_t ArrayBuffer<T, M, A>::pop(uint32_t count) {
	int j = FX_MAX(_length - count, 0);
	if (_length > j) {
		do {
			_length--;
			reinterpret_cast<Sham*>(_val + _length)->~Sham(); // 释放
		} while (_length > j);
		realloc_(_length);
	}
	return _length;
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
	template<>                              ArrayBuffer<T, M, A>::ArrayBuffer(uint32_t length, uint32_t capacity); /*Strong*/ \
	template<>                              ArrayBuffer<T, M, A>::ArrayBuffer(const std::initializer_list<T>& list); /*Strong*/ \
	template<> ArrayBuffer<T, M, A>&        ArrayBuffer<T, M, A>::concat_(T* src, uint32_t src_length); /*Strong*/ \
	template<> uint32_t                     ArrayBuffer<T, M, A>::write(const T* src, int to, uint32_t size); /*Strong*/ \
	template<> uint32_t                     ArrayBuffer<T, M, A>::pop(uint32_t count); /*Strong*/ \
	template<> void                         ArrayBuffer<T, M, A>::clear(); /*Strong/Weak*/ \
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
